/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <host/ble_gap.h>
#include "nimble-console/console.h"
#include "services/ipss/ble_l2cap_ipsp.h"
#include "../../../src/ble_l2cap_coc_priv.h"

#define MODLOG_DFLT(ml_lvl_, ...) console_printf(__VA_ARGS__)

#define BLE_L2CAP_IPSP_PSM 0x0023
#define BLE_L2CAP_IPSP_MTU 256

#define BLE_L2CAP_IPSP_CLIENT_NUM_MAX   5

typedef enum
{
    L2CAP_INTF_SERVER = 0,
    L2CAP_INTF_CLIENT = 1,
} l2cap_intf_type_t;

typedef struct
{
    uint16_t conn_handle;
    struct ble_l2cap_chan *chan;    
} ble_l2cap_intf_client_t;

typedef struct
{
    int conn_nums;
    uint16_t conn_handle[BLE_L2CAP_IPSP_CLIENT_NUM_MAX];
    struct ble_l2cap_chan *chan[BLE_L2CAP_IPSP_CLIENT_NUM_MAX];    
} ble_l2cap_intf_server_t;

typedef struct
{
    l2cap_intf_type_t type;
    union
    {
        ble_l2cap_intf_client_t intf_client;
        ble_l2cap_intf_server_t intf_server;
    } l2cap_intf;
} ble_l2cap_ipsp_intf_t;

static ble_l2cap_ipsp_intf_t gs_ipsp_intf = {L2CAP_INTF_CLIENT, {0xFFFF, OS_NULL}};

void ble_l2cap_ipsp_recv_prepare(uint16_t mtu, struct ble_l2cap_chan *chan)
{
    struct os_mbuf *sdu_rx;

    int rc;

    console_printf("LE CoC accepting, chan: 0x%08lx, peer_mtu %d\r\n", (uint32_t) chan, mtu);

    sdu_rx = os_msys_get_pkthdr(mtu, 0);
    assert(sdu_rx != NULL);

    rc = ble_l2cap_recv_ready(chan, sdu_rx);
    assert(rc == 0);
}

static void ble_ll_ipsp_l2cap_update_event(uint16_t conn_handle, int status, void *arg)
{
    if (status == 0) {
        MODLOG_DFLT(INFO, "L2CAP params updated\n");
    } else {
        MODLOG_DFLT(INFO, "L2CAP params update failed; rc=%d\n", status);
        assert(0);
    }
}

static void ble_ll_util_hex_dump(char *data, int len)
{
    char buf[64];
    int index;

    MODLOG_DFLT(INFO, "dump data total len=%d\n", len);
    memset(buf, 0, sizeof(buf));
    for (index = 0; index < len; index++)
    {    
        sprintf(buf + 3 * (index % 16),"%02x ", data[index]);
        if (!((index + 1) % 16))
        {
            MODLOG_DFLT(INFO, "%s\n", buf);
            memset(buf, 0, sizeof(buf));
        }
    }

    if (index)
    {
        MODLOG_DFLT(INFO, "%s\n", buf);
    }

    return;
}

static void ble_ll_ipsp_update_intf_server(ble_l2cap_intf_server_t *l2cap_intf_server, struct ble_l2cap_event *event)
{
    int index;

    for (index = 0; index < BLE_L2CAP_IPSP_CLIENT_NUM_MAX; index++)
    {
        if (0xFFFF == l2cap_intf_server->conn_handle[index])
        {
            l2cap_intf_server->conn_handle[index] = event->connect.conn_handle;
            l2cap_intf_server->chan[index] = event->connect.chan;
            l2cap_intf_server->conn_nums++;
            break;
        }
    }

    return;
}

static void ble_ll_ipsp_delete_intf_server(ble_l2cap_intf_server_t *l2cap_intf_server, struct ble_l2cap_event *event)
{
    int index;

    for (index = 0; index < BLE_L2CAP_IPSP_CLIENT_NUM_MAX; index++)
    {
        if (event->connect.conn_handle == l2cap_intf_server->conn_handle[index])
        {
            l2cap_intf_server->conn_handle[index] = 0xFFFF;
            l2cap_intf_server->chan[index] = OS_NULL;
            l2cap_intf_server->conn_nums--;
            break;
        }
    }

    return;
}

static int ble_ll_ipsp_l2cap_event(struct ble_l2cap_event *event, void *arg)
{
    int rc;
    struct os_mbuf *data_buf;
    struct os_mbuf *rx_pdu;
    static int data_len = 1000;
    static int send_cnt = 0;
    static bool stalled = false;
    struct ble_l2cap_chan_info chan_info;
    ble_l2cap_intf_server_t *l2cap_intf_server;
    ble_l2cap_intf_client_t *l2cap_intf_client;

    switch (event->type) {
    case BLE_L2CAP_EVENT_COC_CONNECTED:
        if (event->connect.status) {
            MODLOG_DFLT(INFO, "LE COC error: %d\n", event->connect.status);
            return 0;
        }

        ble_l2cap_get_chan_info(event->connect.chan, &chan_info);

        MODLOG_DFLT(INFO,
                    "LE COC connected, conn: %d, chan: 0x%08lx, scid: 0x%04x, "
                    "dcid: 0x%04x, our_mtu: 0x%04x, peer_mtu: 0x%04x\n",
                    event->connect.conn_handle,
                    (uint32_t) event->connect.chan,
                    chan_info.scid,
                    chan_info.dcid,
                    chan_info.our_l2cap_mtu,
                    chan_info.peer_l2cap_mtu);

        struct ble_l2cap_sig_update_params params = {
            .itvl_min = 0x0006,//BLE_GAP_INITIAL_CONN_ITVL_MIN
            .itvl_max = 0x0006,//BLE_GAP_INITIAL_CONN_ITVL_MIN
            .slave_latency = 0x0000,
            .timeout_multiplier = 0x0100,
        };

        if (L2CAP_INTF_SERVER == gs_ipsp_intf.type)
        {
            MODLOG_DFLT(INFO, "AS l2cap server");
            l2cap_intf_server = &gs_ipsp_intf.l2cap_intf.intf_server;
            if (l2cap_intf_server->conn_nums >= BLE_L2CAP_IPSP_CLIENT_NUM_MAX)
            {
                //TODO: server to max, close
                MODLOG_DFLT(INFO, "Failed, L2CAP connect to max: %d\n", l2cap_intf_server->conn_nums);
                return 0;
            }
            ble_ll_ipsp_update_intf_server(l2cap_intf_server, event);
            rc = ble_l2cap_sig_update(event->connect.conn_handle, &params, ble_ll_ipsp_l2cap_update_event, NULL);
            assert(rc == 0);
        }
        else if (L2CAP_INTF_CLIENT == gs_ipsp_intf.type)
        {
            MODLOG_DFLT(INFO, "AS l2cap client");
            l2cap_intf_client = &gs_ipsp_intf.l2cap_intf.intf_client;
            l2cap_intf_client->chan = event->connect.chan;
            l2cap_intf_client->conn_handle = event->connect.conn_handle;

        }

        return 0;

    case BLE_L2CAP_EVENT_COC_DISCONNECTED:
        MODLOG_DFLT(INFO, "LE CoC disconnected, chan: 0x%08lx\n",
                    (uint32_t) event->disconnect.chan);

        if (L2CAP_INTF_SERVER == gs_ipsp_intf.type)
        {
            l2cap_intf_server = &gs_ipsp_intf.l2cap_intf.intf_server;
            ble_ll_ipsp_delete_intf_server(l2cap_intf_server, event);
        }
        else if (L2CAP_INTF_CLIENT == gs_ipsp_intf.type)
        {
            l2cap_intf_client = &gs_ipsp_intf.l2cap_intf.intf_client;
            l2cap_intf_client->chan = OS_NULL;
            l2cap_intf_client->conn_handle = 0xFFFF;
        }
        return 0;

    case BLE_L2CAP_EVENT_COC_ACCEPT:
        MODLOG_DFLT(INFO, "BLE_L2CAP_EVENT_COC_ACCEPT recv\r\n");
        //stress_l2cap_coc_accept(event->accept.peer_sdu_size, event->accept.chan);
        ble_l2cap_ipsp_recv_prepare(event->accept.peer_sdu_size, event->accept.chan);
        return 0;

    case BLE_L2CAP_EVENT_COC_DATA_RECEIVED:
        MODLOG_DFLT(INFO, "BLE_L2CAP_EVENT_COC_DATA_RECEIVED recv\r\n");
        struct os_mbuf_pkthdr *hdr;

        rx_pdu = event->receive.sdu_rx;
        hdr = OS_MBUF_PKTHDR(rx_pdu);
  
        MODLOG_DFLT(INFO, "L2CAP server received data; rx_pdu datalen=%d,over_len=%d\n", rx_pdu->om_len, hdr->omp_len);
        char *data;

        data = (char *)malloc(hdr->omp_len);
        if (OS_NULL == data)
        {
            break;
        }

        os_mbuf_copydata(rx_pdu, 0, hdr->omp_len, data);
        // recycle mbuf
        os_mbuf_free_chain(rx_pdu);

        ble_ll_util_hex_dump(data, hdr->omp_len);
        free(data);

        ble_l2cap_ipsp_recv_prepare(BLE_L2CAP_IPSP_MTU, event->receive.chan);
        break;

    case BLE_L2CAP_EVENT_COC_TX_UNSTALLED:
        MODLOG_DFLT(INFO, "L2CAP unstalled event\n");
        struct ble_l2cap_coc_endpoint *tx;
        struct ble_l2cap_chan *chan;
        chan = event->tx_unstalled.chan;

        //(void)chan->conn_handle;
        //(void)event->tx_unstalled.chan->coc_tx;
        //event->tx_unstalled.chan->coc_tx;
        //tx = event->tx_unstalled.chan->coc_tx;
        //TODO: exception deal
        stalled = false;
        return 0;

    default:
        MODLOG_DFLT(INFO, "Other L2CAP event occurs: %d\n", event->type);
        return 0;
    }
#if 0
//TODO:next
    /* Send pattern data */

    /* Get mbuf for adv data */
    data_buf = os_msys_get_pkthdr(data_len, 0);

    MODLOG_DFLT(INFO, "Data buf %s\n", data_buf ? "OK" : "NOK");
    assert(data_buf != NULL);

    /* The first 2 bytes of data is the size of appended pattern data. */
    rc = os_mbuf_append(data_buf, (uint8_t[]) {data_len >> 8, data_len},
                        2);
    if (rc) {
        os_mbuf_free_chain(data_buf);
        assert(0);
    }

    /* Fill mbuf with the pattern */
    stress_fill_mbuf_with_pattern(data_buf, data_len);

    /* Send data */
    rc = ble_l2cap_send(rx_stress_ctx->chan, data_buf);
    MODLOG_DFLT(INFO, "Return code=%d\n", rc);
    if (rc) {
        MODLOG_DFLT(INFO, "L2CAP stalled - waiting\n");
        stalled = true;
    }

    MODLOG_DFLT(INFO, " %d, %d\n", ++send_cnt, data_len);
    data_len += 500;
#endif
    return 0;
}

int ble_ll_ipsp_l2cap_create_server(void)
{
    ble_l2cap_intf_server_t *l2cap_intf_server;
    int index;
    int do_err;

    gs_ipsp_intf.type = L2CAP_INTF_SERVER;
    l2cap_intf_server = &gs_ipsp_intf.l2cap_intf.intf_server;

    l2cap_intf_server->conn_nums = 0;

    for (index = 0; index < BLE_L2CAP_IPSP_CLIENT_NUM_MAX; index++)
    {
        l2cap_intf_server->conn_handle[index] = 0xFFFF;
        l2cap_intf_server->chan[index] = OS_NULL;
    }

    do_err = ble_l2cap_coc_create_server(BLE_L2CAP_IPSP_PSM, BLE_L2CAP_IPSP_MTU, ble_ll_ipsp_l2cap_event, NULL);

    return do_err;
}

int ble_l2cap_ipsp_l2cap_connect(uint16_t conn_handle)
{
    ble_l2cap_intf_client_t *l2cap_intf_client;
    struct os_mbuf *sdu_rx;
    int rc;

    //assert(gs_ipsp_intf.type == L2CAP_INTF_CLIENT);
    sdu_rx = os_msys_get_pkthdr(BLE_L2CAP_IPSP_MTU, 0);
    assert(sdu_rx != NULL);

    l2cap_intf_client = &gs_ipsp_intf.l2cap_intf.intf_client;
    l2cap_intf_client->conn_handle = conn_handle;
    MODLOG_DFLT(INFO, "begin l2cap connect, handle=0x%04x\n" , conn_handle);
    rc = ble_l2cap_connect(conn_handle, BLE_L2CAP_IPSP_PSM, BLE_L2CAP_IPSP_MTU, sdu_rx, ble_ll_ipsp_l2cap_event, NULL);
    MODLOG_DFLT(INFO, "end l2cap connect, handle=0x%04x\n" , conn_handle);
    return rc;
}

int ble_l2cap_ipsp_send(void *data, uint16_t len)
{
    ble_l2cap_intf_server_t *l2cap_intf_server;
    struct os_mbuf *sdu_xmit;
    int rc;

    //assert(gs_ipsp_intf.type == L2CAP_INTF_SERVER);
    if (gs_ipsp_intf.type != L2CAP_INTF_SERVER)
    {
        MODLOG_DFLT(INFO, "not l2cap server\r\n");
        return -1; 
    }
    l2cap_intf_server = &gs_ipsp_intf.l2cap_intf.intf_server;

    if (l2cap_intf_server->conn_nums < 1 || OS_NULL == l2cap_intf_server->chan[0])
    {
        MODLOG_DFLT(INFO, "Now intf not up, please wait,%d,%p\r\n", l2cap_intf_server->conn_nums, l2cap_intf_server->chan[0]);
        return -1;
    }
    sdu_xmit = os_msys_get_pkthdr(len, 0);

    MODLOG_DFLT(INFO, "Data buf %s\r\n", sdu_xmit ? "OK" : "NOK");
    assert(sdu_xmit != NULL);

    os_mbuf_append(sdu_xmit, data, len);

	/* ble_l2cap_send takes ownership of the sdu */
	rc = ble_l2cap_send(l2cap_intf_server->chan[0], sdu_xmit);
    MODLOG_DFLT(INFO, "data send failed, rc=%d\r\n", rc);
    // others, do not need free buf
    if (BLE_HS_EBUSY == rc || BLE_HS_EBADDATA == rc || BLE_HS_ENOTSUP == rc)
    {
        os_mbuf_free_chain(sdu_xmit);
    }
    //if (0 == rc || BLE_HS_ESTALLED == rc)
    //{ 
    //}
    //os_mbuf_free_chain(sdu_xmit);

    return rc;
}

static os_err_t l2cap_send_func(os_int32_t argc, char **argv)
{
    int buflen;
    char *data;
    int index;
    int rc;

    buflen = 256;
    if (2 == argc)
    {
        buflen = atoi(argv[1]);
    }

    data = malloc(buflen);
    if (OS_NULL == data)
    {
        return 0;
    }

    for (index = 0; index < buflen; index++)
    {
        data[index] = index & 0xFF;
    }

    rc = ble_l2cap_ipsp_send(data, buflen);
    MODLOG_DFLT(INFO, "data send failed stat, rc=%d, buflen = %d\r\n", rc, buflen); 
    free(data);

    return 0;
}

extern struct os_mempool *os_msys_get_mempool(void);

static os_err_t mbuf_dump_func(os_int32_t argc, char **argv)
{
    struct os_mempool *mem;

    mem = os_msys_get_mempool();
    MODLOG_DFLT(INFO, "block_size=%d, num_blocks=%d, num_free=%d, min_free=%d\r\n", mem->mp_block_size, mem->mp_num_blocks, mem->mp_num_free, mem->mp_min_free);

    return 0;
}
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(mbuf_dump, mbuf_dump_func, "mbuf_dump");
SH_CMD_EXPORT(l2cap_send, l2cap_send_func, "l2cap send data");
#endif