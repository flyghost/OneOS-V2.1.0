/**
  ******************************************************************************
  * @file    fm33g0xx_lcd.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the LCD firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_LCD_H
#define __FM33G0XX_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup LCD
  * @{
  */
	 
	 
/* Exported constants --------------------------------------------------------*/ 
/* Exported types ------------------------------------------------------------*/
typedef struct
{       
	uint32_t			LMUX;		/*!<COM����ѡ��*/	
	uint32_t			ENMODE;		/*!<����ģʽѡ��*/
	uint32_t			WFT;		/*!<��������ѡ��*/
	uint32_t			DF;			/*!<��ʾƵ�ʿ��ƼĴ���*/
	uint32_t			BIASMD;		/*!<ƫ�����Ϳ���*/
	uint32_t			SCFSEL;		/*!<SCƵ��ѡ��*/  //����Ƭ�����ģʽ����Ч
	uint32_t			SC_CTRL;	/*!<Ƭ���������ģʽ�£�������ʽ����*/
	uint32_t			IC_CTRL;	/*!<Ƭ�ڵ���ģʽ�£�����������С*/
	uint32_t			LCDBIAS;	/*!<��ʾ�Ҷ�����*/
	FunState			ANTIPOLAR;	/*!<������ʹ�ܿ���*/
	
	FunState			TEST;		/*!<����ģʽʹ�ܿ��ƣ�����DISPMD=1���������Ч*/
	FunState			DISPMD;		/*!<����ģʽʹ�ܿ���*/
	
	uint32_t			LCCTRL;		/*!<����Ϊ0*/
	FunState			TESTEN;		/*!<���ֹر�*/
	
	FunState			FLICK;		/*!<��ʾ��˸ʹĿ����*/
	uint32_t			TON;		/*!<��˸��ʾʱ�ĵ���ʱ�䣽TON��0.25��*/
	uint32_t			TOFF;		/*!<��˸��ʾʱ��Ϩ��ʱ�䣽TOFF��0.25��*/
	FunState			DONIE;		/*!<��ʾ�����ж�ʹ�ܿ���*/
	FunState			DOFFIE;		/*!<��ʾϨ���ж�ʹ�ܿ���*/
	
	FunState			LCDEN;		/*!<LCD��ʾʹ�ܿ���*/
}LCD_InitTypeDef;
 
/* Exported macro ------------------------------------------------------------*/



#define	LCD_DISPCTRL_ANTIPOLAR_Pos	7	/* ������ */
#define	LCD_DISPCTRL_ANTIPOLAR_Msk	(0x1U << LCD_DISPCTRL_ANTIPOLAR_Pos)
	/* 0��COM��SEG��LCD�ر�����¸��� */
	/* 1��COM��SEG��LCD�ر�����½ӵ� */

#define	LCD_DISPCTRL_LCDEN_Pos	6	/* LCD��ʾʹ�� */
#define	LCD_DISPCTRL_LCDEN_Msk	(0x1U << LCD_DISPCTRL_LCDEN_Pos)
	/* 0���ر�LCD��ʾ */
	/* 1������LCD��ʾ */

#define	LCD_DISPCTRL_FLICK_Pos	2	/* ��ʾ��˸ʹ�� */
#define	LCD_DISPCTRL_FLICK_Msk	(0x1U << LCD_DISPCTRL_FLICK_Pos)
	/* 0���ر���˸ */
	/* 1����ʾ��˸����˸Ƶ����TON��TOFF�Ĵ������� */

#define	LCD_DISPCTRL_TEST_Pos	1	/* ����ģʽʹ�ܣ�����DISPMD=1���������Ч */
#define	LCD_DISPCTRL_TEST_Msk	(0x1U << LCD_DISPCTRL_TEST_Pos)
	/* 0����ʾȫ�� */
	/* 1����ʾȫ�� */

#define	LCD_DISPCTRL_DISPMD_Pos	0	/* ����ģʽѡ�� */
#define	LCD_DISPCTRL_DISPMD_Msk	(0x1U << LCD_DISPCTRL_DISPMD_Pos)
	/* 0������ģʽ��TESTλ��Ч */
	/* 1����ʾ����ģʽ��TESTλ��Ч */

#define	LCD_LCDTEST_LCCTRL_Pos	7	/* LCD���Կ���λ�����ڲ���ģʽ����Ч */
#define	LCD_LCDTEST_LCCTRL_Msk	(0x1U << LCD_LCDTEST_LCCTRL_Pos)
#define	LCD_LCDTEST_LCCTRL_0	(0x0U << LCD_LCDTEST_LCCTRL_Pos)
#define	LCD_LCDTEST_LCCTRL_1	(0x1U << LCD_LCDTEST_LCCTRL_Pos)

#define	LCD_LCDTEST_TESTEN_Pos	0	/* ����ģʽʹ�� */
#define	LCD_LCDTEST_TESTEN_Msk	(0x1U << LCD_LCDTEST_TESTEN_Pos)
	/* ��������ģʽ */
	/* LCD����ģʽʹ�� */

#define	LCD_DF_DF_Pos	0	/* ��ʾƵ�ʿ��ƼĴ��� */
#define	LCD_DF_DF_Msk	(0xffU << LCD_DF_DF_Pos)

#define	LCD_TON_TON_Pos	0	/* ��˸��ʾ����ʱ��Ĵ��� */
#define	LCD_TON_TON_Msk	(0xffU << LCD_TON_TON_Pos)

#define	LCD_TOFF_TOFF_Pos	0	/* ��˸��ʾϨ��ʱ��Ĵ��� */
#define	LCD_TOFF_TOFF_Msk	(0xffU << LCD_TOFF_TOFF_Pos)

#define	LCD_DISPIE_DONIE_Pos	1	/* ��ʾ�����ж�ʹ�� */
#define	LCD_DISPIE_DONIE_Msk	(0x1U << LCD_DISPIE_DONIE_Pos)
	/* 0 = ��ʾ�����жϽ�ֹ */
	/* 1 = ��ʾ�����ж�ʹ�� */

#define	LCD_DISPIE_DOFFIE_Pos	0	/* ��ʾϨ���ж�ʹ�� */
#define	LCD_DISPIE_DOFFIE_Msk	(0x1U << LCD_DISPIE_DOFFIE_Pos)
	/* 0 = ��ʾϨ���жϽ�ֹ */
	/* 1 = ��ʾϨ���ж�ʹ�� */

#define	LCD_DISPIF_DONIF_Pos	1	/* ��ʾ�����жϱ�־ */
#define	LCD_DISPIF_DONIF_Msk	(0x1U << LCD_DISPIF_DONIF_Pos)

#define	LCD_DISPIF_DOFFIF_Pos	0	/* ��ʾϨ���жϱ�־ */
#define	LCD_DISPIF_DOFFIF_Msk	(0x1U << LCD_DISPIF_DOFFIF_Pos)

#define	LCD_LCDSET_BIASMD_Pos	4	/* ƫ�����Ϳ��� */
#define	LCD_LCDSET_BIASMD_Msk	(0x1U << LCD_LCDSET_BIASMD_Pos)
#define	LCD_LCDSET_BIASMD_4BIAS	(0x0U << LCD_LCDSET_BIASMD_Pos)	/* 0��1/4 Bias */
#define	LCD_LCDSET_BIASMD_3BIAS	(0x1U << LCD_LCDSET_BIASMD_Pos)	/* 1��1/3 Bias */

#define	LCD_LCDSET_WFT_Pos	2	/* ��������ѡ�� */
#define	LCD_LCDSET_WFT_Msk	(0x1U << LCD_LCDSET_WFT_Pos)
#define	LCD_LCDSET_WFT_ATYPE	(0x0U << LCD_LCDSET_WFT_Pos)	/* 0��A�ನ�� */
#define	LCD_LCDSET_WFT_BTYPE	(0x1U << LCD_LCDSET_WFT_Pos)	/* 1��B�ನ�� */

#define	LCD_LCDSET_LMUX_Pos	0	/* COM����ѡ�� */
#define	LCD_LCDSET_LMUX_Msk	(0x3U << LCD_LCDSET_LMUX_Pos)
#define	LCD_LCDSET_LMUX_4COM	(0x0U << LCD_LCDSET_LMUX_Pos)	/* 00��4COM */
#define	LCD_LCDSET_LMUX_6COM	(0x1U << LCD_LCDSET_LMUX_Pos)	/* 01��6COM */
#define	LCD_LCDSET_LMUX_8COM	(0x2U << LCD_LCDSET_LMUX_Pos)	/* 10/11��8COM */

#define	LCD_LCDDRV_SCFSEL_Pos	5	/* SCƵ��ѡ�� */
#define	LCD_LCDDRV_SCFSEL_Msk	(0x7U << LCD_LCDDRV_SCFSEL_Pos)
#define	LCD_LCDDRV_SCFSEL_X1	(0x0U << LCD_LCDDRV_SCFSEL_Pos)	/* 000 = Ƶ��Ϊ֡Ƶ*COM��(ע�⣺��ѡ��110��111��λʱ�����Ƶ�ʵ���֡Ƶ*2*COM������������000��λ��ͬ) */
#define	LCD_LCDDRV_SCFSEL_X8	(0x1U << LCD_LCDDRV_SCFSEL_Pos)	/* 001 = Ƶ��Ϊdisp_clk/8����disp_clk=32KHz����Ϊ4KHz�� */
#define	LCD_LCDDRV_SCFSEL_X16	(0x2U << LCD_LCDDRV_SCFSEL_Pos)	/* 010 = Ƶ��Ϊdisp_clk/16����disp_clk=32KHz����Ϊ2KHz�� */
#define	LCD_LCDDRV_SCFSEL_X32	(0x3U << LCD_LCDDRV_SCFSEL_Pos)	/* 011 = Ƶ��Ϊdisp_clk/32����disp_clk=32KHz����Ϊ1KHz�� */
#define	LCD_LCDDRV_SCFSEL_X64	(0x4U << LCD_LCDDRV_SCFSEL_Pos)	/* 100 = Ƶ��Ϊdisp_clk/64����disp_clk=32KHz����Ϊ500Hz�� */
#define	LCD_LCDDRV_SCFSEL_X128	(0x5U << LCD_LCDDRV_SCFSEL_Pos)	/* 101 = Ƶ��Ϊdisp_clk/128����disp_clk=32KHz����Ϊ250Hz�� */
#define	LCD_LCDDRV_SCFSEL_X256	(0x6U << LCD_LCDDRV_SCFSEL_Pos)	/* 110 = Ƶ��Ϊdisp_clk/256����disp_clk=32KHz����Ϊ125Hz�� */
#define	LCD_LCDDRV_SCFSEL_X512	(0x7U << LCD_LCDDRV_SCFSEL_Pos)	/* 111= Ƶ��Ϊdisp_clk/512����disp_clk=32KHz����Ϊ62.5Hz�� */

#define	LCD_LCDDRV_SC_CTRL_Pos	3	/* Ƭ���������ģʽ�£�������ʽ���� */
#define	LCD_LCDDRV_SC_CTRL_Msk	(0x3U << LCD_LCDDRV_SC_CTRL_Pos)
#define	LCD_LCDDRV_SC_CTRL_ONE	(0x0U << LCD_LCDDRV_SC_CTRL_Pos)	/* 00 = �������� */
#define	LCD_LCDDRV_SC_CTRL_TWO	(0x1U << LCD_LCDDRV_SC_CTRL_Pos)	/* 01 = ��������2�� */
#define	LCD_LCDDRV_SC_CTRL_FOUR	(0x2U << LCD_LCDDRV_SC_CTRL_Pos)	/* 10 = ��������4�Σ���SCƵ�ʴ��ڵ���4KHzʱ����ѡ��ҲΪ������� */
#define	LCD_LCDDRV_SC_CTRL_CONTINUE	(0x3U << LCD_LCDDRV_SC_CTRL_Pos)	/* 11 = ������� */

#define	LCD_LCDDRV_IC_CTRL_Pos	1	/* ����������С */
#define	LCD_LCDDRV_IC_CTRL_Msk	(0x3U << LCD_LCDDRV_IC_CTRL_Pos)
#define	LCD_LCDDRV_IC_CTRL_L3	(0x0U << LCD_LCDDRV_IC_CTRL_Pos)	/* 00 = ������� */
#define	LCD_LCDDRV_IC_CTRL_L2	(0x1U << LCD_LCDDRV_IC_CTRL_Pos)	/* 01 = �����δ� */
#define	LCD_LCDDRV_IC_CTRL_L1	(0x2U << LCD_LCDDRV_IC_CTRL_Pos)	/* 10 = ������С */
#define	LCD_LCDDRV_IC_CTRL_L0	(0x3U << LCD_LCDDRV_IC_CTRL_Pos)	/* 11 = ������С */

#define	LCD_LCDDRV_ENMODE_Pos	0	/* ����ģʽѡ�� */
#define	LCD_LCDDRV_ENMODE_Msk	(0x1U << LCD_LCDDRV_ENMODE_Pos)
#define	LCD_LCDDRV_ENMODE_EXTERNALCAP	(0x0U << LCD_LCDDRV_ENMODE_Pos)	/* 0 = Ƭ��������� */
#define	LCD_LCDDRV_ENMODE_INNERRESISTER	(0x1U << LCD_LCDDRV_ENMODE_Pos)	/* 1 = Ƭ�ڵ��������� */

#define	LCD_LCDBIAS_LCDBIAS_Pos	0	/* ��ʾ�Ҷ����� */
#define	LCD_LCDBIAS_LCDBIAS_Msk	(0xfU << LCD_LCDBIAS_LCDBIAS_Pos)

#define	LCD_COM_EN_COMEN_Pos	0	/* LCD COM0~3���ʹ�ܿ��� */
#define	LCD_COM_EN_COMEN_Msk	(0xfU << LCD_COM_EN_COMEN_Pos)

#define	LCD_SEG_EN0_SEGENx_Pos	0	/* LCD SEG0~31���ʹ�ܿ��� */
#define	LCD_SEG_EN0_SEGENx_Msk	(0xffffffffU << LCD_SEG_EN0_SEGENx_Pos)

#define	LCD_SEG_EN1_SEGENx_Pos	0	/* LCD SEG32~43,COM4~7���ʹ�ܿ��� */
#define	LCD_SEG_EN1_SEGENx_Msk	(0xfffU << LCD_SEG_EN1_SEGENx_Pos)

#define	LCD_COM_EN_COMEN3_Pos	3	/* COM3�������@4\6\8COMģʽ */
#define	LCD_COM_EN_COMEN3_Msk	(0x1U << LCD_COM_EN_COMEN3_Pos)

#define	LCD_COM_EN_COMEN2_Pos	2	/* COM2�������@4\6\8COMģʽ */
#define	LCD_COM_EN_COMEN2_Msk	(0x1U << LCD_COM_EN_COMEN2_Pos)

#define	LCD_COM_EN_COMEN1_Pos	1	/* COM1�������@4\6\8COMģʽ */
#define	LCD_COM_EN_COMEN1_Msk	(0x1U << LCD_COM_EN_COMEN1_Pos)

#define	LCD_COM_EN_COMEN0_Pos	0	/* COM0�������@4\6\8COMģʽ */
#define	LCD_COM_EN_COMEN0_Msk	(0x1U << LCD_COM_EN_COMEN0_Pos)

#define	LCD_SEG_EN0_SEGEN31_Pos	31	/* SEG31������� */
#define	LCD_SEG_EN0_SEGEN31_Msk	(0x1U << LCD_SEG_EN0_SEGEN31_Pos)

#define	LCD_SEG_EN0_SEGEN30_Pos	30	/* SEG30������� */
#define	LCD_SEG_EN0_SEGEN30_Msk	(0x1U << LCD_SEG_EN0_SEGEN30_Pos)

#define	LCD_SEG_EN0_SEGEN29_Pos	29	/* SEG29������� */
#define	LCD_SEG_EN0_SEGEN29_Msk	(0x1U << LCD_SEG_EN0_SEGEN29_Pos)

#define	LCD_SEG_EN0_SEGEN28_Pos	28	/* SEG28������� */
#define	LCD_SEG_EN0_SEGEN28_Msk	(0x1U << LCD_SEG_EN0_SEGEN28_Pos)

#define	LCD_SEG_EN0_SEGEN27_Pos	27	/* SEG27������� */
#define	LCD_SEG_EN0_SEGEN27_Msk	(0x1U << LCD_SEG_EN0_SEGEN27_Pos)

#define	LCD_SEG_EN0_SEGEN26_Pos	26	/* SEG26������� */
#define	LCD_SEG_EN0_SEGEN26_Msk	(0x1U << LCD_SEG_EN0_SEGEN26_Pos)

#define	LCD_SEG_EN0_SEGEN25_Pos	25	/* SEG25������� */
#define	LCD_SEG_EN0_SEGEN25_Msk	(0x1U << LCD_SEG_EN0_SEGEN25_Pos)

#define	LCD_SEG_EN0_SEGEN24_Pos	24	/* SEG24������� */
#define	LCD_SEG_EN0_SEGEN24_Msk	(0x1U << LCD_SEG_EN0_SEGEN24_Pos)

#define	LCD_SEG_EN0_SEGEN23_Pos	23	/* SEG23������� */
#define	LCD_SEG_EN0_SEGEN23_Msk	(0x1U << LCD_SEG_EN0_SEGEN23_Pos)

#define	LCD_SEG_EN0_SEGEN22_Pos	22	/* SEG22������� */
#define	LCD_SEG_EN0_SEGEN22_Msk	(0x1U << LCD_SEG_EN0_SEGEN22_Pos)

#define	LCD_SEG_EN0_SEGEN21_Pos	21	/* SEG21������� */
#define	LCD_SEG_EN0_SEGEN21_Msk	(0x1U << LCD_SEG_EN0_SEGEN21_Pos)

#define	LCD_SEG_EN0_SEGEN20_Pos	20	/* SEG20������� */
#define	LCD_SEG_EN0_SEGEN20_Msk	(0x1U << LCD_SEG_EN0_SEGEN20_Pos)

#define	LCD_SEG_EN0_SEGEN19_Pos	19	/* SEG19������� */
#define	LCD_SEG_EN0_SEGEN19_Msk	(0x1U << LCD_SEG_EN0_SEGEN19_Pos)

#define	LCD_SEG_EN0_SEGEN18_Pos	18	/* SEG18������� */
#define	LCD_SEG_EN0_SEGEN18_Msk	(0x1U << LCD_SEG_EN0_SEGEN18_Pos)

#define	LCD_SEG_EN0_SEGEN17_Pos	17	/* SEG17������� */
#define	LCD_SEG_EN0_SEGEN17_Msk	(0x1U << LCD_SEG_EN0_SEGEN17_Pos)

#define	LCD_SEG_EN0_SEGEN16_Pos	16	/* SEG16������� */
#define	LCD_SEG_EN0_SEGEN16_Msk	(0x1U << LCD_SEG_EN0_SEGEN16_Pos)

#define	LCD_SEG_EN0_SEGEN15_Pos	15	/* SEG15������� */
#define	LCD_SEG_EN0_SEGEN15_Msk	(0x1U << LCD_SEG_EN0_SEGEN15_Pos)

#define	LCD_SEG_EN0_SEGEN14_Pos	14	/* SEG14������� */
#define	LCD_SEG_EN0_SEGEN14_Msk	(0x1U << LCD_SEG_EN0_SEGEN14_Pos)

#define	LCD_SEG_EN0_SEGEN13_Pos	13	/* SEG13������� */
#define	LCD_SEG_EN0_SEGEN13_Msk	(0x1U << LCD_SEG_EN0_SEGEN13_Pos)

#define	LCD_SEG_EN0_SEGEN12_Pos	12	/* SEG12������� */
#define	LCD_SEG_EN0_SEGEN12_Msk	(0x1U << LCD_SEG_EN0_SEGEN12_Pos)

#define	LCD_SEG_EN0_SEGEN11_Pos	11	/* SEG11������� */
#define	LCD_SEG_EN0_SEGEN11_Msk	(0x1U << LCD_SEG_EN0_SEGEN11_Pos)

#define	LCD_SEG_EN0_SEGEN10_Pos	10	/* SEG10������� */
#define	LCD_SEG_EN0_SEGEN10_Msk	(0x1U << LCD_SEG_EN0_SEGEN10_Pos)

#define	LCD_SEG_EN0_SEGEN9_Pos	9	/* SEG9������� */
#define	LCD_SEG_EN0_SEGEN9_Msk	(0x1U << LCD_SEG_EN0_SEGEN9_Pos)

#define	LCD_SEG_EN0_SEGEN8_Pos	8	/* SEG8������� */
#define	LCD_SEG_EN0_SEGEN8_Msk	(0x1U << LCD_SEG_EN0_SEGEN8_Pos)

#define	LCD_SEG_EN0_SEGEN7_Pos	7	/* SEG7������� */
#define	LCD_SEG_EN0_SEGEN7_Msk	(0x1U << LCD_SEG_EN0_SEGEN7_Pos)

#define	LCD_SEG_EN0_SEGEN6_Pos	6	/* SEG6������� */
#define	LCD_SEG_EN0_SEGEN6_Msk	(0x1U << LCD_SEG_EN0_SEGEN6_Pos)

#define	LCD_SEG_EN0_SEGEN5_Pos	5	/* SEG5������� */
#define	LCD_SEG_EN0_SEGEN5_Msk	(0x1U << LCD_SEG_EN0_SEGEN5_Pos)

#define	LCD_SEG_EN0_SEGEN4_Pos	4	/* SEG4������� */
#define	LCD_SEG_EN0_SEGEN4_Msk	(0x1U << LCD_SEG_EN0_SEGEN4_Pos)

#define	LCD_SEG_EN0_SEGEN3_Pos	3	/* SEG3������� */
#define	LCD_SEG_EN0_SEGEN3_Msk	(0x1U << LCD_SEG_EN0_SEGEN3_Pos)

#define	LCD_SEG_EN0_SEGEN2_Pos	2	/* SEG2������� */
#define	LCD_SEG_EN0_SEGEN2_Msk	(0x1U << LCD_SEG_EN0_SEGEN2_Pos)

#define	LCD_SEG_EN0_SEGEN1_Pos	1	/* SEG1������� */
#define	LCD_SEG_EN0_SEGEN1_Msk	(0x1U << LCD_SEG_EN0_SEGEN1_Pos)

#define	LCD_SEG_EN0_SEGEN0_Pos	0	/* SEG0������� */
#define	LCD_SEG_EN0_SEGEN0_Msk	(0x1U << LCD_SEG_EN0_SEGEN0_Pos)

#define	LCD_SEG_EN1_SEGEN43_Pos	11	/* SEG43������� */
#define	LCD_SEG_EN1_SEGEN43_Msk	(0x1U << LCD_SEG_EN1_SEGEN43_Pos)

#define	LCD_SEG_EN1_SEGEN42_Pos	10	/* SEG42������� */
#define	LCD_SEG_EN1_SEGEN42_Msk	(0x1U << LCD_SEG_EN1_SEGEN42_Pos)

#define	LCD_SEG_EN1_SEGEN41_Pos	9	/* SEG41������� */
#define	LCD_SEG_EN1_SEGEN41_Msk	(0x1U << LCD_SEG_EN1_SEGEN41_Pos)

#define	LCD_SEG_EN1_SEGEN40_Pos	8	/* SEG40������� */
#define	LCD_SEG_EN1_SEGEN40_Msk	(0x1U << LCD_SEG_EN1_SEGEN40_Pos)

#define	LCD_SEG_EN1_SEGEN39_Pos	7	/* SEG39������� */
#define	LCD_SEG_EN1_SEGEN39_Msk	(0x1U << LCD_SEG_EN1_SEGEN39_Pos)

#define	LCD_SEG_EN1_SEGEN38_Pos	6	/* SEG38������� */
#define	LCD_SEG_EN1_SEGEN38_Msk	(0x1U << LCD_SEG_EN1_SEGEN38_Pos)

#define	LCD_SEG_EN1_SEGEN37_Pos	5	/* SEG37������� */
#define	LCD_SEG_EN1_SEGEN37_Msk	(0x1U << LCD_SEG_EN1_SEGEN37_Pos)

#define	LCD_SEG_EN1_SEGEN36_Pos	4	/* SEG36������� */
#define	LCD_SEG_EN1_SEGEN36_Msk	(0x1U << LCD_SEG_EN1_SEGEN36_Pos)

#define	LCD_SEG_EN1_SEGEN35_Pos	3	/* SEG35������� */
#define	LCD_SEG_EN1_SEGEN35_Msk	(0x1U << LCD_SEG_EN1_SEGEN35_Pos)

#define	LCD_SEG_EN1_SEGEN34_Pos	2	/* SEG34������� */
#define	LCD_SEG_EN1_SEGEN34_Msk	(0x1U << LCD_SEG_EN1_SEGEN34_Pos)

#define	LCD_SEG_EN1_SEGEN33_Pos	1	/* SEG33������� */
#define	LCD_SEG_EN1_SEGEN33_Msk	(0x1U << LCD_SEG_EN1_SEGEN33_Pos)

#define	LCD_SEG_EN1_SEGEN32_Pos	0	/* SEG32������� */
#define	LCD_SEG_EN1_SEGEN32_Msk	(0x1U << LCD_SEG_EN1_SEGEN32_Pos)

#define	LCD_SEG_EN1_COM5ENo6COM_Pos	11	/* COM5@6COM������ƣ�����SEGEN43 */
#define	LCD_SEG_EN1_COM5ENo6COM_Msk	(0x1U << LCD_SEG_EN1_COM5ENo6COM_Pos)

#define	LCD_SEG_EN1_COM4ENo6COM_Pos	10	/* COM4@6COM������ƣ�����SEGEN42 */
#define	LCD_SEG_EN1_COM4ENo6COM_Msk	(0x1U << LCD_SEG_EN1_COM4ENo6COM_Pos)

#define	LCD_SEG_EN1_COM7ENo8COM_Pos	11	/* COM7@8COM������ƣ�����SEGEN43 */
#define	LCD_SEG_EN1_COM7ENo8COM_Msk	(0x1U << LCD_SEG_EN1_COM7ENo8COM_Pos)

#define	LCD_SEG_EN1_COM6ENo8COM_Pos	10	/* COM6@8COM������ƣ�����SEGEN42 */
#define	LCD_SEG_EN1_COM6ENo8COM_Msk	(0x1U << LCD_SEG_EN1_COM6ENo8COM_Pos)

#define	LCD_SEG_EN1_COM5ENo8COM_Pos	9	/* COM5@8COM������ƣ�����SEGEN41 */
#define	LCD_SEG_EN1_COM5ENo8COM_Msk	(0x1U << LCD_SEG_EN1_COM5ENo8COM_Pos)

#define	LCD_SEG_EN1_COM4ENo8COM_Pos	8	/* COM4@8COM������ƣ�����SEGEN40 */
#define	LCD_SEG_EN1_COM4ENo8COM_Msk	(0x1U << LCD_SEG_EN1_COM4ENo8COM_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void LCD_Deinit(void);

/* ������ ��غ��� */
extern void LCD_DISPCTRL_ANTIPOLAR_Setable(FunState NewState);
extern FunState LCD_DISPCTRL_ANTIPOLAR_Getable(void);

/* LCD��ʾʹ�� ��غ��� */
extern void LCD_DISPCTRL_LCDEN_Setable(FunState NewState);
extern FunState LCD_DISPCTRL_LCDEN_Getable(void);

/* ��ʾ��˸ʹ�� ��غ��� */
extern void LCD_DISPCTRL_FLICK_Setable(FunState NewState);
extern FunState LCD_DISPCTRL_FLICK_Getable(void);

/* ����ģʽʹ�ܣ�����DISPMD=1���������Ч ��غ��� */
extern void LCD_DISPCTRL_TEST_Setable(FunState NewState);
extern FunState LCD_DISPCTRL_TEST_Getable(void);

/* ����ģʽѡ�� ��غ��� */
extern void LCD_DISPCTRL_DISPMD_Setable(FunState NewState);
extern FunState LCD_DISPCTRL_DISPMD_Getable(void);

/* LCD���Կ���λ�����ڲ���ģʽ����Ч ��غ��� */
extern void LCD_LCDTEST_LCCTRL_Set(uint32_t SetValue);
extern uint32_t LCD_LCDTEST_LCCTRL_Get(void);

/* ����ģʽʹ�� ��غ��� */
extern void LCD_LCDTEST_TESTEN_Setable(FunState NewState);
extern FunState LCD_LCDTEST_TESTEN_Getable(void);

/* ��ʾƵ�ʿ��ƼĴ��� ��غ��� */
extern void LCD_DF_Write(uint32_t SetValue);
extern uint32_t LCD_DF_Read(void);

/* ��˸��ʾ����ʱ��Ĵ��� ��غ��� */
extern void LCD_TON_Write(uint32_t SetValue);
extern uint32_t LCD_TON_Read(void);

/* ��˸��ʾϨ��ʱ��Ĵ��� ��غ��� */
extern void LCD_TOFF_Write(uint32_t SetValue);
extern uint32_t LCD_TOFF_Read(void);

/* ��ʾ�����ж�ʹ�� ��غ��� */
extern void LCD_DISPIE_DONIE_Setable(FunState NewState);
extern FunState LCD_DISPIE_DONIE_Getable(void);

/* ��ʾϨ���ж�ʹ�� ��غ��� */
extern void LCD_DISPIE_DOFFIE_Setable(FunState NewState);
extern FunState LCD_DISPIE_DOFFIE_Getable(void);

/* ��ʾ�����жϱ�־ ��غ��� */
extern void LCD_DISPIF_DONIF_Clr(void);
extern FlagStatus LCD_DISPIF_DONIF_Chk(void);

/* ��ʾϨ���жϱ�־ ��غ��� */
extern void LCD_DISPIF_DOFFIF_Clr(void);
extern FlagStatus LCD_DISPIF_DOFFIF_Chk(void);

/* ƫ�����Ϳ��� ��غ��� */
extern void LCD_LCDSET_BIASMD_Set(uint32_t SetValue);
extern uint32_t LCD_LCDSET_BIASMD_Get(void);

/* ��������ѡ�� ��غ��� */
extern void LCD_LCDSET_WFT_Set(uint32_t SetValue);
extern uint32_t LCD_LCDSET_WFT_Get(void);

/* COM����ѡ�� ��غ��� */
extern void LCD_LCDSET_LMUX_Set(uint32_t SetValue);
extern uint32_t LCD_LCDSET_LMUX_Get(void);

/* SCƵ��ѡ�� ��غ��� */
extern void LCD_LCDDRV_SCFSEL_Set(uint32_t SetValue);
extern uint32_t LCD_LCDDRV_SCFSEL_Get(void);

/* Ƭ���������ģʽ�£�������ʽ���� ��غ��� */
extern void LCD_LCDDRV_SC_CTRL_Set(uint32_t SetValue);
extern uint32_t LCD_LCDDRV_SC_CTRL_Get(void);

/* ����������С ��غ��� */
extern void LCD_LCDDRV_IC_CTRL_Set(uint32_t SetValue);
extern uint32_t LCD_LCDDRV_IC_CTRL_Get(void);

/* ����ģʽѡ�� ��غ��� */
extern void LCD_LCDDRV_ENMODE_Set(uint32_t SetValue);
extern uint32_t LCD_LCDDRV_ENMODE_Get(void);

/* ��ʾ�Ҷ����� ��غ��� */
extern void LCD_LCDBIAS_Write(uint32_t SetValue);
extern uint32_t LCD_LCDBIAS_Read(void);

/* LCD COM0~3���ʹ�ܿ��� ��غ��� */
extern void LCD_COM_EN_Write(uint32_t SetValue);
extern uint32_t LCD_COM_EN_Read(void);

/* LCD SEG0~31���ʹ�ܿ��� ��غ��� */
extern void LCD_SEG_EN0_Write(uint32_t SetValue);
extern uint32_t LCD_SEG_EN0_Read(void);

/* LCD SEG32~43,COM4~7���ʹ�ܿ��� ��غ��� */
extern void LCD_SEG_EN1_Write(uint32_t SetValue);
extern uint32_t LCD_SEG_EN1_Read(void);
//Announce_End



/*DISPDATAx��ʾ���ݼĴ���ˢ��*/
extern void LCD_DISPDATAx_Refresh(uint32_t* DispBuf);

/*LCD��ʾ��������*/
extern void LCD_Init(LCD_InitTypeDef* para);


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_LCD_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/




