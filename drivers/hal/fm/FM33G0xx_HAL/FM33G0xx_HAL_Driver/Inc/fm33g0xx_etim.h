/**
  ******************************************************************************
  * @file    fm33g0xx_etimer.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the ETIM firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0xx_ETIMER_H
#define __FM33G0xx_ETIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 

typedef struct
{
	uint32_t	SIG1SEL;		/* �ڲ��ź�1Դѡ���ڼ���ģʽ�¼���Դ���ɴ�ѡ�񣬲�׽ģʽ�¼���Դ�� */
	uint32_t	SIG2SEL;		/* �ڲ��ź�2Դѡ��(��׽Դ) */
	uint32_t	GRP1SEL;		/* GROUP1 �ź�ѡ����� */
	uint32_t	GRP2SEL;		/* GROUP2 �ź�ѡ����� */
	uint32_t	PRESCALE1;		/* ETxԤ��Ƶ�Ĵ���1 */
	uint32_t	PRESCALE2;		/* ETxԤ��Ƶ�Ĵ���2 */

}ETIM_SRCInitType;

typedef struct
{
	FunState	EXFLT;		/* �������������˲�ʹ�� */
	uint32_t	MOD;		/* ����ģʽѡ�� */
	FunState	CASEN;		/* ��չ��ʱ������ʹ�� */
	uint32_t	EDGESEL;	/* ����ģʽ���ط�ʽѡ�񣨼���ʱ��ѡ��mcu_clkʱ��λ��Ч�����ǲ���mcu_clkʱ�������ؼ����� */
	
	FunState	PWM;		/* PWM������� */
	
	uint32_t	CAPMOD;		/* ��׽ģʽ���� */
	FunState	CAPCLR;		/* �����㲶׽ģʽ���� */
	FunState	CAPONCE;	/* ���β�׽���� */
	uint32_t	CAPEDGE;	/* ��׽��ѡ�� */
	
	uint32_t	INITVALUE;	/* ETx��ֵ�Ĵ��� */
	uint32_t	CMP;		/* ETx�ȽϼĴ��� */
	
	FunState	CMPIE;		/* ��չ��ʱ���Ƚ��ж�ʹ�� */
	FunState	CAPIE;		/* ��չ��ʱ����׽�ж�ʹ�� */
	FunState	OVIE;		/* ��չ��ʱ������ж�ʹ�� */
	
	FunState	CEN;		/* �������� */

}ETIM_CTRLInitType;

typedef struct
{
	ETIM_SRCInitType	sig_src_para;//�ź�Դ����
	ETIM_CTRLInitType	ctrl_para;//���������
	
}ETIM_InitTypeDef;



#define	ETIMx_ETxCR_EXFLT_Pos	9	/* �������������˲�ʹ�� */
#define	ETIMx_ETxCR_EXFLT_Msk	(0x1U << ETIMx_ETxCR_EXFLT_Pos)
	/* 0 = �ر����������ź������˲� */
	/* 1 = �����������ź������˲� */

#define	ETIMx_ETxCR_PWM_Pos	8	/* PWM������� */
#define	ETIMx_ETxCR_PWM_Msk	(0x1U << ETIMx_ETxCR_PWM_Pos)
	/* 0 = PWM�����ֹ */
	/* 1 = PWM���ʹ�� */

#define	ETIMx_ETxCR_CEN_Pos	7	/* �������� */
#define	ETIMx_ETxCR_CEN_Msk	(0x1U << ETIMx_ETxCR_CEN_Pos)
	/* 0 = ֹͣ���������� */
	/* 1 = ������ʱ�� */

#define	ETIMx_ETxCR_MOD_Pos	6	/* ����ģʽѡ�� */
#define	ETIMx_ETxCR_MOD_Msk	(0x1U << ETIMx_ETxCR_MOD_Pos)
#define	ETIMx_ETxCR_MOD_COUNTER	(0x0U << ETIMx_ETxCR_MOD_Pos)	/* 0 = ��ʱ/����ģʽ */
#define	ETIMx_ETxCR_MOD_CAPTURE	(0x1U << ETIMx_ETxCR_MOD_Pos)	/* 1 = ��׽ģʽ */

#define	ETIMx_ETxCR_CASEN_Pos	5	/* ��չ��ʱ������ʹ�� */
#define	ETIMx_ETxCR_CASEN_Msk	(0x1U << ETIMx_ETxCR_CASEN_Pos)
	/* 0 = 16bit��ʱ���������� */
	/* 1 = ET1��ET3����ET2��ET4��������32bit��ʱ�� */

#define	ETIMx_ETxCR_EDGESEL_Pos	4	/* ����ģʽ���ط�ʽѡ�񣨼���ʱ��ѡ��mcu_clkʱ��λ��Ч�����ǲ���mcu_clkʱ�������ؼ����� */
#define	ETIMx_ETxCR_EDGESEL_Msk	(0x1U << ETIMx_ETxCR_EDGESEL_Pos)
#define	ETIMx_ETxCR_EDGESEL_RISING	(0x0U << ETIMx_ETxCR_EDGESEL_Pos)	/* 0 = ����ģʽ�������� */
#define	ETIMx_ETxCR_EDGESEL_FALLING	(0x1U << ETIMx_ETxCR_EDGESEL_Pos)	/* 1 = ����ģʽ���½��� */

#define	ETIMx_ETxCR_CAPMOD_Pos	3	/* ��׽ģʽ���� */
#define	ETIMx_ETxCR_CAPMOD_Msk	(0x1U << ETIMx_ETxCR_CAPMOD_Pos)
#define	ETIMx_ETxCR_CAPMOD_PERIOD	(0x0U << ETIMx_ETxCR_CAPMOD_Pos)	/* 0 = �������ڲ�׽ */
#define	ETIMx_ETxCR_CAPMOD_PULSE	(0x1U << ETIMx_ETxCR_CAPMOD_Pos)	/* 1 = ����׽ */

#define	ETIMx_ETxCR_CAPCLR_Pos	2	/* �����㲶׽ģʽ���� */
#define	ETIMx_ETxCR_CAPCLR_Msk	(0x1U << ETIMx_ETxCR_CAPCLR_Pos)
	/* 0 = ��׽�����㣬������һֱ���ɼ��� */
	/* 1 = �¼�������׽��ʹ�ܺ����������0����׽����һ����Ч��֮��timer�ſ�ʼ���� */

#define	ETIMx_ETxCR_CAPONCE_Pos	1	/* ���β�׽���� */
#define	ETIMx_ETxCR_CAPONCE_Msk	(0x1U << ETIMx_ETxCR_CAPONCE_Pos)
	/* 0 = ������׽ */
	/* 1 = ���β�׽��Ч���ڲ�׽��һ���������ں������ֹͣ������Ҫ�ٴβ�׽���������� */

#define	ETIMx_ETxCR_CAPEDGE_Pos	0	/* ��׽��ѡ�� */
#define	ETIMx_ETxCR_CAPEDGE_Msk	(0x1U << ETIMx_ETxCR_CAPEDGE_Pos)
#define	ETIMx_ETxCR_CAPEDGE_RISING	(0x0U << ETIMx_ETxCR_CAPEDGE_Pos)	/* 0 = ���ڲ�׽ģʽʱ���ز�׽ */
#define	ETIMx_ETxCR_CAPEDGE_FALLING	(0x1U << ETIMx_ETxCR_CAPEDGE_Pos)	/* 1 = ���ڲ�׽ģʽʱ���ز�׽ */

#define	ETIMx_ETxINSEL_SIG2SEL_Pos	7	/* �ڲ��ź�2Դѡ��(��׽Դ) */
#define	ETIMx_ETxINSEL_SIG2SEL_Msk	(0x1U << ETIMx_ETxINSEL_SIG2SEL_Pos)
#define	ETIMx_ETxINSEL_SIG2SEL_GROUP2	(0x0U << ETIMx_ETxINSEL_SIG2SEL_Pos)	/* 0 = ��չ��ʱ��3���ڲ��ź�2ѡ��Group2 */
#define	ETIMx_ETxINSEL_SIG2SEL_GROUP1	(0x1U << ETIMx_ETxINSEL_SIG2SEL_Pos)	/* 1 = ��չ��ʱ��3���ڲ��ź�2ѡ��Group1 */

#define	ETIMx_ETxINSEL_SIG1SEL_Pos	6	/* �ڲ��ź�1Դѡ���ڼ���ģʽ�¼���Դ���ɴ�ѡ�񣬲�׽ģʽ�¼���Դ�� */
#define	ETIMx_ETxINSEL_SIG1SEL_Msk	(0x1U << ETIMx_ETxINSEL_SIG1SEL_Pos)
#define	ETIMx_ETxINSEL_SIG1SEL_GROUP1	(0x0U << ETIMx_ETxINSEL_SIG1SEL_Pos)	/* 0 = ��չ��ʱ��3���ڲ��ź�1ѡ��Group1 */
#define	ETIMx_ETxINSEL_SIG1SEL_GROUP2	(0x1U << ETIMx_ETxINSEL_SIG1SEL_Pos)	/* 1 = ��չ��ʱ��3���ڲ��ź�1ѡ��Group2 */

#define	ETIMx_ETxINSEL_GRP2SEL_Pos	2	/* GROUP2 �ź�ѡ����� */
#define	ETIMx_ETxINSEL_GRP2SEL_Msk	(0x7U << ETIMx_ETxINSEL_GRP2SEL_Pos)
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_UART0_RX	(0x0U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 000 = UART0_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_UART1_RX	(0x1U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 001 = UART1_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_XTLF	(0x2U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 010 = XTLF */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_ET1_IN1	(0x3U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 011 = ET1_IN1 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_ET1_IN2	(0x4U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 100 = ET1_IN2 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_CMP1O	(0x5U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 101 = CMP1O���Ƚ���1����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_CMP2O	(0x6U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 110 = CMP2O���Ƚ���2����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET1_LPTO	(0x7U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET1 111 = LPTO */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_UART2_RX	(0x0U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 000 = UART2_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_UART3_RX	(0x1U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 001 = UART3_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_XTLF	(0x2U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 010 = XTLF */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_ET2_IN1	(0x3U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 011 = ET2_IN1 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_ET2_IN2	(0x4U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 100 = ET2_IN2 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_CMP1O	(0x5U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 101 = CMP1O���Ƚ���1����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_CMP2O	(0x6U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 110 = CMP2O���Ƚ���2����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET2_LPTO	(0x7U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET2 111 = LPTO */
#define	ETIMx_ETxINSEL_GRP2SEL_ET3_ET3_IN1	(0x0U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET3 000 = ET3_IN1 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET3_XTLF	(0x1U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET3 001 = XTLF  */
#define	ETIMx_ETxINSEL_GRP2SEL_ET3_UART4_RX	(0x2U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET3 010 = UART4_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET3_UART5_RX	(0x3U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET3 011 = UART5_RX */
#define	ETIMx_ETxINSEL_GRP2SEL_ET3_RTCSEC	(0x4U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET3 100 = RTCSEC */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_ET4_IN1	(0x0U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 000 = ET4_IN1 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_XTLF	(0x1U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 001 = XTLF */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_UART_RX2	(0x2U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 010 = UART_RX2 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_UART_RX0	(0x3U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 011 = UART_RX0 */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_CMP1O	(0x4U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 100 = CMP1O���Ƚ���1����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_CMP2O	(0x5U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 101 = CMP2O���Ƚ���2����� */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_RTCSEC	(0x6U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 110= RTCSEC */
#define	ETIMx_ETxINSEL_GRP2SEL_ET4_LPTO	(0x7U << ETIMx_ETxINSEL_GRP2SEL_Pos)	/* ET4 111= LPTO  */

#define	ETIMx_ETxINSEL_GRP1SEL_Pos	0	/* GROUP1 �ź�ѡ����� */
#define	ETIMx_ETxINSEL_GRP1SEL_Msk	(0x3U << ETIMx_ETxINSEL_GRP1SEL_Pos)
#define	ETIMx_ETxINSEL_GRP1SEL_ET1_APBCLK	(0x0U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET1 00 = APBCLK */
#define	ETIMx_ETxINSEL_GRP1SEL_ET1_XTLF	(0x1U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET1 01 = XTLF */
#define	ETIMx_ETxINSEL_GRP1SEL_ET1_RCLP	(0x2U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET1 10 = RCLP */
#define	ETIMx_ETxINSEL_GRP1SEL_ET1_ET1_IN0	(0x3U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET1 11 = ET1_IN0 */
#define	ETIMx_ETxINSEL_GRP1SEL_ET2_APBCLK	(0x0U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET2 00 = APBCLK */
#define	ETIMx_ETxINSEL_GRP1SEL_ET2_XTLF	(0x1U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET2 01 = XTLF */
#define	ETIMx_ETxINSEL_GRP1SEL_ET2_RCLP	(0x2U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET2 10 = RCLP */
#define	ETIMx_ETxINSEL_GRP1SEL_ET2_ET2_IN0	(0x3U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET2 11 = ET2_IN0 */
#define	ETIMx_ETxINSEL_GRP1SEL_ET3_APBCLK	(0x0U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET3 00 = APBCLK */
#define	ETIMx_ETxINSEL_GRP1SEL_ET3_ET3_IN0	(0x1U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET3 01 = ET3_IN0 */
#define	ETIMx_ETxINSEL_GRP1SEL_ET3_RTCSEC	(0x2U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET3 10 = RTCSEC */
#define	ETIMx_ETxINSEL_GRP1SEL_ET3_RCLP	(0x3U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET3 11 = RCLP */
#define	ETIMx_ETxINSEL_GRP1SEL_ET4_APBCLK	(0x0U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET4 00 = APBCLK */
#define	ETIMx_ETxINSEL_GRP1SEL_ET4_ET4_IN0	(0x1U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET4 01 = ET4_IN0 */
#define	ETIMx_ETxINSEL_GRP1SEL_ET4_RTC64HZ	(0x2U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET4 10 = RTC64HZ */
#define	ETIMx_ETxINSEL_GRP1SEL_ET4_LPTO	(0x3U << ETIMx_ETxINSEL_GRP1SEL_Pos)	/* ET4 11 = LPTO */

#define	ETIMx_ETxPESCALE1_PRESCALE1_Pos	0	/* ETxԤ��Ƶ�Ĵ���1 */
#define	ETIMx_ETxPESCALE1_PRESCALE1_Msk	(0xffU << ETIMx_ETxPESCALE1_PRESCALE1_Pos)

#define	ETIMx_ETxPESCALE2_PRESCALE2_Pos	0	/* ETxԤ��Ƶ�Ĵ���2 */
#define	ETIMx_ETxPESCALE2_PRESCALE2_Msk	(0xffU << ETIMx_ETxPESCALE2_PRESCALE2_Pos)

#define	ETIMx_ETxIVR_INITVALUE_Pos	0	/* ETx��ֵ�Ĵ��� */
#define	ETIMx_ETxIVR_INITVALUE_Msk	(0xffffU << ETIMx_ETxIVR_INITVALUE_Pos)

#define	ETIMx_ETxCMP_CMP_Pos	0	/* ETx�ȽϼĴ��� */
#define	ETIMx_ETxCMP_CMP_Msk	(0xffffU << ETIMx_ETxCMP_CMP_Pos)

#define	ETIMx_ETxIE_CMPIE_Pos	2	/* ��չ��ʱ���Ƚ��ж�ʹ�� */
#define	ETIMx_ETxIE_CMPIE_Msk	(0x1U << ETIMx_ETxIE_CMPIE_Pos)
	/* 0 = ��ֹ */
	/* 1 = ʹ�� */

#define	ETIMx_ETxIE_CAPIE_Pos	1	/* ��չ��ʱ����׽�ж�ʹ�� */
#define	ETIMx_ETxIE_CAPIE_Msk	(0x1U << ETIMx_ETxIE_CAPIE_Pos)
	/* 0 = ��ֹ */
	/* 1 = ʹ�� */

#define	ETIMx_ETxIE_OVIE_Pos	0	/* ��չ��ʱ������ж�ʹ�� */
#define	ETIMx_ETxIE_OVIE_Msk	(0x1U << ETIMx_ETxIE_OVIE_Pos)
	/* 1 = ʹ�� */
	/* 0 = ��ֹ */

#define	ETIMx_ETxIF_CMPIF_Pos	3	/* �Ƚ�״̬�жϱ�־ */
#define	ETIMx_ETxIF_CMPIF_Msk	(0x1U << ETIMx_ETxIF_CMPIF_Pos)

#define	ETIMx_ETxIF_EDGESTA_Pos	2	/* ��׽��״̬��־ */
#define	ETIMx_ETxIF_EDGESTA_Msk	(0x1U << ETIMx_ETxIF_EDGESTA_Pos)

#define	ETIMx_ETxIF_CAPIF_Pos	1	/* ��չ��ʱ����׽�����жϱ�־ */
#define	ETIMx_ETxIF_CAPIF_Msk	(0x1U << ETIMx_ETxIF_CAPIF_Pos)

#define	ETIMx_ETxIF_OVIF_Pos	0	/* ��չ��ʱ������жϱ�־ */
#define	ETIMx_ETxIF_OVIF_Msk	(0x1U << ETIMx_ETxIF_OVIF_Pos)

#define	ETIMx_ETxCNT_ETxCNT_Pos	0	/* ��չ��ʱ������жϱ�־ */
#define	ETIMx_ETxCNT_ETxCNT_Msk	(0xffffU << ETIMx_ETxCNT_ETxCNT_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void ETIMx_Deinit(ETIMx_Type* ETIMx);

/* �������������˲�ʹ�� ��غ��� */
extern void ETIMx_ETxCR_EXFLT_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_EXFLT_Getable(ETIMx_Type* ETIMx);

/* PWM������� ��غ��� */
extern void ETIMx_ETxCR_PWM_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_PWM_Getable(ETIMx_Type* ETIMx);

/* �������� ��غ��� */
extern void ETIMx_ETxCR_CEN_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_CEN_Getable(ETIMx_Type* ETIMx);

/* ����ģʽѡ�� ��غ��� */
extern void ETIMx_ETxCR_MOD_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxCR_MOD_Get(ETIMx_Type* ETIMx);

/* ��չ��ʱ������ʹ�� ��غ��� */
extern void ETIMx_ETxCR_CASEN_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_CASEN_Getable(ETIMx_Type* ETIMx);

/* ����ģʽ���ط�ʽѡ�񣨼���ʱ��ѡ��mcu_clkʱ��λ��Ч�����ǲ���mcu_clkʱ�������ؼ����� ��غ��� */
extern void ETIMx_ETxCR_EDGESEL_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxCR_EDGESEL_Get(ETIMx_Type* ETIMx);

/* ��׽ģʽ���� ��غ��� */
extern void ETIMx_ETxCR_CAPMOD_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxCR_CAPMOD_Get(ETIMx_Type* ETIMx);

/* �����㲶׽ģʽ���� ��غ��� */
extern void ETIMx_ETxCR_CAPCLR_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_CAPCLR_Getable(ETIMx_Type* ETIMx);

/* ���β�׽���� ��غ��� */
extern void ETIMx_ETxCR_CAPONCE_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxCR_CAPONCE_Getable(ETIMx_Type* ETIMx);

/* ��׽��ѡ�� ��غ��� */
extern void ETIMx_ETxCR_CAPEDGE_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxCR_CAPEDGE_Get(ETIMx_Type* ETIMx);

/* �ڲ��ź�2Դѡ��(��׽Դ) ��غ��� */
extern void ETIMx_ETxINSEL_SIG2SEL_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxINSEL_SIG2SEL_Get(ETIMx_Type* ETIMx);

/* �ڲ��ź�1Դѡ���ڼ���ģʽ�¼���Դ���ɴ�ѡ�񣬲�׽ģʽ�¼���Դ�� ��غ��� */
extern void ETIMx_ETxINSEL_SIG1SEL_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxINSEL_SIG1SEL_Get(ETIMx_Type* ETIMx);

/* GROUP2 �ź�ѡ����� ��غ��� */
extern void ETIMx_ETxINSEL_GRP2SEL_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxINSEL_GRP2SEL_Get(ETIMx_Type* ETIMx);

/* GROUP1 �ź�ѡ����� ��غ��� */
extern void ETIMx_ETxINSEL_GRP1SEL_Set(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxINSEL_GRP1SEL_Get(ETIMx_Type* ETIMx);

/* ETxԤ��Ƶ�Ĵ���1 ��غ��� */
extern void ETIMx_ETxPESCALE1_Write(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxPESCALE1_Read(ETIMx_Type* ETIMx);

/* ETxԤ��Ƶ�Ĵ���2 ��غ��� */
extern void ETIMx_ETxPESCALE2_Write(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxPESCALE2_Read(ETIMx_Type* ETIMx);

/* ETx��ֵ�Ĵ��� ��غ��� */
extern void ETIMx_ETxIVR_Write(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxIVR_Read(ETIMx_Type* ETIMx);

/* ETx�ȽϼĴ��� ��غ��� */
extern void ETIMx_ETxCMP_Write(ETIMx_Type* ETIMx, uint32_t SetValue);
extern uint32_t ETIMx_ETxCMP_Read(ETIMx_Type* ETIMx);

/* ��չ��ʱ���Ƚ��ж�ʹ�� ��غ��� */
extern void ETIMx_ETxIE_CMPIE_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxIE_CMPIE_Getable(ETIMx_Type* ETIMx);

/* ��չ��ʱ����׽�ж�ʹ�� ��غ��� */
extern void ETIMx_ETxIE_CAPIE_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxIE_CAPIE_Getable(ETIMx_Type* ETIMx);

/* ��չ��ʱ������ж�ʹ�� ��غ��� */
extern void ETIMx_ETxIE_OVIE_Setable(ETIMx_Type* ETIMx, FunState NewState);
extern FunState ETIMx_ETxIE_OVIE_Getable(ETIMx_Type* ETIMx);

/* �Ƚ�״̬�жϱ�־ ��غ��� */
extern void ETIMx_ETxIF_CMPIF_Clr(ETIMx_Type* ETIMx);
extern FlagStatus ETIMx_ETxIF_CMPIF_Chk(ETIMx_Type* ETIMx);

/* ��׽��״̬��־ ��غ��� */
extern FlagStatus ETIMx_ETxIF_EDGESTA_Chk(ETIMx_Type* ETIMx);

/* ��չ��ʱ����׽�����жϱ�־ ��غ��� */
extern void ETIMx_ETxIF_CAPIF_Clr(ETIMx_Type* ETIMx);
extern FlagStatus ETIMx_ETxIF_CAPIF_Chk(ETIMx_Type* ETIMx);

/* ��չ��ʱ������жϱ�־ ��غ��� */
extern void ETIMx_ETxIF_OVIF_Clr(ETIMx_Type* ETIMx);
extern FlagStatus ETIMx_ETxIF_OVIF_Chk(ETIMx_Type* ETIMx);

/*��ȡET ��������ֵ*/
extern uint32_t ETIMx_ETxCNT_Read(ETIMx_Type* ETIMx);
//Announce_End

/*ETIMx ��ʼ�����ú���*/
extern void ETIMx_Init(ETIMx_Type* ETIMx, ETIM_InitTypeDef* para);


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_ETIM_H */




/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



