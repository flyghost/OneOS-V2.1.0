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
 * @file        onepos_hardware_control_module.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <math.h> 
#include <string.h>
#include <os_mq.h>
#include <os_sem.h>
#include <os_task.h>
#include <os_clock.h>

#include "onepos_log.h"
#include "onepos_state.h"
#include "onepos_timer.h"
#include "onepos_data_cache.h"
#include "onepos_common.h"
#include "onepos_hardware_control_module.h"
#include "onepos_external_service.h"



#include <os_assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <shell.h>
#include <oneos_config.h>

#include "nimble-console/console.h"
#include "os/os.h"
#include "sysinit/sysinit.h"
#include "log/log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#include "ope_position.h"

#define  OS_EOK               0     

static os_mq_t        *gs_onepos_hardware_mq;
static os_sem_t       *gs_onepos_hardware_ble_scan_sem;

void printff_double(double a)
{
    int n = (long)a % 100000;
    int i = 0;
    double x = a;

    printf("%d.", n);

    while (i < 7)
    {
        x = x * 10;

        printf("%ld", (long)x % 10);

        i++;
    }
}


/**
  ***********************************************************************************************************************
  * @brief        hex_string convert to string   
  *
  * @param[in]    sSrc      hex_string head
  * @param[out]   sDest     string head
  * @param[in]    nSrcLen   hex_string length
  ***********************************************************************************************************************
  */
static void Hex2Str(os_uint8_t *sSrc,  char *sDest, os_int32_t nSrcLen )
{
    os_int32_t  i;
    char        szTmp[3];
 
    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf(szTmp, "%02x", sSrc[i] );
        memcpy(&sDest[i * 2], szTmp, 2 );
    }
    return ;
}


/**
  ***********************************************************************************************************************
  * @brief        hex convert to int64   
  *
  * @param[in]    data 
  * @param[in]    len  
  *
  * @return       os_int64_t
  ***********************************************************************************************************************
  */
static os_int64_t hex2int64(char *data, os_int32_t len)
{
    os_int32_t i = 0;
    os_int64_t res = 0;

    for (i = 0; i < len; i++)
    {
        res |= ((os_int64_t)data[len - 1 - i] & 0xffu) << (i * 8);
    }

    return res;
}


/**
  ***********************************************************************************************************************
  * @brief        bluetooth message analysis   
  *
  * @param[in]    name   target name
  * @param[in]    len    target name length
  *
  * @return       include the target name or not  
  ***********************************************************************************************************************
  */
static void onepos_adv_data_analysis(char *data, os_int32_t len, double *lat, double *lon)
{
    os_int32_t integer_len = 1;
    os_int32_t decimals_len = 3;
    os_int32_t len_all;
    os_int32_t precision = 7;
    os_int64_t integer;
    os_int64_t decimals;

    len_all = integer_len + decimals_len;

    if (len != (len_all * 2))
    {
        *lat = 0;
        *lon = 0;
          
        return;
    }

    integer  = hex2int64(data, integer_len);
    decimals = hex2int64(data + integer_len, decimals_len);
    *lat = integer + decimals * pow(0.1, precision);

    integer  = hex2int64(data + len_all, integer_len);
    decimals = hex2int64(data + len_all + integer_len, decimals_len);
    *lon = integer + decimals * pow(0.1, precision);
}


/**
  ***********************************************************************************************************************
  * @brief        ble devices filter   
  *
  * @param[in]    name   target name
  * @param[in]    len    target name length
  *
  * @return       include the target name or not  
  ***********************************************************************************************************************
  */
static os_bool_t onepos_ble_name_filtrate(char *name, os_int32_t len)
{
    os_int32_t  i;
    char        tmp[256];

    for (i = 0; i < len; i++)
    {
        if ((name[i] != ' ') && (name[i] != '_'))
        {
            tmp[i] = name[i];
        }
        else
        {
            break;
        }
    }

    tmp[i] = '\0';

    if (!strcmp(tmp, "onepos"))
    {
        return OS_TRUE;
    }
    else
    {
        return OS_FALSE;
    }
    
} 


/**
  ***********************************************************************************************************************
  * @brief        bluetooth interrupt function   
  *
  * @param[in]    event   bluetooth interrupt event handler
  * @param[in]    arg     function arg
  *
  * @return       the operation result
  * @retval       OS_EOK
  ***********************************************************************************************************************
  */
static os_err_t onepos_scan_event_func(struct ble_gap_event *event, void *arg)
{
    os_err_t              ret        = OS_ERROR;
    onepos_data_cache_t  *data_cache = arg;
    
    ope_ble_rssi_ap_t  msg_assemble;
    char  mac_temp[12 + 1];
    
    struct ble_hs_adv_fields fields;
    os_int32_t rc;
    char       name[21];
    double     lat = 0;
    double     lon = 0;
    os_int32_t name_len_limit = 20;
    os_int32_t name_len;
    char       data[64];
       
    switch (event->type) 
    {
    case BLE_GAP_EVENT_DISC:
        rc = ble_hs_adv_parse_fields(&fields, 
                                     event->disc.data,
                                     event->disc.length_data);
        if (rc != 0) 
        {
            return OS_EOK;
        }
        
        if (fields.name != NULL)
        {
            name_len = fields.name_len < name_len_limit ? fields.name_len : name_len_limit;
            memcpy(name, fields.name, name_len);
            name[name_len] = '\0';
                       
            /* parse the ble msg */
            if (onepos_ble_name_filtrate(name, name_len))
            {
                /* analysis the target ble msg */
                onepos_adv_data_analysis((char *)fields.mfg_data, fields.mfg_data_len, &lat, &lon); 
                                
                Hex2Str( &(event->disc.addr.val[0]), mac_temp, sizeof(event->disc.addr.val));
                memcpy(msg_assemble.mac, mac_temp, 12);
                msg_assemble.lat  = lat;
                msg_assemble.lon  = lon;
                msg_assemble.rssi = event->disc.rssi;
                 
                ret = onepos_data_cache_add(data_cache, &msg_assemble, sizeof(ope_ble_rssi_ap_t));
                if(OS_EOK != ret)
                {
                    ONEPOS_LOG_W("ble msg added to cache failed, the operation result is %d.", ret);
                }                
            }

        }

        return OS_EOK;
        /* discovery procedure has terminated */
    case BLE_GAP_EVENT_DISC_COMPLETE:
        os_sem_post(gs_onepos_hardware_ble_scan_sem);
        return OS_EOK;
    default:
        ONEPOS_LOG_W("onepos_scan_event_func() error!");
        return OS_EOK;
    }
}


/**
  ***********************************************************************************************************************
  * @brief        actual scanning operation  
  *
  * @param[in]    ble_scan_window   
  * @param[in]    handle             handle of the data cache which is used to storage scan result
  ***********************************************************************************************************************
  */
static void onepos_do_scan(os_uint16_t ble_scan_window, os_uint16_t ble_scan_itvl, onepos_data_cache_t *handle)
{
    /* set scan parameters */
    struct ble_gap_disc_params scan_params;
    scan_params.itvl              = ble_scan_itvl;          /* unit: 0.625ms */
    scan_params.window            = ble_scan_window;        /* unit: 0.625ms */
    scan_params.filter_policy     = 0;
    scan_params.limited           = 0;
    scan_params.passive           = 1;
    scan_params.filter_duplicates = 1;

    os_int32_t ret             = 0;
   
    ret = ble_gap_disc(BLE_OWN_ADDR_RANDOM, 
                       ble_scan_window * 625 / 1000         /* unit:     1ms */, 
                       &scan_params, 
                       onepos_scan_event_func, 
                       handle);
    if(ret != 0)
    {
        ONEPOS_LOG_W("ble_gap_disc() failed, the operation result is %d.", ret);
    }
}


/**
  ***********************************************************************************************************************
  * @brief  hardware control thread          
  ***********************************************************************************************************************
  */
static void onepos_hardware_control_task(void)
{  
    os_tick_t             delay_tick      = 0; 
    os_uint16_t           ble_scan_window = 0;
    os_uint16_t           ble_scan_itvl   = 0;
    os_err_t              init_error      = OS_ERROR;
    os_err_t              init_successful = OS_EOK;
    onepos_data_cache_t  *data_cache      = OS_NULL;
    
    onepos_state_config_param_t config_param;
    memset(&config_param, 0, sizeof(onepos_state_config_param_t));


    /* 0.create the cache */
    data_cache = onepos_data_cache_create(CACHE_MEM_NUM, sizeof(ope_ble_rssi_ap_t));                      
    if(OS_NULL == data_cache)                                        
    {
        ONEPOS_LOG_E("data_cache = onepos_data_cache_create(): data_cache == OS_NULL.");
        os_mq_send(gs_onepos_hardware_mq, &init_error, sizeof(init_error), OS_WAIT_FOREVER);
    } 
    onepos_state_set_one_handle(DATA_CACHE, (void*)data_cache);

    
    /* 1.send hardware_control_task startup successful message*/
    os_mq_send(gs_onepos_hardware_mq, &init_successful, sizeof(init_successful), OS_WAIT_FOREVER);  
    
    while(1)
    {               
        /* 2.get the config param */
        if(OS_EOK != onepos_state_get_config_param(&config_param))
        {
            ONEPOS_LOG_W("onepos_state_get_config_param(&config_param) error.");
            continue;
        }        


        /* 3.set scan parameters */
        ble_scan_window = (config_param.scan_window > config_param.scan_interval)? config_param.scan_interval:config_param.scan_window;
        ble_scan_itvl   = (config_param.scan_interval > config_param.scan_window)? config_param.scan_interval:config_param.scan_window;


        /* 4.start bluetooth scan */                                      
        onepos_do_scan(ble_scan_window, ble_scan_itvl, data_cache);

        
        /* 5.wait for the scanning to end */                                  
        delay_tick = os_tick_from_ms(config_param.scan_interval);
        os_sem_wait(gs_onepos_hardware_ble_scan_sem, 2 * delay_tick);
 
    }
}



/**
  ***********************************************************************************************************************
  * @brief            create a hardware control module 
  *
  * @return           the hardware control task handle
  * @retval           OS_NULL      failed
  ***********************************************************************************************************************
  */
os_task_t *onepos_hardware_control_module_create(void)
{
    os_err_t     msg;
    os_size_t    recv_size;
    os_task_t   *hardware_task       = OS_NULL;  
        
    gs_onepos_hardware_mq            = OS_NULL;
    gs_onepos_hardware_ble_scan_sem  = OS_NULL;
   
    onepos_state_config_param_t obtained_param;
    if(OS_EOK != onepos_state_get_config_param(&obtained_param))
    {
        ONEPOS_LOG_E("onepos_state_get_config_param(&obtained_result) error.");
        return OS_NULL;                                             
    }

    
    /* 1.create the msg queue */
    gs_onepos_hardware_mq = os_mq_create("onepos_hardware_mq", sizeof(os_err_t), 1);            
    if(OS_NULL == gs_onepos_hardware_mq)
    {
        ONEPOS_LOG_E("gs_onepos_hardware_mq == OS_NULL.");
        goto exit;
    } 


    /* 2.create the sem for scanning */
    gs_onepos_hardware_ble_scan_sem = os_sem_create("onepos_ble_scan_sem", 0, SEM_VALUE_TO_MUTEX); 
    if(OS_NULL == gs_onepos_hardware_ble_scan_sem)
    {
        ONEPOS_LOG_E("gs_onepos_hardware_ble_scan_sem == OS_NULL.");
        goto exit;
    } 


    /* 3.create position task */
    hardware_task = os_task_create("onepos_hardware_task", (void*)onepos_hardware_control_task, OS_NULL, 2048, 25); 
    if(OS_NULL == hardware_task)
    {
        ONEPOS_LOG_E("hardware_task == OS_NULL.");
        goto exit;
    }  

      
    /* 4.strt up the position task */
    os_task_startup(hardware_task);


    /* 5.blocking the message queue */
    if(OS_EOK != os_mq_recv(gs_onepos_hardware_mq, &msg, sizeof(os_err_t), OS_TICK_PER_SECOND, &recv_size) 
       || OS_EOK != msg)
    {
        ONEPOS_LOG_E("os_mq_recv(gs_onepos_hardware_mq, &msg, sizeof(os_err_t), OS_TICK_PER_SECOND, &recv_size): error.");
        goto exit;
    }

    ONEPOS_LOG_I("onepos_hardware_control_module_create() succeed."); //onepos_debug
    return hardware_task;
       
exit:   
    if(OS_NULL != hardware_task)
    {
        os_task_destroy(hardware_task);
        hardware_task = OS_NULL;
        onepos_state_set_one_handle_null(HARDWARE_TASK, (void*)hardware_task);
    }    
    
    if(OS_NULL != gs_onepos_hardware_mq)
    {
        os_mq_destroy(gs_onepos_hardware_mq);
        gs_onepos_hardware_mq = OS_NULL;
    }   

    if(OS_NULL != gs_onepos_hardware_ble_scan_sem)
    {
        os_sem_destroy(gs_onepos_hardware_ble_scan_sem);
        gs_onepos_hardware_ble_scan_sem = OS_NULL;
    }  
 
    return OS_NULL;
}

/**
  ***********************************************************************************************************************
  * @brief            destroy the hardware control module
  * 
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_ESTATE   
  ***********************************************************************************************************************
  */
os_err_t onepos_hardware_control_module_destroy(void)
{
    os_task_t            *hardware_task   = OS_NULL;
    onepos_data_cache_t  *data_cache      = OS_NULL;
    
    if(OS_EOK != onepos_state_get_one_handle(HARDWARE_TASK, (void**)&hardware_task) 
       || OS_NULL == hardware_task)
    {
        ONEPOS_LOG_E("OS_EOK != onepos_state_get_one_handle(HARDWARE_TASK, (void**)&hardware_task): error.");
        return POS_ESTATE;
    }

    if(OS_EOK != onepos_state_get_one_handle(DATA_CACHE, (void**)&data_cache) 
       || OS_NULL == data_cache)
    {
        ONEPOS_LOG_E("OS_EOK != onepos_state_get_one_handle(DATA_CACHE, (void**)&data_cache): error.");
        return POS_ESTATE;
    }

    /* Once the gs_onepos_hardware_ble_scan_sem obtained, means one round ble scanning is over. It is time to end.*/
    os_sem_wait(gs_onepos_hardware_ble_scan_sem, OS_WAIT_FOREVER);
       
    os_task_destroy(hardware_task);
    os_mq_destroy(gs_onepos_hardware_mq);
    os_sem_destroy(gs_onepos_hardware_ble_scan_sem);
    onepos_data_cache_destroy(data_cache);

    data_cache    = OS_NULL; 
    onepos_state_set_one_handle_null(DATA_CACHE, (void*)data_cache);
    hardware_task = OS_NULL;
    onepos_state_set_one_handle_null(HARDWARE_TASK, (void*)hardware_task);

    ONEPOS_LOG_I("onepos_hardware_control_module_destroy() succeed.");   //onepos_debug
    return OS_EOK;
}

