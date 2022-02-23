
/**
  ******************************************************************************
  * @file    fm33g0xx_lptim.h
  * @author  FM33g0xx Application Team
  * @version V0.3.01G
  * @date    01-21-2019
  * @brief   This file contains all the functions prototypes for the LPTIM firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_LPTIM_H
#define __FM33G0XX_LPTIM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup LPTIM
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/

/** @defgroup LPTIM_Exported_Constants
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  LPTIM Init Structure definition  
  */ 

typedef struct
{
	uint32_t LPTIM_TMODE;  //����ģʽ
	uint32_t LPTIM_MODE;  //����ģʽ

	uint32_t LPTIM_PWM;   //�������ģʽ
	uint32_t LPTIM_POLAR; //ʱ�Ӽ���ѡ��

	uint32_t LPTIM_TRIG_CFG;  //�ⲿ��������ѡ��
	FunState LPTIM_FLTEN;  //�ⲿ�����˲�ʹ��
	
	uint32_t LPTIM_LPTIN_EDGE;//LPTIN�������ѡ��

	uint32_t LPTIM_CLK_SEL;  //ʱ��Դѡ��
	uint32_t LPTIM_CLK_DIV;  //ʱ�ӷ�Ƶѡ��

	uint32_t LPTIM_compare_value;  //�Ƚ�ֵ
	uint32_t LPTIM_target_value;  //Ŀ��ֵ

}LPTIM_InitTypeDef;



#define	LPTIM_LPTCFG_FLTEN_Pos	15	/* �ⲿTrigger�˲�ʹ�� */
#define	LPTIM_LPTCFG_FLTEN_Msk	(0x1U << LPTIM_LPTCFG_FLTEN_Pos)
	/* 0�����˲����� */
	/* 1�����˲����� */

#define	LPTIM_LPTCFG_DIVSEL_Pos	10	/* ����ʱ�ӷ�Ƶѡ�� */
#define	LPTIM_LPTCFG_DIVSEL_Msk	(0x7U << LPTIM_LPTCFG_DIVSEL_Pos)
#define	LPTIM_LPTCFG_DIVSEL_1	(0x0U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��1��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_2	(0x1U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��2��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_4	(0x2U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��4��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_8	(0x3U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��8��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_16	(0x4U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��16��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_32	(0x5U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��32��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_64	(0x6U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��64��Ƶ */
#define	LPTIM_LPTCFG_DIVSEL_128	(0x7U << LPTIM_LPTCFG_DIVSEL_Pos)	/* ����ʱ��128��Ƶ */

#define	LPTIM_LPTCFG_CLKSEL_Pos	8	/* ʱ��Դѡ�� */
#define	LPTIM_LPTCFG_CLKSEL_Msk	(0x3U << LPTIM_LPTCFG_CLKSEL_Pos)
#define	LPTIM_LPTCFG_CLKSEL_LSCLK	(0x0U << LPTIM_LPTCFG_CLKSEL_Pos)	/* ʱ��Դѡ��LSCLK��Ϊ����ʱ�� */
#define	LPTIM_LPTCFG_CLKSEL_RCLP	(0x1U << LPTIM_LPTCFG_CLKSEL_Pos)	/* ʱ��Դѡ��RCLP��Ϊ����ʱ�� */
#define	LPTIM_LPTCFG_CLKSEL_PCLK	(0x2U << LPTIM_LPTCFG_CLKSEL_Pos)	/* ʱ��Դѡ��PCLK��Ϊ����ʱ�� */
#define	LPTIM_LPTCFG_CLKSEL_LPTIN	(0x3U << LPTIM_LPTCFG_CLKSEL_Pos)	/* ʱ��Դѡ��LPTIN��Ϊ����ʱ�� */

#define	LPTIM_LPTCFG_EDGESEL_Pos	7	/* LPTIN�������ѡ�� */
#define	LPTIM_LPTCFG_EDGESEL_Msk	(0x1U << LPTIM_LPTCFG_EDGESEL_Pos)
#define	LPTIM_LPTCFG_EDGESEL_POS	(0x0U << LPTIM_LPTCFG_EDGESEL_Pos)	/* LPTIN�������ؼ��� */
#define	LPTIM_LPTCFG_EDGESEL_NEG	(0x1U << LPTIM_LPTCFG_EDGESEL_Pos)	/* LPTIN���½��ؼ��� */

#define	LPTIM_LPTCFG_TRIGCFG_Pos	5	/* �ⲿ��������ѡ�� */
#define	LPTIM_LPTCFG_TRIGCFG_Msk	(0x3U << LPTIM_LPTCFG_TRIGCFG_Pos)
#define	LPTIM_LPTCFG_TRIGCFG_POS	(0x0U << LPTIM_LPTCFG_TRIGCFG_Pos)	/* �ⲿ�����ź�������trigger */
#define	LPTIM_LPTCFG_TRIGCFG_NEG	(0x1U << LPTIM_LPTCFG_TRIGCFG_Pos)	/* �ⲿ�����ź��½���trigger */
#define	LPTIM_LPTCFG_TRIGCFG_POS_NEG	(0x2U << LPTIM_LPTCFG_TRIGCFG_Pos)	/* �ⲿ�����ź������½���trigger */

#define	LPTIM_LPTCFG_POLARITY_Pos	4	/* ������μ���ѡ�� */
#define	LPTIM_LPTCFG_POLARITY_Msk	(0x1U << LPTIM_LPTCFG_POLARITY_Pos)
#define	LPTIM_LPTCFG_POLARITY_POS	(0x0U << LPTIM_LPTCFG_POLARITY_Pos)	/* ��һ�μ���ֵ=�Ƚ�ֵʱ����������������� */
#define	LPTIM_LPTCFG_POLARITY_NEG	(0x1U << LPTIM_LPTCFG_POLARITY_Pos)	/* ��һ�μ���ֵ=�Ƚ�ֵʱ������������½��� */

#define	LPTIM_LPTCFG_PWM_Pos	3	/* �������ģʽ */
#define	LPTIM_LPTCFG_PWM_Msk	(0x1U << LPTIM_LPTCFG_PWM_Pos)
#define	LPTIM_LPTCFG_PWM_TOGGLE	(0x0U << LPTIM_LPTCFG_PWM_Pos)	/* ���ڷ������ģʽ */
#define	LPTIM_LPTCFG_PWM_PWM	(0x1U << LPTIM_LPTCFG_PWM_Pos)	/* PWM���ģʽ */

#define	LPTIM_LPTCFG_MODE_Pos	2	/* ����ģʽѡ�� */
#define	LPTIM_LPTCFG_MODE_Msk	(0x1U << LPTIM_LPTCFG_MODE_Pos)
#define	LPTIM_LPTCFG_MODE_CONTINUE	(0x0U << LPTIM_LPTCFG_MODE_Pos)	/* ��������ģʽ */
#define	LPTIM_LPTCFG_MODE_SINGLE	(0x1U << LPTIM_LPTCFG_MODE_Pos)	/* ���μ���ģʽ */

#define	LPTIM_LPTCFG_TMODE_Pos	0	/* ����ģʽѡ�� */
#define	LPTIM_LPTCFG_TMODE_Msk	(0x3U << LPTIM_LPTCFG_TMODE_Pos)
#define	LPTIM_LPTCFG_TMODE_PWMIM	(0x0U << LPTIM_LPTCFG_TMODE_Pos)	/* �������������ͨ��ʱ��ģʽ */
#define	LPTIM_LPTCFG_TMODE_INTRIGGER	(0x1U << LPTIM_LPTCFG_TMODE_Pos)	/* Trigger���崥������ģʽ */
#define	LPTIM_LPTCFG_TMODE_EXTRIGGER	(0x2U << LPTIM_LPTCFG_TMODE_Pos)	/* �ⲿ�첽�������ģʽ */
#define	LPTIM_LPTCFG_TMODE_TIMEOUT	(0x3U << LPTIM_LPTCFG_TMODE_Pos)	/* Timeoutģʽ */

#define	LPTIM_LPTCNT_CNT16_Pos	0
#define	LPTIM_LPTCNT_CNT16_Msk	(0xffffU << LPTIM_LPTCNT_CNT16_Pos)

#define	LPTIM_LPTCMP_COMPARE_REG_Pos	0
#define	LPTIM_LPTCMP_COMPARE_REG_Msk	(0xffffU << LPTIM_LPTCMP_COMPARE_REG_Pos)

#define	LPTIM_LPTTARGET_TARGET_REG_Pos	0
#define	LPTIM_LPTTARGET_TARGET_REG_Msk	(0xffffU << LPTIM_LPTTARGET_TARGET_REG_Pos)

#define	LPTIM_LPTIE_TRIGIE_Pos	2	/* �ⲿ�����ж� */
#define	LPTIM_LPTIE_TRIGIE_Msk	(0x1U << LPTIM_LPTIE_TRIGIE_Pos)
	/* 0���ⲿ���������жϽ�ֹ */
	/* 1���ⲿ���������ж�ʹ�� */

#define	LPTIM_LPTIE_OVIE_Pos	1	/* ����������ж� */
#define	LPTIM_LPTIE_OVIE_Msk	(0x1U << LPTIM_LPTIE_OVIE_Pos)
	/* 0������������жϽ�ֹ */
	/* 1������������ж�ʹ�� */

#define	LPTIM_LPTIE_COMPIE_Pos	0	/* �Ƚ�ֵƥ���ж� */
#define	LPTIM_LPTIE_COMPIE_Msk	(0x1U << LPTIM_LPTIE_COMPIE_Pos)
	/* 0��������ֵ�ͱȽ�ֵƥ���жϽ�ֹ */
	/* 1��������ֵ�ͱȽ�ֵƥ���ж�ʹ�� */

#define	LPTIM_LPTIF_TRIGIF_Pos	2	/* �ⲿ������־ */
#define	LPTIM_LPTIF_TRIGIF_Msk	(0x1U << LPTIM_LPTIF_TRIGIF_Pos)

#define	LPTIM_LPTIF_OVIF_Pos	1	/* �����������־ */
#define	LPTIM_LPTIF_OVIF_Msk	(0x1U << LPTIM_LPTIF_OVIF_Pos)

#define	LPTIM_LPTIF_COMPIF_Pos	0	/* �Ƚ�ֵƥ���־ */
#define	LPTIM_LPTIF_COMPIF_Msk	(0x1U << LPTIM_LPTIF_COMPIF_Pos)

#define	LPTIM_LPTCTRL_LPTEN_Pos	0	/* LPTʹ�� */
#define	LPTIM_LPTCTRL_LPTEN_Msk	(0x1U << LPTIM_LPTCTRL_LPTEN_Pos)
	/* 0����ֹ���������� */
	/* 1��ʹ�ܼ��������� */
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void LPTIM_Deinit(void);

/* �ⲿTrigger�˲�ʹ�� ��غ��� */
extern void LPTIM_LPTCFG_FLTEN_Setable(FunState NewState);
extern FunState LPTIM_LPTCFG_FLTEN_Getable(void);

/* ����ʱ�ӷ�Ƶѡ�� ��غ��� */
extern void LPTIM_LPTCFG_DIVSEL_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_DIVSEL_Get(void);

/* ʱ��Դѡ�� ��غ��� */
extern void LPTIM_LPTCFG_CLKSEL_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_CLKSEL_Get(void);

/* LPTIN�������ѡ�� ��غ��� */
extern void LPTIM_LPTCFG_EDGESEL_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_EDGESEL_Get(void);

/* �ⲿ��������ѡ�� ��غ��� */
extern void LPTIM_LPTCFG_TRIGCFG_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_TRIGCFG_Get(void);

/* ������μ���ѡ�� ��غ��� */
extern void LPTIM_LPTCFG_POLARITY_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_POLARITY_Get(void);

/* �������ģʽ ��غ��� */
extern void LPTIM_LPTCFG_PWM_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_PWM_Get(void);

/* ����ģʽѡ�� ��غ��� */
extern void LPTIM_LPTCFG_MODE_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_MODE_Get(void);

/* ����ģʽѡ�� ��غ��� */
extern void LPTIM_LPTCFG_TMODE_Set(uint32_t SetValue);
extern uint32_t LPTIM_LPTCFG_TMODE_Get(void);
extern uint32_t LPTIM_LPTCNT_Read(void);
extern void LPTIM_LPTCMP_Write(uint32_t SetValue);
extern uint32_t LPTIM_LPTCMP_Read(void);
extern void LPTIM_LPTTARGET_Write(uint32_t SetValue);
extern uint32_t LPTIM_LPTTARGET_Read(void);

/* �ⲿ�����ж� ��غ��� */
extern void LPTIM_LPTIE_TRIGIE_Setable(FunState NewState);
extern FunState LPTIM_LPTIE_TRIGIE_Getable(void);

/* ����������ж� ��غ��� */
extern void LPTIM_LPTIE_OVIE_Setable(FunState NewState);
extern FunState LPTIM_LPTIE_OVIE_Getable(void);

/* �Ƚ�ֵƥ���ж� ��غ��� */
extern void LPTIM_LPTIE_COMPIE_Setable(FunState NewState);
extern FunState LPTIM_LPTIE_COMPIE_Getable(void);

/* �ⲿ������־ ��غ��� */
extern void LPTIM_LPTIF_TRIGIF_Clr(void);
extern FlagStatus LPTIM_LPTIF_TRIGIF_Chk(void);

/* �����������־ ��غ��� */
extern void LPTIM_LPTIF_OVIF_Clr(void);
extern FlagStatus LPTIM_LPTIF_OVIF_Chk(void);

/* �Ƚ�ֵƥ���־ ��غ��� */
extern void LPTIM_LPTIF_COMPIF_Clr(void);
extern FlagStatus LPTIM_LPTIF_COMPIF_Chk(void);

/* LPTʹ�� ��غ��� */
extern void LPTIM_LPTCTRL_LPTEN_Setable(FunState NewState);
extern FunState LPTIM_LPTCTRL_LPTEN_Getable(void);
//Announce_End



void LPTIM_Init(LPTIM_InitTypeDef* para);



#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_LPTIM_H */




