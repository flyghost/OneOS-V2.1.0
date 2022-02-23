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
 * @file        lcd_port.h
 *
 * @brief       This file provides macro definition for lcd.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef LCD_PORT_H_
#define LCD_PORT_H_

#include <oneos_config.h>

#define LCD_WIDTH          OS_GRAPHIC_WIDTH
#define LCD_HEIGHT         OS_GRAPHIC_HEIGHT
#define LCD_BITS_PER_PIXEL 16
#define LCD_PIXEL_FORMAT   OS_GRAPHIC_PIXEL_FORMAT_RGB565

#endif /* LCD_PORT_H_ */
