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
 * @file        event_port.c
 *
 * @brief       This file implements some port functions of FreeRTOS event.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-15   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "os_task.h"
#include "arch_interrupt.h"
#include "../../../kernel/source/os_kernel_internal.h"

/* *INDENT-OFF* */

#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
    #define portCHAR          char
    #define portFLOAT         float
    #define portDOUBLE        double
    #define portLONG          long
    #define portSHORT         short
    #define portSTACK_TYPE    os_uint32_t
    #define portBASE_TYPE     long

    typedef portSTACK_TYPE   StackType_t;
    typedef long             BaseType_t;
    typedef unsigned long    UBaseType_t;

    #if ( configUSE_16_BIT_TICKS == 1 )
        typedef os_uint16_t     TickType_t;
        #define portMAX_DELAY              ( TickType_t ) 0xffff
    #else
        typedef os_uint32_t     TickType_t;
        #define portMAX_DELAY              ( TickType_t ) 0xffffffffUL

/* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
 * not need to be guarded with a critical section. */
        #define portTICK_TYPE_IS_ATOMIC    1
    #endif
/*-----------------------------------------------------------*/

/* Architecture specifics. */
    #define portSTACK_GROWTH          ( -1 )
    #define portTICK_PERIOD_MS        ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
    #define portBYTE_ALIGNMENT        8

/* Constants used with memory barrier intrinsics. */
    #define portSY_FULL_READ_WRITE    ( 15 )

/*-----------------------------------------------------------*/
    extern void xPortYIELD(void);

/* Scheduler utilities. */
    #define portYIELD()    xPortYIELD()

/*-----------------------------------------------------------*/

    #define portNVIC_INT_CTRL_REG     ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
    #define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )
    #define portEND_SWITCHING_ISR( xSwitchRequired )    if( xSwitchRequired != pdFALSE ) portYIELD()
    #define portYIELD_FROM_ISR( x )                     portEND_SWITCHING_ISR( x )
/*-----------------------------------------------------------*/

/* Critical section management. */
    extern void vPortEnterCritical(void);
    extern void vPortExitCritical(void);
    extern void vPortRaiseBASEPRI(void);
    extern uint32_t ulPortRaiseBASEPRI(void);
    extern void vPortSetBASEPRI(uint32_t ulNewMaskValue);

    extern os_ubase_t ux_critical_nest;

    #define portDISABLE_INTERRUPTS() os_irq_disable()
    #define portENABLE_INTERRUPTS() os_irq_enable()
    #define portENTER_CRITICAL() \
        do { \
            OS_ASSERT(OS_FALSE == os_is_irq_active()); \
            portDISABLE_INTERRUPTS(); \
            ux_critical_nest++; \
        } while (0)
    #define portEXIT_CRITICAL() \
         do { \
            OS_ASSERT(OS_FALSE == os_is_irq_active()); \
            OS_ASSERT(0 != ux_critical_nest); \
            if (0 == --ux_critical_nest) \
            { \
                portENABLE_INTERRUPTS(); \
            } \
        } while (0)
    #define portSET_INTERRUPT_MASK_FROM_ISR()         (UBaseType_t )os_irq_lock()
    #define portCLEAR_INTERRUPT_MASK_FROM_ISR( x )    os_irq_unlock( (os_ubase_t)x )

/*-----------------------------------------------------------*/

/* Tickless idle/low power functionality. */
    #ifndef portSUPPRESS_TICKS_AND_SLEEP
        extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
        #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )    vPortSuppressTicksAndSleep( xExpectedIdleTime )
    #endif
/*-----------------------------------------------------------*/

/* Port specific optimisations. */
    #ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
        #define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
    #endif

    #if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

/* Check the configuration. */
        #if ( configMAX_PRIORITIES > 32 )
            #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
        #endif

/* Store/clear the ready priorities in a bit map. */
        #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )    ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
        #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )     ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

/*-----------------------------------------------------------*/

        #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    uxTopPriority = ( 31UL - ( uint32_t ) __clz( ( uxReadyPriorities ) ) )

    #endif /* taskRECORD_READY_PRIORITY */
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
 * not necessary for to use this port.  They are defined so the common demo files
 * (which build with all the ports) will build. */
    #define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
    #define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )
/*-----------------------------------------------------------*/

    #ifdef configASSERT
        void vPortValidateInterruptPriority( void );
        #define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()    vPortValidateInterruptPriority()
    #endif

/* portNOP() is not required by this port. */
    #define portNOP()

    #define portINLINE              __inline

    #ifndef portFORCE_INLINE
        #define portFORCE_INLINE    __forceinline
    #endif

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* PORTMACRO_H */
