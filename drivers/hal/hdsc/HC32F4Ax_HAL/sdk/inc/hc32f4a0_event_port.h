/**
 *******************************************************************************
 * @file  hc32f4a0_event_port.h
 * @brief This file contains all the functions prototypes of the Event Port
 *        driver library.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2020-12-15       Zhangxl         First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#ifndef __HC32F4A0_EVENT_PORT_H__
#define __HC32F4A0_EVENT_PORT_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_common.h"
#include "ddl_config.h"

/**
 * @addtogroup HC32F4A0_DDL_Driver
 * @{
 */

/**
 * @addtogroup DDL_EVENT_PORT
 * @{
 */

#if (DDL_EVENT_PORT_ENABLE == DDL_ON)
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/**
 * @brief  EVENT Pin Set and Reset enumeration
 */
typedef enum
{
    Event_Pin_Reset = 0U,           /*!< Pin reset    */
    Event_Pin_Set   = 1U            /*!< Pin set      */
} en_ep_state_t;

/**
 * @defgroup EP_Global_Types EP Global Types
 * @{
 */
typedef struct stc_ep_init
{
    uint32_t u32PinDir;         /*!< Input/Output setting, @ref EP_PinDirection_Sel for details */
    en_ep_state_t enPinState;   /*!< Corresponding pin initial state, @ref EP_InitState_Sel for details */
    uint32_t u32PinTriggerOps;  /*!< Corresponding pin state after triggered, @ref EP_TriggerOpsSel for details */
    uint32_t u32TriggerEdge;    /*!< Event port trigger edge, @ref EP_Trigger_Sel for details */
    uint32_t u32Filter;         /*!< Filter clock function setting, @ref EP_FilterClock_Sel for details */
    uint32_t u32FilterClk;      /*!< Filter clock, ref@ EP_FilterClock_Div for details */
}stc_ep_init_t;
/**
 * @}
 */

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup EP_Global_Macros EP Global Macros
 * @{
 */

/** @defgroup EP_Port_source EP port source
  * @{
  */
#define EVENT_PORT_1        (0UL)       /*!< Event port 1 */
#define EVENT_PORT_2        (1UL)       /*!< Event port 2 */
#define EVENT_PORT_3        (2UL)       /*!< Event port 3 */
#define EVENT_PORT_4        (3UL)       /*!< Event port 4 */
/**
 * @}
 */

/** @defgroup EP_pins_define EP pin source
 * @{
 */
#define EVENT_PIN_00        (0x0001U)   /*!< Event port Pin 00 */
#define EVENT_PIN_01        (0x0002U)   /*!< Event port Pin 01 */
#define EVENT_PIN_02        (0x0004U)   /*!< Event port Pin 02 */
#define EVENT_PIN_03        (0x0008U)   /*!< Event port Pin 03 */
#define EVENT_PIN_04        (0x0010U)   /*!< Event port Pin 04 */
#define EVENT_PIN_05        (0x0020U)   /*!< Event port Pin 05 */
#define EVENT_PIN_06        (0x0040U)   /*!< Event port Pin 06 */
#define EVENT_PIN_07        (0x0080U)   /*!< Event port Pin 07 */
#define EVENT_PIN_08        (0x0100U)   /*!< Event port Pin 08 */
#define EVENT_PIN_09        (0x0200U)   /*!< Event port Pin 09 */
#define EVENT_PIN_10        (0x0400U)   /*!< Event port Pin 10 */
#define EVENT_PIN_11        (0x0800U)   /*!< Event port Pin 11 */
#define EVENT_PIN_12        (0x1000U)   /*!< Event port Pin 12 */
#define EVENT_PIN_13        (0x2000U)   /*!< Event port Pin 13 */
#define EVENT_PIN_14        (0x4000U)   /*!< Event port Pin 14 */
#define EVENT_PIN_15        (0x8000U)   /*!< Event port Pin 15 */
#define EVENT_PIN_All       (0xFFFFU)   /*!< All event pins are selected */
#define EVENT_PIN_MASK      (0xFFFFU)   /*!< Event pin mask for assert test */
/**
 * @}
 */

/**
 * @defgroup EP_PinDirection_Sel EP pin input/output direction selection
 * @{
 */
#define EP_DIR_IN           (0UL)       /*!< EP input */
#define EP_DIR_OUT          (1UL)       /*!< EP output */
/**
 * @}
 */

/**
 * @defgroup EP_FilterClock_Sel Event port filter function selection
 * @{
 */
#define EP_FILTER_OFF       (0UL)       /*!< EP filter function OFF */
#define EP_FILTER_ON        (1UL)       /*!< EP filter function ON */
/**
 * @}
 */

/**
 * @defgroup EP_FilterClock_Div Event port filter sampling clock division selection
 * @{
 */
#define EP_FCLK_PCLK_DIV1   (0UL)       /*!< PCLK as EP filter clock source */
#define EP_FCLK_PCLK_DIV8   (1UL)       /*!< PCLK div8 as EP filter clock source */
#define EP_FCLK_PCLK_DIV32  (2UL)       /*!< PCLK div32 as EP filter clock source */
#define EP_FCLK_PCLK_DIV64  (3UL)       /*!< PCLK div64 as EP filter clock source */
/**
 * @}
 */

/**
 * @defgroup EP_Trigger_Sel Event port trigger edge selection
 * @{
 */
#define EP_TRIGGER_NONE     (0UL)       /*!< No Trigger by edge */
#define EP_TRIGGER_FALLING  (1UL)       /*!< Trigger by falling edge */
#define EP_TRIGGER_RISING   (2UL)       /*!< Trigger by rising edge */
#define EP_TRIGGER_BOTH     (3UL)       /*!< Trigger by falling and rising edge */
/**
 * @}
 */

/**
 * @defgroup EP_TriggerOpsSel Event port operation after triggered selection
 * @{
 */
#define EP_OPS_NONE         (0UL)       /*!< Pin no action after triggered */
#define EP_OPS_LOW          (1UL)       /*!< Pin ouput low after triggered */
#define EP_OPS_HIGH         (2UL)       /*!< Pin ouput high after triggered */
#define EP_OPS_TOGGLE       (3UL)       /*!< Pin toggle after triggered */
/**
 * @}
 */

/** @defgroup EP_Common_Trigger_Source_definition Event port common Trigger Source Config
 * @{
 */
#define EP_COM_TRIG1        (AOS_PEVNTTRGSR_COMTRG_EN_1)
#define EP_COM_TRIG2        (AOS_PEVNTTRGSR_COMTRG_EN_0)
#define EP_COM_TRIG_MASK    (AOS_PEVNTTRGSR_COMTRG_EN)
/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
/**
 * @addtogroup EP_Global_Functions
 * @{
 */
en_result_t EP_Init(uint8_t u8EventPort, uint16_t u16EventPin,                  \
                    const stc_ep_init_t *pstcEventPortInit);
void EP_DeInit(void);
en_result_t EP_StructInit(stc_ep_init_t *pstcEventPortInit);
en_result_t EP_SetTriggerEdge(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Edge);
en_result_t EP_SetTriggerOps(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Ops);
void EP_SetTriggerSrc(uint8_t u8EventPort, en_event_src_t enEvent);
void EP_ComTriggerCmd(uint8_t u8EventPort, uint32_t u32ComTrig,                 \
                    en_functional_state_t enNewState);
en_ep_state_t EP_ReadInputPins(uint8_t u8EventPort, uint16_t u16EventPin);
uint16_t EP_ReadInputPort(uint8_t u8EventPort);
en_ep_state_t EP_ReadOutputPins(uint8_t u8EventPort, uint16_t u16EventPin);
uint16_t EP_ReadOutputPort(uint8_t u8EventPort);
void EP_SetPins(uint8_t u8EventPort, uint16_t u16EventPin);
void EP_ResetPins(uint8_t u8EventPort, uint16_t u16EventPin);
void EP_SetDir(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Dir);

/**
 * @}
 */

#endif /* DDL_EVENT_PORT_ENABLE */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __HC32F4A0_EVENT_PORT_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
