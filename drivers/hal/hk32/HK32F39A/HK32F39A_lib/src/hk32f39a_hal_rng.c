/**
  ******************************************************************************
  * @file    hk32f39a_hal_rng.c
  * @author  Rakan
  * @date    2019/10/30
  * @brief   RNG module driver.
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_conf.h"


/* Private types -------------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup RNG_Private_Constants
  * @{
  */
#define RNG_TIMEOUT_VALUE     5U

/**
  * @brief  Initializes the RNG peripheral and creates the associated handle.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng)
{

  /* Check the RNG handle allocation */
  if(hrng == NULL)
  {
    return HAL_ERROR;
  }

  if(hrng->State == HAL_RNG_STATE_RESET)
  {  
    /* Allocate lock resource and initialize it */
    hrng->Lock = HAL_UNLOCKED;
    /* Init the low level hardware */
    HAL_RNG_MspInit(hrng);
  }
  
  /* Change RNG peripheral state */
  hrng->State = HAL_RNG_STATE_BUSY;

  /* Enable the RNG Peripheral */
  __HAL_RNG_ENABLE(hrng);

  /* Initialize the RNG state */
  hrng->State = HAL_RNG_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitializes the RNG peripheral. 
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RNG_DeInit(RNG_HandleTypeDef *hrng)
{ 
  /* Check the RNG handle allocation */
  if(hrng == NULL)
  {
    return HAL_ERROR;
  }
  /* Disable the RNG Peripheral */
  CLEAR_BIT(hrng->Instance->CR, RNG_IT_IE | RNG_CR_RNGEN);
  
  /* Clear RNG interrupt status flags */
  CLEAR_BIT(hrng->Instance->SR, RNG_SR_SEIS);
  
  /* DeInit the low level hardware */
  HAL_RNG_MspDeInit(hrng);
  
  /* Update the RNG state */
  hrng->State = HAL_RNG_STATE_RESET; 

  /* Release Lock */
  __HAL_UNLOCK(hrng);
  
  /* Return the function status */
  return HAL_OK;
}

/**
  * @brief  Initializes the RNG MSP.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval None
  */
__weak void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrng);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_RNG_MspInit must be implemented in the user file.
   */
}

/**
  * @brief  DeInitializes the RNG MSP.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval None
  */
__weak void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrng);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_RNG_MspDeInit must be implemented in the user file.
   */
}

/*
@verbatim   
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================  
    [..]  This section provides functions allowing to:
      (+) Get the 32 bit Random number
      (+) Get the 32 bit Random number with interrupt enabled
      (+) Handle RNG interrupt request 

@endverbatim
  * @{
  */

/**
  * @brief  Generates a 32-bit random number.
  * @note   Each time the random number data is read the RNG_FLAG_DRDY flag 
  *         is automatically cleared.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @param  random32bit pointer to generated random number variable if successful.
  * @retval HAL status
  */

HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *hrng, uint32_t *random32bit)
{
  uint32_t tickstart = 0U;    
  HAL_StatusTypeDef status = HAL_OK;

  /* Process Locked */
  __HAL_LOCK(hrng); 
  
  /* Check RNG peripheral state */
  if(hrng->State == HAL_RNG_STATE_READY)
  {
    /* Change RNG peripheral state */  
    hrng->State = HAL_RNG_STATE_BUSY;  

    /* Get tick */
    tickstart = HAL_GetTick();
  
    /* Check if data register contains valid random data */
    while(__HAL_RNG_GET_FLAG(hrng, RNG_FLAG_DRDY) == RESET)
    {
      if((HAL_GetTick() - tickstart ) > RNG_TIMEOUT_VALUE)
      {    
        hrng->State = HAL_RNG_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hrng);
      
        return HAL_TIMEOUT;
      } 
    }
  
    /* Get a 32bit Random number */
    hrng->RandomNumber = hrng->Instance->DR;

    *random32bit = hrng->RandomNumber;
  
    hrng->State = HAL_RNG_STATE_READY;
  }
  else
  {
    status = HAL_ERROR;
  }
  
  /* Process Unlocked */
  __HAL_UNLOCK(hrng);
  
  return status;
}

/**
  * @brief  Generates a 32-bit random number in interrupt mode.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber_IT(RNG_HandleTypeDef *hrng)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Process Locked */
  __HAL_LOCK(hrng);
  
  /* Check RNG peripheral state */
  if(hrng->State == HAL_RNG_STATE_READY)
  {
    /* Change RNG peripheral state */  
    hrng->State = HAL_RNG_STATE_BUSY;  
  
    /* Process Unlocked */
    __HAL_UNLOCK(hrng);
    
    /* Enable the RNG Interrupts: Data Ready, Clock error, Seed error */ 
    __HAL_RNG_ENABLE_IT(hrng);
  }
  else
  {
    /* Process Unlocked */
    __HAL_UNLOCK(hrng);
    
    status = HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Handles RNG interrupt request.
  * @note   In the case of a clock error, the RNG is no more able to generate 
  *         random numbers because the PLL48CLK clock is not correct. User has 
  *         to check that the clock controller is correctly configured to provide
  *         the RNG clock and clear the CEIS bit using __HAL_RNG_CLEAR_IT(). 
  *         The clock error has no impact on the previously generated 
  *         random numbers, and the RNG_DR register contents can be used.
  * @note   In the case of a seed error, the generation of random numbers is 
  *         interrupted as long as the SECS bit is '1'. If a number is 
  *         available in the RNG_DR register, it must not be used because it may 
  *         not have enough entropy. In this case, it is recommended to clear the 
  *         SEIS bit using __HAL_RNG_CLEAR_IT(), then disable and enable 
  *         the RNG peripheral to reinitialize and restart the RNG.
  * @note   User-written HAL_RNG_ErrorCallback() API is called once whether SEIS
  *         or CEIS are set.  
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval None

  */

void HAL_RNG_IRQHandler(RNG_HandleTypeDef *hrng)
{

  /* RNG clock or seed  error interrupt occurred */
  if( (__HAL_RNG_GET_IT(hrng, RNG_IT_SEI) != RESET))
  { 
    /* Change RNG peripheral state */
    hrng->State = HAL_RNG_STATE_ERROR;
  
    HAL_RNG_ErrorCallback(hrng);
    
    /* Clear the clock error flag */
    __HAL_RNG_CLEAR_IT(hrng, RNG_IT_SEI);
    
  }
  
  /* Check RNG data ready interrupt occurred */    
  if(__HAL_RNG_GET_IT(hrng, RNG_IT_DRDY) != RESET)
  {
    /* Generate random number once, so disable the IT */
    __HAL_RNG_DISABLE_IT(hrng);
    
    /* Get the 32bit Random number (DRDY flag automatically cleared) */ 
    hrng->RandomNumber = hrng->Instance->DR;
    
    if(hrng->State != HAL_RNG_STATE_ERROR)
    {
      /* Change RNG peripheral state */
      hrng->State = HAL_RNG_STATE_READY; 
      
      /* Data Ready callback */ 
      HAL_RNG_ReadyDataCallback(hrng, hrng->RandomNumber);
    } 
  }
  
} 

/**
  * @brief  Returns generated random number in polling mode (Obsolete)
  *         Use HAL_RNG_GenerateRandomNumber() API instead.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval Random value
  */
uint32_t HAL_RNG_GetRandomNumber(RNG_HandleTypeDef *hrng)
{
  if(HAL_RNG_GenerateRandomNumber(hrng, &(hrng->RandomNumber)) == HAL_OK)
  {
    return hrng->RandomNumber; 
  }
  else
  {
    return 0U;
  }
}

/**
  * @brief  Returns a 32-bit random number with interrupt enabled (Obsolete),
  *         Use HAL_RNG_GenerateRandomNumber_IT() API instead.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval 32-bit random number
  */
uint32_t HAL_RNG_GetRandomNumber_IT(RNG_HandleTypeDef *hrng)
{
  uint32_t random32bit = 0U;
  
  /* Process locked */
  __HAL_LOCK(hrng);
  
  /* Change RNG peripheral state */  
  hrng->State = HAL_RNG_STATE_BUSY;  
  
  /* Get a 32bit Random number */ 
  random32bit = hrng->Instance->DR;
  
  /* Enable the RNG Interrupts: Data Ready, Clock error, Seed error */ 
  __HAL_RNG_ENABLE_IT(hrng); 


    /* Release Lock */// modify by Rakan 
  __HAL_UNLOCK(hrng);
	
  /* Return the 32 bit random number */   
  return random32bit;
}

/**
  * @brief  Read latest generated random number. 
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval random value
  */
uint32_t HAL_RNG_ReadLastRandomNumber(RNG_HandleTypeDef *hrng)
{
  return(hrng->RandomNumber);
}

/**
  * @brief  Data Ready callback in non-blocking mode. 
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @param  random32bit generated random number.
  * @retval None
  */
__weak void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrng);
  UNUSED(random32bit);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_RNG_ReadyDataCallback must be implemented in the user file.
   */
}

/**
  * @brief  RNG error callbacks.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval None
  */
__weak void HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrng);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_RNG_ErrorCallback must be implemented in the user file.
   */
}


/**
  * @brief  Returns the RNG state.
  * @param  hrng pointer to a RNG_HandleTypeDef structure that contains
  *                the configuration information for RNG.
  * @retval HAL state
  */
HAL_RNG_StateTypeDef HAL_RNG_GetState(RNG_HandleTypeDef *hrng)
{
  return hrng->State;
}


