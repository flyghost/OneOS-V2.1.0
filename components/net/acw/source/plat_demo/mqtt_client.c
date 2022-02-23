/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTOneOS.c
 *
 * \@brief       socket port file for mqtt
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_assert.h>
#include <oneos_config.h>
#include <os_mutex.h>
#include <os_sem.h>
#include <os_list.h>
#include <os_clock.h>
#include <stdio.h>
#include <os_errno.h>
#include <os_timer.h>

#ifdef OS_USING_ST7789VW
#include <st7789vw.h>
#endif

#include "MQTTOneOS.h"
#include "MQTTClient.h"

#include "acw_prot_common.h"
#include "acw_conf.h"
#include "acw_intf.h"
#include "mqtt_client.h"
#include "acw_debug.h"
#include "led_key.h"

#ifdef BOARD_BK7231N
#include "hlw8110.h"
#endif

#define ACW_MQTT_SRV_ADDR_MAX_LEN           32
#define ACW_MQTT_USR_NAME_MAX_LEN           32
#define ACW_MQTT_PASSWD_MAX_LEN             32

#define COMMAND_TIMEOUT 30000

#ifdef MQTT_USING_TLS
#define MQTT_TASK_STACK_SIZE 8192   /* MQTT using tls, MQTT thread stack size need larger than 6K */
#else
#define MQTT_TASK_STACK_SIZE 4096
#endif

#define ACW_AUTH_FORWARD_TIMEOUT            (50 * 1000)
#define ACW_AUTH_FORWARD_INTERVAL           (1000)

#define MQTT_TOPIC_MAX_LEN                  64

#define MQTT_RETRY_CNT_DEF                  2

#define MQTT_REQ_ID_MAX_LEN                 32

typedef struct mqtt_msg_to_send_ctrl
{
    os_list_node_t dlist;
    MQTTMessage message;
    char *topic;
    os_tick_t tick_timeout;     // 0, forever; > 0 (ms)
    os_int8_t sended_cnt;       // 已经尝试发送次数;
    char in_payload[1];
} mqtt_msg_to_send_ctrl_t;

typedef struct
{
    Network network;
    MQTTClient client;
} mqtt_context_t;

typedef struct 
{
    char req_id[MQTT_REQ_ID_MAX_LEN + 1];
    os_uint8_t cmd;
    os_uint16_t cmd_value;
}cmd_msg_t;

typedef struct 
{
    os_uint8_t on_off;
    os_uint8_t temperature;
    os_uint8_t work_mode;
    os_uint8_t speed;
}ac_run_ctrl_t;

typedef struct 
{
    os_uint8_t on_off;
    os_uint8_t work_mode;
    os_uint8_t start_stop;
    os_uint8_t work_state;
    os_uint8_t do_times;
    os_timer_t working_timer;
    os_mutex_t mtx;
}wm_run_ctrl_t;

typedef struct
{
    os_uint8_t on_off;
    os_uint8_t report_power_consumption;
    os_tick_t report_tick;
}pp_run_ctrl_t;

typedef struct
{
    os_uint8_t on_off;
    os_uint8_t brightness;  //default 50%
}lp_run_ctrl_t;

static ac_run_ctrl_t gs_ac_run_ctrl;
static wm_run_ctrl_t gs_wm_run_ctrl;
static pp_run_ctrl_t gs_pp_run_ctrl;
static lp_run_ctrl_t gs_lp_run_ctrl;

static os_task_t *gs_acw_mqtt_task = OS_NULL;
static os_mutex_t gs_mqtt_send_mtx;
//static os_sem_t gs_mqtt_send_sem;
static os_list_node_t gs_send_list;

static os_mutex_t gs_mqtt_auth_mtx;

static char gs_foward_auth_topic[MQTT_TOPIC_MAX][MQTT_TOPIC_MAX_LEN + 1];
static os_bool_t gs_acw_release = OS_FALSE;

static mqtt_msg_to_send_ctrl_t *mqtt_msg_buf_alloc(int len)
{
    mqtt_msg_to_send_ctrl_t *temp;

    len += sizeof(mqtt_msg_to_send_ctrl_t);
    temp = malloc(len);
    if (OS_NULL == temp)
    {
        return OS_NULL;
    }

    memset(temp, 0, len);
    if (len > 0)
    {
        temp->message.payload = temp->in_payload;
    }

    os_list_init(&temp->dlist);

    return temp;
}

static void mqtt_msg_buf_free(mqtt_msg_to_send_ctrl_t *msg)
{
    if (OS_NULL != msg->message.payload && msg->message.payload != msg->in_payload)
    {
        free(msg->message.payload);
        msg->message.payload = OS_NULL;
    }
    
    msg->topic = OS_NULL;
    free(msg);

    return;
}

static void message_zero_conf_resp(MessageData *data)
{
    acw_passwd_enc_type_t enc_type;
    char *key;

    ACW_PRINT_I("zero conf resp message arrived on topic %.*s: %.*s",
            data->topicName->lenstring.len,
            data->topicName->lenstring.data,
            data->message->payloadlen,
            data->message->payload);

    char ret_devid[ACW_DEV_ID_LEN + 1];
    memcpy(ret_devid, data->message->payload, ACW_DEV_ID_LEN);
    ret_devid[ACW_DEV_ID_LEN] = '\0';

    if (OS_NULL != strstr(data->message->payload, "cancel"))
    {
        acw_dev_req_add_home_result_sync(ret_devid, OS_FALSE, ACW_PASSWD_ENC_NONE, OS_NULL);
    }
    else if (OS_NULL != strstr(data->message->payload, "ok"))
    {
        enc_type = ACW_PASSWD_ENC_NONE;
        key = OS_NULL;
        if (OS_NULL != strstr(data->message->payload, "enc_type=hash"))
        {
            enc_type = ACW_PASSWD_ENC_HASH_KEY;
        }
        else if (OS_NULL != strstr(data->message->payload, "enc_type=pri"))
        {
            enc_type = ACW_PASSWD_ENC_PRIVATE_KEY;
            key = strstr(data->message->payload, "key=");
            if (OS_NULL != key)
            {
                key += strlen("key=");
            }
        }
        acw_dev_req_add_home_result_sync(ret_devid, OS_TRUE, enc_type, key);
    }
    else
    {
        (void)0;
    }
}

static os_err_t mqtt_ac_cmd_msg_parase(char *msg, int msg_len, cmd_msg_t *cmd_msg)
{
    char cmd[DEV_CTRL_CMD_MAX_LEN + 1];
    char *msg_temp;
    int item_cnt;
    int cmd_value;
    //int req_id; 
    
    msg_temp = malloc(msg_len + 1);
    if (OS_NULL == msg_temp)
    {
        return OS_ERROR;
    }

    memset(msg_temp, 0, msg_len + 1);
    memcpy(msg_temp, msg, msg_len);

    item_cnt = sscanf(msg_temp, "req_id:%[^,],cmd:%[^,],value:%d",
                      cmd_msg->req_id, cmd, &cmd_value);

    free(msg_temp);
    if (3 != item_cnt)
    {
        return OS_ERROR;
    }

    cmd_msg->cmd = mqtt_trans_str_to_ac_cmd(cmd);
    cmd_msg->cmd_value = cmd_value;
    
    ACW_PRINT_I("ac, req_id:%s,cmd:%s,value:%u", cmd_msg->req_id, cmd, cmd_msg->cmd_value);
    
    return OS_EOK;
}

#define DEV_LCD_SHOW_SIZE   32
#define DEV_TITLE_DIFF      16

#define DEV_TITLE_START     DEV_TITLE_DIFF
#define DEV_ON_OFF_START    (DEV_TITLE_START + DEV_LCD_SHOW_SIZE + DEV_TITLE_DIFF)
#define DEV_TEMP_START      (DEV_ON_OFF_START + DEV_LCD_SHOW_SIZE)
#define DEV_MODE_START      (DEV_TEMP_START + DEV_LCD_SHOW_SIZE)
#define DEV_SPEED_START     (DEV_MODE_START + DEV_LCD_SHOW_SIZE)

static void ac_do_fresh_lcd(void)
{
#ifdef OS_USING_ST7789VW
    static os_bool_t fresh = OS_FALSE;
    char show_item[32];
    char *val;
    
    if (OS_FALSE == fresh)
    {
        lcd_clear(WHITE);
        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);
        fresh = OS_TRUE;
    }

    if (OS_FALSE == gs_acw_release)
    {
        snprintf(show_item, sizeof(show_item), "%s", acw_conf_get_devid());
        lcd_show_string(48, DEV_TITLE_START, DEV_LCD_SHOW_SIZE, show_item);
    }
    else
    {
        memset(show_item, ' ', 14);
        show_item[14] = 0;
        lcd_show_string(0, DEV_TITLE_START, DEV_LCD_SHOW_SIZE, show_item);
    }
 
    snprintf(show_item, sizeof(show_item), "on-off:%-3s", (gs_ac_run_ctrl.on_off == AC_CMD_ON_OFF_VALUE_ON) ? "on" : "off");
    lcd_show_string(0, DEV_ON_OFF_START, DEV_LCD_SHOW_SIZE, show_item);
    snprintf(show_item, sizeof(show_item), "temp  :%-2u C", gs_ac_run_ctrl.temperature);
    lcd_show_string(0, DEV_TEMP_START, DEV_LCD_SHOW_SIZE, show_item);
    
    val = mqtt_get_ac_work_mode_str((acw_ac_work_mode_t)gs_ac_run_ctrl.work_mode);
    os_snprintf(show_item, sizeof(show_item), "mode  :%-7s", val);
    lcd_show_string(0, DEV_MODE_START, DEV_LCD_SHOW_SIZE, show_item);
    
    val = mqtt_get_ac_speed_str((acw_ac_speed_t)gs_ac_run_ctrl.speed);
    os_snprintf(show_item, sizeof(show_item), "speed :%-7s", val);
    lcd_show_string(0, DEV_SPEED_START, DEV_LCD_SHOW_SIZE, show_item);
#endif

    return;
}

static void mqtt_do_ac_on_off(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (AC_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value || AC_CMD_ON_OFF_VALUE_OFF == cmd_msg->cmd_value)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_ac_run_ctrl.on_off = cmd_msg->cmd_value;
        ac_do_fresh_lcd();      
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }

    return;
}

static void mqtt_do_ac_temperature(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (AC_CMD_ON_OFF_VALUE_ON != gs_ac_run_ctrl.on_off)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:op not permit",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);  
        return;
    }

    if (cmd_msg->cmd_value < AC_CMD_TEMPERATURE_VALUE_MIN || cmd_msg->cmd_value > AC_CMD_TEMPERATURE_VALUE_MAX)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);  
    }
    else
    {
        gs_ac_run_ctrl.temperature = cmd_msg->cmd_value;
        ac_do_fresh_lcd(); 
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
    }

    return;
}

static void mqtt_do_ac_work_mode(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (AC_CMD_ON_OFF_VALUE_ON != gs_ac_run_ctrl.on_off)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:op not permit",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);  
        return;
    }

    if (cmd_msg->cmd_value >= AC_WORK_MODE_UNKNOWN)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_ac_run_ctrl.work_mode = cmd_msg->cmd_value;
        ac_do_fresh_lcd();
    }
    
    return;
}

static void mqtt_do_ac_speed(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (AC_CMD_ON_OFF_VALUE_ON != gs_ac_run_ctrl.on_off)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:op not permit",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);  
        return;
    }

    if (cmd_msg->cmd_value >= AC_SPEED_UNKNOWN)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_ac_run_ctrl.speed = cmd_msg->cmd_value;
        ac_do_fresh_lcd();
    }
    
    return;
}

static void mqtt_do_ac_cmd(MessageData *data)
{
    char msg_resp[256];
    cmd_msg_t cmd_msg;
    os_err_t do_err;

    ACW_PRINT_I("dev_ctrl cmd arrive");

    do_err = mqtt_ac_cmd_msg_parase(data->message->payload, data->message->payloadlen, &cmd_msg);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_I("cmd msg invalid");
        return;
    }

    ACW_PRINT_I("req_id:%s,cmd:%u,value:%u", cmd_msg.req_id, cmd_msg.cmd, cmd_msg.cmd_value);

    switch (cmd_msg.cmd)
    {
    case AC_CMD_ON_OFF:
        mqtt_do_ac_on_off(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case AC_CMD_TEMPERATURE:
        mqtt_do_ac_temperature(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case AC_CMD_WORK_MODE:
        mqtt_do_ac_work_mode(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case AC_CMD_SPEED:
        mqtt_do_ac_speed(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    default:
        snprintf(msg_resp, sizeof(msg_resp), "req_id:%s,cmd:%s,value:%d,success:false,message:cmd invalid",
                cmd_msg.req_id, mqtt_get_ac_cmd_str((acw_ac_cmd_t)cmd_msg.cmd), cmd_msg.cmd_value);
        break;
    }

    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_CMD_RESP, QOS1, 2000);

    return;
}

#define DEV_WM_LCD_SHOW_SIZE        32
#define DEV_WM_TITLE_DIFF           16

#define DEV_WM_TITLE_START     DEV_WM_TITLE_DIFF
#define DEV_TIME_START         (DEV_WM_TITLE_START + DEV_WM_LCD_SHOW_SIZE + DEV_WM_TITLE_DIFF)
#define DEV_WM_STATE_START     (DEV_TIME_START + DEV_WM_LCD_SHOW_SIZE + DEV_WM_TITLE_DIFF)
#define DEV_WM_MODE_START      (DEV_WM_STATE_START + DEV_WM_LCD_SHOW_SIZE + DEV_WM_TITLE_DIFF)

static void lcd_show_line_middle(char *buf, char *msg)
{
    int temp;
    int len;

    len = strlen(msg);
    if (len > 14)
    {
        len = 14;
    }

    memset(buf, ' ', 14);
    buf[14] = 0;

    temp = (14 - len) / 2;
    memcpy(buf + temp, msg, len);

    return;
}

static void wm_do_fresh_lcd(void)
{
#ifdef OS_USING_ST7789VW
    static os_bool_t fresh = OS_FALSE;
    char show_item[16];
    char *val;

    if (OS_FALSE == fresh)
    {
        lcd_clear(WHITE);
        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);
        fresh = OS_TRUE;
    }

    if (OS_FALSE == gs_acw_release)
    {
        snprintf(show_item, sizeof(show_item), "%s", acw_conf_get_devid());
        lcd_show_string(48, DEV_WM_TITLE_START, DEV_WM_LCD_SHOW_SIZE, show_item);
    }
    else
    {
        memset(show_item, ' ', 14);
        show_item[14] = 0;
        lcd_show_string(0, DEV_WM_TITLE_START, DEV_WM_LCD_SHOW_SIZE, show_item);
    }
  
    if(WM_START_STOP_SUSPENDED == gs_wm_run_ctrl.work_state || WM_START_STOP_WORKING == gs_wm_run_ctrl.work_state)
    {
        snprintf(show_item, sizeof(show_item), "       %02d       ", gs_wm_run_ctrl.do_times);
    }
    else
    {
        snprintf(show_item, sizeof(show_item), "       --       ");
    }
    lcd_show_string(0, DEV_TIME_START, DEV_WM_LCD_SHOW_SIZE, show_item);

    val = mqtt_get_wm_work_state_str((acw_wm_work_state_t)gs_wm_run_ctrl.work_state);
    lcd_show_line_middle(show_item, val);
    //snprintf(show_item, sizeof(show_item), "   %-11s", val);
    lcd_show_string(0, DEV_WM_STATE_START, DEV_WM_LCD_SHOW_SIZE, show_item);

    val = mqtt_get_wm_work_mode_str((acw_wm_work_mode_t)gs_wm_run_ctrl.work_mode);
    lcd_show_line_middle(show_item, val);
    lcd_show_string(0, DEV_WM_MODE_START, DEV_WM_LCD_SHOW_SIZE, show_item);
#endif

    return;
}

#define DEV_PP_LCD_SHOW_SIZE        32
#define DEV_PP_TITLE_DIFF           16

#define DEV_PP_TITLE_START     DEV_PP_TITLE_DIFF
#define DEV_PP_ON_OFF_START    (DEV_PP_TITLE_START + DEV_PP_LCD_SHOW_SIZE + DEV_PP_TITLE_DIFF)
#define DEV_PP_DAY_START       (DEV_PP_ON_OFF_START + DEV_PP_LCD_SHOW_SIZE + DEV_PP_TITLE_DIFF)
#define DEV_PP_MON_START       (DEV_PP_DAY_START + DEV_PP_LCD_SHOW_SIZE + DEV_PP_TITLE_DIFF)
#define DEV_PP_ALL_START       (DEV_PP_MON_START + DEV_PP_LCD_SHOW_SIZE + DEV_PP_TITLE_DIFF)

static void pp_do_fresh_lcd(void)
{
#ifdef OS_USING_ST7789VW
    static os_bool_t fresh = OS_FALSE;
    char show_item[16];

    if (OS_FALSE == fresh)
    {
        lcd_clear(WHITE);
        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);
        fresh = OS_TRUE;
    }

    if (OS_FALSE == gs_acw_release)
    {
        snprintf(show_item, sizeof(show_item), "%s", acw_conf_get_devid());
        lcd_show_string(48, DEV_PP_TITLE_START, DEV_PP_LCD_SHOW_SIZE, show_item);
    }
    else
    {
        memset(show_item, ' ', 14);
        show_item[14] = 0;
        lcd_show_string(0, DEV_PP_TITLE_START, DEV_PP_LCD_SHOW_SIZE, show_item);
    }
  
    snprintf(show_item, sizeof(show_item), "on-off:%-3s", (gs_pp_run_ctrl.on_off == PP_CMD_ON_OFF_VALUE_ON) ? "on" : "off");
    lcd_show_string(0, DEV_PP_ON_OFF_START, DEV_PP_LCD_SHOW_SIZE, show_item);
#endif

    return;
}

#define DEV_LP_LCD_SHOW_SIZE        32
#define DEV_LP_TITLE_DIFF           16

#define DEV_LP_TITLE_START      DEV_LP_TITLE_DIFF
#define DEV_LP_ON_OFF_START     (DEV_LP_TITLE_START + DEV_LP_LCD_SHOW_SIZE + DEV_LP_TITLE_DIFF)
#define DEV_LP_BRIGHTNESS_START (DEV_LP_ON_OFF_START + DEV_LP_LCD_SHOW_SIZE + DEV_LP_TITLE_DIFF)

static void lp_do_fresh_lcd(void)
{
#ifdef OS_USING_ST7789VW
    static os_bool_t fresh = OS_FALSE;
    char show_item[16];

    if (OS_FALSE == fresh)
    {
        lcd_clear(WHITE);
        /* set the background color and foreground color */
        lcd_set_color(WHITE, BLACK);
        fresh = OS_TRUE;
    }

    if (OS_FALSE == gs_acw_release)
    {
        snprintf(show_item, sizeof(show_item), "%s", acw_conf_get_devid());
        lcd_show_string(48, DEV_LP_TITLE_START, DEV_WM_LCD_SHOW_SIZE, show_item);
    }
    else
    {
        memset(show_item, ' ', 14);
        show_item[14] = 0;
        lcd_show_string(0, DEV_LP_TITLE_START, DEV_WM_LCD_SHOW_SIZE, show_item);
    }

    snprintf(show_item, sizeof(show_item), "on-off:%-3s", (gs_lp_run_ctrl.on_off == LP_CMD_ON_OFF_VALUE_ON) ? "on" : "off");
    lcd_show_string(0, DEV_LP_ON_OFF_START, DEV_LP_LCD_SHOW_SIZE, show_item);

    snprintf(show_item, sizeof(show_item), "br-ns :%-3d%%", gs_lp_run_ctrl.brightness);
    lcd_show_string(0, DEV_LP_BRIGHTNESS_START, DEV_LP_LCD_SHOW_SIZE, show_item);
#endif
    return;
}

static void mqtt_do_wm_on_off(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    os_mutex_lock(&gs_wm_run_ctrl.mtx, OS_WAIT_FOREVER);

    do
    {
        if (WM_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value || WM_CMD_ON_OFF_VALUE_OFF == cmd_msg->cmd_value)
        {
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);

            if (WM_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value)
            {
                if (WM_CMD_ON_OFF_VALUE_ON == gs_wm_run_ctrl.on_off)
                {
                    break;
                }
                gs_wm_run_ctrl.on_off = WM_CMD_ON_OFF_VALUE_ON;
                gs_wm_run_ctrl.work_state = WM_START_STOP_STANDBY;
            }
            else
            {
                if (WM_CMD_START_STOP_VALUE_STOP == gs_wm_run_ctrl.on_off)
                {
                    break;
                }
                os_timer_stop(&gs_wm_run_ctrl.working_timer);
                gs_wm_run_ctrl.on_off = WM_CMD_START_STOP_VALUE_STOP;
                gs_wm_run_ctrl.work_state = WM_START_STOP_CLOSED;
            }

            wm_do_fresh_lcd();      
        }
        else
        {
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
        }
    } while (0);

    os_mutex_unlock(&gs_wm_run_ctrl.mtx);

    return;
}

static void mqtt_do_wm_work_mode(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    os_mutex_lock(&gs_wm_run_ctrl.mtx, OS_WAIT_FOREVER);

    do
    {
        if (WM_START_STOP_WORKING == gs_wm_run_ctrl.work_state || WM_START_STOP_SUSPENDED == gs_wm_run_ctrl.work_state)
        {
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:busy",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
            break;
        }

        if (cmd_msg->cmd_value >= WM_WORK_MODE_UNKNOWN)
        {
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
            break;        
        }

        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_wm_run_ctrl.work_mode = cmd_msg->cmd_value;
        wm_do_fresh_lcd();      
    } while (0);

    os_mutex_unlock(&gs_wm_run_ctrl.mtx);

    return;
}

#define WM_WORK_CNT 20

static void mqtt_do_wm_start_stop(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    os_mutex_lock(&gs_wm_run_ctrl.mtx, OS_WAIT_FOREVER);

    do
    {
        if (WM_CMD_START_STOP_VALUE_START == cmd_msg->cmd_value || WM_CMD_START_STOP_VALUE_STOP == cmd_msg->cmd_value)
        {
            if (WM_CMD_ON_OFF_VALUE_ON != gs_wm_run_ctrl.on_off)
            {
                snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:not power on",
                            cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
                break;
            }

            if (WM_CMD_START_STOP_VALUE_START == cmd_msg->cmd_value)
            {
                if (WM_START_STOP_STANDBY == gs_wm_run_ctrl.work_state || WM_START_STOP_FINISHED == gs_wm_run_ctrl.work_state)
                {
                    gs_wm_run_ctrl.do_times = WM_WORK_CNT;
                    gs_wm_run_ctrl.work_state = WM_START_STOP_WORKING;
                    os_timer_start(&gs_wm_run_ctrl.working_timer);
                }
                else if (WM_START_STOP_SUSPENDED == gs_wm_run_ctrl.work_state)
                {   
                    gs_wm_run_ctrl.work_state = WM_START_STOP_WORKING;
                    os_timer_start(&gs_wm_run_ctrl.working_timer);
                }
                else
                {
                    (void)0;
                }
                gs_wm_run_ctrl.start_stop = WM_CMD_START_STOP_VALUE_START;
            } 
            else
            {
                if  (WM_START_STOP_WORKING == gs_wm_run_ctrl.work_state)
                {
                    os_timer_stop(&gs_wm_run_ctrl.working_timer);
                    gs_wm_run_ctrl.work_state = WM_START_STOP_SUSPENDED;
                }
                else if (WM_START_STOP_FINISHED == gs_wm_run_ctrl.work_state)
                {
                    gs_wm_run_ctrl.work_state = WM_START_STOP_STANDBY;
                }
                else
                {
                    (void)0;
                }
                gs_wm_run_ctrl.start_stop = WM_CMD_START_STOP_VALUE_STOP;
            }
            wm_do_fresh_lcd();
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,state:%d,success:true,message:0",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value, gs_wm_run_ctrl.work_state);
        }
        else
        {
            snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                        cmd_msg->req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        }
    } while (0);

    os_mutex_unlock(&gs_wm_run_ctrl.mtx);

    return;
}

static void mqtt_wm_work_func(void *parameter)
{	
    char mqtt_msg[64];

    os_mutex_lock(&gs_wm_run_ctrl.mtx, OS_WAIT_FOREVER);

    gs_wm_run_ctrl.do_times--;
    if (!gs_wm_run_ctrl.do_times)
    {
        gs_wm_run_ctrl.work_state = WM_START_STOP_FINISHED;
        os_timer_stop(&gs_wm_run_ctrl.working_timer);
        snprintf(mqtt_msg, sizeof(mqtt_msg), "Do [%s] finished", mqtt_get_wm_work_mode_str((acw_wm_work_mode_t)gs_wm_run_ctrl.work_mode));
        mqtt_send_msg(mqtt_msg, strlen(mqtt_msg), MQTT_TOPIC_FINISHED, 1, 10 * 60 * 1000);
    }
    wm_do_fresh_lcd();

    os_mutex_unlock(&gs_wm_run_ctrl.mtx);
    
    return;
}

static os_err_t mqtt_wm_cmd_msg_parase(char *msg, int msg_len, cmd_msg_t *cmd_msg)
{
    char cmd[DEV_CTRL_CMD_MAX_LEN + 1];
    char *msg_temp;
    int item_cnt;
    int cmd_value;
    //int req_id; 
    
    msg_temp = malloc(msg_len + 1);
    if (OS_NULL == msg_temp)
    {
        return OS_ERROR;
    }

    memset(msg_temp, 0, msg_len + 1);
    memcpy(msg_temp, msg, msg_len);

    item_cnt = sscanf(msg_temp, "req_id:%[^,],cmd:%[^,],value:%d",
                      cmd_msg->req_id, cmd, &cmd_value);

    free(msg_temp);
    if (3 != item_cnt)
    {
        return OS_ERROR;
    }

    //cmd_msg->req_id = req_id;
    cmd_msg->cmd = mqtt_trans_str_to_wm_cmd(cmd);
    cmd_msg->cmd_value = cmd_value;
    
    ACW_PRINT_I("req_id:%s,cmd:%s,value:%u", cmd_msg->req_id, cmd, cmd_msg->cmd_value);
    
    return OS_EOK;
}

static void mqtt_do_wm_cmd(MessageData *data)
{
    char msg_resp[256];
    cmd_msg_t cmd_msg;
    os_err_t do_err;

    ACW_PRINT_I("dev_ctrl cmd arrive");

    do_err = mqtt_wm_cmd_msg_parase(data->message->payload, data->message->payloadlen, &cmd_msg);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_I("cmd msg invalid");
        return;
    }

    ACW_PRINT_I("req_id:%s,cmd:%u,value:%u", cmd_msg.req_id, cmd_msg.cmd, cmd_msg.cmd_value);

    switch (cmd_msg.cmd)
    {
    case WM_CMD_ON_OFF:
        mqtt_do_wm_on_off(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case WM_CMD_WORK_MODE:
        mqtt_do_wm_work_mode(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case WM_CMD_START_STOP:
        mqtt_do_wm_start_stop(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    default:
        ACW_PRINT_I("msg cmd invalid");
        snprintf(msg_resp, sizeof(msg_resp), "req_id:%s,cmd:%s,value:%d,success:false,message:cmd invalid",
                cmd_msg.req_id, mqtt_get_wm_cmd_str((acw_wm_cmd_t)cmd_msg.cmd), cmd_msg.cmd_value);
        break;
    }

    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_CMD_RESP, QOS1, 2000);

    return;
}

static os_err_t mqtt_pp_cmd_msg_parase(char *msg, int msg_len, cmd_msg_t *cmd_msg)
{
    char cmd[DEV_CTRL_CMD_MAX_LEN + 1];
    char *msg_temp;
    int item_cnt;
    int cmd_value;
    //int req_id; 
    
    msg_temp = malloc(msg_len + 1);
    if (OS_NULL == msg_temp)
    {
        return OS_ERROR;
    }

    memset(msg_temp, 0, msg_len + 1);
    memcpy(msg_temp, msg, msg_len);

    item_cnt = sscanf(msg_temp, "req_id:%[^,],cmd:%[^,],value:%d",
                      cmd_msg->req_id, cmd, &cmd_value);

    free(msg_temp);
    if (3 != item_cnt)
    {
        return OS_ERROR;
    }

    cmd_msg->cmd = mqtt_trans_str_to_pp_cmd(cmd);
    cmd_msg->cmd_value = cmd_value;
    
    ACW_PRINT_I("pp, req_id:%s,cmd:%s,value:%u", cmd_msg->req_id, cmd, cmd_msg->cmd_value);
    
    return OS_EOK;
}

static void mqtt_do_pp_on_off(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (PP_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value || PP_CMD_ON_OFF_VALUE_OFF == cmd_msg->cmd_value)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_pp_cmd_str((acw_pp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_pp_run_ctrl.on_off = cmd_msg->cmd_value;
        if (PP_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value)
        {
            os_kprintf("do power plug open\r\n");
            acw_powerplug_open();
        }
        else
        {
            os_kprintf("do power plug close\r\n");
            acw_powerplug_close();
        }
        pp_do_fresh_lcd();      
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_pp_cmd_str((acw_pp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }

    return;
}

static void mqtt_do_pp_cmd(MessageData *data)
{
    char msg_resp[256];
    cmd_msg_t cmd_msg;
    os_err_t do_err;

    ACW_PRINT_I("dev_ctrl cmd arrive");

    do_err = mqtt_pp_cmd_msg_parase(data->message->payload, data->message->payloadlen, &cmd_msg);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_I("cmd msg invalid");
        return;
    }

    ACW_PRINT_I("req_id:%s,cmd:%u,value:%u", cmd_msg.req_id, cmd_msg.cmd, cmd_msg.cmd_value);

    switch (cmd_msg.cmd)
    {
    case PP_CMD_ON_OFF:
        mqtt_do_pp_on_off(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    default:
        snprintf(msg_resp, sizeof(msg_resp), "req_id:%s,cmd:%s,value:%d,success:false,message:cmd invalid",
                cmd_msg.req_id, mqtt_get_pp_cmd_str((acw_pp_cmd_t)cmd_msg.cmd), cmd_msg.cmd_value);
        break;
    }

    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_CMD_RESP, QOS1, 2000);

    return;
}

static os_err_t mqtt_lp_cmd_msg_parase(char *msg, int msg_len, cmd_msg_t *cmd_msg)
{
    char cmd[DEV_CTRL_CMD_MAX_LEN + 1];
    char *msg_temp;
    int item_cnt;
    int cmd_value;
    //int req_id; 
    
    msg_temp = malloc(msg_len + 1);
    if (OS_NULL == msg_temp)
    {
        return OS_ERROR;
    }

    memset(msg_temp, 0, msg_len + 1);
    memcpy(msg_temp, msg, msg_len);

    item_cnt = sscanf(msg_temp, "req_id:%[^,],cmd:%[^,],value:%d",
                      cmd_msg->req_id, cmd, &cmd_value);

    free(msg_temp);
    if (3 != item_cnt)
    {
        return OS_ERROR;
    }

    cmd_msg->cmd = mqtt_trans_str_to_lp_cmd(cmd);
    cmd_msg->cmd_value = cmd_value;
    
    ACW_PRINT_I("lp, req_id:%s,cmd:%s,value:%u", cmd_msg->req_id, cmd, cmd_msg->cmd_value);
    
    return OS_EOK;
}

static void mqtt_do_lp_on_off(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (LP_CMD_ON_OFF_VALUE_ON == cmd_msg->cmd_value || LP_CMD_ON_OFF_VALUE_OFF == cmd_msg->cmd_value)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_lp_run_ctrl.on_off = cmd_msg->cmd_value;
        lp_do_fresh_lcd();      
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }

    return;
}

static void mqtt_do_lp_brightness(cmd_msg_t *cmd_msg, char *resp, int max_resp_len)
{
    if (gs_lp_run_ctrl.on_off != LP_CMD_ON_OFF_VALUE_ON)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:power plug is off",
                    cmd_msg->req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);    
        return;
    }

    if (cmd_msg->cmd_value > LP_CMD_BRIGHTNESS_VALUE_MAX)
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:false,message:value invalid",
                    cmd_msg->req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);        
    }
    else
    {
        snprintf(resp, max_resp_len, "req_id:%s,cmd:%s,value:%d,success:true,message:0",
                    cmd_msg->req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg->cmd), cmd_msg->cmd_value);
        gs_lp_run_ctrl.brightness = cmd_msg->cmd_value;
        lp_do_fresh_lcd();      
    }

    return;
}

static void mqtt_do_lp_cmd(MessageData *data)
{
    char msg_resp[256];
    cmd_msg_t cmd_msg;
    os_err_t do_err;

    ACW_PRINT_I("dev_ctrl cmd arrive");

    do_err = mqtt_lp_cmd_msg_parase(data->message->payload, data->message->payloadlen, &cmd_msg);
    if (OS_EOK != do_err)
    {
        ACW_PRINT_I("cmd msg invalid");
        return;
    }

    ACW_PRINT_I("req_id:%s,cmd:%u,value:%u", cmd_msg.req_id, cmd_msg.cmd, cmd_msg.cmd_value);

    switch (cmd_msg.cmd)
    {
    case LP_CMD_ON_OFF:
        mqtt_do_lp_on_off(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    case LP_CMD_BRIGHTNESS:
        mqtt_do_lp_brightness(&cmd_msg, msg_resp, sizeof(msg_resp));
        break;
    default:
        snprintf(msg_resp, sizeof(msg_resp), "req_id:%s,cmd:%s,value:%d,success:false,message:cmd invalid",
                cmd_msg.req_id, mqtt_get_lp_cmd_str((acw_lp_cmd_t)cmd_msg.cmd), cmd_msg.cmd_value);
        break;
    }

    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_CMD_RESP, QOS1, 2000);

    return;
}

static void message_cmd_arrived(MessageData *data)
{
    acw_dev_type_t dev_type;

    dev_type = acw_get_dev_type();
    if (DEV_TYPE_AC == dev_type)
    {
        mqtt_do_ac_cmd(data);
    }
    else if(DEV_TYPE_WM == dev_type)
    {
        mqtt_do_wm_cmd(data);
    }
    else if (DEV_TYPE_PP == dev_type)
    {
        mqtt_do_pp_cmd(data);
    }
    else if (DEV_TYPE_LP == dev_type)
    {
        mqtt_do_lp_cmd(data);
    }
    else
    {
        (void)0;
    }

    return;
}

static void mqtt_do_ac_query(MessageData *data)
{
    char msg_resp[256];
    int len;

    memset(msg_resp, 0, sizeof(msg_resp));
#if 0
    strcpy(msg_resp, "req_id:");
    len = strlen("req_id:");
#else
    len = 0;
#endif

    memcpy(msg_resp + len, data->message->payload, data->message->payloadlen);
    len += data->message->payloadlen;

    snprintf(msg_resp + len, sizeof(msg_resp) - len, ",success:true,%s:%d,%s:%d,%s:%d,%s:%d", 
            mqtt_get_ac_cmd_str(AC_CMD_ON_OFF), gs_ac_run_ctrl.on_off,
            mqtt_get_ac_cmd_str(AC_CMD_TEMPERATURE), gs_ac_run_ctrl.temperature,
            mqtt_get_ac_cmd_str(AC_CMD_WORK_MODE), gs_ac_run_ctrl.work_mode,
            mqtt_get_ac_cmd_str(AC_CMD_SPEED), gs_ac_run_ctrl.speed);
    
    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_QUERY_RESP, QOS1, 2000);
    
    return;
}

static void mqtt_do_wm_query(MessageData *data)
{
    char msg_resp[256];
    int len;

    memset(msg_resp, 0, sizeof(msg_resp));
#if 0
    strcpy(msg_resp, "req_id:");
    len = strlen("req_id:");
#else
    len = 0;
#endif

    memcpy(msg_resp + len, data->message->payload, data->message->payloadlen);
    len += data->message->payloadlen;

    snprintf(msg_resp + len, sizeof(msg_resp) - len, ",success:true,%s:%d,%s:%d,state:%d",
            mqtt_get_wm_cmd_str(WM_CMD_ON_OFF), gs_wm_run_ctrl.on_off, 
            mqtt_get_wm_cmd_str(WM_CMD_WORK_MODE), gs_wm_run_ctrl.work_mode,
            gs_wm_run_ctrl.work_state);
    
    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_QUERY_RESP, QOS1, 2000);

    return;
}

static void mqtt_do_pp_query(MessageData *data)
{
#ifdef BOARD_BK7231N
    hlw8110_actual_result_t hlw8110_act;
    char msg_resp[128];
    int len;
    u32_t power_consumption;

    memset(msg_resp, 0, sizeof(msg_resp));
    len = 0;

    memcpy(msg_resp + len, data->message->payload, data->message->payloadlen);
    len += data->message->payloadlen;
    
    hlw8110_act = get_hlw8110_measurements();

    power_consumption = hlw8110_act.power_a / 10;
    snprintf(msg_resp + len, sizeof(msg_resp) - len, ",success:true,%s:%d,power_consumption:%u",
            mqtt_get_pp_cmd_str(PP_CMD_ON_OFF), gs_pp_run_ctrl.on_off, power_consumption);

    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_QUERY_RESP, QOS1, 2000);
#endif
    return;
}

static void mqtt_do_lp_query(MessageData *data)
{
    char msg_resp[256];
    int len;

    memset(msg_resp, 0, sizeof(msg_resp));
    len = 0;

    memcpy(msg_resp + len, data->message->payload, data->message->payloadlen);
    len += data->message->payloadlen;

    snprintf(msg_resp + len, sizeof(msg_resp) - len, ",success:true,%s:%d,%s:%d",
            mqtt_get_lp_cmd_str(LP_CMD_ON_OFF), gs_lp_run_ctrl.on_off, 
            mqtt_get_lp_cmd_str(LP_CMD_BRIGHTNESS), gs_lp_run_ctrl.brightness);
    
    mqtt_send_msg(msg_resp, strlen(msg_resp), MQTT_TOPIC_QUERY_RESP, QOS1, 2000);

    return;
}

static void message_query_arrived(MessageData *data)
{
    acw_dev_type_t dev_type;

    ACW_PRINT_I("message_query_arrived\r\n");
    dev_type = acw_get_dev_type();
    if (DEV_TYPE_AC == dev_type)
    {
        mqtt_do_ac_query(data);
    }
    else if(DEV_TYPE_WM == dev_type)
    {
        mqtt_do_wm_query(data);
    }
    else if (DEV_TYPE_PP == dev_type)
    {
        mqtt_do_pp_query(data);
    }
    else if (DEV_TYPE_LP == dev_type)
    {
        mqtt_do_lp_query(data);
    }
    else
    {
        (void)0;
    }

    return;
}

/* acw订阅主题 */
static int acw_mqtt_subscribe(MQTTClient* c)
{
    int rc = -1;

    rc = MQTTSubscribe(c, gs_foward_auth_topic[MQTT_TOPIC_CONF_RESP], QOS1, message_zero_conf_resp);
    if (0 != rc)
    {
        ACW_PRINT_I("Return code from MQTT subscribe[%s] is %d", gs_foward_auth_topic[MQTT_TOPIC_CONF_REQ], rc);
        return rc;
    }

    rc = MQTTSubscribe(c, gs_foward_auth_topic[MQTT_TOPIC_CMD], QOS1, message_cmd_arrived);
    if (0 != rc)
    {
        ACW_PRINT_I("Return code from MQTT subscribe[%s] is %d", gs_foward_auth_topic[MQTT_TOPIC_CMD], rc);
        return rc;
    }

    rc = MQTTSubscribe(c, gs_foward_auth_topic[MQTT_TOPIC_QUERY], QOS1, message_query_arrived);
    if (0 != rc)
    {
        ACW_PRINT_I("Return code from MQTT subscribe[%s] is %d", gs_foward_auth_topic[MQTT_TOPIC_QUERY], rc);
        return rc;
    }

    return 0;
}

/* mqtt server重复连接直到成功 */
static void mqtt_do_connect_server(mqtt_context_t *ctx, MQTTPacket_connectData* options)
{
    MQTTClient* c;
    os_bool_t connected;
    os_bool_t clr_key_press;
    os_bool_t red_flicker;
    int rc;

    c = &ctx->client;

    red_flicker = OS_TRUE;
    do
    {
        clr_key_press = acw_check_clr_key_press();
        if (OS_TRUE == clr_key_press)
        {
            acw_clear_conf_notice();
        }

        connected = acw_check_intf_connected();
        if (OS_FALSE == connected)
        {
            os_task_msleep(500);
            if (OS_NULL == acw_conf_get_stored_ssid() || OS_NULL == acw_conf_get_stored_passwd())
            {
                if (OS_TRUE == red_flicker)
                {
                    acw_red_led_close();
                    red_flicker = OS_FALSE;
                }
                else
                {
                    acw_red_led_open();
                    red_flicker = OS_TRUE;
                }
            }
            else
            {
                if (OS_FALSE == red_flicker)
                {
                    acw_red_led_open();
                    red_flicker = OS_TRUE;
                }
            }
            continue; 
        }
        ACW_PRINT_I("Now find intf up");

        MQTTNetworkConnect(&ctx->network);
        rc = MQTTConnect(c, options);
        if (!rc) {
            if (!acw_mqtt_subscribe(c)) {
                ACW_PRINT_I("acw MQTT Subscribe success!");
                break;
            }
        }

        MQTTNetworkDisconnect(&ctx->network);
        os_task_msleep(1000);

    } while (1);

    acw_green_led_open();

    return;
}

static void ac_do_def_init(void)
{
    gs_ac_run_ctrl.on_off = AC_CMD_ON_OFF_VALUE_OFF;
    gs_ac_run_ctrl.temperature = 26;
    gs_ac_run_ctrl.work_mode = AC_WORK_MODE_AUTO;
    gs_ac_run_ctrl.speed = AC_SPEED_AUTO;
}

static void wm_do_def_init(void)
{
    os_mutex_init(&gs_wm_run_ctrl.mtx, "wm_mtx", OS_FALSE);
    gs_wm_run_ctrl.on_off = WM_CMD_ON_OFF_VALUE_OFF;
    gs_wm_run_ctrl.start_stop = WM_CMD_START_STOP_VALUE_STOP;
    gs_wm_run_ctrl.work_state = WM_START_STOP_CLOSED;
    gs_wm_run_ctrl.work_mode = WM_WORK_MODE_MIX;

    os_timer_init(&gs_wm_run_ctrl.working_timer, "wm_work", mqtt_wm_work_func, OS_NULL, os_tick_from_ms(1000), OS_TIMER_FLAG_PERIODIC);
}

static void pp_do_def_init(void)
{
    gs_pp_run_ctrl.on_off = PP_CMD_ON_OFF_VALUE_OFF;
    gs_pp_run_ctrl.report_power_consumption = 0;
    gs_pp_run_ctrl.report_tick = os_tick_get();
#ifdef BOARD_BK7231N	
    hlw8110_init(0);
#endif	
}

static void lp_do_def_init(void)
{
    gs_lp_run_ctrl.on_off = LP_CMD_ON_OFF_VALUE_OFF;
    gs_lp_run_ctrl.brightness = 80;
}

static void mqtt_do_dev_init(void)
{
    acw_dev_type_t dev_type;

    dev_type = acw_get_dev_type();
    if (DEV_TYPE_AC == dev_type)
    {
        ac_do_def_init();
        ac_do_fresh_lcd();
    }
    else if (DEV_TYPE_WM == dev_type)
    {
        wm_do_def_init();
        wm_do_fresh_lcd();
    }
    else if (DEV_TYPE_PP == dev_type)
    {
        pp_do_def_init();
        pp_do_fresh_lcd();
    }
    else if (DEV_TYPE_LP == dev_type)
    {
        lp_do_def_init();
        lp_do_fresh_lcd();
    }
    else
    {
        (void)0;
    }
}

static void mqtt_do_lp_report(void)
{
#ifdef BOARD_BK7231N    
    hlw8110_actual_result_t hlw8110_act;
    os_uint32_t power_con_diff;
    acw_dev_type_t dev_type;
    char mqtt_msg[64];
    os_tick_t diff;
    os_tick_t now;

    dev_type = acw_get_dev_type();
    if (DEV_TYPE_PP != dev_type)
    {
        return;
    }

    hlw8110_act = get_hlw8110_measurements();
    now = os_tick_get();
    diff = now - gs_pp_run_ctrl.report_tick;
    power_con_diff = hlw8110_act.energy_a - gs_pp_run_ctrl.report_power_consumption;

    if (diff > os_tick_from_ms(5 * 60 * 1000) || power_con_diff > 20)
    {
        snprintf(mqtt_msg, sizeof(mqtt_msg), "%d", power_con_diff);
        mqtt_send_msg(mqtt_msg, strlen(mqtt_msg), MQTT_TOPIC_POWER_REPORT, 1, 10 * 60 * 1000);
        gs_pp_run_ctrl.report_power_consumption = hlw8110_act.energy_a;
        gs_pp_run_ctrl.report_tick = now;
    }
#endif    
}

static void acw_mqtt_task_func(void *arg)
{
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    mqtt_msg_to_send_ctrl_t *msg_sent_ctrl;
    mqtt_context_t  mqtt_context = {0};
    unsigned char   sendbuf[512] = {0};
    unsigned char   readbuf[512] = {0};
    int             rc = 0;
    char addr[ACW_MQTT_SRV_ADDR_MAX_LEN + 1];
    char usr[ACW_MQTT_USR_NAME_MAX_LEN + 1];
    char passwd[ACW_MQTT_PASSWD_MAX_LEN + 1]; 
    int port;
    os_bool_t clr_key_press;
    char mqtt_msg[16];

    mqtt_do_dev_init();
    memset(addr, 0, sizeof(addr));
    memset(usr, 0, sizeof(usr));
    memset(passwd, 0, sizeof(passwd));
    acw_get_mqtt_addr(addr, ACW_MQTT_SRV_ADDR_MAX_LEN, &port);
    acw_get_mqtt_user_passwd(usr, ACW_MQTT_USR_NAME_MAX_LEN, passwd, ACW_MQTT_PASSWD_MAX_LEN);

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

#define MAX_CLIENT_ID_LEN   32
    char client_id[MAX_CLIENT_ID_LEN];
    memset(client_id, 0, MAX_CLIENT_ID_LEN);
    snprintf(client_id, MAX_CLIENT_ID_LEN, "%s", acw_conf_get_devid());

    connectData.MQTTVersion = 4; /*3 = 3.1 4 = 3.1.1*/
    connectData.keepAliveInterval = 10;
    connectData.cleansession = 0;
    connectData.willFlag = 0;
    connectData.clientID.cstring = client_id;
    connectData.username.cstring = usr;
    connectData.password.cstring = passwd;
#if 0
    MQTTPacket_willOptions leave_will = MQTTPacket_willOptions_initializer;
    MQTTString will_topic = MQTTString_initializer;
    MQTTString will_msg = MQTTString_initializer;

    will_topic.cstring = gs_foward_auth_topic[MQTT_TOPIC_WILL];
    will_msg.cstring = "disconnect";

    leave_will.qos = 1;
    leave_will.retained = 0;
    leave_will.topicName = will_topic;
    leave_will.message = will_msg;

    connectData.willFlag = 1;
    connectData.will = leave_will;
#endif
    MQTTNetworkInit(&mqtt_context.network, addr, port, OS_NULL);
    MQTTClientInit(&mqtt_context.client, &mqtt_context.network, COMMAND_TIMEOUT, 
                   sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    mqtt_do_connect_server(&mqtt_context, &connectData);

#if defined(MQTT_TASK)
    rc = MQTTStartTask(&mqtt_context.client);
    if (OS_TRUE != rc)
    {
        ACW_PRINT_E("Return code from start tasks is %d", rc);
    }
#endif

    while(1)
    {
        clr_key_press = acw_check_clr_key_press();
        if (OS_TRUE == clr_key_press && OS_FALSE == acw_get_clear_conf_flag())
        {
            snprintf(mqtt_msg, sizeof(mqtt_msg), "initial");
            mqtt_send_msg(mqtt_msg, strlen(mqtt_msg), MQTT_TOPIC_INIT, 1, 2000);
            acw_clear_conf_notice();
            acw_red_led_open();
        }
        mqtt_do_lp_report();
        os_task_msleep(500);
#if !defined(MQTT_TASK)
        rc = MQTTYield(&mqtt_context.client, 1000);
        if (rc != 0)
        {
            ACW_PRINT_I("Return code from yield = %d", rc);
            MQTTNetworkDisconnect(&mqtt_context.network);
            acw_red_led_open();
            mqtt_do_connect_server(&mqtt_context, &connectData);
        }
#endif

        os_mutex_lock(&gs_mqtt_send_mtx, OS_WAIT_FOREVER);
        msg_sent_ctrl = os_list_first_entry_or_null(&gs_send_list, mqtt_msg_to_send_ctrl_t, dlist);
        os_mutex_unlock(&gs_mqtt_send_mtx);
        if (OS_NULL == msg_sent_ctrl)
        {
            continue;
        }
        os_tick_t tick_now;
        tick_now = os_tick_get();

        if (msg_sent_ctrl->sended_cnt && (msg_sent_ctrl->tick_timeout < tick_now || msg_sent_ctrl->sended_cnt > MQTT_RETRY_CNT_DEF))
        {
            os_mutex_lock(&gs_mqtt_send_mtx, OS_WAIT_FOREVER);
            os_list_del_init(&msg_sent_ctrl->dlist);
            os_mutex_unlock(&gs_mqtt_send_mtx);

            ACW_PRINT_I("Drop unsend topic:%s, tick_now=%u, sended_cnt=%d", msg_sent_ctrl->topic, tick_now, msg_sent_ctrl->sended_cnt);
            mqtt_msg_buf_free(msg_sent_ctrl);
            continue;
        }

        msg_sent_ctrl->sended_cnt++;
        rc = MQTTPublish(&mqtt_context.client, msg_sent_ctrl->topic, &msg_sent_ctrl->message);
        if ( 0 != rc)
        {
            MQTTNetworkDisconnect(&mqtt_context.network);
            acw_red_led_open();
            mqtt_do_connect_server(&mqtt_context, &connectData);
        }
        else
        {
            os_mutex_lock(&gs_mqtt_send_mtx, OS_WAIT_FOREVER);
            os_list_del_init(&msg_sent_ctrl->dlist);
            os_mutex_unlock(&gs_mqtt_send_mtx);

            mqtt_msg_buf_free(msg_sent_ctrl);
        }
    }
}

static void mqtt_do_topic_init(void)
{
    int index;
    char *dev_id;

    for (index = 0; index < MQTT_TOPIC_MAX; index++)
    {
        memset(gs_foward_auth_topic[index], 0, sizeof(gs_foward_auth_topic[index]));
    }

    dev_id = acw_conf_get_devid();

    if (OS_NULL == dev_id)
    {
        OS_ASSERT(OS_NULL != dev_id);
    }

    sprintf(gs_foward_auth_topic[MQTT_TOPIC_OPEN_LAMP], ACW_OPEN_LAMP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CLOSE_LAMP], ACW_CLOSE_LAMP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_GET_LAMP], ACW_GET_LAMP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_LAMPSTAT], ACW_LAMP_STAT_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CONF_REQ], ACW_ZERO_CONF_REQ_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CONF_RESP], ACW_ZERO_CONF_RESP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CLEAN_SSID], ACW_CLEAR_SSID_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_ADD_BY], ACW_ADD_BY_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_WILL], ACW_WILL_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_INIT], ACW_DEV_INIT_MQTT_TOPIC, dev_id);

    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CMD], DEV_CTRL_CMD_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_CMD_RESP], DEV_CTRL_CMD_RESP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_QUERY], DEV_CTRL_QUERY_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_QUERY_RESP], DEV_CTRL_QUERY_RESP_MQTT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_POWER_REPORT], ACW_POWER_CONSUMPTION_REPORT_TOPIC, dev_id);
    sprintf(gs_foward_auth_topic[MQTT_TOPIC_FINISHED], DEV_FINISHED_MQTT_TOPIC, dev_id);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function start mqtt sample.
 ***********************************************************************************************************************
 */
void mqtt_start_proc(void)
{
    if (OS_NULL != gs_acw_mqtt_task)
    {
        ACW_PRINT_D("mqtt is already run");
        return;
    }
    mqtt_do_topic_init();

    os_mutex_init(&gs_mqtt_send_mtx, "mqtt_send", OS_FALSE);
    os_mutex_init(&gs_mqtt_auth_mtx, "mqtt_auth", OS_FALSE);
//    os_sem_init(&gs_mqtt_send_sem, "mqtt_sem", 0, OS_IPC_FLAG_FIFO);
    os_list_init(&gs_send_list);
    gs_acw_mqtt_task = os_task_create("mqtt_sample",
                                      acw_mqtt_task_func,
                                      OS_NULL,
                                      MQTT_TASK_STACK_SIZE,
                                      OS_TASK_PRIORITY_MAX / 2 + 1);

    if (OS_NULL == gs_acw_mqtt_task)
    {
        ACW_PRINT_E("mqtt echo thread create failed");
        OS_ASSERT(OS_NULL != gs_acw_mqtt_task);
    }

    os_task_startup(gs_acw_mqtt_task);
}

os_err_t mqtt_send_msg(char *msg, int msg_len, mqtt_topic_item_t topic, uint8_t qos, os_uint32_t timeout_ms)
{
    mqtt_msg_to_send_ctrl_t *msg_sent_ctrl;
    os_tick_t diff;
    os_tick_t now;

    if (OS_NULL == msg || !msg_len || (topic >= MQTT_TOPIC_MAX) || qos > QOS2)
    {
        ACW_PRINT_E("Invalid param input[0x%08x, %d, %d, %d]", msg, msg_len, topic, qos);
        return OS_ERROR;
    }

    msg_sent_ctrl = mqtt_msg_buf_alloc(msg_len);
    if (OS_NULL == msg_sent_ctrl)
    {
        ACW_PRINT_E("malloc memory for mqtt msg failed");
        return OS_ENOMEM;
    }

    now = os_tick_get();
    diff = os_tick_from_ms(timeout_ms);
    msg_sent_ctrl->tick_timeout = now + diff;
    ACW_PRINT_E("topic:%s, tick_timeout=%u", gs_foward_auth_topic[topic], msg_sent_ctrl->tick_timeout);

    msg_sent_ctrl->message.qos = (enum QoS)qos;
    msg_sent_ctrl->message.retained = 0;
    msg_sent_ctrl->topic = gs_foward_auth_topic[topic];

    memcpy(msg_sent_ctrl->in_payload, msg, msg_len);
    msg_sent_ctrl->message.payloadlen = msg_len;
    
    os_mutex_lock(&gs_mqtt_send_mtx, OS_WAIT_FOREVER);
    os_list_add_tail(&gs_send_list, &msg_sent_ctrl->dlist);
    os_mutex_unlock(&gs_mqtt_send_mtx);

    return OS_EOK;
}

static void acw_release_set_func(int argc, char **argv)
{
    acw_dev_type_t dev_type;

    gs_acw_release = !gs_acw_release;
    dev_type = acw_get_dev_type();
    if (DEV_TYPE_AC == dev_type)
    {
        ac_do_fresh_lcd();
    }
    else if (DEV_TYPE_WM == dev_type)
    {
        wm_do_fresh_lcd();
    }
    else if (DEV_TYPE_PP == dev_type)
    {
        pp_do_fresh_lcd();
    }
    else if (DEV_TYPE_LP == dev_type)
    {
        lp_do_fresh_lcd();
    }
    else
    {
        (void)0;
    }

	return;
}

static void mqtt_do_connect_home_succ_sync(char *sender, os_bool_t clr)
{
    char mqtt_msg[64];

    snprintf(mqtt_msg, sizeof(mqtt_msg), "sender:%s,initialized=%s", sender, (clr == OS_TRUE) ? "true":"false");
    mqtt_send_msg(mqtt_msg, strlen(mqtt_msg), MQTT_TOPIC_ADD_BY, 1, 10 * 60 * 1000);

    return;
}

static void mqtt_do_req_add_home(char *slave_devid, os_bool_t clr, os_uint8_t rand)
{
    char mqtt_msg[64];

    snprintf(mqtt_msg, sizeof(mqtt_msg), "%s,initialized=%s,version=%03d", slave_devid, (clr == OS_TRUE) ? "true":"false", rand);
    mqtt_send_msg(mqtt_msg, strlen(mqtt_msg), MQTT_TOPIC_CONF_REQ, 1, 10 * 1000);

    return;
}

void acw_plat_demo_init(acw_intf_t intf)
{
    acw_led_io_init();
    acw_key_io_init();
    acw_red_led_open();

    acw_start_proc(intf, mqtt_do_connect_home_succ_sync, mqtt_do_req_add_home);
    mqtt_start_proc();
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(acw_release, acw_release_set_func, "acw_release");
#endif
