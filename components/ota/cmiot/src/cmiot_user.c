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
 * @file        cmiot_user.c
 *
 * @brief       Implement user functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "cmiot_user.h"
#include "cmiot_config.h"
#ifdef OS_USING_SHELL
#include "shell.h"
#endif
#include <os_task.h>
#include <stdio.h>
#include <string.h>

/**
 ***********************************************************************************************************************
 * @brief           This function will return network type
 * @param        void
 *
 * @retval          network type
 ***********************************************************************************************************************
 */
cmiot_char *cmiot_get_network_type(void)
{
    return "-";
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return socket recive timeout
 * @param        void
 *
 * @retval          socket recive timeout, ms
 ***********************************************************************************************************************
 */
cmiot_uint32 cmiot_get_try_time(void)
{
    return CMIOT_DEFAULT_TRY_TIME;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return retry time when decode sever message failed or an error occurred by client
 * @param        void
 *
 * @retval          retry time
 ***********************************************************************************************************************
 */
cmiot_uint8 cmiot_get_try_count(void)
{
    return CMIOT_DEFAULT_TRY_COUNT;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return now time
 * @param        void
 *
 * @retval          time
 ***********************************************************************************************************************
 */
cmiot_uint32 cmiot_get_utc_time(void)
{
    return CMIOT_DEFAULT_UTC_TIME;
}

/**
 ***********************************************************************************************************************
 * @brief                This function will get device unique id, max len is CMIOT_MID_MAXLEN
 * @param[out]      uid        The memory to saved unique id
 *
 * @retval              void
 ***********************************************************************************************************************
 */
void cmiot_get_uniqueid(cmiot_char *uid)
{
    memcpy(uid, "112233445566778899AABBCCDDEEFF", sizeof("112233445566778899AABBCCDDEEFF") - 1);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return app part name
 * @param        void
 *
 * @retval          app part name
 ***********************************************************************************************************************
 */
cmiot_char *cmiot_app_name()
{
    return CMIOT_ONEOS_APP;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return download part name
 * @param        void
 *
 * @retval          download part name
 ***********************************************************************************************************************
 */
cmiot_char *cmiot_download_name()
{
    return CMIOT_ONEOS_UPDATE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function for output data
 * @param[in]       data     The data
 * @param[in]       len       The data len
 *
 * @retval          void
 ***********************************************************************************************************************
 */
void cmiot_printf(cmiot_char *data, cmiot_uint32 len)
{
    os_kprintf("%s", data);
}

/**
 ***********************************************************************************************************************
 * @brief           This function for sleep
 * @param[in]    time       The sleep time, ms
 *
 * @retval          void
 ***********************************************************************************************************************
 */
void cmiot_msleep(cmiot_uint32 time)
{
    os_task_msleep(time);
}

#ifdef OS_USING_SHELL
/**
 ***********************************************************************************************************************
 * @brief              This function will start the upgrade process
 * @param[in]       argc       The input param num
 * @param[in]       argv       The input param
 *
 * @retval             void
 ***********************************************************************************************************************
 */
void cmiot_cv(int argc, char *argv[])
{
    cmiot_int8 rst = cmiot_upgrade();
    os_kprintf("start upgrade rst:%d\r\n", rst);
}
SH_CMD_EXPORT(cmiot_cv, cmiot_cv, "start the upgrade process");

/**
 ***********************************************************************************************************************
 * @brief              This function will start the report upgrade process
 * @param[in]       argc       The input param num
 * @param[in]       argv       The input param
 *
 * @retval             void
 ***********************************************************************************************************************
 */
void cmiot_ru(int argc, char *argv[])
{
    cmiot_int8 rst = cmiot_report_upgrade();
    os_kprintf("start report upgrade rst:%d\r\n", rst);
}
SH_CMD_EXPORT(cmiot_ru, cmiot_ru, "start the report upgrade process");
#endif
