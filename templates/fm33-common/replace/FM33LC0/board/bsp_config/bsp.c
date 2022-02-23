/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        bps.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <board.h>

void RCC_PLL_ConfigDomain_SYS(uint32_t Source, uint32_t PLL_R, uint32_t PLL_DB, uint32_t PLL_O)
{
  MODIFY_REG(RCC->PLLCR, RCC_PLLCR_DB_Msk | RCC_PLLCR_REFPRSC_Msk | RCC_PLLCR_OSEL_Msk | RCC_PLLCR_INSEL_Msk,
             (PLL_DB << RCC_PLLCR_DB_Pos) | PLL_R | PLL_O | Source);
}

void RCHFInit(uint32_t clock)
{
    switch (clock)
    {
        case FL_RCC_RCHF_FREQUENCY_8MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;
        
        case FL_RCC_RCHF_FREQUENCY_16MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF16M_TRIM);
            break;
        
        case FL_RCC_RCHF_FREQUENCY_24MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF24M_TRIM);
            break;
        
        default:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;
    }
    
    FL_RCC_RCHF_SetFrequency(clock);
}

void SelRCHFToPLL(uint32_t rchf, uint32_t clock)
{  
    uint32_t div = FL_RCC_PLL_PSC_DIV8;
    if(clock > 64) return;
    
    RCHFInit(rchf);
    switch (rchf)
    {
        case FL_RCC_RCHF_FREQUENCY_16MHZ:
            div = FL_RCC_PLL_PSC_DIV16;
            break;
        
        case FL_RCC_RCHF_FREQUENCY_24MHZ:
            div = FL_RCC_PLL_PSC_DIV24;
            break;
        
        default:
            break;
    }

    if(clock <= 24)
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_0CYCLE);
    }
    else if((clock > 24) && (clock <= 48))
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_1CYCLE);
    }
    else
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_2CYCLE);  
    }

    RCC_PLL_ConfigDomain_SYS(FL_RCC_PLL_CLK_SOURCE_RCHF,div,clock,FL_RCC_PLL_OUTPUT_X1);
    FL_RCC_PLL_Enable();
    while(FL_RCC_IsActiveFlag_PLLReady() != FL_SET);
    
    FL_RCC_SetAHBPrescaler(FL_RCC_AHBCLK_PSC_DIV1);
    FL_RCC_SetAPB1Prescaler(FL_RCC_APB1CLK_PSC_DIV1);
    FL_RCC_SetAPB2Prescaler(FL_RCC_APB2CLK_PSC_DIV1);
    FL_RCC_SetSystemClockSource(FL_RCC_SYSTEM_CLK_SOURCE_PLL);
}


void XTHFInit(void)
{
    FL_GPIO_InitTypeDef gpioInitStruction;
    
    // GPIO Init
    gpioInitStruction.mode = FL_GPIO_MODE_ANALOG;
    gpioInitStruction.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpioInitStruction.pull = FL_DISABLE;
    
    gpioInitStruction.pin = FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
    FL_GPIO_Init(GPIOC, &gpioInitStruction);
    
    FL_RCC_XTHF_WriteDriverStrength(7);
    FL_RCC_XTHF_Enable();
}

void SelXTHFToPLL(uint32_t clock)
{
    if(clock > 64) return;
    
    XTHFInit();
    
    if(clock <= 24)
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_0CYCLE);
    }
    else if((clock > 24) && (clock <= 48))
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_1CYCLE);
    }
    else
    {
      FL_FLASH_SetReadWait(FLASH,FL_FLASH_READ_WAIT_2CYCLE);  
    }

    RCC_PLL_ConfigDomain_SYS(FL_RCC_PLL_CLK_SOURCE_XTHF,FL_RCC_PLL_PSC_DIV8,clock,FL_RCC_PLL_OUTPUT_X1);
    FL_RCC_PLL_Enable();
    while(FL_RCC_IsActiveFlag_PLLReady() != FL_SET);
    
    FL_RCC_SetAHBPrescaler(FL_RCC_AHBCLK_PSC_DIV1);
    FL_RCC_SetAPB1Prescaler(FL_RCC_APB1CLK_PSC_DIV1);
    FL_RCC_SetAPB2Prescaler(FL_RCC_APB2CLK_PSC_DIV1);
    
    FL_RCC_SetSystemClockSource(FL_RCC_SYSTEM_CLK_SOURCE_PLL);
}


