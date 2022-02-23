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
 * @file        utils.h
 *
 * @brief       util macro.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ECOREDUMP_UTILS_H__
#define __ECOREDUMP_UTILS_H__

#define MY_ALIGN(x,y)               (((x - 1) / y + 1) * y)
#define ELEM_OFFSET(s, e)           ((int)(((s*)0)->e))

/*
 * Arm Compiler 4/5
 */
#if   defined ( __CC_ARM )
#ifndef   __WEAK
  #define __WEAK                  __attribute__((weak))
#endif
/*
 * Arm Compiler 6 (armclang)
 */
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#ifndef   __WEAK
  #define __WEAK                  __attribute__((weak))
#endif
/*
 * GNU Compiler
 */
#elif defined ( __GNUC__ )
#ifndef   __WEAK
  #define __WEAK                  __attribute__((weak))
#endif
#endif

#define _1_0xFF               0xff
#define _2_0xFF               _1_0xFF,_1_0xFF
#define _4_0xFF               _2_0xFF,_2_0xFF
#define _8_0xFF               _4_0xFF,_4_0xFF
#define _16_0xFF              _8_0xFF,_8_0xFF
#define _32_0xFF              _16_0xFF,_16_0xFF
#define _64_0xFF              _32_0xFF,_32_0xFF
#define _128_0xFF             _64_0xFF,_64_0xFF
#define _256_0xFF             _128_0xFF,_128_0xFF
#define _512_0xFF             _256_0xFF,_256_0xFF
#define _1K_0xFF              _512_0xFF,_512_0xFF
#define _2K_0xFF              _1K_0xFF,_1K_0xFF
#define _4K_0xFF              _2K_0xFF,_2K_0xFF
#define _8K_0xFF              _4K_0xFF,_4K_0xFF
#define _16K_0xFF             _8K_0xFF,_8K_0xFF
#define _32K_0xFF             _16K_0xFF,_16K_0xFF
#define _64K_0xFF             _32K_0xFF,_32K_0xFF

#endif
