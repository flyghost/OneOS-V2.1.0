/******************************************************************************
* Copyright (C) 2019, Huada Semiconductor Co.,Ltd All rights reserved.
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
/** \file hc_i2s.h
 **
 ** Header file for i2s Converter functions
 ** @link I2S Group Some description @endlink
 **
 **   - 2019-07-05   lsq   First Version
 **
 ******************************************************************************/
#ifndef __HC_I2S_H__
#define __HC_I2S_H__
/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "hc_ddl.h"
 
/**
 ******************************************************************************
 ** \brief 以下宏定义对应寄存器的相应位
 ******************************************************************************/  
#define I2S_IT_TXE      ((uint8_t)7) // the TXEIE of I2Sx_CR
#define I2S_IT_RXNEIE   ((uint8_t)6) // the RXNEI of I2Sx_CR
#define I2S_IT_ERRIE    ((uint8_t)5) // the ERRIE of I2Sx_CR
#define I2S_RDMA_EN     ((uint8_t)1) // the RDMA of I2Sx_CR
#define I2S_LDMA_EN     ((uint8_t)0) // the LDMA of I2Sx_CR

#define I2S_RXNE_L      ((uint8_t)0) // the RXNE_L of I2Sx_SR
#define I2S_TXE_L       ((uint8_t)1) // the TXE_L of I2Sx_SR
#define I2S_UDR_L       ((uint8_t)2) // the UDR_L of I2Sx_SR
#define I2S_UDR_R       ((uint8_t)3) // the UDR_R of I2Sx_SR
#define I2S_OVR_L       ((uint8_t)6) // the OVR_L of I2Sx_SR
#define I2S_BSY         ((uint8_t)7) // the BSY of I2Sx_SR
#define I2S_FRE         ((uint8_t)8) // the FRE of I2Sx_SR
#define I2S_OVR_R       ((uint8_t)13)// the OVR_R of I2Sx_SR
#define I2S_RXNE_R      ((uint8_t)14)// the RXNE_R of I2Sx_SR
#define I2S_TXE_R       ((uint8_t)15)// the TXE_R of I2Sx_SR

#define I2S_FLAG_UDF    ((uint8_t)3) // the UDF of I2Sx_ICR
#define I2S_FLAG_OVR    ((uint8_t)6) // the OVR of I2Sx_ICR
#define I2S_FLAG_FRE    ((uint8_t)8) // the FRE of I2Sx_ICR

/**
 ******************************************************************************
 ** \brief 音频采样率
 ******************************************************************************/ 
#define I2S_AudioFreq_192k                (uint32_t)192000u // 192khz
#define I2S_AudioFreq_96k                 (uint32_t)96000u  // 96khz
#define I2S_AudioFreq_48k                 (uint32_t)48000u  // 48khz
#define I2S_AudioFreq_44k                 (uint32_t)44100u  // 44.1khz
#define I2S_AudioFreq_32k                 (uint32_t)32000u  // 32khz
#define I2S_AudioFreq_22k                 (uint32_t)22050u  // 22khz
#define I2S_AudioFreq_16k                 (uint32_t)16000u  // 16khz
#define I2S_AudioFreq_11k                 (uint32_t)11025u  // 11.025khz
#define I2S_AudioFreq_8k                  (uint32_t)8000u   // 8khz

/**
 ******************************************************************************
 ** \brief 使能或者禁止指令
 ******************************************************************************/ 
typedef enum 
{
	DISABLE  = 0,         //禁止
	ENABLE   = 1          //使能
}en_en_state_t;

/**
 ******************************************************************************
 ** \brief 状态标志位
 ******************************************************************************/
typedef enum 
{
	RESET   = 0, 
	SET     = !RESET
}en_flag_status_t;

 /**
 ******************************************************************************
 ** \brief I2S 功能通道选择设置
 ******************************************************************************/ 
typedef enum 
{
	I2S0     = 0u,  // I2S通道0
	I2S1     = 1u,  // I2S通道1
}en_i2s_channel_t;

/**
 ******************************************************************************
 ** \brief I2S 功能使能设置
 ******************************************************************************/ 
typedef enum 
{
	I2sEnable     = 1u,  // I2S模块使能
	I2sDisable    = 0u,  // I2S模块禁止
}en_i2s_en_t;

/**
 ******************************************************************************
 ** \brief I2S 模式配置
 ******************************************************************************/ 
typedef enum 
{
	I2sSlaveSend   = 0u,  //从模式发送
    I2sSlaveRec    = 1u,  //从模式接收
	I2sMasterSend  = 2u,  //主模式发送
	I2sMasterRec   = 3u,  //主模式接收
}en_spi_mode_t;

/**
 ******************************************************************************
 ** \brief I2S PCMSYNC同步帧标志
 ******************************************************************************/ 
typedef enum 
{
	I2sPcmsyncShort = 0u,      //短帧同步
	I2sPcmsyncLong  = 1u       //长帧同步
}en_i2s_pcmsync_t;

/**
 ******************************************************************************
 ** \brief I2S 输入时钟选择
 ******************************************************************************/
typedef enum 
{
	I2sPclk = 0u,              //主模式下I2S时钟：PCLLK
	I2sHclk = 1u               //主模式下I2S时钟：HCLLK
}en_i2s_cksel_t;

/**
 ******************************************************************************
 ** \brief I2S标准选择
 ******************************************************************************/
typedef enum 
{
	i2sstdPhilips = 0u,       //I2S Philips标准
	i2sstdMSBL    = 1u,       //MSB对齐标准(左对齐)
	i2sstdLSBR    = 2u,       //LSB对齐标准(右对齐)
	i2sstdPCM     = 3u        //PCM标准
}en_i2s_i3sstd_t;

/**
 ******************************************************************************
 ** \brief I2S 要传输的数据长度
 ******************************************************************************/
typedef enum 
{
	i2sDatlen16Bit = 0u,      //16位数据长度
	i2sDatlen24Bit = 1u,      //24位数据长度
	i2sDatlen32Bit = 2u       //32位数据长度
}en_i2s_datlen_t;

/**
 ******************************************************************************
 ** \brief I2S 通道数据长度
 ******************************************************************************/
typedef enum 
{
	i2sChlen16Bit = 0u,       //通道数据长度：16位
	i2sChlen32Bit = 1u        //通道数据长度：32位
}en_i2s_chlen_t;

/**
 *******************************************************************************
 ** \brief I2S 配置结构体
 ******************************************************************************/
typedef struct 
{
    en_spi_mode_t               i2s_Mode;        //I2S 模式配置位
    en_i2s_pcmsync_t            i2s_PcmSync;     //I2S 帧同步
    en_i2s_cksel_t              i2s_Cksel;       //I2S 主模式下始终选择：0：PCLK 1:HCLK
    en_i2s_i3sstd_t             i2s_Std;         //I2S 标准选择 0：I2S Philips 1:MAS左对齐 2：LSB右对齐 3：PCM标准
    en_i2s_datlen_t             i2s_Datalen;     //I2S 要传输的数据长度
    en_i2s_chlen_t              i2s_Chlen;       //I2S 每个音频通道的位数，0：16位 1：32位，
    en_en_state_t       		i2s_Mckoe;       //I2S 主时钟MCK输出使能
    uint32_t                    i2s_AudioFreq;   //I2S audio frequecy
    uint8_t                     i2s_Div;         //I2S PR寄存器的DIV，位数8位：线性预分频器
    uint8_t                     i2s_Odd;         //I2S PR寄存器的ODD，位数1位：预分频器的奇数因子
    uint8_t                     i2s_Fract;       //I2S PR寄存器的FRACT小数分频，位数6位
}stc_i2s_config_t;

/*******************************************************************************
 ** \brief I2Sx相关函数声明
 ******************************************************************************/
extern void I2S_ConfIt(M0P_I2S_TypeDef *i2sx, uint8_t i2s_it, en_en_state_t NewState);
extern void I2s_ConfDma(M0P_I2S_TypeDef *i2sx, uint8_t rl_dma_en, en_en_state_t NewState);
extern en_flag_status_t I2s_GetStatus(M0P_I2S_TypeDef *i2sx, uint8_t i2s_status);
extern void I2s_ClearITPendingBit(M0P_I2S_TypeDef *i2sx, uint8_t i2s_it_flag);
extern void I2s_SendDataL(M0P_I2S_TypeDef *i2sx, uint16_t Data);
extern void I2s_SendDataR(M0P_I2S_TypeDef *i2sx, uint16_t Data);
extern uint16_t I2s_ReceiveDataL(M0P_I2S_TypeDef *i2sx);
extern uint16_t I2s_ReceiveDataR(M0P_I2S_TypeDef *i2sx);
extern void I2s_Init(M0P_I2S_TypeDef *i2sx, stc_i2s_config_t *i2s_conf);
extern void I2S_Cmd(M0P_I2S_TypeDef *i2sx, en_en_state_t NewState);

#endif //__HC_I2S_H__

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
