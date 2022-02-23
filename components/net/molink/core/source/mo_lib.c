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
 * @file        mo_lib.c
 *
 * @brief       module link kit lib api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_lib.h"

#include <ctype.h>

void bytes_to_hexstr(const char *source, char *dest, os_size_t source_size)
{
    OS_ASSERT(OS_NULL != source);
    OS_ASSERT(OS_NULL != dest);

    os_uint8_t high_byte = 0;
    os_uint8_t low_byte  = 0;

    for (int i = 0; i < source_size; i++)
    {
        high_byte = source[i] >> 4;
        low_byte  = source[i] & 0x0f;

        high_byte += 0x30;

        if (high_byte > 0x39)
        {
            dest[i * 2] = high_byte + 0x07;
        }
        else
        {
            dest[i * 2] = high_byte;
        }

        low_byte += 0x30;
        if (low_byte > 0x39)
        {
            dest[i * 2 + 1] = low_byte + 0x07;
        }
        else
        {
            dest[i * 2 + 1] = low_byte;
        }
    }
    return;
}

void hexstr_to_bytes(const char *source, char *dest, os_size_t source_size)
{
    OS_ASSERT(OS_NULL != source);
    OS_ASSERT(OS_NULL != dest);

    os_uint8_t high_byte = 0;
    os_uint8_t low_byte  = 0;

    for (int i = 0; i < source_size; i += 2)
    {
        high_byte = toupper(source[i]);
        low_byte  = toupper(source[i + 1]);

        if (high_byte > 0x39)
        {
            high_byte -= 0x37;
        }
        else
        {
            high_byte -= 0x30;
        }

        if (low_byte > 0x39)
        {
            low_byte -= 0x37;
        }
        else
        {
            low_byte -= 0x30;
        }

        dest[i / 2] = (high_byte << 4) | low_byte;
    }
    return;
}
