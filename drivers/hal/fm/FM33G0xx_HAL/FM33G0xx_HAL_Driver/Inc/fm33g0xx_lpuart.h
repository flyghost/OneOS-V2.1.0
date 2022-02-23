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
	uint32_t				BaudRate;		//������
	UART_DataBitTypeDef		DataBit;		//����λ��
	UART_ParityBitTypeDef	ParityBit; 		//У��λ
	UART_StopBitTypeDef		StopBit;		//ֹͣλ
	
}LPUART_SInitTypeDef;	

typedef struct
{
	uint32_t				LPUBAUD;	//�����ʿ���
	FunState				TXEN;		//����ʹ��
	FunState				RXEN;		//����ʹ��
	uint32_t				COMPARE;	//����ƥ��Ĵ���
	uint32_t				MCTL;		//���ƿ��ƼĴ���
	
	uint32_t				SL;			//ֹͣλ����
	uint32_t				DL;			//���ݳ���
	FunState				PAREN;		//У��λʹ��
	uint32_t				PTYP;		//У��λ����
	
	uint32_t				RXEV;		//�����ж��¼�����
	FunState				TCIE;		//��������ж�ʹ��
	FunState				TXIE;		//����buffer���ж�ʹ��
	FunState				RXIE;		//�����ж�ʹ��
	FunState				TXPOL;		//���ݷ��ͼ���ȡ��ʹ��
	FunState				RXPOL;		//���ݽ��ռ���ȡ������
	
	FunState				NEDET;		//�½��ؼ��ʹ��
	FunState				ERRIE;		//�����ж�ʹ��
	
}LPUART_InitTypeDef;	


#define LPUART_MCTL_FOR9600BPS 0x00000952
#define LPUART_MCTL_FOR4800BPS 0x00000EFB
#define LPUART_MCTL_FOR2400BPS 0x000006DB
#define LPUART_MCTL_FOR1200BPS 0x00000492
#define LPUART_MCTL_FOR600BPS  0x000006D6
#define LPUART_MCTL_FOR300BPS  0x00000842



#define	LPUART_LPURXD_LPURXD_Pos	0	/* �������ݼĴ��� */
#define	LPUART_LPURXD_LPURXD_Msk	(0xffU << LPUART_LPURXD_LPURXD_Pos)

#define	LPUART_LPUTXD_LPUTXD_Pos	0	/* �������ݼĴ��� */
#define	LPUART_LPUTXD_LPUTXD_Msk	(0xffU << LPUART_LPUTXD_LPUTXD_Pos)

#define	LPUART_LPUSTA_TC_Pos	7	/* ������ɱ�־ */
#define	LPUART_LPUSTA_TC_Msk	(0x1U << LPUART_LPUSTA_TC_Pos)

#define	LPUART_LPUSTA_TXE_Pos	6	/* ����buffer�ձ�־ */
#define	LPUART_LPUSTA_TXE_Msk	(0x1U << LPUART_LPUSTA_TXE_Pos)

#define	LPUART_LPUSTA_START_Pos	5	/* ��ʼλ����־ */
#define	LPUART_LPUSTA_START_Msk	(0x1U << LPUART_LPUSTA_START_Pos)

#define	LPUART_LPUSTA_PERR_Pos	4	/* У��λ�����־ */
#define	LPUART_LPUSTA_PERR_Msk	(0x1U << LPUART_LPUSTA_PERR_Pos)

#define	LPUART_LPUSTA_FERR_Pos	3	/* ֡��ʽ�����־ */
#define	LPUART_LPUSTA_FERR_Msk	(0x1U << LPUART_LPUSTA_FERR_Pos)

#define	LPUART_LPUSTA_RXOV_Pos	2	/* ���ջ��������־ */
#define	LPUART_LPUSTA_RXOV_Msk	(0x1U << LPUART_LPUSTA_RXOV_Pos)

#define	LPUART_LPUSTA_RXF_Pos	1	/* ���ջ�������־ */
#define	LPUART_LPUSTA_RXF_Msk	(0x1U << LPUART_LPUSTA_RXF_Pos)

#define	LPUART_LPUSTA_MATCH_Pos	0	/* ����ƥ���־ */
#define	LPUART_LPUSTA_MATCH_Msk	(0x1U << LPUART_LPUSTA_MATCH_Pos)

#define	LPUART_LPUCON_TXPOL_Pos	12	/* ���ݷ��ͼ���ȡ��ʹ�� */
#define	LPUART_LPUCON_TXPOL_Msk	(0x1U << LPUART_LPUCON_TXPOL_Pos)
	/* 0��ȡ�� */
	/* 1ȡ�� */

#define	LPUART_LPUCON_TCIE_Pos	11	/* ��������ж�ʹ�� */
#define	LPUART_LPUCON_TCIE_Msk	(0x1U << LPUART_LPUCON_TCIE_Pos)
	/* 0����ֹ��������ж� */
	/* 1������������ж� */

#define	LPUART_LPUCON_TXIE_Pos	10	/* ����buffer���ж�ʹ�� */
#define	LPUART_LPUCON_TXIE_Msk	(0x1U << LPUART_LPUCON_TXIE_Pos)
	/* 0����ֹ����buffer���ж� */
	/* 1��������buffer���ж� */

#define	LPUART_LPUCON_NEDET_Pos	9	/* �½��ؼ��ʹ�� */
#define	LPUART_LPUCON_NEDET_Msk	(0x1U << LPUART_LPUCON_NEDET_Pos)
	/* 0����ֹRXD�½��ؼ�� */
	/* 1��ʹ��RXD�½��ؼ�� */

#define	LPUART_LPUCON_PAREN_Pos	8	/* У��λʹ�� */
#define	LPUART_LPUCON_PAREN_Msk	(0x1U << LPUART_LPUCON_PAREN_Pos)
	/* 0������֡����żУ��λ */
	/* 1������֡����żУ��λ */

#define	LPUART_LPUCON_PTYP_Pos	7	/* У��λ���� */
#define	LPUART_LPUCON_PTYP_Msk	(0x1U << LPUART_LPUCON_PTYP_Pos)
#define	LPUART_LPUCON_PTYP_EVEN	(0x0U << LPUART_LPUCON_PTYP_Pos)	/* 0��żУ�� */
#define	LPUART_LPUCON_PTYP_ODD	(0x1U << LPUART_LPUCON_PTYP_Pos)	/* 1����У�� */

#define	LPUART_LPUCON_SL_Pos	6	/* ֹͣλ���� */
#define	LPUART_LPUCON_SL_Msk	(0x1U << LPUART_LPUCON_SL_Pos)
#define	LPUART_LPUCON_SL_1BIT	(0x0U << LPUART_LPUCON_SL_Pos)	/* 0��1bit */
#define	LPUART_LPUCON_SL_2BIT	(0x1U << LPUART_LPUCON_SL_Pos)	/* 1��2bits */

#define	LPUART_LPUCON_DL_Pos	5	/* ���ݳ��� */
#define	LPUART_LPUCON_DL_Msk	(0x1U << LPUART_LPUCON_DL_Pos)
#define	LPUART_LPUCON_DL_8BIT	(0x0U << LPUART_LPUCON_DL_Pos)	/* 0��8bits */
#define	LPUART_LPUCON_DL_7BIT	(0x1U << LPUART_LPUCON_DL_Pos)	/* 1��7bits */

#define	LPUART_LPUCON_RXPOL_Pos	4	/* ���ݽ��ռ���ȡ������ */
#define	LPUART_LPUCON_RXPOL_Msk	(0x1U << LPUART_LPUCON_RXPOL_Pos)
	/* 0����ȡ�� */
	/* 1��ȡ�� */

#define	LPUART_LPUCON_ERRIE_Pos	3	/* �����ж�ʹ�� */
#define	LPUART_LPUCON_ERRIE_Msk	(0x1U << LPUART_LPUCON_ERRIE_Pos)
	/* 0����ֹ���մ����ж� */
	/* 1��������մ����ж� */

#define	LPUART_LPUCON_RXIE_Pos	2	/* �����ж�ʹ�� */
#define	LPUART_LPUCON_RXIE_Msk	(0x1U << LPUART_LPUCON_RXIE_Pos)
	/* 0����ֹ�����ж� */
	/* 1����������ж� */

#define	LPUART_LPUCON_RXEV_Pos	0	/* �����ж��¼����ã����ڿ��ƺ����¼�����CPU�ṩ�����ж� */
#define	LPUART_LPUCON_RXEV_Msk	(0x3U << LPUART_LPUCON_RXEV_Pos)
#define	LPUART_LPUCON_RXEV_STARTBIT	(0x0U << LPUART_LPUCON_RXEV_Pos)	/* 00��STARTλ��⻽�� */
#define	LPUART_LPUCON_RXEV_1BYTE	(0x1U << LPUART_LPUCON_RXEV_Pos)	/* 01��1byte���ݽ������ */
#define	LPUART_LPUCON_RXEV_MATCH	(0x2U << LPUART_LPUCON_RXEV_Pos)	/* 10/11����������ƥ��ɹ� */

#define	LPUART_LPUIF_TC_IF_Pos	3	/* ��������жϱ�־ */
#define	LPUART_LPUIF_TC_IF_Msk	(0x1U << LPUART_LPUIF_TC_IF_Pos)

#define	LPUART_LPUIF_TXIF_Pos	2	/* ����buffer���жϱ�־ */
#define	LPUART_LPUIF_TXIF_Msk	(0x1U << LPUART_LPUIF_TXIF_Pos)

#define	LPUART_LPUIF_RXNEGIF_Pos	1	/* RXD�½����жϱ�־ */
#define	LPUART_LPUIF_RXNEGIF_Msk	(0x1U << LPUART_LPUIF_RXNEGIF_Pos)

#define	LPUART_LPUIF_RXIF_Pos	0	/* ��������жϱ�־ */
#define	LPUART_LPUIF_RXIF_Msk	(0x1U << LPUART_LPUIF_RXIF_Pos)

#define	LPUART_LPUBAUD_BAUD_Pos	0	/* �����ʿ��� */
#define	LPUART_LPUBAUD_BAUD_Msk	(0x7U << LPUART_LPUBAUD_BAUD_Pos)
#define	LPUART_LPUBAUD_BAUD_9600BPS	(0x0U << LPUART_LPUBAUD_BAUD_Pos)	/* 000��9600 */
#define	LPUART_LPUBAUD_BAUD_4800BPS	(0x1U << LPUART_LPUBAUD_BAUD_Pos)	/* 001��4800 */
#define	LPUART_LPUBAUD_BAUD_2400BPS	(0x2U << LPUART_LPUBAUD_BAUD_Pos)	/* 010��2400 */
#define	LPUART_LPUBAUD_BAUD_1200BPS	(0x3U << LPUART_LPUBAUD_BAUD_Pos)	/* 011��1200 */
#define	LPUART_LPUBAUD_BAUD_600BPS	(0x4U << LPUART_LPUBAUD_BAUD_Pos)	/* 100��600 */
#define	LPUART_LPUBAUD_BAUD_300BPS	(0x5U << LPUART_LPUBAUD_BAUD_Pos)	/* 101/110/111��300 */

#define	LPUART_LPUEN_TXEN_Pos	1	/* ����ʹ�� */
#define	LPUART_LPUEN_TXEN_Msk	(0x1U << LPUART_LPUEN_TXEN_Pos)
	/* 0���ر�LPUART���� */
	/* 1����LPUART���� */

#define	LPUART_LPUEN_RXEN_Pos	0	/* ����ʹ�� */
#define	LPUART_LPUEN_RXEN_Msk	(0x1U << LPUART_LPUEN_RXEN_Pos)
	/* 0���ر�LPUART���� */
	/* 1����LPUART���� */

#define	LPUART_COMPARE_COMPARE_Pos	0	/* ����ƥ��Ĵ��� */
#define	LPUART_COMPARE_COMPARE_Msk	(0xffU << LPUART_COMPARE_COMPARE_Pos)

#define	LPUART_MCTL_MCTL_Pos	0	/* ���ƿ��ƼĴ��� */
#define	LPUART_MCTL_MCTL_Msk	(0xfffU << LPUART_MCTL_MCTL_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void LPUART_Deinit(void);

/* �������ݼĴ��� ��غ��� */
extern void LPUART_LPURXD_Write(uint32_t SetValue);
extern uint32_t LPUART_LPURXD_Read(void);

/* �������ݼĴ��� ��غ��� */
extern void LPUART_LPUTXD_Write(uint32_t SetValue);
extern uint32_t LPUART_LPUTXD_Read(void);

/* ������ɱ�־ ��غ��� */
extern FlagStatus LPUART_LPUSTA_TC_Chk(void);

/* ����buffer�ձ�־ ��غ��� */
extern FlagStatus LPUART_LPUSTA_TXE_Chk(void);

/* ��ʼλ����־ ��غ��� */
extern void LPUART_LPUSTA_START_Clr(void);
extern FlagStatus LPUART_LPUSTA_START_Chk(void);

/* У��λ�����־ ��غ��� */
extern void LPUART_LPUSTA_PERR_Clr(void);
extern FlagStatus LPUART_LPUSTA_PERR_Chk(void);

/* ֡��ʽ�����־ ��غ��� */
extern void LPUART_LPUSTA_FERR_Clr(void);
extern FlagStatus LPUART_LPUSTA_FERR_Chk(void);

/* ���ջ��������־ ��غ��� */
extern void LPUART_LPUSTA_RXOV_Clr(void);
extern FlagStatus LPUART_LPUSTA_RXOV_Chk(void);

/* ���ջ�������־ ��غ��� */
extern void LPUART_LPUSTA_RXF_Clr(void);
extern FlagStatus LPUART_LPUSTA_RXF_Chk(void);

/* ����ƥ���־ ��غ��� */
extern void LPUART_LPUSTA_MATCH_Clr(void);
extern FlagStatus LPUART_LPUSTA_MATCH_Chk(void);

/* ���ݷ��ͼ���ȡ��ʹ�� ��غ��� */
extern void LPUART_LPUCON_TXPOL_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TXPOL_Getable(void);

/* ��������ж�ʹ�� ��غ��� */
extern void LPUART_LPUCON_TCIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TCIE_Getable(void);

/* ����buffer���ж�ʹ�� ��غ��� */
extern void LPUART_LPUCON_TXIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_TXIE_Getable(void);

/* �½��ؼ��ʹ�� ��غ��� */
extern void LPUART_LPUCON_NEDET_Setable(FunState NewState);
extern FunState LPUART_LPUCON_NEDET_Getable(void);

/* У��λʹ�� ��غ��� */
extern void LPUART_LPUCON_PAREN_Setable(FunState NewState);
extern FunState LPUART_LPUCON_PAREN_Getable(void);

/* У��λ���� ��غ��� */
extern void LPUART_LPUCON_PTYP_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_PTYP_Get(void);

/* ֹͣλ���� ��غ��� */
extern void LPUART_LPUCON_SL_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_SL_Get(void);

/* ���ݳ��� ��غ��� */
extern void LPUART_LPUCON_DL_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_DL_Get(void);

/* ���ݽ��ռ���ȡ������ ��غ��� */
extern void LPUART_LPUCON_RXPOL_Setable(FunState NewState);
extern FunState LPUART_LPUCON_RXPOL_Getable(void);

/* �����ж�ʹ�� ��غ��� */
extern void LPUART_LPUCON_ERRIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_ERRIE_Getable(void);

/* �����ж�ʹ�� ��غ��� */
extern void LPUART_LPUCON_RXIE_Setable(FunState NewState);
extern FunState LPUART_LPUCON_RXIE_Getable(void);

/* �����ж��¼����ã����ڿ��ƺ����¼�����CPU�ṩ�����ж� ��غ��� */
extern void LPUART_LPUCON_RXEV_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUCON_RXEV_Get(void);

/* ��������жϱ�־ ��غ��� */
extern void LPUART_LPUIF_TC_IF_Clr(void);
extern FlagStatus LPUART_LPUIF_TC_IF_Chk(void);

/* ����buffer���жϱ�־ ��غ��� */
extern void LPUART_LPUIF_TXIF_Clr(void);
extern FlagStatus LPUART_LPUIF_TXIF_Chk(void);

/* RXD�½����жϱ�־ ��غ��� */
extern void LPUART_LPUIF_RXNEGIF_Clr(void);
extern FlagStatus LPUART_LPUIF_RXNEGIF_Chk(void);

/* ��������жϱ�־ ��غ��� */
extern void LPUART_LPUIF_RXIF_Clr(void);
extern FlagStatus LPUART_LPUIF_RXIF_Chk(void);

/* �����ʿ��� ��غ��� */
extern void LPUART_LPUBAUD_BAUD_Set(uint32_t SetValue);
extern uint32_t LPUART_LPUBAUD_BAUD_Get(void);

/* ����ʹ�� ��غ��� */
extern void LPUART_LPUEN_TXEN_Setable(FunState NewState);
extern FunState LPUART_LPUEN_TXEN_Getable(void);

/* ����ʹ�� ��غ��� */
extern void LPUART_LPUEN_RXEN_Setable(FunState NewState);
extern FunState LPUART_LPUEN_RXEN_Getable(void);

/* ����ƥ��Ĵ��� ��غ��� */
extern void LPUART_COMPARE_Write(uint32_t SetValue);
extern uint32_t LPUART_COMPARE_Read(void);

/* ���ƿ��ƼĴ��� ��غ��� */
extern void LPUART_MCTL_Write(uint32_t SetValue);
extern uint32_t LPUART_MCTL_Read(void);
//Announce_End


/* LPUART����������ʼ������ */
extern void LPUART_Init(LPUART_InitTypeDef* para);

/* LPUART�򵥲�����ʼ������ */
extern void LPUART_SInit(LPUART_SInitTypeDef* para);


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_LPUART_H */

