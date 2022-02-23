/**
  ******************************************************************************
  * @file    fm33g0xx_anac.h
  * @author  FM33g0xx Application Team
  * @version V0.3.02G
  * @date    01-21-2019
  * @brief   This file contains all the functions prototypes for the ANAC firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_ANAC_H
#define __FM33G0XX_ANAC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33G0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup ANAC
  * @{
  */ 

	 
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint32_t	SVDMOD;		/*!<SVD����ģʽѡ��  */
	uint32_t	SVDITVL;	/*!<SVD��Ъʹ�ܼ��  */
	uint32_t	SVDLVL;		/*!<SVD������ֵ����  */
	FunState	DFEN;		/*!<SVD�����˲���SVDMODE=1ʱ������1��  */
	FunState	PFIE;		/*!<SVD��Դ�����ж�  */
	FunState	PRIE;		/*!<SVD��Դ�ָ��ж�  */
	FunState	SVDEN;		/*!<SVDʹ��  */
	
}ANAC_SVD_InitTypeDef;


typedef struct
{
	uint32_t	ADC_TRIM;	/*!<��Ч�ο���ѹ��У�Ĵ���,ת��ѹʱ����Ϊ0x3FF,ת�¶�ʱ����Ϊ0x640��0x4FB(�Զ��²�)  */
	uint32_t	ADC_VANA_EN;/*!<�ڲ�ͨ�����ⲿͨ��ѡ��  */
	uint32_t	BUFSEL;		/*!<ADC����ͨ��ѡ��  */
	FunState	BUFEN;		/*!<��������  */
	FunState	BUFBYP;		/*!<ADC����Buffer Bypass��ʹ��ADC�����ⲿ�ź�����ʱ���ر�bypass�� ʹ��ADC�����ڲ��ź�ʱ�����뽫��λ��1��bypass�� */
	FunState	ADC_IE;		/*!<ADC�ж�ʹ��  */
	FunState	ADC_EN;		/*!<ADCʹ��  */
	
}ANAC_ADC_InitTypeDef;


typedef struct
{
	uint32_t	COMPx;		/*!<�Ƚ���1/2ѡ������1��2  */
	uint32_t	VxPSEL;		/*!<�Ƚ���x��������ѡ�� */
	uint32_t	VxNSEL;		/*!<�Ƚ���x��������ѡ�� */
	uint32_t	CMPxSEL;	/*!<�Ƚ���x�ж�Դѡ�� */
	FunState	CMPxIE;		/*!<����x�ж�ʹ�� */
	FunState	CMPxDF;		/*!<�Ƚ���x�����˲�ʹ�� */
//	FunState	BUFBYP;		�Ƚ���1��2����һ��buffer��buffer�������ⲿ�������������������ýṹ����
//	FunState	BUFENB;
	FunState	CMPxEN;		/*!<�Ƚ���xʹ�� */
	
}ANAC_COMPx_InitTypeDef;

/* Exported macro ------------------------------------------------------------*/
//��0x811800��ʼ�ᱣ��������Ϣ
#define const_temp_TestA 	(*((uint8_t *)(0x1FFFFC90)))	//����ʱ�¶�����λ
#define const_temp_TestB 	(*((uint8_t *)(0x1FFFFC91)))	//����ʱ�¶�С��λ
#define const_adc_Test	 	(*((uint16_t *)(0x1FFFFC92)))	//����ʱPTAT ADCֵ
#define const_adc_Slope 	(*((uint16_t *)(0x1FFFFC54)))	//ADCб�ʣ���1000��ʹ�ã���λԼ2.2mV/lsb
#define const_adc_Offset 	(*((int16_t *)(0x1FFFFC56)))	//ADC�ؾ࣬��100��ʹ�ã���λmV
#define const_adc_Slope_3FF 	(*((uint16_t *)(0x1FFFFC54)))	//ADCб�ʣ���1000��ʹ�ã���λԼ2.2mV/lsb
#define const_adc_Offset_3FF 	(*((int16_t *)(0x1FFFFC56)))	//ADC�ؾ࣬��100��ʹ�ã���λmV
#define const_adc_Slope_1FF 	(*((uint16_t *)(0x1FFFFD50)))	//ADCб�ʣ���1000��ʹ�ã���λԼ4.4mV/lsb
#define const_adc_Offset_1FF 	(*((int16_t *)(0x1FFFFD52)))	//ADC�ؾ࣬��100��ʹ�ã���λmV
#define const_adc_Slope_0FF 	(*((uint16_t *)(0x1FFFFD58)))	//ADCб�ʣ���1000��ʹ�ã���λԼ8.8mV/lsb
#define const_adc_Offset_0FF 	(*((int16_t *)(0x1FFFFD5A)))	//ADC�ؾ࣬��100��ʹ�ã���λmV
#define const_ptat_Offset3V	(*((uint8_t *)(0x1FFFFC94)))	//ptat 5v3v��ֵ
#define const_adc_temp30H	(*((uint8_t *)(0x1FFFFC96)))	//30�ȶ���ֵ��λ
#define const_adc_temp30L	(*((uint8_t *)(0x1FFFFC97)))	//30�ȶ���ֵ��λ

#define const_adc_TrimV ((uint16_t)0x03FF)		//���ѹʱʹ��
#define const_adc_TrimV_3FF ((uint16_t)0x03FF)		//���ѹʱʹ�� adcƵ��1M ʱ ����ʱ��2ms
#define const_adc_TrimV_1FF ((uint16_t)0x01FF)		//���ѹʱʹ�� adcƵ��1M ʱ ����ʱ��1ms
#define const_adc_TrimV_0FF ((uint16_t)0x00FF)		//���ѹʱʹ�� adcƵ��1M ʱ ����ʱ��0.5ms
#define const_adc_TrimT ((uint16_t)0x0640)		//���¶�ʱʹ��
#define const_adc_TrimT_Auto ((uint16_t)0x04FB)		//���¶�ʱʹ��(�Զ��²�)
#define const_TmpsL		5.0379					//�¶�ADCб�ʵ��¶�
#define const_TmpsH		5.0379					//�¶�ADCб�ʸ��¶�
#define const_T_Offset5V	(-3.0)				//оƬ��������@8M
#define const_T_Offset3V	(-2.0)				//оƬ��������
#define const_xtl_top	25.0					//���嶥���¶�

#define CH_PTAT_Auto	0//�¶ȴ�����,�Զ��²�
#define CH_PTAT	      10//�¶ȴ�����������²�
#define CH_VDD	1//��Դ��ѹ
#define CH_IN1	2//PC12
#define CH_IN2	3//PC13
#define CH_IN3	4//PD0
#define CH_IN4	5//PD1
#define CH_IN5	6//PF6
#define CH_IN6	7//PC15
#define CH_IN7	8//PB2
#define CH_IN8	9//PB3

/* Exported functions --------------------------------------------------------*/ 



#define	ANAC_PDRCON_PDRCFG_Pos	1	/* PDR�µ縴λ��ѹ���� */
#define	ANAC_PDRCON_PDRCFG_Msk	(0x3U << ANAC_PDRCON_PDRCFG_Pos)
#define	ANAC_PDRCON_PDRCFG_1P5V	(0x0U << ANAC_PDRCON_PDRCFG_Pos)	/* 00��1.5V */
#define	ANAC_PDRCON_PDRCFG_1P25V	(0x1U << ANAC_PDRCON_PDRCFG_Pos)	/* 01��1.25V��Ĭ�ϣ� */
#define	ANAC_PDRCON_PDRCFG_1P35V	(0x2U << ANAC_PDRCON_PDRCFG_Pos)	/* 10��1.35V */
#define	ANAC_PDRCON_PDRCFG_1P4V	(0x3U << ANAC_PDRCON_PDRCFG_Pos)	/* 11��1.4V */

#define	ANAC_PDRCON_PDREN_Pos	0	/* PDR�µ縴λʹ�� */
#define	ANAC_PDRCON_PDREN_Msk	(0x1U << ANAC_PDRCON_PDREN_Pos)
	/* 0���ر��µ縴λ */
	/* 1��ʹ���µ縴λ */

#define	ANAC_BORCON_BOR_PDRCFG_Pos	1	/* BOR�µ縴λ��ѹ���� */
#define	ANAC_BORCON_BOR_PDRCFG_Msk	(0x3U << ANAC_BORCON_BOR_PDRCFG_Pos)
#define	ANAC_BORCON_BOR_PDRCFG_1P7V	(0x0U << ANAC_BORCON_BOR_PDRCFG_Pos)	/* 00��1.7V */
#define	ANAC_BORCON_BOR_PDRCFG_1P6V	(0x1U << ANAC_BORCON_BOR_PDRCFG_Pos)	/* 01��1.6V��Ĭ�ϣ� */
#define	ANAC_BORCON_BOR_PDRCFG_1P65V	(0x2U << ANAC_BORCON_BOR_PDRCFG_Pos)	/* 10��1.65V */
#define	ANAC_BORCON_BOR_PDRCFG_1P75V	(0x3U << ANAC_BORCON_BOR_PDRCFG_Pos)	/* 11��1.75V */

#define	ANAC_BORCON_OFF_BOR_Pos	0	/* BOR�رտ��ƼĴ��� */
#define	ANAC_BORCON_OFF_BOR_Msk	(0x1U << ANAC_BORCON_OFF_BOR_Pos)
	/* 0���ر�BOR�رգ���BOR�� */
	/* 1��ʹ��BOR�ر� */

#define	ANAC_SVDCFG_PFIE_Pos	9	/* SVD��Դ�����ж�ʹ�� */
#define	ANAC_SVDCFG_PFIE_Msk	(0x1U << ANAC_SVDCFG_PFIE_Pos)
	/* 0����ֹ��Դ�����ж� */
	/* 1��ʹ�ܵ�Դ�����ж� */

#define	ANAC_SVDCFG_PRIE_Pos	8	/* SVD��Դ�ָ��ж�ʹ�� */
#define	ANAC_SVDCFG_PRIE_Msk	(0x1U << ANAC_SVDCFG_PRIE_Pos)
	/* 0����ֹ��Դ�ָ��ж� */
	/* 1��ʹ�ܵ�Դ�ָ��ж� */

#define	ANAC_SVDCFG_SVDLVL_Pos	4	/* SVD������ֵ���� */
#define	ANAC_SVDCFG_SVDLVL_Msk	(0xfU << ANAC_SVDCFG_SVDLVL_Pos)
#define	ANAC_SVDCFG_SVDLVL_1P800V	(0x0U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0000_1.800V_1.900V��SVDLVL_�½���ֵ_������ֵ�� */
#define	ANAC_SVDCFG_SVDLVL_2P014V	(0x1U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0001_2.014V_2.114V */
#define	ANAC_SVDCFG_SVDLVL_2P229V	(0x2U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0010_2.229V_2.329V */
#define	ANAC_SVDCFG_SVDLVL_2P443V	(0x3U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0011_2.443V_2.543V */
#define	ANAC_SVDCFG_SVDLVL_2P657V	(0x4U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0100_2.657V_2.757V */
#define	ANAC_SVDCFG_SVDLVL_2P871V	(0x5U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0101_2.871V_2.971V */
#define	ANAC_SVDCFG_SVDLVL_3P086V	(0x6U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0110_3.086V_3.186V */
#define	ANAC_SVDCFG_SVDLVL_3P300V	(0x7U << ANAC_SVDCFG_SVDLVL_Pos)	/* 0111_3.300V_3.400V */
#define	ANAC_SVDCFG_SVDLVL_3P514V	(0x8U << ANAC_SVDCFG_SVDLVL_Pos)	/* 1000_3.514V_3.614V */
#define	ANAC_SVDCFG_SVDLVL_3P729V	(0x9U << ANAC_SVDCFG_SVDLVL_Pos)	/* 1001_3.729V_3.829V */
#define	ANAC_SVDCFG_SVDLVL_3P943V	(0xaU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1010_3.943V_4.043V */
#define	ANAC_SVDCFG_SVDLVL_4P157V	(0xbU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1011_4.157V_4.257V */
#define	ANAC_SVDCFG_SVDLVL_4P371V	(0xcU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1100_4.371V_4.471V */
#define	ANAC_SVDCFG_SVDLVL_4P586V	(0xdU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1101_4.586V_4.686V */
#define	ANAC_SVDCFG_SVDLVL_4P800V	(0xeU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1110_4.800V_4.900V */
#define	ANAC_SVDCFG_SVDLVL_SVS	(0xfU << ANAC_SVDCFG_SVDLVL_Pos)	/* 1111_SVS_SVS  �̶����0.8V  */

#define	ANAC_SVDCFG_DFEN_Pos	3	/* �����˲�ʹ�ܣ�SVDMODE=1ʱ������1�� */
#define	ANAC_SVDCFG_DFEN_Msk	(0x1U << ANAC_SVDCFG_DFEN_Pos)
	/* 0���ر�SVD����������˲� */
	/* 1������SVD����������˲� */

#define	ANAC_SVDCFG_SVDMOD_Pos	2	/* SVD����ģʽѡ�� */
#define	ANAC_SVDCFG_SVDMOD_Msk	(0x1U << ANAC_SVDCFG_SVDMOD_Pos)
#define	ANAC_SVDCFG_SVDMOD_ALWAYSON	(0x0U << ANAC_SVDCFG_SVDMOD_Pos)	/* 0����ʹ��ģʽ */
#define	ANAC_SVDCFG_SVDMOD_INTERVAL	(0x1U << ANAC_SVDCFG_SVDMOD_Pos)	/* 1����Ъʹ��ģʽ */

#define	ANAC_SVDCFG_SVDITVL_Pos	0	/* SVD��Ъʹ�ܼ�� */
#define	ANAC_SVDCFG_SVDITVL_Msk	(0x3U << ANAC_SVDCFG_SVDITVL_Pos)
#define	ANAC_SVDCFG_SVDITVL_15P625MS	(0x0U << ANAC_SVDCFG_SVDITVL_Pos)	/* 00��15.625ms */
#define	ANAC_SVDCFG_SVDITVL_62P5MS	(0x1U << ANAC_SVDCFG_SVDITVL_Pos)	/* 01��62.5ms */
#define	ANAC_SVDCFG_SVDITVL_256MS	(0x2U << ANAC_SVDCFG_SVDITVL_Pos)	/* 10��256ms */
#define	ANAC_SVDCFG_SVDITVL_1S	(0x3U << ANAC_SVDCFG_SVDITVL_Pos)	/* 11��1s */

#define	ANAC_SVDCON_SVDTE_Pos	8	/* SVD����ʹ�ܣ�����Ϊ0 */
#define	ANAC_SVDCON_SVDTE_Msk	(0x1U << ANAC_SVDCON_SVDTE_Pos)

#define	ANAC_SVDCON_SVDEN_Pos	0	/* SVDʹ�� */
#define	ANAC_SVDCON_SVDEN_Msk	(0x1U << ANAC_SVDCON_SVDEN_Pos)
	/* 0���ر�SVD */
	/* 1������SVD */

#define	ANAC_SVDSIF_SVDO_Pos	8	/* SVD��Դ������(ģ��ģ��ֱ��������ź�,����SVD������ʱ��ɲ�ѯ) */
#define	ANAC_SVDSIF_SVDO_Msk	(0x1U << ANAC_SVDSIF_SVDO_Pos)

#define	ANAC_SVDSIF_SVDR_Pos	7	/* SVD��Դ������,���ڿ����˲����вο����� */
#define	ANAC_SVDSIF_SVDR_Msk	(0x1U << ANAC_SVDSIF_SVDR_Pos)

#define	ANAC_SVDSIF_PFF_Pos	1	/* ��Դ�����жϱ�־�Ĵ�������Դ��ѹ���䵽SVD��ֵ֮��ʱ��λ�����д1���� */
#define	ANAC_SVDSIF_PFF_Msk	(0x1U << ANAC_SVDSIF_PFF_Pos)

#define	ANAC_SVDSIF_PRF_Pos	0	/* ��Դ�ָ��жϱ�־�Ĵ�������Դ��ѹ������SVD��ֵ֮��ʱ��λ�����д1���� */
#define	ANAC_SVDSIF_PRF_Msk	(0x1U << ANAC_SVDSIF_PRF_Pos)

#define	ANAC_SVDVOL_V0P8EN_Pos	2	/*0.8V��׼����ʹ���ź� */
#define	ANAC_SVDVOL_V0P8EN_Msk	(0x1U << ANAC_SVDVOL_V0P8EN_Pos)

#define	ANAC_SVDVOL_V0P75EN_Pos	1	/*0.75V��׼����ʹ���ź� */
#define	ANAC_SVDVOL_V0P75EN_Msk	(0x1U << ANAC_SVDVOL_V0P75EN_Pos)

#define	ANAC_SVDVOL_V0P7EN_Pos	0	/*0.7V��׼����ʹ���ź� */
#define	ANAC_SVDVOL_V0P7EN_Msk	(0x1U << ANAC_SVDVOL_V0P7EN_Pos)

#define	ANAC_FDETIE_FDET_IE_Pos	0	/* XTLFͣ���ⱨ���ж�ʹ��,�ϵ�Ĭ�Ϲرգ�������ϵ�ʱ����δ���񴥷��ж�*/
#define	ANAC_FDETIE_FDET_IE_Msk	(0x1U << ANAC_FDETIE_FDET_IE_Pos)

#define	ANAC_FDETIF_FDETO_Pos	6	/* ͣ����ģ����� */
#define	ANAC_FDETIF_FDETO_Msk	(0x1U << ANAC_FDETIF_FDETO_Pos)

#define	ANAC_FDETIF_FDETIF_Pos	0	/* ͣ�����жϱ�־�Ĵ�����XTLFͣ��ʱӲ���첽��λ�����д1���㣻ֻ����FDETO��Ϊ0������²��ܹ�����˼Ĵ��� */
#define	ANAC_FDETIF_FDETIF_Msk	(0x1U << ANAC_FDETIF_FDETIF_Pos)

#define	ANAC_ADCCON_ADC_IE_Pos	7	/* ADC�ж�ʹ�� */
#define	ANAC_ADCCON_ADC_IE_Msk	(0x1U << ANAC_ADCCON_ADC_IE_Pos)

#define	ANAC_ADCCON_ADC_VANA_EN_Pos	1	/* ADCͨ��ѡ�� */
#define	ANAC_ADCCON_ADC_VANA_EN_Msk	(0x1U << ANAC_ADCCON_ADC_VANA_EN_Pos)
#define	ANAC_ADCCON_ADC_VANA_EN_PTAT	(0x0U << ANAC_ADCCON_ADC_VANA_EN_Pos)	/* 0��ADC�����¶ȴ����� */
#define	ANAC_ADCCON_ADC_VANA_EN_VOLTAGE	(0x1U << ANAC_ADCCON_ADC_VANA_EN_Pos)	/* 1��ADC���ڲ����ⲿ��ѹ */

#define	ANAC_ADCCON_ADC_EN_Pos	0	/* ADCʹ���ź� */
#define	ANAC_ADCCON_ADC_EN_Msk	(0x1U << ANAC_ADCCON_ADC_EN_Pos)
	/* 0��ADC��ʹ�� */
	/* 1��ADCʹ�� */

#define	ANAC_ADCTRIM_ADC_TRIM_Pos	0	/* ADC TRIMֵ,�ӳ�����������ȡ */
#define	ANAC_ADCTRIM_ADC_TRIM_Msk	(0x7ffU << ANAC_ADCTRIM_ADC_TRIM_Pos)

#define	ANAC_ADCDATA_ADC_DATA_Pos	0	/* ADC������ݼĴ��� */
#define	ANAC_ADCDATA_ADC_DATA_Msk	(0x7ffU << ANAC_ADCDATA_ADC_DATA_Pos)

#define	ANAC_ADCIF_ADC_DONE_Pos	1	/* ADCת����������ֻ�����ر�ADC�Ż���0 */
#define	ANAC_ADCIF_ADC_DONE_Msk	(0x1U << ANAC_ADCIF_ADC_DONE_Pos)

#define	ANAC_ADCIF_ADC_IF_Pos	0	/* ADCת������жϱ�־��Ӳ����λ�����д1���㣬д0��Ч */
#define	ANAC_ADCIF_ADC_IF_Msk	(0x1U << ANAC_ADCIF_ADC_IF_Pos)

#define	ANAC_ADCINSEL_BUFEN_Pos	5	/* ADC����ͨ��Bufferʹ�� */
#define	ANAC_ADCINSEL_BUFEN_Msk	(0x1U << ANAC_ADCINSEL_BUFEN_Pos)

#define	ANAC_ADCINSEL_BUFBYP_Pos	4	/* ADC����Buffer Bypass */
#define	ANAC_ADCINSEL_BUFBYP_Msk	(0x1U << ANAC_ADCINSEL_BUFBYP_Pos)
	/* ʹ��ADC�����ⲿ�ź�����ʱ����ҪBypass Buffer */
	/* ʹ��ADC������Դ��ѹʱ�����뽫��λ��1 */

#define	ANAC_ADCINSEL_BUFSEL_Pos	0	/* ADC����ͨ��ѡ�� */
#define	ANAC_ADCINSEL_BUFSEL_Msk	(0xfU << ANAC_ADCINSEL_BUFSEL_Pos)
#define	ANAC_ADCINSEL_BUFSEL_VDD	(0x6U << ANAC_ADCINSEL_BUFSEL_Pos)	/* VDD����Դ */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN1	(0x8U << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1000��ADC_IN1 (PC12) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN2	(0x9U << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1001��ADC_IN2 (PC13) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN3	(0xaU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1010��ADC_IN3 (PD0) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN4	(0xbU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1011��ADC_IN4 (PD1) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN5	(0xcU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1100��ADC_IN5 (PF6) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN6	(0xdU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1101��ADC_IN6 (PC15) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN7	(0xeU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1110��ADC_IN7 (PB2) */
#define	ANAC_ADCINSEL_BUFSEL_ADC_IN8	(0xfU << ANAC_ADCINSEL_BUFSEL_Pos)	/* 1111��ADC_IN8 (PB3) */

#define	ANAC_TRNGCON_TRNGEN_Pos	0	/* TRNG���� */
#define	ANAC_TRNGCON_TRNGEN_Msk	(0x1U << ANAC_TRNGCON_TRNGEN_Pos)
	/* 0���ر�TRNG */
	/* 1������TRNG */

#define	ANAC_COMP1CR_CMP1O_Pos	8	/* �Ƚ���1��������ֻ�� */
#define	ANAC_COMP1CR_CMP1O_Msk	(0x1U << ANAC_COMP1CR_CMP1O_Pos)

#define	ANAC_COMP1CR_V1PSEL_Pos	3	/* �Ƚ���1��������ѡ�� */
#define	ANAC_COMP1CR_V1PSEL_Msk	(0x3U << ANAC_COMP1CR_V1PSEL_Pos)
#define	ANAC_COMP1CR_V1PSEL_PF6	(0x0U << ANAC_COMP1CR_V1PSEL_Pos)	/* 00��PF6 */
#define	ANAC_COMP1CR_V1PSEL_PF1	(0x1U << ANAC_COMP1CR_V1PSEL_Pos)	/* 01��PF1 */
#define	ANAC_COMP1CR_V1PSEL_PG2	(0x2U << ANAC_COMP1CR_V1PSEL_Pos)	/* 10��PG2 */
#define	ANAC_COMP1CR_V1PSEL_PG3	(0x3U << ANAC_COMP1CR_V1PSEL_Pos)	/* 11��PG3 */

#define	ANAC_COMP1CR_V1NSEL_Pos	1	/* �Ƚ���1��������ѡ�� */
#define	ANAC_COMP1CR_V1NSEL_Msk	(0x3U << ANAC_COMP1CR_V1NSEL_Pos)
#define	ANAC_COMP1CR_V1NSEL_PF5	(0x0U << ANAC_COMP1CR_V1NSEL_Pos)	/* 00��PF5 */
#define	ANAC_COMP1CR_V1NSEL_PF1	(0x1U << ANAC_COMP1CR_V1NSEL_Pos)	/* 01��PF1 */
#define	ANAC_COMP1CR_V1NSEL_VREF0P8V	(0x2U << ANAC_COMP1CR_V1NSEL_Pos)	/* 10��Vref 0.8V */
#define	ANAC_COMP1CR_V1NSEL_VREF0P4V	(0x3U << ANAC_COMP1CR_V1NSEL_Pos)	/* 11��Vref/2 0.4V */

#define	ANAC_COMP1CR_CMP1EN_Pos	0	/* �Ƚ���1ʹ��λ */
#define	ANAC_COMP1CR_CMP1EN_Msk	(0x1U << ANAC_COMP1CR_CMP1EN_Pos)
	/* 0���رձȽ���1 */
	/* 1��ʹ�ܱȽ���1 */

#define	ANAC_COMP2CR_CMP2O_Pos	8	/* �Ƚ���2��������ֻ�� */
#define	ANAC_COMP2CR_CMP2O_Msk	(0x1U << ANAC_COMP2CR_CMP2O_Pos)

#define	ANAC_COMP2CR_V2PSEL_Pos	3	/* �Ƚ���2��������ѡ�� */
#define	ANAC_COMP2CR_V2PSEL_Msk	(0x1U << ANAC_COMP2CR_V2PSEL_Pos)
#define	ANAC_COMP2CR_V2PSEL_PC15	(0x0U << ANAC_COMP2CR_V2PSEL_Pos)	/* 0��PC15 */
#define	ANAC_COMP2CR_V2PSEL_PE4	(0x1U << ANAC_COMP2CR_V2PSEL_Pos)	/* 1��PE4 */

#define	ANAC_COMP2CR_V2NSEL_Pos	1	/* �Ƚ���2��������ѡ�� */
#define	ANAC_COMP2CR_V2NSEL_Msk	(0x3U << ANAC_COMP2CR_V2NSEL_Pos)
#define	ANAC_COMP2CR_V2NSEL_PC14	(0x0U << ANAC_COMP2CR_V2NSEL_Pos)	/* 00��PC14 */
#define	ANAC_COMP2CR_V2NSEL_PE3	(0x1U << ANAC_COMP2CR_V2NSEL_Pos)	/* 01��PE3 */
#define	ANAC_COMP2CR_V2NSEL_VREF0P8V	(0x2U << ANAC_COMP2CR_V2NSEL_Pos)	/* 10��Vref 0.8V */
#define	ANAC_COMP2CR_V2NSEL_VREF0P4V	(0x3U << ANAC_COMP2CR_V2NSEL_Pos)	/* 11��Vref/2 0.4V */

#define	ANAC_COMP2CR_CMP2EN_Pos	0	/* �Ƚ���2ʹ��λ */
#define	ANAC_COMP2CR_CMP2EN_Msk	(0x1U << ANAC_COMP2CR_CMP2EN_Pos)
	/* 0���رձȽ���1 */
	/* 1��ʹ�ܱȽ���1 */

#define	ANAC_COMPICR_CMP2DF_Pos	9	/* �Ƚ���2�����˲�ʹ�� */
#define	ANAC_COMPICR_CMP2DF_Msk	(0x1U << ANAC_COMPICR_CMP2DF_Pos)
	/* 0����ֹ�����˲� */
	/* 1��ʹ�������˲� */

#define	ANAC_COMPICR_CMP1DF_Pos	8	/* �Ƚ���1�����˲�ʹ�� */
#define	ANAC_COMPICR_CMP1DF_Msk	(0x1U << ANAC_COMPICR_CMP1DF_Pos)
	/* 0����ֹ�����˲� */
	/* 1��ʹ�������˲� */

#define	ANAC_COMPICR_BUFBYP_Pos	7	/* �Ƚ���Buffer Bypass */
#define	ANAC_COMPICR_BUFBYP_Msk	(0x1U << ANAC_COMPICR_BUFBYP_Pos)
	/* 0����Bypass�Ƚ���Buffer */
	/* 1��Bypass�Ƚ���Buffer */

#define	ANAC_COMPICR_BUFENB_Pos	6	/* �Ƚ���Bufferʹ�� */
#define	ANAC_COMPICR_BUFENB_Msk	(0x1U << ANAC_COMPICR_BUFENB_Pos)
	/* 0��ʹ�ܱȽ���Buffer */
	/* 1����ֹ�Ƚ���Buffer */

#define	ANAC_COMPICR_CMP2SEL_Pos	4	/* �Ƚ���2�ж�Դѡ�� */
#define	ANAC_COMPICR_CMP2SEL_Msk	(0x3U << ANAC_COMPICR_CMP2SEL_Pos)
#define	ANAC_COMPICR_CMP2SEL_OUTBOTH	(0x0U << ANAC_COMPICR_CMP2SEL_Pos)	/* 00/11���Ƚ���2����������½��ز����ж� */
#define	ANAC_COMPICR_CMP2SEL_OUTRISE	(0x1U << ANAC_COMPICR_CMP2SEL_Pos)	/* 01���Ƚ���2��������ز����ж� */
#define	ANAC_COMPICR_CMP2SEL_OUTFALL	(0x2U << ANAC_COMPICR_CMP2SEL_Pos)	/* 10���Ƚ���2����½��ز����ж� */

#define	ANAC_COMPICR_CMP1SEL_Pos	2	/* �Ƚ���1�ж�Դѡ�� */
#define	ANAC_COMPICR_CMP1SEL_Msk	(0x3U << ANAC_COMPICR_CMP1SEL_Pos)
#define	ANAC_COMPICR_CMP1SEL_OUTBOTH	(0x0U << ANAC_COMPICR_CMP1SEL_Pos)	/* 00/11���Ƚ���1����������½��ز����ж� */
#define	ANAC_COMPICR_CMP1SEL_OUTRISE	(0x1U << ANAC_COMPICR_CMP1SEL_Pos)	/* 01���Ƚ���1��������ز����ж� */
#define	ANAC_COMPICR_CMP1SEL_OUTFALL	(0x2U << ANAC_COMPICR_CMP1SEL_Pos)	/* 10���Ƚ���1����½��ز����ж� */

#define	ANAC_COMPICR_CMP2IE_Pos	1	/* �Ƚ���2�ж�ʹ�� */
#define	ANAC_COMPICR_CMP2IE_Msk	(0x1U << ANAC_COMPICR_CMP2IE_Pos)

#define	ANAC_COMPICR_CMP1IE_Pos	0	/* �Ƚ���1�ж�ʹ�� */
#define	ANAC_COMPICR_CMP1IE_Msk	(0x1U << ANAC_COMPICR_CMP1IE_Pos)

#define	ANAC_COMPIF_CMP2IF_Pos	1	/* �Ƚ���2�жϱ�־��Ӳ����λ�����д1���� */
#define	ANAC_COMPIF_CMP2IF_Msk	(0x1U << ANAC_COMPIF_CMP2IF_Pos)

#define	ANAC_COMPIF_CMP1IF_Pos	0	/* �Ƚ���1�жϱ�־��Ӳ����λ�����д1���� */
#define	ANAC_COMPIF_CMP1IF_Msk	(0x1U << ANAC_COMPIF_CMP1IF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void ANAC_Deinit(void);

/* PDR�µ縴λ��ѹ���� ��غ��� */
extern void ANAC_PDRCON_PDRCFG_Set(uint32_t SetValue);
extern uint32_t ANAC_PDRCON_PDRCFG_Get(void);

/* PDR�µ縴λʹ�� ��غ��� */
extern void ANAC_PDRCON_PDREN_Setable(FunState NewState);
extern FunState ANAC_PDRCON_PDREN_Getable(void);

/* BOR�µ縴λ��ѹ���� ��غ��� */
extern void ANAC_BORCON_BOR_PDRCFG_Set(uint32_t SetValue);
extern uint32_t ANAC_BORCON_BOR_PDRCFG_Get(void);

/* BOR�رտ��ƼĴ��� ��غ��� */
extern void ANAC_BORCON_OFF_BOR_Setable(FunState NewState);
extern FunState ANAC_BORCON_OFF_BOR_Getable(void);

/* SVD��Դ�����ж�ʹ�� ��غ��� */
extern void ANAC_SVDCFG_PFIE_Setable(FunState NewState);
extern FunState ANAC_SVDCFG_PFIE_Getable(void);

/* SVD��Դ�ָ��ж�ʹ�� ��غ��� */
extern void ANAC_SVDCFG_PRIE_Setable(FunState NewState);
extern FunState ANAC_SVDCFG_PRIE_Getable(void);

/* SVD������ֵ���� ��غ��� */
extern void ANAC_SVDCFG_SVDLVL_Set(uint32_t SetValue);
extern uint32_t ANAC_SVDCFG_SVDLVL_Get(void);

/* �����˲�ʹ�ܣ�SVDMODE=1ʱ������1�� ��غ��� */
extern void ANAC_SVDCFG_DFEN_Setable(FunState NewState);
extern FunState ANAC_SVDCFG_DFEN_Getable(void);

/* SVD����ģʽѡ�� ��غ��� */
extern void ANAC_SVDCFG_SVDMOD_Set(uint32_t SetValue);
extern uint32_t ANAC_SVDCFG_SVDMOD_Get(void);

/* SVD��Ъʹ�ܼ�� ��غ��� */
extern void ANAC_SVDCFG_SVDITVL_Set(uint32_t SetValue);
extern uint32_t ANAC_SVDCFG_SVDITVL_Get(void);

/* SVD����ʹ�ܣ�����Ϊ0 ��غ��� */
extern void ANAC_SVDCON_SVDTE_Setable(FunState NewState);
extern FunState ANAC_SVDCON_SVDTE_Getable(void);

/* SVDʹ�� ��غ��� */
extern void ANAC_SVDCON_SVDEN_Setable(FunState NewState);
extern FunState ANAC_SVDCON_SVDEN_Getable(void);

/* SVD��Դ������(ģ��ģ��ֱ��������ź�,����SVD������ʱ��ɲ�ѯ) ��غ��� */
extern FlagStatus ANAC_SVDSIF_SVDO_Chk(void);

/* ��Դ�����жϱ�־�Ĵ�������Դ��ѹ���䵽SVD��ֵ֮��ʱ��λ�����д1���� ��غ��� */
extern void ANAC_SVDSIF_PFF_Clr(void);
extern FlagStatus ANAC_SVDSIF_PFF_Chk(void);

/* ��Դ�ָ��жϱ�־�Ĵ�������Դ��ѹ������SVD��ֵ֮��ʱ��λ�����д1���� ��غ��� */
extern void ANAC_SVDSIF_PRF_Clr(void);
extern FlagStatus ANAC_SVDSIF_PRF_Chk(void);

/* ��SVD�ڲ��˲���ĵ�ѹ����־�Ĵ���״̬���� */
extern FlagStatus ANAC_SVDSIF_SVDR_Chk(void);

/*����SVD��׼�����ѹ���� */
extern void ANAC_SVDVOL_CFG(uint32_t SetValue);

/*��SVD�ڲ��˲���ĵ�ѹ����־�Ĵ���״̬����*/
extern FlagStatus ANAC_SVDSIF_SVDR_Chk(void);
 
/*����SVD��׼�����ѹ����*/
extern void ANAC_SVDVOL_CFG(uint32_t SetValue);

/*��SVD�ο���ѹѡ��Ĵ���*/
extern uint32_t ANAC_SVDVOL_Get(void);

/* XTLFͣ���ⱨ���ж�ʹ��,�ϵ�Ĭ�Ϲرգ�������ϵ�ʱ����δ���񴥷��ж�
 ��غ��� */
extern void ANAC_FDETIE_FDET_IE_Setable(FunState NewState);
extern FunState ANAC_FDETIE_FDET_IE_Getable(void);

/* ͣ����ģ����� ��غ��� */
extern FlagStatus ANAC_FDETIF_FDETO_Chk(void);

/* ͣ�����жϱ�־�Ĵ�����XTLFͣ��ʱӲ���첽��λ�����д1���㣻ֻ����FDETO��Ϊ0������²��ܹ�����˼Ĵ��� ��غ��� */
extern void ANAC_FDETIF_FDETIF_Clr(void);
extern FlagStatus ANAC_FDETIF_FDETIF_Chk(void);

/* ADC�ж�ʹ�� ��غ��� */
extern void ANAC_ADCCON_ADC_IE_Setable(FunState NewState);
extern FunState ANAC_ADCCON_ADC_IE_Getable(void);

/* ADCͨ��ѡ�� ��غ��� */
extern void ANAC_ADCCON_ADC_VANA_EN_Set(uint32_t SetValue);
extern uint32_t ANAC_ADCCON_ADC_VANA_EN_Get(void);

/* ADCʹ���ź� ��غ��� */
extern void ANAC_ADCCON_ADC_EN_Setable(FunState NewState);
extern FunState ANAC_ADCCON_ADC_EN_Getable(void);

/* ADC TRIMֵ,�ӳ�����������ȡ ��غ��� */
extern void ANAC_ADCTRIM_Write(uint32_t SetValue);
extern uint32_t ANAC_ADCTRIM_Read(void);

/* ADC������ݼĴ��� ��غ��� */
extern uint32_t ANAC_ADCDATA_Read(void);

/* ADCת����������־���� */
extern FlagStatus ANAC_ADCIF_ADC_DONE_Chk(void);

/* ADCת������жϱ�־��Ӳ����λ�����д1���㣬д0��Ч ��غ��� */
extern void ANAC_ADCIF_ADC_IF_Clr(void);
extern FlagStatus ANAC_ADCIF_ADC_IF_Chk(void);

/* ADC����ͨ��Bufferʹ�� ��غ��� */
extern void ANAC_ADCINSEL_BUFEN_Setable(FunState NewState);
extern FunState ANAC_ADCINSEL_BUFEN_Getable(void);

/* ADC����Buffer Bypass ��غ��� */
extern void ANAC_ADCINSEL_BUFBYP_Setable(FunState NewState);
extern FunState ANAC_ADCINSEL_BUFBYP_Getable(void);

/* ADC����ͨ��ѡ�� ��غ��� */
extern void ANAC_ADCINSEL_BUFSEL_Set(uint32_t SetValue);
extern uint32_t ANAC_ADCINSEL_BUFSEL_Get(void);

/* TRNG���� ��غ��� */
extern void ANAC_TRNGCON_TRNGEN_Setable(FunState NewState);
extern FunState ANAC_TRNGCON_TRNGEN_Getable(void);

/* �Ƚ���1��������ֻ�� ��غ��� */
extern FlagStatus ANAC_COMP1CR_CMP1O_Chk(void);

/* �Ƚ���1��������ѡ�� ��غ��� */
extern void ANAC_COMP1CR_V1PSEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMP1CR_V1PSEL_Get(void);

/* �Ƚ���1��������ѡ�� ��غ��� */
extern void ANAC_COMP1CR_V1NSEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMP1CR_V1NSEL_Get(void);

/* �Ƚ���1ʹ��λ ��غ��� */
extern void ANAC_COMP1CR_CMP1EN_Setable(FunState NewState);
extern FunState ANAC_COMP1CR_CMP1EN_Getable(void);

/* �Ƚ���2��������ֻ�� ��غ��� */
extern FlagStatus ANAC_COMP2CR_CMP2O_Chk(void);

/* �Ƚ���2��������ѡ�� ��غ��� */
extern void ANAC_COMP2CR_V2PSEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMP2CR_V2PSEL_Get(void);

/* �Ƚ���2��������ѡ�� ��غ��� */
extern void ANAC_COMP2CR_V2NSEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMP2CR_V2NSEL_Get(void);

/* �Ƚ���2ʹ��λ ��غ��� */
extern void ANAC_COMP2CR_CMP2EN_Setable(FunState NewState);
extern FunState ANAC_COMP2CR_CMP2EN_Getable(void);

/* �Ƚ���2�����˲�ʹ�� ��غ��� */
extern void ANAC_COMPICR_CMP2DF_Setable(FunState NewState);
extern FunState ANAC_COMPICR_CMP2DF_Getable(void);

/* �Ƚ���1�����˲�ʹ�� ��غ��� */
extern void ANAC_COMPICR_CMP1DF_Setable(FunState NewState);
extern FunState ANAC_COMPICR_CMP1DF_Getable(void);

/* �Ƚ���Buffer Bypass ��غ��� */
extern void ANAC_COMPICR_BUFBYP_Setable(FunState NewState);
extern FunState ANAC_COMPICR_BUFBYP_Getable(void);

/* �Ƚ���Bufferʹ�� ��غ��� */
extern void ANAC_COMPICR_BUFENB_Setable(FunState NewState);
extern FunState ANAC_COMPICR_BUFENB_Getable(void);

/* �Ƚ���2�ж�Դѡ�� ��غ��� */
extern void ANAC_COMPICR_CMP2SEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMPICR_CMP2SEL_Get(void);

/* �Ƚ���1�ж�Դѡ�� ��غ��� */
extern void ANAC_COMPICR_CMP1SEL_Set(uint32_t SetValue);
extern uint32_t ANAC_COMPICR_CMP1SEL_Get(void);

/* �Ƚ���2�ж�ʹ�� ��غ��� */
extern void ANAC_COMPICR_CMP2IE_Setable(FunState NewState);
extern FunState ANAC_COMPICR_CMP2IE_Getable(void);

/* �Ƚ���1�ж�ʹ�� ��غ��� */
extern void ANAC_COMPICR_CMP1IE_Setable(FunState NewState);
extern FunState ANAC_COMPICR_CMP1IE_Getable(void);

/* �Ƚ���2�жϱ�־��Ӳ����λ�����д1���� ��غ��� */
extern void ANAC_COMPIF_CMP2IF_Clr(void);
extern FlagStatus ANAC_COMPIF_CMP2IF_Chk(void);

/* �Ƚ���1�жϱ�־��Ӳ����λ�����д1���� ��غ��� */
extern void ANAC_COMPIF_CMP1IF_Clr(void);
extern FlagStatus ANAC_COMPIF_CMP1IF_Chk(void);
//Announce_End
	 

/*SVD ��ʼ�����ú���*/
extern void ANAC_SVD_Init(ANAC_SVD_InitTypeDef* para);

/*ADC ��ʼ�����ú���*/
void ANAC_ADC_Init(ANAC_ADC_InitTypeDef* para);

/*ADC ����ͨ�����ú���������*/
void ANAC_ADC_Channel_SetEx(uint8_t ChSel_def);

/*ADC ��ѹ���㺯��
	���룺ADֵ����ѹֵ��5��3��
	�������ѹ@mV 
*/
uint32_t ANAC_ADC_VoltageCalc(uint32_t fADCData,uint8_t Vdd);
/*ADC �¶ȼ��㺯��
	���룺ADֵ����ѹֵ��5��3��
	������¶�@�� 
*/
float ANAC_ADC_TemperatureCalc(float fADCData ,uint8_t Vdd);

/*COMP ��ʼ�����ú���*/
void ANAC_COMPx_Init(ANAC_COMPx_InitTypeDef* para);

#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_ANAC_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



