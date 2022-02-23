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
 * @file        coap_onetls.c
 *
 * @brief       coap_onetls functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "coap_internal.h"

#ifdef HAVE_ONETLS
#include "onetls.h"

#define IS_PSK (1 << 0)
#define IS_PKI (1 << 1)
#define IS_CLIENT (1 << 6)
#define IS_SERVER (1 << 7)

/**
***********************************************************************************************************************
* @struct        coap_onetls_context
*
* @brief       coap onetls context.
*
***********************************************************************************************************************
*/
struct coap_onetls_context
{
    uint8_t established;
    uint8_t psk_pki_enabled;		/* psk or pki */
};
typedef struct coap_onetls_context coap_onetls_context_t;

/* psk config */
static uint8_t psk_key[64];
static size_t psk_key_len;
static uint8_t psk_identity[64];
static size_t psk_identity_len;

static uint32_t onetls_get_psk(const uint8_t *hint, uint32_t hint_len, uint8_t *key, uint32_t max_key_len, uint32_t *key_len)
{
    if (max_key_len < psk_key_len) {
        return ONETLS_FAIL;
    }

    memcpy(key, psk_key, psk_key_len);
    *key_len = psk_key_len;

    return ONETLS_SUCCESS;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_is_supported
 *
 * @param[in]       none
 *
 * @return          1: supported; others: not supported
 ***********************************************************************************************************************
 */
int coap_dtls_is_supported(void) 
{
    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_is_supported
 *
 * @param[in]       none
 *
 * @return          1: supported; others: not supported
 * 
 * @note            TODO
 ***********************************************************************************************************************
 */
int coap_tls_is_supported(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_get_tls_library_version
 *
 * @param[in]       none
 *
 * @return          onetls library version
 ***********************************************************************************************************************
 */
coap_tls_version_t *coap_get_tls_library_version(void) 
{ 
    static coap_tls_version_t version;

    // TODO
    // onetls_version();

    version.version = 0x010001;
    version.built_version = 0x010001;
    version.type = COAP_TLS_LIBRARY_ONETLS;

    return &version;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_startup
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_dtls_startup(void)
{
    // onetls do not need startup 
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_set_log_level
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_dtls_set_log_level(int level) 
{
    // onetls no log level
    (void)level;
}

int coap_dtls_get_log_level(void) 
{
    // onetls no log level
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_new_context
 *
 * @param[in]       coap_context: a pointer to coap context
 *
 * @return          ctx
 ***********************************************************************************************************************
 */
void *coap_dtls_new_context(struct coap_context_t *coap_context)
{
    coap_onetls_context_t *dtls_context = coap_malloc(sizeof(coap_onetls_context_t));
    if (!dtls_context) {
        coap_dtls_free_context(dtls_context);
        return NULL;
    }

    return dtls_context;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_free_context
 *
 * @param[in]       handle: ctx handle
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_dtls_free_context(void *handle) 
{
    if (handle) {
        coap_onetls_context_t *dtls_context = (coap_onetls_context_t *)handle;
        coap_free(dtls_context);
    }    
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_new_client_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void *coap_dtls_new_client_session(coap_session_t *session) 
{
    int ret = 0;

    // onetls use context as ssl
    onetls_ctx *ctx = onetls_new_ctx(ONETLS_MODE_DTLS);

    onetls_set_socket(ctx, session->sock.fd, session->sock.fd);

    memcpy(psk_key, session->psk_key, session->psk_key_len);
    memcpy(psk_identity, session->psk_identity, session->psk_identity_len);
    psk_key_len = session->psk_key_len;
    psk_identity_len = session->psk_identity_len;

    onetls_set_psk_callback(ctx, onetls_get_psk);
    ret = onetls_set_outband_psk_hint(ctx, psk_identity, psk_identity_len);
    if (ret != 0) {
        onetls_del_ctx(ctx);
        return NULL;
    }   

    ret = onetls_connect(ctx);
    if (ret != ONETLS_SUCCESS && ret != ONETLS_SOCKET_TRYAGAIN) {
        onetls_del_ctx(ctx);
        return NULL;        
    }

    // while ((ret = onetls_connect(ctx)) != 0) {
    //     /* Only use in non-blocking socket */
    //     if ( ret != ONETLS_SOCKET_TRYAGAIN) {
    //         onetls_del_ctx(ctx);
    //         return NULL;
    //     }
    // }

    if (ret == ONETLS_SUCCESS) {
        coap_onetls_context_t *dtls_context = (coap_onetls_context_t*)(session->context->dtls_context);
        dtls_context->established = 1;
    }

    session->tls = ctx;

    return session->tls;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_free_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_dtls_free_session(coap_session_t *coap_session) 
{
    if (coap_session->context == NULL) {
        return;
    }
        
    onetls_ctx *ctx = (onetls_ctx *)(coap_session->tls);
    if (ctx) {
        onetls_del_ctx(ctx);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_send
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_send(coap_session_t *session, const uint8_t *data, size_t data_len)
{
    int ret = 0;
    uint32_t send_len = 0;

    coap_log(LOG_DEBUG, "call onetls_send\n");

    onetls_ctx *ctx = (onetls_ctx*)(session->tls);
    coap_onetls_context_t *dtls_context = (coap_onetls_context_t*)(session->context->dtls_context);

    if (dtls_context->established == 1) {
        if (session->state == COAP_SESSION_STATE_HANDSHAKE) {
            coap_handle_event(session->context, COAP_EVENT_DTLS_CONNECTED, session);
            coap_session_connected(session);
        }
    } else {
        ret = onetls_connect(ctx);
        if (ret == ONETLS_SUCCESS) {
            dtls_context->established = 1;
            coap_session_connected(session);
        } else {
            return -1;
        }   
    }

    coap_log(LOG_DEBUG, "coap send data hex dump");
    for (uint8_t i = 0; i < data_len; i++) {
        coap_log(LOG_DEBUG, "%02x", data[i]);
    }
    coap_log(LOG_DEBUG, "coap send data hex dump\n");

    ret = onetls_send(ctx, data, data_len, &send_len);
    if (ret != 0) {
        dtls_context->established = 0;
        coap_session_disconnected(session, COAP_NACK_TLS_FAILED);
        return -1;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_receive
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_receive(coap_session_t *session, const uint8_t *data, size_t data_len)
{
    int ret = 0;
    uint32_t recv_len = 0;
    uint8_t pdu[COAP_RXBUFFER_SIZE];

    coap_log(LOG_DEBUG, "call onetls_recv\n");

    onetls_ctx *ctx = (onetls_ctx*)(session->tls);
    coap_onetls_context_t *dtls_context = (coap_onetls_context_t*)(session->context->dtls_context);

    if (dtls_context->established == 1) {
        if (session->state == COAP_SESSION_STATE_HANDSHAKE) {
            coap_handle_event(session->context, COAP_EVENT_DTLS_CONNECTED, session);
            coap_session_connected(session);
        } 
    } else {
        ret = onetls_connect(ctx);
        if (ret == ONETLS_SUCCESS) {
            dtls_context->established = 1;
            coap_session_connected(session);
        } else {
            return -1;
        }   
    }

    ret = onetls_recv(ctx, pdu, sizeof(pdu), &recv_len);
    if (ret != ONETLS_SOCKET_TRYAGAIN && ret != ONETLS_SUCCESS) {
        coap_session_disconnected(session, COAP_NACK_TLS_FAILED);
        dtls_context->established = 0;
        return -1;
    } 

    coap_handle_dgram(session->context, session, pdu, (size_t)ret);

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_context_set_psk
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_context_set_psk(coap_context_t *ctx, const char *hint, coap_dtls_role_t role) 
{
    if (ctx->psk_key_len > sizeof(psk_key) || ctx->psk_hint_len > sizeof(psk_identity)) {
        return 0;
    }
    
    coap_onetls_context_t *dtls_context = (coap_onetls_context_t*)(ctx->dtls_context);
    dtls_context->psk_pki_enabled |= IS_PSK;

    // memcpy(psk_key, ctx->psk_key, ctx->psk_key_len);
    // memcpy(psk_identity, ctx->psk_hint, ctx->psk_hint_len);
    // psk_key_len = ctx->psk_key_len;
    // psk_identity_len = ctx->psk_hint_len;
        
    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_context_set_pki
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_context_set_pki(coap_context_t *ctx, coap_dtls_pki_t* setup_data, coap_dtls_role_t role ) 
{
    (void)ctx;
    (void)setup_data;
    (void)role;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_context_set_pki_root_cas
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_context_set_pki_root_cas(struct coap_context_t *ctx, const char *ca_file, const char *ca_path) 
{
    (void)ctx;
    (void)ca_file;
    (void)ca_path;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_context_check_keys_enabled
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_context_check_keys_enabled(coap_context_t *ctx)
{
    coap_onetls_context_t *dtls_context = (coap_onetls_context_t*)(ctx->dtls_context);
    
    return dtls_context->psk_pki_enabled ? 1 : 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_is_context_timeout
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_is_context_timeout(void) 
{
    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_get_context_timeout
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
coap_tick_t coap_dtls_get_context_timeout(void *dtls_context) 
{
    (void)dtls_context;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_get_timeout
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
coap_tick_t coap_dtls_get_timeout(coap_session_t *session, coap_tick_t now) 
{
    (void)session;
    (void)now;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_handle_timeout
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_dtls_handle_timeout(coap_session_t *session) 
{
    (void)session;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_get_overhead
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
unsigned int coap_dtls_get_overhead(coap_session_t *session) 
{
    (void)session;
    return 13 + 8 + 8;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_new_server_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void *coap_dtls_new_server_session(coap_session_t *session) 
{
    return NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_dtls_hello
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
int coap_dtls_hello(coap_session_t *c_session, const uint8_t *data, size_t data_len) 
{
    (void)c_session;
    (void)data;
    (void)data_len;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_new_server_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void *coap_tls_new_server_session(coap_session_t *session, int *connected) 
{
    (void)session;
    (void)connected;

    return NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_free_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void coap_tls_free_session(coap_session_t *coap_session) 
{
    (void)coap_session;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_new_client_session
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void *coap_tls_new_client_session(coap_session_t *session, int *connected) 
{
    (void)session;
    (void)connected;
    
    return NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_write
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
ssize_t coap_tls_write(coap_session_t *session, const uint8_t *data, size_t data_len) 
{
    return -1;  
}

/**
 ***********************************************************************************************************************
 * @brief           coap_tls_read
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
ssize_t coap_tls_read(coap_session_t *session,uint8_t *data,size_t data_len) 
{
    return -1;
}


#endif