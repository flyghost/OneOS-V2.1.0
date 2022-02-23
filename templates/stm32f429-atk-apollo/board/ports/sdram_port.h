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

#ifndef SDRAM_PORT_H_
#define SDRAM_PORT_H_

#define SDRAM_INIT_DELAY 10000

#define SDRAM_REFRESH_COUNT 683

#define SDRAM_TARGET_BANK 1
#define SDRAM_COLUMN_BITS 9
#define SDRAM_ROW_BITS    13
#define SDRAM_DATA_WIDTH  16
#define SDRAM_CAS_LATENCY 3
#define SDCLOCK_PERIOD    2
#define SDRAM_RPIPE_DELAY 1

#define LOADTOACTIVEDELAY    2
#define EXITSELFREFRESHDELAY 7
#define SELFREFRESHTIME      4
#define ROWCYCLEDELAY        7
#define WRITERECOVERYTIME    2
#define RPDELAY              2
#define RCDDELAY             2

#define SDRAM_BASE (unsigned long)0xc0000000
#define SDRAM_SIZE (unsigned long)0x2000000

#endif /* SDRAM_PORT_H_ */
