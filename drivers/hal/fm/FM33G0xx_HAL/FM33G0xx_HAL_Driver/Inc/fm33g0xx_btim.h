/**
  ******************************************************************************
  * @file    fm33g0xx_btim.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the BTIM firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_BTIM_H
#define __FM33G0XX_BTIM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup BTIM
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/

/** @defgroup BTIM_Exported_Constants
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  BTIM Init Structure definition  
  */ 

typedef struct
{
  uint32_t RTCSEL1;            /* SLELECT RTC SOUCE 1 */
  uint32_t RTCSEL2;          	/* SLELECT RTC SOUCE 2 */
  uint32_t INSEL1;            	/* SLELECT INNER SOUCE 1 */
  uint32_t INSEL2;             /* SLELECT INNER SOUCE 2 */										   	
  uint32_t EXSEL1;							/* SLELECT EX SOUCE 1 */
  uint32_t EXSEL2;							/* SLELECT EX SOUCE 2 */
  uint32_t GRP1SEL;						/* SLELECT GROUP1 SOUCE */
  uint32_t GRP2SEL;						/* SLELECT GROUP2 SOUCE */
  uint32_t CAPSRC;							/* SLELECT CAPTURE SOUCE */
  uint32_t CNTSRC;							/* SLELECT COUNT SOUCE */
  uint32_t SRCSEL;            	/* SET 1 TO LOW COUNTER */
  uint32_t CNTHSEL;							/* SELECT H COUNT SOURCE */
} BTIM_SRCSelType;

typedef struct
{
  uint32_t CAPONCE;            /* 0-CONTINUOUS 1-ONCE */
  uint32_t CAPCLR;          	 	/* CLR AT CAPTURE */
  uint32_t CAPMOD;            	/* 0-CYCLE 1-PULSE */
} BTIM_CAPInitType;

typedef struct
{
  FunState DIREN;            	/* DIR SIGNAL ENABLE */
  //uint32_t STDIR;          	 	/* SET DIR */
  uint32_t DIRPO;             	/* 1- REVERSE THE DIR SIGNAL*/										   	
} BTIM_DIRInitType;

typedef struct
{
  uint32_t OUTCNT;             /* SET OUTPUT PULSE WIDTH,0-FFF */										   	
  uint32_t OUTCLR;							/* SET OUTPUT TO 0 */
  uint32_t OUTINV;							/* SET OUTPUT SIGNAL REVERSE */
  uint32_t OUTMOD;							/* 0-PULSE 1-LEVEL */	
  uint32_t OUTSEL;							/* SELECT OUTPUT SOUCE */
	
} BTIM_OUTInitType;


typedef struct
{
//  BTIM_SRCSelType 		BTIM_CLK_SEL_SETTING;            		/* SLELECT CLK SOUCE */
//  BTIM_CAPInitType 		BTIM_CAPTUTE_SETTING;          	 	/* SLELECT CAPTURE SETTIG */
//  BTIM_DIRInitType 	  BTIM_DIR_SETTING;            	 	  /* SLELECT DIR SETTIG*/
//  BTIM_OUTInitType 		BTIM_OUTPUT_SETTING;             	/* SLELECT OUTPUT SETTIG*/	
	
  uint32_t PRESCALE;		
  uint32_t PRESETH;
  uint32_t PRESETL;
  uint32_t LOADL;	
  uint32_t LOADH;	
  //uint32_t CMPL;
  //uint32_t CMPH;
	
  uint32_t MODE;
  uint32_t EDGESEL;			/* 0-POS 1-NEG */
	
} BTIM_CTRLInitType;


typedef struct
{
	BTIM_CTRLInitType ctrl_para;//���������
	
	BTIM_SRCSelType sig_src_para;//�ź���Դ����
	BTIM_CAPInitType cap_para;//��׽ģʽ����
	BTIM_DIRInitType dir_para;//�����������������
	BTIM_OUTInitType out_para;//�������
	
}BTIM_InitTypeDef;




#define	BTIMx_BTCR1_CHEN_Pos	7	/* ��λ�������������� */
#define	BTIMx_BTCR1_CHEN_Msk	(0x1U << BTIMx_BTCR1_CHEN_Pos)
	/* 0��������λ������ */
	/* 1��ֹͣ��λ������ */

#define	BTIMx_BTCR1_CLEN_Pos	6	/* ��λ�������������� */
#define	BTIMx_BTCR1_CLEN_Msk	(0x1U << BTIMx_BTCR1_CLEN_Pos)
	/* 0��������λ������ */
	/* 1��ֹͣ��λ������ */

#define	BTIMx_BTCR1_MODE_Pos	5	/* ����ģʽѡ�� */
#define	BTIMx_BTCR1_MODE_Msk	(0x1U << BTIMx_BTCR1_MODE_Pos)
#define	BTIMx_BTCR1_MODE_8BITS_TIM_CNT	(0x0U << BTIMx_BTCR1_MODE_Pos)	/* 8bit��ʱ����ģʽ */
#define	BTIMx_BTCR1_MODE_16BITS_CAP	(0x1U << BTIMx_BTCR1_MODE_Pos)	/* 16bit��׽ģʽ */

#define	BTIMx_BTCR1_EDGESEL_Pos	4	/* �����ػ�׽��ѡ�� */
#define	BTIMx_BTCR1_EDGESEL_Msk	(0x1U << BTIMx_BTCR1_EDGESEL_Pos)
#define	BTIMx_BTCR1_EDGESEL_POS	(0x0U << BTIMx_BTCR1_EDGESEL_Pos)	/* �����ؼ�����׽ */
#define	BTIMx_BTCR1_EDGESEL_NEG	(0x1U << BTIMx_BTCR1_EDGESEL_Pos)	/* �½��ؼ�����׽ */

#define	BTIMx_BTCR1_CAPMOD_Pos	3	/* ��׽ģʽ���ƣ�����׽ģʽ����Ч�� */
#define	BTIMx_BTCR1_CAPMOD_Msk	(0x1U << BTIMx_BTCR1_CAPMOD_Pos)
#define	BTIMx_BTCR1_CAPMOD_PAUSE_PERIOD	(0x0U << BTIMx_BTCR1_CAPMOD_Pos)	/* �������ڲ�׽ */
#define	BTIMx_BTCR1_CAPMOD_PAUSE_WIDTH	(0x1U << BTIMx_BTCR1_CAPMOD_Pos)	/* �����Ȳ�׽ */

#define	BTIMx_BTCR1_CAPCLR_Pos	2	/* �����㲶׽ģʽ���� */
#define	BTIMx_BTCR1_CAPCLR_Msk	(0x1U << BTIMx_BTCR1_CAPCLR_Pos)
#define	BTIMx_BTCR1_CAPCLR_CAP_CNT_NO_CLR	(0x0U << BTIMx_BTCR1_CAPCLR_Pos)	/* ��׽������������� */
#define	BTIMx_BTCR1_CAPCLR_CAP_CNT_CLR	(0x1U << BTIMx_BTCR1_CAPCLR_Pos)	/* ��׽����������� */

#define	BTIMx_BTCR1_CAPONCE_Pos	1	/* ���β�׽���� */
#define	BTIMx_BTCR1_CAPONCE_Msk	(0x1U << BTIMx_BTCR1_CAPONCE_Pos)
#define	BTIMx_BTCR1_CAPONCE_CONTINUE	(0x0U << BTIMx_BTCR1_CAPONCE_Pos)	/* ������׽ */
#define	BTIMx_BTCR1_CAPONCE_SINGLE	(0x1U << BTIMx_BTCR1_CAPONCE_Pos)	/* ���β�׽ */

#define	BTIMx_BTCR1_PWM_Pos	0	/* PWMģʽ��� */
#define	BTIMx_BTCR1_PWM_Msk	(0x1U << BTIMx_BTCR1_PWM_Pos)
	/* 0��PWM�����ʹ�� */
	/* 1��PWM���ʹ�� */

#define	BTIMx_BTCR2_SIG2SEL_Pos	7	/* �������ڲ�����Դ�ź�ѡ�� */
#define	BTIMx_BTCR2_SIG2SEL_Msk	(0x1U << BTIMx_BTCR2_SIG2SEL_Pos)
#define	BTIMx_BTCR2_SIG2SEL_GROUP1	(0x0U << BTIMx_BTCR2_SIG2SEL_Pos)	/* �ڲ�����Դѡ��Group1 */
#define	BTIMx_BTCR2_SIG2SEL_GROUP2	(0x1U << BTIMx_BTCR2_SIG2SEL_Pos)	/* �ڲ�����Դѡ��Group2 */

#define	BTIMx_BTCR2_SIG1SEL_Pos	6	/* �������ڲ���׽Դ�ź�ѡ�� */
#define	BTIMx_BTCR2_SIG1SEL_Msk	(0x1U << BTIMx_BTCR2_SIG1SEL_Pos)
#define	BTIMx_BTCR2_SIG1SEL_GROUP2	(0x0U << BTIMx_BTCR2_SIG1SEL_Pos)	/* �ڲ�����Դѡ��Group2 */
#define	BTIMx_BTCR2_SIG1SEL_GROUP1	(0x1U << BTIMx_BTCR2_SIG1SEL_Pos)	/* �ڲ�����Դѡ��Group1 */

#define	BTIMx_BTCR2_CNTHSEL_Pos	4	/* ��λ������Դѡ�� */
#define	BTIMx_BTCR2_CNTHSEL_Msk	(0x3U << BTIMx_BTCR2_CNTHSEL_Pos)
#define	BTIMx_BTCR2_CNTHSEL_CNTL	(0x0U << BTIMx_BTCR2_CNTHSEL_Pos)	/* ���λ���������16λ������ */
#define	BTIMx_BTCR2_CNTHSEL_CAPSRC	(0x1U << BTIMx_BTCR2_CNTHSEL_Pos)	/* ѡ���ڲ���׽�ź�Դ */
#define	BTIMx_BTCR2_CNTHSEL_CNTSRC_DIR	(0x2U << BTIMx_BTCR2_CNTHSEL_Pos)	/* ѡ���ڲ������ź�Դ���ⲿDIR������� */

#define	BTIMx_BTCR2_DIREN_Pos	3	/* �ⲿ����DIR����ʹ�� */
#define	BTIMx_BTCR2_DIREN_Msk	(0x1U << BTIMx_BTCR2_DIREN_Pos)
	/* 0���ⲿ����DIR�ź���Ч */
	/* 1���ⲿ����DIR�ź���Ч */

#define	BTIMx_BTCR2_STDIR_Pos	2	/* �ڲ�DIR�����ź�ʹ�� */
#define	BTIMx_BTCR2_STDIR_Msk	(0x1U << BTIMx_BTCR2_STDIR_Pos)
	/* 0���ڲ�DIR�ź���Ч */
	/* 1���ڲ�DIR�ź�Ϊ�ߵ�ƽ�����8λ���������� */

#define	BTIMx_BTCR2_SRCSEL_Pos	1	/* ��λ������ʹ�ܿ���ѡ���ź� */
#define	BTIMx_BTCR2_SRCSEL_Msk	(0x1U << BTIMx_BTCR2_SRCSEL_Pos)
#define	BTIMx_BTCR2_SRCSEL_STDIR_EXSIG2	(0x0U << BTIMx_BTCR2_SRCSEL_Pos)	/* ��λ��������STDIR���ⲿEX_SIG2������� */
#define	BTIMx_BTCR2_SRCSEL_WITHOUT_DIR	(0x1U << BTIMx_BTCR2_SRCSEL_Pos)	/* ��λ����������DIR���� */

#define	BTIMx_BTCR2_DIRPO_Pos	0	/* �����ź�2����ѡ�� */
#define	BTIMx_BTCR2_DIRPO_Msk	(0x1U << BTIMx_BTCR2_DIRPO_Pos)
#define	BTIMx_BTCR2_DIRPO_NO_ANTI	(0x0U << BTIMx_BTCR2_DIRPO_Pos)	/* ���ⲿ����DIR�źŲ����� */
#define	BTIMx_BTCR2_DIRPO_ANTI	(0x1U << BTIMx_BTCR2_DIRPO_Pos)	/* ���ⲿ����DIR�ź�ȡ���� */

#define	BTIMx_BTCFG1_RTCSEL2_Pos	6	/* RTCOUT2�źſ���2 */
#define	BTIMx_BTCFG1_RTCSEL2_Msk	(0x3U << BTIMx_BTCFG1_RTCSEL2_Pos)
#define	BTIMx_BTCFG1_RTCSEL2_RTC_32768	(0x0U << BTIMx_BTCFG1_RTCSEL2_Pos)	/* ��RTCģ�������32768Hz�ź� */
#define	BTIMx_BTCFG1_RTCSEL2_RTC_SEC	(0x1U << BTIMx_BTCFG1_RTCSEL2_Pos)	/* ��RTCģ����������ź� */
#define	BTIMx_BTCFG1_RTCSEL2_RTC_MIN	(0x2U << BTIMx_BTCFG1_RTCSEL2_Pos)	/* ��RTCģ������ķ����ź� */
#define	BTIMx_BTCFG1_RTCSEL2_LPTIM_OUT	(0x3U << BTIMx_BTCFG1_RTCSEL2_Pos)	/* ��LPTIMģ��������ź� */

#define	BTIMx_BTCFG1_RTCSEL1_Pos	4	/* RTCOUT1�źſ���1 */
#define	BTIMx_BTCFG1_RTCSEL1_Msk	(0x3U << BTIMx_BTCFG1_RTCSEL1_Pos)
#define	BTIMx_BTCFG1_RTCSEL1_RTC_32768	(0x0U << BTIMx_BTCFG1_RTCSEL1_Pos)	/* ��RTCģ�������32768Hz�ź� */
#define	BTIMx_BTCFG1_RTCSEL1_RTC_SEC	(0x1U << BTIMx_BTCFG1_RTCSEL1_Pos)	/* ��RTCģ����������ź� */
#define	BTIMx_BTCFG1_RTCSEL1_RTC_MIN	(0x2U << BTIMx_BTCFG1_RTCSEL1_Pos)	/* ��RTCģ������ķ����ź� */
#define	BTIMx_BTCFG1_RTCSEL1_LPTIM_OUT	(0x3U << BTIMx_BTCFG1_RTCSEL1_Pos)	/* ��LPTIMģ��������ź� */

#define	BTIMx_BTCFG1_GRP2SEL_Pos	2	/* Group2�ź�ѡ����� */
#define	BTIMx_BTCFG1_GRP2SEL_Msk	(0x3U << BTIMx_BTCFG1_GRP2SEL_Pos)
#define	BTIMx_BTCFG1_GRP2SEL_APBCLK	(0x0U << BTIMx_BTCFG1_GRP2SEL_Pos)
#define	BTIMx_BTCFG1_GRP2SEL_RTCOUT2	(0x1U << BTIMx_BTCFG1_GRP2SEL_Pos)
#define	BTIMx_BTCFG1_GRP2SEL_IN_SIG2	(0x2U << BTIMx_BTCFG1_GRP2SEL_Pos)	/* �ڲ������ź�2 */
#define	BTIMx_BTCFG1_GRP2SEL_EX_SIG2	(0x3U << BTIMx_BTCFG1_GRP2SEL_Pos)	/* �ⲿ�����ź�2 */

#define	BTIMx_BTCFG1_GRP1SEL_Pos	0
#define	BTIMx_BTCFG1_GRP1SEL_Msk	(0x3U << BTIMx_BTCFG1_GRP1SEL_Pos)
#define	BTIMx_BTCFG1_GRP1SEL_APBCLK	(0x0U << BTIMx_BTCFG1_GRP1SEL_Pos)
#define	BTIMx_BTCFG1_GRP1SEL_RTCOUT1	(0x1U << BTIMx_BTCFG1_GRP1SEL_Pos)
#define	BTIMx_BTCFG1_GRP1SEL_IN_SIG1	(0x2U << BTIMx_BTCFG1_GRP1SEL_Pos)	/* �ڲ������ź�1 */
#define	BTIMx_BTCFG1_GRP1SEL_EX_SIG1	(0x3U << BTIMx_BTCFG1_GRP1SEL_Pos)	/* �ⲿ�����ź�1 */

#define	BTIMx_BTCFG2_EXSEL2_Pos	6	/* �ⲿ�����ź�ѡ�����2 */
#define	BTIMx_BTCFG2_EXSEL2_Msk	(0x3U << BTIMx_BTCFG2_EXSEL2_Pos)
#define	BTIMx_BTCFG2_EXSEL2_BT_IN0	(0x0U << BTIMx_BTCFG2_EXSEL2_Pos)
#define	BTIMx_BTCFG2_EXSEL2_BT_IN1	(0x1U << BTIMx_BTCFG2_EXSEL2_Pos)
#define	BTIMx_BTCFG2_EXSEL2_BT_IN2	(0x2U << BTIMx_BTCFG2_EXSEL2_Pos)
#define	BTIMx_BTCFG2_EXSEL2_BT_IN3	(0x3U << BTIMx_BTCFG2_EXSEL2_Pos)

#define	BTIMx_BTCFG2_EXSEL1_Pos	4	/* �ⲿ�����ź�ѡ�����1 */
#define	BTIMx_BTCFG2_EXSEL1_Msk	(0x3U << BTIMx_BTCFG2_EXSEL1_Pos)
#define	BTIMx_BTCFG2_EXSEL1_BT_IN0	(0x0U << BTIMx_BTCFG2_EXSEL1_Pos)
#define	BTIMx_BTCFG2_EXSEL1_BT_IN1	(0x1U << BTIMx_BTCFG2_EXSEL1_Pos)
#define	BTIMx_BTCFG2_EXSEL1_BT_IN2	(0x2U << BTIMx_BTCFG2_EXSEL1_Pos)
#define	BTIMx_BTCFG2_EXSEL1_BT_IN3	(0x3U << BTIMx_BTCFG2_EXSEL1_Pos)

#define	BTIMx_BTCFG2_INSEL2_Pos	2	/* �ڲ������ź�ѡ�����2 */
#define	BTIMx_BTCFG2_INSEL2_Msk	(0x3U << BTIMx_BTCFG2_INSEL2_Pos)
#define	BTIMx_BTCFG2_INSEL2_UART_RX3	(0x0U << BTIMx_BTCFG2_INSEL2_Pos)
#define	BTIMx_BTCFG2_INSEL2_UART_RX4	(0x1U << BTIMx_BTCFG2_INSEL2_Pos)
#define	BTIMx_BTCFG2_INSEL2_UART_RX5	(0x2U << BTIMx_BTCFG2_INSEL2_Pos)
#define	BTIMx_BTCFG2_INSEL2_RCLP	(0x3U << BTIMx_BTCFG2_INSEL2_Pos)	/* �����BT1 */
#define	BTIMx_BTCFG2_INSEL2_BT1_OUT	(0x3U << BTIMx_BTCFG2_INSEL2_Pos)	/* �����BT2 */

#define	BTIMx_BTCFG2_INSEL1_Pos	0
#define	BTIMx_BTCFG2_INSEL1_Msk	(0x3U << BTIMx_BTCFG2_INSEL1_Pos)
#define	BTIMx_BTCFG2_INSEL1_UART_RX0	(0x0U << BTIMx_BTCFG2_INSEL1_Pos)
#define	BTIMx_BTCFG2_INSEL1_UART_RX1	(0x1U << BTIMx_BTCFG2_INSEL1_Pos)
#define	BTIMx_BTCFG2_INSEL1_UART_RX2	(0x2U << BTIMx_BTCFG2_INSEL1_Pos)
#define	BTIMx_BTCFG2_INSEL1_RCLP	(0x3U << BTIMx_BTCFG2_INSEL1_Pos)

#define	BTIMx_BTPRES_PRESCALE_Pos	0	/* ����Group1��Ԥ��Ƶ�Ĵ��� */
#define	BTIMx_BTPRES_PRESCALE_Msk	(0xffU << BTIMx_BTPRES_PRESCALE_Pos)

#define	BTIMx_BTLOADCR_LHEN_Pos	4
#define	BTIMx_BTLOADCR_LHEN_Msk	(0x1U << BTIMx_BTLOADCR_LHEN_Pos)
	/* 0��д0��Ч */
	/* 1��д1��ʾ��Ԥ�����Ĵ���PRESETH�ͼ��ؼĴ���LOADH�ֱ���ص�CNTH��CMPH */

#define	BTIMx_BTLOADCR_LLEN_Pos	0
#define	BTIMx_BTLOADCR_LLEN_Msk	(0x1U << BTIMx_BTLOADCR_LLEN_Pos)
	/* 0��д0��Ч */
	/* 1��д1��ʾ��Ԥ�����Ĵ���PRESETL�ͼ��ؼĴ���LOADL�ֱ���ص�CNTL��CMPL */

#define	BTIMx_BTCNTL_CNTL_Pos	0	/* ��������λ����ֵ�Ĵ��� */
#define	BTIMx_BTCNTL_CNTL_Msk	(0xffU << BTIMx_BTCNTL_CNTL_Pos)

#define	BTIMx_BTCNTH_CNTH_Pos	0	/* ��������λ����ֵ�Ĵ��� */
#define	BTIMx_BTCNTH_CNTH_Msk	(0xffU << BTIMx_BTCNTH_CNTH_Pos)

#define	BTIMx_BTPRESET_PRESETH_Pos	8	/* ��������λԤ�����Ĵ��� */
#define	BTIMx_BTPRESET_PRESETH_Msk	(0xffU << BTIMx_BTPRESET_PRESETH_Pos)

#define	BTIMx_BTPRESET_PRESETL_Pos	0	/* ��������λԤ�����Ĵ��� */
#define	BTIMx_BTPRESET_PRESETL_Msk	(0xffU << BTIMx_BTPRESET_PRESETL_Pos)

#define	BTIMx_BTLOADL_LOADL_Pos	0	/* ��������λ���ؼĴ��� */
#define	BTIMx_BTLOADL_LOADL_Msk	(0xffU << BTIMx_BTLOADL_LOADL_Pos)

#define	BTIMx_BTLOADH_LOADH_Pos	0	/* ��������λ���ؼĴ��� */
#define	BTIMx_BTLOADH_LOADH_Msk	(0xffU << BTIMx_BTLOADH_LOADH_Pos)

#define	BTIMx_BTCMPL_CMPL_Pos	0	/* ��������λ�ȽϼĴ��� */
#define	BTIMx_BTCMPL_CMPL_Msk	(0xffU << BTIMx_BTCMPL_CMPL_Pos)

#define	BTIMx_BTCMPH_CMPH_Pos	0	/* ��������λ�ȽϼĴ��� */
#define	BTIMx_BTCMPH_CMPH_Msk	(0xffU << BTIMx_BTCMPH_CMPH_Pos)

#define	BTIMx_BTOUTCNT_OUTCNT_Pos	0	/* ��������������ȼ����� */
#define	BTIMx_BTOUTCNT_OUTCNT_Msk	(0xfffU << BTIMx_BTOUTCNT_OUTCNT_Pos)

#define	BTIMx_BTOCR_OUTCLR_Pos	5	/* ������� */
#define	BTIMx_BTOCR_OUTCLR_Msk	(0x1U << BTIMx_BTOCR_OUTCLR_Pos)
#define	BTIMx_BTOCR_OUTCLR_	(0x0U << BTIMx_BTOCR_OUTCLR_Pos)	/* д0��Ч */
#define	BTIMx_BTOCR_OUTCLR_CLR	(0x1U << BTIMx_BTOCR_OUTCLR_Pos)	/* ������� */

#define	BTIMx_BTOCR_OUTINV_Pos	4	/* �����ƽ����ѡ�� */
#define	BTIMx_BTOCR_OUTINV_Msk	(0x1U << BTIMx_BTOCR_OUTINV_Pos)
#define	BTIMx_BTOCR_OUTINV_NOT_ANTI	(0x0U << BTIMx_BTOCR_OUTINV_Pos)	/* �����ƽ��ȡ�� */
#define	BTIMx_BTOCR_OUTINV_ANTI	(0x1U << BTIMx_BTOCR_OUTINV_Pos)	/* �����ƽȡ�� */

#define	BTIMx_BTOCR_OUTMOD_Pos	3	/* ���ģʽѡ�� */
#define	BTIMx_BTOCR_OUTMOD_Msk	(0x1U << BTIMx_BTOCR_OUTMOD_Pos)
#define	BTIMx_BTOCR_OUTMOD_PULSE	(0x0U << BTIMx_BTOCR_OUTMOD_Pos)	/* ������壬��ȿɵ� */
#define	BTIMx_BTOCR_OUTMOD_ANTI_LEVEL	(0x1U << BTIMx_BTOCR_OUTMOD_Pos)	/* ���֮ǰ�ķ����ƽ */

#define	BTIMx_BTOCR_OUTSEL_Pos	0	/* ����ź�Դѡ�� */
#define	BTIMx_BTOCR_OUTSEL_Msk	(0x7U << BTIMx_BTOCR_OUTSEL_Pos)
#define	BTIMx_BTOCR_OUTSEL_CMPH	(0x0U << BTIMx_BTOCR_OUTSEL_Pos)	/* �����λ�������Ƚ��źţ�������ģʽ��Ч */
#define	BTIMx_BTOCR_OUTSEL_CMPL	(0x1U << BTIMx_BTOCR_OUTSEL_Pos)	/* �����λ�������Ƚ��źţ�������ģʽ��Ч */
#define	BTIMx_BTOCR_OUTSEL_GROUP1	(0x2U << BTIMx_BTOCR_OUTSEL_Pos)	/* ���Group1�������źţ���������׽ģʽ��Ч */
#define	BTIMx_BTOCR_OUTSEL_GROUP2	(0x3U << BTIMx_BTOCR_OUTSEL_Pos)	/* ���Group2�������źţ���������׽ģʽ��Ч */
#define	BTIMx_BTOCR_OUTSEL_PWM	(0x4U << BTIMx_BTOCR_OUTSEL_Pos)	/* PWM��� */

#define	BTIMx_BTIE_CMPHIE_Pos	4	/* ��ʱ����λ�ȽϷ����ź� */
#define	BTIMx_BTIE_CMPHIE_Msk	(0x1U << BTIMx_BTIE_CMPHIE_Pos)
	/* 0���жϽ�ֹ */
	/* 1���ж�ʹ�� */

#define	BTIMx_BTIE_CMPLIE_Pos	3	/* ��ʱ����λ�ȽϷ����ź� */
#define	BTIMx_BTIE_CMPLIE_Msk	(0x1U << BTIMx_BTIE_CMPLIE_Pos)
	/* 0���жϽ�ֹ */
	/* 1���ж�ʹ�� */

#define	BTIMx_BTIE_OVHIE_Pos	2	/* ��ʱ����λ����ź� */
#define	BTIMx_BTIE_OVHIE_Msk	(0x1U << BTIMx_BTIE_OVHIE_Pos)
	/* 0���жϽ�ֹ */
	/* 1���ж�ʹ�� */

#define	BTIMx_BTIE_OVLIE_Pos	1	/* ��ʱ����λ����ź� */
#define	BTIMx_BTIE_OVLIE_Msk	(0x1U << BTIMx_BTIE_OVLIE_Pos)
	/* 0���жϽ�ֹ */
	/* 1���ж�ʹ�� */

#define	BTIMx_BTIE_CAPIE_Pos	0	/* ��ʱ����׽�����ź� */
#define	BTIMx_BTIE_CAPIE_Msk	(0x1U << BTIMx_BTIE_CAPIE_Pos)
	/* 0���жϽ�ֹ */
	/* 1���ж�ʹ�� */

#define	BTIMx_BTIF_EDGESTA_Pos	5	/* ��׽��״̬ */
#define	BTIMx_BTIF_EDGESTA_Msk	(0x1U << BTIMx_BTIF_EDGESTA_Pos)

#define	BTIMx_BTIF_CMPHIF_Pos	4	/* ��λ�ȽϷ����ź� */
#define	BTIMx_BTIF_CMPHIF_Msk	(0x1U << BTIMx_BTIF_CMPHIF_Pos)

#define	BTIMx_BTIF_CMPLIF_Pos	3	/* ��λ�ȽϷ����ź� */
#define	BTIMx_BTIF_CMPLIF_Msk	(0x1U << BTIMx_BTIF_CMPLIF_Pos)

#define	BTIMx_BTIF_OVHIF_Pos	2	/* ��ʱ����λ����ź� */
#define	BTIMx_BTIF_OVHIF_Msk	(0x1U << BTIMx_BTIF_OVHIF_Pos)

#define	BTIMx_BTIF_OVLIF_Pos	1	/* ��ʱ����λ����ź� */
#define	BTIMx_BTIF_OVLIF_Msk	(0x1U << BTIMx_BTIF_OVLIF_Pos)

#define	BTIMx_BTIF_CAPIF_Pos	0	/* ��ʱ����׽�����ź� */
#define	BTIMx_BTIF_CAPIF_Msk	(0x1U << BTIMx_BTIF_CAPIF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void BTIMx_Deinit(BTIMx_Type* BTIMx);

/* ��λ�������������� ��غ��� */
extern void BTIMx_BTCR1_CHEN_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTCR1_CHEN_Getable(BTIMx_Type* BTIMx);

/* ��λ�������������� ��غ��� */
extern void BTIMx_BTCR1_CLEN_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTCR1_CLEN_Getable(BTIMx_Type* BTIMx);

/* ����ģʽѡ�� ��غ��� */
extern void BTIMx_BTCR1_MODE_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR1_MODE_Get(BTIMx_Type* BTIMx);

/* �����ػ�׽��ѡ�� ��غ��� */
extern void BTIMx_BTCR1_EDGESEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR1_EDGESEL_Get(BTIMx_Type* BTIMx);

/* ��׽ģʽ���ƣ�����׽ģʽ����Ч�� ��غ��� */
extern void BTIMx_BTCR1_CAPMOD_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR1_CAPMOD_Get(BTIMx_Type* BTIMx);

/* �����㲶׽ģʽ���� ��غ��� */
extern void BTIMx_BTCR1_CAPCLR_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR1_CAPCLR_Get(BTIMx_Type* BTIMx);

/* ���β�׽���� ��غ��� */
extern void BTIMx_BTCR1_CAPONCE_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR1_CAPONCE_Get(BTIMx_Type* BTIMx);

/* PWMģʽ��� ��غ��� */
extern void BTIMx_BTCR1_PWM_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTCR1_PWM_Getable(BTIMx_Type* BTIMx);

/* �������ڲ�����Դ�ź�ѡ�� ��غ��� */
extern void BTIMx_BTCR2_SIG2SEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR2_SIG2SEL_Get(BTIMx_Type* BTIMx);

/* �������ڲ���׽Դ�ź�ѡ�� ��غ��� */
extern void BTIMx_BTCR2_SIG1SEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR2_SIG1SEL_Get(BTIMx_Type* BTIMx);

/* ��λ������Դѡ�� ��غ��� */
extern void BTIMx_BTCR2_CNTHSEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR2_CNTHSEL_Get(BTIMx_Type* BTIMx);

/* �ⲿ����DIR����ʹ�� ��غ��� */
extern void BTIMx_BTCR2_DIREN_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTCR2_DIREN_Getable(BTIMx_Type* BTIMx);

/* �ڲ�DIR�����ź�ʹ�� ��غ��� */
extern void BTIMx_BTCR2_STDIR_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTCR2_STDIR_Getable(BTIMx_Type* BTIMx);

/* ��λ������ʹ�ܿ���ѡ���ź� ��غ��� */
extern void BTIMx_BTCR2_SRCSEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR2_SRCSEL_Get(BTIMx_Type* BTIMx);

/* �����ź�2����ѡ�� ��غ��� */
extern void BTIMx_BTCR2_DIRPO_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCR2_DIRPO_Get(BTIMx_Type* BTIMx);

/* RTCOUT2�źſ���2 ��غ��� */
extern void BTIMx_BTCFG1_RTCSEL2_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG1_RTCSEL2_Get(BTIMx_Type* BTIMx);

/* RTCOUT1�źſ���1 ��غ��� */
extern void BTIMx_BTCFG1_RTCSEL1_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG1_RTCSEL1_Get(BTIMx_Type* BTIMx);

/* Group2�ź�ѡ����� ��غ��� */
extern void BTIMx_BTCFG1_GRP2SEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG1_GRP2SEL_Get(BTIMx_Type* BTIMx);
extern void BTIMx_BTCFG1_GRP1SEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG1_GRP1SEL_Get(BTIMx_Type* BTIMx);

/* �ⲿ�����ź�ѡ�����2 ��غ��� */
extern void BTIMx_BTCFG2_EXSEL2_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG2_EXSEL2_Get(BTIMx_Type* BTIMx);

/* �ⲿ�����ź�ѡ�����1 ��غ��� */
extern void BTIMx_BTCFG2_EXSEL1_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG2_EXSEL1_Get(BTIMx_Type* BTIMx);

/* �ڲ������ź�ѡ�����2 ��غ��� */
extern void BTIMx_BTCFG2_INSEL2_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG2_INSEL2_Get(BTIMx_Type* BTIMx);
extern void BTIMx_BTCFG2_INSEL1_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCFG2_INSEL1_Get(BTIMx_Type* BTIMx);

/* ����Group1��Ԥ��Ƶ�Ĵ��� ��غ��� */
extern void BTIMx_BTPRES_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTPRES_Read(BTIMx_Type* BTIMx);
extern void BTIMx_BTLOADCR_LHEN_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern void BTIMx_BTLOADCR_LLEN_Setable(BTIMx_Type* BTIMx, FunState NewState);

/* ��������λ����ֵ�Ĵ��� ��غ��� */
extern uint32_t BTIMx_BTCNTL_Read(BTIMx_Type* BTIMx);

/* ��������λ����ֵ�Ĵ��� ��غ��� */
extern uint32_t BTIMx_BTCNTH_Read(BTIMx_Type* BTIMx);

/* ��������λԤ�����Ĵ��� ��غ��� */
extern void BTIMx_BTPRESET_PRESETH_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTPRESET_PRESETH_Get(BTIMx_Type* BTIMx);

/* ��������λԤ�����Ĵ��� ��غ��� */
extern void BTIMx_BTPRESET_PRESETL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTPRESET_PRESETL_Get(BTIMx_Type* BTIMx);

/* ��������λ���ؼĴ��� ��غ��� */
extern void BTIMx_BTLOADL_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTLOADL_Read(BTIMx_Type* BTIMx);

/* ��������λ���ؼĴ��� ��غ��� */
extern void BTIMx_BTLOADH_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTLOADH_Read(BTIMx_Type* BTIMx);

/* ��������λ�ȽϼĴ��� ��غ��� */
extern void BTIMx_BTCMPL_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCMPL_Read(BTIMx_Type* BTIMx);

/* ��������λ�ȽϼĴ��� ��غ��� */
extern void BTIMx_BTCMPH_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTCMPH_Read(BTIMx_Type* BTIMx);

/* ��������������ȼ����� ��غ��� */
extern void BTIMx_BTOUTCNT_Write(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTOUTCNT_Read(BTIMx_Type* BTIMx);

/* ������� ��غ��� */
extern void BTIMx_BTOCR_OUTCLR_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTOCR_OUTCLR_Get(BTIMx_Type* BTIMx);

/* �����ƽ����ѡ�� ��غ��� */
extern void BTIMx_BTOCR_OUTINV_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTOCR_OUTINV_Get(BTIMx_Type* BTIMx);

/* ���ģʽѡ�� ��غ��� */
extern void BTIMx_BTOCR_OUTMOD_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTOCR_OUTMOD_Get(BTIMx_Type* BTIMx);

/* ����ź�Դѡ�� ��غ��� */
extern void BTIMx_BTOCR_OUTSEL_Set(BTIMx_Type* BTIMx, uint32_t SetValue);
extern uint32_t BTIMx_BTOCR_OUTSEL_Get(BTIMx_Type* BTIMx);

/* ��ʱ����λ�ȽϷ����ź� ��غ��� */
extern void BTIMx_BTIE_CMPHIE_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTIE_CMPHIE_Getable(BTIMx_Type* BTIMx);

/* ��ʱ����λ�ȽϷ����ź� ��غ��� */
extern void BTIMx_BTIE_CMPLIE_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTIE_CMPLIE_Getable(BTIMx_Type* BTIMx);

/* ��ʱ����λ����ź� ��غ��� */
extern void BTIMx_BTIE_OVHIE_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTIE_OVHIE_Getable(BTIMx_Type* BTIMx);

/* ��ʱ����λ����ź� ��غ��� */
extern void BTIMx_BTIE_OVLIE_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTIE_OVLIE_Getable(BTIMx_Type* BTIMx);

/* ��ʱ����׽�����ź� ��غ��� */
extern void BTIMx_BTIE_CAPIE_Setable(BTIMx_Type* BTIMx, FunState NewState);
extern FunState BTIMx_BTIE_CAPIE_Getable(BTIMx_Type* BTIMx);

/* ��׽��״̬ ��غ��� */
extern void BTIMx_BTIF_EDGESTA_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_EDGESTA_Chk(BTIMx_Type* BTIMx);

/* ��λ�ȽϷ����ź� ��غ��� */
extern void BTIMx_BTIF_CMPHIF_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_CMPHIF_Chk(BTIMx_Type* BTIMx);

/* ��λ�ȽϷ����ź� ��غ��� */
extern void BTIMx_BTIF_CMPLIF_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_CMPLIF_Chk(BTIMx_Type* BTIMx);

/* ��ʱ����λ����ź� ��غ��� */
extern void BTIMx_BTIF_OVHIF_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_OVHIF_Chk(BTIMx_Type* BTIMx);

/* ��ʱ����λ����ź� ��غ��� */
extern void BTIMx_BTIF_OVLIF_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_OVLIF_Chk(BTIMx_Type* BTIMx);

/* ��ʱ����׽�����ź� ��غ��� */
extern void BTIMx_BTIF_CAPIF_Clr(BTIMx_Type* BTIMx);
extern FlagStatus BTIMx_BTIF_CAPIF_Chk(BTIMx_Type* BTIMx);
//Announce_End

extern void BTIMx_Init(BTIMx_Type* BTIMx, BTIM_InitTypeDef* para);//btimer��ʼ��


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_BTIM_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



