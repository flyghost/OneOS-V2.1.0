/**
  ******************************************************************************
  * @file    fm33g0xx_flash.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the flash firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_FLASH_H
#define __FM33G0XX_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
 
/** @addtogroup FM33G0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup NVMIF
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/

/** @defgroup NVMIF_Exported_Constants
  * @{
  */
#define flash_sector_erase_key0 	((uint32_t)0x96969696)
#define flash_sector_erase_key1 	((uint32_t)0xEAEAEAEA)
	 
#define flash_block_erase_key0 		((uint32_t)0x96969696)
#define flash_block_erase_key1 		((uint32_t)0x3C3C3C3C)

#define flash_chip_erase_key0 		((uint32_t)0x96969696)
#define flash_chip_erase_key1 		((uint32_t)0x7D7D7D7D)
	 
#define flash_erase_data 			((uint32_t)0x1234ABCD)

#define flash_PROG_key0 			((uint32_t)0xA5A5A5A5)
#define flash_PROG_key1 			((uint32_t)0xF1F1F1F1)

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/



#define	FLASH_FLSRDCON_WAIT_Pos	0	/* Flash���ȴ���������(CPUʱ�ӳ���24M���迪wait1) */
#define	FLASH_FLSRDCON_WAIT_Msk	(0x3U << FLASH_FLSRDCON_WAIT_Pos)
#define	FLASH_FLSRDCON_WAIT_0CYCLE	(0x0U << FLASH_FLSRDCON_WAIT_Pos)	/* 00/11��0 wait cycle */
#define	FLASH_FLSRDCON_WAIT_1CYCLE	(0x1U << FLASH_FLSRDCON_WAIT_Pos)	/* 01��1 wait cycle */
#define	FLASH_FLSRDCON_WAIT_2CYCLE	(0x2U << FLASH_FLSRDCON_WAIT_Pos)	/* 10��2 wait cycles */



#define	FLASH_OPTBR_DBGCFGEN_Pos	31	/* �û������ּĴ��� */
#define	FLASH_OPTBR_DBGCFGEN_Msk	(0x1U << FLASH_OPTBR_DBGCFGEN_Pos)

#define	FLASH_OPTBR_BTSEN_Pos	8   /*BootSwap ����ʹ��*/
#define	FLASH_OPTBR_BTSEN_Msk	(0x3U << FLASH_OPTBR_BTSEN_Pos)

#define	FLASH_OPTBR_ACLOCKEN_Pos	2
#define	FLASH_OPTBR_ACLOCKEN_Msk	(0x3U << FLASH_OPTBR_ACLOCKEN_Pos)

#define	FLASH_OPTBR_DBRDPEN_Pos	0
#define	FLASH_OPTBR_DBRDPEN_Msk	(0x3U << FLASH_OPTBR_DBRDPEN_Pos)

#define	FLASH_ACLOCK1_ACLOCK_Pos	0	/* ACLOCK���üĴ�����32bit���ֱ����ڿ���Block31~Block0��Ӧ�ô����д������1����ȡ�Ͳ�дȨ������0����ȡ�Ͳ�дȨ�޷ſ�,���ֻ��д1���������㡣 */
#define	FLASH_ACLOCK1_ACLOCK_Msk	(0xffffffffU << FLASH_ACLOCK1_ACLOCK_Pos)

#define	FLASH_EPCON_ERTYPE_Pos	8	/* ������ */
#define	FLASH_EPCON_ERTYPE_Msk	(0x3U << FLASH_EPCON_ERTYPE_Pos)
#define	FLASH_EPCON_ERTYPE_SECTOR	(0x0U << FLASH_EPCON_ERTYPE_Pos)	/* 00/11��Sector Erase */

#define	FLASH_EPCON_PREQ_Pos	1
#define	FLASH_EPCON_PREQ_Msk	(0x1U << FLASH_EPCON_PREQ_Pos)
#define	FLASH_EPCON_PREQ_NONE	(0x0U << FLASH_EPCON_PREQ_Pos)
#define	FLASH_EPCON_PREQ_PROG	(0x1U << FLASH_EPCON_PREQ_Pos)	/* Program Request�����λ��Ӳ����ɱ�̺��Զ����� */

#define	FLASH_EPCON_EREQ_Pos	0
#define	FLASH_EPCON_EREQ_Msk	(0x1U << FLASH_EPCON_EREQ_Pos)
#define	FLASH_EPCON_EREQ_NONE	(0x0U << FLASH_EPCON_EREQ_Pos)
#define	FLASH_EPCON_EREQ_ERASE	(0x1U << FLASH_EPCON_EREQ_Pos)	/* Erase Request�����λ��Ӳ����ɲ������Զ����� */

#define	FLASH_FLSKEY_FLSKEY_Pos	0	/* Flash��дKey����Ĵ������������SWD��������дǰ������ȷ����˵�ַд��Ϸ�KEY���С��յ�ַ���������޼Ĵ���ʵ�֡� */
#define	FLASH_FLSKEY_FLSKEY_Msk	(0xffffffffU << FLASH_FLSKEY_FLSKEY_Pos)

#define	FLASH_FLSIE_AUTHIE_Pos	10	/* Flash�ж�ʹ�� */
#define	FLASH_FLSIE_AUTHIE_Msk	(0x1U << FLASH_FLSIE_AUTHIE_Pos)
	/* Flash��дȨ�޴����ж�ʹ�� */

#define	FLASH_FLSIE_KEYIE_Pos	9
#define	FLASH_FLSIE_KEYIE_Msk	(0x1U << FLASH_FLSIE_KEYIE_Pos)
	/* Flash KEY�����ж�ʹ�� */

#define	FLASH_FLSIE_CKIE_Pos	8
#define	FLASH_FLSIE_CKIE_Msk	(0x1U << FLASH_FLSIE_CKIE_Pos)
	/* ��д��ʱʱ�Ӵ����ж�ʹ�� */

#define	FLASH_FLSIE_PRDIE_Pos	1
#define	FLASH_FLSIE_PRDIE_Msk	(0x1U << FLASH_FLSIE_PRDIE_Pos)
	/* �����ɱ�־�ж�ʹ�� */

#define	FLASH_FLSIE_ERDIE_Pos	0
#define	FLASH_FLSIE_ERDIE_Msk	(0x1U << FLASH_FLSIE_ERDIE_Pos)
	/* ��д��ɱ�־�ж�ʹ�� */

#define	FLASH_FLSIF_KEYSTA_Pos	17	/* Flash��дKEY����״̬ */
#define	FLASH_FLSIF_KEYSTA_Msk	(0x7U << FLASH_FLSIF_KEYSTA_Pos)
#define	FLASH_FLSIF_KEYSTA_ERR_LOCK	 (0x4U << FLASH_FLSIF_KEYSTA_Pos)/* KEY��������״̬����Ҫ��λ���ܽ���*/
#define	FLASH_FLSIF_KEYSTA_P_UNLOCK	 (0x3U << FLASH_FLSIF_KEYSTA_Pos)/* ��̽���״̬ */
#define	FLASH_FLSIF_KEYSTA_SE_UNLOCK	(0x2U << FLASH_FLSIF_KEYSTA_Pos)/* ����������״̬*/
#define	FLASH_FLSIF_KEYSTA_ALLE_UNLOCK	(0x1U << FLASH_FLSIF_KEYSTA_Pos)/* ȫ������״̬ */
#define	FLASH_FLSIF_KEYSTA_WP         (0x0U << FLASH_FLSIF_KEYSTA_Pos) /* Flash д����״̬��δ����KEY */


#define	FLASH_FLSIF_AUTHIF_Pos	10	/* Flash�жϱ�־ */
#define	FLASH_FLSIF_AUTHIF_Msk	(0x1U << FLASH_FLSIF_AUTHIF_Pos)

#define	FLASH_FLSIF_KEYIF_Pos	9
#define	FLASH_FLSIF_KEYIF_Msk	(0x1U << FLASH_FLSIF_KEYIF_Pos)

#define	FLASH_FLSIF_CKIF_Pos	8
#define	FLASH_FLSIF_CKIF_Msk	(0x1U << FLASH_FLSIF_CKIF_Pos)

#define	FLASH_FLSIF_PRDIF_Pos	1
#define	FLASH_FLSIF_PRDIF_Msk	(0x1U << FLASH_FLSIF_PRDIF_Pos)

#define	FLASH_FLSIF_ERDIF_Pos	0
#define	FLASH_FLSIF_ERDIF_Msk	(0x1U << FLASH_FLSIF_ERDIF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void FLASH_Deinit(void);

/* Flash���ȴ���������(CPUʱ�ӳ���24M���迪wait1) ��غ��� */
extern void FLASH_FLSRDCON_WAIT_Set(uint32_t SetValue);
extern uint32_t FLASH_FLSRDCON_WAIT_Get(void);

/* �û������ּĴ��� ��غ��� */
extern FlagStatus FLASH_OPTBR_DBGCFGEN_Chk(void);
extern FlagStatus FLASH_OPTBR_BTSEN_Chk(void);
extern FlagStatus FLASH_OPTBR_ACLOCKEN_Chk(void);
extern FlagStatus FLASH_OPTBR_DBRDPEN_Chk(void);

/* ACLOCK���üĴ�����32bit���ֱ����ڿ���Block31~Block0��Ӧ�ô����д������1����ȡ�Ͳ�дȨ������0����ȡ�Ͳ�дȨ�޷ſ�,���ֻ��д1���������㡣 ��غ��� */
extern void FLASH_ACLOCK1_Write(uint32_t SetValue);
extern uint32_t FLASH_ACLOCK1_Read(void);

/* ������ ��غ��� */
extern void FLASH_EPCON_ERTYPE_Set(uint32_t SetValue);
extern uint32_t FLASH_EPCON_ERTYPE_Get(void);
extern void FLASH_EPCON_PREQ_Set(uint32_t SetValue);
extern uint32_t FLASH_EPCON_PREQ_Get(void);
extern void FLASH_EPCON_EREQ_Set(uint32_t SetValue);
extern uint32_t FLASH_EPCON_EREQ_Get(void);

/* Flash��дKey����Ĵ������������SWD��������дǰ������ȷ����˵�ַд��Ϸ�KEY���С��յ�ַ���������޼Ĵ���ʵ�֡� ��غ��� */
extern void FLASH_FLSKEY_Write(uint32_t SetValue);

/* Flash�ж�ʹ�� ��غ��� */
extern void FLASH_FLSIE_AUTHIE_Setable(FunState NewState);
extern FunState FLASH_FLSIE_AUTHIE_Getable(void);
extern void FLASH_FLSIE_KEYIE_Setable(FunState NewState);
extern FunState FLASH_FLSIE_KEYIE_Getable(void);
extern void FLASH_FLSIE_CKIE_Setable(FunState NewState);
extern FunState FLASH_FLSIE_CKIE_Getable(void);
extern void FLASH_FLSIE_PRDIE_Setable(FunState NewState);
extern FunState FLASH_FLSIE_PRDIE_Getable(void);
extern void FLASH_FLSIE_ERDIE_Setable(FunState NewState);
extern FunState FLASH_FLSIE_ERDIE_Getable(void);

/* Flash�жϱ�־ ��غ��� */
extern uint32_t FLASH_FLSIF_KEYSTA_Get(void);
extern void FLASH_FLSIF_AUTHIF_Clr(void);
extern FlagStatus FLASH_FLSIF_AUTHIF_Chk(void);
extern void FLASH_FLSIF_KEYIF_Clr(void);
extern FlagStatus FLASH_FLSIF_KEYIF_Chk(void);
extern void FLASH_FLSIF_CKIF_Clr(void);
extern FlagStatus FLASH_FLSIF_CKIF_Chk(void);
extern void FLASH_FLSIF_PRDIF_Clr(void);
extern FlagStatus FLASH_FLSIF_PRDIF_Chk(void);
extern void FLASH_FLSIF_ERDIF_Clr(void);
extern FlagStatus FLASH_FLSIF_ERDIF_Chk(void);
//Announce_End


extern void FLASH_Erase_Sector(uint32_t erase_addr);
extern void FLASH_Prog_SingleByte(uint32_t prog_addr,uint8_t prog_data);
extern void FLASH_Prog_ByteString(uint32_t prog_addr,uint8_t* prog_data, uint16_t Len);

#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_FLASH_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



