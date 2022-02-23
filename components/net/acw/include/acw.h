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
 * @file        acw.h
 *
 * @brief       acw declaration  
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-24   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef __ACW_H__
#define __ACW_H__

typedef enum
{
    acw_intf_molink_esp8266     = 0,
    acw_intf_molink_esp32       = 1,
    acw_intf_soc_wifi_bk7231n   = 2,
}acw_intf_t;

typedef enum
{
    ACW_PASSWD_ENC_NONE         = 0,
    ACW_PASSWD_ENC_HASH_KEY     = 1,
    ACW_PASSWD_ENC_PRIVATE_KEY  = 2,
    ACW_PASSWD_ENC_UNKNOWN
}acw_passwd_enc_type_t;

typedef void (*acw_dev_connect_home_succ_sync)(char *home_id, os_bool_t clr);
typedef void (*acw_dev_req_add_home)(char *dev_id, os_bool_t clr, os_uint8_t rand);
extern void acw_start_proc(acw_intf_t intf, acw_dev_connect_home_succ_sync cnt_succ, acw_dev_req_add_home add_home);
extern void acw_clear_conf_notice(void);
extern os_bool_t acw_get_clear_conf_flag(void);

extern void acw_dev_req_add_home_result_sync(char *dev_id, os_bool_t is_appoved, acw_passwd_enc_type_t enc_type, char *key);

#endif /* end of __ACW_H__ */
