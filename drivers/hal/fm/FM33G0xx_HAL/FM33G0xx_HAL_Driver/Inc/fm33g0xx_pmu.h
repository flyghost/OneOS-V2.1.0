/**
  ******************************************************************************
  * @file    fm33g0xx_pmu.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the PMU firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0xx_PMU_H
#define __FM33G0xx_PMU_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup PMU
  * @{
  */

	 
/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  PMU Init Structure definition  
  */   
  
typedef struct
{
	uint32_t PMOD;				/*!<�͹���ģʽ����  */	
	uint32_t SLPDP;				/*!<DeepSleep���ƼĴ���  */
	uint32_t DSLPRAM_EXT;		/*!<DeepSleepģʽ��RAM���ݱ�������  */	
	FunState CVS;				/*!<�ں˵�ѹ����ʹ�ܿ���  */
	uint32_t XTOFF;				/*!<�ر�XTLF������SLEEP/DEEPSLEEP��������  */
	uint32_t SCR;				/*!<M0ϵͳ���ƼĴ�����һ������Ϊ0����  */	
}PMU_SleepCfg_InitTypeDef;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/



#define	PMU_LPMCFG_FLSDPSEN_Pos	24	/* �ر�XTLF */
#define	PMU_LPMCFG_FLSDPSEN_Msk	(0x1U << PMU_LPMCFG_FLSDPSEN_Pos)
#define	PMU_LPMCFG_FLSDPSEN_EN	(0x1U << PMU_LPMCFG_FLSDPSEN_Pos)	/* 1:  �͹�����FLASH����DPSTB*/
#define	PMU_LPMCFG_FLSDPSEN_DIS	(0x0U << PMU_LPMCFG_FLSDPSEN_Pos)	/* 0:  �͹�����FLASH������DPSTB */

#define	PMU_LPMCFG_XTOFF_Pos	18	/* �ر�XTLF */
#define	PMU_LPMCFG_XTOFF_Msk	(0x3U << PMU_LPMCFG_XTOFF_Pos)
#define	PMU_LPMCFG_XTOFF_DIS	(0x1U << PMU_LPMCFG_XTOFF_Pos)	/* 01: ����XTLF���� */
#define	PMU_LPMCFG_XTOFF_EN	(0x2U << PMU_LPMCFG_XTOFF_Pos)	/* 10: �ر�XTLF */

#define	PMU_LPMCFG_LDO15EN_Pos	17	/* LDO15ʹ��״̬��ֻ�� */
#define	PMU_LPMCFG_LDO15EN_Msk	(0x1U << PMU_LPMCFG_LDO15EN_Pos)

#define	PMU_LPMCFG_SLPDP_Pos	9	/* DeepSleep���Ƽ� */
#define	PMU_LPMCFG_SLPDP_Msk	(0x1U << PMU_LPMCFG_SLPDP_Pos)
#define	PMU_LPMCFG_SLPDP_SLEEP	(0x0U << PMU_LPMCFG_SLPDP_Pos)	/* 0��Sleepģʽ */
#define	PMU_LPMCFG_SLPDP_DEEPSLEEP	(0x1U << PMU_LPMCFG_SLPDP_Pos)	/* 1��DeepSleepģʽ */

#define	PMU_LPMCFG_CVS_Pos	8	/* �͹���ģʽ�ں˵�ѹ���� */
#define	PMU_LPMCFG_CVS_Msk	(0x1U << PMU_LPMCFG_CVS_Pos)
	/* 0���͹���ģʽ�²�ʹ���ں˵�ѹ���� */
	/* 1���͹���ģʽ�½����ں˵�ѹ */

#define	PMU_LPMCFG_PMOD_Pos	0	/* �͹���ģʽ���üĴ��� */
#define	PMU_LPMCFG_PMOD_Msk	(0x3U << PMU_LPMCFG_PMOD_Pos)
#define	PMU_LPMCFG_PMOD_ACTIVE	(0x0U << PMU_LPMCFG_PMOD_Pos)	/* 00: Active mode */
#define	PMU_LPMCFG_PMOD_LPRUN	(0x1U << PMU_LPMCFG_PMOD_Pos)	/* 01: LPRUN mode */
#define	PMU_LPMCFG_PMOD_SLEEP	(0x2U << PMU_LPMCFG_PMOD_Pos)	/* 10: Sleep mode */
#define	PMU_LPMCFG_PMOD_RTCBKP (0x3U << PMU_LPMCFG_PMOD_Pos)	/* 11: RTCBKP mode */

#define	PMU_WKDLYCON_T1a_Pos	0	/* ����ʱ����ƼĴ��� */
#define	PMU_WKDLYCON_T1a_Msk	(0x3U << PMU_WKDLYCON_T1a_Pos)
#define	PMU_WKDLYCON_T1a_14US	(0x0U << PMU_WKDLYCON_T1a_Pos)	/* 00��14us */
#define	PMU_WKDLYCON_T1a_16US	(0x1U << PMU_WKDLYCON_T1a_Pos)	/* 01��16us */
#define	PMU_WKDLYCON_T1a_24US	(0x2U << PMU_WKDLYCON_T1a_Pos)	/* 10��24us */
#define	PMU_WKDLYCON_T1a_32US	(0x3U << PMU_WKDLYCON_T1a_Pos)	/* 11��32us */

#define	PMU_WKPFLAG_DBGWKF_Pos	8	/* CPU Debugger���ѱ�־ */
#define	PMU_WKPFLAG_DBGWKF_Msk	(0x1U << PMU_WKPFLAG_DBGWKF_Pos)

#define	PMU_WKPFLAG_WKP7F_Pos	7	/* NWKUP7 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP7F_Msk	(0x1U << PMU_WKPFLAG_WKP7F_Pos)

#define	PMU_WKPFLAG_WKP6F_Pos	6	/* NWKUP6 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP6F_Msk	(0x1U << PMU_WKPFLAG_WKP6F_Pos)

#define	PMU_WKPFLAG_WKP5F_Pos	5	/* NWKUP5 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP5F_Msk	(0x1U << PMU_WKPFLAG_WKP5F_Pos)

#define	PMU_WKPFLAG_WKP4F_Pos	4	/* NWKUP4 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP4F_Msk	(0x1U << PMU_WKPFLAG_WKP4F_Pos)

#define	PMU_WKPFLAG_WKP3F_Pos	3	/* NWKUP3 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP3F_Msk	(0x1U << PMU_WKPFLAG_WKP3F_Pos)

#define	PMU_WKPFLAG_WKP2F_Pos	2	/* NWKUP2 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP2F_Msk	(0x1U << PMU_WKPFLAG_WKP2F_Pos)

#define	PMU_WKPFLAG_WKP1F_Pos	1	/* NWKUP1 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP1F_Msk	(0x1U << PMU_WKPFLAG_WKP1F_Pos)

#define	PMU_WKPFLAG_WKP0F_Pos	0	/* NWKUP0 Pin���ѱ�־ */
#define	PMU_WKPFLAG_WKP0F_Msk	(0x1U << PMU_WKPFLAG_WKP0F_Pos)

#define	PMU_LPREIE_RTCBKPEIE_Pos	2	/* RTCBKP�����ж�ʹ�� */
#define	PMU_LPREIE_RTCBKPEIE_Msk	(0x1U << PMU_LPREIE_RTCBKPEIE_Pos)
  /* 0����ֹRTCBKP�����ж� */
	/* 1��ʹ��RTCBKP�����ж� */

#define	PMU_LPREIE_SLPEIE_Pos	1	/* SLEEP�����ж�ʹ�� */
#define	PMU_LPREIE_SLPEIE_Msk	(0x1U << PMU_LPREIE_SLPEIE_Pos)
	/* 0����ֹSLEEP�����ж� */
	/* 1��ʹ��SLEEP�����ж� */

#define	PMU_LPREIE_LPREIE_Pos	0	/* LPRUN�����ж�ʹ�� */
#define	PMU_LPREIE_LPREIE_Msk	(0x1U << PMU_LPREIE_LPREIE_Pos)
	/* 0����ֹLPRUN�����ж� */
	/* 1��ʹ��LPRUN�����ж� */

#define	PMU_LPREIF_RTCBKPEIF_Pos	2	/* RTCBK�����жϱ�־��Ӳ����λ�����д1���� */
#define	PMU_LPREIF_RTCBKPEIF_Msk	(0x1U << PMU_LPREIF_RTCBKPEIF_Pos)

#define	PMU_LPREIF_SLPEIF_Pos	1	/* SLEEP�����жϱ�־��Ӳ����λ�����д1����,��PMOD=2��h2��CPUִ��WFI/WFEָ��ǰ��λ��SLEEPDEEP�Ĵ���ʱ��λ */
#define	PMU_LPREIF_SLPEIF_Msk	(0x1U << PMU_LPREIF_SLPEIF_Pos)

#define	PMU_LPREIF_LPREIF_Pos	0	/* LPRUN�����жϱ�־��Ӳ����λ�����д1���㣻�������LPRUNģʽʱ���������LPREIF����оƬ�Խ�ͣ����ACTIVEģʽ,LPRUN Condition Error��������LPRUNʱ�������������
1�� HSCLKѡ����LSCLK��RCLP����
2�� RCHF��PLL��ADCʹ��δ�ر�
 */
#define	PMU_LPREIF_LPREIF_Msk	(0x1U << PMU_LPREIF_LPREIF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void PMU_Deinit(void);

/*�͹�����FLASH DeepStandbyʹ�ܺ���*/
//extern void PMU_LPMCFG_FLSDPSEN_Setable(FunState NewState);

///*��FLSDPSEN ״̬����*/
//extern FunState PMU_LPMCFG_FLSDPSEN_Getable(void);
  
/* �ر�XTLF ��غ��� */
extern void PMU_LPMCFG_XTOFF_Set(uint32_t SetValue);
extern uint32_t PMU_LPMCFG_XTOFF_Get(void);

/* LDO15ʹ��״̬��ֻ�� ��غ��� */
extern FlagStatus PMU_LPMCFG_LDO15EN_Chk(void);

/* DeepSleep���Ƽ� ��غ��� */
extern void PMU_LPMCFG_SLPDP_Set(uint32_t SetValue);
extern uint32_t PMU_LPMCFG_SLPDP_Get(void);

/* �͹���ģʽ�ں˵�ѹ���� ��غ��� */
extern void PMU_LPMCFG_CVS_Setable(FunState NewState);
extern FunState PMU_LPMCFG_CVS_Getable(void);

/* �͹���ģʽ���üĴ��� ��غ��� */
extern void PMU_LPMCFG_PMOD_Set(uint32_t SetValue);
extern uint32_t PMU_LPMCFG_PMOD_Get(void);

/* ����ʱ����ƼĴ��� ��غ��� */
extern void PMU_WKDLYCON_T1a_Set(uint32_t SetValue);
extern uint32_t PMU_WKDLYCON_T1a_Get(void);

/* CPU Debugger���ѱ�־ ��غ��� */
extern void PMU_WKPFLAG_DBGWKF_Clr(void);
extern FlagStatus PMU_WKPFLAG_DBGWKF_Chk(void);

/* RTCBK�����ж�ʹ�� ��غ��� */
extern void PMU_LPREIE_RTCBKPEIE_Setable(FunState NewState);
extern FunState PMU_LPREIE_RTCBKPEIE_Getable(void);

/* SLEEP�����ж�ʹ�� ��غ��� */
extern void PMU_LPREIE_SLPEIE_Setable(FunState NewState);
extern FunState PMU_LPREIE_SLPEIE_Getable(void);

/* LPRUN�����ж�ʹ�� ��غ��� */
extern void PMU_LPREIE_LPREIE_Setable(FunState NewState);
extern FunState PMU_LPREIE_LPREIE_Getable(void);

/* RTCBK�����жϱ�־��Ӳ����λ�����д1����*/
extern void PMU_LPREIF_RTCBKPEIF_Clr(void);
extern FlagStatus PMU_LPREIF_RTCBKPEIF_Chk(void);

/* SLEEP�����жϱ�־��Ӳ����λ�����д1����,��PMOD=2��h2��CPUִ��WFI/WFEָ��ǰ��λ��SLEEPDEEP�Ĵ���ʱ��λ ��غ��� */
extern void PMU_LPREIF_SLPEIF_Clr(void);
extern FlagStatus PMU_LPREIF_SLPEIF_Chk(void);

/* LPRUN�����жϱ�־��Ӳ����λ�����д1���㣻�������LPRUNģʽʱ���������LPREIF����оƬ�Խ�ͣ����ACTIVEģʽ,LPRUN Condition Error��������LPRUNʱ�������������
1�� HSCLKѡ����LSCLK��RCLP����
2�� RCHF��PLL��ADCʹ��δ�ر�
 ��غ��� */
extern void PMU_LPREIF_LPREIF_Clr(void);
extern FlagStatus PMU_LPREIF_LPREIF_Chk(void);
//Announce_End


extern void PMU_WKPFLAG_WKPxF_ClrEx(uint32_t NWKPinDef);
extern FlagStatus PMU_WKPFLAG_WKPxF_ChkEx(uint32_t NWKPinDef);
extern void PMU_SleepCfg_Init(PMU_SleepCfg_InitTypeDef* SleepCfg_InitStruct);




#ifdef __cplusplus
}
#endif

#endif /* __FM33G0xx_PMU_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



