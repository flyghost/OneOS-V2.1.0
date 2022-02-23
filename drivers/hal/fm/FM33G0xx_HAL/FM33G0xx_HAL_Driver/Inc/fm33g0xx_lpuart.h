/**
******************************************************************************
* @file    fm33g0xx_lpuart.h
* @author  FM33g0xx Application Team
* @version V0.3.00G
* @date    08-31-2018
* @brief   This file contains all the functions prototypes for the UART firmware     
******************************************************************************
*/ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_LPUART_H
#define __FM33G0XX_LPUART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
#include "fm33G0xx_uart.h" 

/** @addtogroup FM33G0XX_StdPeriph_Driver
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/

typedef struct
{
	uint32_t				BaudRate;		//波特率
	UART_DataBitTypeDef		DataBit;		//数据位数
	UART_ParityBitTypeDef	ParityBit; 		//校验位
	UART_StopBitTypeDef		StopBit;		//停止位
	
}LPUART_SInitTypeDef;	

typedef struct
{
	uint32_t				LPUBAUD;	//波特率控制
	FunState				TXEN;		//发送使能
	FunState				RXEN;		//接收使能
	uint32_t				COMPARE;	//数据匹配寄存器
	uint32_t				MCTL;		//调制控制寄存器
	
	uint32_t				SL;			//停止位长度
	uint32_t				DL;			//数据长度
	FunState				PAREN;		//校验位使能
	uint32_t				PTYP;		//校验位类型
	
	uint32_t				RXEV;		//接收中断事件配置
	FunState				TCIE;		//发送完成中断使能
	FunState				TXIE;		//发送buffer空中断使能
	FunState				RXIE;		//接收中断使能
	FunState				TXPOL;		//数据发送极性取反使能
	FunState				RXPOL;		//数据接收极性取反控制
	
	FunState				NEDET;		//下降沿检测使能
	FunState				ERRIE;		//错误中断使能
	
}LPUART_InitTypeDef;	


#define LPUART_MCTL_FOR9600BPS 0x00000952
#define LPUART_MCTL_FOR4800BPS 0x00000EFB
#define LPUART_MCTL_FOR2400BPS 0x000006DB
#define LPUART_MCTL_FOR1200BPS 0x00000492
#define LPUART_MCTL_FOR600BPS  0x000006D6
#define LPUART_MCTL_FOR300BPS  0x00000842



#define	LPUART_LPURXD_LPURXD_Pos	0	/* 接收数据寄存器 */
#define	LPUART_LPURXD_LPURXD_Msk	(0xffU << LPUART_LPURXD_LPURXD_Pos)

#define	LPUART_LPUTXD_LPUTXD_Pos	0	/* 发送数据寄存器 */
#define	LPUART_LPUTXD_LPUTXD_Msk	(0xffU << LPUART_LPUTXD_LPUTXD_Pos)

#define	LPUART_LPUSTA_TC_Pos	7	/* 发送完成标志 */
#define	LPUART_LPUSTA_TC_Msk	(0x1U << LPUART_LPUSTA_TC_Pos)

#define	LPUART_LPUSTA_TXE_Pos	6	/* 发送buffer空标志 */
#define	LPUART_LPUSTA_TXE_Msk	(0x1U << LPUART_LPUSTA_TXE_Pos)

#define	LPUART_LPUSTA_START_Pos	5	/* 起始位检测标志 */
#define	LPUART_LPUSTA_START_Msk	(0x1U << LPUART_LPUSTA_START_Pos)

#define	LPUART_LPUSTA_PERR_Pos	4	/* 校验位错误标志 */
#define	LPUART_LPUSTA_PERR_Msk	(0x1U << LPUART_LPUSTA_PERR_Pos)

#define	LPUART_LPUSTA_FERR_Pos	3	/* 帧格式错误标志 */
#define	LPUART_LPUSTA_FERR_Msk	(0x1U << LPUART_LPUSTA_FERR_Pos)

#define	LPUART_LPUSTA_RXOV_Pos	2	/* 接收缓冲溢出标志 */
#define	LPUART_LPUSTA_RXOV_Msk	(0x1U << LPUART_LPUSTA_RXOV_Pos)

#define	LPUART_LPUSTA_RXF_Pos	1	/* 接收缓冲满标志 */
#define	LPUART_LPUSTA_RXF_Msk	(0x1U << LPUART_LPUSTA_RXF_Pos)

#define	LPUART_LPUSTA_MATCH_Pos	0	/* 数据匹配标志 */
#define	LPUART_LPUSTA_MATCH_Msk	(0x1U << LPUART_LPUSTA_MATCH_Pos)

#define	LPUART_LPUCON_TXPOL_Pos	12	/* 数据发送极性取反使能 */
#define	LPUART_LPUCON_TXPOL_Msk	(0x1U << LPUART_LPUCON_TXPOL_Pos)
	/* 0不取反 */
	/* 1取反 */

#define	LPUART_LPUCON_TCIE_Pos	11	/* 发送完成中断使能 */
#define	LPUART_LPUCON_TCIE_Msk	(0x1U << LPUART_LPUCON_TCIE_Pos)
	/* 0：禁止发送完成中断 */
	/* 1：允许发送完成中断 */

#define	LPUART_LPUCON_TXIE_Pos	10	/* 发送buffer空中断使能 */
#define	LPUART_LPUCON_TXIE_Msk	(0x1U << LPUART_LPUCON_TXIE_Pos)
	/* 0：禁止发送buffer空中断 */
	/* 1：允许发送buffer空中断 */

#define	LPUART_LPUCON_NEDET_Pos	9	/* 下降沿检测使能 */
#define	LPUART_LPUCON_NEDET_Msk	(0x1U << LPUART_LPUCON_NEDET_Pos)
	/* 0：禁止RXD下降沿检测 */
	/* 1：使能RXD下降沿检测 */

#define	LPUART_LPUCON_PAREN_Pos	8	/* 校验位使能 */
#define	LPUART_LPUCON_PAREN_Msk	(0x1U << LPUART_LPUCON_PAREN_Pos)
	/* 0：数据帧无奇偶校验位 */
	/* 1：数据帧有奇偶校验位 */

#define	LPUART_LPUCON_PTYP_Pos	7	/* 校验位类型 */
#define	LPUART_LPUCON_PTYP_Msk	(0x1U << LPUART_LPUCON_PTYP_Pos)
#define	LPUART_LPUCON_PTYP_EVEN	(0x0U << LPUART_LPUCON_PTYP_Pos)	/* 0：偶校验 */
#define	LPUART_LPUCON_PTYP_ODD	(0x1U << LPUART_LPUCON_PTYP_Pos)	/* 1：奇校验 */

#define	LPUART_LPUCON_SL_Pos	6	/* 停止位长度 */
#define	LPUART_LPUCON_SL_Msk	(0x1U << LPUART_LPUCON_SL_Pos)
#define	LPUART_LPUCON_SL_1BIT	(0x0U << LPUART_LPUCON_SL_Pos)	/* 0：1bit */
#define	LPUART_LPUCON_SL_2BIT	(0x1U << LPUART_LPUCON_SL_Pos)	/* 1：2bits */

#define	LPUART_LPUCON_DL_Pos	5	/* 数据长度 */
#define	LPUART_LPUCON_DL_Msk	(0x1U << LPUART_LPUCON_DL_Pos)
#define	LPUART_LPUCON_DL_8BIT	(0x0U << LPUART_LPUCON_DL_Pos)	/* 0：8bits */
#define	LPUART_LPUCON_DL_7BIT	(0x1U << LPUART_LPUCON_DL_Pos)	/* 1：7bits */

#define	LPUART_LPUCON_RXPOL_Pos	4	/* 数据接收极性取反控制 */
#define	LPUART_LPUCON_RXPOL_Msk	(0x1U << LPUART_LPUCON_RXPOL_Pos)
	/* 0：非取反 */
	/* 1：取反 */

#define	LPUART_LPUCON_ERRIE_Pos	3	/* 错误中断使能 */
#define	LPUART_LPUCON_ERRIE_Msk	(0x1U << LPUART_LPUCON_ERRIE_Pos)
	/* 0：禁止接收错误中断 */
	/* 1：允许接收错误中断 */

#define	LPUART_LPUCON_RXIE_Pos	2	/* 接收中断使能 */
#define	LPUART_LPUCON_RXIE_Msk	(0x1U << LPUART_LPUCON_RXIE_Pos)
	/* 0：禁止接收中断 */
	/* 1：允许接收中断 */

#define	LPUART_LPUCON_RXEV_Pos	0	/* 接收中断事件配置，用于控制何种事件下向CPU提供接收中断 */
#define	LPUART_LPUCON_RXEV_Msk	(0x3U << LPUART_LPUCON_RXEV_Pos)
#define	LPUART_LPUCON_RXEV_STARTBIT	(0x0U << LPUART_LPUCON_RXEV_Pos)	/* 00：START位检测唤醒 */
#define	LPUART_LPUCON_RXEV_1BYTE	(0x1U << LPUART_LPUCON_RXEV_Pos)	/* 01：1byte数据接收完成 */
#define	LPUART_LPUCON_RXEV_MATCH	(0x2U << LPUART_LPUCON_RXEV_Pos)	/* 10/11：接收数据匹配成功 */

#define	LPUART_LPUIF_TC_IF_Pos	3	/* 发送完成中断标志 */
#define	LPUART_LPUIF_TC_IF_Msk	(0x1U << LPUART_LPUIF_TC_IF_Pos)

#define	LPUART_LPUIF_TXIF_Pos	2	/* 发送buffer空中断标志 */
#define	LPUART_LPUIF_TXIF_Msk	(0x1U << LPUART_LPUIF_TXIF_Pos)

#define	LPUART_LPUIF_RXNEGIF_Pos	1	/* RXD下降沿中断标志 */
#define	LPUART_LPUIF_RXNEGIF_Msk	(0x1U << LPUART_LPUIF_RXNEGIF_Pos)

#define	LPUART_LPUIF_RXIF_Pos	0	/* 接收完成中断标志 */
#define	LPUART_LPUIF_RXIF_Msk	(0x1U << LPUART_LPUIF_RXIF_Pos)

#define	LPUART_LPUBAUD_BAUD_Pos	0	/* 波特率控制 */
#define	LPUART_LPUBAUD_BAUD_Msk	(0x7U << LPUART_LPUBAUD_BAUD_Pos)
#define	LPUART_LPUBAUD_BAUD_9600BPS	(0x0U << LPUART_LPUBAUD_BAUD_Pos)	/* 000：9600 */
#define	LPUART_LPUBAUD_BAUD_4800BPS	(0x1U << LPUART_LPUBAUD_BAUD_Pos)	/* 001：4800 */
#define	LPUART_LPUBAUD_BAUD_2400BPS	(0x2U << LPUART_LPUBAUD_BAUD_Pos)	/* 010：2400 */
#define	LPUART_LPUBAUD_BAUD_1200BPS	(0x3U << LPUART_LPUBAUD_BAUD_Pos)	/* 011：1200 */
#define	LPUART_LPUBAUD_BAUD_600BPS	(0x4U << LPUART_LPUBAUD_BAUD_Pos)	/* 100：600 */
#define	LPUART_LPUBAUD_BAUD_300BPS	(0x5U << LPUART_LPUBAUD_BAUD_Pos)	/* 101/110/111：300 */

#define	LPUART_LPUEN_TXEN_Pos	1	/* 发送使能 */
#define	LPUART_LPUEN_TXEN_Msk	(0x1U << LPUART_LPUEN_TXEN_Pos)
	/* 0：关闭LPUART发送 */
	/* 1：打开LPUART发送 */

#define	LPUART_LPUEN_RXEN_Pos	0	/* 接收使能 */
#define	LPUART_LPUEN_RXEN_Msk	(0x1U << LPUART_LPUEN_RXEN_Pos)
	/* 0：关闭LPUART接收 */
	/* 1：打开LPUART接收 */

#define	LPUART_COMPARE_COMPARE_Pos	0	/* 数据匹配寄存器 */
#define	LPUART_COMPARE_COMPARE_Msk	(0xffU << LPUART_COMPARE_COMPARE_Pos)

#define	LPUART_MCTL_MCTL_Pos	0	/* 调制控制寄存器 */
#define	LPUART_MCTL_MCTL_Msk	(0xfffU << LPUART_MCTL_MCTL_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void LPUART_Deinit(void);

/* 接收数据寄存器 相关函数 */
extern void LPUART_LPURXD_Write(uint32_t SetValue);
extern uint32_t LPUART_LPURXD_Read(void);

/* 发送数据寄存器 相关函数 */
extern void LPUART_LPUTXD_Write(uint32_t SetValue);
extern uint32_t LPUART_LPUTXD_Read(void);

/* 发送完成标志 相关函数 */
extern FlagStatus LPUART_LPUSTA_TC_Chk(void);

/* 发送buffer空标志 相关函数 */
extern FlagStatus LPUART_LPUSTA_TXE_Chk(void);

/* 起始位检测标志 相关函数 */
extern void LPUART_LPUSTA_START_Clr(void);
extern FlagStatus LPUART_LPUSTA_START_Chk(void);

/* 校验位错误标志 相关函数 */
extern void LPUART_LPUSTA_PERR_Clr(void);
extern FlagStatus LPUART_LPUSTA_PERR_Chk(void);

/* 帧格式错误标志 相关函数 */
extern void LPUART_LPUSTA_FERR_Clr(void);
extern FlagStatus LPUART_LPUSTA_FERR_Chk(void);

/* 接收缓冲溢出标志 相关函数 */
extern void LPUART_LPUSTA_RXOV_Clr(void);
extern FlagStatus LPUART_LPUSTA_RXOV_Chk(void);

/* 接收缓冲满标志 相关函数 */
extern void LPUART_LPUSTA_RXF_Clr(void);
extern FlagStatus LPUART_LPUSTA_RXF_Chk(void);

/* 数据匹配标志 相关函数 */
extern void LPUART_LPUSTA_MATCH_Clr(void);
extern FlagStatus LPUART_LPUSTA_MATCH_Chk(void);

/* 数据发送极性取反使能 相关函数 */
extern void LPUART_LPUCON_TXPOL_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TXPOL_Getable(void);

/* 发送完成中断使能 相关函数 */
extern void LPUART_LPUCON_TCIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TCIE_Getable(void);

/* 发送buffer空中断使能 相关函数 */
extern void LPUART_LPUCON_TXIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TXIE_Getable(void);

/* 下降沿检测使能 相关函数 */
extern void LPUART_LPUCON_NEDET_Setable(FunState NewState);
extern FunState LPUART_LPUCON_NEDET_Getable(void);

/* 校验位使能 相关函数 */
extern void LPUART_LPUCON_PAREN_Setable(FunState NewState);
extern FunState LPUART_LPUCON_PAREN_Getable(void);

/* 校验位类型 相关函数 */
extern void LPUART_LPUCON_PTYP_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_PTYP_Get(void);

/* 停止位长度 相关函数 */
extern void LPUART_LPUCON_SL_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_SL_Get(void);

/* 数据长度 相关函数 */
extern void LPUART_LPUCON_DL_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_DL_Get(void);

/* 数据接收极性取反控制 相关函数 */
extern void LPUART_LPUCON_RXPOL_Setable(FunState NewState);
extern FunState LPUART_LPUCON_RXPOL_Getable(void);

/* 错误中断使能 相关函数 */
extern void LPUART_LPUCON_ERRIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_ERRIE_Getable(void);

/* 接收中断使能 相关函数 */
extern void LPUART_LPUCON_RXIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_RXIE_Getable(void);

/* 接收中断事件配置，用于控制何种事件下向CPU提供接收中断 相关函数 */
extern void LPUART_LPUCON_RXEV_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_RXEV_Get(void);

/* 发送完成中断标志 相关函数 */
extern void LPUART_LPUIF_TC_IF_Clr(void);
extern FlagStatus LPUART_LPUIF_TC_IF_Chk(void);

/* 发送buffer空中断标志 相关函数 */
extern void LPUART_LPUIF_TXIF_Clr(void);
extern FlagStatus LPUART_LPUIF_TXIF_Chk(void);

/* RXD下降沿中断标志 相关函数 */
extern void LPUART_LPUIF_RXNEGIF_Clr(void);
extern FlagStatus LPUART_LPUIF_RXNEGIF_Chk(void);

/* 接收完成中断标志 相关函数 */
extern void LPUART_LPUIF_RXIF_Clr(void);
extern FlagStatus LPUART_LPUIF_RXIF_Chk(void);

/* 波特率控制 相关函数 */
extern void LPUART_LPUBAUD_BAUD_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUBAUD_BAUD_Get(void);

/* 发送使能 相关函数 */
extern void LPUART_LPUEN_TXEN_Setable(FunState NewState);
extern FunState LPUART_LPUEN_TXEN_Getable(void);

/* 接收使能 相关函数 */
extern void LPUART_LPUEN_RXEN_Setable(FunState NewState);
extern FunState LPUART_LPUEN_RXEN_Getable(void);

/* 数据匹配寄存器 相关函数 */
extern void LPUART_COMPARE_Write(uint32_t SetValue);
extern uint32_t LPUART_COMPARE_Read(void);

/* 调制控制寄存器 相关函数 */
extern void LPUART_MCTL_Write(uint32_t SetValue);
extern uint32_t LPUART_MCTL_Read(void);
//Announce_End


/* LPUART完整参数初始化函数 */
extern void LPUART_Init(LPUART_InitTypeDef* para);

/* LPUART简单参数初始化函数 */
extern void LPUART_SInit(LPUART_SInitTypeDef* para);


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_LPUART_H */

