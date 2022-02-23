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
 * @file        sdram_port.h
 *
 * @brief       This file provides macro definition for sdram.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SDRAM_PORT_H__
#define __SDRAM_PORT_H__

/* parameters for sdram peripheral */
/* Bank1 or Bank2 */
#define SDRAM_TARGET_BANK 1
/* stm32f4 Bank1:0XC0000000  Bank2:0XD0000000 */
#define SDRAM_BASE ((uint32_t)0XC0000000)

/* data width: 8, 16, 32 */
#define SDRAM_DATA_WIDTH 32
/* column bit numbers: 8, 9, 10, 11 */
#define SDRAM_COLUMN_BITS 8
/* row bit numbers: 11, 12, 13 */
#define SDRAM_ROW_BITS 12
/* cas latency clock number: 1, 2, 3 */
#define SDRAM_CAS_LATENCY 3
/* read pipe delay: 0, 1, 2 */
#define SDRAM_RPIPE_DELAY 0
/* clock divid: 2, 3 */
#define SDCLOCK_PERIOD 2
/* refresh rate counter */
#define SDRAM_REFRESH_COUNT ((uint32_t)0x0569)
#define SDRAM_SIZE          ((uint32_t)0x1000000)

/* Timing configuration for W9825G6KH-6 */
/* 90 MHz of SD clock frequency (180MHz/2) */
/* TMRD: 2 Clock cycles */
#define LOADTOACTIVEDELAY 2
/* TXSR: 7x11.90ns */
#define EXITSELFREFRESHDELAY 7
/* TRAS: 4x11.90ns */
#define SELFREFRESHTIME 4
/* TRC:  7x11.90ns */
#define ROWCYCLEDELAY 7
/* TWR:  3 Clock cycles */
#define WRITERECOVERYTIME 2
/* TRP:  2x11.90ns */
#define RPDELAY 2
/* TRCD: 2x11.90ns */
#define RCDDELAY 2

#endif    // SDRAM_PORT_H_
