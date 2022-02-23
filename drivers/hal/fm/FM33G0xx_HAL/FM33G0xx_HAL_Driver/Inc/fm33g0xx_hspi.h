/**
  ******************************************************************************
  * @file    fm33g0xx_hspi.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the SPI firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_HSPI_H
#define __FM33G0XX_HSPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "FM33G0XX_define_all.h"
#include "FM33G0XX.h" 

/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup SPI
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/

/** @defgroup SPI_Exported_Constants
  * @{
  */

typedef struct
{
	//uint32_t DELAY_CFG;       //slaveģʽ�¶Է�����λ���е���������ʹ�ø�λֵ��������/* SETTING SENING AHESD HALF CYCLE */
	//FunState FLTEN;             //slaveģʽ��ʹ��4ns�˲�/* SET PAD(SSN/SCK/MOSI) FILTER ENABLE AT SLAVE  */	
	uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
}HSPI_Slave_SInitTypeDef;

typedef struct
{
	uint32_t DELAY_CFG;         //slaveģʽ�¶Է�����λ���е���
	FunState FLTEN;             //slaveģʽ��ʹ��4ns�˲�
	uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	FunState TXO;               //TXONLYģʽ
	uint32_t TXO_AC;            //TXONLYӲ���Զ��������

	FunState HSPIEN;             //SPIʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState ERRIE;             //SPI�����ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState TXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState RXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
}HSPI_Slave_InitTypeDef;

typedef struct
{
	//uint32_t DELAY_CFG        //masterģʽ�¶Բ�����λ���е���������ʹ�ø�λֵ��������
	uint32_t BAUD_RATE;         //������/* SETTING BAUD_RATE */	
    uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	uint32_t SSNM;				//ÿ�η���8bit�󱣳�SSNΪ�ͻ������ߣ�SSNӲ��ģʽ��Ч/* SETTING SSN AFTER SENDING */	
	uint32_t SSNSEN;			//SSN��Ӳ�����ƻ������������/* SETTING 1 FOR CONTROL SSN BY SOFTWARE */

}HSPI_Master_SInitTypeDef;

typedef struct
{
	uint32_t DELAY_CFG;         //masterģʽ�¶Բ�����λ���е���
	uint32_t WAIT;              //masterģʽ�£�ÿ�η�����8bit���ټ��뼸��sck���ڵĵȴ�ʱ��
	uint32_t BAUD_RATE;         //������/* SETTING BAUD_RATE */	
    uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	FunState TXO;               //TXONLYģʽ
	uint32_t TXO_AC;            //TXONLYӲ���Զ��������
    uint32_t SSNM;				//ÿ�η���8bit�󱣳�SSNΪ�ͻ������ߣ�SSNӲ��ģʽ��Ч/* SETTING SSN AFTER SENDING */ 
	uint32_t SSNSEN;			//SSN��Ӳ�����ƻ������������/* SETTING 1 FOR CONTROL SSN BY SOFTWARE */

	FunState HSPIEN;             //SPIʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState ERRIE;             //SPI�����ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState TXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState RXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
}HSPI_Master_InitTypeDef;



#define	HSPI_SPICR1_DELAY_CFG_Pos	9	/* Masterģʽ�£����ڵ������ղ�����λ��Slaveģʽ�£����ڵ���������λ */
#define	HSPI_SPICR1_DELAY_CFG_Msk	(0x7U << HSPI_SPICR1_DELAY_CFG_Pos)
#define	HSPI_SPICR1_DELAY_CFG_MASTER_NO_DELAY	(0x0U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Masterģʽ�����ӳ� */
#define	HSPI_SPICR1_DELAY_CFG_SLAVE_NORMAL	(0x0U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Slaveģʽ���������� */
#define	HSPI_SPICR1_DELAY_CFG_MASTER_DLY_L1	(0x1U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Masterģʽ��1���ӳ� */
#define	HSPI_SPICR1_DELAY_CFG_SLAVE_PHASE_1	(0x1U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Slaveģʽ��������λ1 */
#define	HSPI_SPICR1_DELAY_CFG_MASTER_DLY_L2	(0x2U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Masterģʽ��2���ӳ� */
#define	HSPI_SPICR1_DELAY_CFG_SLAVE_PHASE_2	(0x2U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Slaveģʽ��������λ2 */
#define	HSPI_SPICR1_DELAY_CFG_MASTER_DLY_L3	(0x3U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Masterģʽ��3���ӳ� */
#define	HSPI_SPICR1_DELAY_CFG_SLAVE_PHASE_3	(0x3U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Slaveģʽ��������λ3 */
#define	HSPI_SPICR1_DELAY_CFG_MASTER_DLY_L4	(0x4U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Masterģʽ��4���ӳ� */
#define	HSPI_SPICR1_DELAY_CFG_SLAVE_PHASE_4	(0x4U << HSPI_SPICR1_DELAY_CFG_Pos)	/* Slaveģʽ��������λ4 */

#define	HSPI_SPICR1_MM_Pos	8	/* MASTER/SLAVEģʽѡ�� */
#define	HSPI_SPICR1_MM_Msk	(0x1U << HSPI_SPICR1_MM_Pos)
#define	HSPI_SPICR1_MM_SLAVE	(0x0U << HSPI_SPICR1_MM_Pos)	/* Slaveģʽ */
#define	HSPI_SPICR1_MM_MASTER	(0x1U << HSPI_SPICR1_MM_Pos)	/* Masterģʽ */

#define	HSPI_SPICR1_WAIT_Pos	6
#define	HSPI_SPICR1_WAIT_Msk	(0x3U << HSPI_SPICR1_WAIT_Pos)
#define	HSPI_SPICR1_WAIT_WAIT_1	(0x0U << HSPI_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���1��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	HSPI_SPICR1_WAIT_WAIT_2	(0x1U << HSPI_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���2��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	HSPI_SPICR1_WAIT_WAIT_3	(0x2U << HSPI_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���3��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	HSPI_SPICR1_WAIT_WAIT_4	(0x3U << HSPI_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���4��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */

#define	HSPI_SPICR1_BAUD_Pos	3	/* MASTERģʽ���������� */
#define	HSPI_SPICR1_BAUD_Msk	(0x7U << HSPI_SPICR1_BAUD_Pos)
#define	HSPI_SPICR1_BAUD_PCLK	(0x0U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ�Ӳ���Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_2	(0x1U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��2��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_4	(0x2U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��4��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_8	(0x3U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��8��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_16	(0x4U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��16��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_32	(0x5U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��32��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_64	(0x6U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��64��Ƶ */
#define	HSPI_SPICR1_BAUD_PCLK_128	(0x7U << HSPI_SPICR1_BAUD_Pos)	/* ����ʱ��128��Ƶ */

#define	HSPI_SPICR1_LSBF_Pos	2	/* ֡��ʽ */
#define	HSPI_SPICR1_LSBF_Msk	(0x1U << HSPI_SPICR1_LSBF_Pos)
#define	HSPI_SPICR1_LSBF_MSB	(0x0U << HSPI_SPICR1_LSBF_Pos)	/* �ȷ���MSB */
#define	HSPI_SPICR1_LSBF_LSB	(0x1U << HSPI_SPICR1_LSBF_Pos)	/* �ȷ���LSB */

#define	HSPI_SPICR1_CPHOL_Pos	1	/* ʱ�Ӽ���ѡ�� */
#define	HSPI_SPICR1_CPHOL_Msk	(0x1U << HSPI_SPICR1_CPHOL_Pos)
#define	HSPI_SPICR1_CPHOL_LOW	(0x0U << HSPI_SPICR1_CPHOL_Pos)	/* ����ʱ��ֹͣ�ڵ͵�ƽ */
#define	HSPI_SPICR1_CPHOL_HIGH	(0x1U << HSPI_SPICR1_CPHOL_Pos)	/* ����ʱ��ֹͣ�ڸߵ�ƽ */

#define	HSPI_SPICR1_CPHA_Pos	0	/* ʱ����λѡ�� */
#define	HSPI_SPICR1_CPHA_Msk	(0x1U << HSPI_SPICR1_CPHA_Pos)
#define	HSPI_SPICR1_CPHA_FIRST	(0x0U << HSPI_SPICR1_CPHA_Pos)	/* ��һ��ʱ�ӱ����ǵ�һ����׽���� */
#define	HSPI_SPICR1_CPHA_SECOND	(0x1U << HSPI_SPICR1_CPHA_Pos)	/* �ڶ���ʱ�ӱ����ǵ�һ����׽���� */

#define	HSPI_SPICR2_FLTEN_Pos	6	/* Slave����ܽ��˲�ʹ�ܣ�SSN/SCK/MOSI�� */
#define	HSPI_SPICR2_FLTEN_Msk	(0x1U << HSPI_SPICR2_FLTEN_Pos)
	/* 0����ʹ��4ns�˲� */
	/* 1��ʹ��4ns�˲� */

#define	HSPI_SPICR2_SSNM_Pos	5	/* Masterģʽ��SSN����ģʽѡ�� */
#define	HSPI_SPICR2_SSNM_Msk	(0x1U << HSPI_SPICR2_SSNM_Pos)
#define	HSPI_SPICR2_SSNM_LOW	(0x0U << HSPI_SPICR2_SSNM_Pos)	/* ÿ������8bit��Master����SSNΪ�� */
#define	HSPI_SPICR2_SSNM_HIGH	(0x1U << HSPI_SPICR2_SSNM_Pos)	/* ÿ������8bit��Master����SSN��ά�ָߵ�ƽʱ����WAIT�Ĵ������� */

#define	HSPI_SPICR2_TXO_AC_Pos	4	/* TXONLYӲ���Զ���յ�ʹ�� */
#define	HSPI_SPICR2_TXO_AC_Msk	(0x1U << HSPI_SPICR2_TXO_AC_Pos)
#define	HSPI_SPICR2_TXO_AC_CLR_AT_CLOSE	(0x0U << HSPI_SPICR2_TXO_AC_Pos)	/* �ر�TXONLYӲ���Զ����� */
#define	HSPI_SPICR2_TXO_AC_CLR_AT_TX	(0x1U << HSPI_SPICR2_TXO_AC_Pos)	/* TXONLYӲ���Զ�������Ч�����ʹ��TXO�󣬵ȴ�������Ϻ�Ӳ������ */

#define	HSPI_SPICR2_TXO_Pos	3	/* TXONLY����λ */
#define	HSPI_SPICR2_TXO_Msk	(0x1U << HSPI_SPICR2_TXO_Pos)
	/* 0���ر�Master�ĵ�����ģʽ */
	/* 1������Master�ĵ�����ģʽ */

#define	HSPI_SPICR2_SSN_Pos	2	/* Masterģʽ�£��������ͨ����λ����SSN�����ƽ */
#define	HSPI_SPICR2_SSN_Msk	(0x1U << HSPI_SPICR2_SSN_Pos)
#define	HSPI_SPICR2_SSN_LOW	(0x0U << HSPI_SPICR2_SSN_Pos)	/* SSN����͵�ƽ */
#define	HSPI_SPICR2_SSN_HIGH	(0x1U << HSPI_SPICR2_SSN_Pos)	/* SSN����ߵ�ƽ */

#define	HSPI_SPICR2_SSNSEN_Pos	1	/* Masterģʽ�£�SSN�Ŀ��Ʒ�ʽ */
#define	HSPI_SPICR2_SSNSEN_Msk	(0x1U << HSPI_SPICR2_SSNSEN_Pos)
#define	HSPI_SPICR2_SSNSEN_HARD	(0x0U << HSPI_SPICR2_SSNSEN_Pos)	/* Masterģʽ��SSN�����Ӳ���Զ����� */
#define	HSPI_SPICR2_SSNSEN_SOFT	(0x1U << HSPI_SPICR2_SSNSEN_Pos)	/* Masterģʽ��SSN������������ */

#define	HSPI_SPICR2_HSPIEN_Pos	0	/* HSPIʹ�� */
#define	HSPI_SPICR2_HSPIEN_Msk	(0x1U << HSPI_SPICR2_HSPIEN_Pos)
	/* 0���ر�HSPI */
	/* 1��ʹ��HSPI */

#define	HSPI_SPICR3_TXBFC_Pos	3
#define	HSPI_SPICR3_TXBFC_Msk	(0x1U << HSPI_SPICR3_TXBFC_Pos)

#define	HSPI_SPICR3_RXBFC_Pos	2
#define	HSPI_SPICR3_RXBFC_Msk	(0x1U << HSPI_SPICR3_RXBFC_Pos)

#define	HSPI_SPICR3_MERRC_Pos	1
#define	HSPI_SPICR3_MERRC_Msk	(0x1U << HSPI_SPICR3_MERRC_Pos)

#define	HSPI_SPICR3_SERRC_Pos	0
#define	HSPI_SPICR3_SERRC_Msk	(0x1U << HSPI_SPICR3_SERRC_Pos)

#define	HSPI_SPIIE_ERRIE_Pos	2
#define	HSPI_SPIIE_ERRIE_Msk	(0x1U << HSPI_SPIIE_ERRIE_Pos)
	/* 0��HSPI�����жϲ�ʹ�� */
	/* 1��HSPI�����ж�ʹ�� */

#define	HSPI_SPIIE_TXIE_Pos	1
#define	HSPI_SPIIE_TXIE_Msk	(0x1U << HSPI_SPIIE_TXIE_Pos)
	/* 0����������жϲ�ʹ�� */
	/* 1����������ж�ʹ�� */

#define	HSPI_SPIIE_RXIE_Pos	0
#define	HSPI_SPIIE_RXIE_Msk	(0x1U << HSPI_SPIIE_RXIE_Pos)
	/* 0����������жϲ�ʹ�� */

#define	HSPI_SPIIF_MERR_Pos	6	/* Master Error��־ */
#define	HSPI_SPIIF_MERR_Msk	(0x1U << HSPI_SPIIF_MERR_Pos)

#define	HSPI_SPIIF_SERR_Pos	5	/* Slave Error��־ */
#define	HSPI_SPIIF_SERR_Msk	(0x1U << HSPI_SPIIF_SERR_Pos)

#define	HSPI_SPIIF_RXCOL_Pos	4	/* ���ջ�����������д1���� */
#define	HSPI_SPIIF_RXCOL_Msk	(0x1U << HSPI_SPIIF_RXCOL_Pos)

#define	HSPI_SPIIF_TXCOL_Pos	3	/* ���ͻ�����������д1���� */
#define	HSPI_SPIIF_TXCOL_Msk	(0x1U << HSPI_SPIIF_TXCOL_Pos)

#define	HSPI_SPIIF_BUSY_Pos	2	/* HSPI���б�־��ֻ�� */
#define	HSPI_SPIIF_BUSY_Msk	(0x1U << HSPI_SPIIF_BUSY_Pos)

#define	HSPI_SPIIF_TXBE_Pos	1	/* TX Buffer Empty��־λ */
#define	HSPI_SPIIF_TXBE_Msk	(0x1U << HSPI_SPIIF_TXBE_Pos)

#define	HSPI_SPIIF_RXBF_Pos	0	/* RX Buffer Full��־λ */
#define	HSPI_SPIIF_RXBF_Msk	(0x1U << HSPI_SPIIF_RXBF_Pos)

#define	HSPI_SPITXBUF_TXBUF_Pos	0	/* SPI���ͻ��� */
#define	HSPI_SPITXBUF_TXBUF_Msk	(0xffU << HSPI_SPITXBUF_TXBUF_Pos)

#define	HSPI_SPIRXBUF_RXBUF_Pos	0	/* SPI���ջ��� */
#define	HSPI_SPIRXBUF_RXBUF_Msk	(0xffU << HSPI_SPIRXBUF_RXBUF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void HSPI_Deinit(void);

/* Masterģʽ�£����ڵ������ղ�����λ��Slaveģʽ�£����ڵ���������λ ��غ��� */
extern void HSPI_SPICR1_DELAY_CFG_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_DELAY_CFG_Get(void);

/* MASTER/SLAVEģʽѡ�� ��غ��� */
extern void HSPI_SPICR1_MM_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_MM_Get(void);
extern void HSPI_SPICR1_WAIT_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_WAIT_Get(void);

/* MASTERģʽ���������� ��غ��� */
extern void HSPI_SPICR1_BAUD_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_BAUD_Get(void);

/* ֡��ʽ ��غ��� */
extern void HSPI_SPICR1_LSBF_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_LSBF_Get(void);

/* ʱ�Ӽ���ѡ�� ��غ��� */
extern void HSPI_SPICR1_CPHOL_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_CPHOL_Get(void);

/* ʱ����λѡ�� ��غ��� */
extern void HSPI_SPICR1_CPHA_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR1_CPHA_Get(void);

/* Slave����ܽ��˲�ʹ�ܣ�SSN/SCK/MOSI�� ��غ��� */
extern void HSPI_SPICR2_FLTEN_Setable(FunState NewState);
extern FunState HSPI_SPICR2_FLTEN_Getable(void);

/* Masterģʽ��SSN����ģʽѡ�� ��غ��� */
extern void HSPI_SPICR2_SSNM_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR2_SSNM_Get(void);

/* TXONLYӲ���Զ���յ�ʹ�� ��غ��� */
extern void HSPI_SPICR2_TXO_AC_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR2_TXO_AC_Get(void);

/* TXONLY����λ ��غ��� */
extern void HSPI_SPICR2_TXO_Setable(FunState NewState);
extern FunState HSPI_SPICR2_TXO_Getable(void);

/* Masterģʽ�£��������ͨ����λ����SSN�����ƽ ��غ��� */
extern void HSPI_SPICR2_SSN_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR2_SSN_Get(void);

/* Masterģʽ�£�SSN�Ŀ��Ʒ�ʽ ��غ��� */
extern void HSPI_SPICR2_SSNSEN_Set(uint32_t SetValue);
extern uint32_t HSPI_SPICR2_SSNSEN_Get(void);

/* HSPIʹ�� ��غ��� */
extern void HSPI_SPICR2_HSPIEN_Setable(FunState NewState);
extern FunState HSPI_SPICR2_HSPIEN_Getable(void);
extern void HSPI_SPICR3_TXBFC_Clr(void);
extern void HSPI_SPICR3_RXBFC_Clr(void);
extern void HSPI_SPICR3_MERRC_Clr(void);
extern void HSPI_SPICR3_SERRC_Clr(void);
extern void HSPI_SPIIE_ERRIE_Setable(FunState NewState);
extern FunState HSPI_SPIIE_ERRIE_Getable(void);
extern void HSPI_SPIIE_TXIE_Setable(FunState NewState);
extern FunState HSPI_SPIIE_TXIE_Getable(void);
extern void HSPI_SPIIE_RXIE_Setable(FunState NewState);
extern FunState HSPI_SPIIE_RXIE_Getable(void);

/* Master Error��־ ��غ��� */
extern FlagStatus HSPI_SPIIF_MERR_Chk(void);

/* Slave Error��־ ��غ��� */
extern FlagStatus HSPI_SPIIF_SERR_Chk(void);

/* ���ջ�����������д1���� ��غ��� */
extern void HSPI_SPIIF_RXCOL_Clr(void);
extern FlagStatus HSPI_SPIIF_RXCOL_Chk(void);

/* ���ͻ�����������д1���� ��غ��� */
extern void HSPI_SPIIF_TXCOL_Clr(void);
extern FlagStatus HSPI_SPIIF_TXCOL_Chk(void);

/* HSPI���б�־��ֻ�� ��غ��� */
extern FlagStatus HSPI_SPIIF_BUSY_Chk(void);

/* TX Buffer Empty��־λ ��غ��� */
extern FlagStatus HSPI_SPIIF_TXBE_Chk(void);

/* RX Buffer Full��־λ ��غ��� */
extern FlagStatus HSPI_SPIIF_RXBF_Chk(void);

/* SPI���ͻ��� ��غ��� */
extern void HSPI_SPITXBUF_Write(uint32_t SetValue);
extern uint32_t HSPI_SPITXBUF_Read(void);

/* SPI���ջ��� ��غ��� */
extern void HSPI_SPIRXBUF_Write(uint32_t SetValue);
extern uint32_t HSPI_SPIRXBUF_Read(void);
//Announce_End

//Code_Start_HandBy
extern void HSPI_Master_Init(HSPI_Master_SInitTypeDef* para);

extern void HSPI_Slave_Init(HSPI_Slave_SInitTypeDef* para);

extern void HSPI_SSN_Set_Low(void);//������

extern void HSPI_SSN_Set_High(void);//������

extern uint8_t HSPI_Recv_Byte(void);//HSPI����һ�ֽ�
extern void HSPI_Send_Byte(uint8_t data);//HSPI����һ�ֽ�
extern uint8_t HSPI_RW_Byte(uint8_t data);//SPI���Ͳ�����һ�ֽ�

#ifdef __cplusplus
  }
#endif
  
#endif /* __FM33G0XX_SPI_H */
  
  
  
  /************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/
  
  
  

