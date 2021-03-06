/**
  ****************************************************************************************************
  * @file    fm33lg0xx_fl_lptim16.c
  * @author  FMSH Application Team
  * @brief   Src file of LPTIM16 FL Module
  ****************************************************************************************************
  * @attention
  *
  * Copyright (c) [2019] [Fudan Microelectronics]
  * THIS SOFTWARE is licensed under the Mulan PSL v1.
  * can use this software according to the terms and conditions of the Mulan PSL v1.
  * You may obtain a copy of Mulan PSL v1 at:
  * http://license.coscl.org.cn/MulanPSL
  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
  * PURPOSE.
  * See the Mulan PSL v1 for more details.
  *
  ****************************************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl_cmu.h"
#include "fm33lg0xx_fl_rmu.h"
#include "fm33lg0xx_fl_lptim16.h"
#include "fm33_assert.h"

/** @addtogroup FM33LG0xx_FL_Driver_LPTIM16
  * @{
  */

/* Private macros ------------------------------------------------------------*/
/** @addtogroup LPTIM16_FL_Private_Macros
  * @{
  */

#define         IS_LPTIM16_INSTANCE(INSTANCE)                           (((INSTANCE) == LPTIM16))

#define         IS_FL_LPTIM16_CHANNEL(__VALUE__)                        (((__VALUE__) == FL_LPTIM16_CHANNEL_1)||\
                                                                         ((__VALUE__) == FL_LPTIM16_CHANNEL_2))

#define         IS_FL_LPTIM16_CMU_CLK_SOURCE(__VALUE__)                 (((__VALUE__) == FL_CMU_LPTIM16_CLK_SOURCE_RCLF)||\
                                                                         ((__VALUE__) == FL_CMU_LPTIM16_CLK_SOURCE_RCLP)||\
                                                                         ((__VALUE__) == FL_CMU_LPTIM16_CLK_SOURCE_LSCLK)||\
                                                                         ((__VALUE__) == FL_CMU_LPTIM16_CLK_SOURCE_APBCLK))

#define         IS_FL_LPTIM16_CLK_SOURCE(__VALUE__)                     (((__VALUE__) == FL_LPTIM16_CLK_SOURCE_INTERNAL)||\
                                                                         ((__VALUE__) == FL_LPTIM16_CLK_SOURCE_EXTERNAL))

#define         IS_FL_LPTIM16_PSC(__VALUE__)                            (((__VALUE__) == FL_LPTIM16_PSC_DIV1)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV2)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV4)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV8)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV16)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV32)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV64)||\
                                                                         ((__VALUE__) == FL_LPTIM16_PSC_DIV128))

#define         IS_FL_LPTIM16_OPERATION_MODE(__VALUE__)                 (((__VALUE__) == FL_LPTIM16_OPERATION_MODE_NORMAL)||\
                                                                         ((__VALUE__) == FL_LPTIM16_OPERATION_MODE_EXTERNAL_TRIGGER_CNT)||\
                                                                         ((__VALUE__) == FL_LPTIM16_OPERATION_MODE_EXTERNAL_ASYNC_PULSE_CNT)||\
                                                                         ((__VALUE__) == FL_LPTIM16_OPERATION_MODE_TIMEOUT))

#define         IS_FL_LPTIM16_ENCODER_MODE(__VALUE__)                   (((__VALUE__) == FL_LPTIM16_ENCODER_MODE_DISABLE)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ENCODER_MODE_TI1FP1_TI2FP2_CNT)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ENCODER_MODE_TI2FP2_TI1FP1_CNT)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ENCODER_MODE_TI2FP2_CNT_TI1FP1_CNT))

#define         IS_FL_LPTIM16_ETR_TRIGGER_EDGE(__VALUE__)               (((__VALUE__) == FL_LPTIM16_ETR_TRIGGER_EDGE_RISING)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ETR_TRIGGER_EDGE_FALLING)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ETR_TRIGGER_EDGE_BOTH))

#define         IS_FL_LPTIM16_ETR_COUNT_EDGE(__VALUE__)                 (((__VALUE__) == FL_LPTIM16_ETR_COUNT_EDGE_RISING)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ETR_COUNT_EDGE_FALLING))

#define         IS_FL_LPTIM16_ONE_PULSE_MODE(__VALUE__)                 (((__VALUE__) == FL_LPTIM16_ONE_PULSE_MODE_CONTINUOUS)||\
                                                                         ((__VALUE__) == FL_LPTIM16_ONE_PULSE_MODE_SINGLE))

#define         IS_FL_LPTIM16_IC_EDGE(__VALUE__)                        (((__VALUE__) == FL_LPTIM16_IC_EDGE_RISING)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC_EDGE_FALLING)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC_EDGE_BOTH))

#define         IS_FL_LPTIM16_IC_POLARITY(__VALUE__)                    (((__VALUE__) == FL_LPTIM16_IC_POLARITY_NORMAL)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC_POLARITY_INVERT))

#define         IS_FL_LPTIM16_OC_POLARITY(__VALUE__)                    (((__VALUE__) == FL_LPTIM16_OC_POLARITY_NORMAL)||\
                                                                         ((__VALUE__) == FL_LPTIM16_OC_POLARITY_INVERT))

#define         IS_FL_LPTIM16_IC1_CAPTURE_SOURCE(__VALUE__)             (((__VALUE__) == FL_LPTIM16_IC1_CAPTURE_SOURCE_CHANNEL1)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC1_CAPTURE_SOURCE_XTLF)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC1_CAPTURE_SOURCE_RCLP)||\
                                                                         ((__VALUE__) == FL_LPTIM16_IC1_CAPTURE_SOURCE_RCLF))

#define         IS_FL_LPTIM16_TRGO_SOURCE(__VALUE__)                    (((__VALUE__) == FL_LPTIM16_TRGO_ENABLE)||\
                                                                         ((__VALUE__) == FL_LPTIM16_TRGO_UPDATE)||\
                                                                         ((__VALUE__) == FL_LPTIM16_TRGO_OC1_CMP_PULSE)||\
                                                                         ((__VALUE__) == FL_LPTIM16_TRGO_IC1_EVENT)||\
                                                                         ((__VALUE__) == FL_LPTIM16_TRGO_IC2_EVENT))

/**
  * @}
  */

/** @addtogroup LPTIM16_FL_EF_Init
  * @{
  */

/**
  * @brief  ??????LPTIM16 ??????
  * @param  ??????????????????
  * @retval ?????????????????????????????????
  *         -FL_PASS ?????????????????????????????????
  *         -FL_FAIL ???????????????
  */
FL_ErrorStatus FL_LPTIM16_DeInit(LPTIM16_Type *LPTIM16x)
{
    /* ???????????? */
    assert_param(IS_LPTIM16_INSTANCE(LPTIM16x));
    /* ?????????????????? */
    FL_RMU_EnablePeripheralReset(RMU);
    /* ????????????????????? */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_LPTIM16);
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_LPTIM16);
    /* ??????????????????????????????????????? */
    FL_CMU_DisableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_LPTIM16);
    FL_CMU_DisableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_LPTIM16);
    /* ?????????????????? */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}

/**
  * @brief  ????????????????????????LPTIM16????????????????????????????????????????????????
  *
  * @param  LPTIM16x  ??????????????????
  * @param  init ??? @ref FL_LPTIM16_InitTypeDef??????????????????
  *
  * @retval ErrorStatus?????????
  *         -FL_FAIL ????????????????????????
  *         -FL_PASS LPTIM16????????????
  */
FL_ErrorStatus FL_LPTIM16_Init(LPTIM16_Type *LPTIM16x, FL_LPTIM16_InitTypeDef *init)
{
    /* ???????????? */
    assert_param(IS_LPTIM16_INSTANCE(LPTIM16x));
    assert_param(IS_FL_LPTIM16_CMU_CLK_SOURCE(init->clockSource));
    assert_param(IS_FL_LPTIM16_CLK_SOURCE(init->prescalerClockSource));
    assert_param(IS_FL_LPTIM16_PSC(init->prescaler));
    assert_param(IS_FL_LPTIM16_OPERATION_MODE(init->mode));
    assert_param(IS_FL_LPTIM16_ENCODER_MODE(init->encoderMode));
    assert_param(IS_FL_LPTIM16_ONE_PULSE_MODE(init->onePulseMode));
    assert_param(IS_FL_LPTIM16_ETR_TRIGGER_EDGE(init->triggerEdge));
    assert_param(IS_FL_LPTIM16_ETR_COUNT_EDGE(init->countEdge));
    /* ???????????? */
    if(LPTIM16x == LPTIM16)
    {
        /* ?????????????????? */
        FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_LPTIM16);
        /* ?????????????????????????????????????????????????????????????????? */
        if(init->mode != FL_LPTIM16_OPERATION_MODE_EXTERNAL_ASYNC_PULSE_CNT)
        {
            /* ?????????????????? */
            FL_CMU_EnableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_LPTIM16);
            /* ??????????????????????????? */
            FL_CMU_SetLPTIM16ClockSource(init->clockSource);
        }
    }
    else
    {
        return FL_FAIL;
    }
    /* ??????????????????????????? */
    FL_LPTIM16_SetClockSource(LPTIM16x, init->prescalerClockSource);
    /* ?????????????????? */
    FL_LPTIM16_SetPrescaler(LPTIM16x, init->prescaler);
    /* ?????????????????? */
    FL_LPTIM16_WriteAutoReload(LPTIM16x, init->autoReload);
    /* ??????????????????????????? */
    FL_LPTIM16_SetOperationMode(LPTIM16x, init->mode);
    /* ????????????????????? */
    if(init->mode == FL_LPTIM16_OPERATION_MODE_NORMAL)
    {
        FL_LPTIM16_SetEncoderMode(LPTIM16x, init->encoderMode);
    }
    /* ?????????????????? */
    FL_LPTIM16_SetOnePulseMode(LPTIM16x, init->onePulseMode);
    /* ???????????????????????????????????????????????? */
    switch(init->mode)
    {
        case FL_LPTIM16_OPERATION_MODE_NORMAL:
        {
            /* ETR????????????????????????????????????????????????????????????????????????????????????????????????????????? */
            if(init->prescalerClockSource == FL_LPTIM16_CLK_SOURCE_EXTERNAL)
            {
                /* ???????????????????????? */
                FL_LPTIM16_SetETRCountEdge(LPTIM16x, init->countEdge);
                /* ???????????????????????? */
                FL_LPTIM16_EnableETRFilter(LPTIM16x);
            }
        }
        break;
        case FL_LPTIM16_OPERATION_MODE_EXTERNAL_TRIGGER_CNT:
        {
            /* ?????????????????????????????? */
            FL_LPTIM16_SetETRTriggerEdge(LPTIM16x, init->triggerEdge);
        }
        break;
        case FL_LPTIM16_OPERATION_MODE_EXTERNAL_ASYNC_PULSE_CNT:
        {
            /* ???????????????????????? */
            FL_LPTIM16_SetETRCountEdge(LPTIM16x, init->countEdge);
            /* ???????????????????????? */
            FL_LPTIM16_EnableETRFilter(LPTIM16x);
        }
        break;
        case FL_LPTIM16_OPERATION_MODE_TIMEOUT:
        {
            /* ?????????????????????????????? */
            FL_LPTIM16_SetETRTriggerEdge(LPTIM16x, init->triggerEdge);
        }
        break;
        default:
            return FL_FAIL;
    }
    return FL_PASS;
}

/**
  * @brief  ?????? LPTIM16_InitStruct ???????????????
  * @param  init ??? @ref FL_LPTIM16_InitTypeDef??????????????????
  *
  * @retval None
  */
void FL_LPTIM16_StructInit(FL_LPTIM16_InitTypeDef *init)
{
    init->clockSource           = FL_CMU_LPTIM16_CLK_SOURCE_APBCLK;
    init->prescalerClockSource  = FL_LPTIM16_CLK_SOURCE_INTERNAL;
    init->prescaler             = FL_LPTIM16_PSC_DIV1;
    init->autoReload            = 0;
    init->mode                  = FL_LPTIM16_OPERATION_MODE_NORMAL;
    init->countEdge             = FL_LPTIM16_ETR_COUNT_EDGE_RISING;
    init->triggerEdge           = FL_LPTIM16_ETR_TRIGGER_EDGE_RISING;
    init->encoderMode           = FL_LPTIM16_ENCODER_MODE_DISABLE;
    init->onePulseMode          = FL_LPTIM16_ONE_PULSE_MODE_CONTINUOUS;
}

/**
  * @brief  ??????LPTIM16???????????????????????????
  *
  * @param  LPTIM16x  ??????????????????
  * @param  ic_init ??? @ref FL_LPTIM16_IC_InitTypeDef??????????????????
  * @param  Channel LPTIM16????????????
  *
  * @retval ErrorStatus?????????
  *         -FL_FAIL ????????????????????????
  *         -FL_PASS LPTIM16????????????
  */
FL_ErrorStatus FL_LPTIM16_IC_Init(LPTIM16_Type *LPTIM16x, uint32_t Channel, FL_LPTIM16_IC_InitTypeDef *ic_init)
{
    /* ???????????? */
    assert_param(IS_LPTIM16_INSTANCE(LPTIM16x));
    assert_param(IS_FL_LPTIM16_CHANNEL(Channel));
    assert_param(IS_FL_LPTIM16_IC_EDGE(ic_init->ICEdge));
    assert_param(IS_FL_LPTIM16_IC_POLARITY(ic_init->ICInputPolarity));
    assert_param(IS_FL_LPTIM16_IC1_CAPTURE_SOURCE(ic_init->channel1CaptureSource));
    /* ??????1????????? & ????????? */
    if(Channel == FL_LPTIM16_CHANNEL_1)
    {
        FL_LPTIM16_IC_WriteChannel1Prescaler(LPTIM16x, ic_init->channel1Prescaler);
        FL_LPTIM16_IC_SetChannel1CaptureSource(LPTIM16x, ic_init->channel1CaptureSource);
    }
    if(ic_init->ICInputDigitalFilter == FL_DISABLE)
    {
        FL_LPTIM16_DisableDigitalFilter(LPTIM16, Channel);
    }
    else
    {
        FL_LPTIM16_EnableDigitalFilter(LPTIM16, Channel);
    }
    /* ?????????????????? */
    FL_LPTIM16_IC_SetInputPolarity(LPTIM16x, ic_init->ICInputPolarity, Channel);
    /* ?????????????????? */
    FL_LPTIM16_IC_SetCaptureEdge(LPTIM16x, ic_init->ICEdge, Channel);
    /* ???????????????????????? */
    FL_LPTIM16_SetChannelMode(LPTIM16x, FL_LPTIM16_CHANNEL_MODE_INPUT, Channel);
    return FL_PASS;
}

/**
  * @brief  ?????? LPTIM16_IC_InitStruct ???????????????
  * @param  ic_init ??? @ref FL_LPTIM16_IC_InitTypeDef??????????????????
  *
  * @retval None
  */
void FL_LPTIM16_IC_StructInit(FL_LPTIM16_IC_InitTypeDef *ic_init)
{
    ic_init->ICInputPolarity          = FL_LPTIM16_IC_POLARITY_NORMAL;
    ic_init->ICInputDigitalFilter     = FL_DISABLE;
    ic_init->ICEdge                   = FL_LPTIM16_IC_EDGE_RISING;
    ic_init->channel1Prescaler        = 1 - 1;
    ic_init->channel1CaptureSource    = FL_LPTIM16_IC1_CAPTURE_SOURCE_CHANNEL1;
}

/**
  * @brief  ????????????????????????LPTIM16????????????????????????????????????
  *
  * @param  LPTIM16x  ??????????????????
  * @param  oc_init ??? @ref FL_LPTIM16_OC_InitTypeDef??????????????????
  * @param  Channel LPTIM16????????????
  *
  * @retval ErrorStatus?????????
  *         -FL_FAIL ????????????????????????
  *         -FL_PASS LPTIM16????????????
  */
FL_ErrorStatus FL_LPTIM16_OC_Init(LPTIM16_Type *LPTIM16x, uint32_t Channel, FL_LPTIM16_OC_InitTypeDef *oc_init)
{
    /* ???????????? */
    assert_param(IS_LPTIM16_INSTANCE(LPTIM16x));
    assert_param(IS_FL_LPTIM16_CHANNEL(Channel));
    assert_param(IS_FL_LPTIM16_OC_POLARITY(oc_init->OCPolarity));
    /* ?????????????????? */
    FL_LPTIM16_OC_SetPolarity(LPTIM16x, oc_init->OCPolarity, Channel);
    /* ??????????????? */
    switch(Channel)
    {
        case FL_LPTIM16_CHANNEL_1:
            FL_LPTIM16_WriteCompareCH1(LPTIM16x, oc_init->compareValue);
            break;
        case FL_LPTIM16_CHANNEL_2:
            FL_LPTIM16_WriteCompareCH2(LPTIM16x, oc_init->compareValue);
            break;
        default :
            return FL_FAIL;
    }
    /* ???????????????????????? */
    FL_LPTIM16_SetChannelMode(LPTIM16x, FL_LPTIM16_CHANNEL_MODE_OUTPUT, Channel);
    return FL_PASS;
}

/**
  * @brief  ?????? LPTIM16_OC_InitStruct ???????????????
  * @param  oc_init ??? @ref FL_LPTIM16_OC_InitTypeDef??????????????????
  *
  * @retval None
  */
void FL_LPTIM16_OC_StructInit(FL_LPTIM16_OC_InitTypeDef *oc_init)
{
    oc_init->compareValue = 0;
    oc_init->OCPolarity   = FL_LPTIM16_OC_POLARITY_NORMAL;
}

/**
  * @}
  */

/**
  * @}
  */
/*************************************************************END OF FILE************************************************************/

