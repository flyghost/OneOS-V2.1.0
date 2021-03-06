/**
  ******************************************************************************
  * @file    fm33g0xx_anac.c
  * @author  FM33g0xx Application Team
  * @version V0.3.02G
  * @date    01-21-2019
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of....:
  *
*/

/* Includes ------------------------------------------------------------------*/
#include "fm33g0xx_anac.h" 


/** @addtogroup fm33g0xx_StdPeriph_Driver
  * @{
  */

/** @defgroup ANAC 
  * @brief ANAC driver modules
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/



/********************************
PDR下电复位电压配置函数
功能:下电复位电压配置
输入：配置下电复位电压（bit2:1）
输出:无
********************************/
void ANAC_PDRCON_PDRCFG_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->PDRCON;
	tmpreg &= ~(ANAC_PDRCON_PDRCFG_Msk);
	tmpreg |= (SetValue & ANAC_PDRCON_PDRCFG_Msk);
	ANAC->PDRCON = tmpreg;
}
/********************************
读PDR下电复位寄存器位函数
功能:读PDR下电复位电压值寄存器位
输入：无
输出:PDR寄存器电压值
********************************/
uint32_t ANAC_PDRCON_PDRCFG_Get(void)
{
	return (ANAC->PDRCON & ANAC_PDRCON_PDRCFG_Msk);
}

/*********************************
PDR下电复位控制函数 
功能：PDR下电复位启停控制
输入: ENABLE 启动下电复位
      DISABLE 停止下电复位
输出: 无
*********************************/
void ANAC_PDRCON_PDREN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->PDRCON |= (ANAC_PDRCON_PDREN_Msk);
	}
	else
	{
		ANAC->PDRCON &= ~(ANAC_PDRCON_PDREN_Msk);
	}
}
/********************************
读PDR下电复位控制状态函数
功能:读PDR下电复位控制状态
输入：无
输出:ENABLE 已启动
     DISABLE 已停止
********************************/
FunState ANAC_PDRCON_PDREN_Getable(void)
{
	if (ANAC->PDRCON & (ANAC_PDRCON_PDREN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
BOR下电复位电压配置函数
功能:下电复位电压配置
输入：配置BOR下电复位电压（bit2:1）
输出:无
********************************/
void ANAC_BORCON_BOR_PDRCFG_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->BORCON;
	tmpreg &= ~(ANAC_BORCON_BOR_PDRCFG_Msk);
	tmpreg |= (SetValue & ANAC_BORCON_BOR_PDRCFG_Msk);
	ANAC->BORCON = tmpreg;
}

/********************************
读BOR下电复位寄存器电压值函数
功能:读BOR下电复位电压值寄存器位
输入：无
输出:BOR寄存器电压值
********************************/
uint32_t ANAC_BORCON_BOR_PDRCFG_Get(void)
{
	return (ANAC->BORCON & ANAC_BORCON_BOR_PDRCFG_Msk);
}


/*********************************
BOR下电复位控制函数 
功能：BOR下电复位启停控制
输入: ENABLE 启动下电复位
      DISABLE 停止下电复位
输出: 无
*********************************/
void ANAC_BORCON_OFF_BOR_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->BORCON |= (ANAC_BORCON_OFF_BOR_Msk);
	}
	else
	{
		ANAC->BORCON &= ~(ANAC_BORCON_OFF_BOR_Msk);
	}
}
/********************************
读BOR下电复位控制状态函数
功能:读BOR下电复位控制状态
输入：无
输出:ENABLE 已启动
     DISABLE 已停止
********************************/
FunState ANAC_BORCON_OFF_BOR_Getable(void)
{
	if (ANAC->BORCON & (ANAC_BORCON_OFF_BOR_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
SVD电源跌落中断使能控制函数 
功能：SVD电源跌落中断启停控制
输入: ENABLE 启动SVD电源跌落中断
      DISABLE 停止SVD电源跌落中断
输出: 无
*********************************/
void ANAC_SVDCFG_PFIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->SVDCFG |= (ANAC_SVDCFG_PFIE_Msk);
	}
	else
	{
		ANAC->SVDCFG &= ~(ANAC_SVDCFG_PFIE_Msk);
	}
}
/********************************
读SVD电源跌落中断控制状态函数
功能:SVD电源跌落中断控制状态
输入：无
输出:ENABLE 已启动
     DISABLE 已停止
********************************/
FunState ANAC_SVDCFG_PFIE_Getable(void)
{
	if (ANAC->SVDCFG & (ANAC_SVDCFG_PFIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}
/*********************************
SVD电源恢复中断使能控制函数 
功能：SVD电源恢复中断启停控制
输入: ENABLE 启动SVD电源恢复中断
      DISABLE 停止SVD电源恢复中断
输出: 无
*********************************/
void ANAC_SVDCFG_PRIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->SVDCFG |= (ANAC_SVDCFG_PRIE_Msk);
	}
	else
	{
		ANAC->SVDCFG &= ~(ANAC_SVDCFG_PRIE_Msk);
	}
}

/********************************
读SVD电源恢复中断控制状态函数
功能:SVD电源恢复中断控制状态
输入：无
输出:ENABLE 已启动
     DISABLE 已停止
********************************/
FunState ANAC_SVDCFG_PRIE_Getable(void)
{
	if (ANAC->SVDCFG & (ANAC_SVDCFG_PRIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
SVD报警阈值配置函数
功能:SVD报警阈值电压配置
输入：配置报警电压（bit7:4）
输出:无
********************************/
void ANAC_SVDCFG_SVDLVL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->SVDCFG;
	tmpreg &= ~(ANAC_SVDCFG_SVDLVL_Msk);
	tmpreg |= (SetValue & ANAC_SVDCFG_SVDLVL_Msk);
	ANAC->SVDCFG = tmpreg;
}
/********************************
读SVD报警阈值函数
功能:获取SVD报警阈值
输入：无
输出:SVD报警阈值
********************************/
uint32_t ANAC_SVDCFG_SVDLVL_Get(void)
{
	return (ANAC->SVDCFG & ANAC_SVDCFG_SVDLVL_Msk);
}

/********************************
SVD数字滤波控制状态函数
功能:SVD电源恢复中断控制状态
输入：ENABLE 启动数字滤波（SVDMODE=1时必须置1）
      DISABLE 停止数字滤波
输出: 无
********************************/
void ANAC_SVDCFG_DFEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->SVDCFG |= (ANAC_SVDCFG_DFEN_Msk);
	}
	else
	{
		ANAC->SVDCFG &= ~(ANAC_SVDCFG_DFEN_Msk);
	}
}
/********************************
读SVD数字滤波控制状态函数
功能:SVD数字滤波控制状态
输入：无
输出:ENABLE 已启动
     DISABLE 已停止
********************************/
FunState ANAC_SVDCFG_DFEN_Getable(void)
{
	if (ANAC->SVDCFG & (ANAC_SVDCFG_DFEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
SVD工作模式选择函数
功能:SVD工作模式选择
输入：ANAC_SVDCFG_SVDMOD_INTERVAL：间歇使能模式（间歇使能模式下必须开启数字滤波）
      ANAC_SVDCFG_SVDMOD_ALWAYSON：常使能模式
输出: 无
********************************/
void ANAC_SVDCFG_SVDMOD_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->SVDCFG;
	tmpreg &= ~(ANAC_SVDCFG_SVDMOD_Msk);
	tmpreg |= (SetValue & ANAC_SVDCFG_SVDMOD_Msk);
	ANAC->SVDCFG = tmpreg;
}
/********************************
读SVD工作模式函数
功能:读SVD工作模式
输入：无
输出:工作模式状态位
********************************/
uint32_t ANAC_SVDCFG_SVDMOD_Get(void)
{
	return (ANAC->SVDCFG & ANAC_SVDCFG_SVDMOD_Msk);
}

/********************************
SVD间歇使能间隔配置函数
功能:SVD间歇使能间隔
输入：00：15.625ms
      01：62.5ms
      10：256ms
      11：1s
输出: 无
********************************/
void ANAC_SVDCFG_SVDITVL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->SVDCFG;
	tmpreg &= ~(ANAC_SVDCFG_SVDITVL_Msk);
	tmpreg |= (SetValue & ANAC_SVDCFG_SVDITVL_Msk);
	ANAC->SVDCFG = tmpreg;
}
/********************************
读SVD间歇使能间隔时间配置函数
功能:读SVD工作模式
输入：无
输出:间歇使能间隔时间状态位
********************************/
uint32_t ANAC_SVDCFG_SVDITVL_Get(void)
{
	return (ANAC->SVDCFG & ANAC_SVDCFG_SVDITVL_Msk);
}

/*********************************
SVD测试使能控制函数 
功能：SVD测试使能启停控制
输入: ENABLE 启动SVD测试使能
      DISABLE 停止SVD测试使能
输出: 无
*********************************/
void ANAC_SVDCON_SVDTE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->SVDCON |= (ANAC_SVDCON_SVDTE_Msk);
	}
	else
	{
		ANAC->SVDCON &= ~(ANAC_SVDCON_SVDTE_Msk);
	}
}
/*********************************
读SVD测试使能控制状态函数 
功能：读SVD测试使能控制状态
输入: 无
输出: ENABLE 启动SVD测试使能
      DISABLE 停止SVD测试使能
*********************************/
FunState ANAC_SVDCON_SVDTE_Getable(void)
{
	if (ANAC->SVDCON & (ANAC_SVDCON_SVDTE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
SVD使能控制函数 
功能：SVD使能启停控制
输入: ENABLE 启动SVD使能
      DISABLE 停止SVD使能
输出: 无
*********************************/
void ANAC_SVDCON_SVDEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->SVDCON |= (ANAC_SVDCON_SVDEN_Msk);
	}
	else
	{
		ANAC->SVDCON &= ~(ANAC_SVDCON_SVDEN_Msk);
	}
}

/*********************************
读SVD使能控制状态函数 
功能：读SVD使能控制状态
输入: 无
输出: ENABLE 启动SVD使能
      DISABLE 停止SVD使能
*********************************/
FunState ANAC_SVDCON_SVDEN_Getable(void)
{
	if (ANAC->SVDCON & (ANAC_SVDCON_SVDEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}


/*********************************
读SVD电源检测输出制状态函数 
功能：读SVD电源检测输出状态
输入: 无
输出: SET   电源电压高于SVD当前阈值
      RESET 电源电压低于SVD当前阈值
*********************************/
FlagStatus ANAC_SVDSIF_SVDO_Chk(void)
{
	if (ANAC->SVDSIF & ANAC_SVDSIF_SVDO_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
SVD电源跌落中断标志寄存器函数 
功能：电源跌落中断标志，电源电压跌落到SVD阈值之下时置位，软件写1清零
输入: 无
输出: 无
*********************************/
void ANAC_SVDSIF_PFF_Clr(void)
{
	ANAC->SVDSIF = ANAC_SVDSIF_PFF_Msk;
}
/*********************************
读SVD电源跌落中断标志寄存器状态函数 
功能：读SVD电源跌落中断标志状态
输入: 无
输出: SET   中断状态为1
      RESET  中断状态为0
*********************************/
FlagStatus ANAC_SVDSIF_PFF_Chk(void)
{
	if (ANAC->SVDSIF & ANAC_SVDSIF_PFF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
SVD电源恢复中断标志寄存器函数 
功能：电源恢复中断标志，电源电压上升到SVD阈值之上时置位，软件写1清零
输入: 无
输出: 无
*********************************/
void ANAC_SVDSIF_PRF_Clr(void)
{
	ANAC->SVDSIF = ANAC_SVDSIF_PRF_Msk;
}
/*********************************
读SVD电源恢复中断标志寄存器状态函数 
功能：读SVD电源恢复中断标志状态
输入: 无
输出: SET   中断状态为1
      RESET  中断状态为0
*********************************/
FlagStatus ANAC_SVDSIF_PRF_Chk(void)
{
	if (ANAC->SVDSIF & ANAC_SVDSIF_PRF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
读SVD内部滤波后的电压检测标志寄存器状态函数 
功能：读SVD内部滤波后的电压检测标志状态
输入: 无
输出: SET   不欠压
      RESET  欠压
*********************************/
FlagStatus ANAC_SVDSIF_SVDR_Chk(void)
{
	if (ANAC->SVDSIF & ANAC_SVDSIF_SVDR_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
设置SVD基准输入电压函数
功能：设置SVD基准输入电压为0.7V、0.75V、0.8V三选一，默认为0.8V
输入: ANAC_SVDVOL_V0P7EN_Msk、ANAC_SVDVOL_V0P75EN_Msk、ANAC_SVDVOL_V0P8EN_Msk
输出: 无
*********************************/
void ANAC_SVDVOL_CFG(uint32_t SetValue)
{
  if(SetValue==ANAC_SVDVOL_V0P7EN_Msk)
  {
    ANAC->SVDVOL=ANAC_SVDVOL_V0P7EN_Msk;
  }
  else if(SetValue==ANAC_SVDVOL_V0P75EN_Msk)
  {
    ANAC->SVDVOL=ANAC_SVDVOL_V0P75EN_Msk;
  }
  else
  {
    ANAC->SVDVOL=ANAC_SVDVOL_V0P8EN_Msk;
  }
}

/********************************
读SVD参考电压选择寄存器
功能:读SVD参考电压
输入：无
输出: SVD参考电压
********************************/
uint32_t ANAC_SVDVOL_Get(void)
{
	return (ANAC->SVDVOL );
}

/*********************************
XTLF低频检测报警中断使能启动控制函数 
功能：XTLF低频检测报警中断启停控制
输入: ENABLE 启动XTLF低频检测报警中断
      DISABLE 停止XTLF低频检测报警中断
输出: 无
*********************************/
void ANAC_FDETIE_FDET_IE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->FDETIE |= (ANAC_FDETIE_FDET_IE_Msk);
	}
	else
	{
		ANAC->FDETIE &= ~(ANAC_FDETIE_FDET_IE_Msk);
	}
}
/*********************************
XTLF低频检测报警中断使能控制位状态获取函数 
功能：获取XTLF低频检测报警中断使能控制状态
输入:无
输出: ENABLE XTLF低频检测报警中断已启动
      DISABLE XTLF低频检测报警中断已停止
*********************************/
FunState ANAC_FDETIE_FDET_IE_Getable(void)
{
	if (ANAC->FDETIE & (ANAC_FDETIE_FDET_IE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}


/*********************************
XTLF停振检测模块输出状态获取函数 
功能：获取XTLF停振检测模块输出状态
输入:无
输出: SET    XTLF未停振
      RESET  XTLF停振
*********************************/
FlagStatus ANAC_FDETIF_FDETO_Chk(void)
{
	if (ANAC->FDETIF & ANAC_FDETIF_FDETO_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
XTLF停振检测中断标志寄存器函数 
功能：清XTLF停振检测中断标志
输入: 停振检测中断标志寄存器，XTLF停振时硬件异步置位，软件写1清零；只有在FDETO不为0的情况下才能够清除此寄存器
输出: 无
*********************************/
void ANAC_FDETIF_FDETIF_Clr(void)
{
	ANAC->FDETIF = ANAC_FDETIF_FDETIF_Msk;
}
/*********************************
XTLF停振检测中断标志获取函数 
功能：XTLF停振检测中断标志获取
输入:无
输出: SET   产生标志
      RESET 未产生标志
*********************************/
FlagStatus ANAC_FDETIF_FDETIF_Chk(void)
{
	if (ANAC->FDETIF & ANAC_FDETIF_FDETIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}
/*********************************
ADC中断使能控制函数 
功能：ADC中断启停控制
输入: ENABLE 启动高位计数器
      DISABLE 停止高位计数器
输出: 无
*********************************/
void ANAC_ADCCON_ADC_IE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->ADCCON |= (ANAC_ADCCON_ADC_IE_Msk);
	}
	else
	{
		ANAC->ADCCON &= ~(ANAC_ADCCON_ADC_IE_Msk);
	}
}
/*********************************
ADC中断使能状态获取函数 
功能：ADC中断使能状态
输入: 无
输出: ENABLE   ADC中断已启动
      DISABLE  ADC中断已停止
*********************************/
FunState ANAC_ADCCON_ADC_IE_Getable(void)
{
	if (ANAC->ADCCON & (ANAC_ADCCON_ADC_IE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
ADC外部电压通道选择函数 
功能：ADC外部电压通道选择
输入: ANAC_ADCCON_ADC_VANA_EN_PTAT：ADC用作温度传感器
      ANAC_ADCCON_ADC_VANA_EN_VOLTAGE：ADC用于测量外部电压
输出: 无
*********************************/
void ANAC_ADCCON_ADC_VANA_EN_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->ADCCON;
	tmpreg &= ~(ANAC_ADCCON_ADC_VANA_EN_Msk);
	tmpreg |= (SetValue & ANAC_ADCCON_ADC_VANA_EN_Msk);
	ANAC->ADCCON = tmpreg;
}
/*********************************
ADC外部电压通道获取配置函数 
功能：获取ADC外部电压通道模式
输入: 无
输出: 通道寄存器值
*********************************/
uint32_t ANAC_ADCCON_ADC_VANA_EN_Get(void)
{
	return (ANAC->ADCCON & ANAC_ADCCON_ADC_VANA_EN_Msk);
}

/*********************************
ADC使能信号函数 
功能：ADC使能
输入:  ENABLE   ADC已启动
       DISABLE  ADC已停止
输出: 无
*********************************/
void ANAC_ADCCON_ADC_EN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->ADCCON |= (ANAC_ADCCON_ADC_EN_Msk);
	}
	else
	{
		ANAC->ADCCON &= ~(ANAC_ADCCON_ADC_EN_Msk);
	}
}
/*********************************
ADC使能状态获取函数 
功能：获取ADC是否使能
输入: 无
输出:  ENABLE   ADC已启动
       DISABLE  ADC已停止
*********************************/
FunState ANAC_ADCCON_ADC_EN_Getable(void)
{
	if (ANAC->ADCCON & (ANAC_ADCCON_ADC_EN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
ADCTRIM值配置函数 
功能：ADCTRIM值配置
输入: ADC TRIM值
输出:  无
*********************************/
void ANAC_ADCTRIM_Write(uint32_t SetValue)
{
	ANAC->ADCTRIM = (SetValue & ANAC_ADCTRIM_ADC_TRIM_Msk);
}
/*********************************
获取ADCTRIM值函数 
功能：获取ADCTRIM值
输入: 无
输出:  ADC TRIM值
*********************************/
uint32_t ANAC_ADCTRIM_Read(void)
{
	return (ANAC->ADCTRIM & ANAC_ADCTRIM_ADC_TRIM_Msk);
}

/*********************************
获取ADC测量值函数 
功能：获取ADC测量输出值
输入: 无
输出:  ADC测量输出值
*********************************/
uint32_t ANAC_ADCDATA_Read(void)
{
	return (ANAC->ADCDATA & ANAC_ADCDATA_ADC_DATA_Msk);
}

/*********************************
ADC转换完成输出标志函数 
功能：ADC转换完成输出标志，硬件置位，ADC关闭才会清0
输入: 无
输出:  SET     ADC_DONE置位
       RESET   ADC_DONE为0
*********************************/
FlagStatus ANAC_ADCIF_ADC_DONE_Chk(void)
{
	if (ANAC->ADCIF & ANAC_ADCIF_ADC_DONE_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
ADC转换完成中断标志函数 
功能：ADC转换完成中断标志，ADC转换完成中断标志，硬件置位，软件写1清零，写0无效
输入: 无
输出:  无
*********************************/
void ANAC_ADCIF_ADC_IF_Clr(void)
{
	ANAC->ADCIF = ANAC_ADCIF_ADC_IF_Msk;
}

FlagStatus ANAC_ADCIF_ADC_IF_Chk(void)
{
	if (ANAC->ADCIF & ANAC_ADCIF_ADC_IF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
ADC通道buffer使能函数 
功能：ADC通道buffer是否使能
输入: ENABLE   启动buffer
      DISABLE  停止buffer
输出: 无
*********************************/
void ANAC_ADCINSEL_BUFEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->ADCINSEL |= (ANAC_ADCINSEL_BUFEN_Msk);
	}
	else
	{
		ANAC->ADCINSEL &= ~(ANAC_ADCINSEL_BUFEN_Msk);
	}
}

/*********************************
ADC通道buffer使能状态标志获取函数 
功能：获取ADC通道buffer使能状态标志
输入: 无
输出: ENABLE   启动buffer
      DISABLE  停止buffer
*********************************/
FunState ANAC_ADCINSEL_BUFEN_Getable(void)
{
	if (ANAC->ADCINSEL & (ANAC_ADCINSEL_BUFEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
ADC通道buffer Bypass 配置函数 
功能：配置ADC通道buffer是否Bypass，使用ADC测量外部信号输入时，不要Bypass Buffer，使用ADC测量电源电压时，必须将此位置1
输入: ENABLE   启动Bypass
      DISABLE  停止Bypass
输出: 无
*********************************/
void ANAC_ADCINSEL_BUFBYP_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->ADCINSEL |= (ANAC_ADCINSEL_BUFBYP_Msk);
	}
	else
	{
		ANAC->ADCINSEL &= ~(ANAC_ADCINSEL_BUFBYP_Msk);
	}
}

/*********************************
ADC通道buffer Bypass状态获取函数 
功能：获取ADC通道buffer Bypass状态
输入: 无
输出: ENABLE   启动buffer Bypass
      DISABLE  停止buffer Bypass
*********************************/
FunState ANAC_ADCINSEL_BUFBYP_Getable(void)
{
	if (ANAC->ADCINSEL & (ANAC_ADCINSEL_BUFBYP_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}


/*********************************
ADC通道选择函数 
功能：ADC输入通道选择，
输入: ADC_IN1到ADC_IN8   VDD
输出: 无
*********************************/
void ANAC_ADCINSEL_BUFSEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->ADCINSEL;
	tmpreg &= ~(ANAC_ADCINSEL_BUFSEL_Msk);
	tmpreg |= (SetValue & ANAC_ADCINSEL_BUFSEL_Msk);
	ANAC->ADCINSEL = tmpreg;
}


/*********************************
ADC通道选择获取函数 
功能：获取ADC
输入: 无
输出: 选取的ADC通道
*********************************/
uint32_t ANAC_ADCINSEL_BUFSEL_Get(void)
{
	return (ANAC->ADCINSEL & ANAC_ADCINSEL_BUFSEL_Msk);
}

/*********************************
TRNG随机数启动控制函数 
功能：RNG使能寄存器，软件写1启动，完成随机数产生后自动清零
输入: ENABLE   启动随机数
      DISABLE  停止随机数
输出: 无
*********************************/
void ANAC_TRNGCON_TRNGEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->TRNGCON |= (ANAC_TRNGCON_TRNGEN_Msk);
	}
	else
	{
		ANAC->TRNGCON &= ~(ANAC_TRNGCON_TRNGEN_Msk);
	}
}
/*********************************
TRNG随机数启动控制状态函数 
功能：获取RNG状态
输入: 无
输出: ENABLE   启动随机数
      DISABLE  停止随机数
*********************************/
FunState ANAC_TRNGCON_TRNGEN_Getable(void)
{
	if (ANAC->TRNGCON & (ANAC_TRNGCON_TRNGEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器1输出状态获取函数 
功能：获取比较器输出高低
输入: 无
输出: SET   比较器输出高
      RESET 比较器输出低
*********************************/
FlagStatus ANAC_COMP1CR_CMP1O_Chk(void)
{
	if (ANAC->COMP1CR & ANAC_COMP1CR_CMP1O_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
比较器1正极输入选择函数 
功能：比较器1正极输入选择
输入: PF6 PF1 PG2 PG3
输出: 无
*********************************/
void ANAC_COMP1CR_V1PSEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMP1CR;
	tmpreg &= ~(ANAC_COMP1CR_V1PSEL_Msk);
	tmpreg |= (SetValue & ANAC_COMP1CR_V1PSEL_Msk);
	ANAC->COMP1CR = tmpreg;
}
/*********************************
比较器1正极输入源获取函数 
功能：获取比较器1正极输入源
输入: 无
输出: PF6 PF1 PG2 PG3
*********************************/
uint32_t ANAC_COMP1CR_V1PSEL_Get(void)
{
	return (ANAC->COMP1CR & ANAC_COMP1CR_V1PSEL_Msk);
}

/*********************************
比较器1负极输入选择函数 
功能：比较器1负极输入选择
输入: PF5 PF0   Vref_0.8V  Vref/2_0.4V
输出: 无
*********************************/
void ANAC_COMP1CR_V1NSEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMP1CR;
	tmpreg &= ~(ANAC_COMP1CR_V1NSEL_Msk);
	tmpreg |= (SetValue & ANAC_COMP1CR_V1NSEL_Msk);
	ANAC->COMP1CR = tmpreg;
}
/*********************************
比较器1负极输入源获取函数 
功能：获取比较器1负极输入源
输入: 无
输出: PF5 PF0   Vref_0.8V  Vref/2_0.4V
*********************************/
uint32_t ANAC_COMP1CR_V1NSEL_Get(void)
{
	return (ANAC->COMP1CR & ANAC_COMP1CR_V1NSEL_Msk);
}

/*********************************
比较器1使能控制函数 
功能：比较器1使能
输入: ENABLE   启动比较器1
      DISABLE  停止比较器1
输出: 无
*********************************/
void ANAC_COMP1CR_CMP1EN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMP1CR |= (ANAC_COMP1CR_CMP1EN_Msk);
	}
	else
	{
		ANAC->COMP1CR &= ~(ANAC_COMP1CR_CMP1EN_Msk);
	}
}
/*********************************
比较器1状态获取函数 
功能：获取比较器1状态
输入: 无
输出: ENABLE   启动比较器1
      DISABLE  停止比较器1
*********************************/
FunState ANAC_COMP1CR_CMP1EN_Getable(void)
{
	if (ANAC->COMP1CR & (ANAC_COMP1CR_CMP1EN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}


/*********************************
比较器2输出状态获取函数 
功能：获取比较器输出高低
输入: 无
输出: SET   比较器输出高
      RESET 比较器输出低
*********************************/
FlagStatus ANAC_COMP2CR_CMP2O_Chk(void)
{
	if (ANAC->COMP2CR & ANAC_COMP2CR_CMP2O_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
比较器2正极输入选择函数 
功能：比较器2正极输入选择
输入: PC15 PE4
输出: 无
*********************************/
void ANAC_COMP2CR_V2PSEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMP2CR;
	tmpreg &= ~(ANAC_COMP2CR_V2PSEL_Msk);
	tmpreg |= (SetValue & ANAC_COMP2CR_V2PSEL_Msk);
	ANAC->COMP2CR = tmpreg;
}
/*********************************
比较器2正极输入源获取函数 
功能：获取比较器1正极输入源
输入: 无
输出: PC15 PE4
*********************************/
uint32_t ANAC_COMP2CR_V2PSEL_Get(void)
{
	return (ANAC->COMP2CR & ANAC_COMP2CR_V2PSEL_Msk);
}

/*********************************
比较器2负极输入选择函数 
功能：比较器2负极输入选择
输入: PC14 PE3   Vref_0.8V  Vref/2_0.4V
输出: 无
*********************************/
void ANAC_COMP2CR_V2NSEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMP2CR;
	tmpreg &= ~(ANAC_COMP2CR_V2NSEL_Msk);
	tmpreg |= (SetValue & ANAC_COMP2CR_V2NSEL_Msk);
	ANAC->COMP2CR = tmpreg;
}
/*********************************
比较器2负极输入源获取函数 
功能：获取比较器1负极输入源
输入: 无
输出: PC14 PE3   Vref_0.8V  Vref/2_0.4V
*********************************/
uint32_t ANAC_COMP2CR_V2NSEL_Get(void)
{
	return (ANAC->COMP2CR & ANAC_COMP2CR_V2NSEL_Msk);
}

/*********************************
比较器2使能控制函数 
功能：比较器2使能
输入: ENABLE   启动比较器2
      DISABLE  停止比较器2
输出: 无
*********************************/
void ANAC_COMP2CR_CMP2EN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMP2CR |= (ANAC_COMP2CR_CMP2EN_Msk);
	}
	else
	{
		ANAC->COMP2CR &= ~(ANAC_COMP2CR_CMP2EN_Msk);
	}
}
/*********************************
比较器2状态获取函数 
功能：获取比较器2状态
输入: 无
输出: ENABLE   启动比较器2
      DISABLE  停止比较器2
*********************************/
FunState ANAC_COMP2CR_CMP2EN_Getable(void)
{
	if (ANAC->COMP2CR & (ANAC_COMP2CR_CMP2EN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器2数字滤波配置函数 
功能：比较器2数字滤波开启控制
输入: ENABLE   启动数字滤波
      DISABLE  停止数字滤波
输出: 无
*********************************/
void ANAC_COMPICR_CMP2DF_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR |= (ANAC_COMPICR_CMP2DF_Msk);
	}
	else
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_CMP2DF_Msk);
	}
}
/*********************************
比较器2数字滤波状态获取函数 
功能：获取比较器2数字滤波状态
输入: 无
输出: ENABLE   启动比较器2数字滤波
      DISABLE  停止比较器2数字滤波
*********************************/
FunState ANAC_COMPICR_CMP2DF_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_CMP2DF_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器1数字滤波配置函数 
功能：比较器1数字滤波开启控制
输入: ENABLE   启动数字滤波
      DISABLE  停止数字滤波
输出: 无
*********************************/
void ANAC_COMPICR_CMP1DF_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR |= (ANAC_COMPICR_CMP1DF_Msk);
	}
	else
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_CMP1DF_Msk);
	}
}
/*********************************
比较器1数字滤波状态获取函数 
功能：获取比较器1数字滤波状态
输入: 无
输出: ENABLE   启动比较器1数字滤波
      DISABLE  停止比较器1数字滤波
*********************************/
FunState ANAC_COMPICR_CMP1DF_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_CMP1DF_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器Buffer Bypass控制函数 
功能：比较器Buffer Bypass选择
输入: ENABLE   启动比较器Buffer Bypass
      DISABLE  停止比较器Buffer Bypass
输出: 无
*********************************/
void ANAC_COMPICR_BUFBYP_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR |= (ANAC_COMPICR_BUFBYP_Msk);
	}
	else
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_BUFBYP_Msk);
	}
}
/*********************************
比较器Buffer Bypass状态获取函数 
功能：获取比较器Buffer Bypass状态
输入: 无
输出: ENABLE   比较器Buffer Bypass使能
      DISABLE  比较器Buffer Bypass禁止
*********************************/
FunState ANAC_COMPICR_BUFBYP_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_BUFBYP_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器Buffer 控制函数 
功能：比较器Buffer 选择
输入: ENABLE   启动比较器Buffer 
      DISABLE  停止比较器Buffer 
输出: 无
*********************************/
void ANAC_COMPICR_BUFENB_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_BUFENB_Msk);
	}
	else
	{
		ANAC->COMPICR |= (ANAC_COMPICR_BUFENB_Msk);
	}
}
/*********************************
比较器Buffer 状态获取函数 
功能：获取比较器Buffer状态
输入: 无
输出: ENABLE   比较器Buffer启动
      DISABLE  比较器Buffer波停止
*********************************/
FunState ANAC_COMPICR_BUFENB_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_BUFENB_Msk))
	{
		return DISABLE;
	}
	else
	{
		return ENABLE;
	}
}


/*********************************
比较器2中断源选择函数 
功能：比较器2中断源选择bit[5:4]
输入: ANAC_COMPICR_CMP2SEL_OUTBOTH：比较器2输出上升或下降沿产生中断
      ANAC_COMPICR_CMP2SEL_OUTRISE：比较器2输出上升沿产生中断
      ANAC_COMPICR_CMP2SEL_OUTFALL：比较器2输出下降沿产生中断
输出: 无
*********************************/
void ANAC_COMPICR_CMP2SEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMPICR;
	tmpreg &= ~(ANAC_COMPICR_CMP2SEL_Msk);
	tmpreg |= (SetValue & ANAC_COMPICR_CMP2SEL_Msk);
	ANAC->COMPICR = tmpreg;
}
/*********************************
比较器2中断源获取函数 
功能：获取比较器2中断源bit[5:4]
输入: 无
输出: ANAC_COMPICR_CMP2SEL_OUTBOTH：比较器2输出上升或下降沿产生中断
      ANAC_COMPICR_CMP2SEL_OUTRISE：比较器2输出上升沿产生中断
      ANAC_COMPICR_CMP2SEL_OUTFALL：比较器2输出下降沿产生中断
*********************************/
uint32_t ANAC_COMPICR_CMP2SEL_Get(void)
{
	return (ANAC->COMPICR & ANAC_COMPICR_CMP2SEL_Msk);
}

/*********************************
比较器1中断源选择函数 
功能：比较器1中断源选择bit[3:2]
输入: ANAC_COMPICR_CMP1SEL_OUTBOTH：比较器1输出上升或下降沿产生中断
      ANAC_COMPICR_CMP1SEL_OUTRISE：比较器1输出上升沿产生中断
      ANAC_COMPICR_CMP1SEL_OUTFALL：比较器1输出下降沿产生中断
输出: 无
*********************************/
void ANAC_COMPICR_CMP1SEL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = ANAC->COMPICR;
	tmpreg &= ~(ANAC_COMPICR_CMP1SEL_Msk);
	tmpreg |= (SetValue & ANAC_COMPICR_CMP1SEL_Msk);
	ANAC->COMPICR = tmpreg;
}
/*********************************
比较器1中断源获取函数 
功能：获取比较器1中断源bit[3:2]
输入: 无
输出: ANAC_COMPICR_CMP1SEL_OUTBOTH：比较器1输出上升或下降沿产生中断
      ANAC_COMPICR_CMP1SEL_OUTRISE：比较器1输出上升沿产生中断
      ANAC_COMPICR_CMP1SEL_OUTFALL：比较器1输出下降沿产生中断
*********************************/
uint32_t ANAC_COMPICR_CMP1SEL_Get(void)
{
	return (ANAC->COMPICR & ANAC_COMPICR_CMP1SEL_Msk);
}

/*********************************
比较器2中断使能控制函数 
功能：比较器2中断控制开关
输入: ENABLE   启动比较器2中断
      DISABLE  停止比较器2中断波
输出: 无
*********************************/
void ANAC_COMPICR_CMP2IE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR |= (ANAC_COMPICR_CMP2IE_Msk);
	}
	else
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_CMP2IE_Msk);
	}
}

/*********************************
比较器2中断使能状态函数 
功能：获取比较器2中断控制状态
输入: 无
输出: ENABLE   比较器2中断启动
      DISABLE  比较器2中断波停止
*********************************/
FunState ANAC_COMPICR_CMP2IE_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_CMP2IE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器1中断使能控制函数 
功能：比较器1中断控制开关
输入: ENABLE   启动比较器1中断
      DISABLE  停止比较器1中断波
输出: 无
*********************************/
void ANAC_COMPICR_CMP1IE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		ANAC->COMPICR |= (ANAC_COMPICR_CMP1IE_Msk);
	}
	else
	{
		ANAC->COMPICR &= ~(ANAC_COMPICR_CMP1IE_Msk);
	}
}
/*********************************
比较器1中断使能状态函数 
功能：获取比较器1中断控制状态
输入: 无
输出: ENABLE   比较器1中断启动
      DISABLE  比较器1中断波停止
*********************************/
FunState ANAC_COMPICR_CMP1IE_Getable(void)
{
	if (ANAC->COMPICR & (ANAC_COMPICR_CMP1IE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
比较器2中断标志控制函数 
功能：比较器2中断标志控制
输入: 硬件置位，软件写1清零
输出: 无
*********************************/
void ANAC_COMPIF_CMP2IF_Clr(void)
{
	ANAC->COMPIF = ANAC_COMPIF_CMP2IF_Msk;
}
/*********************************
比较器2中断标志状态函数 
功能：获取比较器2中断标志状态
输入: 无
输出: SET    比较器2产生中断标志
      RESET  比较器2未产生中断标志
*********************************/
FlagStatus ANAC_COMPIF_CMP2IF_Chk(void)
{
	if (ANAC->COMPIF & ANAC_COMPIF_CMP2IF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
比较器1中断标志控制函数 
功能：比较器1中断标志控制
输入: 硬件置位，软件写1清零
输出: 无
*********************************/
void ANAC_COMPIF_CMP1IF_Clr(void)
{
	ANAC->COMPIF = ANAC_COMPIF_CMP1IF_Msk;
}
/*********************************
比较器1中断标志状态函数 
功能：获取比较器1中断标志状态
输入: 无
输出: SET    比较器1产生中断标志
      RESET  比较器1未产生中断标志
*********************************/
FlagStatus ANAC_COMPIF_CMP1IF_Chk(void)
{
	if (ANAC->COMPIF & ANAC_COMPIF_CMP1IF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

void ANAC_Deinit(void)
{
	ANAC->PDRCON = 0x00000003;
	ANAC->BORCON = 0x00000002;
	ANAC->SVDCFG = 0x00000008;
	ANAC->SVDCON = 0x00000000;
	//ANAC->SVDSIF = ;
	ANAC->FDETIE = 0x00000000;
	//ANAC->FDETIF = ;
	ANAC->ADCCON = 0x00000000;
	ANAC->ADCTRIM = 0x00000000;
	//ANAC->ADCDATA = ;
	//ANAC->ADCIF = ;
	ANAC->ADCINSEL = 0x00000000;
	ANAC->TRNGCON = 0x00000000;
	ANAC->COMP1CR = 0x00000000;
	ANAC->COMP2CR = 0x00000000;
	ANAC->COMPICR = 0x00000040;
	//ANAC->COMPIF = ;
}
//Code_End



/*********************************
SVD初始化配置函数 
功能：SVD初始化配置
输入: ANAC_SVD_InitTypeDef* para，结构体函数中配置工作模式、阈值、数字滤波等功能
输出: 无
*********************************/
void ANAC_SVD_Init(ANAC_SVD_InitTypeDef* para)
{
	ANAC_SVDCFG_SVDMOD_Set(para->SVDMOD);	//SVD工作模式选择
	ANAC_SVDCFG_SVDITVL_Set(para->SVDITVL);	//SVD间歇使能间隔设置
	ANAC_SVDCFG_SVDLVL_Set(para->SVDLVL);	//SVD报警阈值设置
	ANAC_SVDCFG_DFEN_Setable(para->DFEN);	//SVD数字滤波设置（SVDMODE=1时必须置1）
	ANAC_SVDCFG_PFIE_Setable(para->PFIE);	//SVD电源跌落中断设置
	ANAC_SVDCFG_PRIE_Setable(para->PRIE);	//SVD电源恢复中断设置
	ANAC_SVDCON_SVDTE_Setable(DISABLE);	//关闭测试功能
	ANAC_SVDCON_SVDEN_Setable(para->SVDEN);	//SVD使能设置
}

/*********************************
ADC初始化配置函数 
功能：ADC初始化配置
输入: ANAC_ADC_InitTypeDef* para，结构体函数中配置ADCTRIM、内外部通道、buff等功能
输出: 无
*********************************/
void ANAC_ADC_Init(ANAC_ADC_InitTypeDef* para)
{
	ANAC_ADCTRIM_Write(para->ADC_TRIM);				//<等效参考电压调校寄存器
	ANAC_ADCCON_ADC_VANA_EN_Set(para->ADC_VANA_EN);	//<内部通道、外部通道选择
	ANAC_ADCINSEL_BUFSEL_Set(para->BUFSEL);			//<ADC输入通道选择
	ANAC_ADCINSEL_BUFBYP_Setable(para->BUFBYP);		//<ADC输入Buffer Bypass，ADC输入Buffer Bypass，使用ADC测量外部信号输入时，关闭bypass， 使用ADC测量内部信号时，必须将此位置1（bypass）
	ANAC_ADCINSEL_BUFEN_Setable(para->BUFEN);		//<ADC输入通道Buffer使能，常开即可
	ANAC_ADCCON_ADC_IE_Setable(para->ADC_IE);		//<ADC中断使能
	ANAC_ADCCON_ADC_EN_Setable(para->ADC_EN);		//<ADC使能
}

/*********************************
ADC测量通道配置函数 
功能：ADC通道配置
输入: CH_PTAT  CH_VDD  CH_IN1-8
输出: 无
*********************************/
void ANAC_ADC_Channel_SetEx(uint8_t ChSel_def)
{
	switch( ChSel_def )
	{
		case CH_PTAT_Auto://温度传感器通道,用于自动温补
			ANAC_ADCCON_ADC_VANA_EN_Set(ANAC_ADCCON_ADC_VANA_EN_PTAT);	//测温	
			ANAC_ADCTRIM_Write(const_adc_TrimT_Auto);						//配置参考电压
			ANAC_ADCINSEL_BUFBYP_Setable(ENABLE);	//内部信号测量，bypassbuffer
			ANAC_ADCINSEL_BUFEN_Setable(ENABLE);	//
			break;
    
    case CH_PTAT://温度传感器通道(除自动温补)
			ANAC_ADCCON_ADC_VANA_EN_Set(ANAC_ADCCON_ADC_VANA_EN_PTAT);	//测温	
			ANAC_ADCTRIM_Write(const_adc_TrimT);						//配置参考电压
			ANAC_ADCINSEL_BUFBYP_Setable(ENABLE);	//内部信号测量，bypassbuffer
			ANAC_ADCINSEL_BUFEN_Setable(ENABLE);	//
			break;
    
		case CH_VDD://电源VDD通道
			ANAC_ADCCON_ADC_VANA_EN_Set(ANAC_ADCCON_ADC_VANA_EN_VOLTAGE);	//测VDD电压，注意ADC量程为4.5V（2.2mV*0x7FF）,高于4.5V的电压返回0x7FF
			ANAC_ADCTRIM_Write(const_adc_TrimV);							//配置参考电压
			ANAC_ADCINSEL_BUFBYP_Setable(ENABLE);		//内部输入
			ANAC_ADCINSEL_BUFEN_Setable(ENABLE);		//电压
			ANAC_ADCINSEL_BUFSEL_Set(ANAC_ADCINSEL_BUFSEL_VDD);//vdd
			break;
		
		case CH_IN1://外部输入通道
		case CH_IN2:
		case CH_IN3:
		case CH_IN4:
		case CH_IN5:
		case CH_IN6:
		case CH_IN7:
		case CH_IN8:
			ANAC_ADCCON_ADC_VANA_EN_Set(ANAC_ADCCON_ADC_VANA_EN_VOLTAGE);	//测电压，输入电压不可高于芯片VDD，注意ADC量程为4.5V（2.2mV*0x7FF）,高于4.5V的电压返回0x7FF
			ANAC_ADCTRIM_Write(const_adc_TrimV);							//配置参考电压
			ANAC_ADCINSEL_BUFBYP_Setable(DISABLE);	//外部信号测量，关闭bypassbuffer
			ANAC_ADCINSEL_BUFEN_Setable(ENABLE);	//打开bufferen
			ANAC_ADCINSEL_BUFSEL_Set((ANAC_ADCINSEL_BUFSEL_ADC_IN1 + (ChSel_def - CH_IN1)));//输入通道选择
			break;
		
		default:
			break;
	}
}


/*ADC 电压计算函数
	输入：AD值、电源电压
	输出：电压@mV 
*/
//被测电压低于100mV后测试结果不太准确
//被测电压超过4.4V后测试结果不太准确
//被测电压不可超过电源电压
//被测电压不可超过4.5V
/*********************************
ADC电压计算函数 
功能：计算ADC测量电压值
输入: ADC值、电源电压
输出: 电压值
*********************************/
uint32_t ANAC_ADC_VoltageCalc(uint32_t fADCData,uint8_t Vdd)
{
	uint32_t fVolt = 0;
	uint32_t AdcTrim = 0;
	AdcTrim = ANAC_ADCTRIM_Read();

	if((AdcTrim == 0x0ff)&&(const_adc_Slope_0FF > 8000) && (const_adc_Slope_0FF < 9600))//有adctrim——0ff的成测数据
	{
			//使用成测数据	
			//ADC值换算为电压值
			if(fADCData <= 8)
				fVolt = 0;
			else if(fADCData <= 12)
				fVolt = (fADCData-8)*20;
			else if(fADCData <= 37)
				fVolt = fADCData*const_adc_Slope_0FF/1000;
			else if(fADCData <= 125)
				fVolt = (fADCData*98*const_adc_Slope_0FF/100000);
			else
				fVolt = fADCData*const_adc_Slope_0FF/1000+const_adc_Offset_0FF/100;	
	}
	else if((AdcTrim == 0x1ff)&&(const_adc_Slope_1FF > 4000) && (const_adc_Slope_1FF < 4800))//有adctrim——1ff的成测数据
	{
			//使用成测数据	
			//ADC值换算为电压值
			if(fADCData <= 15)
				fVolt = 0;
			else if(fADCData <= 25)
				fVolt = (fADCData-15)*10;
			else if(fADCData <= 75)
				fVolt = fADCData*const_adc_Slope_1FF/1000;
			else if(fADCData <= 250)
				fVolt = (fADCData*98*const_adc_Slope_1FF/100000);
			else
				fVolt = fADCData*const_adc_Slope_1FF/1000+const_adc_Offset_1FF/100;	
	}
	else
	{
		if(AdcTrim == 0x1ff)
		{
			fADCData*=2;//adc值扩大2倍
		}
		else if(AdcTrim == 0x0ff)
		{
			fADCData*=4;//adc值扩大4倍
		}
		else
		{
			
		}			
		if( (const_adc_Slope > 2000) && (const_adc_Slope < 2400) )
		{
					//使用成测数据	
					//ADC值换算为电压值
					if(fADCData <= 30)
						fVolt = 0;
					else if(fADCData <= 50)
						fVolt = (fADCData-30)*5;
					else if(fADCData <= 150)
						fVolt = fADCData*const_adc_Slope/1000;
					else if(fADCData <= 500)
						fVolt = (fADCData*98*const_adc_Slope/100000);
					else
						fVolt = fADCData*const_adc_Slope/1000+const_adc_Offset/100;	
		}
		else
		{
			//调试临时使用
			fVolt = (fADCData - 10)*2174/1000;
		}	
	}
  if(Vdd==3)
  {
    fVolt=(fVolt*102/100);
  }
	
	return fVolt;
}

/*ADC 温度计算函数
	输入：AD值	 float型（方便处理平均值）
	输入：电源电压 uint08型 仅支持3/5V，用于做电压补偿
	输出：温度@℃ float型
*/
/*********************************
ADC温度计算函数 
功能：计算ADC测量温度值
输入: ADC值、电源电压
输出: 温度值
*********************************/
float ANAC_ADC_TemperatureCalc(float fADCData ,uint8_t Vdd)
{
	float fTestADC, fTestT, fFa;
	float fTemperature;
		
	if( const_temp_TestA == 0x1E )
	{
		//使用温度定标数据
		fTestT = const_temp_TestA+(float)(const_temp_TestB/0x80)*0.5;
		if( Vdd == 3 )
		{
			fTestADC = const_adc_Test  - const_T_Offset3V*const_TmpsH;
		}
		else
		{			
			fTestADC = const_adc_Test  - const_T_Offset5V*const_TmpsH;
		}
	}
	else
	{
		//调试临时使用
		fTestT = 30;
		fTestADC = 1500;
	}
	
	if( fADCData > (fTestADC-25) )	//高温
		fFa = const_TmpsH;
	else
		fFa = const_TmpsL;

	fTemperature = (fADCData - fTestADC)/fFa + fTestT;
	
	return fTemperature;
}

/*ADC 温度计算函数
	输入：AD值	  uint32_t型(计算速度快)
	输入：电源电压 uint08型 仅支持3/5V，用于做电压补偿
	输出：温度@℃ uint32_t型
*/
/*********************************
ADC温度计算函数 
功能：计算ADC测量温度值
输入: ADC值、电源电压
输出: 温度值
*********************************/
int32_t ANAC_ADC_TemperatureCalcInt(uint32_t ADCData ,uint8_t Vdd)
{
	uint32_t TestADC;
	int32_t TestT,Fa;
	int32_t Temperature;
		
	if( const_temp_TestA == 0x1E )
	{
		//使用温度定标数据
		TestT = const_temp_TestA;
		if( Vdd == 3 )
		{
			TestADC = (int32_t)const_adc_Test  - (int32_t)const_T_Offset3V*(const_TmpsH*10000)/10000;
		}
		else
		{			
			TestADC = (int32_t)const_adc_Test  - (int32_t)const_T_Offset5V*(const_TmpsH*10000)/10000;
		}
	}
	else
	{
		//调试临时使用
		TestT = 30;
		TestADC = 1500;
	}
	
	if( ADCData > (TestADC-25) )	//高温
		Fa = const_TmpsH*10000;
	else
		Fa = const_TmpsL*10000;

	Temperature = ((int32_t)(ADCData - TestADC)*10000)/Fa + TestT;
	
	return Temperature;
}
/*********************************
比较器初始化配置函数 
功能：配置比较器参数
输入: ANAC_COMPx_InitTypeDef* para  比较器1或2 寄存器正极输入、负极输入、中断使能等参数
输出: 无
*********************************/
void ANAC_COMPx_Init(ANAC_COMPx_InitTypeDef* para)
{
	if(para->COMPx == 1)//比较器1
	{
		ANAC_COMP1CR_V1PSEL_Set(para->VxPSEL);		//比较器x正极输入选择
		ANAC_COMP1CR_V1NSEL_Set(para->VxNSEL);		//比较器x负极输入选择
		ANAC_COMPICR_CMP1SEL_Set(para->CMPxSEL);	//比较器x中断源选择
		ANAC_COMPICR_CMP1IE_Setable(para->CMPxIE);	//较器x中断使能
		ANAC_COMPICR_CMP1DF_Setable(para->CMPxDF);	//比较器x数字滤波使能
		ANAC_COMP1CR_CMP1EN_Setable(para->CMPxEN);	//比较器x使能
	}
	else		//比较器2
	{
		ANAC_COMP2CR_V2PSEL_Set(para->VxPSEL);		//比较器x正极输入选择
		ANAC_COMP2CR_V2NSEL_Set(para->VxNSEL);		//比较器x负极输入选择
		ANAC_COMPICR_CMP2SEL_Set(para->CMPxSEL);	//比较器x中断源选择
		ANAC_COMPICR_CMP2IE_Setable(para->CMPxIE);	//较器x中断使能
		ANAC_COMPICR_CMP2DF_Setable(para->CMPxDF);	//比较器x数字滤波使能
		ANAC_COMP2CR_CMP2EN_Setable(para->CMPxEN);	//比较器x使能	
	}
}


/******END OF FILE****/



