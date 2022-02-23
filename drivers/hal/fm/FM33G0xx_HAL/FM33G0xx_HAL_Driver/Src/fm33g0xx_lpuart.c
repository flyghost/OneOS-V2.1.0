/**
  ******************************************************************************
  * @file    fm33g0xx_lpuart.c
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Universal synchronous asynchronous receiver
  *          transmitter (UART):           
  *           
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "fm33g0xx_lpuart.h" 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*********************************
LPUART 接收数据函数
功能：读LPUART接收寄存器
输入: 无    
输出: 接收的数据
*********************************/
uint32_t LPUART_LPURXD_Read(void)
{
	return (LPUART->LPURXD);
}

/*********************************
LPUART 发送数据函数
功能：写LPUART发送寄存器
输入: 发送的数据
输出: 无
*********************************/
void LPUART_LPUTXD_Write(uint32_t SetValue)
{
	LPUART->LPUTXD = SetValue;
}


/*********************************
LPUART 发送完成标志检测函数
功能：检测发送完成标志是否置位（不会立即置位，不建议使用TC作为发送完成检测标志）
输入: 无
输出: 0、1
*********************************/
FlagStatus LPUART_LPUSTA_TC_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_TC_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART 发送buff空标志位检测函数
功能：检测发送buff是否为空
输入: 无
输出:  0: 有数据
       1:空
*********************************/
FlagStatus LPUART_LPUSTA_TXE_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_TXE_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART 起始位清0函数
功能：将标志位清 0
输入: 无
输出:  无
*********************************/
void LPUART_LPUSTA_START_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_START_Msk;
}

/*********************************
LPUART 起始位检测函数
功能：检测是否接收到起始位
输入: 无
输出:  0: 没检测到
       1:  检测到起始位
*********************************/
FlagStatus LPUART_LPUSTA_START_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_START_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART 校验位错误标志清0函数
功能：将校验位错误清0
输入: 无
输出:  无
*********************************/
void LPUART_LPUSTA_PERR_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_PERR_Msk;
}

/*********************************
LPUART  校验位错误检测函数
功能：检测校验错标志是否置位
输入: 无
输出:  0: 没校验错
       1:  有校验错
*********************************/
FlagStatus LPUART_LPUSTA_PERR_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_PERR_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  帧格式错误标志清0函数
功能：将帧格式错误清0
输入: 无
输出:  无
*********************************/
void LPUART_LPUSTA_FERR_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_FERR_Msk;
}

/*********************************
LPUART  帧格式错误标志位检测函数
功能：检测帧格式错误标志是否置位
输入: 无
输出:  0: 没有帧格式错
       1:  有帧格式错
*********************************/
FlagStatus LPUART_LPUSTA_FERR_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_FERR_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  接收缓冲溢出标志位清0函数
功能：将接收缓冲溢出错误清0
输入: 无
输出:  无
*********************************/
void LPUART_LPUSTA_RXOV_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_RXOV_Msk;
}

/*********************************
LPUART  接收缓冲溢出标志位检测函数
功能: 检测接收缓冲溢出标志位是否置位
输入: 无
输出:  0: 没有接收缓冲溢出
       1:  有接收缓冲溢出
*********************************/
FlagStatus LPUART_LPUSTA_RXOV_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_RXOV_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  接收缓冲溢出标志位清0函数
功能:  接收缓冲溢出标志清0
输入: 无
输出:  无
*********************************/

void LPUART_LPUSTA_RXF_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_RXF_Msk;
}

/*********************************
LPUART  接收缓冲满标志位检测函数
功能: 检测接收缓冲满标志位是否置位
输入: 无
输出:  0: 接收缓冲没满
       1:  接收缓冲满
*********************************/
FlagStatus LPUART_LPUSTA_RXF_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_RXF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART 接收数据匹配标志清0函数
输入: 无
输出:  无
*********************************/
void LPUART_LPUSTA_MATCH_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_MATCH_Msk;
}

/*********************************
LPUART 接收数据匹配标志位检测函数
输入: 无
输出:  1：匹配
       0：不匹配
*********************************/
FlagStatus LPUART_LPUSTA_MATCH_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_MATCH_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART 数据发送极性取反使能函数
输入: 0：不取反（默认）
      1：取反
输出:  无
*********************************/
void LPUART_LPUCON_TXPOL_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TXPOL_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TXPOL_Msk);
	}
}

/*********************************
LPUART 数据发送极性取反状态检测函数
输入: 无
输出:  0：不取反
       1：取反
*********************************/
FunState LPUART_LPUCON_TXPOL_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TXPOL_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART 发送完成中断使能函数
输入:   0：禁止
        1：使能
输出:   无
*********************************/
void LPUART_LPUCON_TCIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TCIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TCIE_Msk);
	}
}

/*********************************
LPUART 发送完成中断使能状态获取函数
输入:   无
输出:   0:禁止
        1：使能
*********************************/
FunState LPUART_LPUCON_TCIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TCIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  发送buffer空中断使能函数
输入:   0：禁止
        1：使能
输出:   无
*********************************/
void LPUART_LPUCON_TXIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TXIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TXIE_Msk);
	}
}

/*********************************
LPUART  发送buffer空中断使能状态获取函数
输入:   无
输出:   0：禁止
        1：使能
*********************************/
FunState LPUART_LPUCON_TXIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TXIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  下降沿采样使能32K函数
输入:   0：禁止
        1：使能
输出:   无
*********************************/
void LPUART_LPUCON_NEDET_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_NEDET_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_NEDET_Msk);
	}
}
/*********************************
LPUART  下降沿采样使能32K状态获取函数
输入:  无
输出:   0：禁止
        1：使能
*********************************/
FunState LPUART_LPUCON_NEDET_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_NEDET_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  设置数据帧有无奇偶校验位函数
输入:   0：没有
        1：有
输出:   无
*********************************/
void LPUART_LPUCON_PAREN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_PAREN_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_PAREN_Msk);
	}
}

/*********************************
LPUART  数据帧设置奇偶校验状态获取
输入:   无
输出:   0：无奇偶校验
        1：有奇偶校验
*********************************/
FunState LPUART_LPUCON_PAREN_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_PAREN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  设置数据帧奇偶校验函数
输入:   LPUART_LPUCON_PTYP_EVEN：偶校验
        LPUART_LPUCON_PTYP_ODD：奇校验
输出:   无
*********************************/
void LPUART_LPUCON_PTYP_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_PTYP_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART  数据帧奇偶校验状态获取
输入:   无
输出:   LPUART_LPUCON_PTYP_EVEN：偶校验
        LPUART_LPUCON_PTYP_ODD：奇校验
*********************************/
uint32_t LPUART_LPUCON_PTYP_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_PTYP_Msk);
}

/*********************************
LPUART   停止位长度设置函数
输入:    LPUART_LPUCON_SL_1BIT：1bit
         LPUART_LPUCON_SL_2BIT: 2bit
输出:   无
*********************************/
void LPUART_LPUCON_SL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_SL_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART   停止位长度设置状态获取函数
输入:    无
输出:    LPUART_LPUCON_SL_1BIT：1bit
         LPUART_LPUCON_SL_2BIT: 2bit
*********************************/
uint32_t LPUART_LPUCON_SL_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_SL_Msk);
}

/*********************************
LPUART   数据长度设置函数
输入:    LPUART_LPUCON_DL_8BIT：8bit
         LPUART_LPUCON_DL_7BIT: 7bit
输出:    无
*********************************/
void LPUART_LPUCON_DL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_DL_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART   数据长度设置状态获取函数
输入:    无
输出:    LPUART_LPUCON_DL_8BIT：8bit
         LPUART_LPUCON_DL_7BIT: 7bit
*********************************/
uint32_t LPUART_LPUCON_DL_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_DL_Msk);
}

/*********************************
LPUART   接收极性取反设置函数
输入:    0：不取反
         1：取反
输出:    无
*********************************/
void LPUART_LPUCON_RXPOL_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_RXPOL_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_RXPOL_Msk);
	}
}

/*********************************
LPUART   接收极性取反设置状态获取函数
输入:    无
输出:    0：不取反
         1：取反
*********************************/
FunState LPUART_LPUCON_RXPOL_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_RXPOL_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART   错误中断使能函数
输入:    0：禁止
         1：使能
输出:    无
*********************************/
void LPUART_LPUCON_ERRIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_ERRIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_ERRIE_Msk);
	}
}

/*********************************
LPUART   错误中断使能状态获取函数
输入:    无
输出:    0：禁止
         1：使能
*********************************/
FunState LPUART_LPUCON_ERRIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_ERRIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART   接收中断使能函数
输入:    0：禁止
         1：使能
输出:    无
*********************************/
void LPUART_LPUCON_RXIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_RXIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_RXIE_Msk);
	}
}

/*********************************
LPUART   接收中断使能状态获取
输入:    无
输出:    0：禁止
         1：使能
*********************************/
FunState LPUART_LPUCON_RXIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_RXIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  接收中断事件配置函数（用于控制何种事件下向CPU提供接收中断 ）
输入:    00：START位检测
         01：1byte数据接收完成
         10：接收数据匹配
         11：RXD下降沿唤醒
输出：无
*********************************/
void LPUART_LPUCON_RXEV_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_RXEV_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART  接收中断事件配置函数状态获取
输入:    无
输出：   00：START位检测
         01：1byte数据接收完成
         10：接收数据匹配
         11：RXD下降沿唤醒
*********************************/
uint32_t LPUART_LPUCON_RXEV_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_RXEV_Msk);
}

/*********************************
LPUART  发送完成中断标志清0函数
输入:    无
输出：   无
*********************************/
void LPUART_LPUIF_TC_IF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_TC_IF_Msk;
}

/*********************************
LPUART  发送完成中断标志位状态检测函数
输入:    无
输出：   0：无中断
         1：发送完一帧数据后产生中断
*********************************/
FlagStatus LPUART_LPUIF_TC_IF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_TC_IF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  发送buffer空中断标志位清0函数
输入:    无
输出：   无
*********************************/
void LPUART_LPUIF_TXIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_TXIF_Msk;
}

/*********************************
LPUART  发送buffer空中断标志位检测函数
输入:    无
输出：   0：无中断
         1：发送buffer空后产生中断
*********************************/
FlagStatus LPUART_LPUIF_TXIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_TXIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  RXD下降沿中断标志位清0函数
输入:    无
输出：   无
*********************************/
void LPUART_LPUIF_RXNEGIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_RXNEGIF_Msk;
}

/*********************************
LPUART  RXD下降沿中断标志位状态检测函数
输入:   无
输出：  0：无中断产生
        1：中断产生
*********************************/
FlagStatus LPUART_LPUIF_RXNEGIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_RXNEGIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  接收完成中断标志位清0函数
输入:   无
输出：  无
*********************************/
void LPUART_LPUIF_RXIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_RXIF_Msk;
}

/*********************************
LPUART  接收完成中断标志位检测函数
输入:   无
输出：  0：无中断产生
        1：接收完一帧数据后产生中断
*********************************/
FlagStatus LPUART_LPUIF_RXIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_RXIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  波特率设置函数
输入:   000：9600
        001：4800
        010：2400
        011：1200
        100：600
        101/110/111：300
输出：  无
*********************************/
void LPUART_LPUBAUD_BAUD_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUBAUD;
	tmpreg &= ~(LPUART_LPUBAUD_BAUD_Msk);
	tmpreg |= SetValue;
	LPUART->LPUBAUD = tmpreg;
}

/*********************************
LPUART  波特率设置状态获取
输入:   无
输出：  000：9600
        001：4800
        010：2400
        011：1200
        100：600
        101/110/111：300
*********************************/
uint32_t LPUART_LPUBAUD_BAUD_Get(void)
{
	return (LPUART->LPUBAUD & LPUART_LPUBAUD_BAUD_Msk);
}

/*********************************
LPUART  发送使能设置函数
输入:   0：关闭发送
        1：打开发送
输出：  无
*********************************/
void LPUART_LPUEN_TXEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUEN |= (LPUART_LPUEN_TXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_TXEN_Msk)==0); //寄存器与模块不同步，必须确认已经写进寄存器，防止写其他bit时覆盖掉
	}
	else
	{
		LPUART->LPUEN &= ~(LPUART_LPUEN_TXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_TXEN_Msk)!=0);
	}
}

/*********************************
LPUART  发送使能设置状态获取函数
输入:   无
输出：  0：关闭发送
        1：打开发送
*********************************/
FunState LPUART_LPUEN_TXEN_Getable(void)
{
	if (LPUART->LPUEN & (LPUART_LPUEN_TXEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  接收使能设置函数
输入:   0：关闭接收
        1：打开接收
输出：  无
*********************************/
void LPUART_LPUEN_RXEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUEN |= (LPUART_LPUEN_RXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_RXEN_Msk)==0);//寄存器与模块不同步，必须确认已经写进寄存器，防止写其他bit时覆盖掉
	}
	else
	{
		LPUART->LPUEN &= ~(LPUART_LPUEN_RXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_RXEN_Msk)!=0);
	}
}

/*********************************
LPUART  接收使能设置状态获取函数
输入:   无
输出：  0：关闭接收
        1：打开接收
*********************************/
FunState LPUART_LPUEN_RXEN_Getable(void)
{
	if (LPUART->LPUEN & (LPUART_LPUEN_RXEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  数据匹配设置函数（需要RXEV设置为10才有效）
输入:   匹配的数据
输出：  无
*********************************/
void LPUART_COMPARE_Write(uint32_t SetValue)
{
	LPUART->COMPARE = SetValue;
}

/*********************************
LPUART  数据匹配设置数据函数
输入:   无
输出：  匹配的数据
*********************************/
uint32_t LPUART_COMPARE_Read(void)
{
	return (LPUART->COMPARE);
}

/*********************************
LPUART  每个bit的调制控制信号函数
输入:   12bit数据
        波特率9600：LPUART_MCTL_FOR9600BPS
        波特率4800：LPUART_MCTL_FOR4800BPS
        波特率2400：LPUART_MCTL_FOR2400BPS
        波特率1200：LPUART_MCTL_FOR1200BPS
        波特率600：LPUART_MCTL_FOR600BPS
        波特率300：LPUART_MCTL_FOR300BPS
输出：  无
*********************************/
void LPUART_MCTL_Write(uint32_t SetValue)
{
	LPUART->MCTL = SetValue;
}

/*********************************
LPUART  每个bit的调制控制信号读取函数
输入:   无
输出：  12bit数据
*********************************/
uint32_t LPUART_MCTL_Read(void)
{
	return (LPUART->MCTL);
}

/*********************************
LPUART  寄存器初始化
输入:   无
输出：  无
*********************************/
void LPUART_Deinit(void)
{
	//LPUART->LPURXD = ;
	//LPUART->LPUTXD = ;
	//LPUART->LPUSTA = 0x000000C0;
	LPUART->LPUCON = 0x00000000;
	//LPUART->LPUIF = ;
	LPUART->LPUBAUD = 0x00000000;
	LPUART->LPUEN = 0x00000000;
	LPUART->COMPARE = 0x00000000;
	LPUART->MCTL = 0x00000000;
}
//Code_End

/*********************************
LPUART  完整参数初始化函数 
输入:  
输出：  
*********************************/
void LPUART_Init(LPUART_InitTypeDef* para)
{
	LPUART_LPUBAUD_BAUD_Set(para->LPUBAUD);		//波特率控制
	LPUART_LPUEN_TXEN_Setable(para->TXEN);		//发送使能
	LPUART_LPUEN_RXEN_Setable(para->RXEN);		//接收使能
	LPUART_COMPARE_Write(para->COMPARE);		//数据匹配寄存器
	LPUART_MCTL_Write(para->MCTL);				//调制控制寄存器
	
	LPUART_LPUCON_SL_Set(para->SL);				//停止位长度
	LPUART_LPUCON_DL_Set(para->DL);				//数据长度
	LPUART_LPUCON_PAREN_Setable(para->PAREN);	//校验位使能
	LPUART_LPUCON_PTYP_Set(para->PTYP);			//校验位类型
	
	LPUART_LPUCON_RXEV_Set(para->RXEV);		//接收中断事件配置
	LPUART_LPUCON_TCIE_Setable(para->TCIE);		//发送完成中断使能
	LPUART_LPUCON_TXIE_Setable(para->TXIE);		//发送buffer空中断使能
	LPUART_LPUCON_RXIE_Setable(para->RXIE);		//接收中断使能
	LPUART_LPUCON_TXPOL_Setable(para->TXPOL);		//数据发送极性取反使能
	LPUART_LPUCON_RXPOL_Setable(para->RXPOL);		//数据接收极性取反控制
	
	LPUART_LPUCON_NEDET_Setable(para->NEDET);		//下降沿检测使能
	LPUART_LPUCON_ERRIE_Setable(para->ERRIE);		//错误中断使能
}

/*********************************
LPUART 简单参数初始化函数 
输入:  
输出：  
*********************************/
void LPUART_SInit(LPUART_SInitTypeDef* para)
{
	LPUART_InitTypeDef para2;	
		
	switch(para->BaudRate)
	{
		case 9600:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_9600BPS;
			para2.MCTL = LPUART_MCTL_FOR9600BPS;
			break;

		case 4800:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_4800BPS;
			para2.MCTL = LPUART_MCTL_FOR4800BPS;
			break;

		case 2400:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_2400BPS;
			para2.MCTL = LPUART_MCTL_FOR2400BPS;
			break;
		
		case 1200:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_1200BPS;
			para2.MCTL = LPUART_MCTL_FOR1200BPS;
			break;
		
		case 600:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_600BPS;
			para2.MCTL = LPUART_MCTL_FOR600BPS;
			break;
		
		case 300:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_300BPS;
			para2.MCTL = LPUART_MCTL_FOR300BPS;
			break;
		
		default:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_9600BPS;
			para2.MCTL = LPUART_MCTL_FOR9600BPS;
			break;
	}

	para2.TXEN = DISABLE;
	para2.RXEN = DISABLE;
	para2.COMPARE = 0;
	
	//停止位长度
	if(OneBit == para->StopBit)
	{
		para2.SL = LPUART_LPUCON_SL_1BIT;
	}
	else
	{
		para2.SL = LPUART_LPUCON_SL_2BIT;
	}
	//数据位长度
	if(Eight8Bit == para->DataBit)
	{
		para2.DL = LPUART_LPUCON_DL_8BIT;
	}
	else
	{
		para2.DL = LPUART_LPUCON_DL_7BIT;
	}
	
	//校验位
	if(NONE == para->ParityBit)
	{
		para2.PAREN = DISABLE;
		para2.PTYP = LPUART_LPUCON_PTYP_EVEN;
	}
	else
	{
		para2.PAREN = ENABLE;
		if(EVEN == para->ParityBit)
		{
			para2.PTYP = LPUART_LPUCON_PTYP_EVEN;
		}
		else
		{
			para2.PTYP = LPUART_LPUCON_PTYP_ODD;
		}
	}
	
	para2.RXEV =LPUART_LPUCON_RXEV_MATCH;		//接收中断事件配置
	para2.TCIE = DISABLE;		//发送完成中断使能
	para2.TXIE = DISABLE;		//发送buffer空中断使能
	para2.RXIE = DISABLE;		//接收中断使能
	para2.TXPOL = DISABLE;		//数据发送极性取反使能
	para2.RXPOL = DISABLE;		//数据接收极性取反控制
	para2.NEDET = DISABLE;		//下降沿检测使能
	para2.ERRIE = DISABLE;		//错误中断使能	
	
	LPUART_Init(&para2);
}

/******END OF FILE****/

