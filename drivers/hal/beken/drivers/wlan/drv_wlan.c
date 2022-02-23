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
 * @file        drv_wlan.c
 *
 * @brief       This file implements wlan driver for beken
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "include.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/ethernetif.h"

#include <stdio.h>
#include <string.h>
#include "netif/etharp.h"
#include "lwip_netif_address.h"
#include "sa_station.h"
#include "drv_model_pub.h"
#include "mem_pub.h"
#include "common.h"
#include "hostapd_cfg.h"

#include "sk_intf.h"
#include "rw_pub.h"
#include "error.h"
#include "rtos_pub.h"
#include "param_config.h"
#include "wlan_ui_pub.h"

#include <os_task.h>
#include <os_device.h>
#include <os_hw.h>
#include <os_clock.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "os_types.h"
#include "bus.h"
#include "os_errno.h"

#include "drv_flash.h"
#include "drv_wlan.h"
#include "drv_wlan_fast_connect.h"

#include "uart_pub.h"
#include "ieee802_11_defs.h"
#include "wlan_ui_pub.h"
#include "net_param_pub.h"
#include "role_launch.h"
#include "app.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#ifdef BEKEN_DRV_DEBUG
#define DRV_WLAN_DBG(...)     os_kprintf("[DRV_WLAN]"),os_kprintf(__VA_ARGS__)
#else
#define DRV_WLAN_DBG(...)
#endif


#define ETH_INTF_DEBUG  0
#if ETH_INTF_DEBUG
#define ETH_INTF_PRT      warning_prf
#define ETH_INTF_WARN     warning_prf
#define ETH_INTF_FATAL    fatal_prf
#else
#define ETH_INTF_PRT      null_prf
#define ETH_INTF_WARN     null_prf
#define ETH_INTF_FATAL    null_prf
#endif

#define OS_WLAN_DEVICE(eth) (struct os_wlan_device *)(eth)

static char gs_dhcpd_server_ip[16] = "192.168.169.1";
char* DHCPD_SERVER_IP = gs_dhcpd_server_ip;

#ifdef BEKEN_USING_WLAN_STA
static struct os_wlan_device _g_sta_device;
static struct beken_wifi_info _g_sta_info;
#endif

#ifdef BEKEN_USING_WLAN_AP
static struct os_wlan_device _g_ap_device;
static struct beken_wifi_info _g_ap_info;
#endif

#define RT_WLAN_SSID_MAX_LEN 32
#define SCAN_WAIT_OUT_TIME 2000
static os_sem_t* _g_scan_done_sem;
int start_connect_tick = 0;
int end_connect_tick = 0;
int g_beken_rssi = 0;

static int g_sta_status = 0;
static int g_ap_status = 0;


static wifi_country_t country_code_CN   = {.cc= "CN", .schan=1, .nchan=13, .max_tx_power=0, .policy=WIFI_COUNTRY_POLICY_MANUAL};
static wifi_country_t country_code_US   = {.cc= "US", .schan=1, .nchan=11, .max_tx_power=0, .policy=WIFI_COUNTRY_POLICY_MANUAL};
static wifi_country_t country_code_EP   = {.cc= "EP", .schan=1, .nchan=13, .max_tx_power=0, .policy=WIFI_COUNTRY_POLICY_MANUAL};
static wifi_country_t country_code_JP   = {.cc= "JP", .schan=1, .nchan=14, .max_tx_power=0, .policy=WIFI_COUNTRY_POLICY_MANUAL};
static wifi_country_t country_code_AU   = {.cc= "AU", .schan=1, .nchan=13, .max_tx_power=0, .policy=WIFI_COUNTRY_POLICY_MANUAL};


static os_err_t _wifi_easyjoin(os_device_t *dev, void *passwd);
static os_err_t beken_wlan_disconnect(struct os_wlan_device *wlan);

extern void *net_get_sta_handle(void);
extern void *net_get_uap_handle(void);
extern void wifi_get_mac_address(char *mac, u8 type);
extern void *net_get_netif_handle(uint8_t iface);
extern int bmsg_tx_sender(struct pbuf *p, uint32_t vif_idx);
extern void bk_wlan_status_register_cb(FUNC_1PARAM_PTR cb);
extern struct netif* os_wlan_get_netif(struct os_wlan_device *wlan);
extern void dhcp_server_stop(void);
extern int wifi_set_mac_address(char *mac);
extern rw_evt_type mhdr_get_station_status(void);
extern os_bool_t os_wlan_is_connected(void);
extern os_bool_t os_wlan_ap_is_active(void);
extern int bk_wlan_power_save_set_level(BK_PS_LEVEL level);



//#define MINI_DUMP
//#define ETH_RX_DUMP
//#define ETH_TX_DUMP

#if defined(ETH_RX_DUMP) ||  defined(ETH_TX_DUMP)
static void packet_dump(const char *msg, const struct pbuf *p)
{
    const struct pbuf *q;
    os_uint32_t i, j;
    os_uint8_t *ptr;

    os_kprintf("%s %d byte\n", msg, p->tot_len);

#ifdef MINI_DUMP
    return;
#endif

    i = 0;
    for (q = p; q != OS_NULL; q = q->next)
    {
        ptr = q->payload;

        for (j = 0; j < q->len; j++)
        {
            if ((i % 8) == 0)
            {
                os_kprintf("  ");
            }
            if ((i % 16) == 0)
            {
                os_kprintf("\r\n");
            }
            os_kprintf("%02x ", *ptr);

            i++;
            ptr++;
        }
    }

    os_kprintf("\n\n");
}
#endif /* dump */

static os_err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    int ret;
    err_t err = ERR_OK;
    uint8_t vif_idx = rwm_mgmt_get_netif2vif(netif);

#ifdef ETH_TX_DUMP
    packet_dump("TX dump", p);
#endif /* ETH_TX_DUMP */

    if (!netif_is_link_up(netif))
    {
        return ERR_IF;
    }

    ret = bmsg_tx_sender(p, (uint32_t)vif_idx);
    if (0 != ret)
    {
        err = ERR_TIMEOUT;
    }

    return err;
}


void ethernetif_input(int iface, struct pbuf *p)
{
    struct eth_hdr        *ethhdr   = OS_NULL;
    struct netif          *netif    = OS_NULL;
	struct os_wlan_device *dev      = OS_NULL;

#ifdef ETH_RX_DUMP
    packet_dump("RX dump", p);
#endif /* ETH_RX_DUMP */

    if (p->len <= SIZEOF_ETH_HDR)
    {
        pbuf_free(p);
        return;
    }
	
    netif = rwm_mgmt_get_vif2netif((uint8_t)iface);
    if (!netif)
    {
        ETH_INTF_PRT("ethernetif_input no netif found %d\r\n", iface);
        pbuf_free(p);
        p = NULL;
        return;
    }

#ifdef BEKEN_USING_WLAN_STA
	if(netif == os_wlan_get_netif(&_g_sta_device))
		dev = &_g_sta_device;
#endif
#ifdef BEKEN_USING_WLAN_AP
	else if(netif == os_wlan_get_netif(&_g_ap_device))
		dev = &_g_ap_device;
#endif

	if (!dev)
    {
        ETH_INTF_PRT("ethernetif_input no wlan device found %d\r\n", iface);
        pbuf_free(p);
        p = NULL;
        return;
    }

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;

    switch (htons(ethhdr->type))
    {	
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
        /* full packet send to tcpip_thread to process */
        //if (netif->input(p, netif) != ERR_OK)    // ethernet_input
        if(os_wlan_dev_report_data(dev,p, p->tot_len) != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\r\n"));
            pbuf_free(p);
            p = NULL;
        }
        break;

    case ETHTYPE_EAPOL:
        ke_l2_packet_tx(p->payload, p->len, iface);
        pbuf_free(p);
        p = NULL;
        break;

    default:
        pbuf_free(p);
        p = NULL;
        break;
    }

}

/**
 * OS LwIP Interface
 */
 
struct netif *wlan_get_sta_netif(void)
{
#ifdef BEKEN_USING_WLAN_STA
    //return _g_sta_device.prot.netif;
	return os_wlan_get_netif(&_g_sta_device);
#else
    return OS_NULL;
#endif
}

struct netif *wlan_get_uap_netif(void)
{
#ifdef BEKEN_USING_WLAN_AP
    //return _g_ap_device.parent.netif;
	return os_wlan_get_netif(&_g_ap_device);
#else
    return OS_NULL;
#endif
}

// wlan event callbacks
static void wlan_event_handle(void *ctx)
{
    rw_evt_type event = *((rw_evt_type*)ctx);

    struct os_wlan_info info;
    struct os_wlan_buff user_buff;
    
    if ((event < 0) || (event > RW_EVT_MAX))
    {
        return;
    }
        
    DRV_WLAN_DBG("===wlan_event_handle:%d===\r\n",event);
    switch (event)
    {
#ifdef BEKEN_USING_WLAN_STA
    case RW_EVT_STA_CONNECTED:
        end_connect_tick = os_tick_get();
        DRV_WLAN_DBG("[wlan_connect]:start tick =  %d, connect done tick = %d, total = %d \n", start_connect_tick, end_connect_tick, end_connect_tick - start_connect_tick);
        os_wlan_dev_indicate_event_handle(&_g_sta_device, OS_WLAN_DEV_EVT_CONNECT, OS_NULL);
        break;

    case RW_EVT_STA_DISCONNECTED:
        os_wlan_dev_indicate_event_handle(&_g_sta_device, OS_WLAN_DEV_EVT_DISCONNECT, OS_NULL);
        break;

    case RW_EVT_STA_CONNECT_FAILED:
        case RW_EVT_STA_PASSWORD_WRONG:
    case RW_EVT_STA_NO_AP_FOUND:
        if (_g_sta_info.mode == ADVANCED_MODE)
        {
                          //wlan_fast_connect_info_erase();
            /* fast connected failed, switch to normal connect */
            _g_sta_info.mode = NORMAL_MODE;
        }
                
        os_wlan_dev_indicate_event_handle(&_g_sta_device, OS_WLAN_DEV_EVT_CONNECT_FAIL, OS_NULL);
        break;
#endif

#ifdef BEKEN_USING_WLAN_AP
    case RW_EVT_AP_CONNECTED:
        // null data,only  a parameter placeholder to insure  OS_WLAN_DEV_EVT_AP_ASSOCIATED flow  process  
        user_buff.data = &info;
        user_buff.len = sizeof(struct os_wlan_info);
        os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_ASSOCIATED, &user_buff);
        break;

    case RW_EVT_AP_DISCONNECTED:
        // null data,only  a parameter placeholder to insure  RW_EVT_AP_DISCONNECTED flow  process  
        user_buff.data = &info;
        user_buff.len = sizeof(struct os_wlan_info);
        os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_DISASSOCIATED, &user_buff);
        break;

    case RW_EVT_AP_CONNECT_FAILED:
        os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_ASSOCIATE_FAILED, OS_NULL);
        break;

    case RW_EVT_AP_START_COMPLETE:
            os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_START, OS_NULL);
    break;

    case RW_EVT_AP_STOP_COMPLETE:
            os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_STOP, OS_NULL);
    break;
#endif

    default:
        break;
    }
}
static void scan_ap_callback(void *ctxt, uint8_t param)
{
    if (_g_scan_done_sem)
    {
		 os_sem_post(_g_scan_done_sem);
        DRV_WLAN_DBG("release scan done semaphore \n");
    }
}

static int rt_wlan_malloc_scan_result(struct os_wlan_scan_result **scan_result, int num)
{
    struct os_wlan_scan_result *_scan_result;
    int i;
    int result = OS_EOK;

#if 0
    _scan_result = os_malloc(sizeof(struct os_wlan_scan_result));
    if (_scan_result == OS_NULL)
    {
        os_kprintf("rt_wlan_scan_result malloc failed!\r\n");
        result = -OS_ERROR;
        goto _exit;
    }
    os_memset(_scan_result, 0, sizeof(struct os_wlan_scan_result));
#else
	_scan_result = *scan_result;
#endif

    _scan_result->num = num;
    _scan_result->info = os_malloc(sizeof(struct os_wlan_info) * num);
    if (_scan_result->info == OS_NULL)
    {
        DRV_WLAN_DBG("rt_scan_rst table malloc failed!\r\n");
        result = -OS_ERROR;
        goto _exit;
    }
    os_memset(_scan_result->info, 0, sizeof(struct os_wlan_info) * num);

    return OS_EOK;
_exit:
	
    if (_scan_result->info)
    {
        os_free(_scan_result->info);
        _scan_result->info = OS_NULL;
    }

	/*
    if (_scan_result)
    {
        os_free(_scan_result);
        _scan_result = OS_NULL;
    }
    *scan_result = OS_NULL;
	*/

    return -OS_ERROR;
}

static const char *wlan_sec_type_string[] =
{
    "NONE",
    "WEP",
    "WPA-TKIP",
    "WPA-AES",
    "WPA2-TKIP",
    "WPA2-AES",
    "WPA2-MIX",
    "AUTO"
};

int wlan_scan_done_handler(struct os_wlan_scan_result **scan_result)
{
    struct os_wlan_scan_result *_scan_result;
    struct sta_scan_res *scan_rst_table;
    char scan_rst_ap_num = 0;
    int i;

    scan_rst_ap_num = bk_wlan_get_scan_ap_result_numbers();
    if (scan_rst_ap_num == 0)
    {
        DRV_WLAN_DBG("NULL AP \r\n");
        return -OS_ERROR;
    }

    scan_rst_table = (struct sta_scan_res *)os_malloc(sizeof(struct sta_scan_res) * scan_rst_ap_num);
    if (scan_rst_table == OS_NULL)
    {
        DRV_WLAN_DBG("scan_rst_table malloc failed!\r\n");
        return -OS_ERROR;
    }

    bk_wlan_get_scan_ap_result(scan_rst_table, scan_rst_ap_num);

    if (rt_wlan_malloc_scan_result(scan_result, scan_rst_ap_num) != OS_EOK)
    {
        DRV_WLAN_DBG("malloc memory for scan failed \n");
        return -OS_ERROR;
    }
    _scan_result = *scan_result;

    DRV_WLAN_DBG("\r\n");
    for (i = 0; i < scan_rst_ap_num; i++)
    {
        os_strncpy(_scan_result->info[i].ssid.val, scan_rst_table[i].ssid, RT_WLAN_SSID_MAX_LEN);
		_scan_result->info[i].ssid.len = strlen(scan_rst_table[i].ssid);
        os_memcpy(_scan_result->info[i].bssid, scan_rst_table[i].bssid, 6);
        _scan_result->info[i].channel = scan_rst_table[i].channel;

        DRV_WLAN_DBG("\033[36;22m ssid: %-32.*s  security: %-s\r\n", 32, scan_rst_table[i].ssid, wlan_sec_type_string[scan_rst_table[i].security]);

        switch (scan_rst_table[i].security)
        {
        case BK_SECURITY_TYPE_NONE:
            _scan_result->info[i].security = SECURITY_OPEN;
            break;

        case BK_SECURITY_TYPE_WEP:
            _scan_result->info[i].security = SECURITY_WEP_PSK;
            break;

        case BK_SECURITY_TYPE_WPA_TKIP:
            _scan_result->info[i].security = SECURITY_WPA_TKIP_PSK;
            break;

        case BK_SECURITY_TYPE_WPA_AES:
            _scan_result->info[i].security = SECURITY_WPA_AES_PSK;
            break;

        case BK_SECURITY_TYPE_WPA2_TKIP:
            _scan_result->info[i].security = SECURITY_WPA2_TKIP_PSK;
            break;

        case BK_SECURITY_TYPE_WPA2_AES:
            _scan_result->info[i].security = SECURITY_WPA2_AES_PSK;
            break;

        case BK_SECURITY_TYPE_WPA2_MIXED:
            _scan_result->info[i].security = SECURITY_WPA2_MIXED_PSK;
            break;

        case BK_SECURITY_TYPE_AUTO:
            // _scan_result.ap_table[i]->security = SECURITY_WEP_PSK;
            break;
        default:
            break;
        }
        _scan_result->info[i].rssi = scan_rst_table[i].level;
    }
    DRV_WLAN_DBG("\033[0m\r\n");

    if (scan_rst_table != NULL)
    {
        os_free(scan_rst_table);
        scan_rst_table = NULL;
    }

	return OS_EOK;
}

extern int wpa_get_psk(char *psk);
int _wifi_connect_done(void *ctx)
{
#if 0
    LinkStatusTypeDef link_status;
    struct wlan_fast_connect ap_info;

    memset(&link_status, 0, sizeof(LinkStatusTypeDef));
    memset(&ap_info, 0, sizeof(struct wlan_fast_connect));
    if (_g_sta_info.mode == NORMAL_MODE)
    {
        if ((bk_wlan_get_link_status(&link_status) == kNoErr) && (BK_SECURITY_TYPE_WEP != link_status.security))
        {
            memcpy(ap_info.ssid, link_status.ssid, strnlen(link_status.ssid, 32));
            memcpy(ap_info.bssid, link_status.bssid, 6);
            ap_info.channel = link_status.channel;
            ap_info.security = link_status.security;
            wpa_get_psk(ap_info.psk);
            wlan_fast_connect_info_write(&ap_info);
        }
    }
#endif

#ifdef BEKEN_USING_WLAN_STA
	 _g_sta_info.state = CONNECT_DONE;
#endif

    return 0;
}

#if 0
static int _wifi_power_manager(int level)
{
	return 0;
}
#else
extern int bk_wlan_dtim_rf_ps_timer_start(void);
extern int bk_wlan_dtim_rf_ps_timer_pause(void);
static int _wifi_power_manager(int level)
{
 switch (level)
 {
    case 0:
    {
        #if CFG_USE_MCU_PS
        /* disable cpu sleep */
        bk_wlan_mcu_ps_mode_disable();
        #endif
        #if CFG_USE_STA_PS
        /* disable rf sleep */
        bk_wlan_dtim_rf_ps_mode_disable();
        /* pause rf timer */
        bk_wlan_dtim_rf_ps_timer_pause();
        #endif
        break;
    }

    case 1:
    {
        #if CFG_USE_MCU_PS
        /* enable cpu sleep */
        bk_wlan_mcu_ps_mode_enable();
        #endif
        #if CFG_USE_STA_PS
        /* disable rf sleep */
        bk_wlan_dtim_rf_ps_mode_disable();
        /* pause rf timer */
        bk_wlan_dtim_rf_ps_timer_pause();
        #endif
        break;
    }
    case 2:
    {
         #if CFG_USE_MCU_PS
        /* disable cpu sleep */
        bk_wlan_mcu_ps_mode_disable();
		 #endif
		#if CFG_USE_STA_PS
        /* enable rf sleep */
        bk_wlan_dtim_rf_ps_mode_enable();
        /* start rf timer */
        bk_wlan_dtim_rf_ps_timer_start();
		#endif
        break;
    }

    case 3:
    {
        #if CFG_USE_MCU_PS
    	/* enable cpu sleep */
        bk_wlan_mcu_ps_mode_enable();
        #endif
        #if CFG_USE_STA_PS
        /* enable rf sleep */
        bk_wlan_dtim_rf_ps_mode_enable();
        /* start rf timer */
        bk_wlan_dtim_rf_ps_timer_start();
        #endif
        break;
    }

    default:
        break;
 }
}
#endif

#if LWIP_IPV4 && LWIP_IGMP
static err_t igmp_mac_filter(struct netif *netif, const ip4_addr_t *ip4_addr, u8_t action)
{
    uint8_t mac[6];
    const uint8_t *p = (const uint8_t *)ip4_addr;

    mac[0] = 0x01;
    mac[1] = 0x00;
    mac[2] = 0x5E;
    mac[3] = *(p + 1) & 0x7F;
    mac[4] = *(p + 2);
    mac[5] = *(p + 3);

    if (1)
    {
        DRV_WLAN_DBG("%s %s %s ", __FUNCTION__, (action == NETIF_ADD_MAC_FILTER) ? "add" : "del", ip4addr_ntoa(ip4_addr));
        DRV_WLAN_DBG("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    //wifi_add_mcast_filter(mac);

    return 0;
}
#endif /* LWIP_IPV4 && LWIP_IGMP */

#if LWIP_IPV6 && LWIP_IPV6_MLD
static err_t mld_mac_filter(struct netif *netif, const ip6_addr_t *ip6_addr, u8_t action)
{
    uint8_t mac[6];
    const uint8_t *p = (const uint8_t *)&ip6_addr->addr[3];

    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = *(p + 0);
    mac[3] = *(p + 1);
    mac[4] = *(p + 2);
    mac[5] = *(p + 3);

    if (1)
    {
        DRV_WLAN_DBG("%s %s %s ", __FUNCTION__, (action == NETIF_ADD_MAC_FILTER) ? "add" : "del", ip6addr_ntoa(ip6_addr));
        DRV_WLAN_DBG("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    //wifi_add_mcast_filter(mac);

    return 0;
}
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */


static os_err_t beken_wlan_init(struct os_wlan_device *wlan )
{
#if 0
    struct eth_device *eth = (struct eth_device *)dev;
#if LWIP_IPV4 && LWIP_IGMP
    netif_set_igmp_mac_filter(eth->netif, igmp_mac_filter);
#endif /* LWIP_IPV4 && LWIP_IGMP */

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif_set_mld_mac_filter(eth->netif, mld_mac_filter);
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
#endif
    /* Initialize semaphore for scan */
    _g_scan_done_sem = os_sem_create("scan_done", 0, OS_IPC_FLAG_FIFO);

    return OS_EOK;
}

static os_err_t beken_wlan_set_mode(struct os_wlan_device *wlan, os_wlan_mode_t mode)
{

    return OS_EOK;
}

static os_err_t beken_wlan_scan(struct os_wlan_device *wlan, struct os_scan_info *scan_info)
{	
	os_err_t ret = OS_EOK;
	struct os_wlan_scan_result *scan_result = os_wlan_scan_get_result();

	bk_wlan_scan_ap_reg_cb(scan_ap_callback);
	DRV_WLAN_DBG("%s L%d %s cmd: case WIFI_SCAN!\r\n", __FILE__, __LINE__, __FUNCTION__);
	
	if (os_strlen(scan_info->ssid.val) > 0)
	{
		UINT8 *ssid_ary[1];
		ssid_ary[0] = scan_info->ssid.val;
		bk_wlan_start_assign_scan(ssid_ary, 1);
	}
	else
	{
		bk_wlan_start_scan();
	}
	
	if (os_sem_wait(_g_scan_done_sem, os_tick_from_ms(SCAN_WAIT_OUT_TIME)) != OS_EOK)
	{
		DRV_WLAN_DBG("Wait scan_done semaphore timeout \n");
	}

	ret = wlan_scan_done_handler((struct os_wlan_scan_result **)&scan_result);
	os_wlan_dev_indicate_event_handle(wlan,OS_WLAN_DEV_EVT_SCAN_DONE,OS_NULL);
	
#if CFG_ROLE_LAUNCH
	if(mhdr_get_station_status() == RW_EVT_STA_GOT_IP)
	{
		rl_pre_sta_set_status(RL_STATUS_STA_LAUNCHED);
	}
#endif

	return ret;
}

static os_err_t beken_wlan_scan_stop(struct os_wlan_device *wlan)
{
	extern int bk_wlan_stop_scan(void);

	return bk_wlan_stop_scan();
}


static os_err_t beken_wlan_join(struct os_wlan_device *wlan, struct os_sta_info *sta_info)
{
	int ret = OS_EOK;
#ifdef BEKEN_USING_WLAN_STA /* needed only by station  */
	network_InitTypeDef_st wNetConfig;
    const char *ssid = OS_NULL;
    const os_uint8_t *bssid = OS_NULL;
    int len;

	start_connect_tick = os_tick_get();
    DRV_WLAN_DBG("beken_wlan_join: start connect \n");
    _g_sta_info.mode = NORMAL_MODE;
    _g_sta_info.state = CONNECT_DOING;
    ssid  = (char *)sta_info->ssid.val;
    bssid = (char *)sta_info->bssid;
    os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    if ((ssid != NULL) && ('\0' != *ssid))
    {
        len = os_strlen(ssid);
        if (SSID_MAX_LEN < len)
        {
            DRV_WLAN_DBG("ssid name more than 32 Bytes\r\n");
            return -OS_ERROR;
        }

        os_strncpy((char *)wNetConfig.wifi_ssid, ssid, sizeof(wNetConfig.wifi_ssid));
    }
#if CFG_SUPPOET_BSSID_CONNECT
    else if (((bssid[0] != 0xFF)
     || (bssid[1] != 0xFF)
     || (bssid[2] != 0xFF)
     || (bssid[3] != 0xFF)
     || (bssid[4] != 0xFF)
     || (bssid[5] != 0xFF))
     && ((bssid[0] != 0x0)
     || (bssid[1] != 0x0)
     || (bssid[2] != 0x0)
     || (bssid[3] != 0x0)
     || (bssid[4] != 0x0)
     || (bssid[5] != 0x0)))
    {
        os_memcpy((void *)wNetConfig.wifi_bssid, bssid, sizeof(wNetConfig.wifi_bssid));
    }
#endif
    else
    {
        DRV_WLAN_DBG("ssid is null or bssid is invalid/disabled\r\n");
        return -OS_ERROR;
    }


    if (sizeof(wNetConfig.wifi_key) < os_strlen(sta_info->key.val))
    {
        DRV_WLAN_DBG("wifi key is more than %d Bytes\r\n", sizeof(wNetConfig.wifi_key));
        return -OS_ERROR;
    }
    os_strncpy((char *)wNetConfig.wifi_key,(char *)sta_info->key.val,sizeof(wNetConfig.wifi_key));


    wNetConfig.wifi_mode = BK_STATION;
    wNetConfig.dhcp_mode = DHCP_CLIENT;
    wNetConfig.wifi_retry_interval = 100;

    DRV_WLAN_DBG("beken_wlan_join: ssid:%.*s bssid:%02x:%02x:%02x:%02x:%02x:%02x key:%.*s\r\n",
                                            sizeof(wNetConfig.wifi_ssid), wNetConfig.wifi_ssid,
                                            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
                                            sizeof(wNetConfig.wifi_key), wNetConfig.wifi_key);
    ret =  bk_wlan_start(&wNetConfig);
	if(OS_EOK == ret)
		g_sta_status = 1;
#endif

	return ret;
}

void beken_wlan_set_ap_ip(const char* ip)
{
    if (strlen(ip) > 15)
    {
        DRV_WLAN_DBG("beken_wlan_set_ap_ip to set ap ip[%s] too long, please check\r\n", ip);
        return;
    }

    memset((void *)DHCPD_SERVER_IP, sizeof(gs_dhcpd_server_ip), 0);
    strcpy(DHCPD_SERVER_IP, ip);
}

static os_err_t beken_wlan_softap(struct os_wlan_device *wlan, struct os_ap_info *ap_info)
{
	int ret = OS_EOK;
#ifdef BEKEN_USING_WLAN_AP /* needed only by ap  */
	network_InitTypeDef_st wNetConfig;
	const char *ssid = OS_NULL;
	int len;

	ssid = (char *)ap_info->ssid.val;
	os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

	if (ssid == NULL)
	{
		DRV_WLAN_DBG("ssid is null\r\n");
		return -OS_ERROR;
	}

	len = os_strlen(ssid);
	if (SSID_MAX_LEN < len)
	{
		DRV_WLAN_DBG("ssid name more than 32 Bytes\r\n");
		/* continue to use 32 bytes ssid, do not return err */
		len = SSID_MAX_LEN;
	}
	os_strncpy((char *)wNetConfig.wifi_ssid, ssid, sizeof(wNetConfig.wifi_ssid));

	if (sizeof(wNetConfig.wifi_key) < os_strlen(ap_info->key.val))
	{
		DRV_WLAN_DBG("wifi key is more than %d Bytes\r\n", sizeof(wNetConfig.wifi_key));
		return -OS_ERROR;
	}
	os_strncpy((char *)wNetConfig.wifi_key,(char *)ap_info->key.val,sizeof(wNetConfig.wifi_key));


	wNetConfig.wifi_mode = BK_SOFT_AP;
	wNetConfig.dhcp_mode = DHCP_SERVER;
	wNetConfig.wifi_retry_interval = 100;

	os_strcpy((char *)wNetConfig.local_ip_addr, DHCPD_SERVER_IP);
	os_strcpy((char *)wNetConfig.net_mask, "255.255.255.0");
	os_strcpy((char *)wNetConfig.gateway_ip_addr, DHCPD_SERVER_IP);
	os_strcpy((char *)wNetConfig.dns_server_ip_addr, DHCPD_SERVER_IP);

	DRV_WLAN_DBG("_wifi_softap: ssid:%.*s key:%.*s\r\n", sizeof(wNetConfig.wifi_ssid), wNetConfig.wifi_ssid, sizeof(wNetConfig.wifi_key), wNetConfig.wifi_key);

	ret = bk_wlan_start(&wNetConfig);
	
	if(OS_EOK == ret)
		g_ap_status = 1;
#endif
	return ret;
}

static os_err_t beken_wlan_disconnect(struct os_wlan_device *wlan)
{
    os_wlan_mode_t mode;

#if CFG_ROLE_LAUNCH
    LAUNCH_REQ param;
#endif

    mode = wlan->mode;

    if (mode == OS_WLAN_STATION)
    {
    	if(g_sta_status) 
    	{
#if CFG_ROLE_LAUNCH
	        param.req_type = LAUNCH_REQ_DELIF_STA;
	        rl_sta_request_enter(&param, 0);
#else
	        bk_wlan_stop(BK_STATION);
#endif
			g_sta_status = 0;
    	}
		else
		{
			os_wlan_dev_indicate_event_handle(&_g_sta_device, OS_WLAN_DEV_EVT_DISCONNECT, OS_NULL);
		}
    }
    else if (mode == OS_WLAN_AP)
    {
    	if(g_ap_status)
    	{	
	        bk_wlan_stop(BK_SOFT_AP);
			dhcp_server_stop();
			g_ap_status = 0;
    	}
		else
		{
			os_wlan_dev_indicate_event_handle(&_g_ap_device, OS_WLAN_DEV_EVT_AP_STOP, OS_NULL);
		}
    }

    return OS_EOK;
}


static os_err_t beken_wlan_set_channel (struct os_wlan_device *wlan, int channel)
{
	return  bk_wlan_set_channel(channel);
}

static int beken_wlan_get_channel(struct os_wlan_device *wlan)
{
	extern int bk_wlan_get_channel(void);

	return bk_wlan_get_channel();
}

static os_err_t beken_wlan_set_mac(struct os_wlan_device *wlan, os_uint8_t mac[])
{
	return wifi_set_mac_address(mac);
}

static os_err_t beken_wlan_get_mac(struct os_wlan_device *wlan, os_uint8_t mac[])
{
	int result = OS_EOK;
	struct beken_wifi_info *wifi_info = OS_NULL;

	wifi_info = (struct beken_wifi_info *)wlan->user_data;

	/* get MAC address */
	if (mac)
	    os_memcpy(mac, wifi_info->mac, 6);
	else
	    result =  OS_ERROR;

	return result;
}

int beken_wlan_get_rssi(struct os_wlan_device *wlan)
{
	return g_beken_rssi;
}

os_err_t beken_wlan_set_country(struct os_wlan_device *wlan, os_country_code_t country_code)
{
	wifi_country_t *country;

	if(OS_COUNTRY_CHINA == country_code)
		country = &country_code_CN;
	else if(OS_COUNTRY_UNITED_STATES == country_code)
		country = &country_code_US;
	else if(OS_COUNTRY_GERMANY == country_code)
		country = &country_code_EP;
	else if(OS_COUNTRY_JAPAN == country_code)
		country = &country_code_JP;
	else
		return OS_ERROR;
	
	return bk_wlan_set_country(country);
}

os_country_code_t beken_wlan_get_country(struct os_wlan_device *wlan)
{
	int result = OS_EOK;
	wifi_country_t country;
	os_country_code_t country_code = 0;

	result = bk_wlan_get_country(&country);
	if(OS_EOK!= result)
		return country_code;

	if("CN" == country.cc)
		country_code = OS_COUNTRY_CHINA;
	else if("US" == country.cc)
		country_code = OS_COUNTRY_UNITED_STATES;
	else if("EP" == country.cc)
		country_code = OS_COUNTRY_GERMANY;
	else if("JP" == country.cc)
		country_code = OS_COUNTRY_JAPAN;

	return country_code;
}

os_err_t beken_wlan_set_powersave(struct os_wlan_device *wlan, int level)
{
	return bk_wlan_power_save_set_level(level);
}

int beken_wlan_get_powersave(struct os_wlan_device *wlan)
{
	extern BK_PS_LEVEL global_ps_level;

	return global_ps_level;
}

static int beken_wlan_send_raw_frame(struct os_wlan_device *wlan, void *buff, int len)
{
	return bk_wlan_send_80211_raw_frame(buff,len);
}


static int beken_wlan_send(struct os_wlan_device *wlan, void *buff, int len)
{
	struct netif *netif = os_wlan_get_netif(wlan);

	return low_level_output(netif, (struct pbuf*)buff);
}

static int beken_wlan_recv(struct os_wlan_device *wlan, void *buff, int len)
{
	return OS_EOK;
}


static void app_demo_softap_rw_connected_event_func(void)
{
    os_kprintf("--------wlan ap [no password] connected event callback ----------\r\n");
}


/*=================================================================================*/
const static struct  os_wlan_dev_ops beken_wlan_ops =
{
    .wlan_init = beken_wlan_init, 				   		//os_err_t (*wlan_init)(struct os_wlan_device *wlan);
    .wlan_mode = beken_wlan_set_mode,			    	//os_err_t (*wlan_mode)(struct os_wlan_device *wlan, os_wlan_mode_t mode);
    .wlan_scan = beken_wlan_scan,				    	//os_err_t (*wlan_scan)(struct os_wlan_device *wlan, struct os_scan_info *scan_info);
    .wlan_join = beken_wlan_join,				    	//os_err_t (*wlan_join)(struct os_wlan_device *wlan, struct os_sta_info *sta_info);
    .wlan_softap     = beken_wlan_softap,				//os_err_t (*wlan_softap)(struct os_wlan_device *wlan, struct os_ap_info *ap_info);
    .wlan_disconnect = beken_wlan_disconnect,			//os_err_t (*wlan_disconnect)(struct os_wlan_device *wlan);
    .wlan_ap_stop    = beken_wlan_disconnect,      		//os_err_t (*wlan_ap_stop)(struct os_wlan_device *wlan);
    .wlan_ap_deauth  = OS_NULL,                    	 	//os_err_t (*wlan_ap_deauth)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    .wlan_scan_stop  = beken_wlan_scan_stop,			//os_err_t (*wlan_scan_stop)(struct os_wlan_device *wlan);
    .wlan_get_rssi         = beken_wlan_get_rssi,		//int (*wlan_get_rssi)(struct os_wlan_device *wlan);
    .wlan_set_powersave    = beken_wlan_set_powersave,	//os_err_t (*wlan_set_powersave)(struct os_wlan_device *wlan, int level);
    .wlan_get_powersave    = beken_wlan_get_powersave,	//int (*wlan_get_powersave)(struct os_wlan_device *wlan);
    .wlan_cfg_promisc      = OS_NULL,					//os_err_t (*wlan_cfg_promisc)(struct os_wlan_device *wlan, os_bool_t start);
    .wlan_cfg_filter 	   = OS_NULL,					//os_err_t (*wlan_cfg_filter)(struct os_wlan_device *wlan, struct os_wlan_filter *filter);
    .wlan_cfg_mgnt_filter  = OS_NULL,				    //os_err_t (*wlan_cfg_mgnt_filter)(struct os_wlan_device *wlan, os_bool_t start);
    .wlan_set_channel      = beken_wlan_set_channel,    //os_err_t (*wlan_set_channel)(struct os_wlan_device *wlan, int channel);
    .wlan_get_channel	   = beken_wlan_get_channel,    //int (*wlan_get_channel)(struct os_wlan_device *wlan);
    .wlan_set_country	   = beken_wlan_set_country,	//os_err_t (*wlan_set_country)(struct os_wlan_device *wlan, os_country_code_t country_code);
    .wlan_get_country      = beken_wlan_get_country,	//os_country_code_t (*wlan_get_country)(struct os_wlan_device *wlan);
    .wlan_set_mac		   = beken_wlan_set_mac,		//os_err_t (*wlan_set_mac)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    .wlan_get_mac          = beken_wlan_get_mac, 		//os_err_t (*wlan_get_mac)(struct os_wlan_device *wlan, os_uint8_t mac[]);
    .wlan_recv			   = beken_wlan_recv,           //int (*wlan_recv)(struct os_wlan_device *wlan, void *buff, int len);
#ifdef OS_WLAN_PROT_LWIP_PBUF_FORCE /* send pbuf links or a packet */
    .wlan_send  = OS_NULL,//beken_wlan_send,
#else
	.wlan_send  = OS_NULL,//beken_wlan_send_raw_frame,    //int (*wlan_send)(struct os_wlan_device *wlan, void *buff, int len);
#endif
    .wlan_send_raw_frame   = OS_NULL,//beken_wlan_send_raw_frame, //int (*wlan_send_raw_frame)(struct os_wlan_device *wlan, void *buff, int len);
};


/* register wlan device */
static int beken_wlan_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct os_wlan_device *wlan = OS_NULL;
    struct beken_wifi_info *wifi_info = OS_NULL;
	struct beken_wifi_info *wifi_info_tmp = OS_NULL;
    os_err_t result = OS_EOK;
	char temp_mac[6];

	wifi_info_tmp = (struct beken_wifi_info *)dev->info;
	
    // load mac, init mac first
    wifi_get_mac_address(temp_mac, CONFIG_ROLE_NULL);

if (OS_WLAN_STATION == wifi_info_tmp->work_mode)
{
#ifdef BEKEN_USING_WLAN_STA
    wlan = &_g_sta_device;
    wifi_info = &_g_sta_info;
    wifi_get_mac_address(wifi_info->mac, CONFIG_ROLE_STA);
	wifi_info->work_mode = wifi_info_tmp->work_mode;
	wlan->mode = wifi_info_tmp->work_mode;
	
    result = os_wlan_dev_register(wlan, dev->name,&beken_wlan_ops,OS_WLAN_FLAG_STA_ONLY,wifi_info);
    if (result != OS_EOK)
    {
        DRV_WLAN_DBG("register station wlan device failed! \n");
    }
    DRV_WLAN_DBG("register station wlan device sucess! \n");
#endif
}
else
{
#ifdef BEKEN_USING_WLAN_AP
    wlan = &_g_ap_device;
    wifi_info = &_g_ap_info;
    wifi_get_mac_address(wifi_info->mac, CONFIG_ROLE_AP);
	wifi_info->work_mode = wifi_info_tmp->work_mode;
	wlan->mode = wifi_info_tmp->work_mode;
	
    result = os_wlan_dev_register(wlan, dev->name,&beken_wlan_ops,OS_WLAN_FLAG_AP_ONLY,wifi_info);
    if (result != OS_EOK)
    {
        DRV_WLAN_DBG("register soft-ap wlan device failed! \n");
    }
    DRV_WLAN_DBG("register soft-ap wlan device sucess! \n");
	
    //bk_ap_no_password_connected_register_cb(app_demo_softap_rw_connected_event_func);
#endif
}

	bk_wlan_status_register_cb(wlan_event_handle);
    DRV_WLAN_DBG("beken wlan hw init\r\n");

    return 0;
}


OS_DRIVER_INFO beken_wlan_driver = {
    .name   = "Wlan_Type",
    .probe  = beken_wlan_probe,
};
OS_DRIVER_DEFINE(beken_wlan_driver, "0.end.0");


/* init protocol and register to LWIP */
static int beken_wlan_prot_init(void)
{
		os_device_t*		   device = OS_NULL;
		struct os_wlan_device* wlan   = OS_NULL;
		struct netif*		   netif  = OS_NULL;
	
		os_enter_critical();
		os_base_t level = os_hw_interrupt_disable();
		
		app_start();
		/* set wifi work mode */
#ifdef BEKEN_USING_WLAN_STA
		os_wlan_set_mode(OS_WLAN_DEVICE_STA_NAME, OS_WLAN_STATION);
	
		{ // beken LWIP special modifications
		  device = os_device_find(OS_WLAN_DEVICE_STA_NAME);
		  wlan	 = (struct os_wlan_device *)device;
		  netif = os_wlan_get_netif(wlan);
		  netif->linkoutput =(netif_linkoutput_fn)low_level_output;
		}
#endif
	
#ifdef BEKEN_USING_WLAN_AP
		os_wlan_set_mode(OS_WLAN_DEVICE_AP_NAME, OS_WLAN_AP);
	
		{ // beken LWIP special modifications
		  device = os_device_find(OS_WLAN_DEVICE_AP_NAME);
		  wlan	 = (struct os_wlan_device *)device;
		  netif = os_wlan_get_netif(wlan);
		  netif->linkoutput =(netif_linkoutput_fn)low_level_output;
		}
#endif
		os_hw_interrupt_enable(level);
		os_exit_critical();
		
		return 0;
}
OS_APP_INIT(beken_wlan_prot_init);

// eof
