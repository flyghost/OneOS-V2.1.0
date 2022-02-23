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
 * @file        arch_exception_ntz.h
 *
 * @brief       This file provides functions related to the ARM M4 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_ARCH_EXCEPTION_NTZ_H__
#define __OS_ARCH_EXCEPTION_NTZ_H__

#include <oneos_config.h>
#include <os_types.h>


#ifdef __cplusplus
    extern "C" {
#endif

#define SCB_CFSR            (*(volatile const unsigned int *)0xE000ED28)        /* Configurable Fault Status Register */
#define SCB_HFSR            (*(volatile const unsigned int *)0xE000ED2C)        /* HardFault Status Register */
#define SCB_MMAR            (*(volatile const unsigned int *)0xE000ED34)        /* MemManage Fault Address register */
#define SCB_BFAR            (*(volatile const unsigned int *)0xE000ED38)        /* Bus Fault Address Register */
#define SCB_AIRCR           (*(volatile unsigned long *)0xE000ED0C)         /* Reset control Address Register */
#define SCB_RESET_VALUE     0x05FA0004                                      /* Reset value, write to SCB_AIRCR can reset cpu */
      
#define SCB_CFSR_MFSR       (*(volatile const unsigned char*)0xE000ED28)    /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR       (*(volatile const unsigned char*)0xE000ED29)    /* Bus Fault Status Register */
#define SCB_CFSR_UFSR       (*(volatile const unsigned short*)0xE000ED2A)   /* Usage Fault Status Register */

/* 1 (Stack frame contains floating point) or 0 (Stack frame does not contain floating point) */
#define EXCEPTION_STACK_FRAME_TYPE_MASK             0x00000010
/* 1 (Return to Thread) or 0 (Return to Handler) */
#define EXCEPTION_RETURN_MODE_MASK                  0x00000008
#define EXCEPTION_RETURN_MODE_THREAD                0x00000008
#define EXCEPTION_RETURN_MODE_HANDLER               0x00000000

/* 1 (Return with Process Stack) or 0 (Return with Main Stack)*/
#define EXCEPTION_RETURN_STACK_MASK                 0x00000004              




#ifdef STACK_TRACE_EN

#include <stack_trace.h>

extern void _arch_exception_stack_show(void *stack_frame, os_size_t *msp, os_size_t *psp);

#if defined(__CLANG_ARM)
        
    #ifndef CSTACK_BLOCK_NAME
    #define CSTACK_BLOCK_NAME          STACK
    #endif
                          
    #ifndef CODE_SECTION_NAME
    #define CODE_SECTION_NAME          ER_IROM1
    #endif
                          
#elif defined(__CC_ARM)

    #ifndef CSTACK_BLOCK_NAME
    #define CSTACK_BLOCK_NAME          STACK
    #endif
          
    #ifndef CODE_SECTION_NAME
    #define CODE_SECTION_NAME          ER_IROM1
    #endif
          
#elif defined(__ICCARM__)
    #ifndef CSTACK_BLOCK_NAME
    #define CSTACK_BLOCK_NAME          "CSTACK"
    #endif
          
    #ifndef CODE_SECTION_NAME
    #define CODE_SECTION_NAME          ".text"
    #endif
          
#elif defined(__GNUC__)
    #ifndef CSTACK_BLOCK_START
    #define CSTACK_BLOCK_START         _sstack
    #endif
           
    #ifndef CSTACK_BLOCK_END
    #define CSTACK_BLOCK_END           _estack
    #endif
          
    #ifndef CODE_SECTION_START
    #define CODE_SECTION_START         _stext
    #endif
          
    #ifndef CODE_SECTION_END
    #define CODE_SECTION_END           _etext
    #endif
#else
    #error "not supported compiler"
#endif
          
#if defined(__CLANG_ARM)
        
    #define SECTION_START(_name_)                _name_##$$Base
    #define SECTION_END(_name_)                  _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)          Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)            Image$$##_name_##$$Limit

    #define CSTACK_BLOCK_START(_name_)           SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)             SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)           IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)             IMAGE_SECTION_END(_name_)
              
        extern const int CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
        extern const int CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
        extern const int CODE_SECTION_START(CODE_SECTION_NAME);
        extern const int CODE_SECTION_END(CODE_SECTION_NAME);
        
#elif defined(__CC_ARM)

    #define SECTION_START(_name_)                _name_##$$Base
    #define SECTION_END(_name_)                  _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)          Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)            Image$$##_name_##$$Limit
        
    #define CSTACK_BLOCK_START(_name_)           SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)             SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)           IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)             IMAGE_SECTION_END(_name_)
          
        extern const int CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
        extern const int CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
        extern const int CODE_SECTION_START(CODE_SECTION_NAME);
        extern const int CODE_SECTION_END(CODE_SECTION_NAME);
          
#elif defined(__ICCARM__)
    #pragma section=CSTACK_BLOCK_NAME
    #pragma section=CODE_SECTION_NAME
              
#elif defined(__GNUC__)
              extern const int CSTACK_BLOCK_START;
              extern const int CSTACK_BLOCK_END;
              extern const int CODE_SECTION_START;
              extern const int CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif

extern os_err_t  task_stack_show(char *name, os_uint32_t context);

extern os_bool_t disassembly_ins_is_exc_return(os_size_t ins);

extern os_bool_t disassembly_ins_is_bl_blx(os_uint32_t addr);

extern void trace_stack(os_size_t *stack_top, os_size_t *stack_bottom, call_back_trace_t *trace);

#endif  /* STACK_TRACE_EN */

#ifdef __cplusplus
    }
#endif

#endif /* __OS_ARCH_EXCEPTION_H__ */

