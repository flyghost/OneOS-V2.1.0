/**
  ******************************************************************************
  * @file    fm33g0xx_trng.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the RCC firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_TRNG_H
#define __FM33G0XX_TRNG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/* TRNG���� ��غ��� */
extern void ANAC_TRNGCON_TRNGEN_Setable(FunState NewState);
extern FunState ANAC_TRNGCON_TRNGEN_Getable(void);
	 
#define TRNG_TRNGCON_RNGEN_Setable ANAC_TRNGCON_TRNGEN_Setable
#define TRNG_TRNGCON_RNGEN_Getable ANAC_TRNGCON_TRNGEN_Getable
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/



#define	TRNG_TRNGCON_RNGEN_Pos	0	/* RNGʹ�ܼĴ��������д1ʹ�� */
#define	TRNG_TRNGCON_RNGEN_Msk	(0x1U << TRNG_TRNGCON_RNGEN_Pos)
	/* 0���ر�RNG */
	/* 1��ʹ��RNG */

#define	TRNG_RNGOUT_RNGOUT_Pos	0	/* ��������ɽ����CRC�������Ĵ��� */
#define	TRNG_RNGOUT_RNGOUT_Msk	(0xffffffffU << TRNG_RNGOUT_RNGOUT_Pos)

#define	TRNG_RNGIF_LFSREN_Pos	1	/* LFSRʹ�ܱ�־,���Ĵ�����������ģ���жϣ�������ѯ */
#define	TRNG_RNGIF_LFSREN_Msk	(0x1U << TRNG_RNGIF_LFSREN_Pos)

#define	TRNG_RNGIF_RNF_Pos	0	/* ���������ʧ�ܱ�־�����д1��0 */
#define	TRNG_RNGIF_RNF_Msk	(0x1U << TRNG_RNGIF_RNF_Pos)

#define	TRNG_CRCCON_CRCEN_Pos	0	/* CRC���ƼĴ��� */
#define	TRNG_CRCCON_CRCEN_Msk	(0x1U << TRNG_CRCCON_CRCEN_Pos)
	/* 0��CRC�ر� */
	/* 1��CRCʹ�� */

#define	TRNG_CRCIN_CRCIN_Pos	0	/* CRC������������Ĵ��� */
#define	TRNG_CRCIN_CRCIN_Msk	(0xffffffffU << TRNG_CRCIN_CRCIN_Pos)

#define	TRNG_CRCFLAG_CRCD_Pos	0	/* CRC������ɱ�־�����д0���� */
#define	TRNG_CRCFLAG_CRCD_Msk	(0x1U << TRNG_CRCFLAG_CRCD_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void TRNG_Deinit(void);

///* RNGʹ�ܼĴ��������д1ʹ�� ��غ��� */
//extern void TRNG_TRNGCON_RNGEN_Setable(FunState NewState);
//extern FunState TRNG_TRNGCON_RNGEN_Getable(void);

/* ��������ɽ����CRC�������Ĵ��� ��غ��� */
extern uint32_t TRNG_RNGOUT_Read(void);

/* LFSRʹ�ܱ�־,���Ĵ�����������ģ���жϣ�������ѯ ��غ��� */
extern FlagStatus TRNG_RNGIF_LFSREN_Chk(void);

/* ���������ʧ�ܱ�־�����д1��0 ��غ��� */
extern void TRNG_RNGIF_RNF_Clr(void);
extern FlagStatus TRNG_RNGIF_RNF_Chk(void);

/* CRC���ƼĴ��� ��غ��� */
extern void TRNG_CRCCON_CRCEN_Setable(FunState NewState);
extern FunState TRNG_CRCCON_CRCEN_Getable(void);

/* CRC������������Ĵ��� ��غ��� */
extern void TRNG_CRCIN_Write(uint32_t SetValue);
extern uint32_t TRNG_CRCIN_Read(void);

/* CRC������ɱ�־�����д0���� ��غ��� */
extern void TRNG_CRCFLAG_CRCD_Clr(void);
extern FlagStatus TRNG_CRCFLAG_CRCD_Chk(void);
//Announce_End


#ifdef __cplusplus
}
#endif

#endif 



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



