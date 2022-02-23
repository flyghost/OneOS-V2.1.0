/*
 *  tls client demonstration program
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 *  This file is provided under the Apache License 2.0, or the
 *  GNU General Public License v2.0 or later.
 *
 *  **********
 *  Apache License 2.0:
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  **********
 *
 *  **********
 *  GNU General Public License v2.0 or later:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  **********
 */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "os_task.h"
#include "os_errno.h"
#include "os_util.h"

#include "mbedtls/debug.h"

#define SSL_DEMO_STACK_SIZE (4*1024)
#define SSL_DEMO_PRIORITY   (OS_TASK_PRIORITY_MAX/2 - 2)

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time            time
#define mbedtls_time_t          time_t
#define mbedtls_fprintf         fprintf
#define mbedtls_printf          os_kprintf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

static os_bool_t gs_demo_task_state = OS_FALSE;

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_CERTS_C) || !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)

void tls_client_test_entry(void *parament)
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
            "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
            "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
            "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
            "not defined.\r\n");
}
#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#if defined(MBEDTLS_THREADING_ALT)
#include "mbedtls/threading.h"
#endif

#define TLS_DEMO_TAG  "tls_client" // used for both peers and dlog tag

#define SERVER_PORT   "443"
#define SERVER_NAME   "gitee.com"


//#define GET_REQUEST   "GET /cmcc-oneos/OneOS/raw/master/components/security/mbedtls/demo/tls_client_demo_file\r\n" \

#define GET_REQUEST   "GET /tangyao10/myTest/raw/master/test HTTP/1.1\r\n" \
                      "Host: gitee.com\r\n" \
                      "Connection: close\r\n" \
                      "\r\n"

#define SSL_DEMO_BUF_LEN  (1024)

/*DigiCert Global Root CA*/
const char * tls_demo_cas_pem = \
{
    \
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n" \
    "-----END CERTIFICATE-----" \
};

typedef int (*printf_func)( const char *, ...);
typedef int (*snprintf_func)(char * s, size_t n,const char * format, ...);


static void demo_debug( void *ctx, int level,
                        const char *file, int line,
                        const char *str )
{
    ((void) level);

    #define MBEDTLS_SRC_PATH_LENTH (58)
    #ifdef OS_USING_DLOG
    #include "dlog.h"
    LOG_I(TLS_DEMO_TAG, "%s:%04d: %s", file + MBEDTLS_SRC_PATH_LENTH, line, str); //not show the file path
    #else
    mbedtls_printf("%s:%04d: %s\r\n", file + MBEDTLS_SRC_PATH_LENTH, line, str);
    #endif
}

typedef struct mbedtls_comm_session
{
    size_t buffer_len;
    unsigned char *buffer;

    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_net_context server_fd;
    mbedtls_x509_crt cacert;
}mbedtls_comm_session_t;

void tls_client_test_entry(void *parament)
{
    int ret = 1;
    uint32_t flags;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_comm_session_t *ssl_client_instance;

    #ifdef MBEDTLS_PLATFORM_PRINTF_ALT
    mbedtls_platform_set_printf((printf_func)os_kprintf);
    #endif

    #ifdef MBEDTLS_PLATFORM_MEMORY

    #if !defined(MBEDTLS_PLATFORM_CALLOC_MACRO) || !defined(MBEDTLS_PLATFORM_FREE_MACRO)
    mbedtls_platform_set_calloc_free(calloc, mbedtls_oneos_free);
    #endif
    #endif

    #ifdef MBEDTLS_PLATFORM_SNPRINTF_ALT
    mbedtls_platform_set_snprintf((snprintf_func)os_snprintf);
    #endif

    ssl_client_instance = (mbedtls_comm_session_t *)malloc(sizeof(mbedtls_comm_session_t));
    if( NULL == ssl_client_instance ){
        mbedtls_printf( "malloc space for ssl_client_instance failed\r\n" );
        return;
    }

    ssl_client_instance->buffer_len = SSL_DEMO_BUF_LEN;
    ssl_client_instance->buffer = (unsigned char *)malloc( ssl_client_instance->buffer_len * sizeof(char) );
    if( NULL == ssl_client_instance->buffer ){
        mbedtls_printf( "malloc space for buffer failed\r\n" );
        return;
    }
    memset(ssl_client_instance->buffer, 0, ssl_client_instance->buffer_len);

    #if defined(MBEDTLS_THREADING_ALT)
    mbedtls_threading_set_alt(mbedtls_mutex_alt_init, mbedtls_mutex_alt_free, mbedtls_mutex_alt_lock, mbedtls_mutex_alt_unlock);
    #endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init( &ssl_client_instance->server_fd );
    mbedtls_ssl_init( &ssl_client_instance->ssl );
    mbedtls_ssl_config_init( &ssl_client_instance->conf );
    mbedtls_x509_crt_init( &ssl_client_instance->cacert );
    mbedtls_ctr_drbg_init( &ssl_client_instance->ctr_drbg );

    #if defined(MBEDTLS_DEBUG_C)
    mbedtls_printf("%s set debug_level = %d\r\n", __func__, *((int *)parament));
    mbedtls_debug_set_threshold( *((int *)parament) );
    #endif

    mbedtls_printf( "\r\n  . Seeding the random number generator..." );
    fflush( stdout );

    mbedtls_entropy_init( &ssl_client_instance->entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ssl_client_instance->ctr_drbg,
                                mbedtls_entropy_func,
                                &ssl_client_instance->entropy,
                                (const unsigned char *) TLS_DEMO_TAG,
                                strlen( TLS_DEMO_TAG ) ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ctr_drbg_seed returned %d\r\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 0. Initialize certificates
     */
    mbedtls_printf( "  . Loading the CA root certificate ..." );
    fflush( stdout );

    ret = mbedtls_x509_crt_parse( &ssl_client_instance->cacert,
                                (const unsigned char *)tls_demo_cas_pem,
                                (strlen( tls_demo_cas_pem) + 1) );
    if( ret < 0 )
    {
        mbedtls_printf( " failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x\r\n\r\n", -ret );
        goto exit;
    }

    mbedtls_printf( " ok (%d skipped)\r\n", ret );

    /*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT );
    fflush( stdout );

    ret = mbedtls_net_connect( &ssl_client_instance->server_fd,
                                SERVER_NAME,
                                SERVER_PORT,
                                MBEDTLS_NET_PROTO_TCP );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_net_connect returned %d\r\n\r\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 2. Setup stuff
     */
    mbedtls_printf( "  . Setting up the SSL/TLS structure..." );
    fflush( stdout );

    ret = mbedtls_ssl_config_defaults( &ssl_client_instance->conf,
                                        MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ssl_config_defaults returned %d\r\n\r\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\r\n" );

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &ssl_client_instance->conf, MBEDTLS_SSL_VERIFY_REQUIRED );
    mbedtls_ssl_conf_ca_chain( &ssl_client_instance->conf, &ssl_client_instance->cacert, NULL );
    mbedtls_ssl_conf_rng( &ssl_client_instance->conf, mbedtls_ctr_drbg_random, &ssl_client_instance->ctr_drbg );
    mbedtls_ssl_conf_dbg( &ssl_client_instance->conf, demo_debug, stdout );

    ret = mbedtls_ssl_setup( &ssl_client_instance->ssl, &ssl_client_instance->conf );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ssl_setup returned %d\r\n\r\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl_client_instance->ssl, SERVER_NAME ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ssl_set_hostname returned %d\r\n\r\n", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl_client_instance->ssl, &ssl_client_instance->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    while( ( ret = mbedtls_ssl_handshake( &ssl_client_instance->ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\r\n  ! mbedtls_ssl_handshake returned -0x%x\r\n\r\n", -ret );
            goto exit;
        }
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate..." );


    if( ( flags = mbedtls_ssl_get_verify_result( &ssl_client_instance->ssl ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n" );

        mbedtls_x509_crt_verify_info( (char *)ssl_client_instance->buffer, SSL_DEMO_BUF_LEN, "  ! ", flags );

        mbedtls_printf( "%s\r\n", ssl_client_instance->buffer );

        goto exit;
    }
    else
        mbedtls_printf( " ok\r\n" );

    /*
     * 3. Write the GET request
     */
    mbedtls_printf( "  > Write to server:" );
    fflush( stdout );

    while( ( ret = mbedtls_ssl_write( &ssl_client_instance->ssl, (const unsigned char *)GET_REQUEST, strlen(GET_REQUEST) ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\r\n  ! mbedtls_ssl_write returned %d\r\n\r\n", ret );
            goto exit;
        }
    }
    mbedtls_printf( " %d bytes written\r\n\r\n%s", ret, GET_REQUEST);

    /*
     * 7. Read the HTTP response
     */
    mbedtls_printf( "  < Read from server:\r\n" );
    fflush( stdout );

    do
    {
        memset( ssl_client_instance->buffer, 0, ssl_client_instance->buffer_len );
        ret = mbedtls_ssl_read( &ssl_client_instance->ssl, ssl_client_instance->buffer, ssl_client_instance->buffer_len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            mbedtls_printf( "failed\r\n  ! mbedtls_ssl_read returned %d\r\n\r\n", ret );
            break;
        }

        if( ret == 0 )
        {
            mbedtls_printf( "\r\n\nEOF\r\n\r\n" );
            break;
        }

        int cnt;
        for (cnt = 0; cnt < ret; cnt++)
        {
            mbedtls_printf("%c", ssl_client_instance->buffer[cnt]);
        }

        exit_code = MBEDTLS_EXIT_SUCCESS;
        //break;
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl_client_instance->ssl );

exit:

    if( exit_code != MBEDTLS_EXIT_SUCCESS )
    {
        mbedtls_strerror( ret, (char *)ssl_client_instance->buffer, SSL_DEMO_BUF_LEN );
        mbedtls_printf("Last error was: %d - %s\r\n", ret, ssl_client_instance->buffer );
    }

    mbedtls_net_free( &ssl_client_instance->server_fd );

    mbedtls_x509_crt_free( &ssl_client_instance->cacert );
    mbedtls_ssl_free( &ssl_client_instance->ssl );
    mbedtls_ssl_config_free( &ssl_client_instance->conf );
    mbedtls_ctr_drbg_free( &ssl_client_instance->ctr_drbg );
    mbedtls_entropy_free( &ssl_client_instance->entropy );

    free(ssl_client_instance->buffer);
    free(ssl_client_instance);

    mbedtls_printf( "\r\ntls client demo test finished\r\n");
    gs_demo_task_state = OS_FALSE;

}

#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_ENTROPY_C && MBEDTLS_SSL_TLS_C &&
          MBEDTLS_SSL_CLI_C && MBEDTLS_NET_C && MBEDTLS_RSA_C &&
          MBEDTLS_CERTS_C && MBEDTLS_PEM_PARSE_C && MBEDTLS_CTR_DRBG_C &&
          MBEDTLS_X509_CRT_PARSE_C */

int tls_client_demo_start(int argc, char *argv[])
{
	os_task_t *tls_demo_task;
    int debug_level = 0;

    if( OS_FALSE != gs_demo_task_state ){
        mbedtls_printf( "tls client demo are still in run state\r\n");
        return OS_ERROR;
    }

    #if defined(MBEDTLS_DEBUG_C)
    debug_level = MBDTLS_DEBUG_LOG_LEVEL;

    if( argc == 2 ){
        debug_level = atoi(argv[1]);
    }
    #endif

    tls_demo_task = os_task_create("tls_demo",
                            tls_client_test_entry,
                            (void *)&debug_level,
                            SSL_DEMO_STACK_SIZE,
                            SSL_DEMO_PRIORITY);
	if( tls_demo_task )
	{
		os_task_startup(tls_demo_task);
	}
    gs_demo_task_state = OS_TRUE;

    return OS_EOK;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(tls_client, tls_client_demo_start, "a tls client demo, tls_client [0-4](debug level, default: 0)");
#endif
