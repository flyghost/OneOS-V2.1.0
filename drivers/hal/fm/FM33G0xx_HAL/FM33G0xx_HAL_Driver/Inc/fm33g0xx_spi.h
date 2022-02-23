/**
  ******************************************************************************
  * @file    fm33g0xx_spi.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the SPI firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_SPI_H
#define __FM33G0XX_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
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

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  SPI Init Structure definition  
  */ 


typedef struct
{
	//uint32_t SSPA;              //slaveģʽ�¶Է���ʱ������е���������ʹ�ø�λֵ��������/* SETTING SENING AHESD HALF CYCLE */
	//FunState FLTEN;             //slaveģʽ��ʹ��4ns�˲�/* SET PAD(SSN/SCK/MOSI) FILTER ENABLE AT SLAVE  */	
	uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
}SPI_Slave_SInitTypeDef;

typedef struct
{
	uint32_t SSPA;				//slaveģʽ�¶Է���ʱ������е�������ǰ��
	FunState FLTEN;             //slaveģʽ��ʹ��4ns�˲�/
	uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	FunState TXO;               //TXONLYģʽ
	uint32_t TXO_AC;            //TXONLYӲ���Զ��������

	FunState SPIEN;             //SPIʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState ERRIE;             //SPI�����ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState TXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState RXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
}SPI_Slave_InitTypeDef;



typedef struct
{
	//uint32_t MSPA;			//masterģʽ�¶Բ�������е���������ʹ�ø�λֵ��������/* SETTING SAMPLE DELAY HALF CYCLE */
    //uint32_t WAIT;          	//ÿ�η�����8bit���ټ��뼸��sck���ڵĵȴ�ʱ�䣬����ʹ�ø�λֵ��������/* WAIT CYCLE BETWEEN TWO BYTE SENDING */	
    uint32_t BAUD_RATE;         //������/* SETTING BAUD_RATE */	
    uint32_t LSBF;             	//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
    uint32_t CPHOL;			    //ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/	
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	uint32_t SSNM;				//ÿ�η���8bit�󱣳�SSNΪ�ͻ������ߣ�SSNӲ��ģʽ��Ч/* SETTING SSN AFTER SENDING */	
	uint32_t SSNSEN;			//SSN��Ӳ�����ƻ������������/* SETTING 1 FOR CONTROL SSN BY SOFTWARE */

}SPI_Master_SInitTypeDef;

typedef struct
{
	uint32_t MSPA;              //masterģʽ�¶Բ�������е���(�ͺ�)�����ڸ���ͨ��ʱ����PCB�����ӳ�
	uint32_t WAIT;              //masterģʽ�£�ÿ�η�����8bit���ټ��뼸��sck���ڵĵȴ�ʱ��
	uint32_t BAUD_RATE; 		//������/* SETTING BAUD_RATE */	
	uint32_t LSBF;				//�ȷ��͸�λbit���ǵ�λbit/* SETTING 1 FOR LSB */		
	uint32_t CPHOL; 			//ʱ�Ӽ���ѡ��/* SETTING 1 FOR CLK STOP AT HIGH*/ 
	uint32_t CPHA;				//ʱ����λѡ��/* SETTING 1 FOR START AT SECOND CLK EDGE AND 0 FOR FIRST */
	FunState TXO;               //TXONLYģʽ
	uint32_t TXO_AC;            //TXONLYӲ���Զ��������
	uint32_t SSNM;				//ÿ�η���8bit�󱣳�SSNΪ�ͻ������ߣ�SSNӲ��ģʽ��Ч/* SETTING SSN AFTER SENDING */ 
	uint32_t SSNSEN;			//SSN��Ӳ�����ƻ������������/* SETTING 1 FOR CONTROL SSN BY SOFTWARE */

	FunState SPIEN;             //SPIʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState ERRIE;             //SPI�����ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState TXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	FunState RXIE;              //SPI��������ж�ʹ�ܣ���ʼ��ʱ����дΪDISABLE
	
}SPI_Master_InitTypeDef;





#define	SPIx_SPICR1_MSPA_Pos	10	/* Master��MISO�źŵĲ���λ�õ��������ڸ���ͨ��ʱ����PCB�����ӳ� */
#define	SPIx_SPICR1_MSPA_Msk	(0x1U << SPIx_SPICR1_MSPA_Pos)
#define	SPIx_SPICR1_MSPA_NORMAL	(0x0U << SPIx_SPICR1_MSPA_Pos)	/* �����㲻���� */
#define	SPIx_SPICR1_MSPA_HALF_DELAY	(0x1U << SPIx_SPICR1_MSPA_Pos)	/* �������ӳٰ��SCK���� */

#define	SPIx_SPICR1_SSPA_Pos	9	/* Slave MISO����λ�õ��� */
#define	SPIx_SPICR1_SSPA_Msk	(0x1U << SPIx_SPICR1_SSPA_Pos)
#define	SPIx_SPICR1_SSPA_NORMAL	(0x0U << SPIx_SPICR1_SSPA_Pos)	/* ���Ͳ���ǰ */
#define	SPIx_SPICR1_SSPA_HALF_AHEAD	(0x1U << SPIx_SPICR1_SSPA_Pos)	/* ��ǰ���SCK���ڷ��� */

#define	SPIx_SPICR1_MM_Pos	8	/* MASTER/SLAVEģʽѡ�� */
#define	SPIx_SPICR1_MM_Msk	(0x1U << SPIx_SPICR1_MM_Pos)
#define	SPIx_SPICR1_MM_SLAVE	(0x0U << SPIx_SPICR1_MM_Pos)	/* Slaveģʽ */
#define	SPIx_SPICR1_MM_MASTER	(0x1U << SPIx_SPICR1_MM_Pos)	/* Masterģʽ */

#define	SPIx_SPICR1_WAIT_Pos	6
#define	SPIx_SPICR1_WAIT_Msk	(0x3U << SPIx_SPICR1_WAIT_Pos)
#define	SPIx_SPICR1_WAIT_WAIT_1	(0x0U << SPIx_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���1��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	SPIx_SPICR1_WAIT_WAIT_2	(0x1U << SPIx_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���2��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	SPIx_SPICR1_WAIT_WAIT_3	(0x2U << SPIx_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���3��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */
#define	SPIx_SPICR1_WAIT_WAIT_4	(0x3U << SPIx_SPICR1_WAIT_Pos)	/* ÿ����8bit���ټ���4��sck���ڵȴ�ʱ���ٴ�����һ��8Bit���� */

#define	SPIx_SPICR1_BAUD_Pos	3	/* MASTERģʽ���������� */
#define	SPIx_SPICR1_BAUD_Msk	(0x7U << SPIx_SPICR1_BAUD_Pos)
#define	SPIx_SPICR1_BAUD_PCLK_2	(0x0U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��2��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_4	(0x1U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��4��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_8	(0x2U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��8��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_16	(0x3U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��16��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_32	(0x4U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��32��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_64	(0x5U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��64��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_128	(0x6U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��128��Ƶ */
#define	SPIx_SPICR1_BAUD_PCLK_256	(0x7U << SPIx_SPICR1_BAUD_Pos)	/* ����ʱ��256��Ƶ */

#define	SPIx_SPICR1_LSBF_Pos	2	/* ֡��ʽ */
#define	SPIx_SPICR1_LSBF_Msk	(0x1U << SPIx_SPICR1_LSBF_Pos)
#define	SPIx_SPICR1_LSBF_MSB	(0x0U << SPIx_SPICR1_LSBF_Pos)	/* �ȷ���MSB */
#define	SPIx_SPICR1_LSBF_LSB	(0x1U << SPIx_SPICR1_LSBF_Pos)	/* �ȷ���LSB */

#define	SPIx_SPICR1_CPHOL_Pos	1	/* ʱ�Ӽ���ѡ�� */
#define	SPIx_SPICR1_CPHOL_Msk	(0x1U << SPIx_SPICR1_CPHOL_Pos)
#define	SPIx_SPICR1_CPHOL_LOW	(0x0U << SPIx_SPICR1_CPHOL_Pos)	/* ����ʱ��ֹͣ�ڵ͵�ƽ */
#define	SPIx_SPICR1_CPHOL_HIGH	(0x1U << SPIx_SPICR1_CPHOL_Pos)	/* ����ʱ��ֹͣ�ڸߵ�ƽ */

#define	SPIx_SPICR1_CPHA_Pos	0	/* ʱ����λѡ�� */
#define	SPIx_SPICR1_CPHA_Msk	(0x1U << SPIx_SPICR1_CPHA_Pos)
#define	SPIx_SPICR1_CPHA_FIRST	(0x0U << SPIx_SPICR1_CPHA_Pos)	/* ��һ��ʱ�ӱ����ǵ�һ����׽���� */
#define	SPIx_SPICR1_CPHA_SECOND	(0x1U << SPIx_SPICR1_CPHA_Pos)	/* �ڶ���ʱ�ӱ����ǵ�һ����׽���� */

#define	SPIx_SPICR2_FLTEN_Pos	6	/* Slave����ܽ��˲�ʹ�ܣ�SSN/SCK/MOSI�� */
#define	SPIx_SPICR2_FLTEN_Msk	(0x1U << SPIx_SPICR2_FLTEN_Pos)
	/* 0����ʹ��4ns�˲� */
	/* 1��ʹ��4ns�˲� */

#define	SPIx_SPICR2_SSNM_Pos	5	/* Masterģʽ��SSN����ģʽѡ�� */
#define	SPIx_SPICR2_SSNM_Msk	(0x1U << SPIx_SPICR2_SSNM_Pos)
#define	SPIx_SPICR2_SSNM_LOW	(0x0U << SPIx_SPICR2_SSNM_Pos)	/* ÿ������8bit��Master����SSNΪ�� */
#define	SPIx_SPICR2_SSNM_HIGH	(0x1U << SPIx_SPICR2_SSNM_Pos)	/* ÿ������8bit��Master����SSN��ά�ָߵ�ƽʱ����WAIT�Ĵ������� */

#define	SPIx_SPICR2_TXO_AC_Pos	4	/* TXONLYӲ���Զ���յ�ʹ�� */
#define	SPIx_SPICR2_TXO_AC_Msk	(0x1U << SPIx_SPICR2_TXO_AC_Pos)
#define	SPIx_SPICR2_TXO_AC_CLR_AT_CLOSE	(0x0U << SPIx_SPICR2_TXO_AC_Pos)	/* �ر�TXONLYӲ���Զ����� */
#define	SPIx_SPICR2_TXO_AC_CLR_AT_TX	(0x1U << SPIx_SPICR2_TXO_AC_Pos)	/* TXONLYӲ���Զ�������Ч�����ʹ��TXO�󣬵ȴ�������Ϻ�Ӳ������ */

#define	SPIx_SPICR2_TXO_Pos	3	/* TXONLY����λ */
#define	SPIx_SPICR2_TXO_Msk	(0x1U << SPIx_SPICR2_TXO_Pos)
	/* 0���ر�Master�ĵ�����ģʽ */
	/* 1������Master�ĵ�����ģʽ */

#define	SPIx_SPICR2_SSN_Pos	2	/* Masterģʽ�£��������ͨ����λ����SSN�����ƽ */
#define	SPIx_SPICR2_SSN_Msk	(0x1U << SPIx_SPICR2_SSN_Pos)
#define	SPIx_SPICR2_SSN_LOW	(0x0U << SPIx_SPICR2_SSN_Pos)	/* SSN����͵�ƽ */
#define	SPIx_SPICR2_SSN_HIGH	(0x1U << SPIx_SPICR2_SSN_Pos)	/* SSN����ߵ�ƽ */

#define	SPIx_SPICR2_SSNSEN_Pos	1	/* Masterģʽ�£�SSN�Ŀ��Ʒ�ʽ */
#define	SPIx_SPICR2_SSNSEN_Msk	(0x1U << SPIx_SPICR2_SSNSEN_Pos)
#define	SPIx_SPICR2_SSNSEN_HARD	(0x0U << SPIx_SPICR2_SSNSEN_Pos)	/* Masterģʽ��SSN�����Ӳ���Զ����� */
#define	SPIx_SPICR2_SSNSEN_SOFT	(0x1U << SPIx_SPICR2_SSNSEN_Pos)	/* Masterģʽ��SSN������������ */

#define	SPIx_SPICR2_SPIEN_Pos	0	/* SPIʹ�� */
#define	SPIx_SPICR2_SPIEN_Msk	(0x1U << SPIx_SPICR2_SPIEN_Pos)
	/* 0���ر�SPI */
	/* 1��ʹ��SPI */

#define	SPIx_SPICR3_TXBFC_Pos	3
#define	SPIx_SPICR3_TXBFC_Msk	(0x1U << SPIx_SPICR3_TXBFC_Pos)

#define	SPIx_SPICR3_RXBFC_Pos	2
#define	SPIx_SPICR3_RXBFC_Msk	(0x1U << SPIx_SPICR3_RXBFC_Pos)

#define	SPIx_SPICR3_MERRC_Pos	1
#define	SPIx_SPICR3_MERRC_Msk	(0x1U << SPIx_SPICR3_MERRC_Pos)

#define	SPIx_SPICR3_SERRC_Pos	0
#define	SPIx_SPICR3_SERRC_Msk	(0x1U << SPIx_SPICR3_SERRC_Pos)

#define	SPIx_SPIIE_ERRIE_Pos	2
#define	SPIx_SPIIE_ERRIE_Msk	(0x1U << SPIx_SPIIE_ERRIE_Pos)
	/* 0��HSPI�����жϲ�ʹ�� */
	/* 1��HSPI�����ж�ʹ�� */

#define	SPIx_SPIIE_TXIE_Pos	1
#define	SPIx_SPIIE_TXIE_Msk	(0x1U << SPIx_SPIIE_TXIE_Pos)
	/* 0����������жϲ�ʹ�� */
	/* 1����������ж�ʹ�� */

#define	SPIx_SPIIE_RXIE_Pos	0
#define	SPIx_SPIIE_RXIE_Msk	(0x1U << SPIx_SPIIE_RXIE_Pos)
	/* 0����������жϲ�ʹ�� */
	/* 1����������ж�ʹ�� */

#define	SPIx_SPIIF_MERR_Pos	6	/* Master Error��־ */
#define	SPIx_SPIIF_MERR_Msk	(0x1U << SPIx_SPIIF_MERR_Pos)

#define	SPIx_SPIIF_SERR_Pos	5	/* Slave Error��־ */
#define	SPIx_SPIIF_SERR_Msk	(0x1U << SPIx_SPIIF_SERR_Pos)

#define	SPIx_SPIIF_RXCOL_Pos	4	/* ���ջ�����������д1���� */
#define	SPIx_SPIIF_RXCOL_Msk	(0x1U << SPIx_SPIIF_RXCOL_Pos)

#define	SPIx_SPIIF_TXCOL_Pos	3	/* ���ͻ�����������д1���� */
#define	SPIx_SPIIF_TXCOL_Msk	(0x1U << SPIx_SPIIF_TXCOL_Pos)

#define	SPIx_SPIIF_BUSY_Pos	2	/* SPI���б�־��ֻ�� */
#define	SPIx_SPIIF_BUSY_Msk	(0x1U << SPIx_SPIIF_BUSY_Pos)

#define	SPIx_SPIIF_TXBE_Pos	1	/* TX Buffer Empty��־λ */
#define	SPIx_SPIIF_TXBE_Msk	(0x1U << SPIx_SPIIF_TXBE_Pos)

#define	SPIx_SPIIF_RXBF_Pos	0	/* RX Buffer Full��־λ */
#define	SPIx_SPIIF_RXBF_Msk	(0x1U << SPIx_SPIIF_RXBF_Pos)

#define	SPIx_SPITXBUF_TXBUF_Pos	0	/* SPI���ͻ��� */
#define	SPIx_SPITXBUF_TXBUF_Msk	(0xffU << SPIx_SPITXBUF_TXBUF_Pos)

#define	SPIx_SPIRXBUF_RXBUF_Pos	0	/* SPI���ջ��� */
#define	SPIx_SPIRXBUF_RXBUF_Msk	(0xffU << SPIx_SPIRXBUF_RXBUF_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void SPIx_Deinit(SPIx_Type* SPIx);

/* Master��MISO�źŵĲ���λ�õ��������ڸ���ͨ��ʱ����PCB�����ӳ� ��غ��� */
extern void SPIx_SPICR1_MSPA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_MSPA_Get(SPIx_Type* SPIx);

/* Slave MISO����λ�õ��� ��غ��� */
extern void SPIx_SPICR1_SSPA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_SSPA_Get(SPIx_Type* SPIx);

/* MASTER/SLAVEģʽѡ�� ��غ��� */
extern void SPIx_SPICR1_MM_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_MM_Get(SPIx_Type* SPIx);
extern void SPIx_SPICR1_WAIT_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_WAIT_Get(SPIx_Type* SPIx);

/* MASTERģʽ���������� ��غ��� */
extern void SPIx_SPICR1_BAUD_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_BAUD_Get(SPIx_Type* SPIx);

/* ֡��ʽ ��غ��� */
extern void SPIx_SPICR1_LSBF_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_LSBF_Get(SPIx_Type* SPIx);

/* ʱ�Ӽ���ѡ�� ��غ��� */
extern void SPIx_SPICR1_CPHOL_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_CPHOL_Get(SPIx_Type* SPIx);

/* ʱ����λѡ�� ��غ��� */
extern void SPIx_SPICR1_CPHA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_CPHA_Get(SPIx_Type* SPIx);

/* Slave����ܽ��˲�ʹ�ܣ�SSN/SCK/MOSI�� ��غ��� */
extern void SPIx_SPICR2_FLTEN_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_FLTEN_Getable(SPIx_Type* SPIx);

/* Masterģʽ��SSN����ģʽѡ�� ��غ��� */
extern void SPIx_SPICR2_SSNM_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSNM_Get(SPIx_Type* SPIx);

/* TXONLYӲ���Զ���յ�ʹ�� ��غ��� */
extern void SPIx_SPICR2_TXO_AC_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_TXO_AC_Get(SPIx_Type* SPIx);

/* TXONLY����λ ��غ��� */
extern void SPIx_SPICR2_TXO_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_TXO_Getable(SPIx_Type* SPIx);

/* Masterģʽ�£��������ͨ����λ����SSN�����ƽ ��غ��� */
extern void SPIx_SPICR2_SSN_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSN_Get(SPIx_Type* SPIx);

/* Masterģʽ�£�SSN�Ŀ��Ʒ�ʽ ��غ��� */
extern void SPIx_SPICR2_SSNSEN_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSNSEN_Get(SPIx_Type* SPIx);

/* SPIʹ�� ��غ��� */
extern void SPIx_SPICR2_SPIEN_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_SPIEN_Getable(SPIx_Type* SPIx);
extern void SPIx_SPICR3_TXBFC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_RXBFC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_MERRC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_SERRC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPIIE_ERRIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_ERRIE_Getable(SPIx_Type* SPIx);
extern void SPIx_SPIIE_TXIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_TXIE_Getable(SPIx_Type* SPIx);
extern void SPIx_SPIIE_RXIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_RXIE_Getable(SPIx_Type* SPIx);

/* Master Error��־ ��غ��� */
extern FlagStatus SPIx_SPIIF_MERR_Chk(SPIx_Type* SPIx);

/* Slave Error��־ ��غ��� */
extern FlagStatus SPIx_SPIIF_SERR_Chk(SPIx_Type* SPIx);

/* ���ջ�����������д1���� ��غ��� */
extern void SPIx_SPIIF_RXCOL_Clr(SPIx_Type* SPIx);
extern FlagStatus SPIx_SPIIF_RXCOL_Chk(SPIx_Type* SPIx);

/* ���ͻ�����������д1���� ��غ��� */
extern void SPIx_SPIIF_TXCOL_Clr(SPIx_Type* SPIx);
extern FlagStatus SPIx_SPIIF_TXCOL_Chk(SPIx_Type* SPIx);

/* SPI���б�־��ֻ�� ��غ��� */
extern FlagStatus SPIx_SPIIF_BUSY_Chk(SPIx_Type* SPIx);

/* TX Buffer Empty��־λ ��غ��� */
extern FlagStatus SPIx_SPIIF_TXBE_Chk(SPIx_Type* SPIx);

/* RX Buffer Full��־λ ��غ��� */
extern FlagStatus SPIx_SPIIF_RXBF_Chk(SPIx_Type* SPIx);

/* SPI���ͻ��� ��غ��� */
extern void SPIx_SPITXBUF_Write(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPITXBUF_Read(SPIx_Type* SPIx);

/* SPI���ջ��� ��غ��� */
extern void SPIx_SPIRXBUF_Write(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPIRXBUF_Read(SPIx_Type* SPIx);
//Announce_End


/* Exported functions --------------------------------------------------------*/ 
extern void SPIx_Deinit(SPIx_Type* SPIx);

/* Master��MISO�źŵĲ���λ�õ��������ڸ���ͨ��ʱ����PCB�����ӳ� ��غ��� */
extern void SPIx_SPICR1_MSPA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_MSPA_Get(SPIx_Type* SPIx);

/* Slave MISO����λ�õ��� ��غ��� */
extern void SPIx_SPICR1_SSPA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_SSPA_Get(SPIx_Type* SPIx);

/* MASTER/SLAVEģʽѡ�� ��غ��� */
extern void SPIx_SPICR1_MM_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_MM_Get(SPIx_Type* SPIx);
extern void SPIx_SPICR1_WAIT_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_WAIT_Get(SPIx_Type* SPIx);

/* MASTERģʽ���������� ��غ��� */
extern void SPIx_SPICR1_BAUD_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_BAUD_Get(SPIx_Type* SPIx);

/* ֡��ʽ ��غ��� */
extern void SPIx_SPICR1_LSBF_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_LSBF_Get(SPIx_Type* SPIx);

/* ʱ�Ӽ���ѡ�� ��غ��� */
extern void SPIx_SPICR1_CPHOL_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_CPHOL_Get(SPIx_Type* SPIx);

/* ʱ����λѡ�� ��غ��� */
extern void SPIx_SPICR1_CPHA_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR1_CPHA_Get(SPIx_Type* SPIx);

/* Slave����ܽ��˲�ʹ�ܣ�SSN/SCK/MOSI�� ��غ��� */
extern void SPIx_SPICR2_FLTEN_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_FLTEN_Getable(SPIx_Type* SPIx);

/* Masterģʽ��SSN����ģʽѡ�� ��غ��� */
extern void SPIx_SPICR2_SSNM_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSNM_Get(SPIx_Type* SPIx);

/* TXONLYӲ���Զ���յ�ʹ�� ��غ��� */
extern void SPIx_SPICR2_TXO_AC_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_TXO_AC_Get(SPIx_Type* SPIx);

/* TXONLY����λ ��غ��� */
extern void SPIx_SPICR2_TXO_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_TXO_Getable(SPIx_Type* SPIx);

/* Masterģʽ�£��������ͨ����λ����SSN�����ƽ ��غ��� */
extern void SPIx_SPICR2_SSN_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSN_Get(SPIx_Type* SPIx);

/* Masterģʽ�£�SSN�Ŀ��Ʒ�ʽ ��غ��� */
extern void SPIx_SPICR2_SSNSEN_Set(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPICR2_SSNSEN_Get(SPIx_Type* SPIx);

/* SPIʹ�� ��غ��� */
extern void SPIx_SPICR2_SPIEN_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPICR2_SPIEN_Getable(SPIx_Type* SPIx);
extern void SPIx_SPICR3_TXBFC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_RXBFC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_MERRC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPICR3_SERRC_Clr(SPIx_Type* SPIx);
extern void SPIx_SPIIE_ERRIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_ERRIE_Getable(SPIx_Type* SPIx);
extern void SPIx_SPIIE_TXIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_TXIE_Getable(SPIx_Type* SPIx);
extern void SPIx_SPIIE_RXIE_Setable(SPIx_Type* SPIx, FunState NewState);
extern FunState SPIx_SPIIE_RXIE_Getable(SPIx_Type* SPIx);

/* Master Error��־ ��غ��� */
extern FlagStatus SPIx_SPIIF_MERR_Chk(SPIx_Type* SPIx);

/* Slave Error��־ ��غ��� */
extern FlagStatus SPIx_SPIIF_SERR_Chk(SPIx_Type* SPIx);

/* ���ջ�����������д1���� ��غ��� */
extern void SPIx_SPIIF_RXCOL_Clr(SPIx_Type* SPIx);
extern FlagStatus SPIx_SPIIF_RXCOL_Chk(SPIx_Type* SPIx);

/* ���ͻ�����������д1���� ��غ��� */
extern void SPIx_SPIIF_TXCOL_Clr(SPIx_Type* SPIx);
extern FlagStatus SPIx_SPIIF_TXCOL_Chk(SPIx_Type* SPIx);

/* SPI���б�־��ֻ�� ��غ��� */
extern FlagStatus SPIx_SPIIF_BUSY_Chk(SPIx_Type* SPIx);

/* TX Buffer Empty��־λ ��غ��� */
extern FlagStatus SPIx_SPIIF_TXBE_Chk(SPIx_Type* SPIx);

/* RX Buffer Full��־λ ��غ��� */
extern FlagStatus SPIx_SPIIF_RXBF_Chk(SPIx_Type* SPIx);

/* SPI���ͻ��� ��غ��� */
extern void SPIx_SPITXBUF_Write(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPITXBUF_Read(SPIx_Type* SPIx);

/* SPI���ջ��� ��غ��� */
extern void SPIx_SPIRXBUF_Write(SPIx_Type* SPIx, uint32_t SetValue);
extern uint32_t SPIx_SPIRXBUF_Read(SPIx_Type* SPIx);
//Announce_End


//Announce_HandBy
extern void SPI_Master_Init(SPIx_Type* SPIx, SPI_Master_SInitTypeDef* para);
extern void SPI_Slave_Init(SPIx_Type* SPIx, SPI_Slave_SInitTypeDef* para);

extern void SPI_SSN_Set_Low(SPIx_Type* SPIx);//SSN�õ�
extern void SPI_SSN_Set_High(SPIx_Type* SPIx);//SSN�ø�

extern uint8_t SPI_Recv_Byte(SPIx_Type* SPIx);//SPI����һ�ֽ�
extern void SPI_Send_Byte(SPIx_Type* SPIx, uint8_t data);//SPI����һ�ֽ�

extern uint8_t SPI_RW_Byte(SPIx_Type* SPIx, uint8_t data);//SPI���Ͳ�����һ�ֽ�
//Announce_End


#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_SPI_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/











