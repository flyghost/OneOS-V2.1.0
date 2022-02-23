/**
  ******************************************************************************
  * @file    fm33g0xx_dma.h
  * @author  FM33g0x Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the DMA firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_DMA_H
#define __FM33G0XX_DMA_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"


typedef enum {
DMA_CH0 = 0, DMA_CH1 = 1, DMA_CH2 = 2, DMA_CH3 = 3, DMA_CH4 = 4, DMA_CH5 = 5, DMA_CH6 = 6, DMA_CH7 = 7	
}DMA_CH_Type;


typedef struct
{
	DMA_CH_Type CHx;			//DMAͨ����
	uint32_t CHxTSIZE;          //ͨ�����䳤��
	uint32_t CHxPRI;            //ͨ�����ȼ�
	uint32_t CHxINC;            //CH0~CH6ͨ����ַ��������,CH7������
	uint32_t CHxSSEL;           //CH0~CH6����ͨ��ѡ��,CH7���䷽ʽѡ��
	FunState CHxFTIE;           //ͨ����������ж�ʹ��
	FunState CHxHTIE;           //ͨ���������ж�ʹ��
	FunState CHxEN;             //ͨ��ʹ��
	uint32_t CHxRAMAD;          //CH0~CH7ͨ��RAMָ���ַ,ע��CH7ʹ�õ���word��ַ���������Ѵ�����ˣ�

	uint32_t CH7FLSAD;			//ͨ��FLASHָ���ַ�������ͨ��7������	
}DMA_InitTypeDef;




#define	DMA_GLOBALCTRL_DMAEN_Pos	0	/* DMAȫ��ʹ�� */
#define	DMA_GLOBALCTRL_DMAEN_Msk	(0x1U << DMA_GLOBALCTRL_DMAEN_Pos)
	/* 0��DMA�ر� */
	/* 1��DMAʹ�� */


#define	DMA_CHxCTRL_TSIZE_Pos	16	/* Channelx���䳤�� */
#define	DMA_CHxCTRL_TSIZE_Msk	(0x1fffU << DMA_CHxCTRL_TSIZE_Pos)

#define	DMA_CHxCTRL_PRI_Pos	12	/* Channelx���ȼ� */
#define	DMA_CHxCTRL_PRI_Msk	(0x3U << DMA_CHxCTRL_PRI_Pos)
#define	DMA_CHxCTRL_PRI_LOW	(0x0U << DMA_CHxCTRL_PRI_Pos)	/* �����ȼ� */
#define	DMA_CHxCTRL_PRI_MEDIUM	(0x1U << DMA_CHxCTRL_PRI_Pos)	/* �����ȼ� */
#define	DMA_CHxCTRL_PRI_HIGH	(0x2U << DMA_CHxCTRL_PRI_Pos)	/* �����ȼ� */
#define	DMA_CHxCTRL_PRI_VERY_HIGH	(0x3U << DMA_CHxCTRL_PRI_Pos)	/* ������ȼ� */

#define	DMA_CHxCTRL_INC_Pos	11	/* RAM��ַ�������� */
#define	DMA_CHxCTRL_INC_Msk	(0x1U << DMA_CHxCTRL_INC_Pos)
#define	DMA_CHxCTRL_INC_DEC	(0x0U << DMA_CHxCTRL_INC_Pos)	/* ��ַ�ݼ� */
#define	DMA_CHxCTRL_INC_INC	(0x1U << DMA_CHxCTRL_INC_Pos)	/* ��ַ���� */

#define	DMA_CHxCTRL_FTIE_Pos	2	/* Channelx��������ж�ʹ�� */
#define	DMA_CHxCTRL_FTIE_Msk	(0x1U << DMA_CHxCTRL_FTIE_Pos)
	/* 0���رմ�������ж� */
	/* 1��ʹ�ܴ�������ж� */

#define	DMA_CHxCTRL_HTIE_Pos	1	/* Channelx��̴�������ж�ʹ�� */
#define	DMA_CHxCTRL_HTIE_Msk	(0x1U << DMA_CHxCTRL_HTIE_Pos)
	/* 0���رհ���ж� */
	/* 1��ʹ�ܰ���ж� */

#define	DMA_CHxCTRL_EN_Pos	0	/* Channelxʹ�� */
#define	DMA_CHxCTRL_EN_Msk	(0x1U << DMA_CHxCTRL_EN_Pos)
	/* 0���ر�ͨ��0 */
	/* 1��ʹ��ͨ��0 */

#define	DMA_CHxRAMAD_RAMAD_Pos	0	/* Channelxָ��Ĵ��� */
#define	DMA_CHxRAMAD_RAMAD_Msk	(0x7fffU << DMA_CHxRAMAD_RAMAD_Pos)


#define	DMA_CHxCTRL_SSEL_Pos	8	/* Channel0����ͨ��ѡ�� */
#define	DMA_CHxCTRL_SSEL_Msk	(0x7U << DMA_CH0CTRL_CH0SSEL_Pos)

#define	DMA_CH0CTRL_CH0SSEL_Pos	8	/* Channel0����ͨ��ѡ�� */
#define	DMA_CH0CTRL_CH0SSEL_Msk	(0x7U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_ET4_CAP	(0x0U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_ET2_CAP	(0x1U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_BT2_CAP	(0x2U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_ET3_CAP	(0x3U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_BT1_CAP	(0x4U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_CRC_OUT	(0x5U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_AES_IN	(0x6U << DMA_CH0CTRL_CH0SSEL_Pos)
#define	DMA_CH0CTRL_CH0SSEL_ADC_OUT	(0x7U << DMA_CH0CTRL_CH0SSEL_Pos)

#define	DMA_CH1CTRL_CH1SSEL_Pos	8	/* Channel1����ͨ��ѡ�� */
#define	DMA_CH1CTRL_CH1SSEL_Msk	(0x7U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_ET1_CAP	(0x0U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_AES_OUT	(0x1U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_I2C_RX	(0x2U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_U78160_RX	(0x3U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_UART3_RX	(0x4U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_UART0_RX	(0x5U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_HSPI_RX	(0x6U << DMA_CH1CTRL_CH1SSEL_Pos)
#define	DMA_CH1CTRL_CH1SSEL_ADC_OUT	(0x7U << DMA_CH1CTRL_CH1SSEL_Pos)

#define	DMA_CH2CTRL_CH2SSEL_Pos	8	/* Channel2����ͨ��ѡ�� */
#define	DMA_CH2CTRL_CH2SSEL_Msk	(0x7U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_ET3_CAP	(0x0U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_BT1_CAP	(0x1U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_AES_OUT	(0x2U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_I2C_TX	(0x3U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_U78160_TX	(0x4U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_UART3_TX	(0x5U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_UART0_TX	(0x6U << DMA_CH2CTRL_CH2SSEL_Pos)
#define	DMA_CH2CTRL_CH2SSEL_HSPI_TX	(0x7U << DMA_CH2CTRL_CH2SSEL_Pos)

#define	DMA_CH3CTRL_CH3SSEL_Pos	8	/* Channel3����ͨ��ѡ�� */
#define	DMA_CH3CTRL_CH3SSEL_Msk	(0x7U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_BT2_CAP	(0x0U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_ET1_CAP	(0x1U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_I2C_TX	(0x2U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_U78161_RX	(0x3U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_UART4_RX	(0x4U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_UART1_RX	(0x5U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_UART0_RX	(0x6U << DMA_CH3CTRL_CH3SSEL_Pos)
#define	DMA_CH3CTRL_CH3SSEL_SPI1_RX	(0x7U << DMA_CH3CTRL_CH3SSEL_Pos)

#define	DMA_CH4CTRL_CH4SSEL_Pos	8	/* Channel4����ͨ��ѡ�� */
#define	DMA_CH4CTRL_CH4SSEL_Msk	(0x7U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_ET2_CAP	(0x0U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_AES_IN	(0x1U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_I2C_RX	(0x2U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_U78161_TX	(0x3U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_UART4_TX	(0x4U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_UART1_TX	(0x5U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_UART0_TX	(0x6U << DMA_CH4CTRL_CH4SSEL_Pos)
#define	DMA_CH4CTRL_CH4SSEL_SPI1_TX	(0x7U << DMA_CH4CTRL_CH4SSEL_Pos)

#define	DMA_CH5CTRL_CH5SSEL_Pos	8	/* Channel5����ͨ��ѡ�� */
#define	DMA_CH5CTRL_CH5SSEL_Msk	(0x7U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_LPUART_RX	(0x0U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_U78161_RX	(0x1U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_U78160_RX	(0x2U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_UART5_RX	(0x3U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_UART2_RX	(0x4U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_UART1_RX	(0x5U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_SPI2_RX	(0x6U << DMA_CH5CTRL_CH5SSEL_Pos)
#define	DMA_CH5CTRL_CH5SSEL_HSPI_RX	(0x7U << DMA_CH5CTRL_CH5SSEL_Pos)

#define	DMA_CH6CTRL_CH6SSEL_Pos	8	/* Channel5����ͨ��ѡ�� */
#define	DMA_CH6CTRL_CH6SSEL_Msk	(0x7U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_LPUART_TX	(0x0U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_U78161_TX	(0x1U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_U78160_TX	(0x2U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_UART5_TX	(0x3U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_UART2_TX	(0x4U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_UART1_TX	(0x5U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_SPI2_TX	(0x6U << DMA_CH6CTRL_CH6SSEL_Pos)
#define	DMA_CH6CTRL_CH6SSEL_HSPI_TX	(0x7U << DMA_CH6CTRL_CH6SSEL_Pos)


//BIT10	CH7DIR	Channel7���䷽��
//1��Flash->RAM����
//0��RAM->Flash����
//BIT9	CH7RI	Channel7 RAM��ַ�������ã�����Flash->RAM��������Ч
//1��RAM��ַ����
//0��RAM��ַ�ݼ�
//BIT8	CH7FI	Channel7 Flash��ַ�������ã�����Flash->RAM��������Ч
//1��Flash��ַ����
//0��Flash��ַ�ݼ�
#define	DMA_CH7CTRL_CH7SSEL_Pos	8	/* Channel7����ͨ��ѡ�� */
#define	DMA_CH7CTRL_CH7SSEL_Msk	(0x7U << DMA_CH7CTRL_CH7SSEL_Pos)
#define	DMA_CH7CTRL_CH7SSEL_RFRMFM	(0x0U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 RAM->FLASH���� RAM��ַ�ݼ� FLASH��ַ�ݼ� */
#define	DMA_CH7CTRL_CH7SSEL_RFRMFP	(0x1U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 RAM->FLASH���� RAM��ַ�ݼ� FLASH��ַ���� */
#define	DMA_CH7CTRL_CH7SSEL_RFRPFM	(0x2U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 RAM->FLASH���� RAM��ַ���� FLASH��ַ�ݼ� */
#define	DMA_CH7CTRL_CH7SSEL_RFRPFP	(0x3U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 RAM->FLASH���� RAM��ַ���� FLASH��ַ���� */
#define	DMA_CH7CTRL_CH7SSEL_FRRMFM	(0x4U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 FLASH->RAM���� RAM��ַ�ݼ� FLASH��ַ�ݼ� */
#define	DMA_CH7CTRL_CH7SSEL_FRRMFP	(0x5U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 FLASH->RAM���� RAM��ַ�ݼ� FLASH��ַ���� */
#define	DMA_CH7CTRL_CH7SSEL_FRRPFM	(0x6U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 FLASH->RAM���� RAM��ַ���� FLASH��ַ�ݼ� */
#define	DMA_CH7CTRL_CH7SSEL_FRRPFP	(0x7U << DMA_CH7CTRL_CH7SSEL_Pos)	/* Channel7 FLASH->RAM���� RAM��ַ���� FLASH��ַ���� */

//#define	DMA_CH7CTRL_CH7DIR_Pos	10	/* Channel7���䷽�� */
//#define	DMA_CH7CTRL_CH7DIR_Msk	(0x1U << DMA_CH7CTRL_CH7DIR_Pos)
//#define	DMA_CH7CTRL_CH7DIR_RAM2FLASH	(0x0U << DMA_CH7CTRL_CH7DIR_Pos)	/* RAM->FLASH���� */
//#define	DMA_CH7CTRL_CH7DIR_FLASH2RAM	(0x1U << DMA_CH7CTRL_CH7DIR_Pos)	/* FLASH->RAM���� */

//#define	DMA_CH7CTRL_CH7RI_Pos	9	/* Channel7RAM��ַ�������� */
//#define	DMA_CH7CTRL_CH7RI_Msk	(0x1U << DMA_CH7CTRL_CH7RI_Pos)
//#define	DMA_CH7CTRL_CH7RI_DEC	(0x0U << DMA_CH7CTRL_CH7RI_Pos)	/* RAM��ַ�ݼ� */
//#define	DMA_CH7CTRL_CH7RI_INC	(0x1U << DMA_CH7CTRL_CH7RI_Pos)	/* RAM��ַ���� */

//#define	DMA_CH7CTRL_CH7FI_Pos	8	/* Channel7FLASH��ַ�������� */
//#define	DMA_CH7CTRL_CH7FI_Msk	(0x1U << DMA_CH7CTRL_CH7FI_Pos)
//#define	DMA_CH7CTRL_CH7FI_DEC	(0x0U << DMA_CH7CTRL_CH7FI_Pos)	/* FLASH��ַ�ݼ� */
//#define	DMA_CH7CTRL_CH7FI_INC	(0x1U << DMA_CH7CTRL_CH7FI_Pos)	/* FLASH��ַ���� */



#define	DMA_CH7FLSAD_CH7FLSAD_Pos	0	/* Channel7FLASHָ���ַ */
#define	DMA_CH7FLSAD_CH7FLSAD_Msk	(0xffffU << DMA_CH7FLSAD_CH7FLSAD_Pos)

#define	DMA_CH7RAMAD_CH7RAMAD_Pos	0	/* Channel7RAM��ָ���ַ */
#define	DMA_CH7RAMAD_CH7RAMAD_Msk	(0x1fffU << DMA_CH7RAMAD_CH7RAMAD_Pos)


//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void DMA_Deinit(void);

/* DMAȫ��ʹ�� ��غ��� */
extern void DMA_GLOBALCTRL_DMAEN_Setable(FunState NewState);
extern FunState DMA_GLOBALCTRL_DMAEN_Getable(void);


//Announce_End

/* Channelx���䳤�� ��غ��� */
extern void DMA_CHxCTRL_TSIZE_Set(DMA_CH_Type CHx, uint32_t SetValue);
extern uint32_t DMA_CHxCTRL_TSIZE_Get(DMA_CH_Type CHx);

/* Channelx���ȼ� ��غ��� */
extern void DMA_CHxCTRL_PRI_Set(DMA_CH_Type CHx, uint32_t SetValue);
extern uint32_t DMA_CHxCTRL_PRI_Get(DMA_CH_Type CHx);

/* Channelx RAM��ַ�������� ��غ��� */
extern void DMA_CHxCTRL_INC_Set(DMA_CH_Type CHx, uint32_t SetValue);
extern uint32_t DMA_CHxCTRL_INC_Get(DMA_CH_Type CHx);

/* Channelx����ͨ��ѡ�� ��غ��� */
extern void DMA_CHxCTRL_SSEL_Set(DMA_CH_Type CHx, uint32_t SetValue);
extern uint32_t DMA_CHxCTRL_SSEL_Get(DMA_CH_Type CHx);

/* Channelx��������ж�ʹ�� ��غ��� */
extern void DMA_CHxCTRL_FTIE_Setable(DMA_CH_Type CHx, FunState NewState);
extern FunState DMA_CHxCTRL_FTIE_Getable(DMA_CH_Type CHx);

/* Channelx��̴�������ж�ʹ�� ��غ��� */
extern void DMA_CHxCTRL_HTIE_Setable(DMA_CH_Type CHx, FunState NewState);
extern FunState DMA_CHxCTRL_HTIE_Getable(DMA_CH_Type CHx);

/* Channelxʹ�� ��غ��� */
extern void DMA_CHxCTRL_EN_Setable(DMA_CH_Type CHx, FunState NewState);

/* Channelxָ��Ĵ��� ��غ��� */
extern void DMA_CHxRAMAD_Write(DMA_CH_Type CHx, uint32_t SetValue);
extern uint32_t DMA_CHxRAMAD_Read(DMA_CH_Type CHx);

/* Channel7FLASHָ���ַ ��غ��� */
extern void DMA_CH7FLSAD_Write(uint32_t SetValue);
extern uint32_t DMA_CH7FLSAD_Read(void);

/* Channel7RAM��ָ���ַ ��غ��� */
extern void DMA_CH7RAMAD_Write(uint32_t SetValue);
extern uint32_t DMA_CH7RAMAD_Read(void);

/* Channelx�����̱�־ ��غ��� */
extern void DMA_CHSTATUS_DMACHxHT_Clr(DMA_CH_Type CHx);
extern FlagStatus DMA_CHSTATUS_DMACHxHT_Chk(DMA_CH_Type CHx);

/* Channelx������ɱ�־ ��غ��� */
extern void DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH_Type CHx);
extern FlagStatus DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH_Type CHx);

/* DMA��ʼ������ */
extern void DMA_Init(DMA_InitTypeDef* para);



#ifdef __cplusplus
}
#endif

#endif /* __FM33G0XX_DMA_H */



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/




