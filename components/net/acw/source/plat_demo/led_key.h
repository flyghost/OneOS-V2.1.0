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
 * @file        led_key.h
 *
 * @brief       led_key declaration  
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-24   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef __LED_KEY_H__
#define __LED_KEY_H__

#include <ctype.h>

extern void acw_led_io_init(void);
extern void acw_key_io_init(void);
extern os_bool_t acw_check_clr_key_press(void);
extern void acw_red_led_open(void);
extern void acw_red_led_close(void);
extern void acw_green_led_open(void);
extern void acw_all_led_close(void);
extern void acw_powerplug_open(void);
extern void acw_powerplug_close(void);
#endif /* end of __LED_KEY_H__ */
