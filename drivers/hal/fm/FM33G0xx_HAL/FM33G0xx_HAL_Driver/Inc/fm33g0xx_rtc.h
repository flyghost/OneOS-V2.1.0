/**
  ******************************************************************************
  * @file    fm33g0xx_rtc.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the RTC firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_RTC_H
#define __FM33G0XX_RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33G0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup RTC
  * @{
  */

/* Exported constants --------------------------------------------------------*/
#define RTC_WRITE_DISABLE					((uint32_t)0x53535353)
#define RTC_WRITE_ENABLE					((uint32_t)0xACACACAC)
/* Exported types ------------------------------------------------------------*/	 
typedef struct
{       
	uint8_t			Year;		/*!<RTC ��*/
	uint8_t			Month;		/*!<RTC ��*/
	uint8_t			Date;		/*!<RTC ��*/	
	uint8_t			Hour;		/*!<RTC ʱ*/
	uint8_t			Minute;		/*!<RTC ��*/	
	uint8_t			Second;		/*!<RTC ��*/			
	uint8_t			Week;		/*!<RTC ��*/	

}RTC_TimeDateTypeDef,RTC_StampTmieTypeDef;

typedef struct
{     
	uint8_t			Hour;		/*!<RTC���� ʱ*/		
	uint8_t			Minute;		/*!<RTC���� ��*/
	uint8_t			Second;		/*!<RTC���� ��*/

}RTC_AlarmTmieTypeDef;


/* Exported macro ------------------------------------------------------------*/
/*StampType def*/				//ע��ʱ������ܽ�������ģʽ����Ч
#define STAMP0RISE	0x00		//PB4������ʱ���
#define STAMP0FALL	0x01		//PB4�½���ʱ���
#define STAMP1RISE	0x02		//PB5������ʱ���
#define STAMP1FALL	0x03		//PB5�½���ʱ���


//��0x811800��ʼ�ᱣ��������Ϣ
#define const_tx_flag  		(*((uint16_t *)(0x1ffffa20))) //��У��־(0x3cc3Ϊ��̵�У)
#define const_rtc_adj_top 	(*((int16_t *)(0x1ffffa36))) //�������ppm,0.01ppm
#define const_rtc_backup 	0x1FFFF800  //RTC���ݼĴ����׵�ַ



#define	RTC_RTCWE_RTCWE_Pos	0	/* RTCдʹ�ܼĴ��� */
#define	RTC_RTCWE_RTCWE_Msk	(0xffffffffU << RTC_RTCWE_RTCWE_Pos)

#define	RTC_RTCIE_PB5R_IE_Pos	16	/* PB5�������ж�ʹ�� */
#define	RTC_RTCIE_PB5R_IE_Msk	(0x1U << RTC_RTCIE_PB5R_IE_Pos)

#define	RTC_RTCIE_PB5F_IE_Pos	15	/* PB5�½����ж�ʹ�� */
#define	RTC_RTCIE_PB5F_IE_Msk	(0x1U << RTC_RTCIE_PB5F_IE_Pos)

#define	RTC_RTCIE_PB4R_IE_Pos	14	/* PB4�������ж�ʹ�� */
#define	RTC_RTCIE_PB4R_IE_Msk	(0x1U << RTC_RTCIE_PB4R_IE_Pos)

#define	RTC_RTCIE_PB4F_IE_Pos	13	/* PB4�½����ж�ʹ�� */
#define	RTC_RTCIE_PB4F_IE_Msk	(0x1U << RTC_RTCIE_PB4F_IE_Pos)

#define	RTC_RTCIE_ADJ128_IE_Pos	12	/* 128���ж�ʹ�� */
#define	RTC_RTCIE_ADJ128_IE_Msk	(0x1U << RTC_RTCIE_ADJ128_IE_Pos)

#define	RTC_RTCIE_ALARM_IE_Pos	11	/* �����ж�ʹ�� */
#define	RTC_RTCIE_ALARM_IE_Msk	(0x1U << RTC_RTCIE_ALARM_IE_Pos)

#define	RTC_RTCIE_1KHZ_IE_Pos	10	/* 1khz�ж�ʹ�� */
#define	RTC_RTCIE_1KHZ_IE_Msk	(0x1U << RTC_RTCIE_1KHZ_IE_Pos)

#define	RTC_RTCIE_256HZ_IE_Pos	9	/* 256hz�ж�ʹ�� */
#define	RTC_RTCIE_256HZ_IE_Msk	(0x1U << RTC_RTCIE_256HZ_IE_Pos)

#define	RTC_RTCIE_64HZ_IE_Pos	8	/* 64hz�ж�ʹ�� */
#define	RTC_RTCIE_64HZ_IE_Msk	(0x1U << RTC_RTCIE_64HZ_IE_Pos)

#define	RTC_RTCIE_16HZ_IE_Pos	7	/* 16hz�ж�ʹ�� */
#define	RTC_RTCIE_16HZ_IE_Msk	(0x1U << RTC_RTCIE_16HZ_IE_Pos)

#define	RTC_RTCIE_8HZ_IE_Pos	6	/* 8hz�ж�ʹ�� */
#define	RTC_RTCIE_8HZ_IE_Msk	(0x1U << RTC_RTCIE_8HZ_IE_Pos)

#define	RTC_RTCIE_4HZ_IE_Pos	5	/* 4hz�ж�ʹ�� */
#define	RTC_RTCIE_4HZ_IE_Msk	(0x1U << RTC_RTCIE_4HZ_IE_Pos)

#define	RTC_RTCIE_2HZ_IE_Pos	4	/* 2hz�ж�ʹ�� */
#define	RTC_RTCIE_2HZ_IE_Msk	(0x1U << RTC_RTCIE_2HZ_IE_Pos)

#define	RTC_RTCIE_SEC_IE_Pos	3	/* ���ж�ʹ�� */
#define	RTC_RTCIE_SEC_IE_Msk	(0x1U << RTC_RTCIE_SEC_IE_Pos)

#define	RTC_RTCIE_MIN_IE_Pos	2	/* ���ж�ʹ�� */
#define	RTC_RTCIE_MIN_IE_Msk	(0x1U << RTC_RTCIE_MIN_IE_Pos)

#define	RTC_RTCIE_HOUR_IE_Pos	1	/* Сʱ�ж�ʹ�� */
#define	RTC_RTCIE_HOUR_IE_Msk	(0x1U << RTC_RTCIE_HOUR_IE_Pos)

#define	RTC_RTCIE_DATE_IE_Pos	0	/* ���ж�ʹ�� */
#define	RTC_RTCIE_DATE_IE_Msk	(0x1U << RTC_RTCIE_DATE_IE_Pos)

#define	RTC_RTCIF_PB5R_IF_Pos	16	/* PB5�������жϱ�־ */
#define	RTC_RTCIF_PB5R_IF_Msk	(0x1U << RTC_RTCIF_PB5R_IF_Pos)

#define	RTC_RTCIF_PB5F_IF_Pos	15	/* PB5�½����жϱ�־ */
#define	RTC_RTCIF_PB5F_IF_Msk	(0x1U << RTC_RTCIF_PB5F_IF_Pos)

#define	RTC_RTCIF_PB4R_IF_Pos	14	/* PB4�������жϱ�־ */
#define	RTC_RTCIF_PB4R_IF_Msk	(0x1U << RTC_RTCIF_PB4R_IF_Pos)

#define	RTC_RTCIF_PB4F_IF_Pos	13	/* PB4�½����жϱ�־ */
#define	RTC_RTCIF_PB4F_IF_Msk	(0x1U << RTC_RTCIF_PB4F_IF_Pos)

#define	RTC_RTCIF_ADJ128_IF_Pos	12	/* 128���жϱ�־ */
#define	RTC_RTCIF_ADJ128_IF_Msk	(0x1U << RTC_RTCIF_ADJ128_IF_Pos)

#define	RTC_RTCIF_ALARM_IF_Pos	11	/* �����жϱ�־ */
#define	RTC_RTCIF_ALARM_IF_Msk	(0x1U << RTC_RTCIF_ALARM_IF_Pos)

#define	RTC_RTCIF_1KHZ_IF_Pos	10	/* 1khz�жϱ�־ */
#define	RTC_RTCIF_1KHZ_IF_Msk	(0x1U << RTC_RTCIF_1KHZ_IF_Pos)

#define	RTC_RTCIF_256HZ_IF_Pos	9	/* 256hz�жϱ�־ */
#define	RTC_RTCIF_256HZ_IF_Msk	(0x1U << RTC_RTCIF_256HZ_IF_Pos)

#define	RTC_RTCIF_64HZ_IF_Pos	8	/* 64hz�жϱ�־ */
#define	RTC_RTCIF_64HZ_IF_Msk	(0x1U << RTC_RTCIF_64HZ_IF_Pos)

#define	RTC_RTCIF_16HZ_IF_Pos	7	/* 16hz�жϱ�־ */
#define	RTC_RTCIF_16HZ_IF_Msk	(0x1U << RTC_RTCIF_16HZ_IF_Pos)

#define	RTC_RTCIF_8HZ_IF_Pos	6	/* 8hz�жϱ�־ */
#define	RTC_RTCIF_8HZ_IF_Msk	(0x1U << RTC_RTCIF_8HZ_IF_Pos)

#define	RTC_RTCIF_4HZ_IF_Pos	5	/* 4hz�жϱ�־ */
#define	RTC_RTCIF_4HZ_IF_Msk	(0x1U << RTC_RTCIF_4HZ_IF_Pos)

#define	RTC_RTCIF_2HZ_IF_Pos	4	/* 2hz�жϱ�־ */
#define	RTC_RTCIF_2HZ_IF_Msk	(0x1U << RTC_RTCIF_2HZ_IF_Pos)

#define	RTC_RTCIF_SEC_IF_Pos	3	/* ���жϱ�־ */
#define	RTC_RTCIF_SEC_IF_Msk	(0x1U << RTC_RTCIF_SEC_IF_Pos)

#define	RTC_RTCIF_MIN_IF_Pos	2	/* ���жϱ�־ */
#define	RTC_RTCIF_MIN_IF_Msk	(0x1U << RTC_RTCIF_MIN_IF_Pos)

#define	RTC_RTCIF_HOUR_IF_Pos	1	/* Сʱ�жϱ�־ */
#define	RTC_RTCIF_HOUR_IF_Msk	(0x1U << RTC_RTCIF_HOUR_IF_Pos)

#define	RTC_RTCIF_DATE_IF_Pos	0	/* ���жϱ�־ */
#define	RTC_RTCIF_DATE_IF_Msk	(0x1U << RTC_RTCIF_DATE_IF_Pos)

#define	RTC_BCDSEC_BCDSEC_Pos	0	/* BCDʱ����Ĵ��� */
#define	RTC_BCDSEC_BCDSEC_Msk	(0x7fU << RTC_BCDSEC_BCDSEC_Pos)

#define	RTC_BCDMIN_BCDMIN_Pos	0	/* BCDʱ����ӼĴ��� */
#define	RTC_BCDMIN_BCDMIN_Msk	(0x7fU << RTC_BCDMIN_BCDMIN_Pos)

#define	RTC_BCDHOUR_BCDHOUR_Pos	0	/* BCDʱ��Сʱ�Ĵ��� */
#define	RTC_BCDHOUR_BCDHOUR_Msk	(0x3fU << RTC_BCDHOUR_BCDHOUR_Pos)

#define	RTC_BCDDATE_BCDDATE_Pos	0	/* BCDʱ����Ĵ��� */
#define	RTC_BCDDATE_BCDDATE_Msk	(0x3fU << RTC_BCDDATE_BCDDATE_Pos)

#define	RTC_BCDWEEK_BCDWEEK_Pos	0	/* BCDʱ�����ڼĴ��� */
#define	RTC_BCDWEEK_BCDWEEK_Msk	(0x7U << RTC_BCDWEEK_BCDWEEK_Pos)

#define	RTC_BCDMONTH_BCDMONTH_Pos	0	/* BCDʱ���¼Ĵ��� */
#define	RTC_BCDMONTH_BCDMONTH_Msk	(0x1fU << RTC_BCDMONTH_BCDMONTH_Pos)

#define	RTC_BCDYEAR_BCDYEAR_Pos	0	/* BCDʱ����Ĵ��� */
#define	RTC_BCDYEAR_BCDYEAR_Msk	(0xffU << RTC_BCDYEAR_BCDYEAR_Pos)

#define	RTC_ALARM_ALARMHOUR_Pos	16	/* ���ӵ�Сʱ��ֵ */
#define	RTC_ALARM_ALARMHOUR_Msk	(0x3fU << RTC_ALARM_ALARMHOUR_Pos)

#define	RTC_ALARM_ALARMMIN_Pos	8	/* ���ӵķ���ֵ */
#define	RTC_ALARM_ALARMMIN_Msk	(0x7fU << RTC_ALARM_ALARMMIN_Pos)

#define	RTC_ALARM_ALARMSEC_Pos	0	/* ���ӵ�����ֵ */
#define	RTC_ALARM_ALARMSEC_Msk	(0x7fU << RTC_ALARM_ALARMSEC_Pos)

#define	RTC_FSEL_FSEL_Pos	0	/* Ƶ�����ѡ���ź� */
#define	RTC_FSEL_FSEL_Msk	(0xfU << RTC_FSEL_FSEL_Pos)
#define	RTC_FSEL_FSEL_PLL1HZ	(0x0U << RTC_FSEL_FSEL_Pos)	/* 4��b0000�����PLL��Ƶ�õ��ľ�ȷ1�뷽�� */
#define	RTC_FSEL_FSEL_PLL1HZ80	(0x1U << RTC_FSEL_FSEL_Pos)	/* 4��b0001�����PLL��Ƶ�ĸߵ�ƽ���80ms����ʱ�� */
#define	RTC_FSEL_FSEL_SECOND	(0x2U << RTC_FSEL_FSEL_Pos)	/* 4��b0010��������������λ�źţ��ߵ�ƽ���1s */
#define	RTC_FSEL_FSEL_MINUTE	(0x3U << RTC_FSEL_FSEL_Pos)	/* 4��b0011������ּ�������λ�źţ��ߵ�ƽ���1s */
#define	RTC_FSEL_FSEL_HOUR	(0x4U << RTC_FSEL_FSEL_Pos)	/* 4��b0100�����Сʱ��������λ�źţ��ߵ�ƽ���1s */
#define	RTC_FSEL_FSEL_DAY	(0x5U << RTC_FSEL_FSEL_Pos)	/* 4��b0101��������������λ�źţ��ߵ�ƽ���1s */
#define	RTC_FSEL_FSEL_ALARM	(0x6U << RTC_FSEL_FSEL_Pos)	/* 4��b0110���������ƥ���ź� */
#define	RTC_FSEL_FSEL_128HZ	(0x7U << RTC_FSEL_FSEL_Pos)	/* 4��b0111�����128�뷽���ź� */
#define	RTC_FSEL_FSEL_PLL1HZ80REV	(0x8U << RTC_FSEL_FSEL_Pos)	/* 4��b1000���������PLL��Ƶ�ĸߵ�ƽ���80ms����ʱ�� */
#define	RTC_FSEL_FSEL_SECONDREV	(0x9U << RTC_FSEL_FSEL_Pos)	/* 4��b1001������������������λ�ź� */
#define	RTC_FSEL_FSEL_MINUTEREV	(0xaU << RTC_FSEL_FSEL_Pos)	/* 4��b1010����������ּ�������λ�ź� */
#define	RTC_FSEL_FSEL_HOURREV	(0xbU << RTC_FSEL_FSEL_Pos)	/* 4��b1011���������Сʱ��������λ�ź� */
#define	RTC_FSEL_FSEL_DAYREV	(0xcU << RTC_FSEL_FSEL_Pos)	/* 4��b1100������������������λ�ź� */
#define	RTC_FSEL_FSEL_ALARMREV	(0xdU << RTC_FSEL_FSEL_Pos)	/* 4��b1101�������������ƥ���ź� */
#define	RTC_FSEL_FSEL_PLL1HZREV	(0xeU << RTC_FSEL_FSEL_Pos)	/* 4��b1110���������PLL��Ƶ�ľ�ȷ1s�����ź� */
#define	RTC_FSEL_FSEL_SECONDRUN	(0xfU << RTC_FSEL_FSEL_Pos)	/* 4��b1111�����RTC�ڲ���ʱ�귽�� */

#define	RTC_ADJUST_ADJUST_Pos	0	/* LTBC����������ֵ */
#define	RTC_ADJUST_ADJUST_Msk	(0x7ffU << RTC_ADJUST_ADJUST_Pos)

#define	RTC_ADSIGN_ADSIGN_Pos	0	/* LTBC�������� */
#define	RTC_ADSIGN_ADSIGN_Msk	(0x1U << RTC_ADSIGN_ADSIGN_Pos)
#define	RTC_ADSIGN_ADSIGN_PLUS	(0x0U << RTC_ADSIGN_ADSIGN_Pos)	/* 0����ʾ���Ӽ�����ֵ */
#define	RTC_ADSIGN_ADSIGN_MINUS	(0x1U << RTC_ADSIGN_ADSIGN_Pos)	/* 1����ʾ���ټ�����ֵ */

#define	RTC_PR1SEN_PR1SEN_Pos	0	/* �����Уʹ���ź� */
#define	RTC_PR1SEN_PR1SEN_Msk	(0x1U << RTC_PR1SEN_PR1SEN_Pos)
	/* 0����ʾ��ֹ�����У���� */
	/* 1����ʾʹ�������У���� */

#define	RTC_MSECCNT_MSCNT_Pos	0	/* ���������ֵ����256HzΪ���ڼ���������3.9ms */
#define	RTC_MSECCNT_MSCNT_Msk	(0xffU << RTC_MSECCNT_MSCNT_Pos)

#define	RTC_STAMPEN_STAMP1EN_Pos	1	/* PB5������ʱ�������ʹ��λ���޸�λֵ����������ϵ����г�ʼ�� */
#define	RTC_STAMPEN_STAMP1EN_Msk	(0x1U << RTC_STAMPEN_STAMP1EN_Pos)
	/* 0���ر�ʱ��� */
	/* 1����ʱ��� */

#define	RTC_STAMPEN_STAMP0EN_Pos	0	/* PB4������ʱ�������ʹ��λ���޸�λֵ����������ϵ����г�ʼ�� */
#define	RTC_STAMPEN_STAMP0EN_Msk	(0x1U << RTC_STAMPEN_STAMP0EN_Pos)
	/* 0���ر�ʱ��� */
	/* 1����ʱ��� */

#define	RTC_CLKSTAMP0R_0_Pos	0	/* ��⵽PB4������ʱ���ʱ�������� */
#define	RTC_CLKSTAMP0R_0_Msk	(0xffffffffU << RTC_CLKSTAMP0R_0_Pos)

#define	RTC_CALSTAMP0R_1_Pos	0	/* ��⵽PB4������ʱ��������������� */
#define	RTC_CALSTAMP0R_1_Msk	(0xffffffffU << RTC_CALSTAMP0R_1_Pos)

#define	RTC_CLKSTAMP0F_0_Pos	0	/* ��⵽PB4�½���ʱ���ʱ�������� */
#define	RTC_CLKSTAMP0F_0_Msk	(0xffffffffU << RTC_CLKSTAMP0F_0_Pos)

#define	RTC_CALSTAMP0F_1_Pos	0	/* ��⵽PB4�½���ʱ��������������� */
#define	RTC_CALSTAMP0F_1_Msk	(0xffffffffU << RTC_CALSTAMP0F_1_Pos)

#define	RTC_CLKSTAMP1R_0_Pos	0	/* ��⵽PB5������ʱ���ʱ�������� */
#define	RTC_CLKSTAMP1R_0_Msk	(0xffffffffU << RTC_CLKSTAMP1R_0_Pos)

#define	RTC_CALSTAMP1R_1_Pos	0	/* ��⵽PB5������ʱ��������������� */
#define	RTC_CALSTAMP1R_1_Msk	(0xffffffffU << RTC_CALSTAMP1R_1_Pos)

#define	RTC_CLKSTAMP1F_0_Pos	0	/* ��⵽PB5�½���ʱ���ʱ�������� */
#define	RTC_CLKSTAMP1F_0_Msk	(0xffffffffU << RTC_CLKSTAMP1F_0_Pos)

#define	RTC_CALSTAMP1F_1_Pos	0	/* ��⵽PB5�½���ʱ��������������� */
#define	RTC_CALSTAMP1F_1_Msk	(0xffffffffU << RTC_CALSTAMP1F_1_Pos)

#define	RTC_AUTOCAL_ACALEN_Pos	0	/* �Զ��²�����ʹ�� */
#define	RTC_AUTOCAL_ACALEN_Msk	(0x1U << RTC_AUTOCAL_ACALEN_Pos)

#define	RTC_ADOFFSET_ADOFFSET_Pos	0	/* �Զ��²��������ֵ�Ĵ��� */
#define	RTC_ADOFFSET_ADOFFSET_Msk	(0x7ffU << RTC_ADOFFSET_ADOFFSET_Pos)

#define	RTC_CALSTEP_CALSTEP_Pos	0	/* ѡ���У���� */
#define	RTC_CALSTEP_CALSTEP_Msk	(0x1U << RTC_CALSTEP_CALSTEP_Pos)
#define	RTC_CALSTEP_CALSTEP0P119	(0x1U << RTC_CALSTEP_CALSTEP_Pos)
#define	RTC_CALSTEP_CALSTEP0P238	(0x0U << RTC_CALSTEP_CALSTEP_Pos)

#define	RTC_CALBUSY_AUCALBUSY_Pos	0	/* �Զ��²�״̬*/
#define	RTC_CALBUSY_AUCALBUSY_Msk	(0x1U << RTC_CALBUSY_AUCALBUSY_Pos)

#define	RTC_ADJCNT_ADJCNT_Pos	0	/*�������ڼ�����ֵ*/
#define	RTC_ADJCNT_ADJCNT_Msk	(0xff << RTC_ADJCNT_ADJCNT_Pos)

#define	RTC_ACALADJ_AUADJUST_Pos	0	/*��ǰ�Զ��¶Ȳ�����Уֵ*/
#define	RTC_ACALADJ_AUADJUST_Msk	(0x1fff << RTC_ACALADJ_AUADJUST_Pos)


//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void RTC_Deinit(void);

/* RTCдʹ�ܼĴ��� ��غ��� */
extern void RTC_RTCWE_Write(uint32_t SetValue);

/* BCDʱ����Ĵ��� ��غ��� */
extern void RTC_BCDSEC_Write(uint32_t SetValue);
extern uint32_t RTC_BCDSEC_Read(void);

/* BCDʱ����ӼĴ��� ��غ��� */
extern void RTC_BCDMIN_Write(uint32_t SetValue);
extern uint32_t RTC_BCDMIN_Read(void);

/* BCDʱ��Сʱ�Ĵ��� ��غ��� */
extern void RTC_BCDHOUR_Write(uint32_t SetValue);
extern uint32_t RTC_BCDHOUR_Read(void);

/* BCDʱ����Ĵ��� ��غ��� */
extern void RTC_BCDDATE_Write(uint32_t SetValue);
extern uint32_t RTC_BCDDATE_Read(void);

/* BCDʱ�����ڼĴ��� ��غ��� */
extern void RTC_BCDWEEK_Write(uint32_t SetValue);
extern uint32_t RTC_BCDWEEK_Read(void);

/* BCDʱ���¼Ĵ��� ��غ��� */
extern void RTC_BCDMONTH_Write(uint32_t SetValue);
extern uint32_t RTC_BCDMONTH_Read(void);

/* BCDʱ����Ĵ��� ��غ��� */
extern void RTC_BCDYEAR_Write(uint32_t SetValue);
extern uint32_t RTC_BCDYEAR_Read(void);

/* ���ӵ�Сʱ��ֵ ��غ��� */
extern void RTC_ALARM_ALARMHOUR_Set(uint32_t SetValue);
extern uint32_t RTC_ALARM_ALARMHOUR_Get(void);

/* ���ӵķ���ֵ ��غ��� */
extern void RTC_ALARM_ALARMMIN_Set(uint32_t SetValue);
extern uint32_t RTC_ALARM_ALARMMIN_Get(void);

/* ���ӵ�����ֵ ��غ��� */
extern void RTC_ALARM_ALARMSEC_Set(uint32_t SetValue);
extern uint32_t RTC_ALARM_ALARMSEC_Get(void);

/* Ƶ�����ѡ���ź� ��غ��� */
extern void RTC_FSEL_FSEL_Set(uint32_t SetValue);
extern uint32_t RTC_FSEL_FSEL_Get(void);

/* LTBC����������ֵ ��غ��� */
extern void RTC_ADJUST_Write(uint32_t SetValue);
extern uint32_t RTC_ADJUST_Read(void);

/* LTBC�������� ��غ��� */
extern void RTC_ADSIGN_ADSIGN_Set(uint32_t SetValue);
extern uint32_t RTC_ADSIGN_ADSIGN_Get(void);

/* �����Уʹ���ź� ��غ��� */
extern void RTC_PR1SEN_PR1SEN_Setable(FunState NewState);
extern FunState RTC_PR1SEN_PR1SEN_Getable(void);

/* ���������ֵ����256HzΪ���ڼ���������3.9ms ��غ��� */
extern void RTC_MSECCNT_Write(uint32_t SetValue);
extern uint32_t RTC_MSECCNT_Read(void);

/* PB5������ʱ�������ʹ��λ���޸�λֵ����������ϵ����г�ʼ�� ��غ��� */
extern void RTC_STAMPEN_STAMP1EN_Setable(FunState NewState);
extern FunState RTC_STAMPEN_STAMP1EN_Getable(void);

/* PB4������ʱ�������ʹ��λ���޸�λֵ����������ϵ����г�ʼ�� ��غ��� */
extern void RTC_STAMPEN_STAMP0EN_Setable(FunState NewState);
extern FunState RTC_STAMPEN_STAMP0EN_Getable(void);

/* ��⵽PB4������ʱ���ʱ�������� ��غ��� */
extern uint32_t RTC_CLKSTAMP0R_Read(void);

/* ��⵽PB4������ʱ��������������� ��غ��� */
extern uint32_t RTC_CALSTAMP0R_Read(void);

/* ��⵽PB4�½���ʱ���ʱ�������� ��غ��� */
extern uint32_t RTC_CLKSTAMP0F_Read(void);

/* ��⵽PB4�½���ʱ��������������� ��غ��� */
extern uint32_t RTC_CALSTAMP0F_Read(void);

/* ��⵽PB5������ʱ���ʱ�������� ��غ��� */
extern uint32_t RTC_CLKSTAMP1R_Read(void);

/* ��⵽PB5������ʱ��������������� ��غ��� */
extern uint32_t RTC_CALSTAMP1R_Read(void);

/* ��⵽PB5�½���ʱ���ʱ�������� ��غ��� */
extern uint32_t RTC_CLKSTAMP1F_Read(void);

/* ��⵽PB5�½���ʱ��������������� ��غ��� */
extern uint32_t RTC_CALSTAMP1F_Read(void);
//Announce_End



/* RTC�ж�ʹ�ܼĴ��� ��غ��� */
extern void RTC_RTCIE_SetableEx(FunState NewState, uint32_t ie_def);

extern FunState RTC_RTCIE_GetableEx(uint32_t ie_def);

/* RTC�жϱ�־ ��غ��� */
extern void RTC_RTCIF_ClrEx(uint32_t if_def);

extern FlagStatus RTC_RTCIF_ChkEx(uint32_t if_def);

/* RTC����ʱ�� *///ע��Ҫ�ȴ�д������������ʱ��
extern void RTC_TimeDate_SetEx(RTC_TimeDateTypeDef* para);

/* RTC��ȡʱ�� */
extern void RTC_TimeDate_GetEx(RTC_TimeDateTypeDef* para);

/* ����ʱ������ */
extern void RTC_AlarmTime_SetEx(RTC_AlarmTmieTypeDef* para);

/* ����ʱ���ȡ */
extern void RTC_AlarmTime_GetEx(RTC_AlarmTmieTypeDef* para);
	
/* ʱ������ݶ�ȡ */
extern void RTC_CLKSTAMPxx_GetEx(uint8_t StampType_def, RTC_StampTmieTypeDef* para);

/* �Զ��²�ʹ�ܺ���*/
extern void RTC_AUTOCAL_Setable(FunState NewState);

/*��ȡRTC�Զ��²�ʹ��״̬*/
extern FunState RTC_AUTOCAL_Getable(void);

/*дRTC �Զ��²��������ֵ����*/
extern void RTC_ADOFFSET_Write(uint32_t SetValue);

/*��RTC�Զ��²��������ֵ����*/
extern uint32_t RTC_ADOFFSET_Read(void);

/*RTC��С��У�������ú���*/
extern void RTC_CALSTEP_SET(uint32_t SetValue);

/*��ȡRTC��С��У��������*/
extern FunState RTC_CALSTEP_Get(void);

/*��ȡRTC�Զ��²�״̬��*/
extern FunState RTC_CALBUSY_Get(void);

/*��RTC�������ڼ�����ֵ��*/
extern uint32_t RTC_ADJCNT_Read(void);

/*��RTC�Զ��¶Ȳ�������ֵ����*/
extern uint32_t RTC_ACALADJ_Read(void);

/*дRTC���ݼĴ�������*/
extern void RTC_BKREG_Write(uint32_t* prog_data);

/* RTC ʱ�Ӿ��Ȳ������� */
/*
	���� float�Ͳ����ppm
*/
extern void RTC_Trim_Proc(float err_ppm);
	
#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_RTC_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



