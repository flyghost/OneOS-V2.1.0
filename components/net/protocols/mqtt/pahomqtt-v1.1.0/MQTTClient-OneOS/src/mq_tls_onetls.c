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
 * @file        mq_tls_onetls.c
 *
 * @brief       mq_tls_onetls functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-27   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <mq_tls_onetls.h>
#include <dlog.h>
#include <sys/socket.h>

#define MQ_TLS_TAG   "MQ_ONETLS"

static int tcp_client_setup(const char *host, uint16_t port)
{
    int ret = 0;
    int sock = 0;
    struct sockaddr_in serv_addr;

    /* Create a socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_E(MQ_TLS_TAG, "Create a socket failed.");
        return -1;
    }

    /* Fill in the server address */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(host);

    /* Connect to the server */
    ret = connect(sock, (void*)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        LOG_E(MQ_TLS_TAG, "Connect to server failed.");
        return -1;        
    }

    return sock;
}

/**
 ***********************************************************************************************************************
 * @brief           Establish a tls session
 *
 * @param[in]       addr: a pointer to sever address
 * @param[in]       port: server port
 * @param[in]       psk_identity: a pointer to psk identity
 * @param[in]       psk_identity_len: psk_ identity length
 * @param[in]       cb: client psk callback
 *
 * @return          mq_tls_session_t: tls session
 ***********************************************************************************************************************
 */
mq_tls_session_t *mq_tls_network_onetls_establish(const char *addr, uint16_t port, const uint8_t *psk_identity, 
                                                uint16_t psk_identity_len, onetls_psk_callback psk_cb)
{
    int ret = 0;
    mq_tls_session_t *session = NULL;

    if (addr == NULL) {
        return NULL;
    }

    session = (mq_tls_session_t *)malloc(sizeof(mq_tls_session_t));
    memset(session, 0x0, sizeof(mq_tls_session_t));

    /* Setup a tcp client */
    session->sock = tcp_client_setup(addr, port);
    if (session->sock < 0) {
        goto cleanup;
    }
    LOG_I(MQ_TLS_TAG, "Tcp client setup sucessfully.");

    /* Create a OneTLS context */
    session->ctx = onetls_new_ctx(ONETLS_MODE_TLS);
    if (session->ctx == NULL) {
        LOG_E(MQ_TLS_TAG, "Create a OneTLS context failed.");
        goto cleanup;
    }
    LOG_I(MQ_TLS_TAG, "Create a OneTLS context sucessfully.");

    /* Attach context to the socket */
    onetls_set_socket(session->ctx, session->sock, session->sock);

    /* Config psk hint */
    onetls_set_psk_callback(session->ctx, psk_cb);
    ret = onetls_set_outband_psk_hint(session->ctx, (uint8_t*)psk_identity, psk_identity_len);
    if (ret != 0) {
        LOG_E(MQ_TLS_TAG, "Config psk hint failed.");
        goto cleanup;
    }

    /* Handshake with server side */
    while ((ret = onetls_connect(session->ctx)) != 0) {
        /* Only use in non-blocking socket */
        if ( ret != ONETLS_SOCKET_TRYAGAIN) {
            LOG_E(MQ_TLS_TAG, "Handshake with server failed, ret=%d", ret);
            goto cleanup;
        }
    }
    LOG_I(MQ_TLS_TAG, "Handshake with server sucessfully.");

    return session;
cleanup:
    if (session->ctx != NULL) {
        /* Shutdown the connect  */
        onetls_shutdown(session->ctx);
        /* Delete context */
        onetls_del_ctx(session->ctx);   
        session->ctx = NULL;
    }

    /* Close the socket */
    closesocket(session->sock);

    /* Free session */
    free(session);
    session = NULL;

    return NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Close the tls session
 *
 * @param[in]       session: a pointer to the tls session
 *
 * @return          0: success, <0: failure
 ***********************************************************************************************************************
 */
int mq_tls_network_onetls_close(mq_tls_session_t *session)
{
    if (session == NULL) {
        return -1;
    }

    if (session->ctx == NULL) {
        return -1;
    }

    /* Shutdown the connect  */
    onetls_shutdown(session->ctx);

    /* Delete context */
    onetls_del_ctx(session->ctx);   
    session->ctx = NULL;

    /* Close the socket */
    closesocket(session->sock);

    /* Free session */
    free(session);
    session = NULL;

    return 0;    
}

/**
 ***********************************************************************************************************************
 * @brief           Read data from the tls session
 *
 * @param[in]       session: a pointer to the tls session
 * @param[in]       buf: a pointer to the read buffer
 * @param[in]       len: the read buffer length
 *
 * @return          >0: success, readed length
 *                  =0: read timeout
 *                  <0: read failed
 ***********************************************************************************************************************
 */
int mq_tls_network_onetls_read(mq_tls_session_t *tls_session, unsigned char *buf, size_t len)
{
    int ret = 0;
    size_t recv_len = 0;

    if (NULL == tls_session ||  NULL == buf) {
        return -1;
    }

    LOG_D(MQ_TLS_TAG, "Read data from the tls session\r\n");
    ret = onetls_recv(tls_session->ctx, buf, len, &recv_len);  

    switch (ret) {
    case ONETLS_SUCCESS:
        ret = recv_len;
        break;
    case ONETLS_SOCKET_TRYAGAIN:
        ret = -2;
        LOG_D(MQ_TLS_TAG, "Read data from the tls session tryagain\r\n");
        break;
    case ONETLS_SOCKET_TIMEOUT:
        ret = -3;
        LOG_D(MQ_TLS_TAG, "Read data from the tls session timeout\r\n");
        break;
    default:
        LOG_E(MQ_TLS_TAG, "OneTLS read data error, return %d", ret);
        ret = -1;
        break;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Write data to the tls session
 *
 * @param[in]       session: a pointer to the tls session
 * @param[in]       buf: a pointer to the write buffer
 * @param[in]       len: the write buffer length
 *
 * @return          >0: success, written length
 *                  <0: write failed
 ***********************************************************************************************************************
 */
int mq_tls_network_onetls_write(mq_tls_session_t *tls_session, const unsigned char *buf , size_t len)
{
    int ret = 0;

    size_t send_len = 0;

    if (NULL == tls_session || NULL == buf) {
        return -1;
    }

    LOG_D(MQ_TLS_TAG, "Write data to tls session\r\n");
    ret = onetls_send(tls_session->ctx, buf, len, &send_len);
    if (ret != ONETLS_SUCCESS && ret != ONETLS_SOCKET_TRYAGAIN) {
        LOG_E(MQ_TLS_TAG, "OneTLS write data error, return %d", ret);
        return -1;
    }

    return send_len;
}
