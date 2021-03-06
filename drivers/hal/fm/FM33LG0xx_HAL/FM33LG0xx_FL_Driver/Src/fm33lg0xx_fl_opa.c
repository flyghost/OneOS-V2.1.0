/**
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_opa.c
  * @author  FMSH Application Team
  * @brief   Src file of OPA FL Module
  *******************************************************************************************************
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
  *******************************************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl_cmu.h"
#include "fm33lg0xx_fl_rmu.h"
#include "fm33lg0xx_fl_opa.h"
#include "fm33_assert.h"
/** @addtogroup FM33LG0xx_FL_Driver_OPA
  * @{
  */

/* Private macros ------------------------------------------------------------*/
/** @addtogroup OPA_FL_Private_Macros
  * @{
  */


#define         IS_OPA_ALL_INSTANCE(INTENCE)               (((INTENCE) == OPA))


#define         IS_FL_OPA_INP_CHANNAL(__VALUE__)           (((__VALUE__) == FL_OPA_INP_SOURCE_INP1)||\
                                                            ((__VALUE__) == FL_OPA_INP_SOURCE_INP2)||\
                                                            ((__VALUE__) == FL_OPA_INP_SOURCE_DAC))


#define         IS_FL_OPA_INN_CHANNAL(__VALUE__)           (((__VALUE__) == FL_OPA_INN_SOURCE_INN1)||\
                                                            ((__VALUE__) == FL_OPA_INN_SOURCE_INN2))

#define         IS_FL_OPA_MODE(__VALUE__)                  (((__VALUE__) == FL_OPA_MODE_STANDALONE)||\
                                                            ((__VALUE__) == FL_OPA_MODE_PGA)||\
                                                            ((__VALUE__) == FL_OPA_MODE_BUFFER))


#define         IS_FL_OPA_NEGTIVE_TO_PIN(__VALUE__)        (((__VALUE__) == FL_DISABLE)||\
                                                            ((__VALUE__) == FL_ENABLE))

#define         IS_FL_OPA_LOW_POWER_MODE(__VALUE__)        (((__VALUE__) == FL_DISABLE)||\
                                                            ((__VALUE__) == FL_ENABLE))

#define         IS_FL_OPA_GAIN(__VALUE__)                  (((__VALUE__) == FL_OPA_GAIN_NOINVERT_X2)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_NOINVERT_X4)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_NOINVERT_X8)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_NOINVERT_X16)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_INVERT_X1)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_INVERT_X3)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_INVERT_X7)||\
                                                            ((__VALUE__) == FL_OPA_GAIN_INVERT_X15))

#define         IS_FL_OPA_PGA_MODESELECT(__VALUE__)         (((__VALUE__) == FL_OPA_PGA_MODE_FB_TO_NEGATIVE)||\
                                                            ((__VALUE__) == FL_OPA_PGA_MODE_FB_TO_GND))


/**
  * @}
  */
/**
  * @brief  ??????OPA ??????????????????????????????
  * @param  ??????????????????
  * @retval ?????????????????????????????????
  *         -FL_PASS ?????????????????????????????????
  *         -FL_FAIL ???????????????
  */
FL_ErrorStatus FL_OPA_DeInit(OPA_Type *OPAx)
{
    /* ??????????????????????????? */
    assert_param(IS_OPA_ALL_INSTANCE(OPAx));
    /* ?????????????????? */
    FL_RMU_EnablePeripheralReset(RMU);
    /* ????????????????????? */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_OPA);
    FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_OPA);
    /* ?????????????????????????????????????????? */
    FL_CMU_DisableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_OPA);
    /* ?????????????????? */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}

/**
  * @brief  ?????? OPA_InitStruct.
  * @param  OPAx
  * @param  OPA_InitStruct ???????????? @ref FL_OPA_InitTypeDef ?????????
  *         ??????????????????????????????????????????.
  * @retval ErrorStatus?????????
  *         -FL_FAIL ????????????????????????
  *         -FL_PASS OPA????????????
  */
FL_ErrorStatus FL_OPA_Init(OPA_Type *OPAx, FL_OPA_InitTypeDef *initStruct)
{
    FL_ErrorStatus status = FL_PASS;
    /* ?????????????????? */
    assert_param(IS_OPA_ALL_INSTANCE(OPAx));
    assert_param(IS_FL_OPA_INP_CHANNAL(initStruct->INP));
    assert_param(IS_FL_OPA_INN_CHANNAL(initStruct->INN));
    assert_param(IS_FL_OPA_MODE(initStruct->mode));
    assert_param(IS_FL_OPA_NEGTIVE_TO_PIN(initStruct->negtiveToPin));
    assert_param(IS_FL_OPA_LOW_POWER_MODE(initStruct->lowPowermode));
    assert_param(IS_FL_OPA_GAIN(initStruct->gain));
    assert_param(IS_FL_OPA_PGA_MODESELECT(initStruct->PGAModeSelect));
    /*??????????????????*/
    FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_OPA);
    /*????????????*/
    FL_OPA_SetMode(OPAx, initStruct->mode);
    /*??????????????????*/
    FL_OPA_SetINNSource(OPAx, initStruct->INN);
    /*??????????????????*/
    FL_OPA_SetINPSource(OPAx, initStruct->INP);
    /*?????????????????????*/
    if(initStruct->lowPowermode == FL_ENABLE)
    {
        FL_OPA_EnableLowPowerMode(OPAx);
    }
    if(initStruct->mode == FL_OPA_MODE_PGA)
    {
        /*??????PGA??????*/
        FL_OPA_PGA_SetGain(OPAx, initStruct->gain);
        if(initStruct->negtiveToPin == FL_ENABLE)
        {
            /*??????PGA??????????????????????????????PIN*/
            FL_OPA_PGA_EnableINNConnectToPin(OPAx);
        }
    }
    /*PGA????????????*/
    FL_OPA_PGA_SetMode(OPAx, initStruct->PGAModeSelect);
    return status;
}
/**
  * @brief  FL_OPA_InitTypeDef ???????????????
  * @param  FL_OPA_InitTypeDef ??????????????????????????????????????????????????? @ref FL_OPA_InitTypeDef structure
  *         ?????????
  * @retval None
  */

void FL_OPA_StructInit(FL_OPA_InitTypeDef *initStruct)
{
    initStruct->INP              = FL_OPA_INP_SOURCE_INP1;
    initStruct->INN              = FL_OPA_INN_SOURCE_INN1;
    initStruct->mode             = FL_OPA_MODE_STANDALONE;
    initStruct->negtiveToPin     = FL_DISABLE;
    initStruct->gain             = FL_OPA_GAIN_NOINVERT_X2;
    initStruct->lowPowermode     = FL_DISABLE;
    initStruct->PGAModeSelect    = FL_OPA_PGA_MODE_FB_TO_GND;
}
/**
  *@}
  */
/**
  *@}
  */
/*************************************************************END OF FILE************************************************************/

