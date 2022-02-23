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
 * \@file        ctiot_tls_mbedtls.c
 *
 * \@brief       socket port file for ctiot_tls
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-12-02   OneOS Team      first version
 * 2021-01-20   OneOS Team      adapt OneOS-V2.0
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_mutex.h>
#include <os_clock.h>
#include <os_errno.h>
#include <os_util.h>
#include "ctiot_tls_mbedtls.h"
#include "ctiot_log.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef OS_USING_SHELL
#include <shell.h>
#endif


#define _log_mbedtls_error(errno, msg_head)                               \
{                                                                         \
    char err_message[33] = {0};                                           \
    mbedtls_strerror(errno, err_message, 33);                             \
                                                                          \
    CTIOT_LOG(LOG_ERR, "%s, mbedtls error: err_code=%d, err_message=%s.", \
              msg_head,                                                   \
              errno,                                                      \
              err_message);                                               \
}

#if defined(CTIOT_TLS_ENABLE_MBEDTLS_DEBUG)
static void _log_mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void)level);

    CTIOT_LOG(LOG_DEBUG, "%s:%04d: %s", file, line, str);
}
#endif

static char *ctmq_strdup(const char *str)
{
    os_size_t  len;
    char      *str_tmp;

    len = strlen(str) + 1;
    str_tmp = (char *)malloc(len);
    if (!str_tmp)
    {
        return NULL;
    }

    memcpy(str_tmp, str, len);

    return str_tmp;
}

static int tls_network_mbedtls_init(ctiot_tls_session_t *tls_session)
{
    int mbedtls_err = 0;
    const char pers[] = "Hello_OneOS";

    /* Initialize the mbedtls structures in tls session. */
    mbedtls_ssl_config_init(&(tls_session->config));
    mbedtls_ssl_init(&(tls_session->context));
#if defined(CTIOT_TLS_SECURITY_ONE_WAY_AUTHENTICATION)
    mbedtls_x509_crt_init(&(tls_session->credentials.root_ca));
#elif defined(CTIOT_TLS_SECURITY_TWO_WAY_AUTHENTICATION)
    mbedtls_x509_crt_init(&(tls_session->credentials.root_ca));
    mbedtls_x509_crt_init(&(tls_session->credentials.client_cert));
    mbedtls_pk_init(&(tls_session->credentials.client_key));
#endif

    /* Initialize contexts for random number generation. */
    mbedtls_entropy_init(&tls_session->entropy);
    mbedtls_ctr_drbg_init(&tls_session->ctr_drbg);

    /* Seed the random number generator, user can use pers. */
    mbedtls_err = mbedtls_ctr_drbg_seed(&tls_session->ctr_drbg,
                                        mbedtls_entropy_func,
                                        &tls_session->entropy,
                                        (unsigned char *)pers,
                                        strlen(pers));

    if(mbedtls_err != 0)
    {
        _log_mbedtls_error(mbedtls_err, "Failed to seed PRNG in initialization.");
    }
    else
    {
        CTIOT_LOG(LOG_INFO, "TLS network mbedtls initialized.");
    }

    return mbedtls_err;
}

static int tls_network_mbedtls_cleanup(ctiot_tls_session_t *tls_session)
{
    int mbedtls_err = 0;

    if (OS_NULL == tls_session)
    {
        return -1;
    }

    mbedtls_ssl_free(&tls_session->context);

#if defined(CTIOT_TLS_SECURITY_ONE_WAY_AUTHENTICATION)
    mbedtls_x509_crt_free(&(tls_session->credentials.root_ca));
#elif defined(CTIOT_TLS_SECURITY_TWO_WAY_AUTHENTICATION)
    mbedtls_pk_free(&(tls_session->credentials.client_key));
    mbedtls_x509_crt_free(&(tls_session->credentials.client_cert));
    mbedtls_x509_crt_free(&(tls_session->credentials.root_ca));
#endif
    mbedtls_ssl_config_free(&tls_session->config);

    mbedtls_entropy_free(&tls_session->entropy);
    mbedtls_ctr_drbg_free(&tls_session->ctr_drbg);

    CTIOT_LOG(LOG_INFO, "TLS network mbedtls library cleanup done.");

    return mbedtls_err;
}

static int read_tls_credentials(ctiot_tls_session_t *tls_session,
                                const char *root_ca,
                                const char *client_cert,
                                const char *client_key)
{
    int mbedtls_err = -1;
    
#if defined(CTIOT_TLS_SECURITY_ONE_WAY_AUTHENTICATION)
    /* clear warnings about unused parameters. */
    (void) client_cert;
    (void) client_key;

    if (root_ca != NULL && strlen(root_ca) != 0)
    {
        mbedtls_err = mbedtls_x509_crt_parse(&(tls_session->credentials.root_ca),
                                              (const unsigned char *)root_ca,
                                              strlen(root_ca) + 1);
        
        if (mbedtls_err != 0)
        {
            _log_mbedtls_error( mbedtls_err, "Failed to read root CA certificate file." );
            goto exit;
        }
        else
        {
            mbedtls_ssl_conf_ca_chain(&(tls_session->config ),
                                      &(tls_session->credentials.root_ca),
                                      NULL );
        }
    }

#elif defined(CTIOT_TLS_SECURITY_TWO_WAY_AUTHENTICATION)
    if (root_ca != NULL && strlen(root_ca) != 0 &&
        client_cert != NULL && strlen(client_cert) != 0 &&
        client_key != NULL && strlen(client_key) != 0)
    {
        mbedtls_err = mbedtls_x509_crt_parse(&(tls_session->credentials.root_ca),
                                              (const unsigned char *)root_ca,
                                              strlen(root_ca) + 1);

        if (mbedtls_err != 0)
        {
            _log_mbedtls_error( mbedtls_err, "Failed to read root CA certificate file." );
            goto exit;
        }
        else
        {
            mbedtls_ssl_conf_ca_chain(&(tls_session->config ),
                                      &(tls_session->credentials.root_ca),
                                      NULL );
        }

        mbedtls_err = mbedtls_x509_crt_parse(&(tls_session->credentials.client_cert),
                                             (const unsigned char *)client_cert,
                                             strlen(client_cert) + 1);

        if(mbedtls_err < 0)
        {
            _log_mbedtls_error(mbedtls_err, "Failed to read client certificate file.");
            goto exit;
        }

        mbedtls_err = mbedtls_pk_parse_key(&(tls_session->credentials.client_key),
                                           (const unsigned char *)client_key,
                                           strlen(client_key) + 1, 
                                           NULL,
                                           0);

        if (mbedtls_err != 0)
        {
            _log_mbedtls_error(mbedtls_err, "Failed to read client certificate private key file.");
            goto exit;
        }

        /* Set the client credential to the SSL context configuration. */
        mbedtls_err = mbedtls_ssl_conf_own_cert(&(tls_session->config),
                                                &(tls_session->credentials.client_cert),
                                                &(tls_session->credentials.client_key));

        if (mbedtls_err != 0)
        {
            _log_mbedtls_error( mbedtls_err, "Failed to configure credentials." );
            goto exit;
        }
    }
#endif

exit:

    return mbedtls_err;
}

static int tls_network_mbedtls_context(ctiot_tls_session_t *tls_session,
                                       const char *root_ca,
                                       const char *client_cert,
                                       const char *client_key,
                                       const char *client_pwd)
{
    int mbedtls_err = 0;

    mbedtls_err = mbedtls_ssl_config_defaults(&(tls_session->config),
                                              MBEDTLS_SSL_IS_CLIENT,
                                              MBEDTLS_SSL_TRANSPORT_STREAM,
                                              MBEDTLS_SSL_PRESET_DEFAULT);
    if (mbedtls_err != 0)
    {
        _log_mbedtls_error(mbedtls_err, "Failed to set default SSL configuration.");
        goto exit;
    }

    /* Set SSL authmode  */
#if defined(CTIOT_TLS_VERIFY_NONE)
    mbedtls_ssl_conf_authmode(&(tls_session->config), MBEDTLS_SSL_VERIFY_NONE);
#elif defined(CTIOT_TLS_VERIFY_REQUIRED)
    mbedtls_ssl_conf_authmode(&(tls_session->config), MBEDTLS_SSL_VERIFY_REQUIRED);
#elif defined(CTIOT_TLS_VERIFY_OPTIONAL)
    mbedtls_ssl_conf_authmode(&(tls_session->config), MBEDTLS_SSL_VERIFY_OPTIONAL);
#endif

    /* Set the RNG context */
    mbedtls_ssl_conf_rng(&(tls_session->config), mbedtls_ctr_drbg_random, &(tls_session->ctr_drbg));

    /* Set mbedtls debug */
#if defined(CTIOT_TLS_ENABLE_MBEDTLS_DEBUG)
    mbedtls_debug_set_threshold(4); /* 0: No debug; 1: Error; 2: State change; 3: Informational; 4: Verbose */
    mbedtls_ssl_conf_dbg(&tls_session->config, _log_mbedtls_debug, NULL);
#endif

    /* Setup TLS credentials. */
    mbedtls_err = read_tls_credentials(tls_session, root_ca, client_cert, client_key);
    if (mbedtls_err != 0)
    {
        CTIOT_LOG(LOG_ERR, "Failed to read credentials, ret=0x%x", mbedtls_err);
        goto exit;
    }

    /* Initialize the mbed TLS secured connection context. */
    mbedtls_err = mbedtls_ssl_setup(&(tls_session->context),
                                    &(tls_session->config));
    if (mbedtls_err != 0)
    {
        _log_mbedtls_error(mbedtls_err, "Failed to setup mbedtls ssl context.");
        goto exit;
    }

    /* Hostname set here should match CN in server certificate */
    mbedtls_err = mbedtls_ssl_set_hostname(&(tls_session->context),
                                           tls_session->host);
    if ( mbedtls_err != 0 )
    {
        _log_mbedtls_error(mbedtls_err, "Failed to set server name.");
        goto exit;
    }

    /* Set the underlying IO for the TLS connection. */
#if defined(CTIOT_TLS_RECV_BLOCK)
    /* Block until received data */
    mbedtls_ssl_set_bio(&(tls_session->context),
                        &(tls_session->server_fd),
                        mbedtls_net_send,
                        mbedtls_net_recv,
                        NULL);
#elif defined(CTIOT_TLS_RECV_BLOCK_WITH_TIMEOUT)
    /* Block with receive timeout */
    mbedtls_ssl_set_bio(&(tls_session->context),
                        &(tls_session->server_fd),
                        mbedtls_net_send,
                        NULL,
                        mbedtls_net_recv_timeout);

    mbedtls_ssl_conf_read_timeout(&(tls_session->config), 10000);
#endif

exit:

    return mbedtls_err;
}

static int tls_network_mbedtls_handshake(ctiot_tls_session_t *tls_session)
{
    int mbedtls_err = 0;
#ifndef CTIOT_TLS_VERIFY_NONE
    uint32_t verify_result = 0;
#endif

    /* Perform the TLS handshake. */
    CTIOT_LOG(LOG_INFO, "Start handshake tick:%lu", os_tick_get());

    do
    {
        mbedtls_err = mbedtls_ssl_handshake(&(tls_session->context));
    }while((mbedtls_err == MBEDTLS_ERR_SSL_WANT_READ) ||
           (mbedtls_err == MBEDTLS_ERR_SSL_WANT_WRITE));

    if (mbedtls_err != 0)
    {
        _log_mbedtls_error(mbedtls_err, "Failed to perform SSL handshake.");
        goto exit;
    }
    CTIOT_LOG(LOG_INFO, "Finish handshake tick:%lu", os_tick_get());

#ifndef CTIOT_TLS_VERIFY_NONE
    /* Check result of certificate verification. */
    verify_result = mbedtls_ssl_get_verify_result(&(tls_session->context));
    if (verify_result != 0)
    {
        CTIOT_LOG(LOG_ERR, "Failed to verify server certificate, result %u.", verify_result);
        char verify_info[128] = {0};
        mbedtls_x509_crt_verify_info(verify_info, 128, "  ! ", verify_result);
        CTIOT_LOG(LOG_ERR, "Server certificate verification info: %s", verify_info);

        return -1;
    }
    else
    {
        CTIOT_LOG(LOG_INFO, "Server certificate verified success.");
    }
#endif

exit:

    return mbedtls_err;
}

static int tls_setup(ctiot_tls_session_t *tls_session,
                       const char *root_ca,
                       const char *client_cert,
                       const char *client_key,
                       const char *client_pwd)
{
    int mbedtls_err = 0;

#ifdef OS_USING_SHELL
    CTIOT_LOG(LOG_INFO, "Memory usage before TLS connection is established:");
    sh_exec("show_mem");
#endif

    mbedtls_err = tls_network_mbedtls_init(tls_session);
    if (mbedtls_err != 0)
    {
        CTIOT_LOG(LOG_ERR, "Failed to init mbedtls library.");
        goto cleanup;
    }

    mbedtls_err = tls_network_mbedtls_context(tls_session, root_ca, client_cert, client_key, client_pwd);
    if (mbedtls_err != 0)
    {
        CTIOT_LOG(LOG_ERR, "Failed to set mbedtls context.");
        goto cleanup;
    }

    mbedtls_err = tls_network_mbedtls_handshake(tls_session);
    if (mbedtls_err != 0)
    {
        CTIOT_LOG(LOG_ERR, "Failed to finish mbedtls handshake.");
        goto cleanup;
    }

#ifdef OS_USING_SHELL
    CTIOT_LOG(LOG_INFO, "Memory usage after TLS connection is established:");
    sh_exec("show_mem");
#endif

    return 0;

cleanup:
    tls_network_mbedtls_cleanup(tls_session);

    return mbedtls_err;
}

/**
 ***********************************************************************************************************************
 * @brief       This function establish tls session
 *
 * @param[in]   tls_session_ptr     the pointer of  tls_session
 * @param[in]   addr                tls server address
 * @param[in]   port                server port
 * @param[in]   ca_crt              root CA certificate 
 * @param[in]   client_crt          client certificate
 * @param[in]   client_key          client key
 * @param[in]   client_pwd          client password, always NULL
 *
 * @return      Return status
 * @retval      0           success.
 * @retval      others      failed.
 ***********************************************************************************************************************
 */
int ctiot_tls_network_mbedtls_establish(uintptr_t  *tls_session_ptr,
                                        const char *addr,
                                        const char *port,
                                        const char *ca_crt,
                                        const char *client_crt,
                                        const char *client_key,
                                        const char *client_pwd)
{
    int ret = -1;
    int mbedtls_err = 0;
    ctiot_tls_session_t *tls_session = NULL;

    if (NULL==tls_session_ptr || NULL ==addr || NULL ==port || NULL ==ca_crt)
        return ret;

    tls_session = (ctiot_tls_session_t *)malloc(sizeof(ctiot_tls_session_t));
    if (tls_session == NULL)
    {
        CTIOT_LOG(LOG_ERR, "No memory for MbedTLS session object.");
        return ret;
    }
    memset(tls_session, 0x0, sizeof(ctiot_tls_session_t));

    tls_session->host = ctmq_strdup(addr);
    tls_session->port = ctmq_strdup(port);

    /* Initialize mbedtls net context. */
    mbedtls_net_init(&tls_session->server_fd);

    /* Establish a TCP connection first. */
    mbedtls_err = mbedtls_net_connect(&tls_session->server_fd, tls_session->host, 
                                      tls_session->port, MBEDTLS_NET_PROTO_TCP);
    if (mbedtls_err != 0)
    {   
        _log_mbedtls_error(mbedtls_err, "Mbedtls fail to establish tcp connection.");
        goto exit;
    }

    /* Set the mbed TLS network context to blocking mode. */
    /* mbedtls_err = mbedtls_net_set_block(&(tls_session->server_fd)); */ //because molink default is nonblock, and not has ioctlsocket

    mbedtls_err = tls_setup(tls_session, ca_crt, client_crt, client_key, client_pwd);
    if (mbedtls_err != 0)
    {
        CTIOT_LOG(LOG_ERR, "Mbedtls fail to establish tls connection, return -0x%x", -mbedtls_err);
        goto exit;
    }

    CTIOT_LOG(LOG_INFO, "MbedTLS connect successed ");

    *tls_session_ptr = (uintptr_t)tls_session;

    return 0;

exit:
    mbedtls_net_free(&tls_session->server_fd);

    if (tls_session->host)
    {
        free(tls_session->host);
    }

    if (tls_session->port)
    {
        free(tls_session->port);
    }

    if (tls_session)
    {
        free(tls_session);
    }

    CTIOT_LOG(LOG_ERR, "MbedTLS connection establish failed.");

    return ret;
}

int ctiot_tls_network_mbedtls_close(ctiot_tls_session_t **pass_tls_session)
{
    int ret = 0;
    //int mbedtls_err = 0;
    ctiot_tls_session_t *tls_session = *pass_tls_session;

    if (OS_NULL == tls_session)
    {
        return -1;
    }

    /* Notify the server that the SSL connection is being closed. */
    /*do
    {
        mbedtls_err = mbedtls_ssl_close_notify(&(tls_session->context));
    } while((mbedtls_err == MBEDTLS_ERR_SSL_WANT_READ) ||
            (mbedtls_err == MBEDTLS_ERR_SSL_WANT_WRITE));

    if(mbedtls_err != 0)
    {
        _log_mbedtls_error(mbedtls_err, "Failed to notify peer of SSL connection close.");
    }
    else
    {
        CTIOT_LOG(LOG_INFO, "TLS connection closed.");
    }*/
    mbedtls_ssl_close_notify(&(tls_session->context));

    /* Close the TLS connection and free resource. */
    mbedtls_net_free(&tls_session->server_fd);
    mbedtls_ssl_free(&tls_session->context);

#if defined(CTIOT_TLS_SECURITY_ONE_WAY_AUTHENTICATION)
    mbedtls_x509_crt_free(&(tls_session->credentials.root_ca));
#elif defined(CTIOT_TLS_SECURITY_TWO_WAY_AUTHENTICATION)
    mbedtls_pk_free(&(tls_session->credentials.client_key));
    mbedtls_x509_crt_free(&(tls_session->credentials.client_cert));
    mbedtls_x509_crt_free(&(tls_session->credentials.root_ca));
#endif
    mbedtls_ssl_config_free(&tls_session->config);

    mbedtls_entropy_free(&tls_session->entropy);
    mbedtls_ctr_drbg_free(&tls_session->ctr_drbg);

    if (tls_session->host)
    {
        free(tls_session->host);
    }

    if (tls_session->port)
    {
        free(tls_session->port);
    }

    if (tls_session)
    {   
        free(tls_session);
        tls_session = NULL;
    }

    CTIOT_LOG(LOG_INFO, "MbedTLS connection close success, mbedtls library cleanup done.");

    return ret;
}

int ctiot_tls_network_mbedtls_read(ctiot_tls_session_t *tls_session, unsigned char *buf, size_t len)
{
    int ret = 0;

    if (OS_NULL == tls_session ||  OS_NULL == buf)
    {
        return -1;
    }

    ret = mbedtls_ssl_read(&tls_session->context, (unsigned char *)buf, len);
    if (ret < 0
       && ret != MBEDTLS_ERR_SSL_WANT_READ
       && ret != MBEDTLS_ERR_SSL_WANT_WRITE
       && ret != MBEDTLS_ERR_SSL_TIMEOUT)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mbedtls read data error, return -0x%x", -ret);
    }

    return ret;
}

int ctiot_tls_network_mbedtls_write(ctiot_tls_session_t *tls_session, const unsigned char *buf , size_t len)
{
    int ret = 0;

    if (OS_NULL == tls_session || OS_NULL == buf)
    {
        return -1;
    }

    ret = mbedtls_ssl_write(&tls_session->context, (unsigned char *)buf, len);
    if (ret < 0 && ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mbedtls write data error, return -0x%x", -ret);
    }

    return ret;
}
