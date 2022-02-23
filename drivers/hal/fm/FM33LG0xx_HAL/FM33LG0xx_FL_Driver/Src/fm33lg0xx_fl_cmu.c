/**
  ****************************************************************************************************
  * @file    fm33lg0xx_fl_cmu.c
  * @author  FMSH Application Team
  * @brief   Src file of CMU FL Module
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
#include "system_fm33lg0xx.h"
#include "fm33lg0xx_fl_cmu.h"
#include "fm33_assert.h"

/** @addtogroup FM33LG0xx_FL_Driver_CMU
  * @{
  */

/** @addtogroup CMU_FL_OPEREAT
  * @{
  */

///**
//  * @brief  设置系统工作使用时钟。
//  * @param  系统工作时钟源及频率。
//  * @note   函数中用到了XTHF_VALUE 宏，这个宏应该被定义为外部晶振的输入频率值。
//  * @note   其他时钟源可由应用程序实现，如PLL_RCHF_40M, PLL_XTHF_64M
//  * @note   PLL倍频频率范围为32M-64M
//  * @note   对功耗由要求的应用，注意关闭未使用到的空闲时钟源
//  *
//  * @retval 系统时钟设置状态。
//  *
//  */
//FL_ErrorStatus FL_CMU_SetSystemClock(FL_SystemClock systemClock)
//{
//    FL_ErrorStatus errorStatus = FL_PASS;
//
//    /* <= 24M: 0wait; > 24 and <= 48: 1wait; > 48M: 2wait */
//    FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_2CYCLE);
//    /* CDIF接口访问频率（APBCLK）建议不要超过8MHz */
//    FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV8);
//
//    switch (systemClock)
//    {
//        case FL_SYSTEM_CLOCK_RCHF_8M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF8M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_8MHZ);
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCHF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_RCHF_16M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF16M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_16MHZ);
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCHF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV2);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_RCHF_24M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF16M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_24MHZ);
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCHF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV4);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_PLL_RCHF_32M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF16M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_8MHZ);
//
//            FL_CMU_PLL_SetClockSource(FL_CMU_PLL_CLK_SOURCE_RCHF);
//            FL_CMU_PLL_SetPrescaler(FL_CMU_PLL_PSC_DIV8);
//            FL_CMU_PLL_WriteMultiplier(32 - 1);
//            FL_CMU_PLL_SetOutputMultiplier(FL_CMU_PLL_OUTPUT_X1);
//            FL_CMU_PLL_Enable();
//            while (!FL_CMU_IsActiveFlag_PLLReady());
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_PLL);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV4);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_1CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_PLL_RCHF_48M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF16M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_8MHZ);
//
//            FL_CMU_PLL_SetClockSource(FL_CMU_PLL_CLK_SOURCE_RCHF);
//            FL_CMU_PLL_SetPrescaler(FL_CMU_PLL_PSC_DIV8);
//            FL_CMU_PLL_WriteMultiplier(48 - 1);
//            FL_CMU_PLL_SetOutputMultiplier(FL_CMU_PLL_OUTPUT_X1);
//            FL_CMU_PLL_Enable();
//            while (!FL_CMU_IsActiveFlag_PLLReady());
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_PLL);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV8);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_1CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_PLL_RCHF_64M:
//            FL_CMU_RCHF_Enable();
//            FL_CMU_RCHF_WriteTrimValue(RCHF16M_TRIM);
//            FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_8MHZ);
//
//            FL_CMU_PLL_SetClockSource(FL_CMU_PLL_CLK_SOURCE_RCHF);
//            FL_CMU_PLL_SetPrescaler(FL_CMU_PLL_PSC_DIV8);
//            FL_CMU_PLL_WriteMultiplier(64 - 1);
//            FL_CMU_PLL_SetOutputMultiplier(FL_CMU_PLL_OUTPUT_X1);
//            FL_CMU_PLL_Enable();
//            while (!FL_CMU_IsActiveFlag_PLLReady());
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_PLL);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV8);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_2CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_RCLP:
//            FL_CMU_RCLP_Enable();
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCLP);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_RCLF:
//            FL_CMU_RCLF_Enable();
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCLF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_XTLF:
//            FL_CDIF_EnableCPUToVAO(CDIF);
//            FL_CDIF_EnableVAOToCPU(CDIF);
//
//            FL_VAO_XTLF_Enable(VAO);
//            FL_DelayMs(10);
//            while (!FL_VAO_GetXTLFFailOutput(VAO));
//
//            FL_CDIF_DisableCPUToVAO(CDIF);
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_XTLF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//
//        case FL_SYSTEM_CLOCK_XTHF:
//            FL_CMU_XTHF_Enable();
//            FL_DelayMs(10);
//            while (!FL_CMU_IsActiveFlag_XTHFFailOutput())
//
//            FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_XTHF);
//
//            FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV1);
//            FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);
//            break;
//    }
//
//    return errorStatus;
//}

/**
  * @brief  获取系统当前工作时钟SYSCLK。
  * @param  None
  * @note   函数中用到了XTHF_VALUE 宏，这个宏应该被定义为外部晶振的输入频率值。
  *
  * @retval 系统时钟频率 (Hz)。
  *
  */
uint32_t FL_CMU_GetSystemClockFreq(void)
{
    uint32_t frequency = 0;
    /* 获取系统时钟源 */
    switch(FL_CMU_GetSystemClockSource())
    {
        /* 系统时钟源为内部RCHF */
        case FL_CMU_SYSTEM_CLK_SOURCE_RCHF:
            /* 内部RCHF默认为8MHz ,可以配置为16或24M */
            frequency = FL_CMU_GetRCHFClockFreq();
            break;
        /* 系统时钟源为XTHF */
        case FL_CMU_SYSTEM_CLK_SOURCE_XTHF:
            frequency = XTHF_VALUE;
            break;
        /* 系统时钟源为PLL */
        case FL_CMU_SYSTEM_CLK_SOURCE_PLL:
            frequency = FL_CMU_GetPLLClockFreq();
            break;
        /* 系统时钟源为内部RCLF */
        case FL_CMU_SYSTEM_CLK_SOURCE_RCLF:
            /* 根据RC4M的分频配置得出系统时钟 */
            frequency = FL_CMU_GetRCLFClockFreq();
            break;
        /* 系统时钟源为XTLF */
        case FL_CMU_SYSTEM_CLK_SOURCE_XTLF:
            /* 根据外部晶振的频率得出系统时钟 */
            frequency = XTLF_VALUE;
            break;
        /* 系统时钟源为RCLP */
        case FL_CMU_SYSTEM_CLK_SOURCE_RCLP:
            frequency = 32768;
            break;
        default:
            frequency = FL_CMU_GetRCHFClockFreq();
            break;
    }
    return frequency;
}
/**
  * @brief  获取 AHB 总线时钟频率。
  *
  * @param  SYSCLK_Frequency  系统主时钟频率SYSCLK
  *
  * @retval AHB 总线时钟频率(Hz)
  *
  */
uint32_t FL_CMU_GetAHBClockFreq(void)
{
    uint32_t frequency = 0;
    /* 获取AHB分频系数，AHB源自系统主时钟 */
    switch(FL_CMU_GetAHBPrescaler())
    {
        case FL_CMU_AHBCLK_PSC_DIV1:
            frequency = FL_CMU_GetSystemClockFreq();
            break;
        case FL_CMU_AHBCLK_PSC_DIV2:
            frequency = FL_CMU_GetSystemClockFreq() / 2;
            break;
        case FL_CMU_AHBCLK_PSC_DIV4:
            frequency = FL_CMU_GetSystemClockFreq() / 4;
            break;
        case FL_CMU_AHBCLK_PSC_DIV8:
            frequency = FL_CMU_GetSystemClockFreq() / 8;
            break;
        case FL_CMU_AHBCLK_PSC_DIV16:
            frequency = FL_CMU_GetSystemClockFreq() / 16;
            break;
        default:
            frequency = FL_CMU_GetSystemClockFreq();
            break;
    }
    return frequency;
}

/**
  * @brief  获取当前系统的APB总线时钟
  * @param  APB_Frequency APB总线的时钟频率
  *
  * @retval APB clock frequency (in Hz)
  *
  */
uint32_t FL_CMU_GetAPBClockFreq(void)
{
    uint32_t frequency = 0;
    /* 获取APB1分频系数，APB源自AHB */
    switch(FL_CMU_GetAPBPrescaler())
    {
        case FL_CMU_APBCLK_PSC_DIV1:
            frequency = FL_CMU_GetAHBClockFreq();
            break;
        case FL_CMU_APBCLK_PSC_DIV2:
            frequency = FL_CMU_GetAHBClockFreq() / 2;
            break;
        case FL_CMU_APBCLK_PSC_DIV4:
            frequency = FL_CMU_GetAHBClockFreq() / 4;
            break;
        case FL_CMU_APBCLK_PSC_DIV8:
            frequency = FL_CMU_GetAHBClockFreq() / 8;
            break;
        case FL_CMU_APBCLK_PSC_DIV16:
            frequency = FL_CMU_GetAHBClockFreq() / 16;
            break;
        default:
            frequency = FL_CMU_GetAHBClockFreq();
            break;
    }
    return frequency;
}
/**
  * @brief  获取RCLF输出时钟频率
  * @param  None
  *
  * @retval 返回RCLF输出时钟频率(Hz)
  *
  */
uint32_t FL_CMU_GetRCLFClockFreq(void)
{
    uint32_t frequency = 0;
    switch(FL_CMU_RCLF_GetPrescaler())
    {
        case FL_CMU_RCLF_PSC_DIV1:
            frequency = 614400;
            break;
        case FL_CMU_RCLF_PSC_DIV4:
            frequency = 153600;
            break;
        case FL_CMU_RCLF_PSC_DIV8:
            frequency = 76800;
            break;
        case FL_CMU_RCLF_PSC_DIV16:
            frequency = 38400;
            break;
        default:
            frequency = 614400;
            break;
    }
    return frequency;
}
/**
  * @brief  获取RCHF输出时钟频率
  * @param  None
  *
  * @retval 返回RCHF输出时钟频率(Hz)
  *
  */
uint32_t FL_CMU_GetRCHFClockFreq(void)
{
    uint32_t frequency = 0;
    switch(FL_CMU_RCHF_GetFrequency())
    {
        case FL_CMU_RCHF_FREQUENCY_8MHZ:
            frequency = 8000000;
            break;
        case FL_CMU_RCHF_FREQUENCY_16MHZ:
            frequency = 16000000;
            break;
        case FL_CMU_RCHF_FREQUENCY_24MHZ:
            frequency = 24000000;
            break;
        case FL_CMU_RCHF_FREQUENCY_32MHZ:
            frequency = 32000000;
            break;
        default:
            frequency = 8000000;
            break;
    }
    return frequency;
}
/**
  * @brief  获取PLL输出时钟频率
  * @param  None
  *
  * @retval 返回PLL输出时钟频率(Hz)
  *
  */
uint32_t FL_CMU_GetPLLClockFreq(void)
{
    uint32_t frequency = 0;
    uint32_t multiplier = 0;
    /* 获取PLL时钟源 */
    switch(FL_CMU_PLL_GetClockSource())
    {
        case FL_CMU_PLL_CLK_SOURCE_RCHF:
            /* 获取RCHF配置主频 */
            frequency = FL_CMU_GetRCHFClockFreq();
            break;
        case FL_CMU_PLL_CLK_SOURCE_XTHF:
            frequency = XTHF_VALUE;
            break;
        default:
            frequency = FL_CMU_GetRCHFClockFreq();
            break;
    }
    /* 获取PLL时钟分频系数 */
    switch(FL_CMU_PLL_GetPrescaler())
    {
        case FL_CMU_PLL_PSC_DIV1:
            break;
        case FL_CMU_PLL_PSC_DIV2:
            frequency /= 2;
            break;
        case FL_CMU_PLL_PSC_DIV4:
            frequency /= 4;
            break;
        case FL_CMU_PLL_PSC_DIV8:
            frequency /= 8;
            break;
        case FL_CMU_PLL_PSC_DIV12:
            frequency /= 12;
            break;
        case FL_CMU_PLL_PSC_DIV16:
            frequency /= 16;
            break;
        case FL_CMU_PLL_PSC_DIV24:
            frequency /= 24;
            break;
        case FL_CMU_PLL_PSC_DIV32:
            frequency /= 32;
            break;
        default:
            break;
    }
    multiplier = FL_CMU_PLL_ReadMultiplier() + 1;
    frequency *= multiplier;
    return frequency;
}

/**
  * @}
  */
/**
  * @}
  */
/*************************************************************END OF FILE************************************************************/

