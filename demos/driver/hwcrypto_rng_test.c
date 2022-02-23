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
#include <driver.h>
#include <hwcrypto/hwcrypto.h>
#include <hwcrypto/hw_rng.h>
#include <shell.h>

static void hwcrypto_rng_test(int argc, char *argv[])
{
    int i, max_test = 1000;
    int result_table[10];
    os_uint32_t result = 0;

    os_kprintf("hwcrypto_rng_test:\r\n");

    for (i = 0; i < ARRAY_SIZE(result_table); i++)
        result_table[i] = 0;
    
    for (i = 0; i < max_test; i++)
    {
        result = os_hwcrypto_rng_update();
        result_table[result % ARRAY_SIZE(result_table)]++;
    }

    for (i = 0; i < ARRAY_SIZE(result_table); i++)
        os_kprintf("%d: %d\r\n", i, result_table[i]);
}

SH_CMD_EXPORT(hwcrypto_rng_test, hwcrypto_rng_test, "hwcrypto_rng_test");

