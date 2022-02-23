/**
  ******************************************************************************
  * @file    fm33g0xx_iwdt.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the IWDT firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_IWDT_H
#define __FM33G0XX_IWDT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"

/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup IWDT
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define WDTSERV_key	((uint32_t)0x12345A5A)
	 


#define	IWDT_IWDTSERV_IWDTSERV_Pos	0	/* IWDT����Ĵ��� */
#define	IWDT_IWDTSERV_IWDTSERV_Msk	(0xffffffffU << IWDT_IWDTSERV_IWDTSERV_Pos)

#define	IWDT_IWDTCFG_IWDTSLP4096S_Pos	2	/* IWDT����4096s���� */
#define	IWDT_IWDTCFG_IWDTSLP4096S_Msk	(0x1U << IWDT_IWDTCFG_IWDTSLP4096S_Pos)
	/* 0:���ߺ���ʹ��IWDTOVP�����õĶ����� */
	/* 1:���ߺ�IWDT�����Զ�ʹ��4096S */

#define	IWDT_IWDTCFG_IWDTOVP_Pos	0	/* IWDT����������� */
#define	IWDT_IWDTCFG_IWDTOVP_Msk	(0x3U << IWDT_IWDTCFG_IWDTOVP_Pos)
#define	IWDT_IWDTCFG_IWDTOVP_125ms	(0x0U << IWDT_IWDTCFG_IWDTOVP_Pos)	/* x00��125ms */
#define	IWDT_IWDTCFG_IWDTOVP_500ms	(0x1U << IWDT_IWDTCFG_IWDTOVP_Pos)	/* x01��500ms */
#define	IWDT_IWDTCFG_IWDTOVP_2s	(0x2U << IWDT_IWDTCFG_IWDTOVP_Pos)	/* x10��2s */
#define	IWDT_IWDTCFG_IWDTOVP_8s	(0x3U << IWDT_IWDTCFG_IWDTOVP_Pos)	/* x11��8s */

#define	IWDT_IWDTCNT_IWDTCNT_Pos	0	/* IWDT��ǰ����ֵ */
#define	IWDT_IWDTCNT_IWDTCNT_Msk	(0x3ffffU << IWDT_IWDTCNT_IWDTCNT_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void IWDT_Deinit(void);

/* IWDT����Ĵ��� ��غ��� */
extern void IWDT_IWDTSERV_Write(uint32_t SetValue);

/* IWDT����4096s���� ��غ��� */
extern void IWDT_IWDTCFG_IWDTSLP4096S_Setable(FunState NewState);
extern FunState IWDT_IWDTCFG_IWDTSLP4096S_Getable(void);

/* IWDT����������� ��غ��� */
extern void IWDT_IWDTCFG_IWDTOVP_Set(uint32_t SetValue);
extern uint32_t IWDT_IWDTCFG_IWDTOVP_Get(void);

/* IWDT��ǰ����ֵ ��غ��� */
extern uint32_t IWDT_IWDTCNT_Read(void);
//Announce_End

	 
extern void IWDT_Clr(void);

#ifdef __cplusplus
}
#endif

#endif /* __FM33G0xx_IWDT_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



