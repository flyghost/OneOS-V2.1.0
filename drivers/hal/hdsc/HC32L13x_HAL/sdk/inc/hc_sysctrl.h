/*******************************************************************************
* Copyright (C) 2018, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer: 
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file sysctrl.h
 **
 ** Headerfile for SYSCTRL functions
 ** @link SYSCTRL Group Some description @endlink 
 **
 ** History:
 **   - 2018-04-15   Lux     First Version
 **
 ******************************************************************************/

#ifndef __HC_SYSCTRL_H__
#define __HC_SYSCTRL_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_ddl.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup SysCtrlGroup (SYSCTRL)
  **
 ******************************************************************************/
//@{

/**
 *******************************************************************************
 ** function prototypes.
 ******************************************************************************/

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
 
/**
 *******************************************************************************
 ** \brief ϵͳʱ������Դ���Ͷ���
 ** \note
 ******************************************************************************/
typedef enum en_sysctrl_clk_source
{
    SysctrlClkRCH  = 0u,               ///< �ڲ�����ʱ��
    SysctrlClkXTH  = 1u,               ///< �ⲿ����ʱ��
    SysctrlClkRCL  = 2u,               ///< �ڲ�����ʱ��
    SysctrlClkXTL  = 3u,               ///< �ⲿ����ʱ��
    SysctrlClkPLL  = 4u,               ///< PLLʱ��
}en_sysctrl_clk_source_t;

/**
 *******************************************************************************
 ** \brief RCHƵ��ֵö�����Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_rch_freq
{
    SysctrlRchFreq4MHz     = 4u,               ///< 4MHz
    SysctrlRchFreq8MHz     = 3u,               ///< 8MHz
    SysctrlRchFreq16MHz    = 2u,               ///< 16MHz
    SysctrlRchFreq22_12MHz = 1u,               ///< 22.12MHz
    SysctrlRchFreq24MHz    = 0u,               ///< 24MHz
}en_sysctrl_rch_freq_t;

/**
 *******************************************************************************
 ** \brief XTAL�����������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_xtal_driver
{
    SysctrlXtalDriver0 = 0u,                ///< ������������
    SysctrlXtalDriver1 = 1u,                ///< ����������
    SysctrlXtalDriver2 = 2u,                ///< һ����������
    SysctrlXtalDriver3 = 3u,                ///< ��ǿ��������
}en_sysctrl_xtal_driver_t;

/**
 *******************************************************************************
 ** \brief XTHƵ��ֵ��Χѡ�����Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_xth_freq
{
    SysctrlXthFreq4_6MHz   = 0u,                ///< 4~6MHz
    SysctrlXthFreq6_12MHz  = 1u,                ///< 6~12MHz
    SysctrlXthFreq12_20MHz = 2u,                ///< 12~20MHz
    SysctrlXthFreq20_32MHz = 3u,                ///< 20~32MHz
}en_sysctrl_xth_freq_t;

/**
 *******************************************************************************
 ** \brief XTHʱ���ȶ����������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_xth_cycle
{
    SysctrlXthStableCycle256   = 0u,              ///< 256 ��������
    SysctrlXthStableCycle1024  = 1u,              ///< 1024 ��������
    SysctrlXthStableCycle4096  = 2u,              ///< 4096 ��������
    SysctrlXthStableCycle16384 = 3u,              ///< 16384 ��������
}en_sysctrl_xth_cycle_t;

/**
 *******************************************************************************
 ** \brief RCLƵ��ֵö�����Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_rcl_freq
{
    SysctrlRclFreq32768    = 0x11u,               ///< 32.768KHz
    SysctrlRclFreq38400    = 0x10u,               ///< 38.4KHz
}en_sysctrl_rcl_freq_t;

/**
 *******************************************************************************
 ** \brief RCLʱ���ȶ����������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_rcl_cycle
{
    SysctrlRclStableCycle4   = 0u,                  ///< 4 ��������
    SysctrlRclStableCycle16  = 1u,                  ///< 16 ��������
    SysctrlRclStableCycle64  = 2u,                  ///< 64 ��������
    SysctrlRclStableCycle256 = 3u,                  ///< 256 ��������
}en_sysctrl_rcl_cycle_t;

/**
 *******************************************************************************
 ** \brief XTLʱ���ȶ����������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_xtl_cycle
{
    SysctrlXtlStableCycle256   = 0u,                  ///< 256 ��������
    SysctrlXtlStableCycle1024  = 1u,                  ///< 1024 ��������
    SysctrlXtlStableCycle4096  = 2u,                  ///< 4096 ��������
    SysctrlXtlStableCycle16384 = 3u,                  ///< 16384 ��������
}en_sysctrl_xtl_cycle_t;

/**
 *******************************************************************************
 ** \brief XTL�������ö�����Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_xtl_amp
{
    SysctrlXtlAmp0 = 0u,                ///< ��С���
    SysctrlXtlAmp1 = 1u,                ///< С���
    SysctrlXtlAmp2 = 2u,                ///< һ�����
    SysctrlXtlAmp3 = 3u,                ///< ������
}en_sysctrl_xtl_amp_t;

/**
 *******************************************************************************
 ** \brief PLLʱ���ȶ����������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_pll_cycle
{
    SysctrlPllStableCycle128   = 0u,                  ///< 128��������
    SysctrlPllStableCycle256   = 1u,                  ///< 256��������
    SysctrlPllStableCycle512   = 2u,                  ///< 512��������
    SysctrlPllStableCycle1024  = 3u,                  ///< 1024��������
    SysctrlPllStableCycle2048  = 4u,                  ///< 2048��������
    SysctrlPllStableCycle4096  = 5u,                  ///< 4096��������
    SysctrlPllStableCycle8192  = 6u,                  ///< 8192��������
    SysctrlPllStableCycle16384 = 7u,                  ///< 16384��������    
}en_sysctrl_pll_cycle_t;

/**
 *******************************************************************************
 ** \brief PLL����Ƶ�ʷ�Χ���Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_pll_infreq
{
    SysctrlPllInFreq4_6MHz   = 0u,                  ///< 4~16MHz
    SysctrlPllInFreq6_12MHz  = 1u,                  ///< 6~12MHz
    SysctrlPllInFreq12_20MHz = 2u,                  ///< 12~20MHz
    SysctrlPllInFreq20_24MHz = 3u,                  ///< 20~24MHz  
}en_sysctrl_pll_infreq_t;

/**
 *******************************************************************************
 ** \brief PLL���Ƶ�ʷ�Χ���Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_pll_outfreq
{
    SysctrlPllOutFreq8_12MHz  = 0u,                 ///< 8~12MHz
    SysctrlPllOutFreq12_18MHz = 1u,                 ///< 12~18MHz
    SysctrlPllOutFreq18_24MHz = 2u,                 ///< 18~24MHz
    SysctrlPllOutFreq24_36MHz = 3u,                 ///< 24~36MHz
    SysctrlPllOutFreq36_48MHz = 4u,                 ///< 36~48MHz
}en_sysctrl_pll_outfreq_t;

/**
 *******************************************************************************
 ** \brief PLL����ʱ��Դ���Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_pll_clksource
{
    SysctrlPllXthXtal   = 0u,                  ///< XTH���������ʱ��
    SysctrlPllXthIn     = 2u,                  ///< XTH�Ӷ˿������ʱ��
    SysctrlPllRch       = 3u,                  ///< RCHʱ��
}en_sysctrl_pll_clksource_t;

/**
 *******************************************************************************
 ** \brief PLL����ʱ��Դ���Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_pll_mul
{
    SysctrlPllMul2  = 2u,                   ///< 2��Ƶ
    SysctrlPllMul3  = 3u,                   ///< 3��Ƶ
    SysctrlPllMul4  = 4u,                   ///< 4��Ƶ
    SysctrlPllMul5  = 5u,                   ///< 5��Ƶ
    SysctrlPllMul6  = 6u,                   ///< 6��Ƶ
    SysctrlPllMul7  = 7u,                   ///< 7��Ƶ
    SysctrlPllMul8  = 8u,                   ///< 8��Ƶ
    SysctrlPllMul9  = 9u,                   ///< 9��Ƶ
    SysctrlPllMul10 = 10u,                  ///< 10��Ƶ
    SysctrlPllMul11 = 11u,                  ///< 11��Ƶ
    SysctrlPllMul12 = 12u,                  ///< 12��Ƶ
}en_sysctrl_pll_mul_t;

/**
 *******************************************************************************
 ** \brief HCLKʱ�ӷ�Ƶϵ�����Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_hclk_div
{
    SysctrlHclkDiv1   = 0u,              ///< SystemClk
    SysctrlHclkDiv2   = 1u,              ///< SystemClk/2
    SysctrlHclkDiv4   = 2u,              ///< SystemClk/4
    SysctrlHclkDiv8   = 3u,              ///< SystemClk/8
    SysctrlHclkDiv16  = 4u,              ///< SystemClk/16
    SysctrlHclkDiv32  = 5u,              ///< SystemClk/32
    SysctrlHclkDiv64  = 6u,              ///< SystemClk/64
    SysctrlHclkDiv128 = 7u,              ///< SystemClk/128
}en_sysctrl_hclk_div_t;

/**
 *******************************************************************************
 ** \brief PCLK��Ƶϵ��
 ******************************************************************************/
typedef enum en_sysctrl_pclk_div
{
    SysctrlPclkDiv1 = 0u,                ///< HCLK
    SysctrlPclkDiv2 = 1u,                ///< HCLK/2
    SysctrlPclkDiv4 = 2u,                ///< HCLK/4
    SysctrlPclkDiv8 = 3u,                ///< HCLK/8
}en_sysctrl_pclk_div_t;

/**
 *******************************************************************************
 ** \brief RTC����ʱ�Ӳ���ʱ��Ƶ���������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_rtc_adjust
{
    SysctrlRTC4MHz  = 0u,                ///< 4MHz
    SysctrlRTC6MHz  = 1u,                ///< 6MHz
    SysctrlRTC8MHz  = 2u,                ///< 8MHz
    SysctrlRTC12MHz = 3u,                ///< 12MHz
    SysctrlRTC16MHz = 4u,                ///< 16MHz
    SysctrlRTC20MHz = 5u,                ///< 20MHz
    SysctrlRTC24MHz = 6u,                ///< 24MHz
    SysctrlRTC32MHz = 7u,                ///< 32MHz
}en_sysctrl_rtc_adjust_t;

/**
 *******************************************************************************
 ** \brief ϵͳ����ģ�����������������Ͷ���
 ******************************************************************************/
typedef enum en_sysctrl_func
{
    SysctrlEXTHEn           = 1u,                    ///< ʹ���ⲿ����ʱ�Ӵ�������������
    SysctrlEXTLEn           = 2u,                    ///< ʹ���ⲿ������ʱ�Ӵ�������������
    SysctrlXTLAlwaysOnEn    = 3u,                    ///< ʹ�ܺ�XTL_ENֻ����λ
    SysctrlClkFuncRTCLpmEn  = 5u,                    ///< ʹ��RTC�͹���ģʽ
    SysctrlCMLockUpEn       = 6u,                    ///< ʹ�ܺ�CPUִ����Чָ��ḴλMCU
    SysctrlSWDUseIOEn       = 8u,                    ///< SWD�˿���ΪIO����
}en_sysctrl_func_t;

/**
 *******************************************************************************
 ** \brief ����ʱ���ſؿ�������ö��
 ******************************************************************************/
typedef enum en_sysctrl_peripheral_gate
{
    SysctrlPeripheralUart0      = 0u,       ///< ����0
    SysctrlPeripheralUart1      = 1u,       ///< ����1
    SysctrlPeripheralLpUart0    = 2u,       ///< �͹��Ĵ���0
    SysctrlPeripheralLpUart1    = 3u,       ///< �͹��Ĵ���1
    SysctrlPeripheralI2c0       = 4u,       ///< I2C0
    SysctrlPeripheralI2c1       = 5u,       ///< I2C1
    SysctrlPeripheralSpi0       = 6u,       ///< SPI0
    SysctrlPeripheralSpi1       = 7u,       ///< SPI1
    SysctrlPeripheralBaseTim    = 8u,       ///< ������ʱ��TIM0/1/2
    SysctrlPeripheralLpTim      = 9u,       ///< �͹��Ķ�ʱ��
    SysctrlPeripheralAdvTim     = 10u,      ///< �߼���ʱ��TIM4/5/6
    SysctrlPeripheralTim3       = 11u,      ///< ��ʱ��3
    SysctrlPeripheralOpa        = 13u,      ///< OPA
    SysctrlPeripheralPca        = 14u,      ///< �ɱ�̼�������
    SysctrlPeripheralWdt        = 15u,      ///< ���Ź�
    SysctrlPeripheralAdcBgr     = 16u,      ///< ADC&BGR
    SysctrlPeripheralVcLvd      = 17u,      ///< VC��LVD
    SysctrlPeripheralRng        = 18u,      ///< RNG
    SysctrlPeripheralPcnt       = 19u,      ///< PCNT
    SysctrlPeripheralRtc        = 20u,      ///< RTC
    SysctrlPeripheralTrim       = 21u,      ///< ʱ��У׼
    SysctrlPeripheralLcd        = 22u,      ///< LCD
    SysctrlPeripheralTick       = 24u,      ///< ϵͳ��ʱ��
    SysctrlPeripheralSwd        = 25u,      ///< SWD
    SysctrlPeripheralCrc        = 26u,      ///< CRC
    SysctrlPeripheralAes        = 27u,      ///< AES
    SysctrlPeripheralGpio       = 28u,      ///< GPIO
    SysctrlPeripheralDma        = 29u,      ///< DMA
    SysctrlPeripheralDiv        = 30u,      ///< ������
    SysctrlPeripheralFlash      = 31u,      ///< Flash
}en_sysctrl_peripheral_gate_t;

/**
 *******************************************************************************
 ** \brief ʱ�ӳ�ʼ�����ýṹ�嶨��
 ******************************************************************************/
typedef struct
{
    en_sysctrl_clk_source_t  enClkSrc;       ///< ʱ��Դѡ��
    en_sysctrl_hclk_div_t    enHClkDiv;      ///< HCLK��Ƶϵ��
    en_sysctrl_pclk_div_t    enPClkDiv;      ///< PCLK��Ƶϵ��
}stc_sysctrl_clk_cfg_t;

/**
 *******************************************************************************
 ** \brief ʱ�ӳ�ʼ�����ýṹ�嶨��
 ******************************************************************************/
typedef struct
{
    en_sysctrl_pll_infreq_t    enInFreq;        ///< PLL����ʱ��Ƶ�ʷ�Χѡ��
    en_sysctrl_pll_outfreq_t   enOutFreq;       ///< PLL���ʱ��Ƶ�ʷ�Χѡ��
    en_sysctrl_pll_clksource_t enPllClkSrc;     ///< PLL����ʱ��Դѡ��
    en_sysctrl_pll_mul_t       enPllMul;        ///< PLL��Ƶϵ��ѡ��
}stc_sysctrl_pll_cfg_t;

/******************************************************************************
 * Global variable declarations ('extern', definition in C source)
 ******************************************************************************/

/******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
///< ϵͳʱ�ӳ�ʼ��API:�����ϵ��ϵͳ����֮ǰ����Ƶ������ʱ�ӽ��г�ʼ���� 
///< ע��1��ʹ�øó�ʼ������ǰ��Ҫ����ϵͳ��������������Ŀ���ڲ�ʱ��Դ��TRIMֵ���ⲿʱ��Դ��Ƶ�ʷ�Χ��
///< ע��2��XTH��XTL��Ƶ�ʷ�Χ�趨����Ҫ�����ⲿ���������
///< ע��3��������Ĭ�Ϻ궨�壺SYSTEM_XTH=32MHz,SYSTEM_XTL=32768Hz,��ʹ�������ⲿ���񣬱����޸��������궨���ֵ��
en_result_t Sysctrl_ClkInit(stc_sysctrl_clk_cfg_t *pstcCfg);

///< ϵͳʱ��ȥ��ʼ��API:�ָ�Ϊ�ϵ�Ĭ��״̬->PCLK=HCLK=SystemClk=RCH4MHz
en_result_t Sysctrl_ClkDeInit(void);

///< ϵͳʱ��ģ��Ļ�����������
///< ע�⣺ʹ����Ҫʹ�õ�ʱ��Դ֮ǰ��������������Ŀ���ڲ�ʱ��Դ��TRIMֵ���ⲿʱ��Դ��Ƶ�ʷ�Χ
en_result_t Sysctrl_ClkSourceEnable(en_sysctrl_clk_source_t enSource, boolean_t bFlag);

///<�ⲿ�����������ã�ϵͳ��ʼ��Sysctrl_ClkInit()֮�󣬿ɸ�����Ҫ�����ⲿ���������������ʱ�ӳ�ʼ��Sysctrl_ClkInit()Ĭ��Ϊ���ֵ;
en_result_t Sysctrl_XTHDriverCfg(en_sysctrl_xtal_driver_t enDriver);
en_result_t Sysctrl_XTLDriverCfg(en_sysctrl_xtl_amp_t enAmp, en_sysctrl_xtal_driver_t enDriver);

///<ʱ���ȶ���������:ϵͳ��ʼ��Sysctrl_ClkInit()֮�󣬿ɸ�����Ҫ����ʱ�ӿ�������ȶ�֮�䣬Ĭ��Ϊ���ֵ;
en_result_t Sysctrl_SetXTHStableTime(en_sysctrl_xth_cycle_t enCycle);
en_result_t Sysctrl_SetRCLStableTime(en_sysctrl_rcl_cycle_t enCycle);
en_result_t Sysctrl_SetXTLStableTime(en_sysctrl_xtl_cycle_t enCycle);
en_result_t Sysctrl_SetPLLStableTime(en_sysctrl_pll_cycle_t enCycle);

///<ϵͳʱ��Դ�л�������ϵͳʱ�ӣ������Ҫ��ϵͳʱ�ӳ�ʼ��Sysctrl_ClkInit()֮���л���Ƶʱ��Դ����ʹ�øú�����
///< ʱ���л�ǰ�󣬱������Ŀ��Ƶ��ֵ����Flash���ȴ����ڣ������ò�������Ϊ0��1��2��
///< ע��!!!����HCLK����24MHzʱ��FLASH�ȴ����ڲ����������Ϊ1,����������п��ܲ���δ֪����
en_result_t Sysctrl_SysClkSwitch(en_sysctrl_clk_source_t enSource);

///< ʱ��ԴƵ���趨������ϵͳ������������ò�ͬʱ��Դ��Ƶ��ֵ;
///< ʱ��Ƶ������ǰ���������Ŀ��Ƶ��ֵ����Flash���ȴ����ڣ������ò�������Ϊ0��1��2��
///< ����XTL��ʱ�����ⲿ����������������á�
en_result_t Sysctrl_SetRCHTrim(en_sysctrl_rch_freq_t enRCHFreq);
en_result_t Sysctrl_SetRCLTrim(en_sysctrl_rcl_freq_t enRCLFreq);
en_result_t Sysctrl_SetXTHFreq(en_sysctrl_xth_freq_t enXTHFreq);
en_result_t Sysctrl_SetPLLFreq(stc_sysctrl_pll_cfg_t *pstcPLLCfg);

///< ʱ�ӷ�Ƶ����:����ϵͳ�������������HCLK��PCLK�ķ���ֵ;
en_result_t Sysctrl_SetHCLKDiv(en_sysctrl_hclk_div_t enHCLKDiv);
en_result_t Sysctrl_SetPCLKDiv(en_sysctrl_pclk_div_t enPCLKDiv);

///< ʱ��Ƶ�ʻ�ȡ������ϵͳ��Ҫ����ȡ��ǰHCLK��PCLK��Ƶ��ֵ
uint32_t Sysctrl_GetHClkFreq(void);
uint32_t Sysctrl_GetPClkFreq(void);

///< �����ſؿ���/״̬��ȡ�����ڿ�������ģ���ʹ�ܣ�ʹ�ø�ģ��Ĺ���֮ǰ������ʹ�ܸ�ģ����ſ�ʱ�ӣ�
en_result_t Sysctrl_SetPeripheralGate(en_sysctrl_peripheral_gate_t enPeripheral, boolean_t bFlag);
boolean_t   Sysctrl_GetPeripheralGate(en_sysctrl_peripheral_gate_t enPeripheral);

///< ϵͳ�������ã�������������ϵͳ������⹦�ܣ�
en_result_t Sysctrl_SetFunc(en_sysctrl_func_t enFunc, boolean_t bFlag);

///< RTC����ʱ�Ӳ���:��������RTC����ʱ���µ�Ƶ�ʲ���
en_result_t Sysctrl_SetRTCAdjustClkFreq(en_sysctrl_rtc_adjust_t enRtcAdj);

//@} // Sysctrl Group

#ifdef __cplusplus
#endif

#endif /* __HC_SYSCTRL_H__ */
/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


