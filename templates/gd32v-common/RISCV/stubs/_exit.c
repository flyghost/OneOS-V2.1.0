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
 * @file        _exit.c
 *
 * @brief       _exit function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <unistd.h>
#include "stub.h"

void _exit(int code)
{
    const char message[] = "\nProgram has exited with code:";

    write(STDERR_FILENO, message, sizeof(message) - 1);
    write_hex(STDERR_FILENO, code);
    write(STDERR_FILENO, "\n", 1);

  for (;;);
}
