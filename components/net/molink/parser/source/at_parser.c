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
 * @file        at_parser.c
 *
 * @brief       Implement AT Parser
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "at_parser.h"
#include "at_printf.h"

#ifdef MOLINK_USING_PARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_memory.h>
#include <os_stddef.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <os_event.h>

#define MO_LOG_TAG "at.parser"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define AT_PARSER_PROTECT               os_schedule_lock()
#define AT_PARSER_UNPROTECT             os_schedule_unlock()

#define AT_PARSER_RX_ARRIVE            (1UL)

/* AT Parser manage list */
static os_slist_node_t gs_parser_list = {0};

/* AT Parser send buffer */


static void at_parser_list_add(at_parser_t *parser)
{
    OS_ASSERT(parser != OS_NULL);

    AT_PARSER_PROTECT;

    os_slist_init(&(parser->list));

    /* tail insertion */
    os_slist_add_tail(&(gs_parser_list), &(parser->list));

    AT_PARSER_UNPROTECT;
}

static void at_parser_list_del(at_parser_t *parser)
{
    OS_ASSERT(parser != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    at_parser_t     *entry = OS_NULL;

    AT_PARSER_PROTECT;

    for (node = &gs_parser_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, at_parser_t, list);
        if (entry == parser)
        {
            os_slist_del(&(gs_parser_list), &(parser->list));

            break;
        }
    }

    AT_PARSER_UNPROTECT;
}

/**
 ***********************************************************************************************************************
 * @brief           dump hex format data to console device
 *
 * @param[in]       name            Name for hex object, it will show on log header
 * @param[in]       buf             Hex buffer
 * @param[in]       size            buffer size
 ***********************************************************************************************************************
 */
void at_parser_print_raw(const char *name, const char *buf, os_size_t size)
{
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
#define WIDTH_SIZE     32

    for (int i = 0; i < size; i += WIDTH_SIZE)
    {
        os_kprintf("[D/at.parser] %s: %04X-%04X: ", name, i, i + WIDTH_SIZE);
        for (int j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                os_kprintf("%02X ", buf[i + j]);
            }
            else
            {
                os_kprintf("   ");
            }
            if ((j + 1) % 8 == 0)
            {
                os_kprintf(" ");
            }
        }
        os_kprintf("  ");
        for (int j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                os_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        os_kprintf("\r\n");
    }
}

static os_size_t at_parser_vasprintf(at_parser_t *parser, const char *format, va_list args)
{
    os_size_t sent_size = 0;

    /* Gets the actual length of the source string */
    os_size_t len = at_vsnprintf(OS_NULL, 0, format, args);
    if (len <= 0)
    {
        return 0;
    }

    parser->at_cmd_buff = os_calloc(1, len + 1);
    if (OS_NULL == parser->at_cmd_buff)
    {
        ERROR("Send AT command failed. Unable to calloc memory.");
        return 0;
    }

    len = at_vsnprintf(parser->at_cmd_buff, len + 1, format, args);

#ifdef AT_PARSER_PRINT_RAW
    at_parser_print_raw("send", parser->at_cmd_buff, len);
#endif

    while (sent_size < len)
    {
        sent_size += os_device_write_block(parser->device, 0, parser->at_cmd_buff + sent_size, len - sent_size);
    }

    return sent_size;
}

static os_size_t at_parser_vasprintfln(at_parser_t *parser, const char *format, va_list args)
{
    os_size_t len = at_parser_vasprintf(parser, format, args);

    os_device_write_block(parser->device, 0, "\r\n", 2);

    return len + 2;
}

/**
 ***********************************************************************************************************************
 * @brief           Lock the recursive lock that protects the execution of the AT command process.
 * @param[in]       parser          A pointer to AT Parser instance
 *
 * @return          @see os_mutex_recursive_lock.
 ***********************************************************************************************************************
 */
os_err_t at_parser_exec_lock(at_parser_t *parser)
{
    return os_mutex_recursive_lock(&parser->exec_lock, OS_WAIT_FOREVER);
}

/**
 ***********************************************************************************************************************
 * @brief           Unlock the recursive lock that protects the execution of the AT command process.
 * @param[in]       parser          A pointer to AT Parser instance
 *
 * @return          @see os_mutex_recursive_unlock.
 ***********************************************************************************************************************
 */
os_err_t at_parser_exec_unlock(at_parser_t *parser)
{
    return os_mutex_recursive_unlock(&parser->exec_lock);
}

/**
 ***********************************************************************************************************************
 * @brief           Send AT command via AT parser and wait for response
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       resp_need       Whether a response is required
 * @param[in]       cmd_expr        The AT command expression
 * @param[in]       args            The AT command expression arguments
 *
 * @return          Return AT command execution result
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t at_parser_exec_cmd_valist(at_parser_t *parser, at_resp_t *resp, const char *cmd_expr, va_list args)
{
    OS_ASSERT(parser != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(resp->buff != OS_NULL);
    OS_ASSERT(cmd_expr != OS_NULL);

    os_err_t result = OS_EOK;

    at_parser_exec_lock(parser);

    memset(resp->buff, 0, resp->buff_size);
    resp->stat          = RESP_STAT_NULL;
    resp->curr_buff_len = 0;
    resp->line_counts   = 0;

    parser->resp        = resp;

    /* Send at command to module */
    at_parser_vasprintfln(parser, cmd_expr, args);

    if (os_sem_wait(&(parser->resp_notice), resp->timeout) != OS_EOK)
    {
        ERROR("execute command (%s) timeout (%d ticks)!", parser->at_cmd_buff, resp->timeout);
        resp->stat = RESP_STAT_TIMEOUT;
        result     = OS_ETIMEOUT;
        goto __exit;
    }

    if (resp->stat != RESP_STAT_OK)
    {
        ERROR("execute command (%s) failed, parser->resp_status:%d",parser->at_cmd_buff, resp->stat);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    if (parser->at_cmd_buff != OS_NULL)
    {
        os_free(parser->at_cmd_buff);
        parser->at_cmd_buff = OS_NULL;
    }

    parser->resp = OS_NULL;

    at_parser_exec_unlock(parser);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Send AT command via AT parser and wait for response
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       resp_need       Whether a response is required
 * @param[in]       cmd_expr        The AT command expression
 * @param[in]       ...             The AT command expression arguments
 *
 * @return          Return AT command execution result
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t at_parser_exec_cmd(at_parser_t *parser, at_resp_t *resp, const char *cmd_expr, ...)
{
    OS_ASSERT(parser != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(resp->buff != OS_NULL);
    OS_ASSERT(cmd_expr != OS_NULL);


    os_err_t result = OS_EOK;
    va_list  args   = {0};

    va_start(args, cmd_expr);
    result = at_parser_exec_cmd_valist(parser, resp, cmd_expr, args);
    va_end(args);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Send data to modules via AT parser
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       buf             The send data buffer
 * @param[in]       size            The size of send data
 *
 * @return          Send data result, @see os_device_write_block
 ***********************************************************************************************************************
 */
os_size_t at_parser_send(at_parser_t *parser, const char *buf, os_size_t size)
{
    OS_ASSERT(buf);

    os_size_t sent_size = 0;

    if (parser == OS_NULL)
    {
        ERROR("input AT parser object is NULL, please create or get AT parser object!");
        return 0;
    }

#ifdef AT_PARSER_PRINT_RAW
    at_parser_print_raw("send", buf, size);
#endif

    at_parser_exec_lock(parser);

    while (sent_size < size)
    {
        sent_size += os_device_write_block(parser->device, 0, buf + sent_size, size - sent_size);
    }

    at_parser_exec_unlock(parser);

    return sent_size;
}

static os_err_t at_parser_getchar(at_parser_t *parser, char *ch, os_int32_t timeout)
{
    os_err_t result = OS_EOK;

    while (os_device_read_nonblock(parser->device, 0, ch, 1) == 0)
    {
        result = os_event_recv(&parser->rx_notice_evt,
                                AT_PARSER_RX_ARRIVE,
                                OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                                timeout,
                                OS_NULL);

        if (OS_EOK != result)
        {
            return result;
        }
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Receive data from modules via AT parser
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in,out]   buf             Receive data buffer
 * @param[in]       size            Receive fixed size
 * @param[in]       timeout         receive data timeout (ms)
 *
 * @return          Returns the length of the data received
 * @retval          > 0             Receive data size
 * @retval          == 0            Receive failed
 ***********************************************************************************************************************
 */
os_size_t at_parser_recv(at_parser_t *parser, char *buf, os_size_t size, os_int32_t timeout)
{
    OS_ASSERT(parser != OS_NULL);
    OS_ASSERT(buf != OS_NULL);

    char      ch       = 0;
    os_size_t read_idx = 0;
    os_err_t  result   = OS_EOK;

    while (1)
    {
        if (read_idx < size)
        {
            result = at_parser_getchar(parser, &ch, timeout);
            if (result != OS_EOK)
            {
                ERROR("AT parser receive failed, uart device get data error(%d), read cnt(%d)", result, read_idx);
                return 0;
            }

            buf[read_idx++] = ch;
        }
        else
        {
            break;
        }
    }

#ifdef AT_PARSER_PRINT_RAW
    at_parser_print_raw("urc_recv", buf, size);
#endif

    return read_idx;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT test command to test whether the module is connected correctly
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       retry_times     The number of times the AT test was repeated
 *
 * @retval          OS_EOK          The connection was successful
 * @retval          Other           connect failed
 ***********************************************************************************************************************
 */
os_err_t at_parser_connect(at_parser_t *parser, os_uint8_t retry_times)
{
    OS_ASSERT(parser != OS_NULL);

    os_err_t result = OS_EOK;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff)};

    at_parser_exec_lock(parser);

    parser->resp = &resp;

    for (int i = 0; i < retry_times; i++)
    {
        at_parser_send(parser, "AT\r\n", 4);

        result = os_sem_wait(&parser->resp_notice, AT_RESP_TIMEOUT_DEF);
        if (OS_EOK == result)
        {
            break;
        }
    }

    parser->resp = OS_NULL;

    at_parser_exec_unlock(parser);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Set URC(Unsolicited Result Code) table of an AT Parser instance
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       urc_table       The urc table
 * @param[in]       table_size      The length of urc table
 ***********************************************************************************************************************
 */
void at_parser_set_urc_table(at_parser_t *parser, at_urc_t urc_table[], os_size_t table_size)
{
    OS_ASSERT(parser != OS_NULL);

    AT_PARSER_PROTECT;

    for (int i = 0; i < table_size; i++)
    {
        os_slist_init(&(urc_table[i].list));
        os_slist_add_tail(&(parser->urc_list), &(urc_table[i].list));
    }

    AT_PARSER_UNPROTECT;
}

static at_urc_t *at_parser_get_urc(at_parser_t *parser)
{
    OS_ASSERT(parser);

    os_size_t prefix_len = 0;
    os_size_t suffix_len = 0;

    if (parser->urc_list.next == OS_NULL)
    {
        return OS_NULL;
    }

    char     *buffer  = parser->recv_buff;
    os_size_t buff_sz = parser->curr_recv_len;
    at_urc_t *urc     = OS_NULL;

    AT_PARSER_PROTECT;

    for (os_slist_node_t *node = parser->urc_list.next; node; node = os_slist_next(node))
    {
        urc = os_slist_entry(node, at_urc_t, list);
        if (urc != OS_NULL)
        {
            prefix_len = strlen(urc->prefix);
            suffix_len = strlen(urc->suffix);
            if (buff_sz < prefix_len + suffix_len)
            {
                continue;
            }

            if ((prefix_len ? !strncmp(buffer, urc->prefix, prefix_len) : 1) &&
                (suffix_len ? !strncmp(buffer + buff_sz - suffix_len, urc->suffix, suffix_len) : 1))
            {
                AT_PARSER_UNPROTECT;
                return urc;
            }
        }
    }

    AT_PARSER_UNPROTECT;

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Set the end mark of an AT Parser instance
 *
 * @param[in]       parser          A pointer to AT Parser instance
 * @param[in]       end_mark_str    The string of end mark
 * @param[in]       end_mark_len    The length of end mark string
 *
 * @return          Operate
 * @retval          OS_EOK          Operate successfully
 * @retval          OS_ERROR        Operate failed
 ***********************************************************************************************************************
 */
os_err_t at_parser_set_end_mark(at_parser_t *parser, const char *end_mark_str, os_size_t end_mark_len)
{
    OS_ASSERT(parser != OS_NULL);

    if (end_mark_len > 2)
    {
        ERROR("end mark is too long, the maximum length is 2");
        return OS_ERROR;
    }

    memset(parser->end_mark, 0, END_MARK_LEN + 1);

    if (end_mark_str == OS_NULL || end_mark_len == 0)
    {
        parser->end_mark_len = 0;
        return OS_EOK;
    }

    strncpy(parser->end_mark, end_mark_str, end_mark_len);

    parser->end_mark_len = end_mark_len;

    return OS_EOK;
}

static int at_parser_readline(at_parser_t *parser)
{
    os_size_t read_len       = 0;
    os_bool_t is_full        = OS_FALSE;
    char      prev_read_char = 0;
    char      curr_read_char = 0;

    memset(parser->recv_buff, 0x00, parser->recv_buff_len);
    parser->curr_recv_len = 0;

    while (1)
    {
        at_parser_getchar(parser, &curr_read_char, OS_WAIT_FOREVER);

        if (read_len < parser->recv_buff_len)
        {
            parser->recv_buff[read_len++] = curr_read_char;
            parser->curr_recv_len         = read_len;
        }
        else
        {
            is_full = OS_TRUE;
        }

        /* is newline or URC data */
        if (at_parser_get_urc(parser))
        {
            break;
        }
        else if (curr_read_char == '\n' && prev_read_char == '\r')
        {
            break;
        }
        else if (parser->end_mark_len == 1)
        {
            if (curr_read_char == parser->end_mark[0])
            {
                break;
            }
        }
        else if (parser->end_mark_len == 2)
        {
            if (curr_read_char == parser->end_mark[1] && prev_read_char == parser->end_mark[0])
            {
                break;
            }
        }
        prev_read_char = curr_read_char;
    }

    if (is_full)
    {
        ERROR("read line failed. The line data length is out of buffer size(%d)!", parser->recv_buff_len);
        memset(parser->recv_buff, 0x00, parser->recv_buff_len);
        parser->curr_recv_len = 0;
        return OS_EFULL;
    }
    else
    {
#ifdef AT_PARSER_PRINT_RAW
        at_parser_print_raw("recvline", parser->recv_buff, read_len);
#endif
        return read_len;
    }
}

static void at_parser_resp_handle(at_parser_t *parser)
{
    at_resp_t *resp = parser->resp;

    if ('\n' == parser->recv_buff[parser->curr_recv_len - 1])
    {
        /* skip the empty line */
        if (2 >= parser->curr_recv_len) return;

        /* line end with "/r/n" */
        parser->recv_buff[parser->curr_recv_len - 1] = '\0';
    }
    else
    {
        /* line end with user custom end mark */
        /* at_parser_readline() max recv len is parser->recv_buff_len - 1, using it as '\0' */
        parser->curr_recv_len++;
    }

    if (resp->curr_buff_len + parser->curr_recv_len <= resp->buff_size)
    {
        /* copy response lines, separated by '\0' */
        memcpy(resp->buff + resp->curr_buff_len, parser->recv_buff, parser->curr_recv_len);

        /* update the current response information */
        resp->curr_buff_len += parser->curr_recv_len;
        resp->line_counts++;
    }
    else
    {
        resp->stat = RESP_STAT_BUFF_FULL;
        ERROR("Read response buffer failed. The Response buffer size is out of buffer size(%d)!", resp->buff_size);
    }

    /* check response result */
    if (memcmp(parser->recv_buff, AT_RESP_OK, strlen(AT_RESP_OK)) == 0 && resp->line_num == 0 && parser->end_mark_len == 0)
    {
        /* get the end data by response result, return response state END_OK. */
        resp->stat = RESP_STAT_OK;
    }
    else if (strstr(parser->recv_buff, AT_RESP_ERROR))
    {
        resp->stat = RESP_STAT_ERROR;
    }
    else if (resp->line_num != 0 && resp->line_counts == resp->line_num)
    {
        /* get the end data by response line, return response state END_OK.*/
        resp->stat = RESP_STAT_OK;
    }
    else if (0 != parser->end_mark_len &&
             !memcmp(parser->recv_buff + parser->curr_recv_len - parser->end_mark_len - 1,
                    parser->end_mark,
                    parser->end_mark_len))
    {
        /* when process the end mark line, return response state END_OK. */
        resp->stat = RESP_STAT_OK;
    }
    else
    {
        /* not the finish line, return. */
        return;
    }

    parser->resp = OS_NULL;
    os_sem_post(&parser->resp_notice);
}

static void at_parser_task(at_parser_t *parser)
{
    const at_urc_t *urc;

    while (1)
    {
        if (at_parser_readline(parser) > 0)
        {
            urc = at_parser_get_urc(parser);
            if (urc != OS_NULL && urc->func != OS_NULL)
            {
                /* current receive is urc, try to execute related operations */
                urc->func(parser, parser->recv_buff, parser->curr_recv_len);
            }
            else if (parser->resp != OS_NULL)
            {
                /* current receive is response, try to handle response */
                at_parser_resp_handle(parser);
            }
        }
    }
}

static os_err_t at_parser_rx_indicate(os_device_t *dev, struct os_device_cb_info *info)
{
    /* TODO (usb recv size == 0) -> event exit&deinit */
    if (info->size > 0)
    {
        at_parser_t *parser = (at_parser_t *)info->data;

        os_event_send(&parser->rx_notice_evt, AT_PARSER_RX_ARRIVE);
    }

    return OS_EOK;
}

static os_err_t at_parser_device_open(at_parser_t *parser, os_device_t *device)
{
    os_err_t   result = OS_EOK;

    result = os_device_open(device);
    if (result != OS_EOK)
    {
        ERROR("AT Parser initialize failed! Can not open the device.");
        return result;
    }

    parser->device = device;

    struct os_device_cb_info cb_info = {
        .type = OS_DEVICE_CB_TYPE_RX,
        .data = parser, /* The AT Parser operator is passed through a callback structure */
        .cb   = at_parser_rx_indicate,
    };

    result = os_device_control(parser->device, OS_DEVICE_CTRL_SET_CB, &cb_info);
    if (result != OS_EOK)
    {
        ERROR("Set AT Parse device receive indicate failed.");
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Unbind device of AT Parser instance
 *
 * @param[in]       parser          A pointer to AT Parser instance
 *
 * @return          Operate
 * @retval          OS_EOK          Operate successfully
 * @retval          OS_ERROR        Operate failed
 ***********************************************************************************************************************
 */
os_err_t at_parser_unbind_device(at_parser_t *parser)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != parser->device);

    at_parser_exec_lock(parser);

    os_err_t result = os_device_close(parser->device);
    if (OS_EOK != result)
    {
        ERROR("AT Parse device unbind failed.");
        at_parser_exec_unlock(parser);
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Rebind device of AT Parser instance
 *
 * @param[in]       parser          A pointer to AT Parser instance
 *
 * @return          Operate
 * @retval          OS_EOK          Operate successfully
 * @retval          OS_ERROR        Operate failed
 ***********************************************************************************************************************
 */
os_err_t at_parser_rebind_device(at_parser_t *parser)
{
    OS_ASSERT(parser != OS_NULL);

    os_err_t result = at_parser_device_open(parser, parser->device);
    if (OS_EOK != result)
    {
        ERROR("AT Parse device rebind failed.");
    }

    /* unlock at_parser_exec lock when specify process done(eg.PPP) */

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Init an instance of AT parser
 *
 * @param[in]       parser          The pointer to an instance of AT parsers
 * @param[in]       name            The name of AT parser
 * @param[in]       device          The device used by AT parser
 * @param[in]       recv_len        The receive buffer length of AT parser
 *
 * @return          Return operate result
 * @retval          == OS_EOK       Init successfully
 * @retval          != OS_EOK       Init failed
 ***********************************************************************************************************************
 */
os_err_t at_parser_init(at_parser_t *parser, const char *name, os_device_t *device, os_size_t recv_buff_len)
{
    os_err_t result = OS_EOK;

    char name_buffer[OS_NAME_MAX + 1] = {0};

    /* set parser infomation */
    strncpy(parser->name, name, OS_NAME_MAX);

    at_snprintf(name_buffer, OS_NAME_MAX, "%s_rx", name);

    os_event_init(&(parser->rx_notice_evt), name_buffer);

    at_snprintf(name_buffer, OS_NAME_MAX, "%s_resp", name);

    os_sem_init(&(parser->resp_notice), name_buffer, 0, OS_SEM_MAX_VALUE);

    at_snprintf(name_buffer, OS_NAME_MAX, "%s_lock", name);

    os_mutex_init(&(parser->exec_lock), name_buffer, OS_TRUE);

    os_slist_init(&(parser->urc_list));

    /* allocate and clean recv buffer  */
    parser->recv_buff_len = recv_buff_len;

    parser->recv_buff = (char *)os_calloc(1, recv_buff_len);
    if (parser->recv_buff == OS_NULL)
    {
        /* no memory can be allocated */
        ERROR("AT Parser memory allocation failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    result = at_parser_device_open(parser, device);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    at_snprintf(name_buffer, OS_NAME_MAX, "%s_parser", name);

    parser->task = os_task_create(name_buffer,
                                  (void (*)(void *parameter))at_parser_task,
                                  (void *)parser,
                                  AT_PARSER_TASK_STACK_SIZE,
                                  AT_PARSER_TASK_PRIORITY);
    if (parser->task == OS_NULL)
    {
        /* Task name has been occupied or no mem for task */
        ERROR("AT Parser(TE) task create failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    at_parser_list_add(parser);

__exit:
    if (result != OS_EOK)
    {
        os_event_deinit(&(parser->rx_notice_evt));

        os_sem_deinit(&(parser->resp_notice));

        os_mutex_deinit(&(parser->exec_lock));

        if (parser->recv_buff != OS_NULL)
        { /* free receive buffer */
            os_free(parser->recv_buff);
        }

        if (parser->device)
        {
            os_device_close(parser->device);
        }
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinit an instance of AT parser
 *
 * @param[in]       parser          An instance of AT Parser to be deinit
 *
 * @retval          OS_EOK          Deinit successfully
 ***********************************************************************************************************************
 */
os_err_t at_parser_deinit(at_parser_t *parser)
{
    OS_ASSERT(parser != OS_NULL);

    at_parser_list_del(parser);

    if(parser->task != OS_NULL)
    {
        os_task_destroy(parser->task);
    }

    os_event_deinit(&(parser->rx_notice_evt));

    os_sem_deinit(&(parser->resp_notice));

    os_mutex_deinit(&(parser->exec_lock));

    if (parser->recv_buff != OS_NULL)
    {
        os_free(parser->recv_buff);
    }

    if (parser->device != OS_NULL)
    {
        os_device_close(parser->device);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Startup an instance of AT parser
 *
 * @param[in]       parser          An instance of AT Parser to be startup
 *
 * @return          Return the result of the operation, @see os_task_stratup
 ***********************************************************************************************************************
 */
os_err_t at_parser_startup(at_parser_t *parser)
{
    OS_ASSERT(parser != OS_NULL);

    return os_task_startup(parser->task);
}

#endif /* MOLINK_USING_PARSER */
