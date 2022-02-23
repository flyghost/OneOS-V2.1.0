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
 * @file        crypto_test.c
 *
 * @brief       The test file for crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <hwcrypto/hwcrypto.h>
#include <hwcrypto/hw_crc.h>
#include <shell.h>

static void hwcrypto_crc_test(int argc, char *argv[])
{
    struct os_hwcrypto_ctx *ctx;
    os_uint32_t result = 0;
    
    const os_uint8_t temp[] = {0, 1, 2, 3, 4, 5, 6, 7};
    const struct hwcrypto_crc_cfg cfg =
    {
        .last_val = 0xFFFFFFFF,
        .poly     = 0x04C11DB7,
        .width    = 32,
        .xorout   = 0x00000000,
        .flags    = 0,
        //.flags    = CRC_FLAG_REFIN | CRC_FLAG_REFOUT,
    };

    const os_uint32_t expect_result = 0x4ACADC12;

    os_kprintf("hwcrypto_crc_test:");
    
    ctx = os_hwcrypto_crc_create(HWCRYPTO_CRC_CRC32);

    os_hwcrypto_crc_cfg(ctx, &cfg);

    result = os_hwcrypto_crc_update(ctx, temp, sizeof(temp));
    
    os_hwcrypto_crc_destroy(ctx);

    os_kprintf("result: %x \r\n", result);
    os_kprintf("expect_result: %x \r\n", expect_result);
    os_kprintf("%s.\r\n", (result == expect_result) ? "success" : "failed");
}

SH_CMD_EXPORT(hwcrypto_crc_test, hwcrypto_crc_test, "hwcrypto_crc_test");

