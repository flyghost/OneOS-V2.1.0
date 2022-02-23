/**
******************************************************************************
* @file    fm33g0xx_uart.h
* @author  FM33g0xx Application Team
* @version V0.3.00G
* @date    08-31-2018
* @brief   This file contains all the functions prototypes for the UART firmware     
******************************************************************************
*/ 
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_UART_H
#define __FM33G0XX_UART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"

/** @addtogroup FM33G0XX_StdPeriph_Driver
  * @{
  */

/** @addtogroup UART
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
typedef enum
{
	RxInt,
	TxInt
	
}UART_IntTypeDef;

typedef enum
{
	Seven7Bit,		//7λ���ݲ�֧��żУ��λ
	Eight8Bit,
	Nine9Bit		//9λ���ݲ�֧����żУ��λ
	
}UART_DataBitTypeDef;

typedef enum
{
	NONE,
	EVEN,
	ODD
	
}UART_ParityBitTypeDef;

typedef enum
{
	OneBit,
	TwoBit
}UART_StopBitTypeDef;

typedef struct
{
	uint32_t				BaudRate;		//������
	UART_DataBitTypeDef		DataBit;		//����λ��
	UART_ParityBitTypeDef	ParityBit; 		//У��λ
	UART_StopBitTypeDef		StopBit;		//ֹͣλ
	
}UART_SInitTypeDef;	

typedef struct
{
	FunState				RXIE;		//�����ж�
	FunState				TXIE;		//�����ж�
	
	uint32_t				SPBRG;		//�����ʲ������Ĵ���
	
	uint32_t				PDSEL;		//ģʽѡ��00 = 8λ���ݣ�����żУ�飻01 = 8λ���ݣ�żУ�飻10 = 8λ���ݣ���У�顣11 = 9λ���ݣ� ����żУ�飻
	FunState				ERRIE;		//�����ж�ʹ�ܿ���
	FunState				RXEN;		//����ģ��ʹ�ܿ���
	uint32_t				STOPSEL;	//ֹͣλѡ��
	uint32_t				TXIS;		//TX_INTSEL=0ʱ�������ж�ѡ��λ
	FunState				TXEN;		//����ģ��ʹ�ܿ���
	FunState				IREN;		//���ͺ������ʹ��λ
	uint32_t				TX_INTSEL;	//�����ж�ѡ��λ
	
	FunState				RTX7EN;		//�շ�7bit����ʹ��(����PDSEL)
	FunState				RXDFLAG;	//��������ȡ������λ
	FunState				TXDFLAG;	//��������ȡ������λ
	
}UART_InitTypeDef;	




#define	UART_UARTIE_UARTIE_Pos	0	/* UART�ж�����Ĵ��� */
#define	UART_UARTIE_UARTIE_Msk	(0xfffU << UART_UARTIE_UARTIE_Pos)

#define	UART_UARTIF_UARTIF_Pos	0	/* UART�жϱ�־�Ĵ��� */
#define	UART_UARTIF_UARTIF_Msk	(0xfffU << UART_UARTIF_UARTIF_Pos)

#define	UART_IRCON_IRFLAG_Pos	15	/* ������Ʒ������ݷ��� */
#define	UART_IRCON_IRFLAG_Msk	(0x1U << UART_IRCON_IRFLAG_Pos)
	/* 0���������� */
	/* 1�����෢�� */

#define	UART_IRCON_TH_Pos	11	/* ����ռ�ձȵ��Ʋ��� */
#define	UART_IRCON_TH_Msk	(0xfU << UART_IRCON_TH_Pos)

#define	UART_IRCON_TZBRG_Pos	0	/* �������Ƶ�� */
#define	UART_IRCON_TZBRG_Msk	(0x7ffU << UART_IRCON_TZBRG_Pos)

#define	UARTx_RXSTA_PDSEL_Pos	6	/* ģʽѡ��λ */
#define	UARTx_RXSTA_PDSEL_Msk	(0x3U << UARTx_RXSTA_PDSEL_Pos)
#define	UARTx_RXSTA_PDSEL_8BIT_NONE	(0x0U << UARTx_RXSTA_PDSEL_Pos)	/* 00 = 8λ���ݣ�����żУ�飻 */
#define	UARTx_RXSTA_PDSEL_8BIT_EVEN	(0x1U << UARTx_RXSTA_PDSEL_Pos)	/* 01 = 8λ���ݣ�żУ�飻 */
#define	UARTx_RXSTA_PDSEL_8BIT_ODD	(0x2U << UARTx_RXSTA_PDSEL_Pos)	/* 10 = 8λ���ݣ���У�顣 */
#define	UARTx_RXSTA_PDSEL_9BIT_NONE	(0x3U << UARTx_RXSTA_PDSEL_Pos)	/* 11 = 9λ���ݣ� ����żУ�飻 */

#define	UARTx_RXSTA_ERRIE_Pos	5	/* �����ж�����λ,������żУ���֡��ʽ������� */
#define	UARTx_RXSTA_ERRIE_Msk	(0x1U << UARTx_RXSTA_ERRIE_Pos)

#define	UARTx_RXSTA_RXEN_Pos	4	/* ����ģ��ʹ��λ */
#define	UARTx_RXSTA_RXEN_Msk	(0x1U << UARTx_RXSTA_RXEN_Pos)
	/* 0 = ��ֹ����ģ�飬����ģ�鱻��λ */
	/* 1 = ʹ�ܽ���ģ�飻 */

#define	UARTx_RXSTA_PERR_Pos	3	/* ��żУ����־λ */
#define	UARTx_RXSTA_PERR_Msk	(0x1U << UARTx_RXSTA_PERR_Pos)

#define	UARTx_RXSTA_FERR_Pos	2	/* ֡��ʽ���־λ */
#define	UARTx_RXSTA_FERR_Msk	(0x1U << UARTx_RXSTA_FERR_Pos)

#define	UARTx_RXSTA_OERR_Pos	1	/* ������־λ */
#define	UARTx_RXSTA_OERR_Msk	(0x1U << UARTx_RXSTA_OERR_Pos)

#define	UARTx_RXSTA_RX9D_Pos	0	/* �������ݵĵ�9λ */
#define	UARTx_RXSTA_RX9D_Msk	(0x1U << UARTx_RXSTA_RX9D_Pos)

#define	UARTx_TXSTA_STOPSEL_Pos	6	/* ֹͣλѡ��λ */
#define	UARTx_TXSTA_STOPSEL_Msk	(0x1U << UARTx_TXSTA_STOPSEL_Pos)
#define	UARTx_TXSTA_STOPSEL_1STOPBIT	(0x0U << UARTx_TXSTA_STOPSEL_Pos)	/* 0 = ֹͣλΪ1λ */
#define	UARTx_TXSTA_STOPSEL_2STOPBIT	(0x1U << UARTx_TXSTA_STOPSEL_Pos)	/* 1 = ֹͣλΪ2λ */

#define	UARTx_TXSTA_TXIS_Pos	5	/* TX_INTSEL=0ʱ�������ж�ѡ��λ */
#define	UARTx_TXSTA_TXIS_Msk	(0x1U << UARTx_TXSTA_TXIS_Pos)
#define	UARTx_TXSTA_TXIS_BUFFEREMPTY	(0x0U << UARTx_TXSTA_TXIS_Pos)	/* 0 = ���ͻ������ղ����ж� */
#define	UARTx_TXSTA_TXIS_SHIFTEMPTY	(0x1U << UARTx_TXSTA_TXIS_Pos)	/* 1 = ��λ�Ĵ����ղ����ж� */

#define	UARTx_TXSTA_TXEN_Pos	4	/* ����ģ��ʹ��λ */
#define	UARTx_TXSTA_TXEN_Msk	(0x1U << UARTx_TXSTA_TXEN_Pos)
	/* 0 = ��ֹ����ģ�飬����ģ�鱻��λ */
	/* 1 = ʹ�ܷ���ģ�� */

#define	UARTx_TXSTA_IREN_Pos	3	/* ���ͺ������ʹ��λ */
#define	UARTx_TXSTA_IREN_Msk	(0x1U << UARTx_TXSTA_IREN_Pos)
	/* 0 = ��ֹ���ͺ������ */
	/* 1 = ʹ�ܷ��ͺ������ */

#define	UARTx_TXSTA_TX9D_Pos	0	/* �������ݵĵ�9λ */
#define	UARTx_TXSTA_TX9D_Msk	(0x1U << UARTx_TXSTA_TX9D_Pos)
#define	UARTx_TXSTA_TX9D_0	(0x0U << UARTx_TXSTA_TX9D_Pos)
#define	UARTx_TXSTA_TX9D_1	(0x1U << UARTx_TXSTA_TX9D_Pos)

#define	UARTx_RXREG_RXREG_Pos	0	/* �������ݻ���Ĵ��� */
#define	UARTx_RXREG_RXREG_Msk	(0xffU << UARTx_RXREG_RXREG_Pos)

#define	UARTx_TXREG_TXREG_Pos	0	/* �������ݻ���Ĵ��� */
#define	UARTx_TXREG_TXREG_Msk	(0xffU << UARTx_TXREG_TXREG_Pos)

#define	UARTx_SPBRG_SPBRG_Pos	0	/* �����ʲ������Ĵ��� */
#define	UARTx_SPBRG_SPBRG_Msk	(0xffffU << UARTx_SPBRG_SPBRG_Pos)

#define	UARTx_TXBUFSTA_TX_INTSEL_Pos	2	/* �����ж�ѡ��λ */
#define	UARTx_TXBUFSTA_TX_INTSEL_Msk	(0x3U << UARTx_TXBUFSTA_TX_INTSEL_Pos)
#define	UARTx_TXBUFSTA_TX_INTSEL_BYTXIS	(0x0U << UARTx_TXBUFSTA_TX_INTSEL_Pos)	/* 00 =��TXIS���������ж��ڷ��ͻ���ջ�����λ�Ĵ�����ʱ���� */
#define	UARTx_TXBUFSTA_TX_INTSEL_SHIFTEMPTY	(0x1U << UARTx_TXBUFSTA_TX_INTSEL_Pos)	/* 01 = TXBUF������λ�Ĵ����ղ����ж� */
#define	UARTx_TXBUFSTA_TX_INTSEL_BUFFEREMPTY	(0x2U << UARTx_TXBUFSTA_TX_INTSEL_Pos)	/* 10 = TXBUF�ղ����ж� */
#define	UARTx_TXBUFSTA_TX_INTSEL_NOINT	(0x3U << UARTx_TXBUFSTA_TX_INTSEL_Pos)	/* 11 = �������ж� */

#define	UARTx_TXBUFSTA_TXFF_Pos	0	/* TXBUF״̬λ */
#define	UARTx_TXBUFSTA_TXFF_Msk	(0x1U << UARTx_TXBUFSTA_TXFF_Pos)

#define	UARTx_RXBUFSTA_RXFF_Pos	0	/* RXBUF״̬λ */
#define	UARTx_RXBUFSTA_RXFF_Msk	(0x1U << UARTx_RXBUFSTA_RXFF_Pos)

#define	UARTx_RTXCON_RTX7EN_Pos	2	/* �շ�7bit����ʹ��(����PDSEL) */
#define	UARTx_RTXCON_RTX7EN_Msk	(0x1U << UARTx_RTXCON_RTX7EN_Pos)
	/* 0 = �����շ� */
	/* 1 = �շ�7λ����֡����ʽΪ7λ����λ+STOP���շ�����ΪRXREG/TXREG[6:0] */

#define	UARTx_RTXCON_RXDFLAG_Pos	1	/* ��������ȡ������λ */
#define	UARTx_RTXCON_RXDFLAG_Msk	(0x1U << UARTx_RTXCON_RXDFLAG_Pos)
	/* 0 = �������ݲ�ȡ�� */
	/* 1 = ��������ȡ�� */

#define	UARTx_RTXCON_TXDFLAG_Pos	0	/* ��������ȡ������λ */
#define	UARTx_RTXCON_TXDFLAG_Msk	(0x1U << UARTx_RTXCON_TXDFLAG_Pos)
	/* 0 = �������ݲ�ȡ�� */
	/* 1 = ��������ȡ������������ƿ���ʱ����ȡ����Ч */
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void UART_Deinit(void);

/* ������Ʒ������ݷ��� ��غ��� */
extern void UART_IRCON_IRFLAG_Setable(FunState NewState);
extern FunState UART_IRCON_IRFLAG_Getable(void);

/* ����ռ�ձȵ��Ʋ��� ��غ��� */
extern void UART_IRCON_TH_Set(uint32_t SetValue);
extern uint32_t UART_IRCON_TH_Get(void);

/* �������Ƶ�� ��غ��� */
extern void UART_IRCON_TZBRG_Set(uint32_t SetValue);
extern uint32_t UART_IRCON_TZBRG_Get(void);
extern void UARTx_Deinit(UARTx_Type* UARTx);

/* ģʽѡ��λ ��غ��� */
extern void UARTx_RXSTA_PDSEL_Set(UARTx_Type* UARTx, uint32_t SetValue);
extern uint32_t UARTx_RXSTA_PDSEL_Get(UARTx_Type* UARTx);

/* �����ж�����λ,������żУ���֡��ʽ������� ��غ��� */
extern void UARTx_RXSTA_ERRIE_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_RXSTA_ERRIE_Getable(UARTx_Type* UARTx);

/* ����ģ��ʹ��λ ��غ��� */
extern void UARTx_RXSTA_RXEN_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_RXSTA_RXEN_Getable(UARTx_Type* UARTx);

/* ��żУ����־λ ��غ��� */
extern void UARTx_RXSTA_PERR_Clr(UARTx_Type* UARTx);
extern FlagStatus UARTx_RXSTA_PERR_Chk(UARTx_Type* UARTx);

/* ֡��ʽ���־λ ��غ��� */
extern void UARTx_RXSTA_FERR_Clr(UARTx_Type* UARTx);
extern FlagStatus UARTx_RXSTA_FERR_Chk(UARTx_Type* UARTx);

/* ������־λ ��غ��� */
extern void UARTx_RXSTA_OERR_Clr(UARTx_Type* UARTx);
extern FlagStatus UARTx_RXSTA_OERR_Chk(UARTx_Type* UARTx);

/* �������ݵĵ�9λ ��غ��� */
extern FlagStatus UARTx_RXSTA_RX9D_Chk(UARTx_Type* UARTx);

/* ֹͣλѡ��λ ��غ��� */
extern void UARTx_TXSTA_STOPSEL_Set(UARTx_Type* UARTx, uint32_t SetValue);
extern uint32_t UARTx_TXSTA_STOPSEL_Get(UARTx_Type* UARTx);

/* TX_INTSEL=0ʱ�������ж�ѡ��λ ��غ��� */
extern void UARTx_TXSTA_TXIS_Set(UARTx_Type* UARTx, uint32_t SetValue);
extern uint32_t UARTx_TXSTA_TXIS_Get(UARTx_Type* UARTx);

/* ����ģ��ʹ��λ ��غ��� */
extern void UARTx_TXSTA_TXEN_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_TXSTA_TXEN_Getable(UARTx_Type* UARTx);

/* ���ͺ������ʹ��λ ��غ��� */
extern void UARTx_TXSTA_IREN_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_TXSTA_IREN_Getable(UARTx_Type* UARTx);

/* �������ݵĵ�9λ ��غ��� */
extern void UARTx_TXSTA_TX9D_Set(UARTx_Type* UARTx, uint32_t SetValue);

/* �������ݻ���Ĵ��� ��غ��� */
extern uint32_t UARTx_RXREG_Read(UARTx_Type* UARTx);

/* �������ݻ���Ĵ��� ��غ��� */
extern void UARTx_TXREG_Write(UARTx_Type* UARTx, uint32_t SetValue);

/* �����ʲ������Ĵ��� ��غ��� */
extern void UARTx_SPBRG_Write(UARTx_Type* UARTx, uint32_t SetValue);
extern uint32_t UARTx_SPBRG_Read(UARTx_Type* UARTx);

/* �����ж�ѡ��λ ��غ��� */
extern void UARTx_TXBUFSTA_TX_INTSEL_Set(UARTx_Type* UARTx, uint32_t SetValue);
extern uint32_t UARTx_TXBUFSTA_TX_INTSEL_Get(UARTx_Type* UARTx);

/* TXBUF״̬λ ��غ��� */
extern FlagStatus UARTx_TXBUFSTA_TXFF_Chk(UARTx_Type* UARTx);

/* RXBUF״̬λ ��غ��� */
extern FlagStatus UARTx_RXBUFSTA_RXFF_Chk(UARTx_Type* UARTx);

/* �շ�7bit����ʹ��(����PDSEL) ��غ��� */
extern void UARTx_RTXCON_RTX7EN_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_RTXCON_RTX7EN_Getable(UARTx_Type* UARTx);

/* ��������ȡ������λ ��غ��� */
extern void UARTx_RTXCON_RXDFLAG_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_RTXCON_RXDFLAG_Getable(UARTx_Type* UARTx);

/* ��������ȡ������λ ��غ��� */
extern void UARTx_RTXCON_TXDFLAG_Setable(UARTx_Type* UARTx, FunState NewState);
extern FunState UARTx_RTXCON_TXDFLAG_Getable(UARTx_Type* UARTx);
//Announce_End


extern void UART_All_Deinit(void);

/* uart ���� ���� �ж�ʹ�����ú��� */
extern void UART_UARTIE_RxTxIE_SetableEx(UARTx_Type* UARTx, UART_IntTypeDef IntType, FunState NewState);

/* uart ���� ���� �ж�ʹ�ܶ�ȡ���� */
extern FunState UART_UARTIE_RxTxIE_GetableEx(UARTx_Type* UARTx, UART_IntTypeDef IntType);

/* uart ���� �жϱ�־���㺯��
	�����жϱ�־ ֻ�� ͨ����rxreg����
	�����жϱ�־��ͨ��дtxreg����txifд1����
*/
extern void UART_UARTIF_RxTxIF_ClrEx(UARTx_Type* UARTx);

/* uart ���� ���� �жϱ�־��ȡ���� */
extern FlagStatus UART_UARTIF_RxTxIF_ChkEx(UARTx_Type* UARTx, UART_IntTypeDef IntType);

/* �������Ƶ�ʣ�ռ�ձ� ���� */
extern void UART_IRModulation_Init( uint32_t ModuFreq, uint8_t ModuDutyCycle, uint32_t APBClk );

/* UART����������ʼ������ */
extern void UART_Init(UARTx_Type* UARTx, UART_InitTypeDef* para);

/* uart�����ʼĴ���ֵ���㺯�� */
extern uint32_t UART_BaudREGCalc(uint32_t BaudRate, uint32_t APBClk);

/* UART�򵥲�����ʼ������ */
extern void UART_SInit(UARTx_Type* UARTx, UART_SInitTypeDef* para, uint32_t APBClk);


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_UART_H */

