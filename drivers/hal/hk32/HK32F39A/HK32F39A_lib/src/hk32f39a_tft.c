/**
  * @file    hk32f39a_tft.c
  * @author  laura.c
  * @brief   TFT module driver.
*/  

#include "hk32f39a.h"

HAL_StatusTypeDef  FSMC_TFT_Init(FSMC_TFT_TypeDef *Device, FSMC_TFT_InitTypeDef *Init)
{
  /* Check the parameters */


  /* Set TFT device control parameters */
  MODIFY_REG(Device->LCD_CTRL[Init->NSBank], 0xffffffff, (uint32_t)(Init->LCDEN
				| Init->HSPol
				| Init->VSPol
				| Init->DEPol
				| Init->DCLKPol
				| (Init->DLCK_setup_time<<20)
				| Init->LCDEN)
			   );


  MODIFY_REG(Device->LCD_SSCR[Init->NSBank], 0xffffffff, 
				 (uint32_t)((Init->HSW<<16) | ((Init->VSH)&0x7ff)));
 
//	*(uint32_t *)0xa0000218=0x30000;
	
  MODIFY_REG(Device->LCD_BPCR[Init->NSBank], 0xffffffff, 
  				(uint32_t)((Init->AHBP<<16)	| ((Init->AVBP)&0x7ff)));

  MODIFY_REG(Device->LCD_AWCR[Init->NSBank], 0xffffffff, 
  				(uint32_t)((Init->AAW<<16)	| ((Init->AAH)&0x7ff)));

  MODIFY_REG(Device->LCD_TWCR[Init->NSBank], 0xffffffff, 
  				(uint32_t)((Init->TOTALW<<16)	| ((Init->TOTALH)&0x7ff)));
  
  Device->LCD_CPSR=0x0;
  
  MODIFY_REG(Device->LCD_CFG, 0xffffffff, 
  				(uint32_t)(Init->AUTOPOR| (Init->POR_data)));


  return HAL_OK;
}


void FSMC_TFT_Timing_init(FSMC_NORSRAM_TypeDef *Device, uint32_t NSBank, FSMC_NORSRAM_TimingTypeDef *Timing)
{
	MODIFY_REG(Device->BTCR[NSBank*2], BCR_CLEAR_MASK, (uint32_t)(FSMC_NORSRAM_FLASH_ACCESS_DISABLE
				   | FSMC_DATA_ADDRESS_MUX_DISABLE
				   | FSMC_MEMORY_TYPE_SRAM
				   | FSMC_NORSRAM_MEM_BUS_WIDTH_16
				   | FSMC_BURST_ACCESS_MODE_DISABLE
				   | FSMC_WAIT_SIGNAL_POLARITY_LOW
				   | FSMC_WRAP_MODE_DISABLE
				   | FSMC_WAIT_TIMING_BEFORE_WS
				   | FSMC_WRITE_OPERATION_ENABLE
				   | FSMC_WAIT_SIGNAL_DISABLE
				   | FSMC_EXTENDED_MODE_DISABLE
				   | FSMC_ASYNCHRONOUS_WAIT_DISABLE
				   | FSMC_WRITE_BURST_DISABLE)
				  );

	MODIFY_REG(Device->BTCR[NSBank*2 + 1U], BTR_CLEAR_MASK, 	(uint32_t)(Timing->AddressSetupTime 										 | \
						   ((Timing->AddressHoldTime)		 << FSMC_BTRx_ADDHLD_Pos)		 | \
						   ((Timing->DataSetupTime) 		 << FSMC_BTRx_DATAST_Pos)		 | \
						   ((Timing->BusTurnAroundDuration)  << FSMC_BTRx_BUSTURN_Pos)		 | \
						   (((Timing->CLKDivision) - 1U)	 << FSMC_BTRx_CLKDIV_Pos)		 | \
						   (((Timing->DataLatency) - 2U)	 << FSMC_BTRx_DATLAT_Pos)		 | \
						   (Timing->AccessMode)));

	__FSMC_NORSRAM_ENABLE(Device, (NSBank*2)); 

}



