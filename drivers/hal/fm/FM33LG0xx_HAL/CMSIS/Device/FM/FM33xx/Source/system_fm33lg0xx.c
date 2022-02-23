 /**************************************************************************//**
 * @file     system_fm33lg0xx.c
 * @brief    CMSIS Cortex-M0 Device Peripheral Access Layer Source File for
 *           Device FM33LG0XX
 * @version  V2.00
 * @date     15. March 2021
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THES
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/


#include "fm33lg0xx.h"
/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
/* ToDo: add here your necessary defines for device initialization
         following is an example for different system frequencies             */

/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* ToDo: initialize SystemCoreClock with the system core clock frequency value
         achieved after system intitialization.
         This means system core clock frequency after call to SystemInit()    */
uint32_t SystemCoreClock = __SYSTEM_CLOCK;  /*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
    
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
    /* BUS Clock */
    CMU->PCLKCR1 |= 0x1U << 12;     // VREF
    CMU->PCLKCR1 |= 0x1U << 7;      // PAD
    
    /* DEBUG Config */
    DBG->CR = 0x10003;
    
    /* POWER Trim */
    PMU->ULPB_TR = ULPBG_TRIM;
    
    /* CLOCK Trim */
    CMU->RCLFTR = RCLF_TRIM;
    CMU->RCLPTR = RCLP_TRIM;
    PMU->ULPB_TR= ULPBG_TRIM;
    /* SWD PULLUP On */
    GPIOD->PUEN |= 0x3U << 7;
    
    /* PDR Config */
    RMU->PDRCR = 0x5;   // Enable 1.5v
    
    /* CDIF Config */
    CDIF->CR |= 0x1U << 1;  // VAO->CPU Enable
} 

/*----------------------------------------------------------------------------
  NVIC MFang Config functions
 *----------------------------------------------------------------------------*/
/**
  * @brief	NVIC_Init config NVIC
  *
  * @param 	nvicConfigStruct config params
  * @param 	IRQn             Interrupt number
  *
  * @retval	None
  */
void NVIC_Init(NVIC_ConfigTypeDef *nvicConfigStruct,IRQn_Type IRQn)
{
    /* Params Check */
    if(nvicConfigStruct->preemptPriority > 3)
    {
        nvicConfigStruct->preemptPriority = 3;
    }
	NVIC_DisableIRQ(IRQn);
	NVIC_SetPriority(IRQn, nvicConfigStruct->preemptPriority);
	NVIC_EnableIRQ(IRQn);
}



