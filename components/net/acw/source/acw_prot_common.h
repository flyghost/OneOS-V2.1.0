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
 * @file        acw_udp.h
 *
 * @brief       acw  api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ACW_PROT_COMMON_H__
#define __ACW_PROT_COMMON_H__

#include <stdint.h>
#include <sys/socket.h>

#include "oneos_config.h"
#include "acw_debug.h"
#include "acw_conf.h"
#include "os_task.h"
#include "acw.h"

#define ACW_SWITCH_AP_TOPIC_COAP        "sys/awss/device/softap/switchap"
#define ACW_SWITCH_AP_TOPIC_JSON        "awss.device.softap.switchap"
#define ACW_REQ_ACW_TOPIC_COAP          "sys/awss/device/softap/reqacw"
#define ACW_REQ_ACW_TOPIC_JSON          "awss.device.softap.reqacw"
#define ACW_CONFIRM_REQ_ACW_TOPIC_COAP  "sys/awss/device/server/reqacw"
#define ACW_CONFIRM_REQ_ACW_TOPIC_JSON  "awss.device.server.reqacw"

#define ACW_NOTICE_TOPIC_COAP           "sys/awss/device/switchap/notice"
#define ACW_NOTICE_TOPIC_JSON           "awss.device.switchap.notice"

#define ACW_CONF_CLEAN_TOPIC_COAP       "sys/awss/conf/clean"
#define ACW_CONF_CLEAN_TOPIC_JSON       "sys.awss.conf.clean"

#define ACW_CTRL_LED_TOPIC_COAP         "sys/awss/ctrl/led"
#define ACW_CTRL_LED_TOPIC_JSON         "sys.awss.ctrl.led"


#define ACW_SERVER_MAIN_TOPIC_COAP      "sys/server"
#define ACW_SERVER_MAIN_TOPIC_JSON      "sys.server"

#define ACW_DEV_PRIVATE_TOPIC_COAP      "sys/dev/%s"
#define ACW_DEV_PRIVATE_TOPIC_JSON      "sys.dev.%s"

#define HAL_MAX_SSID_LEN                (32 + 1)    /* ssid: 32 octets at most, include the NULL-terminated */
#define HAL_MAX_PASSWD_LEN              (64 + 1)    /* password: 8-63 ascii */

#define PLATFORM_MAX_SSID_LEN           HAL_MAX_SSID_LEN
#define PLATFORM_MAX_PASSWD_LEN         HAL_MAX_PASSWD_LEN
#define ETH_ALEN                        6
#define RANDOM_MAX_LEN                  16
#define MSG_REQ_ID_LEN                  16
#define AWSS_DEV_AP_SWITCHA_RSP_LEN     512

#define ACW_DEVICE_ID_TEST              "1234567884"
#define ACW_PRODUCT_ID_TEST             "SsEDde256"

#define SSID_POSTFIX_STR_LEN            8   /* 设备AP模式区分每个设备的ssid的后缀长度 */
#define ACW_DEVAP_SSID_LEN              22  /* 设备AP模式ssid最大长度 */
#define ACW_NOLINK_AP_SSID		        "OneOS-ZeroNY-%s" /* 设备AP模式SSID */
#define ACW_NOLINK_AP_SSID_PRE          "OneOS-ZeroNY-"   /* 设备AP模式SSID */
#define ACW_NOLINK_AP_SSID_PRE_LEN      13

#define ACW_LINKED_AP_SSID              "OneOS-ZeroAL-" /* 零配默认SSID */
#define ACW_AP_DEFAULT_PASSWD           "Aa123456" /* AP模式默认密码 */

#define ACW_CTRL_LAMP_OPEN           0
#define ACW_CTRL_LAMP_CLOSE          1

#define OS_WLAN_SSID_MAX_LENGTH     32

#define ACW_DEV_LEN_MAX_LEN     8
typedef struct 
{
    char ssid[PLATFORM_MAX_SSID_LEN + 1];
    char passwd[PLATFORM_MAX_PASSWD_LEN + 1];
    uint8_t bssid[ETH_ALEN];
    uint8_t token[RANDOM_MAX_LEN + 1];
    uint16_t msgid;
    uint8_t cnt;
    uint8_t token_found;
} ap_conf_info_t;

typedef struct msg_json_parser{
    int msg_id;
    char *topic;
} msg_json_parser_t;

typedef enum
{
    DEV_ACW_MODE_DEVAP          = 0,
    DEV_ACW_MODE_ZEROC_STA      = 1,
    DEV_ACW_MODE_ZEROC_MASTER   = 2
} dev_acw_conf_mode_t;

typedef enum
{
    FROM_BLE      = 0,
    FROM_DEV      = 1,
    FROM_PHONE    = 2,
    FROM_MAX
} wifi_info_from_type_t;

typedef struct wifi_info
{
    char ssid_str[ACW_SSID_MAX_LEN + 1];	
    char passwd[ACW_AP_PASSWORD_MAX_LEN + 1];    
    char sender[ACW_OWNER_ID_MAX_LEN + 1];        
} wifi_info_t;

typedef struct acw_run_ctrl
{
//    dev_acw_conf_mode_t mode;
    os_task_t*    link_home_task;
    wifi_info_t rx_wifi_info[FROM_MAX];
    int home_ssid_connected;
    os_bool_t is_master_ap_open;
    os_bool_t is_slave_ap_open;
    acw_dev_connect_home_succ_sync cnt_home_succ_sync_func;
    acw_dev_req_add_home req_add_home_func;
} acw_run_ctrl_t;

#ifdef NET_USING_ACW_CRYPTO_HASH
//TOTO,use normal param to generate hash key, now use th same key
static inline void acw_dev_generate_hash_key(unsigned char *key, void *param)
{
    key[0] = 0x23;
    key[1] = 0xd6;
    key[2] = 0x34;
    key[3] = 0x93;
    key[4] = 0x63;
    key[5] = 0x76;
    key[6] = 0xad;
    key[7] = 0xfe;
    key[8] = 0xa3;
    key[9] = 0x41;
    key[10] = 0x60;
    key[11] = 0x32;
    key[12] = 0x22;
    key[13] = 0x98;
    key[14] = 0xff;
    key[15] = 0x12;
}
#endif

#ifdef NET_USING_ACW_CRYPTO
#define ACW_AES_FCB128_INIT_IV  {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}
#endif

extern void acw_main_loop(void *ctrl);
void acw_do_req_add_home(char *dev_id, os_bool_t clr, os_uint8_t rand);
void acw_do_connect_home_succ_sync(char *home_id, os_bool_t clr);

#endif /* end of __ACW_PROT_COMMON_H__ */
