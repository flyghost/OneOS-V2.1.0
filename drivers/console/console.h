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
 * @file        console.h
 *
 * @brief       this file implements console
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-10-29    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRIVERS_CONSOLE_H_
#define _DRIVERS_CONSOLE_H_

#include <device.h>

os_device_t *os_console_set_device(const char *name);
os_device_t *os_console_get_device(void);
void os_hw_console_output(char *log_buff);


#endif /* _DRIVERS_DMA_H_ */
