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
 * @file        write_hex.c
 *
 * @brief       This file implements write_hex function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <unistd.h>

void write_hex(int fd, unsigned long int hex)
{
    uint8_t ii;
    uint8_t jj;
    char    towrite;
    write(fd, "0x", 2);
    for (ii = sizeof(unsigned long int) * 2; ii > 0; ii--)
    {
        jj            = ii - 1;
        uint8_t digit = ((hex & (0xF << (jj * 4))) >> (jj * 4));
        towrite       = digit < 0xA ? ('0' + digit) : ('A' + (digit - 0xA));
        write(fd, &towrite, 1);
    }
}
