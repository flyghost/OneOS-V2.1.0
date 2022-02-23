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
 * @file        entropy_hardware_alt.c
 *
 * @brief       os related entropy generator
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include <os_clock.h>

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

static int os_generate_random_array(unsigned char *out_buf, size_t len)
{
    int i, j;
    int rand_data;

    srand((unsigned int)os_tick_get());

    for (i = 0; i < ((len + 3) & ~3) / 4; i++)
    {
        rand_data = rand();

        for (j = 0; j < 4; j++)
        {
            if ((i * 4 + j) < len)
            {
                out_buf[i * 4 + j] = (unsigned char)(rand_data >> (j * 8));
            } 
            else 
            {
                break;
            }
        }
    }

    return 0;
}

int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen )
{
    os_generate_random_array(output, len);
    *olen = len;

    return 0;
}
#endif

