/**
  ******************************************************************************
  * @file    fm33g0xx_i2c.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the I2C firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_I2C_H
#define __FM33G0XX_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"

/* Defines--------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define I2C_SEND_STARTBIT()		I2C_I2CCTRL_SEN_Setable(ENABLE)
#define I2C_SEND_RESTARTBIT()		I2C_I2CCTRL_RSEN_Setable(ENABLE)
#define I2C_SEND_STOPBIT()		I2C_I2CCTRL_PEN_Setable(ENABLE)
#define I2C_SEND_ACKBIT()			I2C_I2CCTRL_ACKEN_Setable(ENABLE)
	 
#define I2C_SEND_ACK_0()			I2C_I2CSTA_ACKDT_Set(I2C_I2CSTA_ACKDT_L)
#define I2C_SEND_ACK_1()			I2C_I2CSTA_ACKDT_Set(I2C_I2CSTA_ACKDT_H)
	 

	 

#define	I2C_I2CCTRL_SCLHL_Pos	14	/* ��I2Cģ���ֹʱ��SCL�̶�Ϊ�ߵ͵�ƽ����λ */
#define	I2C_I2CCTRL_SCLHL_Msk	(0x1U << I2C_I2CCTRL_SCLHL_Pos)
	/* 0 = ��SCL�ź�ת��Ϊ�͵�ƽ */
	/* 1 = ��SCL�ź�ת��Ϊ�ߵ�ƽ����Ǳ�Ҫ�����鳣̬��Ϊ1 */

#define	I2C_I2CCTRL_SDAHL_Pos	13	/* ��I2Cģ���ֹʱ��SDA�̶�Ϊ�ߵ͵�ƽ����λ */
#define	I2C_I2CCTRL_SDAHL_Msk	(0x1U << I2C_I2CCTRL_SDAHL_Pos)
	/* 0 = ��SDA�ź�ת��Ϊ�͵�ƽ */
	/* 1 = ��SDA�ź�ת��Ϊ�ߵ�ƽ����Ǳ�Ҫ�����鳣̬��Ϊ1 */

#define	I2C_I2CCTRL_ACKEN_Pos	12	/* ���ؽ���ģʽ�£��������ӻ���Ӧʹ��λ */
#define	I2C_I2CCTRL_ACKEN_Msk	(0x1U << I2C_I2CCTRL_ACKEN_Pos)
	/* 0 = ��������Ӧ�ӻ� */
	/* 1 = �������ͻ�ӦACKDT���ӻ� */

#define	I2C_I2CCTRL_RCEN_Pos	11	/* ���ؽ���ģʽ�£�����ʹ��λ */
#define	I2C_I2CCTRL_RCEN_Msk	(0x1U << I2C_I2CCTRL_RCEN_Pos)
	/* 0 = ���ս�ֹ */
	/* 1 = ��������ʹ�� */

#define	I2C_I2CCTRL_PEN_Pos	10	/* STOPʱ�����ʹ�ܿ���λ */
#define	I2C_I2CCTRL_PEN_Msk	(0x1U << I2C_I2CCTRL_PEN_Pos)
	/* 0 = STOPʱ�������ֹ */
	/* 1 = STOPʱ�����ʹ�� */

#define	I2C_I2CCTRL_RSEN_Pos	9	/* Repeated STARTʱ�����ʹ�ܿ���λ */
#define	I2C_I2CCTRL_RSEN_Msk	(0x1U << I2C_I2CCTRL_RSEN_Pos)
	/* 0 = Repeated STARTʱ�������ֹ */
	/* 1 = Repeated STARTʱ�����ʹ�� */

#define	I2C_I2CCTRL_SEN_Pos	8	/* STARTʱ�����ʹ�ܿ���λ */
#define	I2C_I2CCTRL_SEN_Msk	(0x1U << I2C_I2CCTRL_SEN_Pos)
	/* 0 = STARTʱ�������ֹ */
	/* 1 = STARTʱ�����ʹ�� */

#define	I2C_I2CCTRL_I2CEN_Pos	0	/* I2Cģ��ʹ�ܿ���λ */
#define	I2C_I2CCTRL_I2CEN_Msk	(0x1U << I2C_I2CCTRL_I2CEN_Pos)
	/* 0 = I2C��ֹ(��λI2C) */
	/* 1 = I2Cʹ�� */

#define	I2C_I2CSTA_WCOL_Pos	6	/* д��ͻ���λ */
#define	I2C_I2CSTA_WCOL_Msk	(0x1U << I2C_I2CSTA_WCOL_Pos)

#define	I2C_I2CSTA_RW_Pos	5	/* I2C�ӿ�״̬λ */
#define	I2C_I2CSTA_RW_Msk	(0x1U << I2C_I2CSTA_RW_Pos)

#define	I2C_I2CSTA_P_Pos	4	/* STOP��־λ��MCU��ѯ��Ӳ����0 */
#define	I2C_I2CSTA_P_Msk	(0x1U << I2C_I2CSTA_P_Pos)

#define	I2C_I2CSTA_S_Pos	3	/* START��־λ��MCU��ѯ��Ӳ����0 */
#define	I2C_I2CSTA_S_Msk	(0x1U << I2C_I2CSTA_S_Pos)

#define	I2C_I2CSTA_BF_Pos	2	/* ��������״̬λ */
#define	I2C_I2CSTA_BF_Msk	(0x1U << I2C_I2CSTA_BF_Pos)

#define	I2C_I2CSTA_ACKSTA_Pos	1	/* ���ط���ģʽ�£����Դӻ��Ļ�Ӧ�ź� */
#define	I2C_I2CSTA_ACKSTA_Msk	(0x1U << I2C_I2CSTA_ACKSTA_Pos)

#define	I2C_I2CSTA_ACKDT_Pos	0	/* ���ؽ���ģʽ�£�������Ӧ�źŵ�״̬ */
#define	I2C_I2CSTA_ACKDT_Msk	(0x1U << I2C_I2CSTA_ACKDT_Pos)
#define	I2C_I2CSTA_ACKDT_L	(0x0U << I2C_I2CSTA_ACKDT_Pos)	/* 0 = ���������ӻ��Ļ�ӦΪ0 */
#define	I2C_I2CSTA_ACKDT_H	(0x1U << I2C_I2CSTA_ACKDT_Pos)	/* 1 = �����������ӻ���Ӧ */

#define	I2C_I2CBRG_SSPBRG_Pos	0	/* ���������üĴ��� */
#define	I2C_I2CBRG_SSPBRG_Msk	(0x1ffU << I2C_I2CBRG_SSPBRG_Pos)

#define	I2C_I2CBUF_SSPBUF_Pos	0	/* �շ�����Ĵ��� */
#define	I2C_I2CBUF_SSPBUF_Msk	(0xffU << I2C_I2CBUF_SSPBUF_Pos)

#define	I2C_I2CIR_I2CIE_Pos	1	/* I2C�ж�ʹ�ܿ���λ */
#define	I2C_I2CIR_I2CIE_Msk	(0x1U << I2C_I2CIR_I2CIE_Pos)
	/* 0 = I2C�жϽ�ֹ */
	/* 1 = I2C�ж�ʹ�� */

#define	I2C_I2CIR_I2CIF_Pos	0	/* I2C�жϱ�־λ */
#define	I2C_I2CIR_I2CIF_Msk	(0x1U << I2C_I2CIR_I2CIF_Pos)

#define	I2C_I2CFSM_I2CFSM_Pos	0	/* I2C��״̬������ */
#define	I2C_I2CFSM_I2CFSM_Msk	(0xfU << I2C_I2CFSM_I2CFSM_Pos)
#define	I2C_I2CFSM_I2CFSM_IDLE	(0x0U << I2C_I2CFSM_I2CFSM_Pos)	/* 0000 = IDLE */
#define	I2C_I2CFSM_I2CFSM_START1	(0x1U << I2C_I2CFSM_I2CFSM_Pos)	/* 0001 = START1 */
#define	I2C_I2CFSM_I2CFSM_START2	(0x2U << I2C_I2CFSM_I2CFSM_Pos)	/* 0010 = START2 */
#define	I2C_I2CFSM_I2CFSM_START_DONE	(0x3U << I2C_I2CFSM_I2CFSM_Pos)	/* 0011 = START_DONE */
#define	I2C_I2CFSM_I2CFSM_TX_STATE	(0x4U << I2C_I2CFSM_I2CFSM_Pos)	/* 0100 = TX_STATE */
#define	I2C_I2CFSM_I2CFSM_RXACK	(0x5U << I2C_I2CFSM_I2CFSM_Pos)	/* 0101 = RXACK */
#define	I2C_I2CFSM_I2CFSM_OP_IDLE	(0x6U << I2C_I2CFSM_I2CFSM_Pos)	/* 0110 = OP_IDLE */
#define	I2C_I2CFSM_I2CFSM_STOP1	(0x7U << I2C_I2CFSM_I2CFSM_Pos)	/* 0111 = STOP1 */
#define	I2C_I2CFSM_I2CFSM_STOP2	(0x8U << I2C_I2CFSM_I2CFSM_Pos)	/* 1000 = STOP2 */
#define	I2C_I2CFSM_I2CFSM_STOP_DONE	(0x9U << I2C_I2CFSM_I2CFSM_Pos)	/* 1001 = STOP_DONE */
#define	I2C_I2CFSM_I2CFSM_RTP_START	(0xaU << I2C_I2CFSM_I2CFSM_Pos)	/* 1010 = RTP_START */
#define	I2C_I2CFSM_I2CFSM_RX_STATE	(0xbU << I2C_I2CFSM_I2CFSM_Pos)	/* 1011 = RX_STATE */
#define	I2C_I2CFSM_I2CFSM_ACK_STATE	(0xcU << I2C_I2CFSM_I2CFSM_Pos)	/* 1100 = ACK_STATE */
#define	I2C_I2CFSM_I2CFSM_STOP0	(0xdU << I2C_I2CFSM_I2CFSM_Pos)	/* 1101 = STOP0 */

#define	I2C_I2CERR_ERRIE_Pos	8	/* �����־�ж�ʹ�� */
#define	I2C_I2CERR_ERRIE_Msk	(0x1U << I2C_I2CERR_ERRIE_Pos)

#define	I2C_I2CERR_OIERR_Pos	2	/* OP_IDLE״̬�´����־λ */
#define	I2C_I2CERR_OIERR_Msk	(0x1U << I2C_I2CERR_OIERR_Pos)

#define	I2C_I2CERR_SDERR_Pos	1	/* START_DONE״̬�´����־λ */
#define	I2C_I2CERR_SDERR_Msk	(0x1U << I2C_I2CERR_SDERR_Pos)

#define	I2C_I2CERR_IERR_Pos	0	/* IDLE״̬�´����־λ */
#define	I2C_I2CERR_IERR_Msk	(0x1U << I2C_I2CERR_IERR_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void I2C_Deinit(void);

/* ��I2Cģ���ֹʱ��SCL�̶�Ϊ�ߵ͵�ƽ����λ ��غ��� */
extern void I2C_I2CCTRL_SCLHL_Setable(FunState NewState);
extern FunState I2C_I2CCTRL_SCLHL_Getable(void);

/* ��I2Cģ���ֹʱ��SDA�̶�Ϊ�ߵ͵�ƽ����λ ��غ��� */
extern void I2C_I2CCTRL_SDAHL_Setable(FunState NewState);
extern FunState I2C_I2CCTRL_SDAHL_Getable(void);

/* ���ؽ���ģʽ�£��������ӻ���Ӧʹ��λ ��غ��� */
extern void I2C_I2CCTRL_ACKEN_Setable(FunState NewState);
extern FunState I2C_I2CCTRL_ACKEN_Getable(void);

/* ���ؽ���ģʽ�£�����ʹ��λ ��غ��� */
extern void I2C_I2CCTRL_RCEN_Setable(FunState NewState);
extern FunState I2C_I2CCTRL_RCEN_Getable(void);

/* STOPʱ�����ʹ�ܿ���λ ��غ��� */
extern void I2C_I2CCTRL_PEN_Setable(FunState NewState);

/* Repeated STARTʱ�����ʹ�ܿ���λ ��غ��� */
extern void I2C_I2CCTRL_RSEN_Setable(FunState NewState);

/* STARTʱ�����ʹ�ܿ���λ ��غ��� */
extern void I2C_I2CCTRL_SEN_Setable(FunState NewState);

/* I2Cģ��ʹ�ܿ���λ ��غ��� */
extern void I2C_I2CCTRL_I2CEN_Setable(FunState NewState);
extern FunState I2C_I2CCTRL_I2CEN_Getable(void);

/* д��ͻ���λ ��غ��� */
extern void I2C_I2CSTA_WCOL_Clr(void);
extern FlagStatus I2C_I2CSTA_WCOL_Chk(void);

/* I2C�ӿ�״̬λ ��غ��� */
extern FlagStatus I2C_I2CSTA_RW_Chk(void);

/* STOP��־λ��MCU��ѯ��Ӳ����0 ��غ��� */
extern FlagStatus I2C_I2CSTA_P_Chk(void);

/* START��־λ��MCU��ѯ��Ӳ����0 ��غ��� */
extern FlagStatus I2C_I2CSTA_S_Chk(void);

/* ��������״̬λ ��غ��� */
extern FlagStatus I2C_I2CSTA_BF_Chk(void);

/* ���ط���ģʽ�£����Դӻ��Ļ�Ӧ�ź� ��غ��� */
extern void I2C_I2CSTA_ACKSTA_Clr(void);
extern FlagStatus I2C_I2CSTA_ACKSTA_Chk(void);

/* ���ؽ���ģʽ�£�������Ӧ�źŵ�״̬ ��غ��� */
extern void I2C_I2CSTA_ACKDT_Set(uint32_t SetValue);
extern uint32_t I2C_I2CSTA_ACKDT_Get(void);

/* ���������üĴ��� ��غ��� */
extern void I2C_I2CBRG_Write(uint32_t SetValue);
extern uint32_t I2C_I2CBRG_Read(void);

/* �շ�����Ĵ��� ��غ��� */
extern void I2C_I2CBUF_Write(uint32_t SetValue);
extern uint32_t I2C_I2CBUF_Read(void);

/* I2C�ж�ʹ�ܿ���λ ��غ��� */
extern void I2C_I2CIR_I2CIE_Setable(FunState NewState);
extern FunState I2C_I2CIR_I2CIE_Getable(void);

/* I2C�жϱ�־λ ��غ��� */
extern void I2C_I2CIR_I2CIF_Clr(void);
extern FlagStatus I2C_I2CIR_I2CIF_Chk(void);

/* I2C��״̬������ ��غ��� */
extern uint32_t I2C_I2CFSM_I2CFSM_Get(void);

/* �����־�ж�ʹ�� ��غ��� */
extern void I2C_I2CERR_ERRIE_Setable(FunState NewState);
extern FunState I2C_I2CERR_ERRIE_Getable(void);

/* OP_IDLE״̬�´����־λ ��غ��� */
extern void I2C_I2CERR_OIERR_Clr(void);
extern FlagStatus I2C_I2CERR_OIERR_Chk(void);

/* START_DONE״̬�´����־λ ��غ��� */
extern void I2C_I2CERR_SDERR_Clr(void);
extern FlagStatus I2C_I2CERR_SDERR_Chk(void);

/* IDLE״̬�´����־λ ��غ��� */
extern void I2C_I2CERR_IERR_Clr(void);
extern FlagStatus I2C_I2CERR_IERR_Chk(void);
//Announce_End

extern uint32_t I2C_BaudREG_Calc(uint32_t I2CClk, uint32_t APBClk);

/* I2Cģ�鸴λ���� */
extern void I2C_ResetI2C(void);
#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_I2C_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/

