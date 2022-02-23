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
/** \file hc_i2s.c
 **
 ** I2S driver API.
 **
 **   - 2019-07-05  lsq    First Version
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_i2s.h"

/**
 ******************************************************************************
 ** \addtogroup AdcGroup
 ******************************************************************************/
//@{

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 *****************************************************************************/
 
/**
 ******************************************************************************
  * @brief  使能I2Sx相关的中断
  * @param  i2sx: M0P_I2S0或者M0P_I2S1
  * @param  i2s_it: 为以下值
  *     @arg I2S_IT_TXE: 发送缓冲区空中断使能位
  *     @arg I2S_IT_RXNEIE:接收缓冲区空中断使能位
  *     @arg I2S_IT_ERRIE: 错误中断使能位
  * @param  NewState: =ENALE 或 DISABLE
  * @retval None
 **
 ******************************************************************************/
void I2S_ConfIt(M0P_I2S_TypeDef *i2sx, uint8_t i2s_it, en_en_state_t NewState)
{
	uint32_t itmark=0;
	itmark=1<<i2s_it;
	if(NewState == ENABLE)               //使能中断
	{
		i2sx->CR |= itmark;
	}
	else if(NewState == DISABLE)         //禁止中断
	{
		i2sx->CR &= ~itmark;
	}
        else
        {
          ;
        }
}

/**
 ******************************************************************************
 ** \brief  I2S左右声道缓冲区DMA数据发送使能配置
 ** 
  * @param  i2sx: M0P_I2S0或者M0P_I2S1
  * @param  i2s_it: 为以下值
  *     @arg I2S_RDMA_EN: 右声道缓冲区DMA数据发送使能位
  *     @arg I2S_LDMA_EN: 左声道缓冲区DMA数据发送使能位
  * @param  NewState: =ENALE 或 DISABLE
 ** \retval none
 **
 ******************************************************************************/
void I2s_ConfDma(M0P_I2S_TypeDef *i2sx, uint8_t rl_dma_en, en_en_state_t NewState)
{
	uint32_t itmark=0;
	itmark = 1<<rl_dma_en;
	if(NewState == ENABLE)            //使能左声道或右声道DMA
	{
		i2sx->CR |= itmark;
	}
	else if(NewState == DISABLE)      //禁止左声道或右声道DMA
	{
		i2sx->CR &= ~itmark;
	}	
        else
        {
          ;
        }
}

/**
 ******************************************************************************
 ** \brief  I2S状态标志位状态获取
 ** 
  * @param  i2sx: M0P_I2S0或者M0P_I2S1
  * @param  i2s_status: 为以下值
  *     @arg I2S_RXNE_L: 左声道接收缓冲区非空标志
	*     @arg I2S_TXE_L: 左声道发送缓冲区空标志
  *     @arg I2S_UDR_L: 左声道下溢标志
  *     @arg I2S_UDR_R: 右声道下溢标志
  *     @arg I2S_OVR_L: 左声道上溢标志
  *     @arg I2S_BSY: 忙标志
  *     @arg I2S_FRE: 帧错误标志
  *     @arg I2S_OVR_R: 右声道上溢标志
  *     @arg I2S_RXNE_R: 右声道接收缓冲区非空标志
	*     @arg I2S_TXE_R: 右声道发送缓冲区空标志
 ** \retval 状态标志位状态   RESET 和 SET
 **
 ******************************************************************************/
en_flag_status_t I2s_GetStatus(M0P_I2S_TypeDef *i2sx, uint8_t i2s_status)
{
	uint16_t itmark=0;
	en_flag_status_t bitstatus;
	itmark=(uint16_t)1<<i2s_status;	
	if(!(i2sx->SR & itmark))
	{
		bitstatus = RESET;
	}
	else
	{
		bitstatus = SET;
	}
	return bitstatus;
}

/**
 ******************************************************************************
 ** \brief  I2S中断状态标志位清零
 ** 
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 * @param  i2s_status: 为以下值
 *     @arg I2S_FLAG_UDF: 下溢中断标志位
 *     @arg I2S_FLAG_OVR: 上溢中断标志位
 *     @arg I2S_FLAG_FRE: 帧错误标志位
 ** \retval none
 **
 ******************************************************************************/
void I2s_ClearITPendingBit(M0P_I2S_TypeDef *i2sx, uint8_t i2s_it_flag)
{
	uint32_t bitstatus;
	bitstatus = 1<<i2s_it_flag;
	i2sx->ICR &= ~bitstatus;
}

/**
 ******************************************************************************
 ** \brief  向左声道数据寄存器DRL写入要发送的数据
 ** 
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 * @param  Data: 要发送的16位数据
 ** \retval none
 **
 ******************************************************************************/
void I2s_SendDataL(M0P_I2S_TypeDef *i2sx, uint16_t Data)
{	
	i2sx->DRL_f.DRL = Data;
}

/**
 ******************************************************************************
 ** \brief  向右声道数据寄存器DRR写入要发送的数据
 ** 
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 * @param  Data: 要发送的16位数据
 ** \retval none
 **
 ******************************************************************************/
void I2s_SendDataR(M0P_I2S_TypeDef *i2sx, uint16_t Data)
{
	i2sx->DRR_f.DRR = Data;
}

/**
 ******************************************************************************
 ** \brief  从左声道数据寄存器DRL读取所接收的数据
 ** 
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 ** \retval 所读取的数据
 **
 ******************************************************************************/
uint16_t I2s_ReceiveDataL(M0P_I2S_TypeDef *i2sx)
{	
	uint16_t data;
	data = (uint16_t)i2sx->DRL;
	return data;
}

/**
 ******************************************************************************
 ** \brief  从右声道数据寄存器DRR读取所接收的数据
 ** 
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 ** \retval 所读取的数据
 **
 ******************************************************************************/
uint16_t I2s_ReceiveDataR(M0P_I2S_TypeDef *i2sx)
{	
	uint16_t data;
	data = (uint16_t)i2sx->DRR;
	return data;
}

/**
 ******************************************************************************
 ** \brief  初始化I2Sx配置
 **  (一)小数分频FRACT=0:
 **  (1)输出主时钟(MCKOE=1)
 **  通道帧宽度位16位，采样率=主频I2SxCLK/[(16*2)*(2*I2SDIV+ODD)*8]
 **  通道帧宽度位32位，采样率=主频I2SxCLK/[(32*2)*(2*I2SDIV+ODD)*4]
 **  (2)禁止主时钟(MCKOE=0)
 **  通道帧宽度位16位，采样率=主频I2SxCLK/[(16*2)*(2*I2SDIV+ODD)]
 **  通道帧宽度位32位，采样率=主频I2SxCLK/[(32*2)*(2*I2SDIV+ODD)]
 **  (二)小数分频FRACT=0:
 **  (1)输出主时钟(MCKOE=1)
 **  通道帧宽度位16位，采样率=主频I2SxCLK/[(16*2)*(2*(I2SDIV+FRACT/64))*8]
 **  通道帧宽度位32位，采样率=主频I2SxCLK/[(32*2)*(2*(I2SDIV+FRACT/64))*4]
 **  (2)输出主时钟(MCKOE=0)
 **  通道帧宽度位16位，采样率=主频I2SxCLK/[(16*2)*(2*(I2SDIV+FRACT/64))]
 **  通道帧宽度位32位，采样率=主频I2SxCLK/[(32*2)*(2*(I2SDIV+FRACT/64))]
 **  I2SDIV、FRACT和I2SODD按照以上的计算公式根据实际情况设置数值
 * @param  i2sx: M0P_I2S0或者M0P_I2S1
 * @param  i2s_conf ：配置参数
 ** \retval 无
 **
 ******************************************************************************/
void I2s_Init(M0P_I2S_TypeDef *i2sx, stc_i2s_config_t *i2s_conf)
{
	i2sx->CFGR_f.CFG     = i2s_conf->i2s_Mode;       //设置模式
	i2sx->CFGR_f.PCMSYNC = i2s_conf->i2s_PcmSync;    //PCM帧同步位， 只有在I2SSTD=3的情况下该位才有意义
	i2sx->CFGR_f.CKSEL   = i2s_conf->i2s_Cksel;      //主模式下I2S始终选择：0：PCLK 1:HCLK
	i2sx->CFGR_f.STD     = i2s_conf->i2s_Std;        //标准选择 0：I2S Philips 1:MAS左对齐 2：LSB右对齐 3：PCM标准
	i2sx->CFGR_f.DATLEN  = i2s_conf->i2s_Datalen;    //要传输的数据长度
	i2sx->CFGR_f.CHIEN   = i2s_conf->i2s_Chlen;      //每个音频通道的位数，0：16位 1：32位
	i2sx->PR_f.MCKOE     = i2s_conf->i2s_Mckoe;      //主时钟MCK输出使能
	i2sx->PR_f.I2SDIV    = i2s_conf->i2s_Div;
	i2sx->PR_f.FRACT     = i2s_conf->i2s_Fract;
	i2sx->PR_f.ODD       = i2s_conf->i2s_Odd;
}

/**
 ******************************************************************************
 ** \brief  使能或者禁止I2Sx
 ** 
 * @param  i2sx:  M0P_I2S0或者M0P_I2S1
 **
 * @param  NewState :EANBLE 或者DISABLE
 ** \retval 无
 **
 ******************************************************************************/
void I2S_Cmd(M0P_I2S_TypeDef *i2sx, en_en_state_t NewState)
{
	i2sx->CFGR_f.E = NewState;
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/









