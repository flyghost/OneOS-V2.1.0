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
 * @file        onetls_extension.c
 *
 * @brief       onetls_extension functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_extension.h"
#include "onetls_lib.h"
#include "onetls_crypto.h"
#include "onetls_alert.h"

static onetls_extension g_onetls_client_hello_ext[] = {
    {onetls_ext_supported_versions,   onetls_ext_send_support_version},
    {onetls_ext_cookie,               onetls_ext_send_cookie},
    {onetls_ext_server_name,          onetls_ext_send_server_name},
    {onetls_ext_psk_exchange_modes,   onetls_ext_send_psk_ex_mode},
    {onetls_ext_supported_groups,     onetls_ext_send_support_group},
    {onetls_ext_key_share,            onetls_ext_send_key_share},
    {onetls_ext_early_data,           onetls_ext_send_early_data},
    {onetls_ext_session_ticket,       onetls_ext_send_session_tickets},
    {onetls_ext_signature_algorithms, onetls_ext_send_sign_alg},
    {onetls_ext_pre_shared_key,       onetls_ext_send_psk_extension}, /* MUST be last */
};

static onetls_extension g_onetls_server_hello_ext[] = {
    {onetls_ext_cookie,               onetls_ext_recv_cookie},
    {onetls_ext_supported_versions,   onetls_ext_recv_support_version},
    {onetls_ext_key_share,            onetls_ext_recv_key_share},
    {onetls_ext_pre_shared_key,       onetls_ext_recv_psk_ex},
};

static onetls_extension g_onetls_server_ee_ext[] = {
    {onetls_ext_supported_groups,     onetls_ext_recv_support_group},
    {onetls_ext_early_data,           onetls_ext_recv_early_data_empty},
    {onetls_ext_server_name,          onetls_ext_recv_server_name},
//    {onetls_ext_max_fragment_length,  onetls_ext_recv_max_fragment},    
};

static onetls_extension g_onetls_server_nst_ext[] = {
    {onetls_ext_early_data,          onetls_ext_recv_early_data},
};

uint32_t onetls_construct_client_hello_extension(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *start = packet + 2; // 长度晚点反写
    uint8_t *end = packet + packet_len;    
    
    // 添加扩展
    for (uint8_t offset = 0; offset < sizeof(g_onetls_client_hello_ext) / sizeof(onetls_extension); offset ++) {
        uint32_t length = 0;
        uint32_t ret = g_onetls_client_hello_ext[offset].func(ctx, start, end - start, &length);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "client hello extension[ext_id = %d]", g_onetls_client_hello_ext[offset].iana_id);
            return ret;
        }
        start += length;
    }
    *ext_len = start - packet;
    onetls_put_u16(packet, *ext_len - 2);
    return ONETLS_SUCCESS;
}

const onetls_extension *onetls_get_ext_byid(const onetls_extension *exts, uint32_t num, uint16_t id)
{   // 查找extid
    for (uint8_t offset = 0; offset < num; offset ++) {
        if (exts[offset].iana_id == id) {
            return &(exts[offset]);
        }
    }
    return NULL;
}

void onetls_tls_get_server_exts(uint8_t state, const onetls_extension **exts, uint32_t *num)
{
    switch (state) {
        case ONETLS_SERVER_HELLO:
            *num = sizeof(g_onetls_server_hello_ext) / sizeof(onetls_extension);
            *exts = g_onetls_server_hello_ext;
            break;
        case ONETLS_ENCRYPTED_EXTENSIONS:
            *num = sizeof(g_onetls_server_ee_ext) / sizeof(onetls_extension);
            *exts = g_onetls_server_ee_ext;
            break;
        case ONETLS_SERVER_NEW_SESSION_TICKET:
            *num = sizeof(g_onetls_server_nst_ext) / sizeof(onetls_extension);
            *exts = g_onetls_server_nst_ext;
            break;            
        default:
            *num = 0;
            *exts = NULL;
            break;
    }
}

uint32_t onetls_parse_server_extension(onetls_ctx *ctx, uint8_t state, uint8_t *packet, uint32_t packet_len)
{
    uint32_t ret = 0;
    uint32_t num = 0;
    uint32_t out_len = 0;
    uint8_t *start = packet;
    uint8_t *end = packet + packet_len;    
    const onetls_extension *exts = NULL;
    const onetls_extension *ext = NULL;
    uint16_t ext_len = 0;
    uint16_t ext_id = 0;
    onetls_tls_get_server_exts(state, &exts, &num);

    start += onetls_get_u16(start, &ext_len);
    if ((ext_len + 2) != packet_len) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_parse_server_extension");
        return ONETLS_EXT_WRONG_LEN;
    }

    while ((ret == ONETLS_SUCCESS) && (start < end)) {
        // 获取一个extension id
        start += onetls_get_u16(start, &ext_id);
        ext = onetls_get_ext_byid(exts, num, ext_id);
        if (ext == NULL) {
            onetls_check_errlog(ONETLS_EXT_UNKNOWN, "unkown extension[%d]", ext_id);
            return ONETLS_EXT_UNKNOWN;
        }

        ret = ext->func(ctx, start, end - start, &out_len);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_tls_parse_server_ext[%d]", ext->iana_id);
            return ret;
        }
        start += out_len;
    }
    return ret;
}

uint32_t onetls_ext_send_psk_extension(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    if (ctx->handshake->binder_len == 0) {
        return ONETLS_SUCCESS;
    }

    *ext_len = ctx->handshake->binder_len + ctx->handshake->identity_len + 8;
    if (*ext_len > packet_len) {        
        return ONETLS_EXT_PACKET_TOO_SMALL;
    }

    p += onetls_put_u16(p, onetls_ext_pre_shared_key);
    p += onetls_put_u16(p, *ext_len - 4);
    p += onetls_put_u16(p, ctx->handshake->identity_len);

    if (ctx->resumption.ticket != NULL) { 
        p += onetls_put_u16(p, ctx->resumption.ticket_len);
        p += onetls_put_data(p, ctx->resumption.ticket, ctx->resumption.ticket_len);
        p += onetls_put_u32(p, ctx->resumption.age_add);    // 故意的
    }

    if (ctx->hint != NULL) {
        p += onetls_put_u16(p, ctx->hint_len);
        p += onetls_put_data(p, ctx->hint, ctx->hint_len);
        p += onetls_put_u32(p, 0);
    }
    // binder内容在外面填写
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_send_support_version(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    p += onetls_put_u16(p, onetls_ext_supported_versions);
    p += onetls_put_u16(p, 3);
    p += onetls_put_u8(p, 2);
    p += onetls_put_u16(p, ONETLS_VERSION_TLS_13);

    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_send_cookie(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;    
    if (ctx->handshake->cookie == NULL) {
        return ONETLS_SUCCESS;
    }
    p += onetls_put_u16(p, onetls_ext_cookie);
    p += onetls_put_u16(p, ctx->handshake->cookie_len + 2);
    p += onetls_put_u16(p, ctx->handshake->cookie_len);
    p += onetls_put_data(p, ctx->handshake->cookie, ctx->handshake->cookie_len);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_send_server_name(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t len = strlen(onetls_version());

    p += onetls_put_u16(p, onetls_ext_server_name);
    p += onetls_put_u16(p, len + 5);
    p += onetls_put_u16(p, len + 3);
    p += onetls_put_u8(p, 0);
    p += onetls_put_u16(p, len);
    p += onetls_put_data(p, (const uint8_t*)onetls_version(), len);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_send_psk_ex_mode(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    p += onetls_put_u16(p, onetls_ext_psk_exchange_modes);
    p += onetls_put_u16(p, 3);
    p += onetls_put_u8(p, 2);
    p += onetls_put_u8(p, ONETLS_DH_PSK_ONLY);
    p += onetls_put_u8(p, ONETLS_DH_PSK_DHE);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_send_support_group(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{   
    uint32_t length = 0;
    uint8_t *p = packet;
    p += onetls_put_u16(p, onetls_ext_supported_groups);
    onetls_construct_client_hello_attr(ONETLS_ATTR_GROUPS, p + 2, &length);
    p += onetls_put_u16(p, length);
    p += length;
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

void onetls_construct_ext_key_share(onetls_ctx *ctx, uint8_t *packet, uint32_t *length)
{
    uint8_t *p = packet + 2; // 总长度待填充
    for (uint8_t loop = 0; loop < ctx->handshake->key_share.key_share_num; loop++) {
        p += onetls_put_u16(p, ctx->handshake->key_share.key_share_all[loop].group->iana_id);
        p += onetls_put_u16(p, ctx->handshake->key_share.key_share_all[loop].group->public_key_len);
        p += onetls_put_data(p, 
                             ctx->handshake->key_share.key_share_all[loop].public_key,
                             ctx->handshake->key_share.key_share_all[loop].group->public_key_len);
    }
    *length = p - packet;
    onetls_put_u16(packet, *length - 2);
}

uint32_t onetls_ext_send_key_share(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint32_t length = 0;
    uint8_t *p = packet;
    p += onetls_put_u16(p, onetls_ext_key_share);
    onetls_construct_ext_key_share(ctx, p + 2, &length);
    p += onetls_put_u16(p, length);
    p += length;
    *ext_len = p - packet;
    return ONETLS_SUCCESS;    
}

uint32_t onetls_ext_send_early_data(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    if (ctx->early_data.data == NULL) {
        return ONETLS_SUCCESS;
    }

    p += onetls_put_u16(p, onetls_ext_early_data);
    p += onetls_put_u16(p, 0);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;    
}

uint32_t onetls_ext_send_session_tickets(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    if (ctx->psk_cb == NULL) {
        return ONETLS_SUCCESS;
    }

    p += onetls_put_u16(p, onetls_ext_session_ticket);
    p += onetls_put_u16(p, 0);
    *ext_len = p - packet;
    return ONETLS_SUCCESS; 
}

uint32_t onetls_ext_send_sign_alg(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint32_t length = 0;
    uint8_t *p = packet;
    p += onetls_put_u16(p, onetls_ext_signature_algorithms);
    onetls_construct_client_hello_attr(ONETLS_ATTR_SIGN, p + 2, &length);
    p += onetls_put_u16(p, length);
    p += length;
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_cookie(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint16_t length_a = 0;
    uint16_t length_b = 0;
    uint8_t *p = packet;
    if (!ctx->handshake->msg_tag.hrr) {
        onetls_check_errlog(ONETLS_EXT_NOT_NEED, "onetls_ext_recv_cookie");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_ILLEGAL_PARAMETER);
        return ONETLS_EXT_NOT_NEED;
    }

    if (packet_len < 4) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_cookie");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &length_a);
    p += onetls_get_u16(p, &length_b);
    if ((length_a != (length_b + 2)) ||
        (length_b > packet_len)) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_cookie");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;   
    }

    if (ctx->handshake->cookie) {
        onetls_free(ctx->handshake->cookie);
    }

    ctx->handshake->cookie = onetls_malloc(length_b);
    if (ctx->handshake->cookie == NULL) {
        onetls_check_errlog(ONETLS_MALLOC_FAIL, "onetls_malloc");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_INTERNAL_ERROR);
        return ONETLS_MALLOC_FAIL;
    }

    ctx->handshake->cookie_len = length_b;
    p += onetls_get_data(p, ctx->handshake->cookie, length_b);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_support_version(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length = 0;
    uint16_t version = 0;
    
    p += onetls_get_u16(p, &length);
    if (length != 2) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_support_version");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &version);
    if (version < ONETLS_VERSION_TLS_13) {
        onetls_check_errlog(ONETLS_EXT_WRONG_VER, "version not support");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_PROTOCOL_VERSION);
        return ONETLS_EXT_WRONG_VER;
    }

    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_key_share(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length = 0;
    uint16_t pub_len = 0;
    uint16_t group_id = 0;
    
    if (packet_len < 6) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_key_share");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &length);
    p += onetls_get_u16(p, &group_id);
    p += onetls_get_u16(p, &pub_len);

    if (length != (pub_len + 4)) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_key_share");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    if ((ctx->handshake->key_share.server_selected_id != 0) && 
        (ctx->handshake->key_share.server_selected_id != group_id)) {
        onetls_check_errlog(ONETLS_EXT_KEY_SHARE_MISS, "onetls_ext_recv_key_share");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_ILLEGAL_PARAMETER);
        return ONETLS_EXT_KEY_SHARE_MISS;
    }

    ctx->handshake->key_share.server_selected_id = group_id;
    for (uint8_t loop = 0; loop < ctx->handshake->key_share.key_share_num; loop++) {
        const onetls_dh_groups *group = ctx->handshake->key_share.key_share_all[loop].group;
        if (group->iana_id != group_id) {
            continue;
        }

        ctx->handshake->key_share.server_public = onetls_malloc(pub_len);
        if (ctx->handshake->key_share.server_public == NULL) {
            return ONETLS_MALLOC_FAIL;
        }
        ctx->handshake->key_share.server_public_key_len = pub_len;
        p += onetls_get_data(p, ctx->handshake->key_share.server_public, pub_len);

        uint32_t ret = onetls_gen_kex_with_pub_key(ctx, 
                                                   &(ctx->handshake->key_share.key_share_all[loop]),
                                                   ctx->handshake->key_share.server_public,
                                                   pub_len);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_gen_kex_with_pub_key");
            onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_INTERNAL_ERROR);
            return ret;
        }
        break;
    }
    onetls_release_handshake_key_share(ctx);
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_psk_ex(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t psk_id = 0;
    uint16_t length = 0;    

    p += onetls_get_u16(p, &length);
    if (length != 2) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_psk_ex");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &psk_id);
    if (psk_id > 1) { // 当前最多也就支持两个。0号是ticket/psk，1号是psk
        onetls_check_errlog(ONETLS_EXT_INVALID_PARAM, "onetls_ext_recv_psk_ex");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_ILLEGAL_PARAMETER);
        return ONETLS_EXT_INVALID_PARAM;        
    }
    ctx->handshake->server_selected_psk = psk_id;
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_early_data_empty(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length = 0;

    if (ctx->early_data.data == NULL) {
        onetls_check_errlog(ONETLS_EXT_NOT_NEED, "onetls_ext_recv_early_data");
        return ONETLS_EXT_NOT_NEED;        
    }

    p += onetls_get_u16(p, &length);
    if (length != 0) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_early_data_empty");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }    

    ctx->handshake->msg_tag.early_data_ee = 1;
    *ext_len = p - packet;
    return ONETLS_SUCCESS;    
}

uint32_t onetls_ext_recv_early_data(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length = 0;    

    if (packet_len < 6) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_early_data");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &length);
    if (length != 4) {
        onetls_check_errlog(ONETLS_EXT_WRONG_LEN, "onetls_ext_recv_early_data");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u32(p, &(ctx->resumption.max_early_data));
    *ext_len = p - packet;
    return ONETLS_SUCCESS;
}

uint32_t onetls_ext_recv_support_group(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length = 0;
    uint16_t length_id = 0;

    if (packet_len < 4) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &length);
    p += onetls_get_u16(p, &length_id);

    if ((length != (length_id + 2)) || (length > packet_len)) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    // TODO: 当前无实际使用场景，忽略该内容
    p += length_id;

    *ext_len = p - packet;
    return ONETLS_SUCCESS;    
}

uint32_t onetls_ext_recv_server_name(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *ext_len)
{
    uint8_t *p = packet;
    uint16_t length_a = 0;
    uint16_t length_b = 0;
    uint16_t length_c = 0;
    uint8_t type = 0;   // 暂时没用

    if (packet_len < 7) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u16(p, &length_a);
    p += onetls_get_u16(p, &length_b);
    if ((length_a != (length_b + 2)) || (length_a > packet_len)) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }

    p += onetls_get_u8(p, &type);
    p += onetls_get_u16(p, &length_c);
    if (length_b != (length_c + 3)) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_EXT_WRONG_LEN;
    }    

    // TODO: 这里因为暂时没有用server name，所以直接跳过去，如果后续要用，可以修改一下
    p += length_c;

    *ext_len = p - packet;
    return ONETLS_SUCCESS;  
}
