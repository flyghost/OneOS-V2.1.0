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
 * @file        hal_ota.c
 *
 * @brief     a port file of ota for iotkit
 *
 * @details  
 *
 * @revision
 * Date               Author             Notes
 * 2020-06-10         OneOS Team         first version
 ***********************************************************************************************************************
 */

#include <string.h>

//#include "os_kernel.h"
#include "wrappers_defs.h"
#include "os_assert.h"
#define DBG_EXT_TAG "ali.ota"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

int HAL_GetFirmwareVersion(char *version)
{
    OS_ASSERT(version);

    char *ver = "OneOS-V0.0.1.0alpha";
    int   len = strlen(ver);
    memset(version, 0x0, IOTX_FIRMWARE_VER_LEN);
    strncpy(version, ver, IOTX_FIRMWARE_VER_LEN);
    version[len] = '\0';
    return strlen(version);
}

OS_WEAK void HAL_Firmware_Persistence_Start(void)
{
    LOG_I(DBG_EXT_TAG, "OTA start... [Not implemented]");
    return;
}

OS_WEAK int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
    LOG_I(DBG_EXT_TAG, "OTA write... [Not implemented]");
    return 0;
}

OS_WEAK int HAL_Firmware_Persistence_Stop(void)
{
    /* check file md5, and burning it to flash ... finally reboot system */

    LOG_I(DBG_EXT_TAG, "OTA finish... [Not implemented]");
    return 0;
}
