#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <os_mutex.h>
#include <sys/socket.h>
#include <os_clock.h>

#undef ACW_BK7231N_LWIP_IPSTACK
#undef ACW_ESP8266_MOLINK_IPSTACK

#if defined (BSD_USING_MOLINK) && defined (MOLINK_USING_ESP8266)
#define ACW_ESP8266_MOLINK_IPSTACK
#endif

#if defined (BSD_USING_LWIP) && defined (BOARD_BK7231N)
#define ACW_BK7231N_LWIP_IPSTACK
#endif

#ifdef ACW_ESP8266_MOLINK_IPSTACK
#include <mo_api.h>
#include <mo_object.h>
#include <mo_wifi.h>
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
#include "lwip/udp.h"
#include "wlan_mgnt.h"
#include "wlan_dev.h"
#endif

#include "acw_intf.h"
#include "acw_conf.h"
#include "acw_debug.h"
#include "acw_prot_common.h"

#ifdef ACW_ESP8266_MOLINK_IPSTACK
typedef struct 
{
    mo_object_t *mo_obj;
    int skfd;
    acw_recv_cb_t cb;
} acw_intf_esp8266_t;
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
typedef struct 
{
    struct udp_pcb *srv_pcb;
    acw_recv_cb_t cb;
} acw_intf_bk7231n_t;
#endif

typedef struct acw_intf_run_ctrl
{
    acw_intf_t intf;
    acw_intf_stat_t intf_stat;
    os_mutex_t intf_mtx;
    union
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        acw_intf_esp8266_t esp8266_intf;
#endif
#ifdef ACW_BK7231N_LWIP_IPSTACK
        acw_intf_bk7231n_t bk7231n_intf;
#endif
        char unknown;
    }u;
}acw_intf_run_ctrl_t;

static acw_intf_run_ctrl_t gs_intf_ctrl;

#ifdef ACW_ESP8266_MOLINK_IPSTACK
static os_err_t acw_intf_do_esp8266_wifi_scan_info(mo_object_t *mo_obj, char *ssid, int *channel, int timout_s, acw_intf_wifi_scan_result_t *scan_result)
{
    mo_wifi_scan_result_t local_scan;
    os_err_t do_err;
    int index;
    int len;

    local_scan.info_array = OS_NULL;
    local_scan.info_num = 0;

    do_err = mo_wifi_scan_info(mo_obj, ssid, &local_scan);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_E("do wifi scan failed, do_err=%d", do_err);
        return OS_ERROR;
    }

    scan_result->info_num = local_scan.info_num;
    if (scan_result->info_num)
    {
        len = scan_result->info_num * sizeof(acw_intf_wifi_info_t);
        scan_result->info_array = (acw_intf_wifi_info_t *)malloc(len);
        if (OS_NULL == scan_result->info_array)
        {
            ACW_PRINT_E("memeory alloc failed");
            mo_wifi_scan_info_free(&local_scan);
            return OS_ERROR;
        }

        memset(scan_result->info_array, 0, len);
    }
    
    for (index = 0; index < scan_result->info_num; index++)
    {
        scan_result->info_array[index].channel = local_scan.info_array[index].channel;
        scan_result->info_array[index].rssi = local_scan.info_array[index].rssi;
        strncpy(scan_result->info_array[index].ssid.val, local_scan.info_array[index].ssid.val, WIFI_SSID_MAX_LENGTH);
    }
    
    mo_wifi_scan_info_free(&local_scan);

    return OS_EOK;
}
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
static os_err_t acw_intf_do_bk7231n_wifi_scan_info(char *ssid, int *channel, int timout_s, acw_intf_wifi_scan_result_t *scan_result)
{
    struct os_wlan_scan_result *local_scan;
    struct os_wlan_info filter;
    int need_filter;
    int do_err;
    int index;
    int len;

    local_scan = OS_NULL;
    INVALID_INFO(&filter);
    need_filter = 0;
    do_err = OS_ERROR;

    if (OS_NULL != ssid)
    {
        SSID_SET(&filter, ssid);
        need_filter = 1;
    }

    if (OS_NULL != channel)
    {
        filter.channel = *channel;
        need_filter = 1;
    }

    os_wlan_scan_result_clean();
    local_scan = os_wlan_scan_with_info(need_filter ? &filter : OS_NULL);
    do
    {
        if (OS_NULL == local_scan)
        {
            break;
        }

        scan_result->info_num = local_scan->num;
        if (scan_result->info_num)
        {
            len = scan_result->info_num * sizeof(acw_intf_wifi_info_t);
            scan_result->info_array = (acw_intf_wifi_info_t *)malloc(len);
            if (OS_NULL == scan_result->info_array)
            {
                ACW_PRINT_E("memeory alloc failed");
                break;
            }
            memset(scan_result->info_array, 0, len);
        }
        
        for (index = 0; index < scan_result->info_num; index++)
        {
            scan_result->info_array[index].channel =  local_scan->info[index].channel;
            scan_result->info_array[index].rssi = local_scan->info[index].rssi;
            strncpy(scan_result->info_array[index].ssid.val, local_scan->info[index].ssid.val, WIFI_SSID_MAX_LENGTH);
        }
        do_err = OS_EOK;
    } while (0);
    os_wlan_scan_result_clean();
    
    return do_err;
}
#endif

os_err_t acw_intf_do_wifi_scan(char *ssid, int *channel, int timout_s, acw_intf_wifi_scan_result_t *scan_result)
{
    os_err_t do_err;

    if (OS_NULL == scan_result)
    {
        return do_err;
    }

    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        do_err = acw_intf_do_esp8266_wifi_scan_info(gs_intf_ctrl.u.esp8266_intf.mo_obj, ssid, channel, timout_s, scan_result);
#else
        do_err = OS_ERROR;
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        do_err = OS_ERROR;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        int sta_num;
        sta_num = os_wlan_ap_get_sta_num();
		//todo: just for optimize, if there is a sta connect soft ap, do not scan. but test, sta connect soft ap a long time, then leave, we not known it
		sta_num = 0;
        if (!sta_num)
        {
            do_err = acw_intf_do_bk7231n_wifi_scan_info(ssid, channel, timout_s, scan_result);
        }
        else
        {
            ACW_PRINT_W("has %d sta connect ap, do not scan\r\n", sta_num);
            do_err = OS_ERROR;
        }
#endif
    }
    else
    {
        do_err = OS_ERROR;
    }

    return do_err;
}

#ifdef ACW_ESP8266_MOLINK_IPSTACK
static void acw_get_intf_esp8266_ipaddr(mo_object_t *mo_obj, acw_intf_type_t type, ip_addr_t *addr)
{
    ip_addr_t gw;
    ip_addr_t mask;
    do
    {
        if (acw_intf_type_sta == type)
        {
            mo_wifi_get_sta_cip(mo_obj, addr, &gw, &mask, OS_NULL, OS_NULL);
        }
        else if (acw_intf_type_ap == type)
        {
            mo_wifi_get_ap_cip(mo_obj, addr, &gw, &mask);
        }
        else
        {
            break;
        }
    } while(0);

    return;  
}
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
extern void lwip_get_intf_ipaddr(char *intf_name, ip_addr_t *addr);
static void acw_get_intf_bk7231n_ipaddr(acw_intf_type_t type, ip_addr_t *addr)
{
    lwip_get_intf_ipaddr((acw_intf_type_ap == type) ? "ap" : "w0", addr);
}
#endif

#include <os_types.h>

os_bool_t acw_check_intf_connected(void)
{
    if (OS_NULL == acw_conf_get_stored_ssid() || OS_NULL == acw_conf_get_stored_passwd())
    {
        return OS_FALSE;
    }

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        mo_wifi_stat_t stat;
        stat = mo_wifi_get_stat(gs_intf_ctrl.u.esp8266_intf.mo_obj);
        if (stat >= MO_WIFI_STAT_GOT_IP)
        {
            os_mutex_unlock(&gs_intf_ctrl.intf_mtx);
            return OS_TRUE;
        }
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        (void)0;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        ip_addr_t intf_addr;

        intf_addr.addr = 0;
        acw_get_intf_bk7231n_ipaddr(acw_intf_type_sta, &intf_addr);
        if (intf_addr.addr)
        {
            os_mutex_unlock(&gs_intf_ctrl.intf_mtx);
            return OS_TRUE;
        }
#endif
    }
    else
    {
        (void)0;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return OS_FALSE;
}

void acw_get_intf_ipaddr(acw_intf_type_t type, ip_addr_t *addr)
{
    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        acw_get_intf_esp8266_ipaddr(gs_intf_ctrl.u.esp8266_intf.mo_obj, type, addr);
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        (void)0;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        acw_get_intf_bk7231n_ipaddr(type, addr);
#endif
    }
    else
    {
        (void)0;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return;
}

os_err_t acw_get_intf_ipaddr_timeout(acw_intf_type_t type, int loop_ms, unsigned int loop_cnt)
{
    ip4_addr_t *intf_addr4;
    ip_addr_t intf_addr;
    
    intf_addr4 = ip_2_ip4(&intf_addr);
    while (loop_cnt-- > 0)
    {
        intf_addr4->addr = 0;
        acw_get_intf_ipaddr(type, &intf_addr);
        if (intf_addr4->addr)
        {
            return OS_EOK;
        }
        os_kprintf(".");
        os_task_msleep(loop_ms);
    };
    os_kprintf("\r\n");

    return OS_ERROR;
}

#ifdef ACW_ESP8266_MOLINK_IPSTACK
static void acw_get_intf_esp8266_gateway(mo_object_t *mo_obj, acw_intf_type_t type, ip_addr_t *gw)
{
    ip_addr_t ip;
    ip_addr_t mask;

    if (acw_intf_type_sta == type)
    {
        mo_wifi_get_sta_cip(mo_obj, &ip, gw, &mask, OS_NULL, OS_NULL);  
    }
    else if (acw_intf_type_ap == type)
    {
        mo_wifi_get_ap_cip(mo_obj, &ip, gw, &mask);
    }
    else
    {
        (void)0;
    }

    return;
}
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
extern void lwip_get_intf_gateway(char *intf_name, ip_addr_t *addr);
static void acw_get_intf_bk7231n_gateway(acw_intf_type_t type, ip_addr_t *addr)
{
    lwip_get_intf_gateway((acw_intf_type_ap == type) ? "ap" : "w0", addr);
}
#endif

void acw_get_intf_gateway(acw_intf_type_t type, ip_addr_t *gw)
{
    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
   
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        acw_get_intf_esp8266_gateway(gs_intf_ctrl.u.esp8266_intf.mo_obj, type, gw);
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        (void)0;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        acw_get_intf_bk7231n_gateway(type, gw);
#endif
    }
    else
    {
        (void)0;
    }

    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);
    
    return;
}

#ifdef ACW_BK7231N_LWIP_IPSTACK
extern int lwip_get_intf_mac(char *intf_name, char mac[]);
static os_err_t acw_get_intf_bk7231n_mac(acw_intf_type_t type, char mac[])
{
    return lwip_get_intf_mac((acw_intf_type_ap == type) ? "ap" : "w0", mac);
}
#endif

os_err_t acw_get_intf_mac(acw_intf_type_t type, char mac[])
{
    os_err_t do_err;

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    do
    {    
        if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
        {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
            if (acw_intf_type_sta == type)
            {            
                do_err = mo_wifi_get_sta_mac(gs_intf_ctrl.u.esp8266_intf.mo_obj, mac);
            }
            else if (acw_intf_type_ap == type)
            {
                do_err = mo_wifi_get_ap_mac(gs_intf_ctrl.u.esp8266_intf.mo_obj, mac);
            }
            else
            {
                do_err = OS_ERROR; 
            }
#else
            do_err = OS_ERROR;
#endif           
            break;
        }
        else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
        {
            do_err = OS_ERROR;
            break;
        }
        else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
        {
#ifdef ACW_BK7231N_LWIP_IPSTACK
            do_err = acw_get_intf_bk7231n_mac(type, mac);
#endif
            break;
        }
        else
        {
            do_err = OS_ERROR;
            break;
        }
    } while (0);
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return do_err;
}

void acw_intf_set_ap_ip(const char* ip)
{
    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    do
    {    
        if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
        {  
#ifdef ACW_ESP8266_MOLINK_IPSTACK 
            mo_wifi_set_ap_cip(gs_intf_ctrl.u.esp8266_intf.mo_obj, (char *)ip, OS_NULL, OS_NULL);
#endif       
            break;
        }
        else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
        {
            break;
        }
        else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
        {
#ifdef ACW_BK7231N_LWIP_IPSTACK
extern void beken_wlan_set_ap_ip(const char* ip);
            beken_wlan_set_ap_ip(ip);
#endif
            break;
        }
        else
        {
            break;
        }
    } while (0);
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);    
}

static os_err_t acw_intf_mo_start_ap(char *ssid, char *passwd)
{
    os_err_t do_err;

    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        do_err = mo_wifi_start_ap(gs_intf_ctrl.u.esp8266_intf.mo_obj, ssid, passwd, 6, 3);
#else
        do_err = OS_ERROR;   
#endif
    }
    else if(acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        do_err = OS_ERROR;
    }
    else
    {
        do_err = OS_ERROR;
    }

    return do_err;
}

os_err_t acw_intf_start_ap(char *ssid, char *passwd)
{
    os_err_t do_err;

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf || acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        do_err = acw_intf_mo_start_ap(ssid, passwd);
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        do_err = os_wlan_staos_ap(ssid, passwd);
#endif
    }

    if (OS_EOK == do_err)
    {
       gs_intf_ctrl.intf_stat = ACW_INTF_STAT_AP;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return do_err;
}

#ifdef ACW_BK7231N_LWIP_IPSTACK
static void acw_intf_bk7231n_ap_send_resp(struct udp_pcb *upcb, ip_addr_t *addr, u16_t port, char *resp, os_int32_t len)
{
    struct pbuf* p_reply;
    
    p_reply = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM); 
    if (OS_NULL == p_reply)
    {
        ACW_PRINT_E("pbuf alloc failed");
        return;
    }

    memcpy(p_reply->payload, resp, len);
    udp_sendto(upcb, p_reply, addr, port);
    pbuf_free(p_reply);

    return;
}
#endif

void acw_intf_ap_send_send_resp(ip_addr_t addr, os_int32_t remote_port, char *resp, os_int32_t len)
{
#if defined ACW_ESP8266_MOLINK_IPSTACK || defined ACW_BK7231N_LWIP_IPSTACK
    struct sockaddr_in addr_in;
#endif
    ip4_addr_t *intf_addr4;
    intf_addr4 = ip_2_ip4(&addr);
    
    if (!intf_addr4->addr || !remote_port)
    {
        return;
    }

    //ACW_PRINT_I("addr:%08x,port:%d,resp_len:%d,resp:%s", addr.addr, remote_port, len, resp);
    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        memset(&addr_in, 0, sizeof(addr_in));
        addr_in.sin_family = AF_INET;
        addr_in.sin_addr.s_addr = intf_addr4->addr;
        addr_in.sin_port = htons(remote_port); //TODO:
        
        (void)sendto(gs_intf_ctrl.u.esp8266_intf.skfd, resp, len, 0, (struct sockaddr *)&addr_in, sizeof(addr_in));
#endif    
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {

    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        if (OS_NULL != gs_intf_ctrl.u.bk7231n_intf.srv_pcb)
        {
            acw_intf_bk7231n_ap_send_resp(gs_intf_ctrl.u.bk7231n_intf.srv_pcb, &addr, remote_port, resp, len);
        }
#endif
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return;
}

os_err_t acw_intf_init(acw_intf_t intf)
{
    os_err_t do_err;

    do_err = OS_EOK;
    gs_intf_ctrl.intf = intf;
    gs_intf_ctrl.intf_stat = ACW_INTF_STAT_INIT;

    os_mutex_init(&gs_intf_ctrl.intf_mtx, "intfmtx", OS_FALSE);
    if (acw_intf_molink_esp8266 == intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK 
        mo_object_t *defmo_obj;

        defmo_obj = mo_get_default();
        if (OS_NULL == defmo_obj)
        {
            ACW_PRINT_E("Molink esp8266 get defmo_obj failed");
            return OS_ERROR;
        }
        gs_intf_ctrl.u.esp8266_intf.mo_obj = defmo_obj;
        gs_intf_ctrl.u.esp8266_intf.skfd = -1;
        gs_intf_ctrl.u.esp8266_intf.cb = OS_NULL;
#else
    //TODO:
        do_err = OS_ERROR;
#endif
    }
    else if (acw_intf_molink_esp32 == intf)
    {
        do_err = OS_ERROR;
    }
    else if (acw_intf_soc_wifi_bk7231n == intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        gs_intf_ctrl.u.bk7231n_intf.srv_pcb = OS_NULL;
        do_err = OS_EOK;
#else
    //TODO:
        do_err = OS_ERROR;      
#endif
    }
    else
    {
        do_err = OS_ERROR;
    }

    return do_err;
}

#ifdef ACW_ESP8266_MOLINK_IPSTACK
static void acw_intf_esp8266_udp_rx_cb(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size)
{
    if (OS_NULL != gs_intf_ctrl.u.esp8266_intf.cb)
    {
        gs_intf_ctrl.u.esp8266_intf.cb(addr, port, data, size);
    }
}
#endif

#ifdef ACW_BK7231N_LWIP_IPSTACK
static struct udp_pcb *acw_intf_bk7231n_ap_start_recv(ip_addr_t *r_addr, int port)
{
    struct udp_pcb *pcb;
    os_err_t do_err;

    pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    if (OS_NULL == pcb)
    {
        return OS_NULL;
    }
    
    do_err = udp_bind(pcb, r_addr, port);
    if (do_err != ERR_OK) 
    {
        udp_remove(pcb);
        return OS_NULL;
    }

    return pcb;  
}

static void acw_intf_bk7231n_udp_rx_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    u16_t len;
    
    if (OS_NULL != gs_intf_ctrl.u.bk7231n_intf.cb)
    {
        len = p->tot_len;
        if (p->len == len)
        {
            gs_intf_ctrl.u.bk7231n_intf.cb(*addr, port, p->payload, len);
        }
        else
        {
            //pbuf_get_contiguous
            ACW_PRINT_E("please do pbuf copy");
            OS_ASSERT(0);
        }
    }
    pbuf_free(p);

    return;
}
#endif

os_err_t acw_intf_ap_start_recv_proc(acw_recv_cb_t recv_cb)
{
#define ACW_CONF_PORT_DEF   9999
    os_err_t do_err;
    
#if defined ACW_ESP8266_MOLINK_IPSTACK || defined ACW_BK7231N_LWIP_IPSTACK
    struct sockaddr_in addr_in;
    ip_addr_t mask;
    ip_addr_t addr;
    ip_addr_t ip;
    ip_addr_t gw;
    int socket_fd;
#endif    

    do_err = OS_ERROR;
    if (OS_NULL == recv_cb)
    {
        return do_err;
    }

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    do
    {
        if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
        {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        do_err = mo_wifi_get_ap_cip(gs_intf_ctrl.u.esp8266_intf.mo_obj, &ip, &gw, &mask);
        if (OS_EOK != do_err)
        {
            ACW_PRINT_I("mo_wifi_get_ap_cip, errno=%d", do_err);
            break;   
        }

        addr.addr = (ip.addr & mask.addr) | (~mask.addr);

        socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0)
        {
            ACW_PRINT_I("Socket error, errno=%d", socket_fd);
            break;
        }

        memset(&addr_in, 0, sizeof(addr_in));
        addr_in.sin_family = AF_INET;
        addr_in.sin_addr.s_addr = addr.addr;//inet_addr("255.255.255.255");
        addr_in.sin_port = htons(ACW_CONF_PORT_DEF);

        os_task_msleep(1000);
        do_err = bind_with_cb(socket_fd, (struct sockaddr *)&addr_in, sizeof(addr_in), acw_intf_esp8266_udp_rx_cb);
        if (do_err < 0)
        {
            ACW_PRINT_E("connect skfd[%d] error, errno=%d", socket_fd, do_err);
            closesocket(socket_fd);
            break;  
        }
        gs_intf_ctrl.u.esp8266_intf.skfd = socket_fd;
        gs_intf_ctrl.u.esp8266_intf.cb = recv_cb;
#endif
        }
        else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
        {

        }
        else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
        {
#ifdef ACW_BK7231N_LWIP_IPSTACK
            if (OS_NULL != gs_intf_ctrl.u.bk7231n_intf.srv_pcb)
            {
                ACW_PRINT_I("Find bk7231n intf srv.pvb not null[0x%08x]", gs_intf_ctrl.u.bk7231n_intf.srv_pcb);
                udp_remove(gs_intf_ctrl.u.bk7231n_intf.srv_pcb);
                gs_intf_ctrl.u.bk7231n_intf.srv_pcb = OS_NULL;
            }
            ip_addr_t r_addr;
            acw_get_intf_bk7231n_gateway(acw_intf_type_ap, &r_addr);
            gs_intf_ctrl.u.bk7231n_intf.srv_pcb = acw_intf_bk7231n_ap_start_recv(&r_addr, ACW_CONF_PORT_DEF);
            if (OS_NULL == gs_intf_ctrl.u.bk7231n_intf.srv_pcb)
            {
                ACW_PRINT_I("Find bk7231n intf srv.pvb null coming, check please");
                break;
            }
            udp_recv(gs_intf_ctrl.u.bk7231n_intf.srv_pcb, acw_intf_bk7231n_udp_rx_cb, OS_NULL);
            gs_intf_ctrl.u.bk7231n_intf.cb = recv_cb;
#endif
        }
        else
        {
            break;
        }
        
        do_err = OS_EOK;

    } while (0);
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return do_err;
}

os_err_t acw_intf_connect_home_ap(char *ssid, char *passwd)
{
    os_err_t do_err;

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        (void)mo_wifi_disconnect_ap(gs_intf_ctrl.u.esp8266_intf.mo_obj);
        do_err = mo_wifi_connect_ap(gs_intf_ctrl.u.esp8266_intf.mo_obj, ssid, passwd);
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        do_err = OS_ERROR;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        do_err = os_wlan_connect_noscan(ssid, passwd);
#endif             
    }

    if (OS_EOK == do_err)
    {
        gs_intf_ctrl.intf_stat = ACW_INTF_STAT_STA;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    ACW_PRINT_I("connect [%s,%s] do_err=%d", ssid, passwd, do_err);
    return do_err;
}

os_err_t acw_intf_disconnect_ap(void)
{
    os_err_t do_err;

    do_err = OS_EOK;

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf || acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
//        ACW_PRINT_I("do disonnect++++++++++");
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        do_err = mo_wifi_disconnect_ap(gs_intf_ctrl.u.esp8266_intf.mo_obj);
#endif
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK        
        do_err = os_wlan_disconnect();
#endif        
    }

    if (OS_ERROR == do_err)
    {
        gs_intf_ctrl.intf_stat = ACW_INTF_STAT_INIT;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);   

    return do_err;    
}

os_err_t acw_intf_stop_ap(void)
{
    os_err_t do_err;

    os_mutex_lock(&gs_intf_ctrl.intf_mtx, OS_WAIT_FOREVER);
    if (acw_intf_molink_esp8266 == gs_intf_ctrl.intf)
    {
#ifdef ACW_ESP8266_MOLINK_IPSTACK
        if (gs_intf_ctrl.u.esp8266_intf.skfd >= 0)
        {
            closesocket(gs_intf_ctrl.u.esp8266_intf.skfd);
            gs_intf_ctrl.u.esp8266_intf.skfd = -1;
            gs_intf_ctrl.u.esp8266_intf.cb = OS_NULL;
        } 
#endif
    }
    else if (acw_intf_molink_esp32 == gs_intf_ctrl.intf)
    {
        do_err = OS_ERROR;
    }
    else if (acw_intf_soc_wifi_bk7231n == gs_intf_ctrl.intf)
    {
#ifdef ACW_BK7231N_LWIP_IPSTACK
        if (OS_NULL != gs_intf_ctrl.u.bk7231n_intf.srv_pcb)
        {
            ACW_PRINT_I("do udp remove[0x%08x]", gs_intf_ctrl.u.bk7231n_intf.srv_pcb);
            udp_remove(gs_intf_ctrl.u.bk7231n_intf.srv_pcb);
            gs_intf_ctrl.u.bk7231n_intf.srv_pcb = OS_NULL;
        }
        else
        {
            ACW_PRINT_I("++++++++++++++++++++why bk7231n coming++++++++");
        }
        do_err = os_wlan_ap_stop();
#endif
    }

    if (OS_EOK == do_err)
    {
        gs_intf_ctrl.intf_stat = ACW_INTF_STAT_STA;
    }
    os_mutex_unlock(&gs_intf_ctrl.intf_mtx);

    return OS_EOK;
}
