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
 * @file        drv_lcd.h
 *
 * @brief       This file implements lcd driver
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__
#include <drv_common.h>
#include "board.h"
#include "fm33g0xx_lib.h"

#define COM4_BIT_MASK(n)      LCD_COM_EN_COMEN##n_Msk
#define COM6_BIT_MASK(n)      LCD_SEG_EN1_COM##nENo6COM_Msk
#define COM8_BIT_MASK(n)      LCD_SEG_EN1_COM##nENo8COM_Msk

#define SEG0_31_BIT_MASK(n)   LCD_SEG_EN0_SEGEN##n_Msk
#define SEG32_43_BIT_MASK(n)  LCD_SEG_EN1_SEGEN##n_Msk

#define SEG(n)                ((os_uint64_t )1 << n)
#define COM(n)                ((os_uint8_t  )1 << n)


#define COM0_PIN  GET_PIN(A, 0)
#define COM1_PIN  GET_PIN(A, 1)
#define COM2_PIN  GET_PIN(A, 2)
#define COM3_PIN  GET_PIN(A, 3)

#define SEG00_PIN GET_PIN(A, 13)
#define SEG01_PIN GET_PIN(A, 12)
#define SEG02_PIN GET_PIN(A, 11)
#define SEG03_PIN GET_PIN(A, 10)
#define SEG04_PIN GET_PIN(A, 9)
#define SEG05_PIN GET_PIN(A, 8)
#define SEG06_PIN GET_PIN(A, 7)
#define SEG07_PIN GET_PIN(A, 6)
#define SEG08_PIN GET_PIN(A, 5)
#define SEG09_PIN GET_PIN(A, 4)
#define SEG10_PIN GET_PIN(B, 15)
#define SEG11_PIN GET_PIN(B, 14)
#define SEG12_PIN GET_PIN(B, 13)
#define SEG13_PIN GET_PIN(B, 12)
#define SEG14_PIN GET_PIN(B, 11)
#define SEG15_PIN GET_PIN(B, 10)
#define SEG16_PIN GET_PIN(B, 9)
#define SEG17_PIN GET_PIN(B, 8)
#define SEG18_PIN GET_PIN(B, 7)
#define SEG19_PIN GET_PIN(B, 6)
#define SEG20_PIN GET_PIN(B, 5)

#define MASK_ZERO         0xFC
#define MASK_ONE          0x60
#define MASK_TWO          0xDA
#define MASK_THREE        0xF2
#define MASK_FOURE        0x66
#define MASK_FIVE         0xB6
#define MASK_SIX          0xBE
#define MASK_SEVEN        0xE0
#define MASK_EIGHT        0xFE
#define MASK_NINE         0xF6
#define MASK_NONE         0x00

enum
{
    COM0 = 0x00,
    COM1,
    COM2,
    COM3,
    COM4,
    COM5,
    COM6,
    COM7,
};
    
enum
{
    SEG0 = 0x00,
    SEG1,
    SEG2,
    SEG3,
    SEG4,
    SEG5,
    SEG6,
    SEG7,
    SEG8,
    SEG9,
    SEG10,
    SEG11,
    SEG12,
    SEG13,
    SEG14,
    SEG15,
    SEG16,
    SEG17,
    SEG18,
    SEG19,
    SEG20,
    SEG21,
    SEG22,
    SEG23,
    SEG24,
    SEG25,
    SEG26,
    SEG27,
    SEG28,
    SEG29,
    SEG30,
    SEG31,
    SEG32,
    SEG33,
    SEG34,
    SEG35,
    SEG36,
    SEG37,
    SEG38,
    SEG39,
    SEG40,
    SEG41,
    SEG42,
    SEG43,
};
#define LCD_CRD_MAP_INFO                                \
{                                                       \
    {"T7",  {COM0, SEG5}},                              \
    {"S11", {COM0, SEG4}},                              \
    {"S13", {COM0, SEG3}},                              \
    {"S5",  {COM0, SEG2}},                              \
    {"T1",  {COM0, SEG1}},                              \
    {"1D",  {COM0, SEG0}},                              \
    {"1C",  {COM0, SEG41}},                             \
    {"2D",  {COM0, SEG40}},                             \
    {"2C",  {COM0, SEG43}},                             \
    {"3D",  {COM0, SEG42}},                             \
    {"3C",  {COM0, SEG17}},                             \
    {"4D",  {COM0, SEG16}},                             \
    {"4C",  {COM0, SEG15}},                             \
    {"5D",  {COM0, SEG14}},                             \
    {"5C",  {COM0, SEG13}},                             \
    {"6D",  {COM0, SEG12}},                             \
    {"6C",  {COM0, SEG11}},                             \
    {"7D",  {COM0, SEG10}},                             \
    {"7C",  {COM0, SEG9}},                              \
    {"8D",  {COM0, SEG8}},                              \
    {"8C",  {COM0, SEG7}},                              \
                                                        \
    {"T6",  {COM1, SEG5}},                              \
    {"S12", {COM1, SEG4}},                              \
    {"S6",  {COM1, SEG3}},                              \
    {"S4",  {COM1, SEG2}},                              \
    {"T2",  {COM1, SEG1}},                              \
    {"1E",  {COM1, SEG0}},                              \
    {"1G",  {COM1, SEG41}},                             \
    {"2E",  {COM1, SEG40}},                             \
    {"2G",  {COM1, SEG43}},                             \
    {"3E",  {COM1, SEG42}},                             \
    {"3G",  {COM1, SEG17}},                             \
    {"4E",  {COM1, SEG16}},                             \
    {"4G",  {COM1, SEG15}},                             \
    {"5E",  {COM1, SEG14}},                             \
    {"5G",  {COM1, SEG13}},                             \
    {"6E",  {COM1, SEG12}},                             \
    {"6G",  {COM1, SEG11}},                             \
    {"7E",  {COM1, SEG10}},                             \
    {"7G",  {COM1, SEG9}},                              \
    {"8E",  {COM1, SEG8}},                              \
    {"8G",  {COM1, SEG7}},                              \
                                                        \
    {"T5",  {COM2, SEG5}},                              \
    {"T8",  {COM2, SEG4}},                              \
    {"S7",  {COM2, SEG3}},                              \
    {"S3",  {COM2, SEG2}},                              \
    {"T3",  {COM2, SEG1}},                              \
    {"1F",  {COM2, SEG0}},                              \
    {"1B",  {COM2, SEG41}},                             \
    {"2F",  {COM2, SEG40}},                             \
    {"2B",  {COM2, SEG43}},                             \
    {"3F",  {COM2, SEG42}},                             \
    {"3B",  {COM2, SEG17}},                             \
    {"4F",  {COM2, SEG16}},                             \
    {"4B",  {COM2, SEG15}},                             \
    {"5F",  {COM2, SEG14}},                             \
    {"5B",  {COM2, SEG13}},                             \
    {"6F",  {COM2, SEG12}},                             \
    {"6B",  {COM2, SEG11}},                             \
    {"7F",  {COM2, SEG10}},                             \
    {"7B",  {COM2, SEG9}},                              \
    {"8F",  {COM2, SEG8}},                              \
    {"8B",  {COM2, SEG7}},                              \
                                                        \
    {"S9",  {COM3, SEG5}},                              \
    {"T4",  {COM3, SEG4}},                              \
    {"S8",  {COM3, SEG3}},                              \
    {"S2",  {COM3, SEG2}},                              \
    {"S1",  {COM3, SEG1}},                              \
    {"1A",  {COM3, SEG0}},                              \
    {"T10", {COM3, SEG41}},                             \
    {"2A",  {COM3, SEG40}},                             \
    {"T9",  {COM3, SEG43}},                             \
    {"3A",  {COM3, SEG42}},                             \
    {"P5",  {COM3, SEG17}},                             \
    {"4A",  {COM3, SEG16}},                             \
    {"P1",  {COM3, SEG15}},                             \
    {"5A",  {COM3, SEG14}},                             \
    {"P2",  {COM3, SEG13}},                             \
    {"6A",  {COM3, SEG12}},                             \
    {"P3",  {COM3, SEG11}},                             \
    {"7A",  {COM3, SEG10}},                             \
    {"P4",  {COM3, SEG9}},                              \
    {"8A",  {COM3, SEG8}},                              \
    {"S10", {COM3, SEG7}},                              \
}

struct lcd_num_seg_map
{
    char *seg_a;
    char *seg_b;
    char *seg_c;
    char *seg_d;
    char *seg_e;
    char *seg_f;
    char *seg_g;
};


struct lcd_num_map
{
    os_int8_t    val;
    os_uint8_t   mask;
};


#endif /* __DRV_LCD_H__ */



