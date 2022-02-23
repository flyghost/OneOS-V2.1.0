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
 * @file        cmiot_typedef.h
 *
 * @brief       The typedef header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_TYPE_H__
#define __CMIOT_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define cmiot_extern extern

#if defined(__GNUC__)
#ifndef cmiot_weak
#define cmiot_weak __attribute__((weak))
#endif                             /* cmiot_weak */
#define CMIOT_ASM           __asm  /* !< asm keyword for GNU Compiler */
#define CMIOT_INLINE        inline /* !< inline keyword for GNU Compiler */
#define CMIOT_STATIC_INLINE static inline
#define CMIOT_ALIGN(x)      __attribute__((aligned(x)))
#elif defined(__ICCARM__) || defined(__ICCRX__) || defined(__ICCSTM8__)
#ifndef cmiot_weak
#define cmiot_weak __weak
#endif                             /* cmiot_weak */
#define CMIOT_ASM           __asm  /* !< asm keyword for IAR Compiler */
#define CMIOT_INLINE        inline /* !< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define CMIOT_STATIC_INLINE static inline
#define CMIOT_ALIGN(x)      __attribute__((aligned(x)))
#elif defined(__CC_ARM)
#ifndef cmiot_weak
#define cmiot_weak __weak
#endif                               /* cmiot_weak */
#define CMIOT_ASM           __asm    /* !< asm keyword for ARM Compiler */
#define CMIOT_INLINE        __inline /* !< inline keyword for ARM Compiler */
#define CMIOT_STATIC_INLINE static __inline
#define CMIOT_ALIGN(x)      __align(x)
#else
#error "Alignment not supported for this compiler."
#endif

typedef enum
{
    CMIOT_FILETYPE_APP,
    CMIOT_FILETYPE_PATCH,
    CMIOT_FILETYPE_BACKUP,
    CMIOT_FILETYPE_PATCH_INFO,
    CMIOT_FILETYPE_END
} cmiot_peer_filetype_t;

#define CMIOT_UPDATE_SUCCESS       1
#define CMIOT_UPDATE_AUTH_FAIL     98
#define CMIOT_UPDATE_FAIL          99
#define CMIOT_MID_MAXLEN           31
#define CMIOT_DEVICEID_MAX_LEN     31
#define CMIOT_DEVICESECRET_MAX_LEN 63

typedef char               cmiot_char;
typedef unsigned char      cmiot_uint8;
typedef signed char        cmiot_int8;
typedef unsigned short int cmiot_uint16;
typedef signed short int   cmiot_int16;

#ifdef CMIOT_8BIT
typedef unsigned long      cmiot_uint32;
typedef signed long        cmiot_int32;
typedef unsigned long long cmiot_uint64;
typedef signed long long   cmiot_int64;
#else
typedef unsigned int       cmiot_uint32;
typedef signed int         cmiot_int32;
typedef unsigned long long cmiot_uint64;
typedef signed long long   cmiot_int64;
#endif

typedef enum
{
    STATE_INIT = 1,
    STATE_CV, /* Check Version */
    STATE_DL, /* Download*/
    STATE_RD, /* Report download result*/
    STATE_RU, /* Report upgrade result */
    STATE_RG, /* Register */
    STATE_END
} flow_state_t;

#define E_CMIOT_SUCCESS      0
#define E_CMIOT_FAILURE      -1
#define E_CMIOT_NOMEMORY     -2
#define E_CMIOT_NOT_INITTED  -3
#define E_CMIOT_LAST_VERSION -4
#define E_CMIOT_NO_UPGRADE   -5

typedef enum
{
    CMIOT_DIFF_PATCH,
    CMIOT_FULL_PATCH,
    CMIOT_PATCH_END
} cmiot_patch_method_t;

typedef cmiot_uint8 cmiot_bool;
#define cmiot_true  ((cmiot_bool)1)
#define cmiot_false ((cmiot_bool)0)

#ifdef __cplusplus
}
#endif

#endif
