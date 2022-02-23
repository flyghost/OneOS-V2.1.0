/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        cms_con_coap.h
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CMS_CON_COAP_H__
#define __CMS_CON_COAP_H__
#include "cms_con_def.h"
// #include "cms_con_coap_option_def.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    coap_code_get      = 0x01,    // 0.01
    coap_code_post     = 0x02,    // 0.02
    coap_code_put      = 0x03,    // 0.03
    coap_code_del      = 0x04,    // 0.04
    coap_code_created  = 0x41,    // 2.01
    coap_code_deleted  = 0x42,    // 2.02
    coap_code_valid    = 0x43,    // 2.03
    coap_code_changed  = 0x44,    // 2.04
    coap_code_content  = 0x45,    // 2.05
    coap_code_band_req = 0x80,    // 4.00
} cms_coap_method_code;

typedef enum
{
    carrying_mode,
    // separation_mode,         //not support
} cms_coap_interactive_mode;

typedef enum
{
    cms_coap_message_confirmable,
    cms_coap_message_non_confirmable,
    cms_coap_message_ack
} cms_coap_message_type;


typedef enum
{
    coap_option_if_match       = 1,  /* 0-8 B */
    coap_option_uri_host       = 3,  /* 1-255 B */
    coap_option_e_tag          = 4,  /* 1-8 B */
    coap_option_if_none_match  = 5,  /* 0 B */
    coap_option_observe        = 6,  /* 0-3 B */
    coap_option_uri_port       = 7,  /* 0-2 B */
    coap_option_location_path  = 8,  /* 0-255 B */
    coap_option_uri_path       = 11, /* 0-255 B */
    coap_option_content_format = 12, /* 0-2 B */
    coap_option_max_age        = 14, /* 0-4 B */
    coap_option_uri_query      = 15, /* 0-270 B */
    coap_option_accept         = 17, /* 0-2 B */
    coap_option_location_query = 20, /* 1-270 B */
    coap_option_block2         = 23, /* 1-3 B */
    // coap_option_block1         = 27, /* 1-3 B */
    coap_option_size2 = 28, /* 0-4 B */
    // coap_option_proxy_uri      = 35, /* 1-270 B */
    // coap_option_size1          = 60, /* 0-4 B */
    coap_option_max_value = 0xFFFF,
} cms_coap_option;

typedef enum
{
    coap_option_text_plain                   = 0,
    coap_option_text_xml                     = 1,
    coap_option_text_csv                     = 2,
    coap_option_text_html                    = 3,
    coap_option_image_gif                    = 21,
    coap_option_image_jpeg                   = 22,
    coap_option_image_png                    = 23,
    coap_option_image_tiff                   = 24,
    coap_option_audio_raw                    = 25,
    coap_option_video_raw                    = 26,
    coap_option_application_link_format      = 40,
    coap_option_application_xml              = 41,
    coap_option_application_octet_stream     = 42,
    coap_option_application_rdf_xml          = 43,
    coap_option_application_soap_xml         = 44,
    coap_option_application_atom_xml         = 45,
    coap_option_application_xmpp_xml         = 46,
    coap_option_application_exi              = 47,
    coap_option_application_fastinfoset      = 48,
    coap_option_application_soap_fastinfoset = 49,
    coap_option_application_json             = 50,
    coap_option_application_x_obix_binary    = 51,
    coap_option_content_max_value            = 0xffff
} cms_coap_option_content_format;

typedef struct _cms_con_coap_multi_option_
{
    struct _cms_con_coap_multi_option_ *next;
    uint8_t                             len;
    uint8_t *                           data;
} cms_con_coap_multi_option;

typedef struct
{
    uint8_t  len;
    uint8_t *data;
} cms_con_coap_single_option;

typedef struct
{
    uint8_t if_match;
    uint8_t if_none_match;
} cms_con_coap_option_if_match;

typedef struct
{
    cms_con_coap_single_option *uri_host;
    uint16_t                    uri_port;
    cms_con_coap_multi_option * uri_path;
    cms_con_coap_multi_option * uri_query;
} cms_con_coap_option_uri;

typedef struct
{
    cms_con_coap_multi_option *location_uri_path;
    cms_con_coap_multi_option *location_uri_query;
} cms_con_coap_option_location_uri;

typedef struct
{
    uint32_t max_age;
    uint8_t  observer;
} cms_con_coap_option_observer;

typedef struct
{
    uint16_t content_format;
} cms_con_coap_option_content_format;

typedef struct
{
    uint16_t content_format_accept;
} cms_con_coap_option_accept;

typedef struct
{
    uint32_t block_num;
    uint8_t  more_flag;
    uint16_t block_size;
} cms_con_coap_option_block2;

typedef struct
{
    uint32_t encoded_block_size;
} cms_con_coap_option_size2;

typedef struct
{
    cms_con_coap_option_if_match *      if_match;
    cms_con_coap_single_option *        e_tag;
    cms_con_coap_option_uri *           uri;
    cms_con_coap_option_location_uri *  location_uri;
    cms_con_coap_option_observer *      observer;
    cms_con_coap_option_content_format *content_format;
    cms_con_coap_option_accept *        accept;
    cms_con_coap_option_block2 *        block2;
    cms_con_coap_option_size2 *         size2;
} cms_con_coap_option;

typedef struct
{
    int                  message_type;
    int                  code;
    cms_con_coap_option *option;
    uint8_t *            buf;
    size_t               length;
} cms_coap_message;

typedef int (*cms_coap_on_recv_handler)(void *                  handle,
                                        const cms_coap_message *rec_message,
                                        cms_coap_message *      ack_message);
typedef void (*cms_coap_on_error_handler)(void *handle, int error_code);

typedef struct
{
    uint16_t                  timeout_ms;
    size_t                    sendbuf_size;
    size_t                    recvbuf_size;
    cms_coap_interactive_mode interactive_mode;
    cms_coap_on_recv_handler  on_recv;
    cms_coap_on_error_handler on_error;
} cms_coap_param;

void *cms_coap_init(int scode, const cms_coap_param *param);

void cms_coap_deinit(void *handle);

int cms_coap_send(void *handle, cms_coap_message *message);

#if defined(__cplusplus)
}
#endif

#endif
