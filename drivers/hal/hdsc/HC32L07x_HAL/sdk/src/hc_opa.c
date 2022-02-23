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
/** \file hc_opa.c
 **
 ** opa driver API.
 ** @link opa Group Some description @endlink
 **
 **   - 2019-04-11       First Version
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_opa.h"

/**
 ******************************************************************************
 ** \addtogroup OPAGroup
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
	** \brief   OPA 输出通道设置
	**
	** \param   Opax:       指定使用哪个OPA，共3个OPA，取值:Opa0,Opa1,Opa2
	**          OutChs:     OPA输出通道配置，可配置多个通道使能
	** \retval  无
	**
******************************************************************************/
void Opa_OutChannelConfig(en_opa_t Opax, stc_opa_oenx_config_t OutChs)
{
	if ((Opax==Opa0)||(Opax==Opa1)||(Opax==Opa2))
	{
		SetBit((uint32_t)(&(M0P_OPA->CR0)), OPA_CHANNEL_OUT_Pos(Opax, Opa_Ch_Oen1), OutChs.opa_ch1);
		SetBit((uint32_t)(&(M0P_OPA->CR0)), OPA_CHANNEL_OUT_Pos(Opax, Opa_Ch_Oen2), OutChs.opa_ch2);
		SetBit((uint32_t)(&(M0P_OPA->CR0)), OPA_CHANNEL_OUT_Pos(Opax, Opa_Ch_Oen3), OutChs.opa_ch3);
		SetBit((uint32_t)(&(M0P_OPA->CR0)), OPA_CHANNEL_OUT_Pos(Opax, Opa_Ch_Oen4), OutChs.opa_ch4);
	}
}

/**
******************************************************************************
	** \brief  OPAx 使能或禁止
	**
	** \param  Opax:       共5个OPA，取值:Opa0,Opa1,Opa2,Opa3,Opa4
	**  	   NewStatus : 配置Opx使能或禁止 TRUE or FALSE	
	** \retval 无
	**
******************************************************************************/
void Opa_Cmd(en_opa_t Opax, boolean_t NewStatus)
{
	SetBit((uint32_t)(&(M0P_OPA->CR1)), Opax, NewStatus);
	if(Opax == Opa3)       /*当OPA3使能时，DAC0不能使用OPA3作为输出缓存*/
	{
		M0P_OPA->CR1 &= (uint32_t)~(1<<Opa_Dac0Buff);
	}else if(Opax == Opa4) /*当OPA4使能时，DAC1不能使用OPA4作为输出缓存*/
	{
		M0P_OPA->CR1 &= (uint32_t)~(1<<Opa_Dac1Buff);
	}else
    {
    
    }
}

/**
******************************************************************************
	** \brief  DAC缓存 使能或禁止
	**
	** \param  Buffx:      Opa_Adc0Buff or Opa_Adc1Buff
	**  	   NewStatus : 配置Buffx使能或禁止 TRUE or FALSE	
	** \retval 无
	**
******************************************************************************/
void Opa_DacBufCmd(en_opa_dac_buff_t Buffx, boolean_t NewStatus)
{
	SetBit((uint32_t)(&(M0P_OPA->CR1)), Buffx, NewStatus);
	if (Buffx == Opa_Dac0Buff)
    {
		M0P_OPA->CR1 &= (uint32_t)~(1<<Opa3);  /*DAC0使用OP3单位增加缓存使能，则OPA3使能不可用*/
	}else
	{
		M0P_OPA->CR1 &= (uint32_t)~(1<<Opa4);  /*DAC1使用OP4单位增加缓存使能，则OPA4使能不可用*/
	}
}

/**
******************************************************************************
	** \brief  配置OPA教零使能
	**
	** \param  Opax:       指定使用哪个OPA，共5个OPA，取值:Opa0,Opa1,Opa2,Opa3,Opa4
	**  	   NewStatus : 配置Opx教零使能或禁止 TRUE or FALSE	
	** \retval 无
	**
******************************************************************************/
void Opa_CalCmd(en_opa_t Opax, boolean_t NewStatus)
{
	SetBit((uint32_t)(&(M0P_OPA->CR1)), OPA_AZEN_Pos(Opax), NewStatus);
}



/**
******************************************************************************
	** \brief  自动教零参数配置
	**
	** \param  InitZero :  
	** \retval 无
	**
******************************************************************************/
void Opa_CalConfig(stc_opa_zconfig_t* InitZero)
{
	M0P_OPA->CR_f.CLK_SEL = InitZero->enClksrc;
	M0P_OPA->CR_f.CLK_SW_SET = InitZero->bClk_sw_set;
	M0P_OPA->CR_f.AZ_PULSE = InitZero->bAz_pulse;
	M0P_OPA->CR_f.ADCTR_EN   = InitZero->bAdctr_en; 
}

/**
******************************************************************************
	** \brief  软件触发自动教零
	**
	** \param  InitZero :  
	** \retval 无
	**
******************************************************************************/
void Opa_CalSwTrig(void)
{
	M0P_OPA->CR_f.TRIGGER = TRUE;
}

