#ifndef __ONEPOS_WIFI_LOCA_H__
#define __ONEPOS_WIFI_LOCA_H__

#include <cJSON.h>
#include <mo_api.h>
#include <mo_wifi.h>
#include "onepos_common.h"

#define ONEPOS_WIFI_MAC_STR_LEN (MO_WIFI_BSSID_ARRAY_LENGTH * 2 + MO_WIFI_BSSID_ARRAY_LENGTH - 1) /* mac addr len : 17*/
#define ONEPOS_WIFI_RSSI_LEN    4u
#define ONEPOS_WIFI_MSG_FORMAT  "%02x:%02x:%02x:%02x:%02x:%02x,%4d"
#define ONEPOS_WIFI_INFO_LEN    22

extern os_bool_t onepos_get_wifi_sta(void);
extern os_err_t onepos_init_wifi_dev(void);
extern os_err_t onepos_wifi_pos_pub_msg(cJSON* json_src);

#endif /* __ONEPOS_WIFI_LOCA_H__ */
