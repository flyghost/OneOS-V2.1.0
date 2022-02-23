/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (c) 2006-2018 RT-Thread Development Team.
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
 * @file        hal_os.c
 *
 * @brief     a port file of os for iotkit
 *
 * @details  
 *
 * @revision
 * Date               Author             Notes
 * 2019-07-21         MurphyZhao         first edit
 * 2020-06-10         OneOS Team         format and change request resource
 ***********************************************************************************************************************
 */

#include <stdlib.h>


//#include "os_kernel.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "wrappers_defs.h"
#include "wrappers_os.h"
//#include "sys/socket.h"
#include "os_util.h"
#include "os_memory.h"
#include "os_clock.h"
#include "os_task.h"
#include "os_mutex.h"
#include "os_sem.h"
#define DBG_EXT_TAG "ali.os"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

#define ONEOS_TASK_DEFAULT_PRIORITY (OS_TASK_PRIORITY_MAX / 2 + OS_TASK_PRIORITY_MAX / 4)
#define ONEOS_TASK_DEFAULT_TIME_SLICE     ( 5 )
static char log_buf[OS_LOG_BUFF_SIZE];

/* please use ifconfig to get your wireless card's name, and replace g_ifname */
char *g_ifname = "------------";

		
int HAL_ThreadCreate(void **thread_handle,  
                     void *(*work_routine)(void *), 
                     void *arg,                   
                     hal_os_thread_param_t *hal_os_thread_param, 
                     int *stack_used)  
{
	char *task_name;
	int task_stack_size; 
	os_task_t *task_handle = OS_NULL;

    if (OS_NULL == thread_handle) 
    {
        LOG_E(DBG_EXT_TAG, "");
        return -1;
    }

    if(OS_NULL == hal_os_thread_param)
    {
    LOG_E(DBG_EXT_TAG, "");
    return -1;
    }

	task_name = hal_os_thread_param->name;
	task_stack_size = hal_os_thread_param->stack_size;
	task_handle = os_task_create(task_name, (void (*)(void *))work_routine, arg, task_stack_size, ONEOS_TASK_DEFAULT_PRIORITY);

    if (OS_NULL == task_handle)
    {
        LOG_E(DBG_EXT_TAG, "create fail");
        return -1;
    }

	os_task_startup(task_handle);

	*thread_handle = task_handle;

	return 0;
}

void *HAL_MutexCreate(void)
{
    os_mutex_t *mutex = os_mutex_create("ali_ld_mutex", 0);
    if (!mutex)
    {
        LOG_E(DBG_EXT_TAG, "mutex create failed!");
    }
    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_destroy((os_mutex_t *)mutex)))
    {
        LOG_E(DBG_EXT_TAG, "destroy mutex failed, err num: %d", err_num);
    }
}

void HAL_MutexLock(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_lock((os_mutex_t *)mutex, OS_WAIT_FOREVER)))
    {
        LOG_E(DBG_EXT_TAG, "lock mutex failed, err num: %d", err_num);
    }
}

void HAL_MutexUnlock(void *mutex)
{
    os_err_t err_num;

    if (0 != (err_num = os_mutex_unlock((os_mutex_t *)mutex)))
    {
        LOG_E(DBG_EXT_TAG, "unlock mutex failed, err num: %d", err_num);
    }
}

/**
 * @brief   create a semaphore
 *
 * @return semaphore handle.
 * @see None.
 * @note not more than 100 semaphore.
 */
void *HAL_SemaphoreCreate(void)
{
    char           name[10] = {0};
    static uint8_t sem_num  = 0;
    os_snprintf(name, sizeof(name), "sem%02d", ((++sem_num) % 100));
    os_sem_t *sem = os_sem_create(name, 0, OS_SEM_MAX_VALUE);
    if (!sem)
    {
        LOG_E(DBG_EXT_TAG, "Semaphore create failed!");
    }
    return (void *)sem;
}

/**
 * @brief   destory a semaphore
 *
 * @param[in] sem @n the specified sem.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphoreDestroy(void *sem)
{
    os_err_t        err = OS_EOK;
    os_sem_t *semaphore = sem;

    if (!semaphore)
    {
        LOG_E(DBG_EXT_TAG, "In param (sem) is NULL!");
        return;
    }

    err = os_sem_destroy(semaphore);
    if (err != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "sem delete failed! errno:%d", err);
    }

    return;
}

/**
 * @brief   signal thread wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphorePost(void *sem)
{
    os_err_t        err = OS_EOK;
    os_sem_t *semaphore = sem;

    if (!semaphore)
    {
        LOG_E(DBG_EXT_TAG, "In param (sem) is NULL!");
        return;
    }

    err = os_sem_post(semaphore);
    if (err != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "sem release failed! errno:%d", err);
    }

    return;
}

/**
 * @brief   wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @param[in] timeout_ms @n timeout interval in millisecond.
     If timeout_ms is PLATFORM_WAIT_INFINITE, the function will return only when the semaphore is signaled.
 * @return
   @verbatim
   =  0: The state of the specified object is signaled.
   =  -1: The time-out interval elapsed, and the object's state is nonsignaled.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
    os_err_t        err = OS_EOK;
    os_sem_t *semaphore = sem;

    if (!semaphore)
    {
        LOG_E(DBG_EXT_TAG, "In param (sem) is NULL!");
        return -1;
    }

    err = os_sem_wait(semaphore, timeout_ms);
    if (err != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "sem take failed! errno:%d", err);
    }

    return (err == OS_EOK ? 0 : -1);
}

void *HAL_Malloc(uint32_t size)
{
    void *result;
    result = (void*)os_malloc(size);
    return result;
}

void HAL_Free(void *ptr)
{
    os_free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
    uint64_t tick;
    tick = os_tick_get();

    tick = tick * 1000;

    return (tick + OS_TICK_PER_SECOND - 1) / OS_TICK_PER_SECOND;
}

void HAL_SleepMs(uint32_t ms)
{
    os_task_msleep(ms);
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (rand() % region) : 0;
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = os_vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
    return os_vsnprintf(str, len, format, ap);
}

void HAL_Printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    os_vsnprintf(log_buf, OS_LOG_BUFF_SIZE, fmt, args);
    va_end(args);
    os_kprintf("%s", log_buf);
}

#ifdef HAL_KV
OS_WEAK int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    return 0;
}

OS_WEAK int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
    return 0;
}

OS_WEAK int HAL_Kv_Del(const char *key)
{
    return 0;
}
#endif

#if((defined HTTP2_COMM_ENABLED) || (defined WIFI_PROVISION_ENABLED) || (defined DEV_BIND_ENABLED))
/*Wifi provision &http2 feature do not support yet*/
void HAL_Wifi_Get_IP(void)
{
		/*do not support*/
		return;
}

char *HAL_Wifi_Get_Mac(_OU_ char mac_str[HAL_MAC_LEN])
{
		/*do not support*/
}

int HAL_Fclose(void *stream)
{
	 /*do not support*/
}
void *HAL_Fopen(const char *path, const char *mode)
{

}
uint32_t HAL_Fread(void *buff, uint32_t size, uint32_t count, void *stream)
{
	
}
int HAL_Fseek(void *stream, long offset, int framewhere)
{
	
}
long HAL_Ftell(void *stream)
{
	
}
int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)	
{
	
}

int HAL_Sys_Net_Is_Ready()
{
	
}
int HAL_Wifi_Enable_Mgmt_Frame_Filter(
            _IN_ uint32_t filter_mask,
            _IN_OPT_ uint8_t vendor_oui[3],
            _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
	
}

int HAL_Wifi_Get_Ap_Info(char ssid, char passwd, uint8_t bssid)
{
	//TODO
    return 0;
}

int HAL_Awss_Close_Ap()
{
	//TODO
    return 0;
}

void HAL_Awss_Close_Monitor()
{
	//TODO
    return;
}

int HAL_Awss_Open_Ap()
{
	//TODO
    return 0;
}

void HAL_Awss_Open_Monitor()
{
	//TODO
    return;
}

int HAL_Awss_Connect_Ap(_IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
	
}

void HAL_Awss_Switch_Channel(
            _IN_ char primary_channel,
            _IN_OPT_ char secondary_channel,
            _IN_OPT_ uint8_t bssid[ETH_ALEN])
{
	
}
#endif
