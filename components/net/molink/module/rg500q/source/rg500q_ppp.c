/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        rg500q_ppp.c
 *
 * @brief       rg500q module link kit ppp api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drv_cfg.h>
#include <os_task.h>

#include "rg500q.h"
#include "rg500q_ppp.h"

#include <lwip/err.h>
#include "lwip/dns.h"

#include "ppp/ppp.h"
#include "ppp/pppapi.h"

#ifdef RG500Q_USING_PPP_OPS

#define MO_LOG_TAG "rg500q_ppp"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define RG500Q_EVENT_PPP_SUCCESS   (1L << 0)
#define RG500Q_EVENT_PPP_FAILED    (1L << 1)

#define RG500Q_PPP_NAME            "rg500q_ppp"
#define RG500Q_PPP_RX_TASK_NAME    "PPP_rx_thread"
#define RG500Q_PPP_RECONNECT_SEC   (30)
#define RG500Q_MAX_MAILS           (20)
#define RG500Q_REG_RETRY           (5)
#define RG500Q_REG_RETRY_DELAY_MS  (5000)


typedef os_err_t (* rg500q_ppp_func_t)  (mo_object_t *module);

typedef enum rg500q_err
{
    RG500Q_PPP_INIT = 0,
    RG500Q_PPP_DIAL,
    RG500Q_PPP_DEV_INIT,
    RG500Q_PPP_TASK_CREATE,
    RG500Q_PPP_LWIP_SETUP,
    RG500Q_PPP_FINISHED,
} rg500q_err_t;

typedef struct rg500q_act_map
{
    rg500q_err_t      state;
    rg500q_ppp_func_t do_func;
    rg500q_ppp_func_t err_func;
} rg500q_act_map_t;

extern os_err_t at_parser_exec_unlock(at_parser_t *parser);
extern os_err_t at_parser_unbind_device(at_parser_t *parser);
extern os_err_t at_parser_rebind_device(at_parser_t *parser);

os_err_t rg500q_ppp_init(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    os_err_t     result = OS_EOK;
    at_parser_t *parser = &module->parser;
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);
    os_int32_t   retry  = RG500Q_REG_RETRY;
    eps_reg_info_t eps_info = {0};
    t5g_reg_info_t t5g_info = {0};

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    rg500q->ppp_evt   = os_event_create(RG500Q_PPP_NAME);
    rg500q->ppp_mb    = os_mb_create(RG500Q_PPP_NAME, RG500Q_MAX_MAILS);
    rg500q->ppp_netif = os_calloc(1, sizeof(struct netif));

    if (OS_NULL == rg500q->ppp_evt || OS_NULL == rg500q->ppp_mb || OS_NULL == rg500q->ppp_netif)
    {
        ERROR("%s no enough memory.", __func__);
        goto __exit;
    }

    result = rg500q_set_cfun(module, 0);
    if (OS_EOK != result)
    {
        ERROR("%s Module set cfun 0 failed.", __func__);
        goto __exit;
    }

    result = rg500q_set_cfun(module, 1);
    if (OS_EOK != result)
    {
        ERROR("%s Module set cfun 1 failed.", __func__);
        goto __exit;
    }

    /* 1: Registered, home network; 5: Registered, roaming */
    for (; retry > 0; retry--)
    {
        result = rg500q_get_reg(module, &eps_info);
        if (OS_EOK == result)
        {
            if ((1 == eps_info.reg_stat || 5 == eps_info.reg_stat))
            break;
        }

        result = rg500q_get_5g_reg(module, &t5g_info);
        if (OS_EOK == result)
        {
            if ((1 == t5g_info.reg_stat || 5 == t5g_info.reg_stat))
            break;
        }

        os_task_msleep(RG500Q_REG_RETRY_DELAY_MS);
    }

    if (0 == retry)
    {
        ERROR("%s Module has not registered yet.", __func__);
        result = OS_ERROR;
        goto __exit;
    }
    
    /* Set PDP context */
    memset(resp_buff, 0, sizeof(resp_buff));

#if 0
    /* Private APN has not been supported yet. */
    result = at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IP\",\"%s\"", APN);
#else
    result = at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IP\"");
#endif

    if (OS_EOK != result)
    {
        ERROR("%s set apn failed!", __func__);
        goto __exit;
    }
    else DEBUG("%s set apn success", __func__);

__exit:
    if (result != OS_EOK)
    {
        ERROR("RG500Q ppp init failed.");

        if (OS_NULL != rg500q->ppp_evt)
        {
            os_event_destroy(rg500q->ppp_evt);
            rg500q->ppp_evt = OS_NULL;
        }

        if (OS_NULL != rg500q->ppp_mb)
        {
            os_mb_destroy(rg500q->ppp_mb);
            rg500q->ppp_mb = OS_NULL;
        }

        if (OS_NULL != rg500q->ppp_netif)
        {
            os_free(rg500q->ppp_netif);
            rg500q->ppp_netif = OS_NULL;
        }
    }

    return result;
}

static os_err_t rg500q_ppp_deinit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    if (OS_NULL != rg500q->ppp_evt)
    {
        os_free(rg500q->ppp_evt);
        rg500q->ppp_evt = OS_NULL;
    }

    if (OS_NULL != rg500q->ppp_mb)
    {
        os_free(rg500q->ppp_mb);
        rg500q->ppp_mb = OS_NULL;
    }

    return OS_EOK;
}

os_err_t rg500q_ppp_dial(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;
    os_uint32_t  event  = 0;
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    OS_ASSERT(OS_NULL != rg500q->ppp_evt);

    os_event_clear(rg500q->ppp_evt, RG500Q_EVENT_PPP_SUCCESS | RG500Q_EVENT_PPP_FAILED);

    at_parser_exec_lock(parser);
    at_parser_send(parser, "ATD*99#\r\n", 9);
    at_parser_exec_unlock(parser);

    result = os_event_recv(rg500q->ppp_evt,
                           RG500Q_EVENT_PPP_SUCCESS | RG500Q_EVENT_PPP_FAILED,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           10 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s get ppp dialing result failed!", module->name);
        goto __exit;
    }

    if (event & RG500Q_EVENT_PPP_FAILED)
    {
        ERROR("Module %s PPP dialing failed!", module->name);
        result = OS_ERROR;
        goto __exit;
    }

    /* unbind device for ppp using */
    result = at_parser_unbind_device(parser);
    if (OS_EOK != result)
    {
        ERROR("%s at unbind device failed!", module->name);
        goto __exit;
    }

__exit:
    if (OS_EOK != result)
    {
        INFO("%s failed!", __func__);
    }
    else
    {
        INFO("%s Into PPP data mode!", module->name);
    }
    
    return result;
}

os_err_t rg500q_ppp_exit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);
    
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &module->parser;

    /* rebind device for ppp using */
    result = at_parser_rebind_device(parser);
    if (OS_EOK != result)
    {
        ERROR("%s at rebind device failed!", __func__);
        goto __exit;
    }
    
    /* exit ppp, do not send anything to device in exit process */

    /* 1) Do not input any character within 1s or longer before inputting “+++”. */
    os_task_msleep(RG500Q_PPP_EXIT_DELAY_MS);
    
    /* 2) Input “+++” within 1s, and no other characters can be inputted during the time. */
    os_device_write_nonblock(parser->device, 0, "+++", 3);
    
    /* 3) Do not input any character within 1s after “+++” has been inputted. */
    os_task_msleep(RG500Q_PPP_EXIT_DELAY_MS);

    /* unlock at_parser_exec lock */
    at_parser_exec_unlock(parser);
    
    /* test result by at cmd */
    for (int i = 0; i < 5; i++)
    {
        result = rg500q_at_test(module);
        if (OS_EOK == result) break;
    }
    
__exit:

#ifdef RG500Q_SUPPORT_DESTROY
    if (OS_EOK != result)
    {
        ERROR("%s ppp exit failed", __func__);
    }
    else 
    {
        INFO("%s ppp exit success, into command mode.", __func__);
    }
#endif

    OS_ASSERT_EX(OS_EOK == result, "%s ppp exit failed", __func__);
    
    return result;
}

static void rg500q_ppp_success_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    OS_ASSERT(OS_NULL != rg500q->ppp_evt);

    DEBUG("PPP CONNECT");
    os_event_send(rg500q->ppp_evt, RG500Q_EVENT_PPP_SUCCESS);
}

static void rg500q_ppp_failed_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    OS_ASSERT(OS_NULL != rg500q->ppp_evt);

    DEBUG("PPP NO CARRIER");
    os_event_send(rg500q->ppp_evt, RG500Q_EVENT_PPP_FAILED);
}

static at_urc_t rg500q_ppp_urc_table[] = {
    { .prefix = "CONNECT",    .suffix = "\r\n", .func = rg500q_ppp_success_func },
    { .prefix = "NO CARRIER", .suffix = "\r\n", .func = rg500q_ppp_failed_func  },
};

void rg500q_urc_register(mo_rg500q_t *module)
{
    /* Set ppp urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, rg500q_ppp_urc_table, sizeof(rg500q_ppp_urc_table) / sizeof(at_urc_t));
    return;
}

static void rg500q_lwip_status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
    struct netif *pppif = ppp_netif(pcb);
    LWIP_UNUSED_ARG(ctx);

    switch (err_code)
    {
        case PPPERR_NONE:
        {
#if LWIP_DNS
            const ip_addr_t *ns;
#endif /* LWIP_DNS */
            DEBUG("PPP status_cb: Connected");
#if PPP_IPV4_SUPPORT
            DEBUG("   our_ipaddr  = %s", ipaddr_ntoa(&pppif->ip_addr));
            DEBUG("   his_ipaddr  = %s", ipaddr_ntoa(&pppif->gw));
            DEBUG("   netmask     = %s", ipaddr_ntoa(&pppif->netmask));
#if LWIP_DNS
            ns = dns_getserver(0);
            DEBUG("   dns1        = %s", ipaddr_ntoa(ns));
            ns = dns_getserver(1);
            DEBUG("   dns2        = %s", ipaddr_ntoa(ns));
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
            DEBUG("   our6_ipaddr = %s", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */
            return;
        }
        case PPPERR_PARAM:
        {
            DEBUG("PPP status_cb: Invalid parameter");
            break;
        }
        case PPPERR_OPEN:
        {
            DEBUG("PPP status_cb: Unable to open PPP session");
            break;
        }
        case PPPERR_DEVICE:
        {
            DEBUG("PPP status_cb: Invalid I/O device for PPP");
            break;
        }
        case PPPERR_ALLOC:
        {
            DEBUG("PPP status_cb: Unable to allocate resources");
            break;
        }
        case PPPERR_USER:
        {
            DEBUG("PPP status_cb: User interrupt");
            return;
        }
        case PPPERR_CONNECT:
        {
            DEBUG("PPP status_cb: Connection lost");
            break;
        }
        case PPPERR_AUTHFAIL:
        {
            DEBUG("PPP status_cb: Failed authentication challenge");
            break;
        }
        case PPPERR_PROTOCOL:
        {
            DEBUG("PPP status_cb: Failed to meet protocol");
            break;
        }
        case PPPERR_PEERDEAD:
        {
            DEBUG("PPP status_cb: Connection timeout");
            break;
        }
        case PPPERR_IDLETIMEOUT:
        {
            DEBUG("PPP status_cb: Idle Timeout");
            break;
        }
        case PPPERR_CONNECTTIME:
        {
            DEBUG("PPP status_cb: Max connect time reached");
            break;
        }
        case PPPERR_LOOPBACK:
        {
            DEBUG("PPP status_cb: Loopback detected");
            break;
        }
        default:
        {
            DEBUG("PPP status_cb: Unknown error code %d", err_code);
            break;
        }
    }

    /*
     * Try to reconnect in 30 seconds, if you need a modem chatscript you have
     * to do a much better signaling here ;-)
     */
    ppp_connect(pcb, RG500Q_PPP_RECONNECT_SEC);
}

static os_uint32_t rg500q_lwip_output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
    OS_ASSERT(OS_NULL != data);
    OS_ASSERT(OS_NULL != ctx);

    return os_device_write_nonblock((os_device_t *)ctx, 0, data, len);
}

static os_err_t rg500q_ppp_rx_indicate(os_device_t *dev, struct os_device_cb_info *info)
{
    OS_ASSERT(OS_NULL != info);
    OS_ASSERT(OS_NULL != info->data);

    os_err_t result = os_mb_send(info->data, (os_uint32_t)info->size, OS_NO_WAIT);
    if (OS_EOK != result)
    {
        ERROR("Too many unsolved data.");
    }
    return result;
}

static void rg500q_ppp_rx_thread(void *parameter)
{
    OS_ASSERT(OS_NULL != parameter);

    int            rx_cnt  = 0;
    os_ubase_t     size    = 0;
    unsigned char *rx_buff = os_calloc(1, OS_SERIAL_RX_BUFSZ);
    mo_object_t   *module  = (mo_object_t *)parameter;
    mo_rg500q_t   *rg500q  = os_container_of(module, mo_rg500q_t, parent);
    os_device_t   *device  = module->parser.device;
    
    OS_ASSERT(OS_NULL != rx_buff);

    while (1)
    {
        if (os_mb_recv(rg500q->ppp_mb, &size, OS_TICK_PER_SECOND) != OS_EOK)
            continue;

        while (size)
        {
            rx_cnt = os_device_read_block(device, 0, rx_buff, min(OS_USBH_CDC_RX_BUFSZ, size));
            if (rx_cnt == 0)
                break;

            pppos_input_tcpip((ppp_pcb *)(rg500q->ppp_ctlblock), rx_buff, rx_cnt);
            size -= rx_cnt;
        }

        size = 0;
    }
}

static os_err_t rg500q_lwip_ppp_device_init(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != module->parser.device);

    os_err_t     result = os_device_open(module->parser.device);
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    if (OS_EOK == result)
    {
        struct os_device_cb_info cb_info = {
            .type = OS_DEVICE_CB_TYPE_RX,
            .data = (void *)rg500q->ppp_mb,
            .cb   = rg500q_ppp_rx_indicate,
        };

        result = os_device_control(module->parser.device, OS_DEVICE_CTRL_SET_CB, &cb_info);
        if (OS_EOK != result)
        {
            os_device_close(module->parser.device);
            ERROR("%s-%d, Set ppp device receive indicate failed.", __func__, __LINE__);
        }
    }

    return result;
}

static os_err_t rg500q_lwip_ppp_device_deinit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != module->parser.device);

    os_device_close(module->parser.device);
    return OS_EOK;
}

static os_err_t rg500q_ppp_rx_task_create(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    os_err_t     result = OS_EOK;
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    rg500q->rx_task = os_task_create(RG500Q_PPP_RX_TASK_NAME, rg500q_ppp_rx_thread, module, 2048, 8);

    if (OS_NULL != rg500q->rx_task)
    {
        os_task_startup(rg500q->rx_task);
        INFO("%s-%d, PPP RX task create OK.", __func__, __LINE__);
    }
    else
    {
        ERROR("PPP rx task create failed.");
        result = OS_ERROR;
    }

    return result;
}

static os_err_t rg500q_ppp_rx_task_destroy(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    os_err_t     result = OS_EOK;
    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);

    OS_ASSERT(OS_NULL != rg500q->rx_task);

    if (OS_NULL != rg500q->rx_task)
    {
        result = os_task_destroy(rg500q->rx_task);
    }

    OS_ASSERT(OS_EOK == result);
    rg500q->rx_task = OS_NULL;

    return result;
}

static os_err_t rg500q_lwip_ppp_setup(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_rg500q_t *rg500q  = os_container_of(module, mo_rg500q_t, parent);
    os_device_t *device  = module->parser.device;
    u16_t        holdoff = 0;

    OS_ASSERT(OS_NULL != device);
    OS_ASSERT(OS_NULL != rg500q->ppp_netif);

    /* Start PPP */
    rg500q->ppp_ctlblock = (void *)pppos_create((struct netif *)(rg500q->ppp_netif), rg500q_lwip_output_cb, rg500q_lwip_status_cb, (void *)device);

    OS_ASSERT(OS_NULL != rg500q->ppp_ctlblock);

    /* Set this interface as default route */
    ppp_set_default(((ppp_pcb *)(rg500q->ppp_ctlblock)));

    /*
    * Basic PPP client configuration. Can only be set if PPP session is in the
    * dead state (i.e. disconnected). We don't need to provide thread-safe
    * equivalents through PPPAPI because those helpers are only changing
    * structure members while session is inactive for lwIP core. Configuration
    * only need to be done once.
    */

    /* Ask the peer for up to 2 DNS server addresses. */
    ppp_set_usepeerdns(((ppp_pcb *)(rg500q->ppp_ctlblock)), 1);

    /* Auth configuration, this is pretty self-explanatory */

    ppp_set_auth((ppp_pcb *)(rg500q->ppp_ctlblock), PPPAUTHTYPE_NONE, OS_NULL, OS_NULL);

    /*
    * Initiate PPP negotiation, without waiting (holdoff=0), can only be called
    * if PPP session is in the dead state (i.e. disconnected).
    */
    ppp_connect((ppp_pcb *)(rg500q->ppp_ctlblock), holdoff);

    return OS_EOK;
}

#if 0
static os_err_t rg500q_lwip_ppp_exit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_rg500q_t *rg500q = os_container_of(module, mo_rg500q_t, parent);
    os_int8_t result = OS_EOK;

    result = ppp_close((ppp_pcb *)(rg500q->ppp_ctlblock), 0);
    OS_ASSERT(0 == result);

    result = ppp_free((ppp_pcb *)(rg500q->ppp_ctlblock));
    rg500q->ppp_ctlblock = OS_NULL;
    OS_ASSERT(0 == result);

    return OS_EOK;
}
#endif

static const rg500q_act_map_t act_map[] = 
{
    { RG500Q_PPP_INIT        , rg500q_ppp_init             , OS_NULL                       },
    { RG500Q_PPP_DIAL        , rg500q_ppp_dial             , rg500q_ppp_deinit             },
    { RG500Q_PPP_DEV_INIT    , rg500q_lwip_ppp_device_init , rg500q_ppp_exit               },
    { RG500Q_PPP_TASK_CREATE , rg500q_ppp_rx_task_create   , rg500q_lwip_ppp_device_deinit },
    { RG500Q_PPP_LWIP_SETUP  , rg500q_lwip_ppp_setup       , rg500q_ppp_rx_task_destroy    },
    { RG500Q_PPP_FINISHED    , OS_NULL                     , OS_NULL                       },
};

os_int32_t rg500q_init_do_act(mo_object_t *module)
{
    os_err_t   result     = OS_EOK;
    os_int32_t init_index = 0;
    
    do
    {
        OS_ASSERT(OS_NULL != act_map[init_index].do_func);

        if (OS_NULL != act_map[init_index].do_func)
        {
            result = act_map[init_index].do_func(module);
        }
 
        if (OS_EOK != result) break;

    } while (++init_index < RG500Q_PPP_FINISHED);
    
    return init_index;
}

void rg500q_init_err_handler(os_int32_t state, mo_object_t *module)
{
    os_int32_t init_index = state;
    
    /* notice: state == 0 */
    while(0 != init_index)
    {   
        if (OS_NULL != act_map[init_index].err_func)
        {
            act_map[init_index].err_func(module);
        }
        --init_index;
    };
}

os_err_t rg500q_ppp_startup(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    os_err_t   result       = OS_EOK;
    os_int32_t finish_state = RG500Q_PPP_INIT;

    finish_state = rg500q_init_do_act(module);
    if (RG500Q_PPP_FINISHED != finish_state)
    {
        ERROR("%s PPP failed to start, do recycle.", __func__);
        result = OS_ERROR;
        rg500q_init_err_handler(finish_state, module);
    }
    else
    {
        INFO("%s PPP start success.", __func__);
    }

    return result;
}

#ifdef RG500Q_SUPPORT_DESTROY
os_err_t rg500q_ppp_shutdown(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    rg500q_ppp_rx_task_destroy(module);
    rg500q_lwip_ppp_device_deinit(module);
    rg500q_lwip_ppp_exit(module);
    rg500q_ppp_exit(module);
    rg500q_ppp_deinit(module);

    return OS_EOK;
}
#endif

#endif /* RG500Q_USING_PPP_OPS */
