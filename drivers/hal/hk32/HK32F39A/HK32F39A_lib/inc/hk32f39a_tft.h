/**
  ******************************************************************************
  * @file    hk32f39a_tft.c
  * @author  laura.c
  * @brief   TFT module driver.
*/  



#ifndef _HK_FSMC_TFT_LCD_H
#define _HK_FSMC_TFT_LCD_H
#include "hk32f39a_hal_def.h"
#include "hk32f39a_hal_dma.h"

#define FSMC_TFT_R_BASE       (FSMC_R_BASE + 0x00000200U)    /*!< FSMC Bank4 registers base address */
#define FSMC_TFT			((FSMC_TFT_TypeDef *)FSMC_TFT_R_BASE)

#define FSMC_TFT_TypeDef             	FSMC_TFT_TypeDef
#define FSMC_TFT_DEVICE              	FSMC_TFT


#define FSMC_LCDCTRLx_HSPOL_Pos                 (31U)                               
#define FSMC_LCDCTRLx_HSPOL_Msk                 (0x1U << FSMC_LCDCTRLx_HSPOL_Pos)     
#define FSMC_LCDCTRLx_HSPOL                     FSMC_LCDCTRLx_HSPOL_Msk       

#define FSMC_LCDCTRLx_VSPOL_Pos                 (30U)                               
#define FSMC_LCDCTRLx_VSPOL_Msk                 (0x1U << FSMC_LCDCTRLx_VSPOL_Pos)    
#define FSMC_LCDCTRLx_VSPOL                     FSMC_LCDCTRLx_VSPOL_Msk       

#define FSMC_LCDCTRLx_DEPOL_Pos                 (29U)                               
#define FSMC_LCDCTRLx_DEPOL_Msk                 (0x1U << FSMC_LCDCTRLx_DEPOL_Pos)    
#define FSMC_LCDCTRLx_DEPOL                     FSMC_LCDCTRLx_DEPOL_Msk       

#define FSMC_LCDCTRLx_DCLKPOL_Pos               (28U)                               
#define FSMC_LCDCTRLx_DCLKPOL_Msk               (0x1U << FSMC_LCDCTRLx_DCLKPOL_Pos)    
#define FSMC_LCDCTRLx_DCLKPOL                   FSMC_LCDCTRLx_DCLKPOL_Msk   

#define FSMC_LCDCTRLx_LCDEN_Pos                 (0U)                               
#define FSMC_LCDCTRLx_LCDEN_Msk                 (0x1U << FSMC_LCDCTRLx_LCDEN_Pos)    
#define FSMC_LCDCTRLx_LCDEN                     FSMC_LCDCTRLx_LCDEN_Msk   

#define FSMC_LCDCFG_AUTOPOR_Pos                 (31U)                               
#define FSMC_LCDCFG_AUTOPOR_Msk                 (0x1U << FSMC_LCDCFG_AUTOPOR_Pos)     
#define FSMC_LCDCFG_AUTOPOR  					FSMC_LCDCFG_AUTOPOR_Msk


#define FSMC_LCD_HORIZONTAL_POLARITY_LOW            0x00000000U
#define FSMC_LCD_HORIZONTAL_POLARITY_HIGH          ((uint32_t)FSMC_LCDCTRLx_HSPOL)

#define FSMC_LCD_VERTICAL_POLARITY_LOW            	0x00000000U
#define FSMC_LCD_VERTICAL_POLARITY_HIGH            ((uint32_t)FSMC_LCDCTRLx_VSPOL)

#define FSMC_LCD_DATA_ENABLE_POLARITY_LOW          	0x00000000U
#define FSMC_LCD_DATA_ENABLE_POLARITY_HIGH         ((uint32_t)FSMC_LCDCTRLx_DEPOL)

#define FSMC_LCD_DCLK_ACTIVE_NEGATIVE		        0x00000000U
#define FSMC_LCD_DCLK_ACTIVE_POSITIVE         		((uint32_t)FSMC_LCDCTRLx_DCLKPOL)

#define FSMC_LCD_DISABLE		          			0x00000000U
#define FSMC_LCD_ENABLE		        				((uint32_t)FSMC_LCDCTRLx_LCDEN)

#define FSMC_LCD_AUTO_POR_DISABLE		          	0x00000000U
#define FSMC_LCD_AUTO_POR_ENABLE		        	((uint32_t)FSMC_LCDCFG_AUTOPOR)





typedef struct
{
  __IO uint32_t LCD_CTRL[4];  
  __IO uint32_t LCD_SSCR[4];
  __IO uint32_t LCD_BPCR[4]; 
  __IO uint32_t LCD_AWCR[4]; 
  __IO uint32_t LCD_TWCR[4];  

  __IO uint32_t LCD_CPSR;  
  __IO uint32_t LCD_CFG; 
} FSMC_TFT_TypeDef; 


typedef struct
{
  	uint32_t NSBank;     
	uint32_t HSPol; 
	uint32_t VSPol;
	uint32_t DEPol;
	uint32_t DCLKPol;
	uint32_t DLCK_setup_time;
	
	uint32_t HSW;
	uint32_t VSH;
	uint32_t AHBP;
	uint32_t AVBP;
	uint32_t AAW;
	uint32_t AAH;
	uint32_t TOTALW;
	uint32_t TOTALH;

	uint32_t AUTOPOR;
	uint32_t POR_data;
	uint32_t LCDEN;	
}FSMC_TFT_InitTypeDef;


typedef struct
{
  FSMC_TFT_TypeDef           *Instance;  /* Register base address                        */   
  FSMC_TFT_InitTypeDef       Init;       /* TFT device control configuration parameters */  
  DMA_HandleTypeDef             *hdma;   /* Pointer DMA handler                          */  
}TFT_HandleTypeDef; 


HAL_StatusTypeDef  FSMC_TFT_Init(FSMC_TFT_TypeDef *Device, FSMC_TFT_InitTypeDef *Init);
void FSMC_TFT_Timing_init(FSMC_NORSRAM_TypeDef *Device, uint32_t NSBank, FSMC_NORSRAM_TimingTypeDef *Timing);
void FSMC_TFT_SHOW_PIC_DMA(DMA_HandleTypeDef *hdma, uint32_t image_addr);
void  FSMC_TFT_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma);


#endif


