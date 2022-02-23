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
 * 2021-02-09   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ACW_CONF_H__
#define __ACW_CONF_H__

#include <stdint.h>
#include <os_types.h>

#define ACW_DEV_ID_MAX_LEN		32
#define ACW_PRODUCT_ID_MAX_LEN	32
#define ACW_DEV_ID_LEN			8
#define ACW_SSID_MAX_LEN		32

#define MAX_REJECT_SSID     	(8)

#define ACW_AP_PASSWORD_MAX_LEN	32
#define ACW_OWNER_ID_MAX_LEN	11

typedef enum
{
	DEV_TYPE_WM 	= 0, //洗衣机
	DEV_TYPE_AC 	= 1, //空调
	DEV_TYPE_PP 	= 2, //插座
	DEV_TYPE_LP 	= 3, //灯
	DEV_TYPE_UNKNOWN
}acw_dev_type_t;

typedef struct dev_acw_info {
	char magic;
	char dev_id[ACW_DEV_ID_MAX_LEN + 1];
	char product_id[ACW_PRODUCT_ID_MAX_LEN + 1]; 
	char ssid[ACW_SSID_MAX_LEN + 1];
	char password[ACW_AP_PASSWORD_MAX_LEN + 1];
	char owner_id[ACW_OWNER_ID_MAX_LEN + 1];
} dev_acw_info_t;

extern os_bool_t acw_get_init_flag(void);
acw_dev_type_t acw_get_dev_type(void);
extern void acw_set_init_flag(void);
extern os_uint8_t acw_get_rand(void);
extern void acw_clr_init_flag(void);
extern char *acw_conf_get_stored_ssid(void);
extern char *acw_conf_get_stored_passwd(void);
extern char *acw_conf_get_pid(void);
extern char *acw_conf_get_devid(void);
extern char *acw_conf_get_owner_id(void);
extern void acw_conf_save_ap_info(char *ssid, char *passwd, char *owner_id);
#ifdef NET_USING_ACW_CRYPTO_PRI
extern void acw_get_private_key(char *key);
#endif
extern void acw_conf_clean_ap_info(void);
extern int acw_get_master_connect_mim_rssi(void);
extern int acw_conf_init(void);
//extern void acw_set_req_result(os_bool_t result);
extern void acw_wait_net_port_ok(char* portname, int loop_ms);
//extern char* acw_get_req_devid(void);
extern void acw_clear_reject_devid(char *rej_devid);
extern void acw_get_mqtt_addr(char addr[], int len, int *port);
extern void acw_get_mqtt_user_passwd(char user_name[], int user_len, char passwd[], int passwd_len);



#endif /* end of __ACW_CONF_H__ */
