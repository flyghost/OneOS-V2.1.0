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

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <os_types.h>
#include <os_errno.h>

#define ACW_OPEN_LAMP_MQTT_TOPIC        "%s/openlamp"
#define ACW_CLOSE_LAMP_MQTT_TOPIC       "%s/closelamp"
#define ACW_GET_LAMP_MQTT_TOPIC         "%s/getlamp"
#define ACW_LAMP_STAT_MQTT_TOPIC        "%s/lampstat"
#define ACW_ZERO_CONF_REQ_MQTT_TOPIC    "%s/zero_conf_req"
#define ACW_ZERO_CONF_RESP_MQTT_TOPIC   "%s/zero_conf_res"
#define ACW_CLEAR_SSID_MQTT_TOPIC       "%s/clearssid"
#define ACW_ADD_BY_MQTT_TOPIC           "%s/add_by"
#define ACW_WILL_MQTT_TOPIC             "%s/network_state"
#define ACW_DEV_INIT_MQTT_TOPIC         "%s/initial"
#define DEV_CTRL_CMD_MQTT_TOPIC         "%s/cmd"
#define DEV_CTRL_CMD_RESP_MQTT_TOPIC    "%s/cmd/response"
#define DEV_CTRL_QUERY_MQTT_TOPIC       "%s/cmd/query"
#define DEV_CTRL_QUERY_RESP_MQTT_TOPIC  "%s/cmd/query/response"
#define ACW_POWER_CONSUMPTION_REPORT_TOPIC  "%s/socket_power_consumption_report"
#define DEV_FINISHED_MQTT_TOPIC         "%s/finished"

#define DEV_CTRL_CMD_MAX_LEN            16

typedef enum
{
    MQTT_TOPIC_OPEN_LAMP        = 0,
    MQTT_TOPIC_CLOSE_LAMP       = 1,
    MQTT_TOPIC_GET_LAMP         = 2,
    MQTT_TOPIC_LAMPSTAT         = 3, 
    MQTT_TOPIC_CONF_REQ         = 4,
    MQTT_TOPIC_CONF_RESP        = 5,
    MQTT_TOPIC_CLEAN_SSID       = 6,
    MQTT_TOPIC_ADD_BY           = 7,
    MQTT_TOPIC_WILL             = 8,
    MQTT_TOPIC_INIT             = 9,
    MQTT_TOPIC_CMD              = 10,
    MQTT_TOPIC_CMD_RESP         = 11,
    MQTT_TOPIC_QUERY            = 12,
    MQTT_TOPIC_QUERY_RESP       = 13,
    MQTT_TOPIC_POWER_REPORT     = 14,
    MQTT_TOPIC_FINISHED,
    MQTT_TOPIC_MAX
}mqtt_topic_item_t;

typedef enum
{
	AC_CMD_ON_OFF 	    = 0, //开关
	AC_CMD_TEMPERATURE 	= 1, //温度设置
	AC_CMD_WORK_MODE    = 2, //工作模式
    AC_CMD_SPEED        = 3, //风速
    AC_CMD_UNKNOWN
}acw_ac_cmd_t;

#define ACW_AC_CMD_INIT   {"on_off", \
                           "temperature", \
                           "mode", \
                           "speed"}

static inline char *mqtt_get_ac_cmd_str(acw_ac_cmd_t cmd)
{
    char *ac_cmd_str[AC_CMD_UNKNOWN] = ACW_AC_CMD_INIT;

    if (cmd >= AC_CMD_UNKNOWN)
    {
        return "Unknown";
    }

    return ac_cmd_str[cmd];
}

static inline acw_ac_cmd_t mqtt_trans_str_to_ac_cmd(char *cmd_str)
{
    char *ac_cmd_str[AC_CMD_UNKNOWN] = ACW_AC_CMD_INIT;
    int index;

    for (index = 0; index < AC_CMD_UNKNOWN; index++)
    {
        if (!strcmp(cmd_str, ac_cmd_str[index]))
        {
            break;
        }
    }

    return (acw_ac_cmd_t)index;
}

#define AC_CMD_ON_OFF_VALUE_ON  0
#define AC_CMD_ON_OFF_VALUE_OFF 1

#define AC_CMD_TEMPERATURE_VALUE_MIN  16
#define AC_CMD_TEMPERATURE_VALUE_MAX  30

typedef enum
{
	AC_WORK_MODE_AUTO   = 0, //Auto
	AC_WORK_MODE_COOL 	= 1, //Cool
	AC_WORK_MODE_HEAT   = 2, //Heat
    AC_WORK_MODE_DRY    = 3, //Dry
    AC_WORK_MODE_FAN    = 4, //Fan
    AC_WORK_MODE_ECO    = 5, //ECO
    AC_WORK_MODE_UNKNOWN
} acw_ac_work_mode_t;

#define ACW_AC_WORK_MODE_INIT   {"Auto", \
                                 "Cool", \
                                 "Heat", \
                                 "Dry", \
                                 "Fan", \
                                 "ECO"}

static inline char *mqtt_get_ac_work_mode_str(acw_ac_work_mode_t mode)
{
    char *ac_work_mode_str[AC_WORK_MODE_UNKNOWN] = ACW_AC_WORK_MODE_INIT;
    
    if (mode >= AC_WORK_MODE_UNKNOWN)
    {
        return "Unknown";
    }

    return ac_work_mode_str[mode];
}

typedef enum
{
	AC_SPEED_AUTO       = 0, //Auto
	AC_SPEED_SLEEP 	    = 1, //Sleep
	AC_SPEED_NATURAL    = 2, //Natural
    AC_SPEED_HIGH       = 3, //High
    AC_SPEED_MEDIUM     = 4, //Fan
    AC_SPEED_LOW        = 5, //Eco
    AC_SPEED_UNKNOWN
} acw_ac_speed_t;

#define ACW_AC_SLEEP_INIT       {"Auto", \
                                 "Sleep", \
                                 "Natural", \
                                 "High", \
                                 "Medium", \
                                 "Low"}

static inline char *mqtt_get_ac_speed_str(acw_ac_speed_t speed)
{
    char *ac_speed_str[AC_SPEED_UNKNOWN] = ACW_AC_SLEEP_INIT;
    
    if (speed >= AC_SPEED_UNKNOWN)
    {
        return "Unknown";
    }

    return ac_speed_str[speed];
}

typedef enum
{
	WM_CMD_ON_OFF 	    = 0, //开关
	WM_CMD_WORK_MODE 	= 1, //工作模式设置
	WM_CMD_START_STOP   = 2, //启停
    WM_CMD_UNKNWON
}acw_wm_cmd_t;

#define ACW_WM_CMD_INIT       {"on_off", \
                               "mode", \
                               "start_stop"}

static inline char *mqtt_get_wm_cmd_str(acw_wm_cmd_t cmd)
{
    char *wm_cmd_str[WM_CMD_UNKNWON] = ACW_WM_CMD_INIT;

    if (cmd >= WM_CMD_UNKNWON)
    {
        return "Unknown";
    }

    return wm_cmd_str[cmd];
}

static inline acw_wm_cmd_t mqtt_trans_str_to_wm_cmd(char *cmd_str)
{
    char *wm_cmd_str[WM_CMD_UNKNWON] = ACW_WM_CMD_INIT;
    int index;

    for (index = 0; index < WM_CMD_UNKNWON; index++)
    {
        if (!strcmp(cmd_str, wm_cmd_str[index]))
        {
            break;
        }
    }

    return (acw_wm_cmd_t)index;
}

#define WM_CMD_ON_OFF_VALUE_ON  0
#define WM_CMD_ON_OFF_VALUE_OFF 1

typedef enum
{
	WM_WORK_MODE_MIX        = 0, //Mix
	WM_WORK_MODE_COTTON 	= 1, //Cotton
	WM_WORK_MODE_WOOL       = 2, //Wool
    WM_WORK_MODE_BLANKET    = 3, //Blanket
    WM_WORK_MODE_SHIRT      = 4, //shirt
    WM_WORK_MODE_UNDERWEAR  = 5, //Underwear
	WM_WORK_MODE_JEANS      = 6, //Jeans
	WM_WORK_MODE_FEATHER 	= 7, //Feather
	WM_WORK_MODE_QC         = 8, //Quick cleaning
    WM_WORK_MODE_SC         = 9, //Soft cleaning
    WM_WORK_MODE_AW         = 10, //Air Washing
    WM_WORK_MODE_TC         = 11, //Tub Cleaning
    WM_WORK_MODE_SW         = 12, //Soak Washing
	WM_WORK_MODE_ODRY 	    = 13, //Only Dry
	WM_WORK_MODE_ODEHY      = 14, //only Dehydrate
    WM_WORK_MODE_OR         = 15, //Only Rinse
    WM_WORK_MODE_UNKNOWN
}acw_wm_work_mode_t;

#define ACW_WM_WORK_MODE_INIT   {"Mix", \
                                 "Cotton", \
                                 "Wool", \
                                 "Blanket", \
                                 "shirt", \
                                 "Underwear", \
                                 "Jeans", \
                                 "Feather", \
                                 "Quick cleaning", \
                                 "Soft cleaning", \
                                 "Air Washing", \
                                 "Tub Cleaning", \
                                 "Soak Washing", \
                                 "Only Dry", \
                                 "Only Dehydrate", \
                                 "Only Rinse"}

static inline char *mqtt_get_wm_work_mode_str(acw_wm_work_mode_t mode)
{
    char *wm_work_mode_str[WM_WORK_MODE_UNKNOWN] = ACW_WM_WORK_MODE_INIT;

    if (mode >= WM_WORK_MODE_UNKNOWN)
    {
        return "Unknown";
    }

    return wm_work_mode_str[mode];
}

#define WM_CMD_START_STOP_VALUE_START   0
#define WM_CMD_START_STOP_VALUE_STOP    1

typedef enum
{
	WM_START_STOP_CLOSED    = 0, //closed
	WM_START_STOP_STANDBY   = 1, //standby
	WM_START_STOP_WORKING   = 2, //working
    WM_START_STOP_SUSPENDED = 3, //suspended
    WM_START_STOP_FINISHED  = 4, //finished
    WM_START_STOP_UNKNOWN
}acw_wm_work_state_t;

#define ACW_WM_WORK_STATE_INIT  {"Closed", \
                                 "Ready", \
                                 "Spin", \
                                 "Pause", \
                                 "Done"}

static inline char *mqtt_get_wm_work_state_str(acw_wm_work_state_t state)
{
    char *wm_work_state_str[WM_START_STOP_UNKNOWN] = ACW_WM_WORK_STATE_INIT;

    if (state >= WM_START_STOP_UNKNOWN)
    {
        return "Unknown";
    }

    return wm_work_state_str[state];
}

typedef enum
{
	PP_CMD_ON_OFF 	    = 0, //开关
    PP_CMD_UNKNWON
}acw_pp_cmd_t;

#define ACW_PP_CMD_INIT       {"on_off"}

static inline char *mqtt_get_pp_cmd_str(acw_pp_cmd_t cmd)
{
    char *pp_cmd_str[PP_CMD_UNKNWON] = ACW_PP_CMD_INIT;

    if (cmd >= PP_CMD_UNKNWON)
    {
        return "Unknown";
    }

    return pp_cmd_str[cmd];
}

static inline acw_pp_cmd_t mqtt_trans_str_to_pp_cmd(char *cmd_str)
{
    char *pp_cmd_str[PP_CMD_UNKNWON] = ACW_PP_CMD_INIT;
    int index;

    for (index = 0; index < PP_CMD_UNKNWON; index++)
    {
        if (!strcmp(cmd_str, pp_cmd_str[index]))
        {
            break;
        }
    }

    return (acw_pp_cmd_t)index;
}

#define PP_CMD_ON_OFF_VALUE_ON  0
#define PP_CMD_ON_OFF_VALUE_OFF 1

typedef enum
{
	LP_CMD_ON_OFF 	    = 0, //开关
    LP_CMD_BRIGHTNESS   = 1, //亮度
    LP_CMD_UNKNWON
}acw_lp_cmd_t;

#define ACW_LP_CMD_INIT       {"on_off", \
                               "brightness"}

static inline char *mqtt_get_lp_cmd_str(acw_lp_cmd_t cmd)
{
    char *lp_cmd_str[LP_CMD_UNKNWON] = ACW_LP_CMD_INIT;

    if (cmd >= LP_CMD_UNKNWON)
    {
        return "Unknown";
    }

    return lp_cmd_str[cmd];
}

static inline acw_lp_cmd_t mqtt_trans_str_to_lp_cmd(char *cmd_str)
{
    char *lp_cmd_str[LP_CMD_UNKNWON] = ACW_LP_CMD_INIT;
    int index;

    for (index = 0; index < LP_CMD_UNKNWON; index++)
    {
        if (!strcmp(cmd_str, lp_cmd_str[index]))
        {
            break;
        }
    }

    return (acw_lp_cmd_t)index;
}

#define LP_CMD_ON_OFF_VALUE_ON  0
#define LP_CMD_ON_OFF_VALUE_OFF 1

#define LP_CMD_BRIGHTNESS_VALUE_MIN  0
#define LP_CMD_BRIGHTNESS_VALUE_MAX  100

extern void mqtt_start_proc(void);
extern os_err_t mqtt_send_msg(char *msg, int msg_len, mqtt_topic_item_t topic, uint8_t qos, os_uint32_t timeout_ms);

#endif /* end of __MQTT_CLIENT_H__ */
