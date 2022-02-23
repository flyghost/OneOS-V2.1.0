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
 * @file        onetls_tls_client_test.c
 *
 * @brief       onetls_tls_client_test functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <sys/socket.h>
#include <onetls_lib.h>

/* Target server params */
#define TEST_TARGET_ADDR    "172.20.227.220"
#define TEST_TARGET_PORT    1234

/* PSK hint(or PSK Identity) */
// static uint8_t test_psk_hint[] = {0x74, 0x65, 0x73, 0x74};
// static uint8_t test_psk_key[] = {0xaa, 0xbb, 0xcc, 0xdd};

static uint8_t test_psk_hint[] = "Client_identity";
static uint8_t test_psk_key[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 
                                 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                                 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                                 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

/* Test application data */
static uint8_t test_app_data[] = "Hello server!\r\n";

/* New session ticket */
static uint8_t test_ticket[512];
static uint16_t test_ticket_len;

/* resumption */
static uint8_t test_resumption;

static uint32_t onetls_get_psk(const uint8_t *hint, uint32_t hint_len, uint8_t *key, uint32_t max_key_len, uint32_t *key_len)
{
    if (max_key_len < sizeof(test_psk_key)) {
        return ONETLS_FAIL;
    }

    memcpy(key, test_psk_key, sizeof(test_psk_key));
    *key_len = sizeof(test_psk_key);
    return ONETLS_SUCCESS;
}

static uint32_t onetls_recv_new_session_ticket(onetls_ctx *ctx, uint8_t *ticket, uint32_t len)
{
    (void)ctx;
    memcpy(test_ticket, ticket, len);
    test_ticket_len = len;

    os_kprintf("Recv new session ticket len=%d\r\n", len);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           onetls_tls_client_nst_test
 *
 * @param[in]       none
 *
 * @return          -1: failed
 *                  0: success
 ***********************************************************************************************************************
 */
int onetls_tls_client_nst_test(void)
{
    int ret = 0;
    int sock = 0;
    struct sockaddr_in serv_addr;
    onetls_ctx *ctx = NULL;
    uint8_t buf[64] = {0};
    uint32_t send_len = 0;
    uint32_t recv_len = 0;

    test_resumption = 0;

    /* Echo OneTLS version  */
    os_kprintf("%s\r\n", onetls_version());

resumption:
    /* Create a socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        os_kprintf("Create a socket failed\r\n");
        return -1;
    }
    os_kprintf("Create a socket sucessfully\r\n");

    /* Fill in the server address */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TEST_TARGET_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(TEST_TARGET_ADDR);

    /* Connect to the server */
    ret = connect(sock, (void*)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        os_kprintf("Connect to server failed, ret=%d\r\n", ret);
        return -1;        
    }
    os_kprintf("Connect to server sucessfully\r\n");

    /* Create a OneTLS context */
    ctx = onetls_new_ctx(ONETLS_MODE_TLS);
    if (ctx == NULL) {
        os_kprintf("Create a OneTLS context failed, ret=%d\r\n", ret);
        return -1;
    }
    os_kprintf("Create a OneTLS context sucessfully\r\n");

    /* Attach context to the socket */
    onetls_set_socket(ctx, sock, sock);

    /* Config psk hint */
    onetls_set_psk_callback(ctx, onetls_get_psk);
    ret = onetls_set_outband_psk_hint(ctx, test_psk_hint, sizeof(test_psk_hint) - 1);
    if (ret != 0) {
        os_kprintf("Config psk hint failed, ret=%d\r\n", ret);
        return -1;
    }

    /* Config recv NST callback */
    onetls_set_nst_callback(ctx, onetls_recv_new_session_ticket);

    /* Set new session ticket */
    if (test_resumption) {
        onetls_set_ticket(ctx, test_ticket, test_ticket_len);
    }

    /* Handshake with server side */
    while ((ret = onetls_connect(ctx)) != 0) {
        /* Only use in non-blocking socket */
        if ( ret != ONETLS_SOCKET_TRYAGAIN) {
            os_kprintf("Handshake with server failed, ret=%d\r\n", ret);
            goto cleanup;
        }
    }
    os_kprintf("Handshake with server sucessfully\r\n");

    /* Send and receive application data */
    ret = onetls_send(ctx, test_app_data, sizeof(test_app_data), &send_len);
    if (ret != 0) {
        os_kprintf("Send data to server failed, ret=%d\r\n", ret);
        goto cleanup;
    }
    os_kprintf("Send data to server sucessfully\r\n");

    do {
        ret = onetls_recv(ctx, buf, sizeof(buf), &recv_len);
    } while (ret == ONETLS_SOCKET_TRYAGAIN);
    if (ret != 0) {
        os_kprintf("Recv data from server failed, ret=%d\r\n", ret);
        goto cleanup;
    }
    os_kprintf("Recv data from server sucessfully\r\n");
    if (recv_len) {
        os_kprintf("Recv data from server[%d]:\r\n", recv_len);
        os_kprintf("%s\r\n", buf);
    }
    os_kprintf("Close the connection ...done\r\n"); 

    /* Delay 3s, then shutdown ctx */
    if (test_resumption == 0) {
        os_kprintf("Delay a few time to shutdown the connect, then test resumption\r\n");
        os_task_msleep(3000);
        onetls_shutdown(ctx);
        onetls_del_ctx(ctx);
        closesocket(sock);
        test_resumption = 1;
        os_task_msleep(100);
        goto resumption;
    }

cleanup:
    /* Shutdown the connect  */
    onetls_shutdown(ctx);

    /* Delete context */
    onetls_del_ctx(ctx);

    /* Close the socket */
    closesocket(sock);

    return ret;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(tls_resume_test, onetls_tls_client_nst_test, "start onetls tls1.3 client resume test");
#endif
