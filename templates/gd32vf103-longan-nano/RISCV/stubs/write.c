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
 * @file        write.c
 *
 * @brief        This file implements write function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <gd32vf103.h>
#include <unistd.h>
#include <sys/types.h>

#include "stub.h"
#include "gd32vf103.h"

typedef unsigned int size_t;

extern int _put_char(int ch) __attribute__((weak));

ssize_t _write(int fd, const void *ptr, size_t len)
{
    const uint8_t *current = (const uint8_t *)ptr;

    /*if (isatty(fd)) */
    {
        for (size_t jj = 0; jj < len; jj++)
        {
            _put_char(current[jj]);

            if (current[jj] == '\n')
            {
                _put_char('\r');
            }
        }
        return len;
    }

    return _stub(EBADF);
}

int puts(const char *string)
{
    return _write(0, (const void *)string, strlen(string));
}

int _put_char(int ch)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
    {
    }

    return ch;
}
