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
#include <os_assert.h>

#include "acw_dev_ap.h"
#include "acw_prot_common.h"
#include "acw_conf.h"
#include "acw_intf.h"
#include "udp_client.h"
#include "acw_debug.h"

typedef enum
{
    SLAVE_STAT_INIT = 0,
    SLAVE_STAT_REQUESTED,
    SLAVE_STAT_APPROVED,
    SLAVE_STAT_ALREADY_CONFIG,
    SLAVE_STAT_REJECTED,
    SLAVE_STAT_ALREADY_REJECTED,
    SLAVE_STAT_MAX = 255
}slave_conf_stat_t;

typedef struct
{
    os_list_node_t dlist;
    char slave_devid[ACW_DEV_ID_LEN + 1];
#ifdef NET_USING_ACW_CRYPTO
    unsigned char pri_key[16];   //采用aes128，key为16Bytes
#endif
    os_uint8_t conf_stat;
    os_tick_t tick;
} slave_dev_conf_node_t;

// typedef struct acw_dev_ap
// {
//     os_uint16_t item_cnt;
//     os_uint16_t item_now;
//     char dev_id_start[1];
// }slave_dev_unconf_table_t;

// static slave_dev_unconf_table_t *gs_master_dev_unconf_tb = OS_NULL;
//static acw_run_ctrl_t *gs_ctrl = OS_NULL;
static struct os_mutex gs_master_msg_mtx;
static os_list_node_t gs_master_msg_list;
static struct os_mutex gs_slave_node_mtx;
static os_list_node_t gs_slave_node_list;

// static void acw_dev_unconf_table_dump(void)
// {
//     char *dev_id_start;

//     if (OS_NULL == gs_master_dev_unconf_tb)
//     {
//         return;
//     }

//     for (os_uint16_t index = 0; index < gs_master_dev_unconf_tb->item_now; index++)
//     {
//         dev_id_start = gs_master_dev_unconf_tb->dev_id_start + index * (ACW_DEV_ID_LEN + 1);
//         ACW_PRINT_I("index[%d]:%s", index, dev_id_start);
//     }

//     return;
// }

// static os_bool_t acw_is_dev_in_unconf_table(char *dev_id)
// {
//     char *dev_id_start;

//     if (OS_NULL == gs_master_dev_unconf_tb)
//     {
//         return OS_FALSE;
//     }

//     for (os_uint16_t index = 0; index < gs_master_dev_unconf_tb->item_now; index++)
//     {
//         dev_id_start = gs_master_dev_unconf_tb->dev_id_start + index * (ACW_DEV_ID_LEN + 1);
//         if (0 == strcmp(dev_id_start, dev_id))
//         {
//             return OS_TRUE;
//         }
//     }
    
//     return OS_FALSE;
// }

// static void acw_add_dev_to_unconf_table(char *dev_id)
// {
//     slave_dev_unconf_table_t *temp;
//     char *dev_id_start;
//     int len;

//     if (OS_NULL == gs_master_dev_unconf_tb)
//     {
//         len = sizeof(slave_dev_unconf_table_t) + ACW_TABLE_ADD_INTER_DEF * (ACW_DEV_ID_LEN + 1);
//         gs_master_dev_unconf_tb = (slave_dev_unconf_table_t *)malloc(len);
//         if (OS_NULL == gs_master_dev_unconf_tb)
//         {
//             ACW_PRINT_E("Why memory alloc failed");
//             return;
//         }
//         memset(gs_master_dev_unconf_tb, 0, len);

//         gs_master_dev_unconf_tb->item_now = 0;
//         gs_master_dev_unconf_tb->item_cnt = ACW_TABLE_ADD_INTER_DEF;
//     }
    
//     if (gs_master_dev_unconf_tb->item_now == gs_master_dev_unconf_tb->item_cnt)
//     {
//         len = sizeof(slave_dev_unconf_table_t) + (gs_master_dev_unconf_tb->item_cnt + ACW_TABLE_ADD_INTER_DEF) * (ACW_DEV_ID_LEN + 1);
//         temp = (slave_dev_unconf_table_t *)malloc(len);
//         if (OS_NULL == temp)
//         {
//             ACW_PRINT_E("Why memory alloc failed");
//             return;
//         }

//         memset(temp, 0, len);
//         temp->item_now =  gs_master_dev_unconf_tb->item_now;
//         temp->item_cnt = gs_master_dev_unconf_tb->item_cnt + ACW_TABLE_ADD_INTER_DEF;

//         len = gs_master_dev_unconf_tb->item_cnt * (ACW_DEV_ID_LEN + 1);
//         memcpy(temp->dev_id_start, gs_master_dev_unconf_tb->dev_id_start, len);

//         free(gs_master_dev_unconf_tb);
//         gs_master_dev_unconf_tb = temp;
//     }

//     dev_id_start = gs_master_dev_unconf_tb->dev_id_start + gs_master_dev_unconf_tb->item_now * (ACW_DEV_ID_LEN + 1);
//     strncpy(dev_id_start, dev_id, ACW_DEV_ID_LEN);
//     gs_master_dev_unconf_tb->item_now++;

//     return;
// }

// static void acw_del_dev_to_unconf_table(char *dev_id)
// {
//     char *dev_id_start;
//     os_uint16_t index;
//     int len;

//     if (OS_NULL == gs_master_dev_unconf_tb)
//     {
//         return;
//     }

//     for (index = 0; index < gs_master_dev_unconf_tb->item_now; index++)
//     {
//         dev_id_start = gs_master_dev_unconf_tb->dev_id_start + index * (ACW_DEV_ID_LEN + 1);
//         if (0 == strcmp(dev_id_start, dev_id))
//         {
//             break;
//         }
//     }

//     if (index >= gs_master_dev_unconf_tb->item_now)
//     {
//         return;
//     }

//     len = (gs_master_dev_unconf_tb->item_now - index - 1) * (ACW_DEV_ID_LEN + 1);
//     memcpy(dev_id_start, dev_id_start + ACW_DEV_ID_LEN + 1, len);
//     gs_master_dev_unconf_tb->item_now--;

//     return;
// }

acw_dev_ap_msg_t *acw_master_msg_alloc(int len)
{
    acw_dev_ap_msg_t *temp;

    len += sizeof(acw_dev_ap_msg_t);
    temp = (acw_dev_ap_msg_t *)malloc(len);

    if (OS_NULL == temp)
    {
        return OS_NULL;
    }

    memset(temp, 0, len);
    os_list_init(&temp->dlist);

    return temp;
}

static char *slave_conf_stat_trans_str(os_uint8_t stat)
{
    switch (stat)
    {
    case SLAVE_STAT_INIT:
        return "init";
    case SLAVE_STAT_REQUESTED:
        return "requested";
    case SLAVE_STAT_APPROVED:
        return "approved";
    case SLAVE_STAT_ALREADY_CONFIG:
        return "already_config";
    case SLAVE_STAT_REJECTED:
        return "rejected";
    case SLAVE_STAT_ALREADY_REJECTED:
        return "already_rejected";
    default:
        return "unkonown";
    }
}

void slave_conf_table_recycle_all(void)
{
    slave_dev_conf_node_t *node;
    slave_dev_conf_node_t *next;

    os_mutex_lock(&gs_slave_node_mtx, OS_WAIT_FOREVER);
    if (OS_TRUE == os_list_empty(&gs_slave_node_list))
    {
        os_mutex_unlock(&gs_slave_node_mtx);
        return;
    }

    os_list_for_each_entry_safe(node, next, &gs_slave_node_list, slave_dev_conf_node_t, dlist)
    {
        os_list_del_init(&node->dlist);
        free(node);
    }
    os_mutex_unlock(&gs_slave_node_mtx);

    return;
}

static os_bool_t slave_conf_table_do_recyle(slave_dev_conf_node_t *node)
{
    os_bool_t recycled;
    os_tick_t diff;
    os_tick_t now;

    now = os_tick_get();
    diff = OS_TICK_DIFF(now, node->tick);
    recycled = OS_FALSE;

    if (SLAVE_STAT_ALREADY_CONFIG == node->conf_stat)
    {
        if (diff > os_tick_from_ms(SLAVE_ALREADY_CONFIGED_TIMEOUT))
        {
            os_list_del_init(&node->dlist);
            free(node);
            recycled = OS_TRUE;
        }
    }
    else if (SLAVE_STAT_APPROVED == node->conf_stat)
    {
        if (diff > os_tick_from_ms(SLAVE_APPROVED_TIMEOUT))
        {
            os_list_del_init(&node->dlist);
            free(node);
            recycled = OS_TRUE;
        }
    }
    else if (SLAVE_STAT_REJECTED == node->conf_stat || SLAVE_STAT_ALREADY_REJECTED == node->conf_stat)
    {
        if (diff > os_tick_from_ms(SLAVE_REJECT_TIMEOUT))
        {
            os_list_del_init(&node->dlist);
            free(node);
            recycled = OS_TRUE;
        }
    }
    else
    {
        if (diff > os_tick_from_ms(SLAVE_REUSE_TIMEOUT_DEF))
        {
            os_list_del_init(&node->dlist);
            free(node);
            recycled = OS_TRUE;
        }
    }

    return recycled;
}

void slave_dev_req_table_refresh(void)
{
    slave_dev_conf_node_t *node;
    slave_dev_conf_node_t *next;

    os_mutex_lock(&gs_slave_node_mtx, OS_WAIT_FOREVER);
    if (OS_TRUE == os_list_empty(&gs_slave_node_list))
    {
        os_mutex_unlock(&gs_slave_node_mtx);
        return;
    }
    
    os_list_for_each_entry_safe(node, next, &gs_slave_node_list, slave_dev_conf_node_t, dlist)
    {
        (void)slave_conf_table_do_recyle(node);
    }
    os_mutex_unlock(&gs_slave_node_mtx);

    return;
}

static slave_dev_conf_node_t *slave_dev_add_to_req_table(char *dev_id)
{
    slave_dev_conf_node_t *node;
    slave_dev_conf_node_t *next;
    //os_bool_t recycled;
    int found;

    found = 0;
    do
    {
        if (OS_TRUE == os_list_empty(&gs_slave_node_list))
        {
            break;
        }

        os_list_for_each_entry_safe(node, next, &gs_slave_node_list, slave_dev_conf_node_t, dlist)
        {
            // recycled = slave_conf_table_do_recyle(node);
            // if (OS_TRUE == recycled)
            // {
            //     continue;
            // }

            if (OS_EOK == strcmp(node->slave_devid, dev_id))
            {
                found = 1;
                break;
            }
        }
    } while(0);

    if (found)
    {
        return node;
    }

    node = (slave_dev_conf_node_t *)malloc(sizeof(slave_dev_conf_node_t));
    if (OS_NULL != node)
    {
        os_list_init(&node->dlist);
        node->conf_stat = SLAVE_STAT_INIT;
        node->tick = os_tick_get();
        memset(node->slave_devid, 0 , sizeof(node->slave_devid));
        strncpy(node->slave_devid, dev_id, ACW_DEV_ID_LEN);
        os_list_add_tail(&gs_slave_node_list, &node->dlist);
    }

    return node;
}

void slave_dev_conf_table_show(void)
{
    slave_dev_conf_node_t *node;
    slave_dev_conf_node_t *next;

    os_mutex_lock(&gs_slave_node_mtx, OS_WAIT_FOREVER);
    os_list_for_each_entry_safe(node, next, &gs_slave_node_list, slave_dev_conf_node_t, dlist)
    {
        ACW_PRINT_I("[%s]-[%s]", node->slave_devid, slave_conf_stat_trans_str(node->conf_stat));
    }
    os_mutex_unlock(&gs_slave_node_mtx);

    return;
}

void acw_dev_req_add_home_result_sync(char *dev_id, os_bool_t is_appoved, acw_passwd_enc_type_t enc_type, char *key)
{
    slave_dev_conf_node_t *node;
    slave_dev_conf_node_t *next;
    os_bool_t do_find;
    int do_err;

#ifdef NET_USING_ACW_CRYPTO
#ifdef NET_USING_ACW_CRYPTO_HASH
    if (ACW_PASSWD_ENC_HASH_KEY != enc_type)
    {
        ACW_PRINT_E("crypto hash mode, enc_type[%d] invalid", enc_type);
        return;
    }
#else
    if (ACW_PASSWD_ENC_PRIVATE_KEY != enc_type || OS_NULL == key || strlen(key) != 16)
    {
        ACW_PRINT_E("crypto private key mode, enc_type[%d] or key invalid", enc_type);
        return;
    }
#endif
#else
    if (ACW_PASSWD_ENC_NONE != enc_type)
    {
        ACW_PRINT_E("none crypto mode, enc_type[%d] invalid", enc_type);
        return;
    }
#endif

    do_find = OS_FALSE;
    os_mutex_lock(&gs_slave_node_mtx, OS_WAIT_FOREVER);
    do
    {
        if (OS_TRUE == os_list_empty(&gs_slave_node_list))
        {
            break;
        }

        os_list_for_each_entry_safe(node, next, &gs_slave_node_list, slave_dev_conf_node_t, dlist)
        {
            do_err = strcmp(node->slave_devid, dev_id);
            if (!do_err)
            {
                if (SLAVE_STAT_REQUESTED == node->conf_stat)
                {
                    node->tick = os_tick_get();
                    if (OS_TRUE == is_appoved)
                    {
                        node->conf_stat = SLAVE_STAT_APPROVED;
#ifdef NET_USING_ACW_CRYPTO
#ifdef NET_USING_ACW_CRYPTO_HASH
                        memset(node->pri_key, 0, sizeof(node->pri_key));
                        acw_dev_generate_hash_key(node->pri_key, OS_NULL);
#else
                        memset(node->pri_key, 0, sizeof(node->pri_key));
                        memcpy(node->pri_key, key, strlen(key));
#endif
#endif
                    }
                    else
                    {
                        node->conf_stat = SLAVE_STAT_REJECTED;
                    }
                }
                do_find = OS_TRUE;
                break;
            }
        }
    } while(0);

    /* for a family, many master oneline, server will notice all maseter online */
    if (OS_FALSE == do_find)
    {
        slave_dev_conf_node_t *node;
        node = (slave_dev_conf_node_t *)malloc(sizeof(slave_dev_conf_node_t));
        if (OS_NULL != node)
        {
            os_list_init(&node->dlist);
            node->tick = os_tick_get();
            memset(node->slave_devid, 0 , sizeof(node->slave_devid));
            strncpy(node->slave_devid, dev_id, ACW_DEV_ID_LEN);
            if (OS_TRUE == is_appoved)
            {
                node->conf_stat = SLAVE_STAT_APPROVED;
#ifdef NET_USING_ACW_CRYPTO
#ifdef NET_USING_ACW_CRYPTO_HASH
                memset(node->pri_key, 0, sizeof(node->pri_key));
                acw_dev_generate_hash_key(node->pri_key, OS_NULL);
#else
                memset(node->pri_key, 0, sizeof(node->pri_key));
                memcpy(node->pri_key, key, strlen(key));
#endif
#endif
            }
            else
            {
                node->conf_stat = SLAVE_STAT_REJECTED;
            }
            os_list_add_tail(&gs_slave_node_list, &node->dlist);
        }
    }

    os_mutex_unlock(&gs_slave_node_mtx);

    return;
}

static os_bool_t isHomeConnected()
{
    return acw_check_intf_connected();
}

static int acw_master_msg_do_parse(char* udp_buff, os_uint16_t len, char* p_slave_devid, os_bool_t *cleared, os_uint8_t *rand) 
{
    const char *devid_start;
    char *rand_start;
    const char *clr;
    char *split;
    char *end;

    if (len == 0 || (strncmp(udp_buff, "iam:", 4) != 0 && strncmp(udp_buff, "irx:", 4) != 0))
    {
        return -1;
    }

    end = udp_buff + len - 1;
    devid_start = udp_buff + 4;

    split = strchr(devid_start, '\n');
    if (split == OS_NULL || split >= end)
    {
        return -2;
    }
    *split = '\0';
    strcpy(p_slave_devid, devid_start);
    if (strlen(p_slave_devid) != ACW_DEV_ID_LEN)
    {
        ACW_PRINT_I("p_slave_devid:%s", p_slave_devid);
        return -3;
    }

    clr = split + 1;
    split = strchr(clr, '\n');
    if (split == OS_NULL || split >= end)
    {
        *cleared = OS_FALSE;
        return 0;
    }
    
    *split = '\0';
    if (!strncmp(clr, "true", strlen("true")))
    {
        *cleared = OS_TRUE;
    }
    else
    {
        *cleared = OS_FALSE;
    }

    rand_start = split + 1;
    split = strchr(rand_start, '\n');
    if (split == OS_NULL || split >= end)
    {
        *rand = 0;
        return 0;
    }

    *split = '\0';
    *rand = atoi(rand_start);

    return 0;
}

static void acw_ap_master_do_recv_proc(ip_addr_t remote_addr, os_int32_t remote_port, char *data, os_int32_t len)
{
    acw_dev_ap_msg_t *msg;

    msg = acw_master_msg_alloc(len);
    if (OS_NULL == msg)
    {
        return;
    }

    msg->addr = remote_addr;
    msg->port = remote_port;

    memcpy(msg->msg, data, len);
    msg->msg_len = len;

    os_mutex_lock(&gs_master_msg_mtx, OS_WAIT_FOREVER);
    os_list_add_tail(&gs_master_msg_list, &msg->dlist);
    os_mutex_unlock(&gs_master_msg_mtx);

    return;
}

#ifdef NET_USING_ACW_CRYPTO

#include <mbedtls/aes.h>
#include <mbedtls/base64.h>

static int acw_master_do_home_ap_passwd_crypto(unsigned char* ssid_plain, int plain_len, unsigned char *key, unsigned char *passwd_cipher)
{
    mbedtls_aes_context aes_ctx;
    unsigned char pd_iv[16] = ACW_AES_FCB128_INIT_IV;
    size_t pd_iv_len = 0;
    int do_err;

    pd_iv_len = 0;
    mbedtls_aes_init(&aes_ctx);
    do_err = mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    do_err |= mbedtls_aes_crypt_cfb128(&aes_ctx, MBEDTLS_AES_ENCRYPT, plain_len, &pd_iv_len, pd_iv, ssid_plain, passwd_cipher);
    mbedtls_aes_free(&aes_ctx);

    return do_err;
}
#endif

static void acw_master_do_udp_msg_proc(void)
{
    char slave_devid[ACW_DEV_ID_LEN + 1];
    slave_dev_conf_node_t *slave_node;
    acw_dev_ap_msg_t *next;
    acw_dev_ap_msg_t *msg;
    os_bool_t def_resp;
    os_err_t do_err;
    os_tick_t diff;
    os_tick_t now;
    char resp[128];
    int resp_len;
    os_bool_t clr;
    os_uint8_t rand;
    char* passwd;
    char *ssid;

    os_mutex_lock(&gs_master_msg_mtx, OS_WAIT_FOREVER);
    os_list_for_each_entry_safe(msg, next, &gs_master_msg_list, acw_dev_ap_msg_t, dlist)
    {
        os_list_del_init(&msg->dlist);
        os_mutex_unlock(&gs_master_msg_mtx);

        do
        {
            def_resp = OS_TRUE;
            resp_len = 0;
            memset(resp, 0, sizeof(resp));
            do_err = acw_master_msg_do_parse(msg->msg, msg->msg_len, slave_devid, &clr, &rand);
            if (OS_EOK != do_err)
            {
                ACW_PRINT_E("master msg parse failed[%d]", do_err);
                break;
            }

            os_mutex_lock(&gs_slave_node_mtx, OS_WAIT_FOREVER);
            slave_node = slave_dev_add_to_req_table(slave_devid);
            if (OS_NULL == slave_node)
            {
                os_mutex_unlock(&gs_slave_node_mtx);
                ACW_PRINT_E("add slave [%s] to table failed, memory not enough", slave_devid);
                break;
            }

            if (strncmp(msg->msg, "iam:", 4) == 0)
            {
                now = os_tick_get();
                diff = OS_TICK_DIFF(now, slave_node->tick);
                ACW_PRINT_E("slave_dev:%s,stat=%d,diff=%d,req_timeout=%d", slave_node->slave_devid, slave_node->conf_stat, diff, os_tick_from_ms(SLAVE_REQUESTED_TIMEOUT));

                if (SLAVE_STAT_INIT == slave_node->conf_stat || 
                    (SLAVE_STAT_REQUESTED == slave_node->conf_stat && diff > os_tick_from_ms(SLAVE_REQUESTED_TIMEOUT)))
                {
                    slave_node->conf_stat = SLAVE_STAT_REQUESTED;
                    slave_node->tick = os_tick_get();
                    os_mutex_unlock(&gs_slave_node_mtx);
                    acw_do_req_add_home(slave_devid, clr, rand);
                    break;
                }
                else if (SLAVE_STAT_APPROVED == slave_node->conf_stat)
                {
                    ssid = acw_conf_get_stored_ssid();
                    passwd = acw_conf_get_stored_passwd();
#ifdef NET_USING_ACW_CRYPTO
                    size_t b64_encode_len;
                    int pd_len;
                    pd_len = strlen(passwd);
                    unsigned char *passwd_cipher;
                    passwd_cipher = malloc(pd_len * 3);
                    if (OS_NULL == passwd_cipher)
                    {
                        break;
                    }
                    memset(passwd_cipher, 0, pd_len * 3);
                    acw_master_do_home_ap_passwd_crypto((unsigned char *)passwd, pd_len, slave_node->pri_key, passwd_cipher);
                    mbedtls_base64_encode(passwd_cipher + pd_len, pd_len * 2, &b64_encode_len, passwd_cipher, pd_len);
                    os_kprintf("base64-encode:%s, blen=%d, slen=%d\r\n", passwd_cipher + pd_len, b64_encode_len, pd_len);
                    snprintf(resp, sizeof(resp), "zcw:%s\n%s\n%s\n", ssid, passwd_cipher + pd_len, acw_conf_get_owner_id());
                    resp_len = strlen(resp) + 1;
                    def_resp = OS_FALSE;
                    free(passwd_cipher);
#else
                    snprintf(resp, sizeof(resp), "zcw:%s\n%s\n%s\n", ssid, passwd, acw_conf_get_owner_id());
                    resp_len = strlen(resp) + 1;
                    def_resp = OS_FALSE;
#endif
                }
                else if (SLAVE_STAT_REJECTED == slave_node->conf_stat)
                {
                    snprintf(resp, sizeof(resp), "reject:%s\n", acw_conf_get_owner_id());
                    resp_len = strlen(resp) + 1;
                    def_resp = OS_FALSE;
                }
                else
                {
                    (void)0;
                }
            }
            else if (strncmp(msg->msg, "irx:", 4) == 0)
            {
                if (SLAVE_STAT_APPROVED == slave_node->conf_stat)
                {
                    slave_node->conf_stat = SLAVE_STAT_ALREADY_CONFIG;
                    slave_node->tick = os_tick_get();
                }
                else if (SLAVE_STAT_REJECTED == slave_node->conf_stat)
                {
                    slave_node->conf_stat = SLAVE_STAT_ALREADY_REJECTED;
                    slave_node->tick = os_tick_get();                
                }
                else
                {
                    (void)0;         
                }
            }
            else
            {
                (void)0;
            }
            os_mutex_unlock(&gs_slave_node_mtx);
        } while (0);
        
        if (OS_TRUE == def_resp)
        {
            snprintf(resp, sizeof(resp), "%s", MASTER_NO_MSG_DEF_RESP);
            resp_len = strlen(resp) + 1;
        }

        if (resp_len)
        {
            acw_intf_ap_send_send_resp(msg->addr, msg->port, resp, resp_len);
        }

        free(msg);
        
        os_mutex_lock(&gs_master_msg_mtx, OS_WAIT_FOREVER);
    }
    os_mutex_unlock(&gs_master_msg_mtx);

    return;
}

/*主配设备开ap*/
void acw_master_open_ap(acw_run_ctrl_t *ctrl)
{
    if (OS_TRUE == ctrl->is_master_ap_open)
    {
        return;
    }

    if (OS_TRUE == ctrl->is_slave_ap_open)
    {
        acw_intf_stop_ap();
        //ACW_PRINT_I("do stop slave ap");
        ctrl->is_slave_ap_open = OS_FALSE;
        os_task_msleep(1000);
        //return;
    }

    //ACW_PRINT_I("++++++++++++do master open ap");
	ip_addr_t intf_addr;
 
    char ssid_str[ACW_SSID_MAX_LEN + 1];
    memset(ssid_str, 0, sizeof(ssid_str));
    char *owner_id = acw_conf_get_owner_id();

    OS_ASSERT(owner_id);
    char phone16_char[32];

    acw_owner_id_to_ap_spec(owner_id, phone16_char, sizeof(phone16_char));
    os_snprintf(ssid_str, sizeof(ssid_str), "%s%s", ACW_LINKED_AP_SSID, phone16_char);

    acw_intf_set_ap_ip(UDP_SERVER_ADDR_MASTER);
    int do_err = acw_intf_start_ap(ssid_str, ACW_AP_DEFAULT_PASSWD);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_E("devap[%s] create error: %d", ssid_str, do_err);
        return;
    }

    do
    {
        acw_get_intf_ipaddr(acw_intf_type_ap, &intf_addr);
        if (ip_2_ip4(&intf_addr)->addr)
        {
            break;
        }
        os_kprintf(".");
        os_task_msleep(100);
    } while (1);
    os_kprintf("\r\n");

    acw_intf_ap_start_recv_proc(acw_ap_master_do_recv_proc);
    ctrl->is_master_ap_open = OS_TRUE;

    return;
}

void acw_master_close_ap(acw_run_ctrl_t *ctrl)
{
    //TODO: clear all conf table

    ACW_PRINT_I("master ap=%d", ctrl->is_master_ap_open);
    slave_conf_table_recycle_all();
    if (!ctrl->is_master_ap_open)
    {
       return;
    }

    (void)acw_intf_stop_ap();
    ACW_PRINT_I("master AP exit -------, to be slave");

    ctrl->is_master_ap_open = OS_FALSE;

    return;
}

os_bool_t gs_clear_flag = OS_FALSE;
void acw_clear_conf_notice(void)
{
    gs_clear_flag = OS_TRUE;
}

os_bool_t acw_get_clear_conf_flag(void)
{
    return gs_clear_flag;
}

void acw_main_loop(void* para)
{
    int do_err;
    acw_run_ctrl_t *ctrl = (acw_run_ctrl_t *)para;
    char *passwd;
    char *ssid;

    //gs_ctrl = ctrl;
    ssid = acw_conf_get_stored_ssid();
    passwd = acw_conf_get_stored_passwd();
	if (ssid && passwd)
    {
		do_err = acw_intf_connect_home_ap(ssid, passwd);
    }
    else
    {
        acw_slave_search_master(ctrl);
    }

    int continue_broken = 0;
    os_bool_t is_passwd_changed = OS_FALSE;
    os_bool_t home_cnted;

    while(1) 
    {
        if (OS_TRUE == gs_clear_flag)
        {
            acw_set_init_flag();
            os_task_msleep(2000);
            do_err = acw_intf_disconnect_ap();
            //do_err = acw_intf_stop_ap();
            //ctrl->is_master_ap_open = OS_FALSE;
            //ctrl->is_slave_ap_open = OS_FALSE;
            acw_conf_clean_ap_info();
            acw_master_close_ap(ctrl);
            gs_clear_flag = OS_FALSE;
        }
        
        slave_dev_req_table_refresh();
        
        ssid = acw_conf_get_stored_ssid();
        passwd = acw_conf_get_stored_passwd();

        if (OS_NULL == ssid || OS_NULL == passwd || is_passwd_changed)
        {
            acw_slave_loop(ctrl);
        }

        home_cnted = isHomeConnected();
        if (OS_TRUE == home_cnted)
        {
            acw_master_open_ap(ctrl);
            acw_master_do_udp_msg_proc();
        }
        else
        {
            ACW_PRINT_I("find isHomeConnected=%d", home_cnted);
        }

        if (OS_NULL != passwd && OS_NULL != ssid && isHomeConnected() == OS_FALSE )
        {
            do_err = acw_intf_connect_home_ap(ssid, passwd);
            continue_broken++;
            if (continue_broken > 10)
            {
                acw_intf_wifi_scan_result_t scan_result;
                scan_result.info_array = OS_NULL;
                scan_result.info_num = 0;
                do_err = acw_intf_do_wifi_scan(ssid, OS_NULL, 10, &scan_result);
                if (OS_EOK == do_err && scan_result.info_num > 0)
                {
                    ACW_PRINT_I("acw detect %s password change.", ssid);
                    is_passwd_changed = OS_TRUE; 
                    free(scan_result.info_array);
					acw_master_close_ap(ctrl);   
                }
                continue_broken = 0;
            }
        }
        else
        {
            continue_broken = 0;
            is_passwd_changed = OS_FALSE;
        }

        os_task_msleep(2000);
    }
}

void acw_master_dev_ap_init(void)
{
    os_mutex_init(&gs_master_msg_mtx, "master_msg_mtx", OS_FALSE);
    os_list_init(&gs_master_msg_list);

    os_mutex_init(&gs_slave_node_mtx, "slave_node_mtx", OS_FALSE);
    os_list_init(&gs_slave_node_list);

    return;
}

void acw_master_dev_ap_exit(void)
{
    return;  
}

static void acw_do_conf_table_show_func(int argc, char **argv)
{
    slave_dev_conf_table_show();
}

// static void acw_do_unconf_table_show_func(int argc, char **argv)
// {
//     acw_dev_unconf_table_dump();
// }

#ifdef OS_USING_SHELL
#include <shell.h>

SH_CMD_EXPORT(acw_conf_table_show, acw_do_conf_table_show_func, "acw conf table show");
//SH_CMD_EXPORT(acw_unconf_table_show, acw_do_unconf_table_show_func, "acw unconf table show");

#endif  /* end of using OS_USING_SHELL */
