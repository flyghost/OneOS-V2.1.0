#include <os_types.h>
#include <os_errno.h>
#include <os_util.h>
#include <os_clock.h>
#include <string.h>
#include <os_stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <os_list.h>
#include <os_mutex.h>

#include "acw_dev_ap.h"
#include "acw_prot_common.h"
#include "acw_conf.h"
#include "acw_intf.h"
#include "udp_client.h"
//#include "mqtt_client.h"
#include "acw_debug.h"

typedef struct acw_reject_owner
{
    os_uint16_t item_cnt;
    os_uint16_t item_now;
    char owner_start[1];    //item size = ACW_OWNER_ID_MAX_LEN + 1
}acw_reject_owner_table_t;

static acw_reject_owner_table_t *gs_slave_reject_owner_tb = OS_NULL;
static os_list_node_t gs_slave_msg_list = OS_LIST_INIT(gs_slave_msg_list);
static struct os_mutex gs_slave_msg_mtx;

static void acw_reject_owner_table_dump(void)
{
    char *owner_start;

    if (OS_NULL == gs_slave_reject_owner_tb)
    {
        return;
    }

    for (os_uint16_t index = 0; index < gs_slave_reject_owner_tb->item_now; index++)
    {
        owner_start = gs_slave_reject_owner_tb->owner_start + index * (ACW_OWNER_ID_MAX_LEN + 1);
        ACW_PRINT_I("index[%d]:%s", index, owner_start);
    }

    return;
}

static os_bool_t acw_is_in_reject_owner_table(char *owner)
{
    char *owner_start;

    if (OS_NULL == gs_slave_reject_owner_tb)
    {
        return OS_FALSE;
    }

    for (os_uint16_t index = 0; index < gs_slave_reject_owner_tb->item_now; index++)
    {
        owner_start = gs_slave_reject_owner_tb->owner_start + index * (ACW_OWNER_ID_MAX_LEN + 1);
        if (0 == strcmp(owner_start, owner))
        {
            return OS_TRUE;
        }
    }
    
    return OS_FALSE;
}

static void acw_add_owner_to_reject_owner_table(char *owner)
{
    acw_reject_owner_table_t *temp;
    char *owner_start;
    int len;

    if (OS_NULL == gs_slave_reject_owner_tb)
    {
        len = sizeof(acw_reject_owner_table_t) + ACW_TABLE_ADD_INTER_DEF * (ACW_OWNER_ID_MAX_LEN + 1);
        gs_slave_reject_owner_tb = (acw_reject_owner_table_t *)malloc(len);
        if (OS_NULL == gs_slave_reject_owner_tb)
        {
            ACW_PRINT_E("Why memory alloc failed");
            return;
        }
        memset(gs_slave_reject_owner_tb, 0, len);

        gs_slave_reject_owner_tb->item_now = 0;
        gs_slave_reject_owner_tb->item_cnt = ACW_TABLE_ADD_INTER_DEF;
    }
    
    if (gs_slave_reject_owner_tb->item_now == gs_slave_reject_owner_tb->item_cnt)
    {
        len = sizeof(acw_reject_owner_table_t) + (gs_slave_reject_owner_tb->item_cnt + ACW_TABLE_ADD_INTER_DEF) * (ACW_OWNER_ID_MAX_LEN + 1);
        temp = (acw_reject_owner_table_t *)malloc(len);
        if (OS_NULL == temp)
        {
            ACW_PRINT_E("Why memory alloc failed");
            return;
        }

        memset(temp, 0, len);
        temp->item_now =  gs_slave_reject_owner_tb->item_now;
        temp->item_cnt = gs_slave_reject_owner_tb->item_cnt + ACW_TABLE_ADD_INTER_DEF;

        len = gs_slave_reject_owner_tb->item_cnt * (ACW_OWNER_ID_MAX_LEN + 1);
        memcpy(temp->owner_start, gs_slave_reject_owner_tb->owner_start, len);

        free(gs_slave_reject_owner_tb);
        gs_slave_reject_owner_tb = temp;
    }

    owner_start = gs_slave_reject_owner_tb->owner_start + gs_slave_reject_owner_tb->item_now * (ACW_OWNER_ID_MAX_LEN + 1);
    strncpy(owner_start, owner, ACW_OWNER_ID_MAX_LEN);
    gs_slave_reject_owner_tb->item_now++;

    return;
}

#if 0
static void acw_del_owner_to_reject_owner_table(char *owner)
{
    char *owner_start;
    os_uint16_t index;
    int len;

    if (OS_NULL == gs_slave_reject_owner_tb)
    {
        return;
    }

    for (index = 0; index < gs_slave_reject_owner_tb->item_now; index++)
    {
        owner_start = gs_slave_reject_owner_tb->owner_start + index * (ACW_OWNER_ID_MAX_LEN + 1);
        if (0 == strcmp(owner_start, owner))
        {
            break;
        }
    }

    if (index >= gs_slave_reject_owner_tb->item_now)
    {
        return;
    }

    len = (gs_slave_reject_owner_tb->item_now - index - 1) * (ACW_OWNER_ID_MAX_LEN + 1);
    memcpy(owner_start, owner_start + ACW_OWNER_ID_MAX_LEN + 1, len);
    gs_slave_reject_owner_tb->item_now--;

    return;
}
#endif

/* 开启设备AP热点 */
static os_err_t acw_dev_ap_open(void)
{
    os_err_t os_ret = OS_ERROR;
    char ssid_str[ACW_SSID_MAX_LEN + 1];	
    char chip_mac[ETH_ALEN] = {0};

    acw_get_intf_mac(acw_intf_type_sta, chip_mac);
    memset(ssid_str, 0, sizeof(ssid_str));
    char *dev_id = acw_conf_get_devid();
    if (dev_id)
    {
        os_snprintf(ssid_str, sizeof(ssid_str), "%s%s", ACW_NOLINK_AP_SSID_PRE, dev_id);
    }
    else
    {
        os_snprintf(ssid_str, sizeof(ssid_str), "%slt%02X%02X%02X", ACW_NOLINK_AP_SSID_PRE, chip_mac[3], chip_mac[4], chip_mac[5]);
    }

    acw_intf_set_ap_ip(UDP_SERVER_ADDR_SLAVE);
    os_ret = acw_intf_start_ap(ssid_str, ACW_AP_DEFAULT_PASSWD);
    if (OS_EOK == os_ret) 
    {
        ACW_PRINT_I("devap[%s] create success", ssid_str);
    }
    else
    {
        ACW_PRINT_E("devap[%s] create error: %d", ssid_str, os_ret);
    }

    return os_ret;
}

static int acw_check_cfg_msg_valid(char* udp_buff, os_uint16_t len)
{
    return OS_EOK;
}

static int acw_slave_msg_do_parse(char* udp_buff, os_uint16_t len, char** p_ssid, char** p_passwd, char** p_sender) 
{
    char *split;
    char *end;

    if (len == 0 || strncmp(udp_buff, "zcw:", 4) != 0)
    {
        return -1;
    }

    end = udp_buff + len - 1;

    *p_ssid = udp_buff + 4;
    split = strchr(*p_ssid, '\n');
    if (split == OS_NULL || split >= end)
    {
        return -2;
    }
    *split = '\0';
    if (0 == strlen(*p_ssid) || strlen(*p_ssid) > ACW_SSID_MAX_LEN)
    {
        return -3;
    }

    *p_passwd = split + 1;
    split = strchr(*p_passwd, '\n');
    if (split == OS_NULL || split >= end)
    {
        return -4;
    }
    *split = '\0';
    if (0 == strlen(*p_passwd) || strlen(*p_passwd) > ACW_AP_PASSWORD_MAX_LEN)
    {
        return -5;
    }

    *p_sender = split + 1;
    split = strchr(*p_sender, '\n');
    if (split == OS_NULL || split > end)
    {
        return -6;
    }
    *split = '\0';
    if (0 == strlen(*p_sender) || strlen(*p_sender) > ACW_OWNER_ID_MAX_LEN)
    {
        return -7;
    }

    ACW_PRINT_I("recv cfg, ssid=%s, pass=%s, sender=%s.", *p_ssid, *p_passwd, *p_sender);

    return 0;
}

static os_err_t acw_do_add_cfg_msg(ip_addr_t remote_addr, os_int32_t remote_port, char *data, os_int32_t len)
{
    acw_dev_ap_msg_t *temp;

    if (OS_NULL == data || len <= 0)
    {
        return OS_ERROR;
    }
   
    temp = acw_master_msg_alloc(len);
    if (OS_NULL == temp)
    {
        return OS_ERROR;
    }

    ACW_PRINT_I("recv cfg+++++++++++");

    temp->addr = remote_addr;
    temp->port = remote_port;

    temp->msg_len = len;
    memcpy(temp->msg, data, len);  

    os_mutex_lock(&gs_slave_msg_mtx, OS_WAIT_FOREVER);
    os_list_add_tail(&gs_slave_msg_list, &temp->dlist);
    os_mutex_unlock(&gs_slave_msg_mtx);

    return OS_EOK;
}

static void acw_ap_do_recv_proc(ip_addr_t remote_addr, os_int32_t remote_port, char *data, os_int32_t len)
{
    (void)acw_do_add_cfg_msg(remote_addr, remote_port, data, len);
}

/* 开启设备AP热点 */
static void acw_slave_open_ap(acw_run_ctrl_t *ctrl)
{
	ip_addr_t intf_addr;
    os_err_t do_err;

    if (OS_TRUE == ctrl->is_slave_ap_open)
    {
        ACW_PRINT_I("Now slave ap is open");
        return;
    }
    
    if (OS_TRUE == ctrl->is_master_ap_open)
    {
        acw_intf_stop_ap();
        ctrl->is_master_ap_open = OS_FALSE;
    }

    do_err = acw_dev_ap_open(); /* 开启设备ap热点 */
    if (OS_EOK != do_err)
    {
        ACW_PRINT_I("slave ap open failed, do_err = %d", do_err);
        return;
    }
    
    do
    {
        acw_get_intf_ipaddr(acw_intf_type_ap, &intf_addr);
        if (ip_2_ip4(&intf_addr)->addr)
        {
            ACW_PRINT_I("ap ipaddr: %08x", ip_2_ip4(&intf_addr)->addr);
            break;
        }
        os_kprintf(".");
        os_task_msleep(100);
    } while (1);
    os_kprintf("\r\n");

    acw_intf_ap_start_recv_proc(acw_ap_do_recv_proc);
    
    ctrl->is_slave_ap_open = OS_TRUE;

    return;
}

/*待配设备连接主配设备，并发送自己的uuid*/
/*发送成功，且收到ssid返回 0, 发送成功，收到回复，但未收到ssid返回 1，否则返回 -1 */
static int acw_slave_send_devid_to_master(acw_run_ctrl_t *ctrl, char* master_ssid)
{
    struct sockaddr_in addr_in;
	ip_addr_t gw_addr;
    struct timeval tv;
    char buffer[128];
    int sock_fd;    
    int do_err;
    int len;
    int rc;

    rc = OS_ERROR;
    sock_fd = -1;
    
    do
    {
        do_err = acw_intf_connect_home_ap(master_ssid, ACW_AP_DEFAULT_PASSWD);
        if (OS_EOK != do_err)
        {
            rc = do_err;
            break;
        }

        do_err = acw_get_intf_ipaddr_timeout(acw_intf_type_sta, 400, 100);
        if (do_err)
        {
            ACW_PRINT_E("wait ip timeout.");
            break;
        }
        acw_get_intf_gateway(acw_intf_type_sta, &gw_addr);

        sock_fd = udp_client_init(gw_addr, UDP_SERVER_PORT_TEST);
        if (sock_fd < 0)
        {
            ACW_PRINT_I("udp sta client create socket failed, err=%d", sock_fd);
            break; 
        }
        
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));    //set 1 sec time out 

        memset(&addr_in, 0, sizeof(addr_in));
        addr_in.sin_family = AF_INET;
        addr_in.sin_addr.s_addr = ip_2_ip4(&gw_addr)->addr;//inet_addr(server_ip);
        addr_in.sin_port = htons(UDP_SERVER_PORT_TEST);

        snprintf(buffer, sizeof(buffer),"iam:%s\n%s\n%03d\n", acw_conf_get_devid(), (acw_get_init_flag()==OS_TRUE) ? "true" : "false", acw_get_rand());
        len = strlen(buffer) + 1;
        
        (void)sendto(sock_fd, buffer, len, 0, (struct sockaddr *)&addr_in, sizeof(struct sockaddr));

        //req zcw from master
        int retry = 10;
        while (retry--)
        {
            len = recvfrom(sock_fd, buffer, sizeof(buffer), 0, OS_NULL, OS_NULL);
            if (len <= 0)
            {
                continue;
            }

            if (OS_EOK == strcmp(buffer, MASTER_NO_MSG_DEF_RESP))
            {
                os_task_msleep(1000);
                //ACW_PRINT_I("requtest to master end, rc=%d", rc);
                break;
            }
            else if(OS_NULL != strstr(buffer, "reject"))
            {
                //del last '\n' if has
                char *split = strchr(buffer, '\n');
                if ( OS_NULL != split)
                {
                    *split = 0;
                }
                
                ACW_PRINT_I("++++++++%s", buffer);
                acw_add_owner_to_reject_owner_table(buffer + strlen("reject:"));
                
                snprintf(buffer, sizeof(buffer), "irx:%s\n", acw_conf_get_devid());
                len = strlen(buffer) + 1;
                (void)sendto(sock_fd, buffer, len, 0, (struct sockaddr *)&addr_in, sizeof(struct sockaddr));
            }
            else
            {
                do_err = acw_check_cfg_msg_valid(buffer, len);
                if (!do_err)
                {
                    ip_addr_t temp_addr;
                    IP_SET_TYPE(&temp_addr, IPADDR_TYPE_V4);
                    ip_2_ip4(&temp_addr)->addr = 0;
                    acw_do_add_cfg_msg(temp_addr, 0, buffer, len);
                    rc = OS_EOK;
                }
                else
                {
                    ACW_PRINT_I("acw_slave_msg_do_parse %d.", do_err);
                }

                snprintf(buffer, sizeof(buffer), "irx:%s\n", acw_conf_get_devid());
                len = strlen(buffer) + 1;
                (void)sendto(sock_fd, buffer, len, 0, (struct sockaddr *)&addr_in, sizeof(struct sockaddr));
                ACW_PRINT_I("Recv cfg msg from master");
            }
        }
    } while (0);

    if (sock_fd >= 0)
    {
        closesocket(sock_fd);
        os_task_msleep(100);
    }
    
    acw_intf_disconnect_ap();
    os_task_msleep(100);

    return rc;
}

/*待配设备搜索主配设备，并发送自己的uuid*/
/*发送成功，且收到ssid返回 1，否则返回 -1 */
int acw_slave_search_master(acw_run_ctrl_t *ctrl)
{
    acw_intf_wifi_scan_result_t scan_result;
    char owner[ACW_OWNER_ID_MAX_LEN + 1];
    char *ownered;
    os_err_t do_err;
    char *found;
    int cnt_cnt;
    int index;

    scan_result.info_array = OS_NULL;
    scan_result.info_num = 0;
    do_err = acw_intf_do_wifi_scan(OS_NULL, OS_NULL, 10, &scan_result);
    if (OS_EOK != do_err || !scan_result.info_num)
    {
        return OS_ERROR;
    }
    ownered = acw_conf_get_owner_id();
    do_err = OS_ERROR;
    cnt_cnt = 0;
    for (index = 0; index < scan_result.info_num; index++)
    {
        found = strstr(scan_result.info_array[index].ssid.val, ACW_LINKED_AP_SSID);
        if (OS_NULL != found)
        {
            acw_ap_spec_to_owner(scan_result.info_array[index].ssid.val + strlen(ACW_LINKED_AP_SSID), owner, sizeof(owner));
            if (OS_NULL != ownered && 0 != strcmp(ownered, owner))
            {
                ACW_PRINT_I("Dev is ownered to:%s,jump uuid:%s", ownered, owner);
                continue;
            }

            if (scan_result.info_array[index].rssi < acw_get_master_connect_mim_rssi())
            {
                //ACW_PRINT_I("jump ssid:%s, uuid:%s, rssi=%d", scan_result.info_array[index].ssid.val, owner, scan_result.info_array[index].rssi);
                continue;
            }

            if (OS_NULL != ownered)
            {
                ACW_PRINT_I("Dev is ownered to:%s", ownered);
            }

            if (OS_TRUE == acw_is_in_reject_owner_table(owner))
            {
                //ACW_PRINT_I("AP:%s is in reject tb", owner);
                continue;
            }

            ACW_PRINT_I("Dev is req to:%s", owner);

            if (cnt_cnt)
            {
                acw_intf_disconnect_ap();
                os_task_msleep(2000);
            }
            cnt_cnt++;
            do_err = acw_slave_send_devid_to_master(ctrl, scan_result.info_array[index].ssid.val);
            if (OS_EOK == do_err)
            {
                break;
            }
            if (OS_ETIMEOUT == do_err || do_err > 0)
            {
                ACW_PRINT_I("Connect [%s, do_err=%d] failed, rssi = %d", scan_result.info_array[index].ssid.val, do_err, scan_result.info_array[index].rssi);
            }
        }
    }

    if (cnt_cnt)
    {
        acw_intf_disconnect_ap();
        os_task_msleep(2000);
    }

    free(scan_result.info_array);

    return do_err;
}

static void acw_do_slave_msg_recycle(void)
{
    acw_dev_ap_msg_t *next;
    acw_dev_ap_msg_t *msg;
    
    os_mutex_lock(&gs_slave_msg_mtx, OS_WAIT_FOREVER);
    do 
    {
        if (OS_TRUE == os_list_empty(&gs_slave_msg_list))
        {
            break;
        }
        os_list_for_each_entry_safe(msg, next, &gs_slave_msg_list, acw_dev_ap_msg_t, dlist)
        {
            os_list_del_init(&msg->dlist);
            free(msg);
        }
    } while(0);
    os_mutex_unlock(&gs_slave_msg_mtx);

    return;
}

#ifdef NET_USING_ACW_CRYPTO

#include <mbedtls/base64.h>
#include <mbedtls/aes.h>

static int acw_master_do_home_ap_passwd_decrypto(unsigned char* ssid_plain, int plain_len, unsigned char *passwd_cipher)
{
    mbedtls_aes_context aes_ctx;
    unsigned char pd_iv[16] = ACW_AES_FCB128_INIT_IV;
    size_t pd_iv_len;
    unsigned char key[16];
    int do_err;

    pd_iv_len = 0;
    memset(key, 0, sizeof(key));
#ifdef NET_USING_ACW_CRYPTO_HASH
    acw_dev_generate_hash_key(key, OS_NULL);
#else
    acw_get_private_key(key);
#endif
    mbedtls_aes_init(&aes_ctx);
    do_err = mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    do_err |= mbedtls_aes_crypt_cfb128(&aes_ctx, MBEDTLS_AES_DECRYPT, plain_len, &pd_iv_len, pd_iv, ssid_plain, passwd_cipher);
    mbedtls_aes_free(&aes_ctx);

    os_kprintf("dec pd:%s\r\n", passwd_cipher);

    return do_err;
}
#endif

/* 待配设备开启设备AP模式，等待被配置WIFI信息 */
void acw_slave_loop(acw_run_ctrl_t *ctrl)
{
    int do_err;    
    os_bool_t no_sleep = OS_FALSE;
    char* ssid_str;
    char* passwd;
    char* sender;
    char resp[32];
    //char mqtt_msg[64];
    os_bool_t home_ap_cnt_succ;
    acw_dev_ap_msg_t *next;
    acw_dev_ap_msg_t *msg;

    acw_slave_open_ap(ctrl);

    do
    {
        do
        {
            os_mutex_lock(&gs_slave_msg_mtx, OS_WAIT_FOREVER);
            if (OS_TRUE == os_list_empty(&gs_slave_msg_list))
            {
                os_mutex_unlock(&gs_slave_msg_mtx);
                break;
            }

            home_ap_cnt_succ = OS_FALSE;
            os_list_for_each_entry_safe(msg, next, &gs_slave_msg_list, acw_dev_ap_msg_t, dlist)
            {
                os_list_del_init(&msg->dlist);
                os_mutex_unlock(&gs_slave_msg_mtx);
                do
                {
                    do_err = acw_slave_msg_do_parse(msg->msg, msg->msg_len, &ssid_str, &passwd, &sender);
                    if (OS_EOK != do_err)
                    {
                        ACW_PRINT_W("slave msg parse err, no = %d", do_err);
                        do_err = -1;
                    }
#ifdef NET_USING_ACW_CRYPTO
                    char pd_base64_dec[ACW_AP_PASSWORD_MAX_LEN + 1];
                    char pd_dec[ACW_AP_PASSWORD_MAX_LEN + 1];
                    size_t de_len;
                    int len;

                    len = strlen(passwd);
                    memset(pd_base64_dec, 0, sizeof(pd_base64_dec));
                    memset(pd_dec, 0, sizeof(pd_dec));

                    de_len = 0;
                    mbedtls_base64_decode((unsigned char *)pd_base64_dec, ACW_AP_PASSWORD_MAX_LEN, &de_len, (unsigned char *)passwd, len);
                    acw_master_do_home_ap_passwd_decrypto((unsigned char *)pd_base64_dec, de_len, (unsigned char *)pd_dec);
                    passwd = pd_dec;
#endif
                    snprintf(resp, sizeof(resp),"recv=%d", do_err);
                    acw_intf_ap_send_send_resp(msg->addr, msg->port, resp, strlen(resp) + 1);
                    ACW_PRINT_I("send resp[%s] to[0x%08x,%d]", resp, msg->addr, msg->port);
                    os_task_msleep(500);
                    if (OS_EOK != do_err)
                    { 
                        break;
                    }
                    
                    do_err = acw_intf_connect_home_ap(ssid_str, passwd);

                    snprintf(resp, sizeof(resp),"connect=%d", do_err);
                    ACW_PRINT_I("send resp[%s] to[0x%08x,%d]", resp, msg->addr, msg->port);
                    acw_intf_ap_send_send_resp(msg->addr, msg->port, resp, strlen(resp) + 1);
                    os_task_msleep(500);
                    if (OS_EOK != do_err)
                    {
                        (void)acw_intf_disconnect_ap();
                        break;
                    }
                    do_err = acw_get_intf_ipaddr_timeout(acw_intf_type_sta, 400, 100);
                    if (OS_EOK == do_err)
                    {
                        home_ap_cnt_succ = OS_TRUE;
                        break;
                    }
                } while (0);
                
                os_mutex_lock(&gs_slave_msg_mtx, OS_WAIT_FOREVER);
                if (OS_TRUE == home_ap_cnt_succ)
                {
                    ACW_PRINT_I("Connect home ap success[%s]", ssid_str);
                    break;
                }
                free(msg);
            }
            os_mutex_unlock(&gs_slave_msg_mtx);

            if (OS_TRUE == home_ap_cnt_succ)
            {
                do_err = acw_intf_stop_ap();
                ctrl->is_slave_ap_open = OS_FALSE;

                ACW_PRINT_I("add by %s . AP exit -------, to be master", sender);
                ctrl->home_ssid_connected = 1;

                // if ap connect success, save conf ap info to flash
                acw_conf_save_ap_info(ssid_str, passwd, sender);
                acw_do_connect_home_succ_sync(sender, acw_get_init_flag());

                acw_clr_init_flag();
                free(msg);
                acw_do_slave_msg_recycle();

                return;
            }
        } while(0);

        //检查到密码修改的情况下，也有可能是信号不好，或者连上后获取不到ip，因此需要周期性的重试连接路由器
        char *passwd;
        char *ssid;
        ssid = acw_conf_get_stored_ssid();
        passwd = acw_conf_get_stored_passwd();
        if (ssid && passwd)
        {
            int do_err = acw_intf_connect_home_ap(ssid, passwd);
            if (!do_err) {
                (void)acw_intf_stop_ap();
                ctrl->is_slave_ap_open = OS_FALSE;
                ACW_PRINT_I("slave AP exit -------, to be master");
                ctrl->home_ssid_connected = 1;
                return;
            }
        }

        if (acw_slave_search_master(ctrl) == 0)
        {
            no_sleep = OS_TRUE;
        }

        if (no_sleep)
        {
            no_sleep = OS_FALSE;
        }
        else
        {
            os_task_msleep(5000);
        }
    } while(1);
}

void acw_slave_dev_ap_init(void)
{
    os_mutex_init(&gs_slave_msg_mtx, "slave_msg_mtx", OS_FALSE);
    return;
}

void acw_slave_dev_ap_exit(void)
{
    return;  
}

static void acw_do_reject_table_show_func(int argc, char **argv)
{
    acw_reject_owner_table_dump();
    return;
}

#ifdef OS_USING_SHELL
#include <shell.h>

SH_CMD_EXPORT(acw_reject_table_show, acw_do_reject_table_show_func, "acw reject table show");

#endif  /* end of using OS_USING_SHELL */
