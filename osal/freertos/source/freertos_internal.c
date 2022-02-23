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
 * @file        freertos_internal.c
 *
 * @brief       This file implements some port functions of FreeRTOS mem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-12   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include "string.h"
#include "freertos_internal.h"

os_ubase_t ux_critical_nest = 0;

char* oneos_itoa(int value, char* string, int radix)
{
    char zm[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[10] = {0};

    int sum = value;
    char* cp = string;
    int i = 0;

    if (value < 0)
    {
        return string;
    }
    else if (0 == value)
    {
        char *res_str = "0";

        strncpy(string, res_str, strlen(res_str));
        return string;
    }
    else
    {
        while (sum > 0)
        {
            aa[i++] = zm[sum%radix];
            sum/=radix;
        }
    }

    for (int j=i-1; j>=0; j--)
    {
        *cp++ = aa[j];
    }

    *cp='\0';

    return string;
}

