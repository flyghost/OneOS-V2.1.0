#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_mutex.h>
#include <os_sem.h>
#include <os_spinlock.h>
#include <os_list.h>
#include <arch_interrupt.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_clock.h>
#include <os_util.h>
#include <string.h>
#include <ring_blk_buff.h>
#include <dlog.h>
#include <stdio.h>

#include "log_internal.h"

#ifdef OS_USING_DLOG

#ifdef DLOG_OUTPUT_FLOAT
#include <stdio.h>
#endif

#ifdef DLOG_TIME_USING_TIMESTAMP
#include <sys/time.h>
#endif

#define DLOG_FILTER_KW_MAX_LEN          15
#define DLOG_FILTER_TAG_MAX_LEN         15

#define DLOG_NEWLINE_SIGN               "\r\n"
#define DLOG_FRAME_MAGIC                0x10

#ifdef DLOG_USING_ASYNC_OUTPUT
#define DLOG_ASYNC_OUTPUT_STORE_LINES   (DLOG_ASYNC_OUTPUT_BUF_SIZE * 3 / 2 / OS_LOG_BUFF_SIZE)

#if (DLOG_ASYNC_OUTPUT_TASK_PRIORITY > OS_TASK_PRIORITY_MAX - 1)
#error "Dlog async task priority is greater than or equal to OS_TASK_PRIORITY_MAX"
#endif

#endif /* DLOG_USING_ASYNC_OUTPUT */

#ifdef DLOG_USING_COLOR
/*
 * CSI(Control Sequence Introducer/Initiator) sign.
 * More information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START                       "\033["
#define CSI_END                         "\033[0m"

/* Output log front color */
#define F_BLACK                         "30m"
#define F_RED                           "31m"
#define F_GREEN                         "32m"
#define F_YELLOW                        "33m"
#define F_BLUE                          "34m"
#define F_MAGENTA                       "35m"
#define F_CYAN                          "36m"
#define F_WHITE                         "37m"

#define DLOG_COLOR_DEBUG                (OS_NULL)
#define DLOG_COLOR_INFO                 (F_GREEN)
#define DLOG_COLOR_WARN                 (F_YELLOW)
#define DLOG_COLOR_ERROR                (F_RED)
#endif /* DLOG_USING_COLOR */

struct dlog_ctrl_info
{
    os_bool_t           init_ok;
    os_mutex_t          log_locker;

    /* Global level */
    os_uint16_t         global_level;
    
    /* All backends */
    os_list_node_t      backend_list_head;
    
    /* The task log's line buffer */
    char                log_buf_task[OS_LOG_BUFF_SIZE];

#ifdef DLOG_USING_ISR_LOG
    os_ubase_t          irq_save;
    os_spinlock_t       isr_spinlock;
    char                log_buf_isr[OS_LOG_BUFF_SIZE];
#endif

#ifdef DLOG_USING_ASYNC_OUTPUT
    rbb_ctrl_info_t    *async_rbb;
    os_task_t          *async_task;       
    os_sem_t            async_notice_sem;
#endif

#ifdef DLOG_USING_FILTER

    /* All tag's level filter */
    os_list_node_t  tag_lvl_list_head;

    /* Global filter tag and keyword */    
    char            tag[DLOG_FILTER_TAG_MAX_LEN + 1];
    char            keyword[DLOG_FILTER_KW_MAX_LEN + 1];
#endif
};

/* Tag's level filter */
struct dlog_tag_lvl_filter
{
    os_list_node_t  list_node;
    char            tag[DLOG_FILTER_TAG_MAX_LEN + 1];
    os_uint16_t     level;
};
typedef struct dlog_tag_lvl_filter dlog_tag_lvl_filter_t;

struct dlog_frame
{
    os_uint8_t      magic;      /* Magic word is 0x10 ('lo') */
    os_uint8_t      is_raw;
    os_uint16_t     level;
    os_uint32_t     log_len;
    char           *log;
};
typedef struct dlog_frame dlog_frame_t;



static const char *gs_level_output_info[] =
{
    "EM/",
    "A/",
    "C/",
    "E/",
    "W/",
    "N/",
    "I/",
    "D/"
};

#ifdef DLOG_USING_COLOR
/* Color output info */
static const char *gs_color_output_info[] =
{
    F_MAGENTA,              /* Compatible for LOG_EMERG */
    F_MAGENTA,              /* Compatible for LOG_ALERT */
    F_RED,                  /* Compatible for LOG_CRIT */
    DLOG_COLOR_ERROR,
    DLOG_COLOR_WARN,
    F_GREEN,                /* Compatible for LOG_NOTICE */
    DLOG_COLOR_INFO,
    DLOG_COLOR_DEBUG,
};
#endif /* DLOG_USING_COLOR */

/* Dlog local ctrl info */
static struct dlog_ctrl_info gs_dlog_ctrl_info = {0};

static os_size_t dlog_strcpy(os_size_t cur_len, char *dst, const char *src)
{
    const char *src_old;

    src_old = src;
    while (*src != '\0')
    {
        /* Make sure destination has enough space */
        if (cur_len < OS_LOG_BUFF_SIZE)
        {
            *dst = *src;
            cur_len++;
            dst++;
            src++;
        }
        else
        {
            break;
        }
    }
    
    return src - src_old;
}

OS_UNUSED static os_size_t dlog_ultoa(char *str, unsigned long value)
{
    os_size_t i;
    os_size_t j;
    os_size_t len;
    char      swap;

    i   = 0;
    j   = 0;
    len = 0;
    
    do
    {
        str[len] = value % 10 + '0';
        value    = value / 10;
        len++;
    } while (value);
    
    str[len] = '\0';

    /* Reverse string */
    for (i = 0, j = len - 1; i < j; ++i, --j)
    {
        swap   = str[i];
        str[i] = str[j];
        str[j] = swap;
    }
    
    return len;
}

static void dlog_lock(void)
{
    os_bool_t irq_active;
    os_bool_t irq_disabled;
    os_bool_t sched_locked;
    os_err_t  ret;

    irq_active   = os_is_irq_active();
    irq_disabled = os_is_irq_disabled();
    sched_locked = os_is_schedule_locked();

    if ((OS_TRUE == irq_active) || (OS_TRUE == irq_disabled) || (OS_TRUE == sched_locked))
    {
#ifdef DLOG_USING_ISR_LOG
        /* Interrupt context, interrupt disabled, or schedule locked */
        os_spin_lock_irqsave(&gs_dlog_ctrl_info.isr_spinlock, &gs_dlog_ctrl_info.irq_save);
#endif        
    }
    else
    {
        ret = os_mutex_lock(&gs_dlog_ctrl_info.log_locker, OS_WAIT_FOREVER);
        if (OS_EOK != ret)
        {
            OS_ASSERT_EX(0, "Get dlog locker failed!");
        }
    }

    return;
}

static void dlog_unlock(void)
{
    os_bool_t irq_active;
    os_bool_t irq_disabled;
    os_bool_t sched_locked;
    os_err_t  ret;

    irq_active   = os_is_irq_active();
    irq_disabled = os_is_irq_disabled();
    sched_locked = os_is_schedule_locked();

    if ((OS_TRUE == irq_active) || (OS_TRUE == irq_disabled) || (OS_TRUE == sched_locked))
    {
#ifdef DLOG_USING_ISR_LOG
        /* Interrupt context, interrupt disabled, or schedule locked */
        os_spin_unlock_irqrestore(&gs_dlog_ctrl_info.isr_spinlock, gs_dlog_ctrl_info.irq_save);
#endif
    }
    else
    {
        ret = os_mutex_unlock(&gs_dlog_ctrl_info.log_locker);
        if (OS_EOK != ret)
        {
            OS_ASSERT_EX(0, "Release dlog locker failed!");
        }
    }

    return;
}

static char *dlog_get_log_buf(void)
{
    os_bool_t  irq_active;
    char      *log_buff;;

    log_buff   = OS_NULL;
    irq_active = os_is_irq_active();

    /* Is in task context */
    if (OS_FALSE == irq_active)
    {
        log_buff = gs_dlog_ctrl_info.log_buf_task;
    }
    /* Is in interrupt context */
    else
    {
#ifdef DLOG_USING_ISR_LOG
        log_buff = gs_dlog_ctrl_info.log_buf_isr;
#else
        os_kprintf("Current mode not supported run in ISR. Please enable DLOG_USING_ISR_LOG.\r\n");
#endif
    }

    return log_buff;
}

#ifdef DLOG_USING_FILTER
OS_UNUSED static os_err_t dlog_tag_lvl_filter_set(const char *tag, os_uint16_t level)
{
    os_list_node_t        *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_int32_t             ret;
    os_bool_t              found;

    OS_ASSERT(OS_NULL != tag);
    OS_ASSERT(level <= DLOG_DEBUG);

    if (OS_TRUE != gs_dlog_ctrl_info.init_ok)
    {
        return OS_ERROR;
    }

    dlog_lock();

    found = OS_FALSE;
    os_list_for_each(node, &gs_dlog_ctrl_info.tag_lvl_list_head) 
    {
        tag_lvl = os_list_entry(node, dlog_tag_lvl_filter_t, list_node);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found          = OS_TRUE;
            tag_lvl->level = level;
            
            break;
        }
    }
    
    dlog_unlock();
    
    ret  = OS_EOK;
    if (!found)
    {
        /* New a tag's level filter */
        tag_lvl = (dlog_tag_lvl_filter_t *)os_malloc(sizeof(dlog_tag_lvl_filter_t));
        if (tag_lvl)
        {
            memset(tag_lvl->tag, 0, sizeof(tag_lvl->tag));
            strncpy(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN);
            tag_lvl->level = level;

            dlog_lock();
            os_list_add_tail(&gs_dlog_ctrl_info.tag_lvl_list_head, &tag_lvl->list_node);
            dlog_unlock();
        }
        else
        {
            os_kprintf("Malloc tag filter failed.\r\n");
            ret = OS_ENOMEM;
        }
    }
    
    return ret;
}

OS_UNUSED static os_err_t dlog_tag_lvl_filter_get(const char *tag, os_uint16_t *level)
{
    os_list_node_t        *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_bool_t              found;
    os_err_t               ret;

    OS_ASSERT(OS_NULL != tag);
    OS_ASSERT(OS_NULL != level);
    
    if (OS_TRUE != gs_dlog_ctrl_info.init_ok)
    {
        return OS_ERROR;
    }
    
    dlog_lock();

    found = OS_FALSE;
    os_list_for_each(node, &gs_dlog_ctrl_info.tag_lvl_list_head) 
    {
        tag_lvl = os_list_entry(node, dlog_tag_lvl_filter_t, list_node);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found  = OS_TRUE;
            *level = tag_lvl->level;
            
            break;
        }
    }
    
    dlog_unlock();

    if (found)
    {
        ret = OS_EOK;
    }
    else
    {
        ret = OS_ERROR;
    }

    return ret;
}

OS_UNUSED static os_err_t dlog_tag_lvl_filter_del(const char *tag)
{
    os_list_node_t        *node;
    dlog_tag_lvl_filter_t *tag_lvl;
    os_bool_t              found;
    os_err_t               ret;

    OS_ASSERT(OS_NULL != tag);
    
    if (OS_TRUE != gs_dlog_ctrl_info.init_ok)
    {
        return OS_ERROR;
    }
    
    dlog_lock();

    found = OS_FALSE;
    os_list_for_each(node, &gs_dlog_ctrl_info.tag_lvl_list_head) 
    {
        tag_lvl = os_list_entry(node, dlog_tag_lvl_filter_t, list_node);
        if (!strncmp(tag_lvl->tag, tag, DLOG_FILTER_TAG_MAX_LEN))
        {
            found  = OS_TRUE; 
            os_list_del(&tag_lvl->list_node);
            os_free(tag_lvl);
            break;
        }
    }
    
    dlog_unlock();

    if (found)
    {
        ret = OS_EOK;
    }
    else
    {
        ret = OS_ERROR;
    }

    return ret;

}

OS_UNUSED static void dlog_global_filter_tag_set(const char *tag)
{
    OS_ASSERT(OS_NULL != tag);

    memset(gs_dlog_ctrl_info.tag, 0, sizeof(gs_dlog_ctrl_info.tag));
    strncpy(gs_dlog_ctrl_info.tag, tag, DLOG_FILTER_TAG_MAX_LEN);
    
    return;
}

OS_UNUSED static const char *dlog_global_filter_tag_get(void)
{
    return gs_dlog_ctrl_info.tag;
}

OS_UNUSED static void dlog_global_filter_tag_del(void)
{
    memset(gs_dlog_ctrl_info.tag, 0, sizeof(gs_dlog_ctrl_info.tag));

    return;
}

OS_UNUSED static void dlog_global_filter_kw_set(const char *keyword)
{
    OS_ASSERT(OS_NULL != keyword);

    memset(gs_dlog_ctrl_info.keyword, 0, sizeof(gs_dlog_ctrl_info.keyword));
    strncpy(gs_dlog_ctrl_info.keyword, keyword, DLOG_FILTER_KW_MAX_LEN);

    return;
}

OS_UNUSED static const char *dlog_global_filter_kw_get(void)
{
    return gs_dlog_ctrl_info.keyword;
}

OS_UNUSED static void dlog_global_filter_kw_del(void)
{
    memset(gs_dlog_ctrl_info.keyword, 0, sizeof(gs_dlog_ctrl_info.keyword));

    return;
}
#endif /* DLOG_USING_FILTER */

void dlog_global_lvl_set(os_uint16_t level)
{
    OS_ASSERT(level <= DLOG_DEBUG);
    
    gs_dlog_ctrl_info.global_level = level;
    
    return;
}

os_uint16_t dlog_global_lvl_get(void)
{
    return gs_dlog_ctrl_info.global_level;
}

static os_size_t dlog_formater(char        *log_buf,
                               os_uint16_t  level,
                               const char  *tag,
                               os_bool_t    newline,
                               const char  *format,
                               va_list      args)
{
    /* The caller has locker, so it can use static variable for reduce stack usage */
    static os_size_t  s_log_len;
    static os_size_t  s_newline_len;
    static os_int32_t s_fmt_result;

    s_log_len = 0;

    if (OS_TRUE == newline)
    {
        s_newline_len = strlen(DLOG_NEWLINE_SIGN);
    }
    else
    {
        s_newline_len = 0;
    }
    
#ifdef DLOG_USING_COLOR
    /* Add CSI start sign and color info */
    if (OS_NULL != gs_color_output_info[level])
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, CSI_START);
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, gs_color_output_info[level]);
    }
#endif /* DLOG_USING_COLOR */

#ifdef DLOG_OUTPUT_TIME_INFO
    /* Add time info */
    {
#ifdef DLOG_TIME_USING_TIMESTAMP
        static time_t     s_now;
        static struct tm *s_tm;
        static struct tm  s_tm_tmp;

        s_now = time(OS_NULL);
        s_tm  = gmtime_r(&s_now, &s_tm_tmp);
        
        snprintf(log_buf + s_log_len,
                 OS_LOG_BUFF_SIZE - s_log_len,
                 "%02d-%02d %02d:%02d:%02d",
                 s_tm->tm_mon + 1,
                 s_tm->tm_mday,
                 s_tm->tm_hour,
                 s_tm->tm_min,
                 s_tm->tm_sec);
#else
        static os_size_t tick_len = 0;

        log_buf[s_log_len] = '[';
        tick_len = dlog_ultoa(log_buf + s_log_len + 1, os_tick_get());
        log_buf[s_log_len + 1 + tick_len]     = ']';
        log_buf[s_log_len + 1 + tick_len + 1] = '\0';
#endif /* DLOG_TIME_USING_TIMESTAMP */

        s_log_len += strlen(log_buf + s_log_len);
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, " ");
    }
#endif /* DLOG_OUTPUT_TIME_INFO */

    /* Add level info */
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, gs_level_output_info[level]);

    /* Add tag info */
    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, tag);

    s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, ": ");

#ifdef DLOG_OUTPUT_FLOAT
    s_fmt_result = vsnprintf(log_buf + s_log_len, OS_LOG_BUFF_SIZE - s_log_len, format, args);
#else
    s_fmt_result = os_vsnprintf(log_buf + s_log_len, OS_LOG_BUFF_SIZE - s_log_len, format, args);
#endif /* DLOG_OUTPUT_FLOAT */

    /* Calculate log length */
    if ((s_log_len + s_fmt_result <= OS_LOG_BUFF_SIZE) && (s_fmt_result >= 0))
    {
        s_log_len += s_fmt_result;
    }
    else
    {
        /* Using max length */
        s_log_len = OS_LOG_BUFF_SIZE;
    }

    /* overflow check and reserve some space for CSI end sign and newline sign */
#ifdef DLOG_USING_COLOR
    if (s_log_len + (sizeof(CSI_END) - 1) + s_newline_len >= OS_LOG_BUFF_SIZE)
    {
        /* Using max length */
        s_log_len = OS_LOG_BUFF_SIZE;

        /* Reserve some space for CSI end sign */
        s_log_len -= (sizeof(CSI_END) - 1);

        /* Reserve some space for newline sign and '\0' */
        s_log_len -= (s_newline_len + 1);
    }
#else
    if (s_log_len + s_newline_len >= OS_LOG_BUFF_SIZE)
    {
        /* Using max length */
        s_log_len = OS_LOG_BUFF_SIZE;

        /* Reserve some space for newline sign and '\0' */
        s_log_len -= (s_newline_len + 1);
    }
#endif /* DLOG_USING_COLOR */

    /* Package newline sign */
    if (newline)
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, DLOG_NEWLINE_SIGN);
    }

#ifdef DLOG_USING_COLOR
    /* Add CSI end sign  */
    if (gs_color_output_info[level])
    {
        s_log_len += dlog_strcpy(s_log_len, log_buf + s_log_len, CSI_END);
    }
#endif /* DLOG_USING_COLOR */

    log_buf[s_log_len] = '\0';
    s_log_len++;
    
    return s_log_len;
}

static void dlog_output_to_all_backend(os_uint16_t  level,
                                       os_bool_t    is_raw,
                                       char        *log,
                                       os_size_t    log_len)
{
    os_list_node_t  *node;
    dlog_backend_t  *backend;
    os_bool_t        irq_active;

    irq_active = os_is_irq_active();
 
    /* Output for all backends */
    os_list_for_each(node, &gs_dlog_ctrl_info.backend_list_head)  
    {
        backend = os_list_entry(node, dlog_backend_t, list_node);

        /* Is in interrupt */
        if (OS_TRUE == irq_active)
        {
            if (OS_FALSE == backend->support_isr)
            {
                continue;
            }
        }

#ifndef DLOG_USING_COLOR
        backend->output(backend, log, log_len);
#else
        if (backend->support_color || is_raw)
        {
            backend->output(backend, log, log_len);
        }
        else
        {
            /* Recalculate the log start address and log size when backend not supported color */
            os_size_t color_info_len;
            os_size_t output_size;

            color_info_len = strlen(gs_color_output_info[level]);
            output_size    = log_len;
            
            if (color_info_len)
            {
                os_size_t color_hdr_len;

                color_hdr_len = strlen(CSI_START) + color_info_len;
                log += color_hdr_len;

                /* "1" is '\0' */
                output_size -= (color_hdr_len + strlen(CSI_END));
            }
            
            backend->output(backend, log, output_size);
        }
#endif /* DLOG_USING_COLOR */
    }

    return;
}

static void dlog_do_output(os_uint16_t level, const char *tag, os_bool_t is_raw, char *log_buf, os_size_t log_len)
{
#ifdef DLOG_USING_ASYNC_OUTPUT
    rbb_blk_t    *log_blk;
    dlog_frame_t *log_frame;
    os_err_t      ret;

    /* allocate log frame */
    log_blk = rbb_blk_alloc(gs_dlog_ctrl_info.async_rbb, OS_ALIGN_UP(sizeof(dlog_frame_t) + log_len, OS_ALIGN_SIZE));
    if (log_blk)
    {
        /* Package the log frame */
        log_frame          = (dlog_frame_t *)log_blk->buf;
        log_frame->magic   = DLOG_FRAME_MAGIC;
        log_frame->is_raw  = is_raw;
        log_frame->level   = level;
        log_frame->log_len = log_len;
        log_frame->log     = (char *)log_blk->buf + sizeof(dlog_frame_t);

        /* Copy log data */
        memcpy(log_blk->buf + sizeof(dlog_frame_t), log_buf, log_len);
        
        /* Put the block */
        rbb_blk_put(log_blk);
        
        /* Send a notice */
        ret = os_sem_post(&gs_dlog_ctrl_info.async_notice_sem);
        if (OS_EOK != ret)
        {
            os_kprintf("Why sem post failed? \r\n");
        }
    }
    else
    {
        static os_bool_t already_output = OS_FALSE;
        
        if (already_output == OS_FALSE)
        {
            os_kprintf("Warning: There is no enough buffer for saving async log, "
                       "please increase the DLOG_ASYNC_OUTPUT_BUF_SIZE option.\n");

            already_output = OS_TRUE;
        }
    }
#else

    /* Output to all backends */
    dlog_output_to_all_backend(level, is_raw, log_buf, log_len);
        
#endif /* DLOG_USING_ASYNC_OUTPUT */

    return;
}

void dlog_voutput(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, va_list args)
{
    char        *log_buf;
    os_size_t    log_len;
    os_uint16_t  tag_level;
    os_uint16_t  global_level;
#ifdef DLOG_USING_FILTER
    os_err_t     ret;
#endif    
	
    OS_ASSERT(level <= DLOG_DEBUG);
    OS_ASSERT(OS_NULL != tag);
    OS_ASSERT(OS_NULL != format);

    do
    {
        if (OS_TRUE != gs_dlog_ctrl_info.init_ok)
        {
            break;
        }

        tag_level = OS_UINT16_MAX;
        
#ifdef DLOG_USING_FILTER
        ret = dlog_tag_lvl_filter_get(tag, &tag_level);
        if (ret != OS_EOK)
        {
            tag_level = OS_UINT16_MAX;
        }
#endif

        if (OS_UINT16_MAX == tag_level)
        {
            global_level = dlog_global_lvl_get();
            if (level > global_level)
            {
                break;
            }
        }
        else
        {
            if (level > tag_level)
            {
                break;
            }
        }

#ifdef DLOG_USING_FILTER
        if ('\0' != gs_dlog_ctrl_info.tag[0])
        {
            if (strncmp(gs_dlog_ctrl_info.tag, tag, DLOG_FILTER_TAG_MAX_LEN))
            {
                break;
            }
        }
#endif
        log_buf = dlog_get_log_buf();
        if (OS_NULL == log_buf)
        {
            break;
        }

        dlog_lock();

        log_len = dlog_formater(log_buf, level, tag, newline, format, args);

#ifdef DLOG_USING_FILTER
        /* Keyword filter */
        if ('\0' != gs_dlog_ctrl_info.keyword[0])
        {
            /* Find the keyword */
            if (!strstr(log_buf, gs_dlog_ctrl_info.keyword))
            {
                dlog_unlock();
                break;
            }
        }
#endif /* DLOG_USING_FILTER */

        /* Do log output */
        dlog_do_output(level, tag, OS_FALSE, log_buf, log_len);
        
        dlog_unlock();

    } while (0);
    
    return;
}


void dlog_output(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    dlog_voutput(level, tag, newline, format, args);
    va_end(args);

    return;
}

void dlog_raw(const char *format, ...)
{
    os_size_t   log_len;
    char       *log_buf;
    va_list     args;

    if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
    {
        log_buf = dlog_get_log_buf();
        if (OS_NULL != log_buf)
        {
            dlog_lock();

            va_start(args, format);
#ifdef DLOG_OUTPUT_FLOAT
            /* 
             * When log size is larger than buffer size, this function will truncature
             * and fill '\0' at the buffer tail. The return len is log size without '\0'
             */
            log_len = vsnprintf(log_buf, OS_LOG_BUFF_SIZE, format, args);
#else
            log_len = os_vsnprintf(log_buf, OS_LOG_BUFF_SIZE, format, args);
#endif /* DLOG_OUTPUT_FLOAT */
            va_end(args);

            /* Calculate log length */
            if (log_len > OS_LOG_BUFF_SIZE - 1)
            {
                log_len = OS_LOG_BUFF_SIZE - 1;
            }

            log_buf[log_len] = '\0';
            log_len++;

            dlog_do_output(DLOG_DEBUG, OS_NULL, OS_TRUE, log_buf, log_len);

            dlog_unlock();
        }
    }

    return;    
}



static void dlog_do_hexdump(const char *tag,
                            os_size_t   width,
                            os_uint8_t *data_buf,
                            os_size_t   data_buf_size,
                            char       *log_buf,
                            os_size_t   log_buf_size)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127U - ' ')
    
    os_size_t  i;
    os_size_t  j;
    os_size_t  log_len;
    os_size_t  name_len;
    char       dump_string[8];
    os_int32_t fmt_result;
 
    name_len = strlen(tag);
    
    dlog_lock();

    log_len = 0;
    for (i = 0; i < data_buf_size; i += width)
    {
        /* Package header */
        if (i == 0)
        {
            log_len += dlog_strcpy(log_len, log_buf + log_len, "D/HEX ");
            log_len += dlog_strcpy(log_len, log_buf + log_len, tag);
            log_len += dlog_strcpy(log_len, log_buf + log_len, ": ");
        }
        else
        {
            log_len = 6 + name_len + 2;
            memset(log_buf, ' ', log_len);
        }
       
        fmt_result = os_snprintf(log_buf + log_len, log_buf_size - log_len, "%04X-%04X: ",
                                 (os_uint16_t)i,
                                 (os_uint16_t)(i + width));

        /* Calculate log length */
        if (fmt_result < log_buf_size - log_len)
        {
            log_len += fmt_result;
        }
        else
        {
            log_len = log_buf_size;
        }
       
        /* Dump hex */
        for (j = 0; j < width; j++)
        {
            if (i + j < data_buf_size)
            {
                os_snprintf(dump_string, sizeof(dump_string), "%02X ", data_buf[i + j]);
            }
            else
            {
                strncpy(dump_string, "   ", sizeof(dump_string));
            }
           
            log_len += dlog_strcpy(log_len, log_buf + log_len, dump_string);
            if ((j + 1) % 8 == 0)
            {
                log_len += dlog_strcpy(log_len, log_buf + log_len, " ");
            }
        }
       
        log_len += dlog_strcpy(log_len, log_buf + log_len, "  ");

        /* Dump char for hex */
        for (j = 0; j < width; j++)
        {
            if (i + j < data_buf_size)
            {
                os_snprintf(dump_string, sizeof(dump_string), "%c", __is_print(data_buf[i + j]) ? data_buf[i + j] : '.');
                log_len += dlog_strcpy(log_len, log_buf + log_len, dump_string);
            }
        }
       
        /* Overflow check and reserve some space for newline sign */
        if (log_len > log_buf_size - strlen(DLOG_NEWLINE_SIGN) - 1)
        {
            log_len = log_buf_size - strlen(DLOG_NEWLINE_SIGN) - 1;
        }
       
        /* Package newline sign */
        log_len += dlog_strcpy(log_len, log_buf + log_len, DLOG_NEWLINE_SIGN);
        log_buf[log_len] = '\0';
        log_len++;
       
        /* Do log output */
        dlog_do_output(DLOG_DEBUG, OS_NULL, OS_TRUE, log_buf, log_len);
    }
       
    dlog_unlock();

    return;
}

void dlog_hexdump(const char *tag, os_size_t width, os_uint8_t *buf, os_size_t size)
{
    char *log_buf;

    OS_ASSERT(OS_NULL != tag);
    OS_ASSERT(OS_NULL != buf);
    OS_ASSERT(size > 0LU);
    OS_ASSERT(width > 0LU);

    if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
    {
        log_buf = dlog_get_log_buf();
        if (OS_NULL != log_buf)
        {
            dlog_do_hexdump(tag, width, buf, size, log_buf, OS_LOG_BUFF_SIZE);
        }
    }

    return;
}

#ifdef DLOG_USING_ASYNC_OUTPUT
static void dlog_async_output_task_entry(void *arg)
{
    rbb_blk_t    *log_blk;
    dlog_frame_t *log_frame;
    os_err_t      ret;

    while (1)
    {
        ret = os_sem_wait(&gs_dlog_ctrl_info.async_notice_sem, OS_WAIT_FOREVER);
        if (OS_EOK != ret)
        {
            OS_ASSERT_EX(0, "Why sem wait failed?");
        }
        
        while (1)
        {   
            log_blk = rbb_blk_get(gs_dlog_ctrl_info.async_rbb);
            if (OS_NULL == log_blk)
            {
                break;
            }

            log_frame = (dlog_frame_t *)log_blk->buf;
            if (DLOG_FRAME_MAGIC == log_frame->magic)
            {
                /* Output to all backends */
                dlog_output_to_all_backend(log_frame->level, log_frame->is_raw, log_frame->log, log_frame->log_len);
            }
            
            rbb_blk_free(gs_dlog_ctrl_info.async_rbb, log_blk);
        }
    }
}

OS_UNUSED static void dlog_async_output(void)
{
    rbb_blk_t    *log_blk;
    dlog_frame_t *log_frame;

    log_blk = rbb_blk_get(gs_dlog_ctrl_info.async_rbb);
    while (OS_NULL != log_blk)
    {
        log_frame = (dlog_frame_t *)log_blk->buf;
        if (DLOG_FRAME_MAGIC == log_frame->magic)
        {
            dlog_output_to_all_backend(log_frame->level, log_frame->is_raw, log_frame->log, log_frame->log_len);
        }
        
        rbb_blk_free(gs_dlog_ctrl_info.async_rbb, log_blk);
        
        log_blk = rbb_blk_get(gs_dlog_ctrl_info.async_rbb);
    }

    return;
}
#endif /* DLOG_USING_ASYNC_OUTPUT */

void dlog_flush(void)
{
    os_list_node_t *node;
    dlog_backend_t *backend;

    dlog_lock();

    if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
    {
#ifdef DLOG_USING_ASYNC_OUTPUT
        dlog_async_output();
#endif

        /* Flush all backends */
        os_list_for_each(node, &gs_dlog_ctrl_info.backend_list_head)
        {
            backend = os_list_entry(node, dlog_backend_t, list_node);
            if (backend->flush)
            {
                backend->flush(backend);
            }
        }
    }

    dlog_unlock();
    
    return;
}

os_err_t dlog_backend_register(dlog_backend_t *backend)
{
    dlog_backend_t *backend_iter;
    os_list_node_t *node;
    os_bool_t       found;
    os_err_t        ret;

    OS_ASSERT(OS_NULL != backend);
    OS_ASSERT(OS_NULL != backend->output);

    found = OS_FALSE;
    ret   = OS_EOK;

    if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
    {
        dlog_lock();
        os_list_for_each(node, &gs_dlog_ctrl_info.backend_list_head) 
        {
            backend_iter = os_list_entry(node, dlog_backend_t, list_node);
            if (backend == backend_iter)
            {
                found = OS_TRUE;
                break;
            } 
        }
        dlog_unlock();

        if (OS_FALSE == found)
        {
            if (OS_NULL != backend->init)
            {
                backend->init(backend);
            }

            dlog_lock();
            os_list_add_tail(&gs_dlog_ctrl_info.backend_list_head, &backend->list_node);
            dlog_unlock();
        }
    }
    else
    {
        ret = OS_EPERM;
    }
    
    return ret;
}

os_err_t dlog_backend_unregister(dlog_backend_t *backend)
{
    dlog_backend_t *backend_iter;
    os_list_node_t *node;
    os_bool_t       found;
    os_err_t        ret;

    OS_ASSERT(OS_NULL != backend);

    found = OS_FALSE;
    ret   = OS_EOK;

    if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
    {
        dlog_lock();
        os_list_for_each(node, &gs_dlog_ctrl_info.backend_list_head) 
        {
            backend_iter = os_list_entry(node, dlog_backend_t, list_node);
            if (backend == backend_iter)
            {
                found = OS_TRUE;
                os_list_del(&backend->list_node);
                
                break;
            } 
        }
        dlog_unlock();

        if ((OS_TRUE == found) && (OS_NULL != backend->deinit))
        {
            backend->deinit(backend);
        }
    }
    else
    {
        ret = OS_EPERM;
    }

    return ret;
}

os_err_t dlog_init(void)
{
    os_err_t ret;

    ret = OS_EOK;

    do
    {
        if (OS_TRUE == gs_dlog_ctrl_info.init_ok)
        {
            break;
        }

        ret = os_mutex_init(&gs_dlog_ctrl_info.log_locker, "dlog_locker", OS_FALSE);
        OS_ASSERT(OS_EOK == ret);
        
        os_list_init(&gs_dlog_ctrl_info.backend_list_head);

#ifdef DLOG_USING_FILTER
        os_list_init(&gs_dlog_ctrl_info.tag_lvl_list_head);
#endif

#ifdef DLOG_USING_ISR_LOG
        os_spin_lock_init(&gs_dlog_ctrl_info.isr_spinlock);
#endif

#ifdef DLOG_USING_ASYNC_OUTPUT        
        OS_ASSERT_EX(DLOG_ASYNC_OUTPUT_STORE_LINES >= 2, "Asynchronous output buffer(%u) is too small.",
                     DLOG_ASYNC_OUTPUT_BUF_SIZE);

        /* Async output ring block buffer */
        gs_dlog_ctrl_info.async_rbb = rbb_create(OS_ALIGN_UP(DLOG_ASYNC_OUTPUT_BUF_SIZE, OS_ALIGN_SIZE),
                                                 DLOG_ASYNC_OUTPUT_STORE_LINES);
        if (OS_NULL == gs_dlog_ctrl_info.async_rbb)
        {
            os_kprintf("Dlog init failed! No memory for async rbb.\r\n");
            
            (void)os_mutex_deinit(&gs_dlog_ctrl_info.log_locker);
            ret = OS_ENOMEM;

            break;
        }

        /* Async output task */
        gs_dlog_ctrl_info.async_task = os_task_create("dlog_async",
                                                      dlog_async_output_task_entry,
                                                      &gs_dlog_ctrl_info,
                                                      DLOG_ASYNC_OUTPUT_TASK_STACK_SIZE,
                                                      DLOG_ASYNC_OUTPUT_TASK_PRIORITY);
        if (OS_NULL == gs_dlog_ctrl_info.async_task)
        {
            os_kprintf("Dlog init failed! Create async output task failed.\r\n");

            rbb_destroy(gs_dlog_ctrl_info.async_rbb);
            
            ret = os_mutex_deinit(&gs_dlog_ctrl_info.log_locker);
            OS_ASSERT(OS_EOK == ret);
            
            ret = OS_ERROR;
            break;
        }

        ret = os_sem_init(&gs_dlog_ctrl_info.async_notice_sem, "dlog_sem", 0, OS_SEM_MAX_VALUE);
        OS_ASSERT(OS_EOK == ret);
        
        ret = os_task_startup(gs_dlog_ctrl_info.async_task);
        OS_ASSERT(OS_EOK == ret);

#endif /* DLOG_USING_ASYNC_OUTPUT */

        dlog_global_lvl_set(DLOG_GLOBAL_PRINT_LEVEL);
        gs_dlog_ctrl_info.init_ok = OS_TRUE;

        break;
    } while (0);
    
    return ret;
}
OS_PREV_INIT(dlog_init, OS_INIT_SUBLEVEL_HIGH);

#ifdef OS_USING_SHELL
#include <shell.h>
#include <stdlib.h>
#include <option_parse.h>

#define  SH_DLOG_LEVEL_INVALID      0xFFFFU

struct dlog_cmd_ctrl_info
{
    os_int32_t              ctrl_info;
    
#ifdef DLOG_USING_FILTER
    char                    tag_name[DLOG_FILTER_TAG_MAX_LEN + 1];
    char                    keyword [DLOG_FILTER_KW_MAX_LEN + 1];
#endif
    
    os_uint16_t             level;
};
typedef struct dlog_cmd_ctrl_info dlog_cmd_ctrl_info_t;

static dlog_cmd_ctrl_info_t     gs_dlog_cmd_ctrl_info = {0};

static char *gs_dlog_level_info[8] =
{
    "emerg",
    "alert",
    "crit",
    "error",
    "warning",
    "notice",
    "info",
    "debug" 
};

#define COMMAND_SET_OPTION      1
#define COMMAND_GET_OPTION      2
#define COMMAND_DEL_OPTION      3

static os_err_t sh_dlog_ctrl_info_get(os_int32_t argc, char * const *argv, dlog_cmd_ctrl_info_t *ctrl_info)
{
    opt_state_t state;
    os_int32_t  opt_ret;
    os_int32_t  ret;

    memset(ctrl_info, 0 , sizeof(dlog_cmd_ctrl_info_t));
    ctrl_info->level = SH_DLOG_LEVEL_INVALID;
    
    memset(&state, 0, sizeof(state));
    opt_init(&state, 1);

    ret = OS_EOK;
    while (1)
    {
        opt_ret = opt_get(argc, argv, "t:sgdl:k:", &state);
        if (opt_ret == OPT_EOF)
        {
            break;
        }

        if ((opt_ret == OPT_BADOPT) || (opt_ret == OPT_BADARG))
        {
            ret = OS_ERROR;
            break;
        }
    
        switch (opt_ret)
        {
        case 's':
            ctrl_info->ctrl_info = COMMAND_SET_OPTION;
            break;

        case 'g':
            ctrl_info->ctrl_info = COMMAND_GET_OPTION;
            break;
            
        case 'd':
            ctrl_info->ctrl_info = COMMAND_DEL_OPTION;
            break;
            
        case 'l':
            ctrl_info->level = (os_uint16_t)atoi(state.opt_arg);
            break;
            
#ifdef DLOG_USING_FILTER
        case 't':
            memset(ctrl_info->tag_name, 0, sizeof(ctrl_info->tag_name));
            strncpy(ctrl_info->tag_name, state.opt_arg, DLOG_FILTER_TAG_MAX_LEN);
            break;

        case 'k':
            memset(ctrl_info->keyword, 0, sizeof(ctrl_info->keyword));
            strncpy(ctrl_info->keyword, state.opt_arg, DLOG_FILTER_KW_MAX_LEN);
            break;
#endif            

        default:
            os_kprintf("Invalid option: %c\r\n", (char)opt_ret);

            ret = OS_EINVAL;
            break;
        }

        if (ret != OS_EOK)
        {
            break;
        }
    }

    return ret;
}

static void sh_dlog_glvl_ctrl_help(void)
{
    os_kprintf("Command format:\r\n");
    os_kprintf("dlog_glvl_ctrl <-s | -g> [-l global level]\r\n");
    os_kprintf("parameter Usage:\r\n");

    os_kprintf("         -s     Set global level option.\r\n");
    os_kprintf("         -g     Get global level option.\r\n");
    os_kprintf("         -l     Specify a global level that want to be set.\r\n");
    os_kprintf("                level: 0-emerg, 1-alert, 2-crit, 3-error, 4-warning, 5-notice, 6-info, 7-debug\r\n");
    
    return;
}

static os_err_t sh_dlog_glvl_ctrl(os_int32_t argc, char **argv)
{
    os_err_t    ret;
    os_uint16_t level;

    ret = sh_dlog_ctrl_info_get(argc, argv, &gs_dlog_cmd_ctrl_info);
    if (OS_EOK == ret)
    {
        if (COMMAND_GET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            level = dlog_global_lvl_get();
            
            os_kprintf("Global level is: %s\r\n", gs_dlog_level_info[level]);        
        }
        else if (COMMAND_SET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            if ((gs_dlog_cmd_ctrl_info.level > DLOG_DEBUG) && (gs_dlog_cmd_ctrl_info.level < SH_DLOG_LEVEL_INVALID))
            {
                ret = OS_EINVAL;
                os_kprintf("Invalid global level(%u)\r\n", gs_dlog_cmd_ctrl_info.level);
            }
            else if (SH_DLOG_LEVEL_INVALID == gs_dlog_cmd_ctrl_info.level)
            {
                ret = OS_EINVAL;
                os_kprintf("No global level option\r\n");
            }
            else
            {
                 (void)dlog_global_lvl_set(gs_dlog_cmd_ctrl_info.level);
                 os_kprintf("Set global level(%s) success!\r\n", gs_dlog_level_info[gs_dlog_cmd_ctrl_info.level]);
            }
        }
        else
        {
            ret = OS_EINVAL;
            os_kprintf("No command option, please input -s or -g.\r\n");
        }
    }
    else
    {
        os_kprintf("Get option failed!\r\n");
    }

    if (OS_EOK != ret)
    {
        sh_dlog_glvl_ctrl_help();
    }

    return ret; 

}
SH_CMD_EXPORT(dlog_glvl_ctrl, sh_dlog_glvl_ctrl, "Dlog global level control");


#ifdef DLOG_USING_FILTER
static void sh_dlog_tlvl_ctrl_help(void)
{
    os_kprintf("Command format:\r\n");
    os_kprintf("dlog_tlvl_ctrl <-s | -g | -d> <-t tag name> [-l tag level]\r\n");
    os_kprintf("parameter Usage:\r\n");
    
    os_kprintf("         -s     Set tag level option.\r\n");
    os_kprintf("         -g     Get tag level option.\r\n");
    os_kprintf("         -d     Delete tag level option.\r\n");
    os_kprintf("         -t     Specify the tag name that want set level.\r\n");
    os_kprintf("         -l     Specify a tag level that want to be set.\r\n");
    os_kprintf("                level: 0-emerg, 1-alert, 2-crit, 3-error, 4-warning, 5-notice, 6-info, 7-debug\r\n");

    return;
}

static os_err_t sh_do_dlog_tlvl_ctrl(os_int32_t ctrl_opt, char *tag_name, os_uint16_t set_level)
{
    os_uint16_t get_level;
    os_err_t    ret;

    ret = OS_EOK;

    if (COMMAND_GET_OPTION == ctrl_opt)
    {
        ret = dlog_tag_lvl_filter_get(tag_name, &get_level);
        if (OS_EOK != ret)
        {
            os_kprintf("Not found tag(%s), get tag level failed.\r\n", tag_name);
        }
        else
        {
            os_kprintf("The tag(%s) level is: %s.\r\n", tag_name, gs_dlog_level_info[get_level]);    
        }
    }
    else if (COMMAND_SET_OPTION == ctrl_opt)
    {
        if ((set_level > DLOG_DEBUG) && (set_level < SH_DLOG_LEVEL_INVALID))
        {
            ret = OS_EINVAL;
            os_kprintf("Invalid tag level(%u)\r\n", set_level);
        }
        else if (SH_DLOG_LEVEL_INVALID == set_level)
        {
            ret = OS_EINVAL;
            os_kprintf("No tag level option\r\n");
        }
        else
        {
            ret = dlog_tag_lvl_filter_set(tag_name, set_level);
            if (OS_EOK != ret)
            {
                os_kprintf("Set tag failed, tag: %s, level: %s\r\n", tag_name, gs_dlog_level_info[set_level]);
            }
            else
            {
                os_kprintf("Set tag success, tag: %s, level: %s\r\n", tag_name, gs_dlog_level_info[set_level]);
            } 
        }
    }
    else if (COMMAND_DEL_OPTION == ctrl_opt)
    {
        ret = dlog_tag_lvl_filter_del(tag_name);
        if (OS_EOK == ret)
        {
            os_kprintf("Del tag(%s) success.\r\n", tag_name);
        }
        else
        {
            os_kprintf("Del tag(%s) failed.\r\n", tag_name);
        }
    }
    else
    {
        ret = OS_EINVAL;
        os_kprintf("Invalid ctrl_opt(%u)\r\n", ctrl_opt);
    }

    return ret;
}

static os_err_t sh_dlog_tlvl_ctrl(os_int32_t argc, char **argv)
{
    os_bool_t cmd_option_valid;
    os_bool_t tag_name_valid;
    os_err_t  ret;

    cmd_option_valid = OS_TRUE;
    tag_name_valid   = OS_TRUE;

    ret = sh_dlog_ctrl_info_get(argc, argv, &gs_dlog_cmd_ctrl_info);
    if (OS_EOK == ret)
    {
        if ((COMMAND_GET_OPTION != gs_dlog_cmd_ctrl_info.ctrl_info)
            && (COMMAND_SET_OPTION != gs_dlog_cmd_ctrl_info.ctrl_info)
            && (COMMAND_DEL_OPTION != gs_dlog_cmd_ctrl_info.ctrl_info))
        {
            os_kprintf("No command option, please input -s, -g or -d!\r\n");
            cmd_option_valid = OS_FALSE;  
        }

        if (0U == strlen(gs_dlog_cmd_ctrl_info.tag_name))
        {
            os_kprintf("No tag name option!\r\n");
            tag_name_valid = OS_FALSE;    
        }  

        if ((OS_TRUE == cmd_option_valid) && (OS_TRUE == tag_name_valid))
        {
            ret = sh_do_dlog_tlvl_ctrl(gs_dlog_cmd_ctrl_info.ctrl_info,
                                       gs_dlog_cmd_ctrl_info.tag_name,
                                       gs_dlog_cmd_ctrl_info.level);   
        }
        else
        {
            ret = OS_EINVAL;
        }

    }
    else
    {
        os_kprintf("Get option failed!\r\n");
    }

    if (OS_EOK != ret)
    {
        sh_dlog_tlvl_ctrl_help();
    }

    return ret;
}
SH_CMD_EXPORT(dlog_tlvl_ctrl, sh_dlog_tlvl_ctrl, "Dlog tag level control");

static void sh_dlog_gtag_ctrl_help(void)
{
    os_kprintf("Command format:\r\n");
    os_kprintf("dlog_gtag_ctrl <-s | -g | -d> [-t tag name]\r\n");
    os_kprintf("parameter Usage:\r\n");
    os_kprintf("         -s     Set global tag option.\r\n");
    os_kprintf("         -g     Get global tag option.\r\n");
    os_kprintf("         -d     Delete global tag option.\r\n");
    os_kprintf("         -t     Specify the global tag name.\r\n");

    return;
}

static os_err_t sh_dlog_gtag_ctrl(os_int32_t argc, char **argv)
{
    const char *tag;
    os_err_t    ret;

    ret = sh_dlog_ctrl_info_get(argc, argv, &gs_dlog_cmd_ctrl_info);
    if (OS_EOK == ret)
    {
        if (COMMAND_GET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            tag = dlog_global_filter_tag_get();

            if (0U == strlen(tag))
            {
                os_kprintf("No global filter tag\r\n");
            }
            else
            {
                os_kprintf("The global filter tag is %s\r\n", tag);
            }
        }
        else if (COMMAND_SET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            if (0U == strlen(gs_dlog_cmd_ctrl_info.tag_name))
            {
                os_kprintf("No tag name option.\r\n");
                ret = OS_EINVAL;
            }
            else
            {
                dlog_global_filter_tag_set(gs_dlog_cmd_ctrl_info.tag_name);
                os_kprintf("Set global filter tag(%s) success\r\n", gs_dlog_cmd_ctrl_info.tag_name);
            }
        }
        else if (COMMAND_DEL_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            dlog_global_filter_tag_del();
            os_kprintf("Del global filter tag success.\r\n");   
        }
        else
        {
            os_kprintf("Invalid command option, please input -s, -g or -d!\r\n");
            ret = OS_EINVAL;
        }
    }
    else
    {
        os_kprintf("Get option failed!\r\n");
    }

    if (OS_EOK != ret)
    {
        sh_dlog_gtag_ctrl_help();
    }

    return ret;
}
SH_CMD_EXPORT(dlog_gtag_ctrl, sh_dlog_gtag_ctrl, "Dlog global tag control");

static void sh_dlog_gkw_ctrl_help(void)
{
    os_kprintf("Command format:\r\n");
    os_kprintf("dlog_gkw_ctrl <-s | -g | -d> [-k keyword]\r\n");
    os_kprintf("parameter Usage:\r\n");

    os_kprintf("         -s     Set global keyword option.\r\n");
    os_kprintf("         -g     Get global keyword option.\r\n");
    os_kprintf("         -d     Delete global keyword option.\r\n");
    os_kprintf("         -k     Specify the gobal keyword.\r\n");

    return;
}

static os_err_t sh_dlog_gkw_ctrl(os_int32_t argc, char **argv)
{
    const char *keyword;
    os_err_t    ret;

    ret = sh_dlog_ctrl_info_get(argc, argv, &gs_dlog_cmd_ctrl_info);
    if (OS_EOK == ret)
    {
        if (COMMAND_GET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            keyword = dlog_global_filter_kw_get();

            if (0U == strlen(keyword))
            {
                os_kprintf("No global filter keyword\r\n");
            }
            else
            {
                os_kprintf("The global filter keyword is %s\r\n", keyword);
            }
        }
        else if (COMMAND_SET_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            if (0U == strlen(gs_dlog_cmd_ctrl_info.keyword))
            {
                os_kprintf("No keyword option.\r\n");
                ret = OS_EINVAL;
            }
            else
            {
                dlog_global_filter_kw_set(gs_dlog_cmd_ctrl_info.keyword);
                os_kprintf("Set global filter keyword(%s) success\r\n", gs_dlog_cmd_ctrl_info.keyword);
            }
        }
        else if (COMMAND_DEL_OPTION == gs_dlog_cmd_ctrl_info.ctrl_info)
        {
            dlog_global_filter_kw_del();
            os_kprintf("Del global filter keyword success.\r\n");   
        }
        else
        {
            os_kprintf("Invalid command option, please input -s, -g or -d!\r\n");
            ret = OS_EINVAL;
        }
    }
    else
    {
        os_kprintf("Get option failed!\r\n");
    }

    if (OS_EOK != ret)
    {
        sh_dlog_gkw_ctrl_help();
    }

    return ret;
}
SH_CMD_EXPORT(dlog_gkw_ctrl, sh_dlog_gkw_ctrl, "Dlog global keyword control");
#endif /* DLOG_USING_FILTER */

static os_err_t sh_dlog_flush(os_int32_t argc, char **argv)
{
    dlog_flush();

    os_kprintf("Flush dlog cache success!\r\n");

    return OS_EOK;
}
SH_CMD_EXPORT(dlog_flush, sh_dlog_flush, "Flush dlog cache");
#endif /* OS_USING_SHELL */

#endif /* OS_USING_DLOG */

