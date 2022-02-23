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
 * @file        board.h
 *
 * @brief       Board resource definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <drv_cfg.h>

extern unsigned char __bss_start;
extern unsigned char __bss_end;

#define OS_HW_HEAP_BEGIN    (void*)&__bss_end
#define OS_HW_HEAP_END      (void*)(0x80000000 + 64 * 1024 * 1024)

#ifdef OS_USING_PUSH_BUTTON
extern const struct push_button key_table[];
extern const int                key_table_size;
#endif

#ifdef OS_USING_LED
extern const led_t led_table[];
extern const int   led_table_size;
#endif

#endif /* _BOARD_H_ */
