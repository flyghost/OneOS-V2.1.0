/**
  ******************************************************************************
  * @file    fm33g0xx_crc.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the  firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_CRC_H
#define __FM33G0XX_CRC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup fm33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup CRC
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{       	
	uint32_t			CRCSEL;		/*!<CRCУ�����ʽѡ��*/
	FunState			RFLTIN;		/*!<CRC���뷴ת����*/
	FunState			RFLTO;		/*!<CRC�����ת����*/
	FunState			XOR;		/*!<������ʹ��*/
	uint32_t			CRC_XOR;	/*!<���������Ĵ���*/

}CRC_InitTypeDef;

/* Exported macro ------------------------------------------------------------*/




#define	CRC_CRCDR_CRCDR_Pos	0	/* CRC���ݼĴ��� */
#define	CRC_CRCDR_CRCDR_Msk	(0xffffU << CRC_CRCDR_CRCDR_Pos)

#define	CRC_CRCCR_RFLTIN_Pos	6	/* CRC���뷴ת���� */
#define	CRC_CRCCR_RFLTIN_Msk	(0x1U << CRC_CRCCR_RFLTIN_Pos)
	/* 0�����벻��ת */
	/* 1�����밴�ֽڷ�ת */

#define	CRC_CRCCR_RFLTO_Pos	5	/* CRC�����ת���� */
#define	CRC_CRCCR_RFLTO_Msk	(0x1U << CRC_CRCCR_RFLTO_Pos)
	/* 0�����벻��ת */
	/* 1�����밴�ֽڷ�ת */

#define	CRC_CRCCR_RES_Pos	4	/* CRC�����־λ��ֻ�� */
#define	CRC_CRCCR_RES_Msk	(0x1U << CRC_CRCCR_RES_Pos)

#define	CRC_CRCCR_BUSY_Pos	3	/* CRC�����־λ��ֻ�� */
#define	CRC_CRCCR_BUSY_Msk	(0x1U << CRC_CRCCR_BUSY_Pos)

#define	CRC_CRCCR_XOR_Pos	2	/* ������ʹ�� */
#define	CRC_CRCCR_XOR_Msk	(0x1U << CRC_CRCCR_XOR_Pos)
	/* 0����������CRC_XOR�Ĵ��� */
	/* 1��������CRC_XOR�Ĵ��� */

#define	CRC_CRCCR_CRCSEL_Pos	0	/* CRCУ�����ʽѡ�� */
#define	CRC_CRCCR_CRCSEL_Msk	(0x3U << CRC_CRCCR_CRCSEL_Pos)
#define	CRC_CRCCR_CRCSEL_CRC16	(0x0U << CRC_CRCCR_CRCSEL_Pos)	/* 00��CRC16 */
#define	CRC_CRCCR_CRCSEL_CRC8	(0x1U << CRC_CRCCR_CRCSEL_Pos)	/* 01��CRC8 */
#define	CRC_CRCCR_CRCSEL_CCITT	(0x2U << CRC_CRCCR_CRCSEL_Pos)	/* 10/11��CCITT */

#define	CRC_CRCLFSR_LFSR_Pos	0	/* CRC����Ĵ��� */
#define	CRC_CRCLFSR_LFSR_Msk	(0xffffU << CRC_CRCLFSR_LFSR_Pos)

#define	CRC_CRCXOR_CRC_XOR_Pos	0	/* CRC������Ĵ��� */
#define	CRC_CRCXOR_CRC_XOR_Msk	(0xffffU << CRC_CRCXOR_CRC_XOR_Pos)

#define	CRC_CRCFLSEN_FLSCRCEN_Pos	0	/* Flash����CRCУ��ʹ�� */
#define	CRC_CRCFLSEN_FLSCRCEN_Msk	(0x1U << CRC_CRCFLSEN_FLSCRCEN_Pos)
	/* 0����ʹ��Flash CRCУ�� */
	/* 1������Flash CRCУ�� */

#define	CRC_CRCFLSAD_FLSAD_Pos	0	/* FlashУ����ʼ��ַ��Word��ַ�� */
#define	CRC_CRCFLSAD_FLSAD_Msk	(0x1ffffU << CRC_CRCFLSAD_FLSAD_Pos)

#define	CRC_CRCFLSSIZE_FLSS_Pos	0	/* CRC FlashУ�����ݳ��ȣ�Word���ȣ� */
#define	CRC_CRCFLSSIZE_FLSS_Msk	(0x1ffffU << CRC_CRCFLSSIZE_FLSS_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void CRC_Deinit(void);

/* CRC���ݼĴ��� ��غ��� */
extern void CRC_CRCDR_Write(uint32_t SetValue);
extern uint32_t CRC_CRCDR_Read(void);

/* CRC���뷴ת���� ��غ��� */
extern void CRC_CRCCR_RFLTIN_Setable(FunState NewState);
extern FunState CRC_CRCCR_RFLTIN_Getable(void);

/* CRC�����ת���� ��غ��� */
extern void CRC_CRCCR_RFLTO_Setable(FunState NewState);
extern FunState CRC_CRCCR_RFLTO_Getable(void);

/* CRC�����־λ��ֻ�� ��غ��� */
extern FlagStatus CRC_CRCCR_RES_Chk(void);

/* CRC�����־λ��ֻ�� ��غ��� */
extern FlagStatus CRC_CRCCR_BUSY_Chk(void);

/* ������ʹ�� ��غ��� */
extern void CRC_CRCCR_XOR_Setable(FunState NewState);
extern FunState CRC_CRCCR_XOR_Getable(void);

/* CRCУ�����ʽѡ�� ��غ��� */
extern void CRC_CRCCR_CRCSEL_Set(uint32_t SetValue);
extern uint32_t CRC_CRCCR_CRCSEL_Get(void);

/* CRC����Ĵ��� ��غ��� */
extern void CRC_CRCLFSR_Write(uint32_t SetValue);
extern uint32_t CRC_CRCLFSR_Read(void);

/* CRC������Ĵ��� ��غ��� */
extern void CRC_CRCXOR_Write(uint32_t SetValue);
extern uint32_t CRC_CRCXOR_Read(void);

/* Flash����CRCУ��ʹ�� ��غ��� */
extern void CRC_CRCFLSEN_FLSCRCEN_Setable(FunState NewState);
extern FunState CRC_CRCFLSEN_FLSCRCEN_Getable(void);

/* FlashУ����ʼ��ַ��Word��ַ�� ��غ��� */
extern void CRC_CRCFLSAD_Write(uint32_t SetValue);
extern uint32_t CRC_CRCFLSAD_Read(void);

/* CRC FlashУ�����ݳ��ȣ�Word���ȣ� ��غ��� */
extern void CRC_CRCFLSSIZE_Write(uint32_t SetValue);
extern uint32_t CRC_CRCFLSSIZE_Read(void);
//Announce_End


/* CRC��ʼ�����ú���*/
void CRC_Init(CRC_InitTypeDef* para);
	
#ifdef __cplusplus
}
#endif

#endif



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



