/**
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_lcd.c
  * @author  FMSH Application Team
  * @brief   Src file of LCD FL Module
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
#include "fm33lg0xx_fl_lcd.h"
#include "fm33_assert.h"
/** @addtogroup FM33LG0xx_FL_Driver_LCD
  * @{
  */

/** @addtogroup Private_Macros
  * @{
  */

#define         IS_FL_LCD_INSTANCE(INTENCE)             ((INTENCE) == LCD)

#define         IS_FL_LCD_BIASCURRENT(__VALUE__)        (((__VALUE__) == (FL_LCD_BIAS_CURRENT_VERYHIGH))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_CURRENT_HIGH))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_CURRENT_MEDIUM))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_CURRENT_LOW)))

#define         IS_FL_LCD_ENMODE(__VALUE__)             ((__VALUE__) == (FL_LCD_DRIVER_MODE_INNER_RESISTER)||\
                                                        ((__VALUE__) == (FL_LCD_DRIVER_MODE_OUTER_CAPACITY)))

#define         IS_FL_LCD_BIASVOLTAGE(__VALUE__)        (((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL0))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL1))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL2))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL3))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL4))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL5))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL6))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL7))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL8))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL9))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL10))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL11))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL12))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL13))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL14))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_VOLTAGE_LEVEL15)))

#define         IS_FL_LCD_BIASMD(__VALUE__)             (((__VALUE__) == (FL_LCD_BIAS_MODE_4BIAS))||\
                                                        ((__VALUE__) == (FL_LCD_BIAS_MODE_3BIAS)))

#define         IS_FL_LCD_BWFT(__VALUE__)               (((__VALUE__) == (FL_LCD_WAVEFORM_TYPEA))||\
                                                        ((__VALUE__) == (FL_LCD_WAVEFORM_TYPEB)))

#define         IS_FL_LCD_LMUX(__VALUE__)               (((__VALUE__) == (FL_LCD_COM_NUM_4COM))||\
                                                        ((__VALUE__) == (FL_LCD_COM_NUM_6COM))||\
                                                        ((__VALUE__) == (FL_LCD_COM_NUM_8COM)))


/**
  * @}
  */
/** @addtogroup LCD_FL_EF_Init
  * @{
  */

/**
  * @brief  复位对应LCD寄存器.
  * @param  LCDx
  * @retval ErrorStatus枚举值:
  *         -FL_PASS 外设寄存器值恢复复位值
  *         -FL_FAIL 未成功执行
  */
static uint32_t FL_LCD_DisplayFreq(uint32_t freq)
{
    uint32_t displayFreq = 32;
    if((freq > 0) && (freq <= 100))
    {
        switch(FL_LCD_GetCOMNumber(LCD))
        {
            case FL_LCD_COM_NUM_4COM:
                displayFreq = (uint32_t)(32768.0 / (float)(4 * freq * 2) + 0.5);
                break;
            case FL_LCD_COM_NUM_6COM:
                displayFreq = (uint32_t)(32768.0 / (float)(6 * freq * 2) + 0.5);
                break;
            case FL_LCD_COM_NUM_8COM:
                displayFreq = (uint32_t)(32768.0 / (float)(8 * freq * 2) + 0.5);
                break;
        }
    }
    return displayFreq & 0xff;
}

/**
  * @brief  复位对应LCD寄存器.
  * @param  LCDx
  * @retval ErrorStatus枚举值:
  *         -FL_PASS 外设寄存器值恢复复位值
  *         -FL_FAIL 未成功执行
  */
static uint32_t FL_LCD_FlickTime(uint32_t time)
{
    float stepTime = 0;
    switch(FL_LCD_GetCOMNumber(LCD))
    {
        case FL_LCD_COM_NUM_4COM:
            stepTime = (float)(4 * FL_LCD_ReadDisplayFrequency(LCD) * 2 * 16 * 1000) / 32768.0;
            break;
        case FL_LCD_COM_NUM_6COM:
            stepTime = (float)(6 * FL_LCD_ReadDisplayFrequency(LCD) * 2 * 16 * 1000) / 32768.0;
            break;
        case FL_LCD_COM_NUM_8COM:
            stepTime = (float)(8 * FL_LCD_ReadDisplayFrequency(LCD) * 2 * 16 * 1000) / 32768.0;
            break;
    }
    return (uint32_t)(time / stepTime + 0.5);
}

/**
  * @brief  复位对应LCD寄存器.

  * @param  LCDx
  * @retval ErrorStatus枚举值:
  *         -FL_PASS 外设寄存器值恢复复位值
  *         -FL_FAIL 未成功执行
  */
FL_ErrorStatus FL_LCD_DeInit(LCD_Type *LCDx)
{
    assert_param(IS_FL_LCD_INSTANCE(LCDx));
    /* 使能外设复位 */
    FL_RMU_EnablePeripheralReset(RMU);
    /* 复位LCD外设寄存器 */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_LCD);
    FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_LCD);
    /* 关闭外设总线始时钟和工作时钟 */
    FL_CMU_DisableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_LCD);
    /* 锁定外设复位 */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}

/**
  * @brief  根据 LCD_InitStruct 的配置信息初始化对应外设入口地址的寄存器值.
  * @param  LCDx LCDx
  * @param  LCD_InitStruct 指向一个 @ref FL_LCD_InitTypeDef 结构体
  *         其中包含了外设的相关配置信息.
  * @retval ErrorStatus枚举值
  *         -FL_FAIL 配置过程发生错误
  *         -FL_PASS LCD配置成功
  */
FL_ErrorStatus FL_LCD_Init(LCD_Type *LCDx, FL_LCD_InitTypeDef *initStruct)
{
    assert_param(IS_FL_LCD_INSTANCE(LCDx));
    assert_param(IS_FL_LCD_BIASCURRENT(initStruct->biasCurrent));
    assert_param(IS_FL_LCD_ENMODE(initStruct->mode));
    assert_param(IS_FL_LCD_BIASVOLTAGE(initStruct->biasVoltage));
    assert_param(IS_FL_LCD_BIASMD(initStruct->biasMode));
    assert_param(IS_FL_LCD_BWFT(initStruct->waveform));
    assert_param(IS_FL_LCD_LMUX(initStruct->COMxNum));
    /* 外设总线始时钟 */
    FL_CMU_EnableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_LCD);
    /* 电流源电流控制 */
    FL_LCD_SetBiasCurrent(LCD, initStruct->biasCurrent);
    /* LCD驱动模式 */
    FL_LCD_SetDriverMode(LCD, initStruct->mode);
    /* 偏执电压设置 */
    FL_LCD_SetBiasVoltage(LCD, initStruct->biasVoltage);
    /* 偏执模式选择 */
    FL_LCD_SetBiasMode(LCD, initStruct->biasMode);
    /* 驱动波形设置 */
    FL_LCD_SetWaveform(LCD, initStruct->waveform);
    /* COMx口选择 */
    FL_LCD_SetCOMNumber(LCD, initStruct->COMxNum);
    /* 设置工作频率 */
    FL_LCD_WriteDisplayFrequency(LCD, FL_LCD_DisplayFreq(initStruct->displayFreq));
    /* 设置闪烁频率 */
    FL_LCD_WriteDisplayOnTime(LCD, FL_LCD_FlickTime(initStruct->flickOnTime));
    FL_LCD_WriteDisplayOffTime(LCD, FL_LCD_FlickTime(initStruct->flickOffTime));
    /* 使能外设 */
    FL_LCD_Enable(LCD);
    return FL_PASS;
}

/**
  * @brief  设置 LCD_InitStruct 为默认配置
  * @param  LCD_InitStruct 指向需要将值设置为默认配置的结构体 @ref FL_LCD_InitTypeDef 结构体
  *
  * @retval None
  */
void FL_LCD_StructInit(FL_LCD_InitTypeDef *initStruct)
{
    initStruct->biasCurrent         =   FL_LCD_BIAS_CURRENT_HIGH;
    initStruct->mode                =   FL_LCD_DRIVER_MODE_INNER_RESISTER;
    initStruct->biasVoltage         =   FL_LCD_BIAS_VOLTAGE_LEVEL10;
    initStruct->biasMode            =   FL_LCD_BIAS_MODE_3BIAS;
    initStruct->waveform            =   FL_LCD_WAVEFORM_TYPEA;
    initStruct->COMxNum             =   FL_LCD_COM_NUM_6COM;
    initStruct->displayFreq         =   42;
    initStruct->flickOnTime         =   0;
    initStruct->flickOffTime        =   0;
}

/**
  * @brief  设置 LCD 4COM显示字端
  * @param  display 指向显示信息的缓存区域，可直接指向LCD的DATAx寄存器。
  * @param  com 待显示字段所在COM。范围0-4。
  * @param  seg 待显示字段所在SEG。范围0-43。
  * @param  state 待显示字段状态。0，熄灭；否则，点亮。
  *
  * @retval None
  */
void FL_LCD_4COMDisplay(uint32_t *display, uint8_t com, uint8_t seg, uint8_t state)
{
    uint32_t temp;
    if(state != 0)
    {
        state = 1;
    }
    if(seg > 31)
    {
        temp = (seg - 32) + com * 12;
        com = 4 + temp / 32;
        seg = temp % 32;
    }
    MODIFY_REG(display[com], 0x1 << seg, state << seg);
}

/**
  * @brief  设置 LCD 6COM显示字端
  * @param  display 指向显示信息的缓存区域，可直接指向LCD的DATAx寄存器。
  * @param  com 待显示字段所在COM。范围0-6。
  * @param  seg 待显示字段所在SEG。范围0-41。
  * @param  state 待显示字段状态。0，熄灭；否则，点亮。
  *
  * @retval None
  */
void FL_LCD_6COMDisplay(uint32_t *display, uint8_t com, uint8_t seg, uint8_t state)
{
    uint32_t temp;
    if(state != 0)
    {
        state = 1;
    }
    if(seg > 31)
    {
        temp = (seg - 32) + com * 10;
        com = 6 + temp / 32;
        seg = temp % 32;
    }
    MODIFY_REG(display[com], 0x1 << seg, state << seg);
}

/**
  * @brief  设置 LCD 8COM显示字端
  * @param  display 指向显示信息的缓存区域，可直接指向LCD的DATAx寄存器。
  * @param  com 待显示字段所在COM。范围0-8。
  * @param  seg 待显示字段所在SEG。范围0-39。
  * @param  state 待显示字段状态。0，熄灭；否则，点亮。
  *
  * @retval None
  */
void FL_LCD_8COMDisplay(uint32_t *display, uint8_t com, uint8_t seg, uint8_t state)
{
    uint32_t temp;
    if(state != 0)
    {
        state = 1;
    }
    if(seg > 31)
    {
        temp = (seg - 32) + com * 8;
        com = 8 + temp / 32;
        seg = temp % 32;
    }
    MODIFY_REG(display[com], 0x1 << seg, state << seg);
}

/**
  * @}
  */
/**
  * @}
  */
/**************************Py_Code_Generator Version: 0.1-0.5-0.1 @ 2019-10-17**************************/
/*************************(C) COPYRIGHT Fudan Microelectronics **** END OF FILE*************************/
