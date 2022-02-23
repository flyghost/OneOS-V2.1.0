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
 * @file        atk_tflcd9341.h
 *
 * @brief       This file implements lcd mipi driver for atk_tflcd9341.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _ATK_TFLCD9341_H_
#define _ATK_TFLCD9341_H_

#include <board.h>
#include <drv_cfg.h>

/*point color*/
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430

/*scan direction:L->left,R->right,U->up,D->down*/
#define L2R_U2D  0 		
#define L2R_D2U  1 		
#define R2L_U2D  2 		
#define R2L_D2U  3 		

#define U2D_L2R  4 
#define U2D_R2L  5 
#define D2U_L2R  6 		
#define D2U_R2L  7
/*default scan direction*/
#define DFT_SCAN_DIR  L2R_U2D

struct atk_lcd_info
{
    os_uint16_t width;
    os_uint16_t height;
    os_uint16_t id;
    os_uint8_t  dir;
    os_uint16_t wramcmd;
    os_uint16_t setxcmd;
    os_uint16_t setycmd;
};

struct atk_lcd_mem
{
   volatile os_uint16_t reg;
   volatile os_uint16_t ram;
};
#define LCD_BASE        ((os_uint32_t)(0x6C000000 | 0x000007FE))



#endif /* _ATK_TFLCD9341_H_ */

