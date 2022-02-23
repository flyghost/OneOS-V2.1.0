/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        mqtt_atest.c
 *
 * @brief       This is mqtt test file based atest.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"
#if defined(CMS_CONNECT_COAP)
#include "cms_con_coap.h"
#include <shell.h>
#include <atest.h>
#include <os_errno.h>
#include <os_assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <os_task.h>
#include <os_clock.h>

#define COAP_BUFF_LENGTH 1024
static int   scode       = 302;
static void *coap_handle = NULL;

static void atest_cms_coap_print_buff(const char *title, uint8_t *buf, size_t size)
{
    printf("%s: ", title);
    for (int i = 0; i < size; i++)
    {
        printf("%02x ", buf[i]);
    }
    printf("\r\n\r\n");
}
static void atest_cms_coap_print_string(const char *title, uint8_t *buf, size_t size)
{
    OS_ASSERT(size < 0x200);
    char *str = malloc(size + 1);

    if (str == NULL)
    {
        printf("%s:out of memery", __func__);
        return;
    }
    memcpy(str, buf, size);
    str[size] = 0;
    printf("%s: %s\r\n\r\n", title, str);
    free(str);
}

static int
atest_cms_coap_on_recv_handler(void *handle, const cms_coap_message *rec_message, cms_coap_message *ack_message)
{
    OS_ASSERT(handle == coap_handle);
    OS_ASSERT(rec_message != NULL);
    OS_ASSERT(ack_message != NULL);
    cms_con_coap_multi_option *multi_path = NULL;
    printf("message_type:%d(0:confirmable, 1:non conf, 2:ack)\r\n\r\n", rec_message->message_type);
    printf("code:%d.%d\r\n\r\n", (rec_message->code >> 5), (rec_message->code & 0x1F));
    if (rec_message->option != NULL)
    {
        if (rec_message->option->if_match != NULL)
        {
            if (rec_message->option->if_match->if_match)
                printf("if match:\r\n");
            else
                printf("if none match:\r\n");
            if (rec_message->option->e_tag != NULL)
            {
                atest_cms_coap_print_buff("etag",
                                          rec_message->option->e_tag->data,
                                          rec_message->option->e_tag->len);
            }
        }
        if (rec_message->option->uri != NULL)
        {
            if (rec_message->option->uri->uri_host != NULL)
            {
                atest_cms_coap_print_string("uri host",
                                            rec_message->option->uri->uri_host->data,
                                            rec_message->option->uri->uri_host->len);
            }
            if (rec_message->option->uri->uri_port != 0)
            {
                printf("uri port:%d\r\n\r\n", rec_message->option->uri->uri_port);
            }
            if (rec_message->option->uri->uri_path != NULL)
            {
                multi_path = rec_message->option->uri->uri_path;
                while (multi_path != NULL)
                {
                    atest_cms_coap_print_string("uri path", multi_path->data, multi_path->len);
                    multi_path = multi_path->next;
                }
            }
            if (rec_message->option->uri->uri_query != NULL)
            {
                multi_path = rec_message->option->uri->uri_query;
                while (multi_path != NULL)
                {
                    atest_cms_coap_print_string("uri query", multi_path->data, multi_path->len);
                    multi_path = multi_path->next;
                }
            }
        }
        if (rec_message->option->location_uri != NULL)
        {
            if (rec_message->option->location_uri->location_uri_path != NULL)
            {
                multi_path = rec_message->option->location_uri->location_uri_path;
                while (multi_path != NULL)
                {
                    atest_cms_coap_print_string("location uri path", multi_path->data, multi_path->len);
                    multi_path = multi_path->next;
                }
            }
            if (rec_message->option->location_uri->location_uri_query != NULL)
            {
                multi_path = rec_message->option->location_uri->location_uri_query;
                while (multi_path != NULL)
                {
                    atest_cms_coap_print_string("location uri query", multi_path->data, multi_path->len);
                    multi_path = multi_path->next;
                }
            }
        }
        if (rec_message->option->observer != NULL)
        {
            printf("observer:%d(0:enable, other:disable)\r\n\r\n", rec_message->option->observer->observer);
            if (rec_message->option->observer->max_age)
                printf("max_age:%d\r\n\r\n", rec_message->option->observer->max_age);
        }

        if (rec_message->option->content_format != NULL || rec_message->option->accept != NULL)
        {
            if (rec_message->option->content_format != NULL)
                printf("content format:%d\r\n\r\n", rec_message->option->content_format->content_format);
            if (rec_message->option->accept != NULL)
                printf("accept:%d\r\n\r\n", rec_message->option->accept->content_format_accept);
            printf("text_plain:0, text_xml:1, text_csv:2, text_html:3, image_gif:21, image_jpeg:22, image_png:23, "
                   "image_tiff:24, audio_raw:25, video_raw:26, application_link_format:40, application_xml:41, "
                   "application_atom_xml:45, application_xmpp_xml:46, application_exi:47, application_fastinfoset:48, "
                   "application_soap_fastinfoset:49, application_json:50, application_x_obix_binary:51\r\n\r\n");
        }
        if (rec_message->option->block2 != NULL)
        {
            printf("block2:\r\nblock_num:%d,more_flag:%d,block_size:%d\r\n\r\n",
                   rec_message->option->block2->block_num,
                   rec_message->option->block2->more_flag,
                   rec_message->option->block2->block_size);
        }
        if (rec_message->option->size2 != NULL)
        {
            printf("size2:\r\n"
                   "encoded_block_size:%d\r\n\r\n",
                   rec_message->option->size2->encoded_block_size);
        }
    }
    if (rec_message->length)
    {
        atest_cms_coap_print_string("payload", rec_message->buf, rec_message->length);
    }
    return 0;
}
static void atest_cms_coap_on_error_handler(void *handle, int error_code)
{
    printf("%s: error!", __func__);
}

static void atest_cms_coap_init(void)
{
    if (coap_handle != NULL)
        return;
    cms_coap_param param;
    param.timeout_ms       = 5 * 1000;
    param.sendbuf_size     = COAP_BUFF_LENGTH;
    param.recvbuf_size     = COAP_BUFF_LENGTH;
    param.interactive_mode = carrying_mode;
    param.on_recv          = atest_cms_coap_on_recv_handler;
    param.on_error         = atest_cms_coap_on_error_handler;
    coap_handle            = cms_coap_init(scode, &param);
    tp_assert_true(coap_handle != NULL);
}
static void atest_cms_coap_deinit(void)
{
    cms_coap_deinit(coap_handle);
    coap_handle = NULL;
}

static void atest_cms_coap_structure_message_if_match(cms_con_coap_option *option)
{
    static cms_con_coap_option_if_match if_match;
    static cms_con_coap_single_option    e_tag;
    if_match.if_match = TRUE;
    // if_match.if_none_match = TRUE;
    e_tag.data        = (uint8_t *)"1234";
    e_tag.len = 4;
    option->if_match   = &if_match;
    option->e_tag      = &e_tag;
}
static void atest_cms_coap_structure_message_uri(cms_con_coap_option *option)
{
    static cms_con_coap_single_option uri_host;
    static uint16_t                   uri_port;
    static cms_con_coap_multi_option  uri_path;
    static cms_con_coap_multi_option  uri_query;
    static cms_con_coap_option_uri    uri;
    static cms_con_coap_multi_option  multi_option;

    multi_option.data = (uint8_t *)"multi_option";
    multi_option.len  = strlen((char *)multi_option.data);
    multi_option.next = NULL;
    uri_host.data     = (uint8_t *)"this is a sample uri host";
    uri_host.len      = strlen((char *)uri_host.data);
    uri_port          = 6588;
    uri_path.data     = (uint8_t *)"root_path";
    uri_path.len      = strlen((char *)uri_path.data);
    uri_path.next     = &multi_option;
    uri_query.data    = (uint8_t *)"root_query";
    uri_query.len     = strlen((char *)uri_query.data);
    uri_query.next    = &multi_option;

    uri.uri_host  = &uri_host;
    uri.uri_port  = uri_port;
    uri.uri_path  = &uri_path;
    uri.uri_query = &uri_query;
    option->uri   = &uri;
}
static void atest_cms_coap_structure_message_location_uri(cms_con_coap_option *option)
{
    static cms_con_coap_multi_option        location_uri_path;
    static cms_con_coap_multi_option        location_uri_query;
    static cms_con_coap_multi_option        multi_option;
    static cms_con_coap_option_location_uri location_uri;

    multi_option.data       = (uint8_t *)"multi_option";
    multi_option.len        = strlen((char *)multi_option.data);
    multi_option.next       = NULL;
    location_uri_path.data  = (uint8_t *)"location_root_path";
    location_uri_path.len   = strlen((char *)location_uri_path.data);
    location_uri_path.next  = &multi_option;
    location_uri_query.data = (uint8_t *)"location_root_query";
    location_uri_query.len  = strlen((char *)location_uri_query.data);
    location_uri_query.next = &multi_option;

    location_uri.location_uri_path  = &location_uri_path;
    location_uri.location_uri_query = &location_uri_query;
    option->location_uri            = &location_uri;
}
static void atest_cms_coap_structure_message_observer(cms_con_coap_option *option)
{
    static cms_con_coap_option_observer observer;
    observer.observer = 0;
    observer.max_age  = 5;
    option->observer  = &observer;
}
static void atest_cms_coap_structure_message_content_format(cms_con_coap_option *option)
{
    static cms_con_coap_option_content_format content_format;

    content_format.content_format = coap_option_application_json;
    option->content_format        = &content_format;
}
static void atest_cms_coap_structure_message_accept(cms_con_coap_option *option)
{
    static cms_con_coap_option_accept accept;

    accept.content_format_accept = coap_option_text_xml;
    option->accept               = &accept;
}
static void atest_cms_coap_structure_message_block2(cms_con_coap_option *option)
{
    static cms_con_coap_option_block2 block2;

    // block2.block_num = 0x5;
    // block2.block_num = 0x5a5;
    block2.block_num  = 0x5a5a5;
    block2.more_flag  = 1;
    block2.block_size = 1 << (4 + 6);
    option->block2    = &block2;
}
static void atest_cms_coap_structure_message_size2(cms_con_coap_option *option)
{
    static cms_con_coap_option_size2 size2;
    size2.encoded_block_size = 0xf14a5a5a;
    option->size2            = &size2;
}

static void atest_cms_coap_send(void)
{
    static uint8_t             buf[64];
    static cms_coap_message    message;
    static cms_con_coap_option option;

    for (int index = 0; index < sizeof(buf); index++)
    {
        buf[index] = index % 10 + '0';
    }
    memset(&option, 0, sizeof(cms_con_coap_option));

    message.message_type = cms_coap_message_non_confirmable;
    message.code         = coap_code_post;
    message.buf          = buf;
    message.length       = sizeof(buf);
    message.option       = &option;
    atest_cms_coap_structure_message_if_match(&option);
    atest_cms_coap_structure_message_uri(&option);
    atest_cms_coap_structure_message_location_uri(&option);
    atest_cms_coap_structure_message_observer(&option);
    atest_cms_coap_structure_message_content_format(&option);
    atest_cms_coap_structure_message_accept(&option);
    atest_cms_coap_structure_message_block2(&option);
    atest_cms_coap_structure_message_size2(&option);

    int rc = cms_coap_send(coap_handle, &message);
    tp_assert_true(rc == 0);
}

static void cms_coap_all(void)
{
    ATEST_UNIT_RUN(atest_cms_coap_init);
    ATEST_UNIT_RUN(atest_cms_coap_send);
    ATEST_UNIT_RUN(atest_cms_coap_deinit);
}

ATEST_TC_EXPORT(cms.coap.all, cms_coap_all, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.coap.init, atest_cms_coap_init, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.coap.deinit, atest_cms_coap_deinit, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.coap.send, atest_cms_coap_send, NULL, NULL, TC_PRIORITY_LOW);
#endif
