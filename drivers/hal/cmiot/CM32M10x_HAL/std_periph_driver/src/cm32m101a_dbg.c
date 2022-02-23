/*
  ******************************************************************************
  *
  * COPYRIGHT(c) 2020, China Mobile IOT
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of China Mobile IOT nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/**
 * @file cm32m101a_dbg.c
 * @author CMIOT
 * @version v1.0.0
 *
 * @COPYRIGHT(c) 2020, China Mobile IOT. All rights reserved.
 */
#include "cm32m101a_dbg.h"

/** @addtogroup cm32m101a_StdPeriph_Driver
 * @{
 */

/** @addtogroup DBG
 * @brief DBG driver modules
 * @{
 */

/** @addtogroup DBGMCU_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup DBGMCU_Private_Defines
 * @{
 */

#define IDCODE_DEVID_MASK ((uint32_t)0x00000FFF)
/**
 * @}
 */

/** @addtogroup DBGMCU_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup DBGMCU_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup DBGMCU_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup DBGMCU_Private_Functions
 * @{
 */

/**
 * @brief  Returns the device revision number.
 * @return Device revision identifier
 */
uint32_t DBG_GetRevNum(void)
{
    return (DBG->ID & 0x00FF);
}

/**
 * @brief  Returns the device identifier.
 * @return Device identifier
 */
uint32_t DBG_GetDevNum(void)
{
    uint32_t id = DBG->ID;
    return ((id & 0x00F00000) >> 20) | ((id & 0xFF00) >> 4);
}

/**
 * @brief  Configures the specified peripheral and low power mode behavior
 *   when the MCU under Debug mode.
 * @param DBG_Periph specifies the peripheral and low power mode.
 *   This parameter can be any combination of the following values:
 *     @arg DBG_SLEEP Keep debugger connection during SLEEP mode
 *     @arg DBG_STOP Keep debugger connection during STOP mode
 *     @arg DBG_STDBY Keep debugger connection during STANDBY mode
 *     @arg DBG_IWDG_STOP Debug IWDG stopped when Core is halted
 *     @arg DBG_WWDG_STOP Debug WWDG stopped when Core is halted
 *     @arg DBG_TIM1_STOP TIM1 counter stopped when Core is halted
 *     @arg DBG_TIM2_STOP TIM2 counter stopped when Core is halted
 *     @arg DBG_TIM3_STOP TIM3 counter stopped when Core is halted
 *     @arg DBG_TIM4_STOP TIM4 counter stopped when Core is halted
 *     @arg DBG_CAN_STOP Debug CAN stopped when Core is halted
 *     @arg DBG_I2C1SMBUS_TIMEOUT I2C1 SMBUS timeout mode stopped when Core is halted
 *     @arg DBG_I2C2SMBUS_TIMEOUT I2C2 SMBUS timeout mode stopped when Core is halted
 *     @arg DBG_TIM8_STOP TIM8 counter stopped when Core is halted
 *     @arg DBG_TIM5_STOP TIM5 counter stopped when Core is halted
 *     @arg DBG_TIM6_STOP TIM6 counter stopped when Core is halted
 *     @arg DBG_TIM7_STOP TIM7 counter stopped when Core is halted
 *     @arg DBG_TIM9_STOP TIM9 counter stopped when Core is halted

 * @param Cmd new state of the specified peripheral in Debug mode.
 *   This parameter can be: ENABLE or DISABLE.
 */
void DBG_ConfigPeriph(uint32_t DBG_Periph, FunctionalState Cmd)
{
    /* Check the parameters */
    assert_param(IS_DBGMCU_PERIPH(DBG_Periph));
    assert_param(IS_FUNCTIONAL_STATE(Cmd));

    if (Cmd != DISABLE)
    {
        DBG->CTRL |= DBG_Periph;
    }
    else
    {
        DBG->CTRL &= ~DBG_Periph;
    }
}

/**
 * @brief Get FLASH size of this chip.
 *
 * @return FLASH size in bytes.
 */
uint32_t DBG_GetFlashSize(void)
{
    return (DBG->ID & 0x000F0000);
}

/**
 * @brief Get SRAM size of this chip.
 *
 * @return SRAM size in bytes.
 */
uint32_t DBG_GetSramSize(void)
{
    return (((DBG->ID & 0xF0000000) >> 28) + 1) << 14;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
