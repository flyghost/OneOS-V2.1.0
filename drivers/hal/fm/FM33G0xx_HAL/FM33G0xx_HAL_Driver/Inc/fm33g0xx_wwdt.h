/**
  ******************************************************************************
  * @file    fm33g0xx_wwdt.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the  firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_WWDT_H
#define __FM33G0XX_WWDT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup WWDT
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/

/** @defgroup WWDT_Exported_Constants
  * @{
  */
#define WWDT_START_key 				((uint32_t)0x5A)
#define WWDT_CLR_key 					((uint32_t)0xAC)
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/



#define	WWDT_WWDTCON_WWDTCON_Pos	0	/* WWDT���ƼĴ��� */
#define	WWDT_WWDTCON_WWDTCON_Msk	(0xffU << WWDT_WWDTCON_WWDTCON_Pos)

#define	WWDT_WWDTCFG_WWDTCFG_Pos	0	/* ���ÿ��Ź����ʱ�� */
#define	WWDT_WWDTCFG_WWDTCFG_Msk	(0x7U << WWDT_WWDTCFG_WWDTCFG_Pos)
#define	WWDT_WWDTCFG_WWDTCFG_X1	(0x0U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 000��TPCLK * 4096 * 1 */
#define	WWDT_WWDTCFG_WWDTCFG_X4	(0x1U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 001��TPCLK * 4096 * 4 */
#define	WWDT_WWDTCFG_WWDTCFG_X16	(0x2U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 010��TPCLK * 4096 * 16 */
#define	WWDT_WWDTCFG_WWDTCFG_X64	(0x3U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 011��TPCLK * 4096 * 64 */
#define	WWDT_WWDTCFG_WWDTCFG_X128	(0x4U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 100��TPCLK * 4096 * 128 */
#define	WWDT_WWDTCFG_WWDTCFG_X256	(0x5U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 101��TPCLK * 4096 * 256 */
#define	WWDT_WWDTCFG_WWDTCFG_X512	(0x6U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 110��TPCLK * 4096 * 512 */
#define	WWDT_WWDTCFG_WWDTCFG_X1024	(0x7U << WWDT_WWDTCFG_WWDTCFG_Pos)	/* 111��TPCLK * 4096 * 1024 */

#define	WWDT_WWDTCNT_WWDTCNT_Pos	0	/* WWDT�����Ĵ���ֵ */
#define	WWDT_WWDTCNT_WWDTCNT_Msk	(0x3ffU << WWDT_WWDTCNT_WWDTCNT_Pos)

#define	WWDT_WWDTIE_WWDTIE_Pos	0	/* WWDT�ж�ʹ�� */
#define	WWDT_WWDTIE_WWDTIE_Msk	(0x1U << WWDT_WWDTIE_WWDTIE_Pos)
	/* 0���ж�ʹ�ܽ�ֹ */
	/* 1���ж�ʹ�ܴ� */

#define	WWDT_WWDTIF_WWDTIF_Pos	0	/* WWDT 75%��ʱ�жϱ�־��д1���� */
#define	WWDT_WWDTIF_WWDTIF_Msk	(0x1U << WWDT_WWDTIF_WWDTIF_Pos)

#define	WWDT_WWDTDIV_DIVCNT_Pos	0	/* WWDT��4096Ԥ��Ƶ������ */
#define	WWDT_WWDTDIV_DIVCNT_Msk	(0xfffU << WWDT_WWDTDIV_DIVCNT_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void WWDT_Deinit(void);

/* WWDT���ƼĴ��� ��غ��� */
extern void WWDT_WWDTCON_Write(uint32_t SetValue);

/* ���ÿ��Ź����ʱ�� ��غ��� */
extern void WWDT_WWDTCFG_WWDTCFG_Set(uint32_t SetValue);
extern uint32_t WWDT_WWDTCFG_WWDTCFG_Get(void);

/* WWDT�����Ĵ���ֵ ��غ��� */
extern uint32_t WWDT_WWDTCNT_Read(void);

/* WWDT�ж�ʹ�� ��غ��� */
extern void WWDT_WWDTIE_WWDTIE_Setable(FunState NewState);
extern FunState WWDT_WWDTIE_WWDTIE_Getable(void);

/* WWDT 75%��ʱ�жϱ�־��д1���� ��غ��� */
extern void WWDT_WWDTIF_WWDTIF_Clr(void);
extern FlagStatus WWDT_WWDTIF_WWDTIF_Chk(void);

/* WWDT��4096Ԥ��Ƶ������ ��غ��� */
extern uint32_t WWDT_WWDTDIV_Read(void);
//Announce_End


extern void WWDT_Start(void);

extern void WWDT_Clr(void);


#ifdef __cplusplus
}
#endif

#endif



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



