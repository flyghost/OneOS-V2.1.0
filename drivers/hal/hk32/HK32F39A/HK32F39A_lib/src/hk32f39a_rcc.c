/**
  ******************************************************************************
  * @file    hk32f39a_rcc.c
  * @author  laura.c  
  * @version V1.0.0
  * @date    2019-09-16
  * @brief   This file provides all the RCC firmware functions.
  ****************************************************************************** 
  */

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_rcc.h"



/* ------------ RCC registers bit address in the alias region ----------- */
#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)

/* --- CR Register ---*/

/* Alias word address of HSION bit */
#define CR_OFFSET                 (RCC_OFFSET + 0x00)
#define HSION_BitNumber           0x00
#define CR_HSION_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4))

/* Alias word address of PLLON bit */
#define PLLON_BitNumber           0x18
#define CR_PLLON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PLLON_BitNumber * 4))

/* Alias word address of CSSON bit */
#define CSSON_BitNumber           0x13
#define CR_CSSON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (CSSON_BitNumber * 4))

/* --- CFGR Register ---*/

/* Alias word address of USBPRE bit */
#define CFGR_OFFSET               (RCC_OFFSET + 0x04)

 #define OTGFSPRE_BitNumber        0x16
 #define CFGR_OTGFSPRE_BB          (PERIPH_BB_BASE + (CFGR_OFFSET * 32) + (OTGFSPRE_BitNumber * 4))

/* --- BDCR Register ---*/

/* Alias word address of RTCEN bit */
#define BDCR_OFFSET               (RCC_OFFSET + 0x20)
#define RTCEN_BitNumber           0x0F
#define BDCR_RTCEN_BB             (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (RTCEN_BitNumber * 4))

/* Alias word address of BDRST bit */
#define BDRST_BitNumber           0x10
#define BDCR_BDRST_BB             (PERIPH_BB_BASE + (BDCR_OFFSET * 32) + (BDRST_BitNumber * 4))

/* --- CSR Register ---*/

/* Alias word address of LSION bit */
#define CSR_OFFSET                (RCC_OFFSET + 0x24)
#define LSION_BitNumber           0x00
#define CSR_LSION_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (LSION_BitNumber * 4))

/* --- CFGR5 Register ---*/

/* Alias word address of MCO3 bit */
#define CFGR5_OFFSET               (RCC_OFFSET + 0xEC)
#define MCO3_BitNumber          	27
#define CFGR5_MCO3_BB              (PERIPH_BB_BASE + (CFGR5_OFFSET * 32) + (MCO3_BitNumber * 4))

/* --- CR2 Register ---*/

#define CR2_OFFSET                 (RCC_OFFSET + 0x34)
#define HSI28ON_BitNumber           18
#define CR2_HSI28ON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4))


#define CR2_OFFSET                 (RCC_OFFSET + 0x34)
#define HSI56ON_BitNumber           16
#define CR2_HSI56ON_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (HSION_BitNumber * 4))


/* ---------------------- RCC registers bit mask ------------------------ */

/* CR register bit mask */
#define CR_HSEBYP_Reset           ((uint32_t)0xFFFBFFFF)
#define CR_HSEBYP_Set             ((uint32_t)0x00040000)
#define CR_HSEON_Reset            ((uint32_t)0xFFFEFFFF)
#define CR_HSEON_Set              ((uint32_t)0x00010000)
#define CR_HSITRIM_Mask           ((uint32_t)0xFFFFFF07)

/* CFGR register bit mask */
 #define CFGR_PLL_Mask            ((uint32_t)0xFFC2FFFF)

#define CFGR_PLLMull_Mask         ((uint32_t)0x003C0000)
#define CFGR_PLLSRC_Mask          ((uint32_t)0x00010000)
#define CFGR_PLLXTPRE_Mask        ((uint32_t)0x00020000)
#define CFGR_SWS_Mask             ((uint32_t)0x0000000C)
#define CFGR_SW_Mask              ((uint32_t)0xFFFFFFFC)
#define CFGR_HPRE_Reset_Mask      ((uint32_t)0xFFFFFF0F)
#define CFGR_HPRE_Set_Mask        ((uint32_t)0x000000F0)
#define CFGR_PPRE1_Reset_Mask     ((uint32_t)0xFFFFF8FF)
#define CFGR_PPRE1_Set_Mask       ((uint32_t)0x00000700)
#define CFGR_PPRE2_Reset_Mask     ((uint32_t)0xFFFFC7FF)
#define CFGR_PPRE2_Set_Mask       ((uint32_t)0x00003800)
#define CFGR_ADCPRE_Reset_Mask    ((uint32_t)0xFFFF3FFF)
#define CFGR_ADCPRE_Set_Mask      ((uint32_t)0x0000C000)

#define RTC_CFGR_PLLHSIPRE_Mask	  ((uint32_t)0x80000000)


/* CSR register bit mask */
#define CSR_RMVF_Set              	((uint32_t)0x01000000)

#define CFGR2_PREDIV1            	((uint32_t)0x0000000F)


/* CFGR3 register bit mask */
#define RCC_CFGR3_USBSW_Mask		((uint32_t)0x00000700)


/* CFGR5 register bit mask */



/* RCC Flag Mask */
#define FLAG_Mask                 ((uint8_t)0x1F)

/* CIR register byte 2 (Bits[15:8]) base address */
#define CIR_BYTE2_ADDRESS         ((uint32_t)0x40021009)

/* CIR register byte 3 (Bits[23:16]) base address */
#define CIR_BYTE3_ADDRESS         ((uint32_t)0x4002100A)

/* CFGR register byte 4 (Bits[31:24]) base address */
#define CFGR_BYTE4_ADDRESS        ((uint32_t)0x40021007)

/* BDCR register base address */
#define BDCR_ADDRESS              (PERIPH_BASE + BDCR_OFFSET)


static __I uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
static __I uint8_t ADCPrescTable[4] = {2, 4, 6, 8};

/**
  * @brief  Resets the RCC clock configuration to the default reset state.
  * @param  None
  * @retval None
  */
void RCC_DeInit(void)
{
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CFGR &= (uint32_t)0xF0FF0000;
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;

  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;

}

/**
  * @brief  Configures the External High Speed oscillator (HSE).
  * @note   HSE can not be stopped if it is used directly or through the PLL as system clock.
  * @param  RCC_HSE: specifies the new state of the HSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_HSE_OFF: HSE oscillator OFF
  *     @arg RCC_HSE_ON: HSE oscillator ON
  *     @arg RCC_HSE_Bypass: HSE oscillator bypassed with external clock
  * @retval None
  */
void RCC_HSEConfig(uint32_t RCC_HSE)
{
  /* Check the parameters */
  assert_param(IS_RCC_HSE(RCC_HSE));
  /* Reset HSEON and HSEBYP bits before configuring the HSE ------------------*/
  /* Reset HSEON bit */
  RCC->CR &= CR_HSEON_Reset;
  /* Reset HSEBYP bit */
  RCC->CR &= CR_HSEBYP_Reset;
  /* Configure HSE (RCC_HSE_OFF is already covered by the code section above) */
  switch(RCC_HSE)
  {
    case RCC_HSE_ON:
      /* Set HSEON bit */
      RCC->CR |= CR_HSEON_Set;
      break;
      
    case RCC_HSE_Bypass:
      /* Set HSEBYP and HSEON bits */
      RCC->CR |= CR_HSEBYP_Set | CR_HSEON_Set;
      break;
      
    default:
      break;
  }
}

/**
  * @brief  Waits for HSE start-up.
  * @param  RDYFlag  
	  RCC_FLAG_HSI8MRDY
	  RCC_FLAG_HSERDY  
	  RCC_FLAG_PLLRDY  
	  RCC_FLAG_LSERDY  
	  RCC_FLAG_LSIRDY  
	  RCC_FLAG_HSI28MRDY
	  RCC_FLAG_HSI56MRDY	  
  * @retval An ErrorStatus enumuration value:
  * - SUCCESS: HSE oscillator is stable and ready to use
  * - ERROR: HSE oscillator not yet ready
  */
ErrorStatus RCC_WaitForStartUp(uint8_t RDYFlag)
{
  __IO uint32_t StartUpCounter = 0;
  ErrorStatus status = ERROR;
  FlagStatus HSEStatus = RESET;
  
  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC_GetFlagStatus(RDYFlag);
    StartUpCounter++;  
  } while((StartUpCounter != HSE_STARTUP_TIMEOUT) && (HSEStatus == RESET));
  
  if (RCC_GetFlagStatus(RDYFlag) != RESET)
  {
    status = SUCCESS;
  }
  else
  {
    status = ERROR;
  }  
  return (status);
}

/**
  * @brief  Adjusts the Internal High Speed oscillator (HSI) calibration value.
  * @param  HSICalibrationValue: specifies the calibration trimming value.
  *   This parameter must be a number between 0 and 0x1F.
  * @retval None
  */
void RCC_AdjustHSICalibrationValue(uint8_t HSICalibrationValue)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_RCC_CALIBRATION_VALUE(HSICalibrationValue));
  tmpreg = RCC->CR;
  /* Clear HSITRIM[4:0] bits */
  tmpreg &= CR_HSITRIM_Mask;
  /* Set the HSITRIM[4:0] bits according to HSICalibrationValue value */
  tmpreg |= (uint32_t)HSICalibrationValue << 3;
  /* Store the new value */
  RCC->CR = tmpreg;
}

/**
  * @brief  Enables or disables the Internal High Speed oscillator (HSI).
  * @note   HSI can not be stopped if it is used directly or through the PLL as system clock.
  * @param  NewState: new state of the HSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_HSI8MCmd(FunctionalState NewState)
{
  	/* Check the parameters */
  	assert_param(IS_FUNCTIONAL_STATE(NewState));
  	*(__IO uint32_t *) CR_HSION_BB = (uint32_t)NewState;
}
/**
  * @brief  Enables or disables the Internal High Speed oscillator (HSI).
  * @note   HSI can not be stopped if it is used directly or through the PLL as system clock.
  * @param  NewState: new state of the HSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_HSI28MCmd(FunctionalState NewState)
{
  /* Check the parameters */
	assert_param(IS_FUNCTIONAL_STATE(NewState));
	*(__IO uint32_t *) CR2_HSI28ON_BB = (uint32_t)NewState;
}
/**
  * @brief  Enables or disables the Internal High Speed oscillator (HSI).
  * @note   HSI can not be stopped if it is used directly or through the PLL as system clock.
  * @param  NewState: new state of the HSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_HSI56MCmd(FunctionalState NewState)
{
  /* Check the parameters */
	assert_param(IS_FUNCTIONAL_STATE(NewState));
   *(__IO uint32_t *) CR2_HSI56ON_BB = (uint32_t)NewState;
}

/**
  * @brief  Configures the PLL clock source and multiplication factor.
  * @note   This function must be used only when the PLL is disabled.
  * @param  RCC_PLLSource: specifies the PLL entry clock source.
  *   this parameter can be one of the following values:
  *     @arg RCC_PLLSource_HSI8M_Div2: HSI oscillator clock divided by 2 selected as PLL clock entry
  *     @arg RCC_PLLSource_HSE: HSE oscillator clock selected as PLL clock entr
  *     @arg RCC_PLLSource_HSI56M: HSI oscillator clock selected as PLL clock entr
  * @param  RCC_PLLPre_DIV: specifies the PLL Predivision factor.  
			RCC_PLLPREDIV_1                    
			RCC_PLLPREDIV_2                 
			RCC_PLLPREDIV_3                 
			RCC_PLLPREDIV_4                  
			RCC_PLLPREDIV_5                  
			RCC_PLLPREDIV_6                  
			RCC_PLLPREDIV_7                   
			RCC_PLLPREDIV_8                  
			RCC_PLLPREDIV_9                 
			RCC_PLLPREDIV_10                 
			RCC_PLLPREDIV_11                  
			RCC_PLLPREDIV_12               
			RCC_PLLPREDIV_13                 
			RCC_PLLPREDIV_14                 
			RCC_PLLPREDIV_15                 
			RCC_PLLPREDIV_16                
  * @param  RCC_PLLMul: specifies the PLL multiplication factor.
			RCC_PLLMul_2 
			RCC_PLLMul_3 
			RCC_PLLMul_4 
			RCC_PLLMul_5 
			RCC_PLLMul_6 
			RCC_PLLMul_7 
			RCC_PLLMul_8 
			RCC_PLLMul_9 
			RCC_PLLMul_10
			RCC_PLLMul_11
			RCC_PLLMul_12
			RCC_PLLMul_13
			RCC_PLLMul_14
			RCC_PLLMul_15
			RCC_PLLMul_16  
  * @retval None
  */
void RCC_PLLConfig(uint32_t RCC_PLLSource, uint32_t RCC_PLLPre_DIV, uint32_t RCC_PLLMul)
{
  	uint32_t tmpreg = 0;
	uint32_t RCC_PLLSource_in_CFGR;

  	/* Check the parameters */
  	assert_param(IS_RCC_PLL_SOURCE(RCC_PLLSource));
  	assert_param(IS_RCC_PLL_PREDIV(RCC_PLLPre_DIV));	
  	assert_param(IS_RCC_PLL_MUL(RCC_PLLMul));

	switch (RCC_PLLSource)
	{
		case RCC_PLLSource_HSI56M:
			//ppss=1
			RCC->CFGR4|=RCC_CFGR4_PPSS_HSI56M;
			//set pll pre-div
			RCC->CFGR2 = RCC_PLLPre_DIV;
			//pllsrc=1
			RCC_PLLSource_in_CFGR=0x10000;
		break;
		
		case RCC_PLLSource_HSE:
			//ppss=0
			RCC->CFGR4&=~RCC_CFGR4_PPSS_HSI56M;
			//set pll pre-div
			RCC->CFGR2 = RCC_PLLPre_DIV;
			//pllsrc=1
			RCC_PLLSource_in_CFGR=0x10000;
		break;		

		case RCC_PLLSource_HSI8M_Div2:		
			RCC_PLLSource_in_CFGR=RCC_PLLSource;
		break;

		default:
			return;
	}

	tmpreg = RCC->CFGR;
	/* Clear PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
	tmpreg &= CFGR_PLL_Mask;
	/* Set the PLL configuration bits */
	tmpreg |= RCC_PLLSource_in_CFGR | RCC_PLLMul;
	/* Store the new value */
	RCC->CFGR = tmpreg;
	
}

/**
  * @brief  Enables or disables the PLL.
  * @note   The PLL can not be disabled if it is used as system clock.
  * @param  NewState: new state of the PLL. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_PLLCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  *(__IO uint32_t *) CR_PLLON_BB = (uint32_t)NewState;
}



/**
  * @brief  Configures the system clock (SYSCLK).
  * @param  RCC_SYSCLKSource: specifies the clock source used as system clock.
  *   This parameter can be one of the following values:
	  RCC_SYSCLKSource_HSI8M 
	  RCC_SYSCLKSource_HSE	  
	  RCC_SYSCLKSource_PLLCLK
	  RCC_SYSCLKSource_LSE	 
	  RCC_SYSCLKSource_LSI	 
	  RCC_SYSCLKSource_HSI56M
	  RCC_SYSCLKSource_HSI28M
	  RCC_SYSCLKSource_EXTCLK
  * @retval None
  */
void RCC_SYSCLKConfig(uint32_t RCC_SYSCLKSource)
{
	uint32_t tmpreg = 0;
	/* Check the parameters */
	assert_param(IS_RCC_SYSCLK_SOURCE(RCC_SYSCLKSource));

	switch (RCC_SYSCLKSource)
	{
		case RCC_SYSCLKSource_HSI8M:
		case RCC_SYSCLKSource_HSE:
		case RCC_SYSCLKSource_PLLCLK:	
			
			tmpreg = RCC->CFGR;
			// Clear SW[1:0] bits 
			tmpreg &= CFGR_SW_Mask;	
			tmpreg |= RCC_SYSCLKSource;
			RCC->CFGR = tmpreg;

			//reset CFGR5.ESSS
			RCC->CFGR5 &=~(RCC_CFGR5_ESSS_SET);
			break;
		case RCC_SYSCLKSource_LSE:
		case RCC_SYSCLKSource_LSI:
		case RCC_SYSCLKSource_HSI56M:
		case RCC_SYSCLKSource_HSI28M:
		case RCC_SYSCLKSource_EXTCLK:
			
			tmpreg = RCC->CFGR5;
			// Clear ESW[2:0] bits 
			tmpreg &= RCC_CFGR5_ESW_Mask;	

			tmpreg |= (RCC_SYSCLKSource&RCC_CFGR5_ESW_Mask);
			RCC->CFGR5 = tmpreg;

			//set CFGR5.ESSS
			RCC->CFGR5 |=RCC_CFGR5_ESSS_SET;
			break;		
	}  
}

/**
  * @brief  Returns the clock source used as system clock.
  * @param  None
  * @retval The clock source used as system clock. The returned value can
  *   be one of the following:
  *     - 0x00: HSI8M used as system clock
  *     - 0x04: HSE used as system clock
  *     - 0x08: PLL used as system clock
  *     - 0x0F: LSE used as system clock
  *     - 0x1F: LSI used as system clock
  *     - 0x2F: HSI56M used as system clock
  *     - 0x3F: HSI28M used as system clock
  *     - 0x4F: EXTCLK used as system clock
  */
uint8_t RCC_GetSYSCLKSource(void)
{
	if ((RCC->CFGR5&RCC_CFGR5_ESSS_Mask)==RCC_CFGR5_ESSS_SET)
		return ((uint8_t)((RCC->CFGR5 & RCC_CFGR5_ESWS_Mask)|0xF));
	else
  		return ((uint8_t)(RCC->CFGR & CFGR_SWS_Mask));
}

/**
  * @brief  Configures the AHB clock (HCLK).
  * @param  RCC_SYSCLK: defines the AHB clock divider. This clock is derived from 
  *   the system clock (SYSCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_SYSCLK_Div1: AHB clock = SYSCLK
  *     @arg RCC_SYSCLK_Div2: AHB clock = SYSCLK/2
  *     @arg RCC_SYSCLK_Div4: AHB clock = SYSCLK/4
  *     @arg RCC_SYSCLK_Div8: AHB clock = SYSCLK/8
  *     @arg RCC_SYSCLK_Div16: AHB clock = SYSCLK/16
  *     @arg RCC_SYSCLK_Div64: AHB clock = SYSCLK/64
  *     @arg RCC_SYSCLK_Div128: AHB clock = SYSCLK/128
  *     @arg RCC_SYSCLK_Div256: AHB clock = SYSCLK/256
  *     @arg RCC_SYSCLK_Div512: AHB clock = SYSCLK/512
  * @retval None
  */
void RCC_HCLKConfig(uint32_t RCC_SYSCLK)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_RCC_HCLK(RCC_SYSCLK));
  tmpreg = RCC->CFGR;
  /* Clear HPRE[3:0] bits */
  tmpreg &= CFGR_HPRE_Reset_Mask;
  /* Set HPRE[3:0] bits according to RCC_SYSCLK value */
  tmpreg |= RCC_SYSCLK;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the Low Speed APB clock (PCLK1).
  * @param  RCC_HCLK: defines the APB1 clock divider. This clock is derived from 
  *   the AHB clock (HCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_HCLK_Div1: APB1 clock = HCLK
  *     @arg RCC_HCLK_Div2: APB1 clock = HCLK/2
  *     @arg RCC_HCLK_Div4: APB1 clock = HCLK/4
  *     @arg RCC_HCLK_Div8: APB1 clock = HCLK/8
  *     @arg RCC_HCLK_Div16: APB1 clock = HCLK/16
  * @retval None
  */
void RCC_PCLK1Config(uint32_t RCC_HCLK)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_RCC_PCLK(RCC_HCLK));
  tmpreg = RCC->CFGR;
  /* Clear PPRE1[2:0] bits */
  tmpreg &= CFGR_PPRE1_Reset_Mask;
  /* Set PPRE1[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the High Speed APB clock (PCLK2).
  * @param  RCC_HCLK: defines the APB2 clock divider. This clock is derived from 
  *   the AHB clock (HCLK).
  *   This parameter can be one of the following values:
  *     @arg RCC_HCLK_Div1: APB2 clock = HCLK
  *     @arg RCC_HCLK_Div2: APB2 clock = HCLK/2
  *     @arg RCC_HCLK_Div4: APB2 clock = HCLK/4
  *     @arg RCC_HCLK_Div8: APB2 clock = HCLK/8
  *     @arg RCC_HCLK_Div16: APB2 clock = HCLK/16
  * @retval None
  */
void RCC_PCLK2Config(uint32_t RCC_HCLK)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_RCC_PCLK(RCC_HCLK));
  tmpreg = RCC->CFGR;
  /* Clear PPRE2[2:0] bits */
  tmpreg &= CFGR_PPRE2_Reset_Mask;
  /* Set PPRE2[2:0] bits according to RCC_HCLK value */
  tmpreg |= RCC_HCLK << 3;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Enables or disables the specified RCC interrupts.
  * @param  RCC_IT: specifies the RCC interrupt sources to be enabled or disabled.
  *  this parameter can be any combination of the following values        
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *       
  * @param  NewState: new state of the specified RCC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_ITConfig(uint8_t RCC_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_IT(RCC_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Perform Byte access to RCC_CIR bits to enable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS |= RCC_IT;
  }
  else
  {
    /* Perform Byte access to RCC_CIR bits to disable the selected interrupts */
    *(__IO uint8_t *) CIR_BYTE2_ADDRESS &= (uint8_t)~RCC_IT;
  }
}

/**
  * @brief  Configures the USB clock (USBCLK).
  * @param  RCC_USBCLKSource: specifies the USB clock source. This clock is 
  *   derived from the PLL output to 48MHz
  *   This parameter can be one of the following values:
	  RCC_USBCLKSource_PLLCLK_1Div5:PLL clock(72MHz) divided by 1.5 selected as USB clock source
	  RCC_USBCLKSource_PLLCLK_1Div :PLL clock(48MHz) divided by 1 selected as USB clock source
	  RCC_USBCLKSource_PLLCLK_2Div:PLL clock(96MHz) divided by 2 selected as USB clock source  
	  RCC_USBCLKSource_PLLCLK_2Div5:PLL clock(120MHz) divided by 2.5 selected as USB clock source
	  RCC_USBCLKSource_PLLCLK_3Div :PLL clock(144MHz) divided by 3 selected as USB clock source
	  RCC_USBCLKSource_PLLCLK_3Div5:PLL clock(168MHz) divided by 3.5 selected as USB clock source
	  RCC_USBCLKSource_PLLCLK_4Div :PLL clock(192MHz) divided by 4 selected as USB clock source
  * @retval None
  */
void RCC_USBCLKConfig(uint32_t RCC_USBCLKSource)
{
	/* Check the parameters */
	assert_param(IS_RCC_USBCLK_SOURCE(RCC_USBCLKSource));
	switch (RCC_USBCLKSource)
	{
	  	case RCC_USBCLKSource_PLLCLK_1Div5:
	  	case RCC_USBCLKSource_PLLCLK_1Div:
			RCC->CFGR3 &= ~RCC_CFGR3_USBSW_Mask;
		 	*(__IO uint32_t *) RCC_CFGR3_USBSW_Mask = RCC_USBCLKSource;
		
			break;
		
	  	case RCC_USBCLKSource_PLLCLK_2Div:   
		case RCC_USBCLKSource_PLLCLK_2Div5:	
		case RCC_USBCLKSource_PLLCLK_3Div:  
		case RCC_USBCLKSource_PLLCLK_3Div5:  
		case RCC_USBCLKSource_PLLCLK_4Div:  
			RCC->CFGR3 &= ~RCC_CFGR3_USBSW_Mask;
			RCC->CFGR3 |= (RCC_CFGR3_USBSW_Mask & RCC_USBCLKSource);

			break;
		default:
			break;
	}

}

/**
  * @brief  Configures the ADC clock (ADCCLK).
  * @param  RCC_PCLK2: defines the ADC clock divider. This clock is derived from 
  *   the APB2 clock (PCLK2).
  *   This parameter can be one of the following values:
  *     @arg RCC_PCLK2_Div2: ADC clock = PCLK2/2
  *     @arg RCC_PCLK2_Div4: ADC clock = PCLK2/4
  *     @arg RCC_PCLK2_Div6: ADC clock = PCLK2/6
  *     @arg RCC_PCLK2_Div8: ADC clock = PCLK2/8
  * @retval None
  */
void RCC_ADCCLKConfig(uint32_t RCC_PCLK2)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_RCC_ADCCLK(RCC_PCLK2));
  tmpreg = RCC->CFGR;
  /* Clear ADCPRE[1:0] bits */
  tmpreg &= CFGR_ADCPRE_Reset_Mask;
  /* Set ADCPRE[1:0] bits according to RCC_PCLK2 value */
  tmpreg |= RCC_PCLK2;
  /* Store the new value */
  RCC->CFGR = tmpreg;
}

/**
  * @brief  Configures the SAI clock (ADCCLK).
  * @param  RCC_CLK: defines the ADC clock divider. 
  *   This parameter can be one of the following values:
  *     @arg RCC_SAIA_EXT_CLK: ext clock
  *     @arg RCC_SAIA_HSE_CLK: HSE clock  
  *     @arg RCC_SAIA_PCLK2_Div2: SAi_A clock = PCLK2/2
  *     @arg RCC_SAIA_PCLK2_Div4: SAi_A clock = PCLK2/4
  *     @arg RCC_SAIA_PCLK2_Div6: SAi_A clock = PCLK2/6
  *     @arg RCC_SAIA_PCLK2_Div8: SAi_A clock = PCLK2/8
  *     @arg RCC_SAIA_PCLK2_Div10: SAi_A clock = PCLK2/10
  *     @arg RCC_SAIA_PCLK2_Div12: SAi_A clock = PCLK2/12
  *     @arg RCC_SAIA_PCLK2_Div14: SAi_A clock = PCLK2/14
  *     @arg RCC_SAIA_PCLK2_Div16: SAi_A clock = PCLK2/16  
  *
  *     @arg RCC_SAIB_EXT_CLK: ext clock
  *     @arg RCC_SAIB_HSE_CLK: HSE clock  
  *     @arg RCC_SAIB_PCLK2_Div2: SAi_B clock = PCLK2/2
  *     @arg RCC_SAIB_PCLK2_Div4: SAi_B clock = PCLK2/4
  *     @arg RCC_SAIB_PCLK2_Div6: SAi_B clock = PCLK2/6
  *     @arg RCC_SAIB_PCLK2_Div8: SAi_B clock = PCLK2/8
  *     @arg RCC_SAIB_PCLK2_Div10: SAi_B clock = PCLK2/10
  *     @arg RCC_SAIB_PCLK2_Div12: SAi_B clock = PCLK2/12
  *     @arg RCC_SAIB_PCLK2_Div14: SAi_B clock = PCLK2/14
  *     @arg RCC_SAIB_PCLK2_Div16: SAi_B clock = PCLK2/16  
  * @retval None
  */
void RCC_SAICLKConfig(uint32_t RCC_CLK)
{
	uint32_t tmpreg = 0;
	
	assert_param(IS_SAI_FLAG(RCC_CLK));

	tmpreg = RCC->CFGR6;
	if((RCC_CLK & 0x80000000) == 0x80000000)
	{
		//sai_B
		tmpreg&=~(RCC_CFGR6_SAI_B_Mask);
		tmpreg|=(RCC_CFGR6_SAI_B_Mask&RCC_CLK);
	}
	else
	{
		//sai_A
		tmpreg&=~(RCC_CFGR6_SAI_A_Mask);
		tmpreg|=(RCC_CFGR6_SAI_A_Mask&RCC_CLK);
	}
	RCC->CFGR6 = tmpreg;
}

/**
  * @brief  Configures the External Low Speed oscillator (LSE).
  * @param  RCC_LSE: specifies the new state of the LSE.
  *   This parameter can be one of the following values:
  *     @arg RCC_LSE_OFF: LSE oscillator OFF
  *     @arg RCC_LSE_ON: LSE oscillator ON
  *     @arg RCC_LSE_Bypass: LSE oscillator bypassed with external clock
  * @retval None
  */
void RCC_LSEConfig(uint8_t RCC_LSE)
{
  /* Check the parameters */
  assert_param(IS_RCC_LSE(RCC_LSE));
  /* Reset LSEON and LSEBYP bits before configuring the LSE ------------------*/
  /* Reset LSEON bit */
  *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_OFF;

  /* Configure LSE (RCC_LSE_OFF is already covered by the code section above) */
  switch(RCC_LSE)
  {
    case RCC_LSE_ON:
      /* Set LSEON bit */
      *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_ON;
      break;
      
    case RCC_LSE_Bypass:
      /* Set LSEBYP and LSEON bits */
      *(__IO uint8_t *) BDCR_ADDRESS = RCC_LSE_Bypass | RCC_LSE_ON;
      break;            
      
    default:
      break;      
  }
}

/**
  * @brief  Enables or disables the Internal Low Speed oscillator (LSI).
  * @note   LSI can not be disabled if the IWDG is running.
  * @param  NewState: new state of the LSI. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_LSICmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR_LSION_BB = (uint32_t)NewState;
}

/**
  * @brief  Configures the RTC clock (RTCCLK).
  * @note   Once the RTC clock is selected it can't be changed unless the Backup domain is reset.
  * @param  RCC_RTCCLKSource: specifies the RTC clock source.
  *   This parameter can be one of the following values:
  *     @arg RCC_RTCCLKSource_LSE: LSE selected as RTC clock
  *     @arg RCC_RTCCLKSource_LSI: LSI selected as RTC clock
  *     @arg RCC_RTCCLKSource_HSE_Div128: HSE clock divided by 128 selected as RTC clock
  * @retval None
  */
void RCC_RTCCLKConfig(uint32_t RCC_RTCCLKSource)
{
  /* Check the parameters */
  assert_param(IS_RCC_RTCCLK_SOURCE(RCC_RTCCLKSource));
  /* Select the RTC clock source */
  RCC->BDCR |= RCC_RTCCLKSource;
}

/**
  * @brief  Enables or disables the RTC clock.
  * @note   This function must be used only after the RTC clock was selected using the RCC_RTCCLKConfig function.
  * @param  NewState: new state of the RTC clock. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_RTCCLKCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) BDCR_RTCEN_BB = (uint32_t)NewState;
}

/**
  * @brief  Returns the frequencies of different on chip clocks.
  * @param  RCC_Clocks: pointer to a RCC_ClocksTypeDef structure which will hold
  *         the clocks frequencies.
  * @note   The result of this function could be not correct when using 
  *         fractional value for HSE crystal.  
  * @retval None
  */
void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks)
{
	uint32_t tmp = 0, pllmull = 0, pllsource = 0, presc = 0;
	uint32_t esss = 0,  pllpre_div = 0, pllppss = 0;

	RCC_Clocks->SYSCLK_Frequency = HSI8M_VALUE;
	/* Get SYSCLK source -------------------------------------------------------*/
	esss = RCC->CFGR5 & RCC_CFGR5_ESSS_Mask;
	if(esss==0x0)
	{
		tmp = RCC->CFGR & CFGR_SWS_Mask;
		switch (tmp)
	 	{
			case 0x00:  /* HSI used as system clock */
			 	RCC_Clocks->SYSCLK_Frequency = HSI8M_VALUE;
			 	break;
			case 0x04:  /* HSE used as system clock */
			 	RCC_Clocks->SYSCLK_Frequency = HSE_VALUE;
			 	break;
			case 0x08:  /* PLL used as system clock */
				
				/* Get PLL clock source and multiplication factor ----------------------*/
				pllmull = RCC->CFGR & CFGR_PLLMull_Mask;
				pllsource = RCC->CFGR & CFGR_PLLSRC_Mask;

				pllmull = ( pllmull >> 18) + 2;			 
				if (pllsource == 0x00)
				{
					/* HSI oscillator clock as PLL clock entry */
					if((RCC->CFGR & RTC_CFGR_PLLHSIPRE_Mask) == 0x0)
						RCC_Clocks->SYSCLK_Frequency = (HSI8M_VALUE >> 1) * pllmull;
					else
						RCC_Clocks->SYSCLK_Frequency = HSI8M_VALUE * pllmull;
				}
			 	else
			 	{
			   		pllpre_div=RCC->CFGR2 & RCC_CFGR2_PREDIV_Mask;
					pllpre_div+=1;
					pllppss=RCC->CFGR4 & RCC_CFGR4_PPSS_Mask;
					if(pllppss==0)
						RCC_Clocks->SYSCLK_Frequency = (HSE_VALUE * pllmull)/pllpre_div;
					else
						RCC_Clocks->SYSCLK_Frequency = (HSI56M_VALUE * pllmull)/pllpre_div;
				 }

			 	break;

			default:
			 	RCC_Clocks->SYSCLK_Frequency = HSI8M_VALUE;
			 	break;
	 	}
	}
	else
	{
		tmp = RCC->CFGR5 & RCC_CFGR5_ESWS_Mask;
		switch (tmp)
		{
			case 0x00:
				//LSE used as system clock
				RCC_Clocks->SYSCLK_Frequency = LSE_VALUE;
				break;
			case 0x10:
				//LSI used as system clock
				RCC_Clocks->SYSCLK_Frequency = LSI_VALUE;
				break;
			case 0x20:
				//HSI56M used as system clock
				RCC_Clocks->SYSCLK_Frequency = HSI56M_VALUE;
				break;
			case 0x30:
				//HSI28M used as system clock
				RCC_Clocks->SYSCLK_Frequency = HSI28M_VALUE;
				break;
			case 0x40:
				//EXTIO used as system clock
				RCC_Clocks->SYSCLK_Frequency = EXTCLK_VALUE;			
				break;
			default:

				break;
		}

	}

  	/* Compute HCLK, PCLK1, PCLK2 and ADCCLK clocks frequencies ----------------*/
  	/* Get HCLK prescaler */
  	tmp = RCC->CFGR & CFGR_HPRE_Set_Mask;
  	tmp = tmp >> 4;
	presc = APBAHBPrescTable[tmp];
	/* HCLK clock frequency */
	RCC_Clocks->HCLK_Frequency = RCC_Clocks->SYSCLK_Frequency >> presc;
	
	/* Get PCLK1 prescaler */
	tmp = RCC->CFGR & CFGR_PPRE1_Set_Mask;
	tmp = tmp >> 8;
	presc = APBAHBPrescTable[tmp];
	/* PCLK1 clock frequency */
	RCC_Clocks->PCLK1_Frequency = RCC_Clocks->HCLK_Frequency >> presc;
	
	/* Get PCLK2 prescaler */
	tmp = RCC->CFGR & CFGR_PPRE2_Set_Mask;
	tmp = tmp >> 11;
	presc = APBAHBPrescTable[tmp];
	/* PCLK2 clock frequency */
	RCC_Clocks->PCLK2_Frequency = RCC_Clocks->HCLK_Frequency >> presc;
	
	/* Get ADCCLK prescaler */
	tmp = RCC->CFGR & CFGR_ADCPRE_Set_Mask;
	tmp = tmp >> 14;
	presc = ADCPrescTable[tmp];
	/* ADCCLK clock frequency */
	RCC_Clocks->ADCCLK_Frequency = RCC_Clocks->PCLK2_Frequency / presc;
}


uint32_t RCC_GetSaiCLKFreq(SAI_Block_TypeDef *sai_mode)
{
	uint32_t tmp = 0, presc = 0, tmpreg=0;
	uint32_t HCLK_Frq=0, PCLK2_Frq=0, sai_Frq=0;
	tmpreg = RCC->CFGR6;

	//check sai sorce
	if(sai_mode==SAI1_Block_A)
	{
		if((tmpreg & RCC_CFGR6_SAIA_EXT_CLK)==RCC_CFGR6_SAIA_EXT_CLK)
			sai_Frq= SAI_EXT_CLK;
		else if((tmpreg & RCC_CFGR6_SAIA_HSE_CLK)==RCC_CFGR6_SAIA_HSE_CLK)
			sai_Frq= HSE_VALUE;
		else
		{
			/* Get HCLK prescaler */
			tmp = RCC->CFGR & CFGR_HPRE_Set_Mask;
			tmp = tmp >> 4;
			presc = APBAHBPrescTable[tmp];
			/* HCLK clock frequency */
			HCLK_Frq=SystemCoreClock >> presc;

			/* Get PCLK2 prescaler */
			tmp = RCC->CFGR & CFGR_PPRE2_Set_Mask;
			tmp = tmp >> 11;
			presc = APBAHBPrescTable[tmp];
			/* PCLK2 clock frequency */
			PCLK2_Frq = HCLK_Frq >> presc;


			/* Get SAI prescaler */
			tmp = RCC->CFGR6 & RCC_CFGR6_SAIA_PCLK2_Mask;
			presc = (tmp+1)*2;	
			/* sai clock frequency */
			sai_Frq = PCLK2_Frq/presc;
		}

	}
	else if(sai_mode==SAI1_Block_B)
	{
		if((tmpreg & RCC_CFGR6_SAIB_EXT_CLK)==RCC_CFGR6_SAIA_EXT_CLK)
			sai_Frq= SAI_EXT_CLK;		
		else if((tmpreg & RCC_CFGR6_SAIB_HSE_CLK)==RCC_CFGR6_SAIA_HSE_CLK)
			sai_Frq=  HSE_VALUE;
		else
		{
			/* Get HCLK prescaler */
			tmp = RCC->CFGR & CFGR_HPRE_Set_Mask;
			tmp = tmp >> 4;
			presc = APBAHBPrescTable[tmp];
			/* HCLK clock frequency */
			HCLK_Frq=SystemCoreClock >> presc;
			
			/* Get PCLK2 prescaler */
			tmp = RCC->CFGR & CFGR_PPRE2_Set_Mask;
			tmp = tmp >> 11;
			presc = APBAHBPrescTable[tmp];
			/* PCLK2 clock frequency */
			PCLK2_Frq = HCLK_Frq >> presc;
			
			
			/* Get SAI prescaler */
			tmp = RCC->CFGR6 & RCC_CFGR6_SAIB_PCLK2_Mask;
			tmp=tmp>>5;
			presc = (tmp+1)*2;	
			/* sai clock frequency */
			sai_Frq = PCLK2_Frq/presc;
		}

	}
	return sai_Frq;
}
/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @param  RCC_AHBPeriph: specifies the AHB peripheral to gates its clock.
  *     @arg RCC_AHBPeriph_DMA1
  *     @arg RCC_AHBPeriph_DMA2
  *     @arg RCC_AHBPeriph_SRAM
  *     @arg RCC_AHBPeriph_FLITF
  *     @arg RCC_AHBPeriph_CRC
  *     @arg RCC_AHBPeriph_FSMC
  *     @arg RCC_AHBPeriph_SDIO
  *   
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_AHBPeriphClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_AHB_PERIPH(RCC_AHBPeriph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    RCC->AHBENR |= RCC_AHBPeriph;
  }
  else
  {
    RCC->AHBENR &= ~RCC_AHBPeriph;
  }
}

/**
  * @brief  Enables or disables the AHB peripheral clock.
  * @param  RCC_AHBPeriph: specifies the AHB peripheral to gates its clock.
		RCC_AHBPeriph2_COALU
		RCC_AHBPeriph2_AES  
		RCC_AHBPeriph2_HASH 
		RCC_AHBPeriph2_TRNG 
		RCC_AHBPeriph2_DCMI 
		RCC_AHBPeriph2_QSPI    
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_AHBPeriph2ClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_AHB_PERIPH2(RCC_AHBPeriph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    RCC->AHBENR2 |= RCC_AHBPeriph;
  }
  else
  {
    RCC->AHBENR2 &= ~RCC_AHBPeriph;
  }
}


/**
  * @brief  Enables or disables the High Speed APB (APB2) peripheral clock.
  * @param  RCC_APB2Periph: specifies the APB2 peripheral to gates its clock.
  *   This parameter can be any combination of the following values:
  *     @arg RCC_APB2Periph_AFIO, RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOB,
  *          RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD, RCC_APB2Periph_GPIOE,
  *          RCC_APB2Periph_GPIOF, RCC_APB2Periph_GPIOG, RCC_APB2Periph_ADC1,
  *          RCC_APB2Periph_ADC2, RCC_APB2Periph_TIM1, RCC_APB2Periph_SPI1,
  *          RCC_APB2Periph_TIM8, RCC_APB2Periph_USART1, RCC_APB2Periph_ADC3,
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB2_PERIPH(RCC_APB2Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB2ENR |= RCC_APB2Periph;
  }
  else
  {
    RCC->APB2ENR &= ~RCC_APB2Periph;
  }
}
/**
  * @brief  Enables or disables the High Speed APB (APB2) peripheral2 clock.
  * @param  RCC_APB2Periph: specifies the APB2 peripheral2 to gates its clock.
  *   This parameter can be any combination of the following values:
	  RCC_APB2Periph2_USART6
	  RCC_APB2Periph2_VC	
	  RCC_APB2Periph2_SAIA	
	  RCC_APB2Periph2_SAIB	
	  RCC_APB2Periph2_PDMA	
	  RCC_APB2Periph2_PDMB	
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB2Periph2ClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB2_PERIPH2(RCC_APB2Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB2ENR2 |= RCC_APB2Periph;
  }
  else
  {
    RCC->APB2ENR2 &= ~RCC_APB2Periph;
  }
}

/**
  * @brief  Enables or disables the Low Speed APB (APB1) peripheral clock.
  * @param  RCC_APB1Periph: specifies the APB1 peripheral to gates its clock.
  *   This parameter can be any combination of the following values:
	  RCC_APB1Periph_TIM2  
	  RCC_APB1Periph_TIM3  
	  RCC_APB1Periph_TIM4  
	  RCC_APB1Periph_TIM5  
	  RCC_APB1Periph_TIM6  
	  RCC_APB1Periph_TIM7  
	  RCC_APB1Periph_WWDG  
	  RCC_APB1Periph_SPI2  
	  RCC_APB1Periph_SPI3  
	  RCC_APB1Periph_USART2
	  RCC_APB1Periph_USART3
	  RCC_APB1Periph_UART4 
	  RCC_APB1Periph_UART5 
	  RCC_APB1Periph_I2C1  
	  RCC_APB1Periph_I2C2  
	  RCC_APB1Periph_USB   
	  RCC_APB1Periph_CAN1  
	  RCC_APB1Periph_CAN2  
	  RCC_APB1Periph_BKP   
	  RCC_APB1Periph_PWR   
	  RCC_APB1Periph_DAC 
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB1_PERIPH(RCC_APB1Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB1ENR |= RCC_APB1Periph;
  }
  else
  {
    RCC->APB1ENR &= ~RCC_APB1Periph;
  }
}



/**
  * @brief  Forces or releases High Speed APB (APB2) peripheral reset.
  * @param  RCC_APB2Periph: specifies the APB2 peripheral to reset.
  *   This parameter can be any combination of the following values:
	  RCC_APB2Periph_AFIO  
	  RCC_APB2Periph_GPIOA 
	  RCC_APB2Periph_GPIOB 
	  RCC_APB2Periph_GPIOC 
	  RCC_APB2Periph_GPIOD 
	  RCC_APB2Periph_GPIOE 
	  RCC_APB2Periph_GPIOF 
	  RCC_APB2Periph_GPIOG 
	  RCC_APB2Periph_ADC1  
	  RCC_APB2Periph_ADC2  
	  RCC_APB2Periph_TIM1  
	  RCC_APB2Periph_SPI1  
	  RCC_APB2Periph_TIM8  
	  RCC_APB2Periph_USART1
	  RCC_APB2Periph_ADC3  	
  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB2PeriphResetCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB2_PERIPH(RCC_APB2Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB2RSTR |= RCC_APB2Periph;
  }
  else
  {
    RCC->APB2RSTR &= ~RCC_APB2Periph;
  }
}

/**
  * @brief  Forces or releases High Speed APB (APB2) peripheral2 reset.
  * @param  RCC_APB2Periph: specifies the APB2 peripheral2 to reset.
  *   This parameter can be any combination of the following values:
	  RCC_APB2Periph2_USART6
	  RCC_APB2Periph2_SAIA	
	  RCC_APB2Periph2_SAIB	
	  RCC_APB2Periph2_PDMA	
	  RCC_APB2Periph2_PDMB	
  * @param  NewState: new state of the specified peripheral reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB2Periph2ResetCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB2_PERIPH2(RCC_APB2Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB2RSTR2 |= RCC_APB2Periph;
  }
  else
  {
    RCC->APB2RSTR2 &= ~RCC_APB2Periph;
  }
}



/**
  * @brief  Forces or releases Low Speed APB (APB1) peripheral reset.
  * @param  RCC_APB1Periph: specifies the APB1 peripheral to reset.
  *   This parameter can be any combination of the following values:
      RCC_APB1Periph_TIM2  
	  RCC_APB1Periph_TIM3  
	  RCC_APB1Periph_TIM4  
	  RCC_APB1Periph_TIM5  
	  RCC_APB1Periph_TIM6  
	  RCC_APB1Periph_TIM7  
	  RCC_APB1Periph_WWDG  
	  RCC_APB1Periph_SPI2  
	  RCC_APB1Periph_SPI3  
	  RCC_APB1Periph_USART2
	  RCC_APB1Periph_USART3
	  RCC_APB1Periph_UART4 
	  RCC_APB1Periph_UART5 
	  RCC_APB1Periph_I2C1  
	  RCC_APB1Periph_I2C2  
	  RCC_APB1Periph_USB   
	  RCC_APB1Periph_CAN1  
	  RCC_APB1Periph_CAN2  
	  RCC_APB1Periph_BKP   
	  RCC_APB1Periph_PWR   
	  RCC_APB1Periph_DAC 
  * @param  NewState: new state of the specified peripheral clock.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_APB1PeriphResetCmd(uint32_t RCC_APB1Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB1_PERIPH(RCC_APB1Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB1RSTR |= RCC_APB1Periph;
  }
  else
  {
    RCC->APB1RSTR &= ~RCC_APB1Periph;
  }
}

/**
  * @brief  Forces or releases the Backup domain reset.
  * @param  NewState: new state of the Backup domain reset.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_BackupResetCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) BDCR_BDRST_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables the Clock Security System.
  * @param  NewState: new state of the Clock Security System..
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RCC_ClockSecuritySystemCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CR_CSSON_BB = (uint32_t)NewState;
}

/**
  * @brief  Selects the clock source to output on MCO pin.
  * @param  RCC_MCO: specifies the clock source to output.
  *   
  *   RCC_MCO can be one of the following values:       
	  RCC_MCO_NoClock	 
	  RCC_MCO_SYSCLK	 
	  RCC_MCO_HSI8M 	 
	  RCC_MCO_HSE		 
	  RCC_MCO_PLLCLK_Div2
	  RCC_MCO_LSI			 
	  RCC_MCO_LSE		   
	  RCC_MCO_HSI28M	
  *   RCC_MCOPRE can be one of the following values:       
	  RCC_MCO_Div1	
	  RCC_MCO_Div2	
	  RCC_MCO_Div4	
	  RCC_MCO_Div8	
	  RCC_MCO_Div16 
	  RCC_MCO_Div32 
	  RCC_MCO_Div64 
	  RCC_MCO_Div128	 	  
  * @retval None
  */
void RCC_MCOConfig(uint8_t RCC_MCO, uint8_t RCC_MCOPRE)
{
  /* Check the parameters */
  assert_param(IS_RCC_MCO(RCC_MCO));

  /* Perform Byte access to MCO bits to select the MCO source */
  *(__IO uint8_t *) CFGR_BYTE4_ADDRESS = RCC_MCO&0x7;
  *(__IO uint32_t *) CFGR5_MCO3_BB = ((RCC_MCO>>3)&0x1);

  RCC->CFGR5 &=~(RCC_MCOPRE_MASK<<28);
  RCC->CFGR5 |=(RCC_MCOPRE<<28);
}

/**
  * @brief  Checks whether the specified RCC flag is set or not.
  * @param  RCC_FLAG: specifies the ready flag to check.
	RCC_FLAG_HSI8MRDY 
	RCC_FLAG_HSERDY	
	RCC_FLAG_PLLRDY	
	RCC_FLAG_LSERDY	
	RCC_FLAG_LSIRDY	
	RCC_FLAG_PINRST	
	RCC_FLAG_PORRST	
	RCC_FLAG_SFTRST	
	RCC_FLAG_IWDGRST	
	RCC_FLAG_WWDGRST	
	RCC_FLAG_LPWRRST	
	RCC_FLAG_HSI28MRDY
	RCC_FLAG_HSI56MRDY
  *   For @b HK32_Connectivity_line_devices, this parameter can be one of the
  *   following values:
  *   
  * @retval The new state of RCC_FLAG (SET or RESET).
  */
FlagStatus RCC_GetFlagStatus(uint8_t RCC_FLAG)
{
  uint32_t tmp = 0;
  uint32_t statusreg = 0;
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_RCC_FLAG(RCC_FLAG));

  /* Get the RCC register index */
  tmp = RCC_FLAG >> 5;
  if (tmp == 1)               /* The flag to check is in CR register */
  {
    statusreg = RCC->CR;
  }
  else if (tmp == 2)          /* The flag to check is in BDCR register */
  {
    statusreg = RCC->BDCR;
  }
  else if (tmp == 3)         /* The flag to check is in CSR register */
  {
     statusreg = RCC->CSR;
  }
  else                       /* The flag to check is in CS2 register */
  {
     statusreg = RCC->CR2;
  }

  /* Get the flag position */
  tmp = RCC_FLAG & FLAG_Mask;
  if ((statusreg & ((uint32_t)1 << tmp)) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  /* Return the flag status */
  return bitstatus;
}

/**
  * @brief  Clears the RCC reset flags.
  * @note   The reset flags are: RCC_FLAG_PINRST, RCC_FLAG_PORRST, RCC_FLAG_SFTRST,
  *   RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST, RCC_FLAG_LPWRRST
  * @param  None
  * @retval None
  */
void RCC_ClearFlag(void)
{
  /* Set RMVF bit to clear the reset flags */
  RCC->CSR |= CSR_RMVF_Set;
}

/**
  * @brief  Checks whether the specified RCC interrupt has occurred or not.
  * @param  RCC_IT: specifies the RCC interrupt source to check.
  *   
  *   For @b HK32_Connectivity_line_devices, this parameter can be one of the
  *   following values:
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  *   
  * @retval The new state of RCC_IT (SET or RESET).
  */
ITStatus RCC_GetITStatus(uint8_t RCC_IT)
{
  ITStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_RCC_GET_IT(RCC_IT));

  /* Check the status of the specified RCC interrupt */
  if ((RCC->CIR & RCC_IT) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  /* Return the RCC_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the RCC's interrupt pending bits.
  * @param  RCC_IT: specifies the interrupt pending bit to clear.
  *   For this parameter can be any combination of the
  *   following values:        
  *     @arg RCC_IT_LSIRDY: LSI ready interrupt
  *     @arg RCC_IT_LSERDY: LSE ready interrupt
  *     @arg RCC_IT_HSIRDY: HSI ready interrupt
  *     @arg RCC_IT_HSERDY: HSE ready interrupt
  *     @arg RCC_IT_PLLRDY: PLL ready interrupt
  *     @arg RCC_IT_CSS: Clock Security System interrupt
  * @retval None
  */
void RCC_ClearITPendingBit(uint8_t RCC_IT)
{
  /* Check the parameters */
  assert_param(IS_RCC_CLEAR_IT(RCC_IT));

  /* Perform Byte access to RCC_CIR[23:16] bits to clear the selected interrupt
     pending bits */
  *(__IO uint8_t *) CIR_BYTE3_ADDRESS = RCC_IT;
}


/******************* (C) COPYRIGHT   HKMicroChip *****END OF FILE****/

