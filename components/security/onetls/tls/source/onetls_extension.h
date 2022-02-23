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
 * @file        onetls_extension.h
 *
 * @brief       onetls_extension header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_EXTENSION_H__
#define __ONETLS_EXTENSION_H__
#include "onetls.h"

#define ONETLS_DH_PSK_ONLY   (0)
#define ONETLS_DH_PSK_DHE    (1)

typedef enum {
    onetls_ext_server_name = (0),                                         /* RFC 6066 */
    onetls_ext_max_fragment_length = (1),                                 /* RFC 6066 */
    onetls_ext_status_request = (5),                                      /* RFC 6066 */
    onetls_ext_supported_groups = (10),                                   /* RFC 8422, 7919 */
    onetls_ext_signature_algorithms = (13),                               /* RFC 8446 */
    onetls_ext_use_srtp = (14),                                           /* RFC 5764 */
    onetls_ext_heartbeat = (15),                                          /* RFC 6520 */
    onetls_ext_application_layer_protocol_negotiation = (16),             /* RFC 7301 */
    onetls_ext_signed_certificate_timestamp = (18),                       /* RFC 6962 */
    onetls_ext_client_certificate_type = (19),                            /* RFC 7250 */
    onetls_ext_server_certificate_type = (20),                            /* RFC 7250 */
    onetls_ext_padding = (21),                                            /* RFC 7685 */
    onetls_ext_session_ticket = (35),                                     /* RFC 5077 */
    onetls_ext_pre_shared_key = (41),                                     /* RFC 8446 */
    onetls_ext_early_data = (42),                                         /* RFC 8446 */
    onetls_ext_supported_versions = (43),                                 /* RFC 8446 */
    onetls_ext_cookie = (44),                                             /* RFC 8446 */
    onetls_ext_psk_exchange_modes = (45),                                 /* RFC 8446 */
    onetls_ext_certificate_authorities = (47),                            /* RFC 8446 */
    onetls_ext_oid_filters = (48),                                        /* RFC 8446 */
    onetls_ext_post_handshake_auth = (49),                                /* RFC 8446 */
    onetls_ext_signature_algorithms_cert = (50),                          /* RFC 8446 */
    onetls_ext_key_share = (51),                                          /* RFC 8446 */
} onetls_ext_extension_type;

typedef uint32_t(*onetls_extension_func)(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *length);
typedef struct {
    uint16_t    iana_id;
    onetls_extension_func func;
} onetls_extension;

uint32_t onetls_parse_server_extension(onetls_ctx *ctx, uint8_t state, uint8_t *packet, uint32_t packet_len);

uint32_t onetls_construct_client_hello_extension(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_psk_extension(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_support_version(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_cookie(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_server_name(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_psk_ex_mode(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_support_group(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_key_share(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_early_data(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_session_tickets(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_send_sign_alg(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);

uint32_t onetls_ext_recv_server_name(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_support_group(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_early_data(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_cookie(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_support_version(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_key_share(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_psk_ex(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);
uint32_t onetls_ext_recv_early_data_empty(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len);


#endif
