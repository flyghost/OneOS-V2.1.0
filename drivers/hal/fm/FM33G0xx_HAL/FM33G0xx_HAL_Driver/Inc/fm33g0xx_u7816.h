/**
  ******************************************************************************
  * @file    fm33g0xx_u7816.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the U7816 firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_U7816_H
#define __FM33G0XX_U7816_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"

/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */


	 
/* Exported constants --------------------------------------------------------*/

/** @defgroup U7816_Exported_Constants
  * @{
  */	 
/* Exported types ------------------------------------------------------------*/


typedef struct
{
	FunState				TXEN;		//����ʹ��
	FunState				RXEN;		//����ʹ��
	FunState				CKOEN;		//U7816ʱ��CLK���ʹ�ܿ���
	FunState				HPUAT;		//U7816ͨ�����ݷ���ǿ���������Զ���Ч����
	FunState				HPUEN;		//U7816ͨ��ǿ����ʹ�ܿ���
	
	uint32_t				ERSW;		//ERROR SIGNAL���ѡ��
	uint32_t				ERSGD;		//ERROR SIGNAL��GUARDTIME���ѡ�񣨽��ڷ���ʱ��Ч��
	FunState				BGTEN;		//BGT��block guard time������
	uint32_t				REP_T;		//���ƽ���������żУ�����ʱ�Զ��ط�����
	uint32_t				PAR;		//��żУ������ѡ��
	FunState				FREN;		//Guard Time���ȿ���λ������ʱ�ϸ���Э��2etu��
	FunState				TREPEN;		//������������żУ���Ĵ���ʽѡ��
	FunState				RREPEN;		//����������żУ���Ĵ���ʽѡ��
	FunState				DICONV;		//������������ʹ��
	
	uint32_t				TXEGT;		//����ʱ�����EGTʱ�䣨��ETUΪ��λ��
	uint32_t				CLKDIV;		//U7816ʱ�������Ƶ���ƼĴ���
	uint32_t				PDIV;		//U7816Ԥ��Ƶ���ƼĴ���������7816ͨ�ŷ�Ƶ�ȣ������ʣ�
	
	FunState				RXIE;		//���ݽ����ж�ʹ��λ����ӦRX_FLAG�жϱ�־λ
	FunState				TXIE;		//���ݷ����ж�ʹ��λ����ӦTX_FLAG�жϱ�־λ
	FunState				LSIE;		//��·״̬�ж�ʹ��λ����ӦERROR_FLAG�жϱ�־λ
	
}U7816_InitTypeDef;



#define	U7816x_U7816CTRL_TXEN_Pos	5	/* U7816ͨ������ʹ�ܿ��� */
#define	U7816x_U7816CTRL_TXEN_Msk	(0x1U << U7816x_U7816CTRL_TXEN_Pos)
	/* 0 = ͨ�����ͽ�ֹ�����ɷ������ݣ����ض�����˿� */
	/* 1 = ͨ������ʹ�ܣ��ɷ������� */

#define	U7816x_U7816CTRL_RXEN_Pos	4	/* U7816ͨ������ʹ�ܿ��� */
#define	U7816x_U7816CTRL_RXEN_Msk	(0x1U << U7816x_U7816CTRL_RXEN_Pos)
	/* 0 = ͨ�����ս�ֹ�����ɽ������ݣ����ض�����˿� */
	/* 1 = ͨ������ʹ�ܣ��ɽ������� */

#define	U7816x_U7816CTRL_CKOEN_Pos	3	/* U7816ʱ��CLK���ʹ�ܿ��� */
#define	U7816x_U7816CTRL_CKOEN_Msk	(0x1U << U7816x_U7816CTRL_CKOEN_Pos)
	/* 0 = 7816ʱ�������ֹ */
	/* 1 = 7816ʱ�����ʹ�� */

#define	U7816x_U7816CTRL_HPUAT_Pos	2	/* U7816ͨ�����ݷ���ǿ���������Զ���Ч���� */
#define	U7816x_U7816CTRL_HPUAT_Msk	(0x1U << U7816x_U7816CTRL_HPUAT_Pos)
	/* 0 = ���ݷ���ʱ���������Զ���Ч���ܽ�ֹ������������HPUEN��LPUEN���� */
	/* 1 = ���ݷ���ʱ���������Զ���Ч������̬����������Ч */

#define	U7816x_U7816CTRL_HPUEN_Pos	1	/* U7816ͨ��ǿ����ʹ�ܿ��� */
#define	U7816x_U7816CTRL_HPUEN_Msk	(0x1U << U7816x_U7816CTRL_HPUEN_Pos)
	/* 0 = ǿ������Ч */
	/* 1 = ǿ������Ч */

#define	U7816x_U7816FRC_ERSW_Pos	9	/* ERROR SIGNAL���ѡ�� */
#define	U7816x_U7816FRC_ERSW_Msk	(0x3U << U7816x_U7816FRC_ERSW_Pos)
#define	U7816x_U7816FRC_ERSW_1ETU	(0x3U << U7816x_U7816FRC_ERSW_Pos)	/* 11 = ERROR SIGNAL���Ϊ1ETU; */
#define	U7816x_U7816FRC_ERSW_1P5ETU	(0x2U << U7816x_U7816FRC_ERSW_Pos)	/* 10 = ERROR SIGNAL���Ϊ1.5ETU; */
#define	U7816x_U7816FRC_ERSW_2ETU	(0x0U << U7816x_U7816FRC_ERSW_Pos)	/* 01/00 = ERROR SIGNAL���Ϊ2ETU; */

#define	U7816x_U7816FRC_ERSGD_Pos	8	/* ERROR SIGNAL��GUARDTIME���ѡ�񣨽��ڷ���ʱ��Ч�� */
#define	U7816x_U7816FRC_ERSGD_Msk	(0x1U << U7816x_U7816FRC_ERSGD_Pos)
#define	U7816x_U7816FRC_ERSGD_1PETU	(0x1U << U7816x_U7816FRC_ERSGD_Pos)	/* 1 = ERROR SIGNAL��GUARDTIMEΪ1~1.5ETU�� */
#define	U7816x_U7816FRC_ERSGD_2PETU	(0x0U << U7816x_U7816FRC_ERSGD_Pos)	/* 0 = ERROR SIGNAL��GUARDTIMEΪ2~2.5ETU�� */

#define	U7816x_U7816FRC_BGTEN_Pos	7	/* BGT��block guard time������ */
#define	U7816x_U7816FRC_BGTEN_Msk	(0x1U << U7816x_U7816FRC_BGTEN_Pos)
	/* 0 = BGT��ֹ��������Block guard time(22 etu); */
	/* 1 = BGTʹ�ܣ�����Block guard time(22 etu); */

#define	U7816x_U7816FRC_REP_T_Pos	6	/* ���ƽ���������żУ�����ʱ�Զ��ط����� */
#define	U7816x_U7816FRC_REP_T_Msk	(0x1U << U7816x_U7816FRC_REP_T_Pos)
#define	U7816x_U7816FRC_REP_T_1TIME	(0x0U << U7816x_U7816FRC_REP_T_Pos)	/* 0 = 1�� */
#define	U7816x_U7816FRC_REP_T_3TIME	(0x1U << U7816x_U7816FRC_REP_T_Pos)	/* 1 = 3�� */

#define	U7816x_U7816FRC_PAR_Pos	4	/* ��żУ������ѡ�� */
#define	U7816x_U7816FRC_PAR_Msk	(0x3U << U7816x_U7816FRC_PAR_Pos)
#define	U7816x_U7816FRC_PAR_EVEN	(0x0U << U7816x_U7816FRC_PAR_Pos)	/* 00��Even */
#define	U7816x_U7816FRC_PAR_ODD	(0x1U << U7816x_U7816FRC_PAR_Pos)	/* 01��Odd */
#define	U7816x_U7816FRC_PAR_ALWAYS1	(0x2U << U7816x_U7816FRC_PAR_Pos)	/* 10��Always 1 */
#define	U7816x_U7816FRC_PAR_NONE	(0x3U << U7816x_U7816FRC_PAR_Pos)	/* 11����У�飬���� */

#define	U7816x_U7816FRC_FREN_Pos	3	/* Guard Time���ȿ���λ������ʱ�ϸ���Э��2etu�� */
#define	U7816x_U7816FRC_FREN_Msk	(0x1U << U7816x_U7816FRC_FREN_Pos)
	/* 0 = Guard timeΪ2 etu */
	/* 1 = Guard timeΪ1 etu */

#define	U7816x_U7816FRC_TREPEN_Pos	2	/* ������������żУ���Ĵ���ʽѡ�� */
#define	U7816x_U7816FRC_TREPEN_Msk	(0x1U << U7816x_U7816FRC_TREPEN_Pos)
	/* 0 = �յ�Error signalʱ�������Զ��ط�����tx_parity_err��־��ֱ���ж� */
	/* 1 = �յ���żУ������־��error signal��������T��0Э���Զ����лط����ڵ�һbyte�ظ����ʹ�������REP_T����tx_parity_err��־�������ж� */

#define	U7816x_U7816FRC_RREPEN_Pos	1	/* ����������żУ���Ĵ���ʽѡ�� */
#define	U7816x_U7816FRC_RREPEN_Msk	(0x1U << U7816x_U7816FRC_RREPEN_Pos)
	/* 0 = ��żУ������Զ�����ERROR SIGNAL����RX_PARITY_ERR��־�������ж� */
	/* 1 = ��żУ�������T=0Э���Զ��ط�ERROR SIGNAL����һBYTE�������մ�������REP_T����RX_PARITY_ERR��־�������ж� */

#define	U7816x_U7816FRC_DICONV_Pos	0	/* ������������ʹ�� */
#define	U7816x_U7816FRC_DICONV_Msk	(0x1U << U7816x_U7816FRC_DICONV_Pos)
	/* 0 = ������룬���շ�LSB �� (�շ�����+У��λ)���߼���ƽ */
	/* 1 = ������룬���շ�MSB��(�շ�����+У��λ)���߼���ƽ */

#define	U7816x_U7816EGT_TXEGT_Pos	0	/* ����ʱ�����EGTʱ�䣨��ETUΪ��λ�� */
#define	U7816x_U7816EGT_TXEGT_Msk	(0xffU << U7816x_U7816EGT_TXEGT_Pos)

#define	U7816x_U7816CLKDIV_CLKDIV_Pos	0	/* U7816ʱ�������Ƶ���ƼĴ��� */
#define	U7816x_U7816CLKDIV_CLKDIV_Msk	(0x1fU << U7816x_U7816CLKDIV_CLKDIV_Pos)

#define	U7816x_U7816PDIV_PDIV_Pos	0	/* U7816Ԥ��Ƶ���ƼĴ���������7816ͨ�ŷ�Ƶ�ȣ������ʣ� */
#define	U7816x_U7816PDIV_PDIV_Msk	(0xfffU << U7816x_U7816PDIV_PDIV_Pos)

#define	U7816x_U7816RXBUF_RXBUF_Pos	0	/* U7816���ݽ��ջ���Ĵ��� */
#define	U7816x_U7816RXBUF_RXBUF_Msk	(0xffU << U7816x_U7816RXBUF_RXBUF_Pos)

#define	U7816x_U7816TXBUF_TXBUF_Pos	0	/* U7816���ݷ��ͻ���Ĵ��� */
#define	U7816x_U7816TXBUF_TXBUF_Msk	(0xffU << U7816x_U7816TXBUF_TXBUF_Pos)

#define	U7816x_U7816IE_RXIE_Pos	2	/* ���ݽ����ж�ʹ��λ����ӦRX_FLAG�жϱ�־λ */
#define	U7816x_U7816IE_RXIE_Msk	(0x1U << U7816x_U7816IE_RXIE_Pos)
	/* 0 = ���ݽ����ж�ʹ��λ */
	/* 1 = ���ݽ����ж�ʹ��λ */

#define	U7816x_U7816IE_TXIE_Pos	1	/* ���ݷ����ж�ʹ��λ����ӦTX_FLAG�жϱ�־λ */
#define	U7816x_U7816IE_TXIE_Msk	(0x1U << U7816x_U7816IE_TXIE_Pos)
	/* 0 = ���ݷ����жϽ�ֹ */
	/* 1 = ���ݷ����ж�ʹ�� */

#define	U7816x_U7816IE_LSIE_Pos	0	/* ��·״̬�ж�ʹ��λ����ӦERROR_FLAG�жϱ�־λ */
#define	U7816x_U7816IE_LSIE_Msk	(0x1U << U7816x_U7816IE_LSIE_Pos)
	/* 0 = ��·״̬�жϽ�ֹ */
	/* 1 = ��·״̬�ж�ʹ�� */

#define	U7816x_U7816IF_ERRIF_Pos	2	/* �����־���Ĵ������ó����������г���Ӳ����λ����U7816ERR����Ӧ�����־���� */
#define	U7816x_U7816IF_ERRIF_Msk	(0x1U << U7816x_U7816IF_ERRIF_Pos)

#define	U7816x_U7816IF_TXIF_Pos	1	/* ���ͻ������ձ�־ */
#define	U7816x_U7816IF_TXIF_Msk	(0x1U << U7816x_U7816IF_TXIF_Pos)

#define	U7816x_U7816IF_RXIF_Pos	0	/* ������ɱ�־ */
#define	U7816x_U7816IF_RXIF_Msk	(0x1U << U7816x_U7816IF_RXIF_Pos)

#define	U7816x_U7816ERR_TPARERR_Pos	3	/* ����������żУ������־λ */
#define	U7816x_U7816ERR_TPARERR_Msk	(0x1U << U7816x_U7816ERR_TPARERR_Pos)

#define	U7816x_U7816ERR_RPARERR_Pos	2	/* ����������żУ������־λ */
#define	U7816x_U7816ERR_RPARERR_Msk	(0x1U << U7816x_U7816ERR_RPARERR_Pos)

#define	U7816x_U7816ERR_FRERR_Pos	1	/* ����֡��ʽ�����־λ */
#define	U7816x_U7816ERR_FRERR_Msk	(0x1U << U7816x_U7816ERR_FRERR_Pos)

#define	U7816x_U7816ERR_OVERR_Pos	0	/* ������������־λ */
#define	U7816x_U7816ERR_OVERR_Msk	(0x1U << U7816x_U7816ERR_OVERR_Pos)

#define	U7816x_U7816STA_WAIT_RPT_Pos	2	/* U7816�ӿڷ����˴����źţ����ڵȴ��Է��ط�����.״̬�����뷢�ʹ����ź�״̬ʱ��λ���յ�������ʼλ���߽��뷢��״̬ʱӲ������;
 */
#define	U7816x_U7816STA_WAIT_RPT_Msk	(0x1U << U7816x_U7816STA_WAIT_RPT_Pos)

#define	U7816x_U7816STA_TXBUSY_Pos	1	/* ��������æ��־ */
#define	U7816x_U7816STA_TXBUSY_Msk	(0x1U << U7816x_U7816STA_TXBUSY_Pos)

#define	U7816x_U7816STA_RXBUSY_Pos	0	/* ��������æ��־ */
#define	U7816x_U7816STA_RXBUSY_Msk	(0x1U << U7816x_U7816STA_RXBUSY_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void U7816x_Deinit(U7816x_Type* U7816x);

/* U7816ͨ������ʹ�ܿ��� ��غ��� */
extern void U7816x_U7816CTRL_TXEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816CTRL_TXEN_Getable(U7816x_Type* U7816x);

/* U7816ͨ������ʹ�ܿ��� ��غ��� */
extern void U7816x_U7816CTRL_RXEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816CTRL_RXEN_Getable(U7816x_Type* U7816x);

/* U7816ʱ��CLK���ʹ�ܿ��� ��غ��� */
extern void U7816x_U7816CTRL_CKOEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816CTRL_CKOEN_Getable(U7816x_Type* U7816x);

/* U7816ͨ�����ݷ���ǿ���������Զ���Ч���� ��غ��� */
extern void U7816x_U7816CTRL_HPUAT_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816CTRL_HPUAT_Getable(U7816x_Type* U7816x);

/* U7816ͨ��ǿ����ʹ�ܿ��� ��غ��� */
extern void U7816x_U7816CTRL_HPUEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816CTRL_HPUEN_Getable(U7816x_Type* U7816x);

/* ERROR SIGNAL���ѡ�� ��غ��� */
extern void U7816x_U7816FRC_ERSW_Set(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816FRC_ERSW_Get(U7816x_Type* U7816x);

/* ERROR SIGNAL��GUARDTIME���ѡ�񣨽��ڷ���ʱ��Ч�� ��غ��� */
extern void U7816x_U7816FRC_ERSGD_Set(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816FRC_ERSGD_Get(U7816x_Type* U7816x);

/* BGT��block guard time������ ��غ��� */
extern void U7816x_U7816FRC_BGTEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816FRC_BGTEN_Getable(U7816x_Type* U7816x);

/* ���ƽ���������żУ�����ʱ�Զ��ط����� ��غ��� */
extern void U7816x_U7816FRC_REP_T_Set(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816FRC_REP_T_Get(U7816x_Type* U7816x);

/* ��żУ������ѡ�� ��غ��� */
extern void U7816x_U7816FRC_PAR_Set(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816FRC_PAR_Get(U7816x_Type* U7816x);

/* Guard Time���ȿ���λ������ʱ�ϸ���Э��2etu�� ��غ��� */
extern void U7816x_U7816FRC_FREN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816FRC_FREN_Getable(U7816x_Type* U7816x);

/* ������������żУ���Ĵ���ʽѡ�� ��غ��� */
extern void U7816x_U7816FRC_TREPEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816FRC_TREPEN_Getable(U7816x_Type* U7816x);

/* ����������żУ���Ĵ���ʽѡ�� ��غ��� */
extern void U7816x_U7816FRC_RREPEN_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816FRC_RREPEN_Getable(U7816x_Type* U7816x);

/* ������������ʹ�� ��غ��� */
extern void U7816x_U7816FRC_DICONV_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816FRC_DICONV_Getable(U7816x_Type* U7816x);

/* ����ʱ�����EGTʱ�䣨��ETUΪ��λ�� ��غ��� */
extern void U7816x_U7816EGT_Write(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816EGT_Read(U7816x_Type* U7816x);

/* U7816ʱ�������Ƶ���ƼĴ��� ��غ��� */
extern void U7816x_U7816CLKDIV_Write(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816CLKDIV_Read(U7816x_Type* U7816x);

/* U7816Ԥ��Ƶ���ƼĴ���������7816ͨ�ŷ�Ƶ�ȣ������ʣ� ��غ��� */
extern void U7816x_U7816PDIV_Write(U7816x_Type* U7816x, uint32_t SetValue);
extern uint32_t U7816x_U7816PDIV_Read(U7816x_Type* U7816x);

/* U7816���ݽ��ջ���Ĵ��� ��غ��� */
extern uint32_t U7816x_U7816RXBUF_Read(U7816x_Type* U7816x);

/* U7816���ݷ��ͻ���Ĵ��� ��غ��� */
extern void U7816x_U7816TXBUF_Write(U7816x_Type* U7816x, uint32_t SetValue);

/* ���ݽ����ж�ʹ��λ����ӦRX_FLAG�жϱ�־λ ��غ��� */
extern void U7816x_U7816IE_RXIE_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816IE_RXIE_Getable(U7816x_Type* U7816x);

/* ���ݷ����ж�ʹ��λ����ӦTX_FLAG�жϱ�־λ ��غ��� */
extern void U7816x_U7816IE_TXIE_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816IE_TXIE_Getable(U7816x_Type* U7816x);

/* ��·״̬�ж�ʹ��λ����ӦERROR_FLAG�жϱ�־λ ��غ��� */
extern void U7816x_U7816IE_LSIE_Setable(U7816x_Type* U7816x, FunState NewState);
extern FunState U7816x_U7816IE_LSIE_Getable(U7816x_Type* U7816x);

/* �����־���Ĵ������ó����������г���Ӳ����λ����U7816ERR����Ӧ�����־���� ��غ��� */
extern FlagStatus U7816x_U7816IF_ERRIF_Chk(U7816x_Type* U7816x);

/* ���ͻ������ձ�־ ��غ��� */
extern FlagStatus U7816x_U7816IF_TXIF_Chk(U7816x_Type* U7816x);

/* ������ɱ�־ ��غ��� */
extern FlagStatus U7816x_U7816IF_RXIF_Chk(U7816x_Type* U7816x);

/* ����������żУ������־λ ��غ��� */
extern void U7816x_U7816ERR_TPARERR_Clr(U7816x_Type* U7816x);
extern FlagStatus U7816x_U7816ERR_TPARERR_Chk(U7816x_Type* U7816x);

/* ����������żУ������־λ ��غ��� */
extern void U7816x_U7816ERR_RPARERR_Clr(U7816x_Type* U7816x);
extern FlagStatus U7816x_U7816ERR_RPARERR_Chk(U7816x_Type* U7816x);

/* ����֡��ʽ�����־λ ��غ��� */
extern void U7816x_U7816ERR_FRERR_Clr(U7816x_Type* U7816x);
extern FlagStatus U7816x_U7816ERR_FRERR_Chk(U7816x_Type* U7816x);

/* ������������־λ ��غ��� */
extern void U7816x_U7816ERR_OVERR_Clr(U7816x_Type* U7816x);
extern FlagStatus U7816x_U7816ERR_OVERR_Chk(U7816x_Type* U7816x);

/* U7816�ӿڷ����˴����źţ����ڵȴ��Է��ط�����.״̬�����뷢�ʹ����ź�״̬ʱ��λ���յ�������ʼλ���߽��뷢��״̬ʱӲ������;
 ��غ��� */
extern FlagStatus U7816x_U7816STA_WAIT_RPT_Chk(U7816x_Type* U7816x);

/* ��������æ��־ ��غ��� */
extern FlagStatus U7816x_U7816STA_TXBUSY_Chk(U7816x_Type* U7816x);

/* ��������æ��־ ��غ��� */
extern FlagStatus U7816x_U7816STA_RXBUSY_Chk(U7816x_Type* U7816x);
//Announce_End


/* U7816����������ʼ������ */
void U7816_Init(U7816x_Type* U7816x, U7816_InitTypeDef* para);



#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_U7816_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



