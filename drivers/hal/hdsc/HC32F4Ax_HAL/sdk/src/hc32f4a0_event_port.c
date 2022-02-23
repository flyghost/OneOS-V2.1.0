/**
 *******************************************************************************
 * @file  hc32f4a0_event_port.c
 * @brief This file provides firmware functions to manage the Event Port (EP).
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

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f4a0_event_port.h"
#include "hc32f4a0_utility.h"

/**
 * @addtogroup HC32F4A0_DDL_Driver
 * @{
 */

/**
 * @defgroup DDL_EVENT_PORT EP
 * @brief EP Driver Library
 * @{
 */

#if (DDL_EVENT_PORT_ENABLE == DDL_ON)

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/**
 * @defgroup EP_Local_Macros EP Local Macros
 * @{
 */
#define EP1_BASE        (0x40010800UL + 0x0100UL)
#define EP2_BASE        (0x40010800UL + 0x011CUL)
#define EP3_BASE        (0x40010800UL + 0x0138UL)
#define EP4_BASE        (0x40010800UL + 0x0154UL)
#define EP1_DIR_BASE    (0x00UL)
#define EP1_IDR_BASE    (0x04UL)
#define EP1_ODR_BASE    (0x08UL)
#define EP1_ORR_BASE    (0x0CUL)
#define EP1_OSR_BASE    (0x10UL)
#define EP1_RISR_BASE   (0x14UL)
#define EP1_FAL_BASE    (0x18UL)
#define EP_NFCR_BASE    (0x40010800UL + 0x0170UL)

/**
 * @defgroup EP_Check_Parameters_Validity EP Check Parameters Validity
 * @{
 */
/*! Parameter validity check for port group. */
#define IS_EVENT_PORT(port)                                                     \
(   ((port) == EVENT_PORT_1)                    ||                              \
    ((port) == EVENT_PORT_2)                    ||                              \
    ((port) == EVENT_PORT_3)                    ||                              \
    ((port) == EVENT_PORT_4))

/*! Parameter validity check for pin. */
#define IS_EVENT_PIN(pin)               (((pin) & EVENT_PIN_MASK ) != 0x0000U)

/*! Parameter valid check for event port common trigger configuration. */
#define IS_EP_COM_TRIG(comtrig)                                                 \
(   ((comtrig) != 0x00UL)                       &&                              \
    (((comtrig) | EP_COM_TRIG_MASK) == EP_COM_TRIG_MASK))

/*! Parameter valid check for event port operation after triggered. */
#define IS_EP_OPS(ops)                                                          \
(   ((ops) == EP_OPS_NONE)                      ||                              \
    ((ops) == EP_OPS_LOW)                       ||                              \
    ((ops) == EP_OPS_HIGH)                      ||                              \
    ((ops) == EP_OPS_TOGGLE))

/*! Parameter valid check for event port trigger edge. */
#define IS_EP_TRIG_EDGE(edge)                                                   \
(   ((edge) == EP_TRIGGER_NONE)                 ||                              \
    ((edge) == EP_TRIGGER_FALLING)              ||                              \
    ((edge) == EP_TRIGGER_RISING)               ||                              \
    ((edge) == EP_TRIGGER_BOTH))

/*! Parameter valid check for event port direction. */
#define IS_EP_DIR(dir)                                                          \
(   ((dir) == EP_DIR_IN)                        ||                              \
    ((dir) == EP_DIR_OUT))

/*! Parameter valid check for event port filter function. */
#define IS_EP_FILTER(filter)                                                    \
(   ((filter) == EP_FILTER_OFF)                 ||                              \
    ((filter) == EP_FILTER_ON))

/*! Parameter valid check for event port filter clock div. */
#define IS_EP_FILTER_CLK(clk)                                                   \
(   ((clk) == EP_FCLK_PCLK_DIV1)                ||                              \
    ((clk) == EP_FCLK_PCLK_DIV8)                ||                              \
    ((clk) == EP_FCLK_PCLK_DIV32)               ||                              \
    ((clk) == EP_FCLK_PCLK_DIV64))

/*! Parameter valid check for event port initial output state. */
#define IS_EP_STATE(state)                                                      \
(   ((state) == Event_Pin_Reset)                ||                              \
    ((state) == Event_Pin_Set))

/**
 * @}
 */

/**
 * @}
 */

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief   Event Port init
 **
 ** \param   [in]  u8EventPort          Event port index, This parameter can be
 **                                     any value of @ref EP_Port_source
 ** \param   [in]  u16EventPin          Event pin index, This parameter can be
 **                                     any composed value of @ref EP_pins_define
 ** \param   [in]  pstcEventPortInit    Structure pointer of event port configuration
 **
 ** \retval  Ok                         Init successful
 **          ErrorInvalidParameter      Event port index invalid
 **
 ******************************************************************************/
/**
 * @brief  Initialize Event Port.
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP port peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @param  [in] pstcEventPortInit Pointer to a stc_ep_init_t structure that
 *                                contains configuration information.
 * @retval Ok: Event Port initialize successful
 *         ErrorInvalidParameter: NULL pointer
 */
en_result_t EP_Init(uint8_t u8EventPort, uint16_t u16EventPin, const stc_ep_init_t *pstcEventPortInit)
{
    uint16_t u16PinPos;
    __IO uint32_t *EPDIRx;                  /* Direction register */
    __IO uint32_t *EPODRx;                  /* Output data register */
    __IO uint32_t *EPORRx;                  /* Reset after trigger enable register */
    __IO uint32_t *EPOSRx;                  /* Set after trigger enable register */
    __IO uint32_t *EPRISRx;                 /* Rising edge detect enable register */
    __IO uint32_t *EPFALx;                  /* Falling edge detect enable register */
    en_result_t enRet = Ok;

    EPDIRx = (__IO uint32_t *)(EP1_BASE + EP1_DIR_BASE + (0x1CUL * u8EventPort));
    EPODRx = (__IO uint32_t *)(EP1_BASE + EP1_ODR_BASE + (0x1CUL * u8EventPort));
    EPORRx = (__IO uint32_t *)(EP1_BASE + EP1_ORR_BASE + (0x1CUL * u8EventPort));
    EPOSRx = (__IO uint32_t *)(EP1_BASE + EP1_OSR_BASE + (0x1CUL * u8EventPort));
    EPRISRx= (__IO uint32_t *)(EP1_BASE + EP1_RISR_BASE+ (0x1CUL * u8EventPort));
    EPFALx = (__IO uint32_t *)(EP1_BASE + EP1_FAL_BASE + (0x1CUL * u8EventPort));

    if (NULL == pstcEventPortInit)
    {
        enRet = ErrorInvalidParameter;
    }
    else
    {
        DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
        DDL_ASSERT(IS_EVENT_PIN(u16EventPin));
        DDL_ASSERT(IS_EP_OPS(pstcEventPortInit->u32PinTriggerOps));
        DDL_ASSERT(IS_EP_TRIG_EDGE(pstcEventPortInit->u32TriggerEdge));
        DDL_ASSERT(IS_EP_DIR(pstcEventPortInit->u32PinDir));
        DDL_ASSERT(IS_EP_FILTER(pstcEventPortInit->u32Filter));
        DDL_ASSERT(IS_EP_FILTER_CLK(pstcEventPortInit->u32FilterClk));
        DDL_ASSERT(IS_EP_STATE(pstcEventPortInit->enPinState));

        for (u16PinPos = 0U; u16PinPos < 16U; u16PinPos++)
        {
            if ((u16EventPin & (1UL<<u16PinPos)) != 0U)
            {
                /* Direction config */
                if (EP_DIR_OUT == pstcEventPortInit->u32PinDir)
                {
                    SET_REG32_BIT(*EPDIRx, u16EventPin);
                }
                else
                {
                    CLEAR_REG32_BIT(*EPDIRx, u16EventPin);
                }
                /* Set pin initial output value */
                if (Event_Pin_Set == pstcEventPortInit->enPinState)
                {
                    SET_REG32_BIT(*EPODRx, u16EventPin);
                }
                else
                {
                    CLEAR_REG32_BIT(*EPODRx, u16EventPin);
                }
                /* Set Pin operation after triggered */
                switch (pstcEventPortInit->u32PinTriggerOps)
                {
                    case EP_OPS_NONE:
                        CLEAR_REG32_BIT(*EPORRx, u16EventPin);
                        CLEAR_REG32_BIT(*EPOSRx, u16EventPin);
                        break;
                    case EP_OPS_LOW:
                        SET_REG32_BIT(*EPORRx, u16EventPin);
                        CLEAR_REG32_BIT(*EPOSRx, u16EventPin);
                        break;
                    case EP_OPS_HIGH:
                        CLEAR_REG32_BIT(*EPORRx, u16EventPin);
                        SET_REG32_BIT(*EPOSRx, u16EventPin);
                        break;
                    case EP_OPS_TOGGLE:
                        SET_REG32_BIT(*EPORRx, u16EventPin);
                        SET_REG32_BIT(*EPOSRx, u16EventPin);
                        break;
                    default:
                        enRet = ErrorInvalidParameter;
                        break;
                }
                /* Set trigger edge */
                switch (pstcEventPortInit->u32TriggerEdge)
                {
                    case EP_TRIGGER_NONE:
                        CLEAR_REG32_BIT(*EPFALx, u16EventPin);
                        CLEAR_REG32_BIT(*EPRISRx, u16EventPin);
                        break;
                    case EP_TRIGGER_FALLING:
                        SET_REG32_BIT(*EPFALx, u16EventPin);
                        CLEAR_REG32_BIT(*EPRISRx, u16EventPin);
                        break;
                    case EP_TRIGGER_RISING:
                        CLEAR_REG32_BIT(*EPFALx, u16EventPin);
                        SET_REG32_BIT(*EPRISRx, u16EventPin);
                        break;
                    case EP_TRIGGER_BOTH:
                        SET_REG32_BIT(*EPFALx, u16EventPin);
                        SET_REG32_BIT(*EPRISRx, u16EventPin);
                        break;
                    default:
                        enRet = ErrorInvalidParameter;
                        break;
                }
            }
            switch (u8EventPort)
            {
                case EVENT_PORT_1:
                    MODIFY_REG32(M4_AOS->PEVNTNFCR,                             \
                                (pstcEventPortInit->u32Filter |                 \
                                 pstcEventPortInit->u32FilterClk),              \
                                (AOS_PEVNTNFCR_NFEN1 |AOS_PEVNTNFCR_DIVS1));
                    break;
                case EVENT_PORT_2:
                    MODIFY_REG32(M4_AOS->PEVNTNFCR,                             \
                                (pstcEventPortInit->u32Filter |                 \
                                 pstcEventPortInit->u32FilterClk) << 8UL,       \
                                (AOS_PEVNTNFCR_NFEN2 | AOS_PEVNTNFCR_DIVS2));
                    break;
                case EVENT_PORT_3:
                    MODIFY_REG32(M4_AOS->PEVNTNFCR,                             \
                                (pstcEventPortInit->u32Filter |                 \
                                 pstcEventPortInit->u32FilterClk) << 16UL,      \
                                (AOS_PEVNTNFCR_NFEN3 | AOS_PEVNTNFCR_DIVS3));
                    break;
                case EVENT_PORT_4:
                    MODIFY_REG32(M4_AOS->PEVNTNFCR,                             \
                                (pstcEventPortInit->u32Filter |                 \
                                 pstcEventPortInit->u32FilterClk) << 24UL,      \
                                (AOS_PEVNTNFCR_NFEN4 | AOS_PEVNTNFCR_DIVS4));
                    break;
                default:
                    enRet = ErrorInvalidParameter;
                    break;
            }
        }
    }
    return enRet;
}

/**
 * @brief  De-init Event Port register to default value
 * @param  None
 * @retval None
 */
void EP_DeInit(void)
{
    __IO uint32_t EPDIRx ;
    __IO uint32_t EPODRx ;
    __IO uint32_t EPORRx ;
    __IO uint32_t EPOSRx ;
    __IO uint32_t EPRISRx;
    __IO uint32_t EPFALx ;
    uint8_t u8EPCnt;

    EPDIRx = (uint32_t)(EP1_BASE + EP1_DIR_BASE);
    EPODRx = (uint32_t)(EP1_BASE + EP1_ODR_BASE);
    EPORRx = (uint32_t)(EP1_BASE + EP1_ORR_BASE);
    EPOSRx = (uint32_t)(EP1_BASE + EP1_OSR_BASE);
    EPRISRx= (uint32_t)(EP1_BASE + EP1_RISR_BASE);
    EPFALx = (uint32_t)(EP1_BASE + EP1_FAL_BASE);

    /* Restore all registers to default value */
    M4_AOS->PEVNTTRGSR12 = 0x1FFUL;
    M4_AOS->PEVNTTRGSR34 = 0x1FFUL;
    M4_AOS->PEVNTNFCR = 0UL;
    for (u8EPCnt = 0U; u8EPCnt < 4U; u8EPCnt++)
    {
        *(__IO uint32_t *)(EPDIRx + 0x1CUL * u8EPCnt) = 0UL;
        *(__IO uint32_t *)(EPODRx + 0x1CUL * u8EPCnt) = 0UL;
        *(__IO uint32_t *)(EPORRx + 0x1CUL * u8EPCnt) = 0UL;
        *(__IO uint32_t *)(EPOSRx + 0x1CUL * u8EPCnt) = 0UL;
        *(__IO uint32_t *)(EPRISRx+ 0x1CUL * u8EPCnt) = 0UL;
        *(__IO uint32_t *)(EPFALx + 0x1CUL * u8EPCnt) = 0UL;
    }
}

/**
 * @brief  Initialize Event Port config structure. Fill each pstcEventPortInit with default value
 * @param  [in] pstcEventPortInit: Pointer to a stc_ep_init_t structure that
 *                                 contains configuration information.
 * @retval Ok: Event Port structure initialize successful
 *         ErrorInvalidParameter: NULL pointer
 */
en_result_t EP_StructInit(stc_ep_init_t *pstcEventPortInit)
{
    en_result_t enRet = Ok;
    /* Check if pointer is NULL */
    if (NULL == pstcEventPortInit)
    {
        enRet = ErrorInvalidParameter;
    }
    else
    {
        /* Reset Event Port init structure parameters values */
        pstcEventPortInit->u32PinDir        = EP_DIR_IN;
        pstcEventPortInit->enPinState       = Event_Pin_Reset;
        pstcEventPortInit->u32PinTriggerOps = EP_OPS_NONE;
        pstcEventPortInit->u32TriggerEdge   = EP_TRIGGER_NONE;
        pstcEventPortInit->u32Filter        = EP_FILTER_OFF;
        pstcEventPortInit->u32FilterClk     = EP_FCLK_PCLK_DIV1;
    }
    return enRet;
}

en_result_t EP_SetTriggerEdge(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Edge)
{
    __IO uint32_t *EPRISRx;
    __IO uint32_t *EPFALx;
    en_result_t enRet = Ok;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));
    DDL_ASSERT(IS_EP_TRIG_EDGE(u32Edge));

    EPRISRx= (__IO uint32_t *)(EP1_BASE + EP1_RISR_BASE+ (0x1CUL * u8EventPort));
    EPFALx = (__IO uint32_t *)(EP1_BASE + EP1_FAL_BASE + (0x1CUL * u8EventPort));

    /* Set trigger edge */
    switch (u32Edge)
    {
        case EP_TRIGGER_NONE:
            CLEAR_REG32_BIT(*EPFALx, u16EventPin);
            CLEAR_REG32_BIT(*EPRISRx, u16EventPin);
            break;
        case EP_TRIGGER_FALLING:
            SET_REG32_BIT(*EPFALx, u16EventPin);
            CLEAR_REG32_BIT(*EPRISRx, u16EventPin);
            break;
        case EP_TRIGGER_RISING:
            CLEAR_REG32_BIT(*EPFALx, u16EventPin);
            SET_REG32_BIT(*EPRISRx, u16EventPin);
            break;
        case EP_TRIGGER_BOTH:
            SET_REG32_BIT(*EPFALx, u16EventPin);
            SET_REG32_BIT(*EPRISRx, u16EventPin);
            break;
        default:
            enRet = ErrorInvalidParameter;
            break;
    }
    return enRet;
}


en_result_t EP_SetTriggerOps(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Ops)
{
    __IO uint32_t *EPORRx;
    __IO uint32_t *EPOSRx;
    en_result_t enRet = Ok;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));
    DDL_ASSERT(IS_EP_OPS(u32Ops));

    EPORRx = (__IO uint32_t *)(EP1_BASE + EP1_ORR_BASE + (0x1CUL * u8EventPort));
    EPOSRx = (__IO uint32_t *)(EP1_BASE + EP1_OSR_BASE + (0x1CUL * u8EventPort));

    switch (u32Ops)
    {
        case EP_OPS_NONE:
            CLEAR_REG32_BIT(*EPORRx, u16EventPin);
            CLEAR_REG32_BIT(*EPOSRx, u16EventPin);
            break;
        case EP_OPS_LOW:
            SET_REG32_BIT(*EPORRx, u16EventPin);
            CLEAR_REG32_BIT(*EPOSRx, u16EventPin);
            break;
        case EP_OPS_HIGH:
            CLEAR_REG32_BIT(*EPORRx, u16EventPin);
            SET_REG32_BIT(*EPOSRx, u16EventPin);
            break;
        case EP_OPS_TOGGLE:
            SET_REG32_BIT(*EPORRx, u16EventPin);
            SET_REG32_BIT(*EPOSRx, u16EventPin);
            break;
        default:
            enRet = ErrorInvalidParameter;
            break;
    }
    return enRet;
}

/**
 * @brief  Event Port Hardware trigger event configuration
 * @param  [in] u8EventPort     Event port index, This parameter can be any value
 *                              @ref EP_Port_source
 * @param  [in] enEvent         Event configuration for event port hardware trigger
 *                              @ref en_event_src_t
 * @retval None
 */
void EP_SetTriggerSrc(uint8_t u8EventPort, en_event_src_t enEvent)
{
    __IO uint32_t *EP_TRGSRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));

    EP_TRGSRx = (__IO uint32_t *)((uint32_t)&M4_AOS->PEVNTTRGSR12 + (4UL * ((uint32_t)u8EventPort/2UL)));
    MODIFY_REG32(*EP_TRGSRx, AOS_PEVNTTRGSR_TRGSEL, enEvent);
}

/**
 * @brief  Event Port Hardware trigger common event function command
 * @param  [in] u8EventPort     EVENT_PORT_x, x can be (1~4) to select the EP port peripheral
 * @param  [in] u32ComTrig      Common trigger event enable.
 *                              This parameter can be one of the following values:
 *   @arg  EP_COM_TRIG1: Common trigger event 1.
 *   @arg  EP_COM_TRIG2: Common trigger event 2.
 * @param  [in] enNewState      New state of common trigger function.
 * @retval none
 */
void EP_ComTriggerCmd(uint8_t u8EventPort, uint32_t u32ComTrig, en_functional_state_t enNewState)
{
    __IO uint32_t *EP_TRGSRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EP_COM_TRIG(u32ComTrig));
    DDL_ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    EP_TRGSRx = (__IO uint32_t *)((uint32_t)&M4_AOS->PEVNTTRGSR12 + (4UL * ((uint32_t)u8EventPort/2UL)));

    if (Enable == enNewState)
    {
        SET_REG32_BIT(*EP_TRGSRx, u32ComTrig);
    }
    else
    {
        CLEAR_REG32_BIT(*EP_TRGSRx, u32ComTrig);
    }
}

/**
 * @brief  Read specified Event port input data port pins
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @retval Specified Event port pin input value
 */
en_ep_state_t EP_ReadInputPins(uint8_t u8EventPort, uint16_t u16EventPin)
{
    __IO uint32_t *PEVNTIDRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));

    PEVNTIDRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTIDR1) + (0x1CUL * u8EventPort));

    return ((READ_REG32(*PEVNTIDRx) & (u16EventPin)) != 0UL) ? Event_Pin_Set : Event_Pin_Reset;
}

/**
 * @brief  Read specified Event port input data port
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the Event Port peripheral
 * @retval Specified Event Port input value
 */
uint16_t EP_ReadInputPort(uint8_t u8EventPort)
{
    __IO uint32_t *PEVNTIDRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));

    PEVNTIDRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTIDR1) + (0x1CUL * u8EventPort));
    return (uint16_t)(READ_REG32(*PEVNTIDRx) & 0xFFFFUL);
}

/**
 * @brief  Read specified Event port output data port pins
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @retval Specified Event port pin output value
 */
en_ep_state_t EP_ReadOutputPins(uint8_t u8EventPort, uint16_t u16EventPin)
{
    __IO uint32_t *PEVNTODRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));

    PEVNTODRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTODR1) + (0x1CUL * u8EventPort));
    return ((READ_REG32(*PEVNTODRx) & (u16EventPin)) != 0UL) ? Event_Pin_Set : Event_Pin_Reset;
}

/**
 * @brief  Read specified Event port output data port
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the Event Port peripheral
 * @retval Specified Event Port output value
 */
uint16_t EP_ReadOutputPort(uint8_t u8EventPort)
{
    __IO uint32_t *PEVNTODRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));

    PEVNTODRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTODR1) + (0x1CUL * u8EventPort));
    return (uint16_t)(READ_REG32(*PEVNTODRx) & 0xFFFFUL);
}

/**
 * @brief  Set specified Event port output data port pins
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @retval None
 */
void EP_SetPins(uint8_t u8EventPort, uint16_t u16EventPin)
{
    __IO uint32_t *PEVNTODRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));

    PEVNTODRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTODR1) + (0x1CUL * u8EventPort));

    SET_REG32_BIT(*PEVNTODRx, u16EventPin);
}

/**
 * @brief  Reset specified Event port output data port pins
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @retval None
 */
void EP_ResetPins(uint8_t u8EventPort, uint16_t u16EventPin)
{
    __IO uint32_t *PEVNTODRx;

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));

    PEVNTODRx = (__IO uint32_t *)((uint32_t)(&M4_AOS->PEVNTODR1) + (0x1CUL * u8EventPort));

    CLEAR_REG32_BIT(*PEVNTODRx, u16EventPin);
}

/**
 * @brief  Set specified Event port pins direction
 * @param  [in] u8EventPort: EVENT_PORT_x, x can be (1~4) to select the EP peripheral
 * @param  [in] u16EventPin: EVENT_PIN_x, x can be (00~15) to select the EP pin index
 * @param  [in] u32Dir: Pin direction
 *   @arg  EP_DIR_IN
 *   @arg  EP_DIR_OUT
 * @retval None
 */
void EP_SetDir(uint8_t u8EventPort, uint16_t u16EventPin, uint32_t u32Dir)
{
    __IO uint32_t *EPDIRx;                  /* Direction register */

    DDL_ASSERT(IS_EVENT_PORT(u8EventPort));
    DDL_ASSERT(IS_EVENT_PIN(u16EventPin));
    DDL_ASSERT(IS_EP_DIR(u32Dir));

    EPDIRx = (__IO uint32_t *)(EP1_BASE + EP1_DIR_BASE + (0x1CUL * u8EventPort));

    if (EP_DIR_OUT == u32Dir)
    {
        SET_REG32_BIT(*EPDIRx, u16EventPin);
    }
    else
    {
        CLEAR_REG32_BIT(*EPDIRx, u16EventPin);
    }
}

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

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
