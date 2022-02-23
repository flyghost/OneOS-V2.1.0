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
 * \@file        mq_tls_mbedtls.h
 *
 * \@brief       socket port file for tls
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first  version
 * 2020-12-29   OneOS Team      modify tls netowrk interface
 ***********************************************************************************************************************
 */

#if !defined(MQ_TLS_MBEDTLS_H)
#define MQ_TLS_MBEDTLS_H

#include <oneos_config.h>

#ifdef MQTT_USING_TLS_MBEDTLS

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"


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
}mq_tls_session_t;

/* Mbedtls recv mode */
//#define MQ_TLS_RECV_BLOCK
#define MQ_TLS_RECV_BLOCK_WITH_TIMEOUT

#ifdef MQTT_USING_TLS_MBEDTLS
/* TLS certificate mode */
/* MQTT security mode config one-way authentication */
#define MQ_TLS_SECURITY_ONE_WAY_AUTHENTICATION
/* MQTT security mode config two-way authentication */
//#define MQ_TLS_SECURITY_TWO_WAY_AUTHENTICATION
#endif

/* MQTT TLS vefiry server cert level */
#define MQ_TLS_VERIFY_NONE
//#define MQ_TLS_VERIFY_OPTIONAL
//#define MQ_TLS_VERIFY_REQUIRED

#ifdef MBEDTLS_DEBUG_C
#define MQ_TLS_ENABLE_MBEDTLS_DEBUG
#endif

int mq_tls_network_mbedtls_establish(uintptr_t  *tls_session_ptr,
                                     const char *addr,
                                     const char *port,
                                     const char *ca_crt,
                                     const char *client_crt,
                                     const char *client_key,
                                     const char *client_pwd);
int mq_tls_network_mbedtls_close(mq_tls_session_t **pass_tls_session);
int mq_tls_network_mbedtls_read(mq_tls_session_t *tls_session, unsigned char *buf, size_t len);
int mq_tls_network_mbedtls_write(mq_tls_session_t *tls_session, const unsigned char *buf , size_t len);

#endif

#endif
