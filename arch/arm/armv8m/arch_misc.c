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
 * @file        os_arch_misc.c
 *
 * @brief       This file provides misc related functions under the ARMv8-M architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>

#if defined(__GNUC__) || defined(__CLANG_ARM)

/**
 ***********************************************************************************************************************
 * @brief           os_ffs.
 *
 * @param[in]       value
 *
 * @return          Returns one plus the index of the least significant 1-bit of value, or if value is zero, returns zero.
 ***********************************************************************************************************************
 */
os_int32_t os_ffs(os_uint32_t value)
{
    return __builtin_ffs(value);
}

/**
 ***********************************************************************************************************************
 * @brief           Get the sp of the current task.
 *
 * @param           None
 *
 * @return          sp pointer
 ***********************************************************************************************************************
 */
void *os_get_current_task_sp(void)
{
    register void *result;

    __asm__ __volatile__(
        "   MRS %0, PSP\n"
        :   "=r"(result));

    return result;
}

#elif defined(__CC_ARM)

/**
 ***********************************************************************************************************************
 * @brief           os_ffs.
 *
 * @param[in]       value
 *
 * @return          Returns one plus the index of the least significant 1-bit of value, or if value is zero, returns zero.
 ***********************************************************************************************************************
 */
__asm os_int32_t os_ffs(os_uint32_t value)
{
    CMP     R0, #0x00
    BEQ     exit

    RBIT    R0, R0
    CLZ     R0, R0
    ADDS    R0, R0, #0x01

exit
    BX      LR
}

/**
 ***********************************************************************************************************************
 * @brief           Get the sp of the current task.
 *
 * @param           None
 *
 * @return          sp pointer
 ***********************************************************************************************************************
 */
__asm void *os_get_current_task_sp(void)
{
    MRS     R0, PSP
    BX      LR
}

#endif

os_int32_t os_fls(os_uint32_t value)
{
    os_int32_t pos;

    pos = 32;
     
    if (!value)
    {
        pos = 0;
    }
    else
    {
        if (!(value & 0xFFFF0000U))
        {
            value <<= 16;
            pos    -= 16;
        }
        
        if (!(value & 0xFF000000U))
        {
            value <<= 8;
            pos    -= 8;
        }
        
        if (!(value & 0xF0000000U))
        {
            value <<= 4;
            pos -= 4;
        }
        
        if (!(value & 0xC0000000U))
        {
            value <<= 2;
            pos -= 2;
        }
        
        if (!(value & 0x80000000U))
        {
            value <<= 1;
            pos -= 1;
        }
    }
    
    return pos;
}

