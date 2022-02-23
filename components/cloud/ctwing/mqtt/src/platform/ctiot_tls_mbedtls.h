/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        ctiot_tls_mbedtls.h
 *
 * \@brief       socket port file for tls
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-12-02   OneOS Team      first version
 * 2021-01-20   OneOS Team      adapt OneOS-V2.0
 ***********************************************************************************************************************
 */

#if !defined(CTIOT_TLS_MBEDTLS_H)
#define CTIOT_TLS_MBEDTLS_H

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "ctiot_mqtt_config.h"

typedef struct
{
    char* host;
    char* port;

    mbedtls_ssl_config config;
    mbedtls_ssl_context context;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_net_context server_fd;
    
    struct
    {
        mbedtls_x509_crt root_ca;
        mbedtls_x509_crt client_cert;
        mbedtls_pk_context client_key;
    }credentials;
}ctiot_tls_session_t;

/* Mbedtls recv mode */
//#define CTIOT_TLS_RECV_BLOCK
#define CTIOT_TLS_RECV_BLOCK_WITH_TIMEOUT

/* CTIOT TLS vefiry server cert level */
#define CTIOT_TLS_VERIFY_NONE
//#define CTIOT_TLS_VERIFY_OPTIONAL
//#define CTIOT_TLS_VERIFY_REQUIRED

#ifdef MBEDTLS_DEBUG_C
#define CTIOT_TLS_ENABLE_MBEDTLS_DEBUG
#endif

int ctiot_tls_network_mbedtls_establish(uintptr_t  *tls_session_ptr,
                                        const char *addr,
                                        const char *port,
                                        const char *ca_crt,
                                        const char *client_crt,
                                        const char *client_key,
                                        const char *client_pwd);
int ctiot_tls_network_mbedtls_close(ctiot_tls_session_t **pass_tls_session);
int ctiot_tls_network_mbedtls_read(ctiot_tls_session_t *tls_session, unsigned char *buf, size_t len);
int ctiot_tls_network_mbedtls_write(ctiot_tls_session_t *tls_session, const unsigned char *buf , size_t len);

#endif
