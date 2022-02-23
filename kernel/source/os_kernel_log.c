/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        os_kernel_log.c
 *
 * @brief       Implementation of kernel printing function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-05   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_list.h>
#include <os_clock.h>
#include <string.h>
#include <board.h>
#include <os_util.h>
#include <os_assert.h>


#ifdef OS_USING_KERNEL_DEBUG

#define KLOG_NEWLINE_SIGN               "\r\n"

#ifdef KLOG_USING_COLOR
/*
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
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

#define KLOG_COLOR_DEBUG                (OS_NULL)
#define KLOG_COLOR_INFO                 (F_GREEN)
#define KLOG_COLOR_WARN                 (F_YELLOW)
#define KLOG_COLOR_ERROR                (F_RED)

/* Color output info */
static const char *gs_klog_color_output_info[] =
{
    KLOG_COLOR_ERROR,
    KLOG_COLOR_WARN,
    KLOG_COLOR_INFO,
    KLOG_COLOR_DEBUG,
};
#endif /* KLOG_USING_COLOR */

#define KLOG_FILTER_TAG_MAX_LEN         15

struct os_klog_tag_lvl_filter
{
    os_list_node_t list;
    char           tag[KLOG_FILTER_TAG_MAX_LEN + 1];
    os_uint16_t    level; 
};
typedef struct os_klog_tag_lvl_filter os_klog_tag_lvl_filter_t;

struct os_klog_ctrl_info
{
    os_list_node_t tag_lvl_list;
    os_uint16_t    global_level;
    char           log_buff[OS_LOG_BUFF_SIZE];
};
typedef struct os_klog_ctrl_info os_klog_ctrl_info_t;

static os_klog_ctrl_info_t gs_klog_ctrl_info = 
{
    .tag_lvl_list = OS_LIST_INIT(gs_klog_ctrl_info.tag_lvl_list),
    .global_level = KLOG_GLOBAL_LEVEL
};

const os_int16_t     *g_klog_global_lvl   = (os_int16_t *)&gs_klog_ctrl_info.global_level; 
const os_list_node_t *g_klog_tag_lvl_list = &gs_klog_ctrl_info.tag_lvl_list;

static const char    *gs_klog_level_output_info[] =
{
    "E/",
    "W/",
    "I/",
    "D/"
};

static os_bool_t os_get_klog_tag_lvl(const char *tag, os_uint16_t *level)
{
    os_list_node_t           *node;
    os_klog_tag_lvl_filter_t *tag_lvl_filter;
    os_bool_t                found;
    os_int32_t               ret;
    
    OS_ASSERT(tag);
    OS_ASSERT(level);

    //os_enter_critical();

    found = OS_FALSE;
    os_list_for_each(node, &gs_klog_ctrl_info.tag_lvl_list)
    {
        tag_lvl_filter = os_list_entry(node, os_klog_tag_lvl_filter_t, list);

        ret = strncmp(tag_lvl_filter->tag, tag, KLOG_FILTER_TAG_MAX_LEN);
        if (0 == ret)
        {
            found  = OS_TRUE;
            *level = tag_lvl_filter->level;
            
            break;
        }
    }
    
    //os_exit_critical();
    
    return found;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will print kernel log on system console.
 *
 * @attention       This function can only be used in the kernel.
 *
 * @param[in]       level           The Log level.
 * @param[in]       tag             The log tag.
 * @param[in]       newline         Has newline.
 * @param[in]       fmt             Output format.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_kernel_print(os_uint16_t level, const char *tag, os_bool_t newline, const char *fmt, ...)
{
    va_list            args;
    static os_int32_t  s_cnt;
    static os_int32_t  s_offset;
    static os_bool_t   s_found;
    static os_uint16_t s_tag_level;
    static os_int32_t  s_newline_len;

    OS_ASSERT(OS_NULL != tag);
    
    if (os_list_empty(&gs_klog_ctrl_info.tag_lvl_list))
    {
        if (level > gs_klog_ctrl_info.global_level)
        {
            return;
        }
    }
    else
    {
        s_found = os_get_klog_tag_lvl(tag, &s_tag_level);
        if (s_found)
        {
            if (level > s_tag_level)
            {
                return;
            }
        }
        else
        {
            if (level > gs_klog_ctrl_info.global_level)
            {
                return;
            }
        }
    }

    if (newline)
    {
        s_newline_len = strlen(KLOG_NEWLINE_SIGN);
    }
    else
    {
        s_newline_len = 0;
    }
    
    s_offset = 0;
    (void)memset(gs_klog_ctrl_info.log_buff, 0, OS_LOG_BUFF_SIZE);

#ifdef KLOG_USING_COLOR
    if (gs_klog_color_output_info[level])
    {
        s_cnt = os_snprintf(gs_klog_ctrl_info.log_buff + s_offset,
                            OS_LOG_BUFF_SIZE - s_offset,
                            "%s%s",
                            CSI_START,
                            gs_klog_color_output_info[level]);
    
        s_offset += s_cnt;
    }
#endif /* KLOG_USING_COLOR */

    s_cnt = os_snprintf(gs_klog_ctrl_info.log_buff + s_offset,
                        OS_LOG_BUFF_SIZE - s_offset,
                        "[%lu] %s%s: ",
                        os_tick_get(),
                        gs_klog_level_output_info[level],
                        tag);
    if (s_cnt < 0)
    {
        OS_ASSERT(0);
    }

    va_start(args, fmt);
    s_offset += s_cnt;
    s_cnt = os_vsnprintf(gs_klog_ctrl_info.log_buff + s_offset, OS_LOG_BUFF_SIZE - s_offset, fmt, args);
    va_end(args);
    
    if (s_cnt < 0)
    {
        OS_ASSERT(s_cnt >= 0);
        return;
    }
    
    s_offset += s_cnt;

#ifdef KLOG_USING_COLOR
    if (s_offset + s_newline_len + strlen(CSI_END) + 1 > OS_LOG_BUFF_SIZE)
    {
        s_offset = OS_LOG_BUFF_SIZE;
        s_offset -= ( s_newline_len + strlen(CSI_END));
        s_offset -= 1;
    }
#else
    if (s_offset + s_newline_len + 1 > OS_LOG_BUFF_SIZE)
    {
        s_offset = OS_LOG_BUFF_SIZE;
        s_offset -= s_newline_len;
        s_offset -= 1;
    }
#endif

    if (s_newline_len)
    {
        (void)strcpy(gs_klog_ctrl_info.log_buff + s_offset, KLOG_NEWLINE_SIGN);
        s_offset += s_newline_len;
    }

#ifdef KLOG_USING_COLOR
    if (gs_klog_color_output_info[level])
    {
        (void)strcpy(gs_klog_ctrl_info.log_buff + s_offset, CSI_END);
        s_offset += strlen(CSI_END);
    }
#endif /* KLOG_USING_COLOR */

    gs_klog_ctrl_info.log_buff[s_offset] = '\0';

    os_hw_console_output(gs_klog_ctrl_info.log_buff);
    
    return;
}

#endif /* OS_USING_KERNEL_DEBUG */

