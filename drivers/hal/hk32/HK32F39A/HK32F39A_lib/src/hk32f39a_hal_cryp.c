/**
  ******************************************************************************
  * @file    hk32f39a_hal_cryp.c
  * @author  Rakan
  * @date    2019/10/30
  * @brief   CRYP  module driver.
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_conf.h"

// Pending and Resume parameters for CRYP aes
typedef struct
{
  uint32_t u32Aes_cr ;
  uint32_t u32Aes_cr2 ;
  uint32_t u32Aes_sr ;
  uint32_t u32Aes_sr2 ;
  uint32_t u32Aes_keyr0 ;
  uint32_t u32Aes_keyr1 ;
  uint32_t u32Aes_keyr2 ;
  uint32_t u32Aes_keyr3 ;

  uint32_t u32Aes_ivr0 ;
  uint32_t u32Aes_ivr1 ;
  uint32_t u32Aes_ivr2 ;
  uint32_t u32Aes_ivr3 ;

  uint32_t u32Aes_Doutr0 ;
  uint32_t u32Aes_Doutr1 ;
  uint32_t u32Aes_Doutr2 ;
  uint32_t u32Aes_Doutr3 ;

  uint32_t u32Aes_Dinr0 ;
  uint32_t u32Aes_Dinr1 ;
  uint32_t u32Aes_Dinr2 ;
  uint32_t u32Aes_Dinr3 ;
  uint32_t u32tickstart ;
}AES_PAR;

AES_PAR sAesPar = { 0 };

//PendingTimeout :Pending  in interrupt mode,The aes operation will be suspended when a higher priority interrupt occurs in aes operations,so we need a timeout counter
uint32_t PendingTimeout = 0xFFFFFFFE;

/** @addtogroup CRYP_Private CRYP Private
  * @{
  */

#define  CRYP_ALGO_CHAIN_MASK         (AES_CR_MODE | AES_CR_CHMOD)
#define  HK_TIMEOUT						1000UL
#define  CRYP_KEYSIZE_MASK				AES_CR2_KEY_SIZE


/** @addtogroup CRYP_Private
  * @{
  */

static HAL_StatusTypeDef  CRYP_EncryptDecrypt_IT(CRYP_HandleTypeDef *hcryp);
static void               CRYP_SetInitVector(CRYP_HandleTypeDef *hcryp, uint8_t *InitVector);
static void               CRYP_SetKey(CRYP_HandleTypeDef *hcryp, uint8_t *Key);
static HAL_StatusTypeDef  CRYP_ProcessData(CRYP_HandleTypeDef *hcryp, uint8_t* Input, uint16_t Ilength, uint8_t* Output, uint32_t Timeout);
static void               CRYP_DMAInCplt(DMA_HandleTypeDef *hdma);
static void               CRYP_DMAOutCplt(DMA_HandleTypeDef *hdma);
static void               CRYP_DMAError(DMA_HandleTypeDef *hdma);
static void               CRYP_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr);



/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CRYP according to the specified
  *         parameters in the CRYP_InitTypeDef and creates the associated handle.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_Init(CRYP_HandleTypeDef *hcryp)
{ 
  /* Check the CRYP handle allocation */
  if(hcryp == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_AES_ALL_INSTANCE(hcryp->Instance));
  assert_param(IS_CRYP_DATATYPE(hcryp->Init.DataType));
  
 if(hcryp->State == HAL_CRYP_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcryp->Lock = HAL_UNLOCKED;

    /* Init the low level hardware */
    HAL_CRYP_MspInit(hcryp);
  }
//  /* Check if AES already enabled */
 if (HAL_IS_BIT_CLR(hcryp->Instance->CR, AES_CR_EN))
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;  

    /* Set the data type*/
    MODIFY_REG(hcryp->Instance->CR, AES_CR_DATATYPE, hcryp->Init.DataType);
    
    /* Reset CrypInCount and CrypOutCount */
    hcryp->CrypInCount = 0U;
    hcryp->CrypOutCount = 0U;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Set the default CRYP phase */
    hcryp->Phase = HAL_CRYP_PHASE_READY;
    
    /* Return function status */
    return HAL_OK;
	}
	else
  {
    /* The Datatype selection must be changed if the AES is disabled. Writing these bits while the AES is */
    /* enabled is forbidden to avoid unpredictable AES behavior.*/

    /* Return function status */
    return HAL_ERROR;
  }

}

/**
  * @brief  DeInitializes the CRYP peripheral. 
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_DeInit(CRYP_HandleTypeDef *hcryp)
{
  /* Check the CRYP handle allocation */
  if(hcryp == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_BUSY;
  
  /* Set the default CRYP phase */
  hcryp->Phase = HAL_CRYP_PHASE_READY;
  
  /* Reset CrypInCount and CrypOutCount */
  hcryp->CrypInCount = 0U;
  hcryp->CrypOutCount = 0U;
  
  /* Disable the CRYP Peripheral Clock */
  __HAL_CRYP_DISABLE(hcryp);
  
  /* DeInit the low level hardware: CLOCK, NVIC.*/
  HAL_CRYP_MspDeInit(hcryp);
  
  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_RESET;
  
  /* Release Lock */
  __HAL_UNLOCK(hcryp);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initializes the CRYP MSP.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_MspInit(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed, 
            the HAL_CRYP_MspInit can be implemented in the user file */
}

/**
  * @brief  DeInitializes CRYP MSP.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed, 
            the HAL_CRYP_MspDeInit can be implemented in the user file */
}


/** @addtogroup CRYP_Exported_Functions_Group2
 *  @brief   processing functions. 
 *
 */

/**
  * @brief  Initializes the CRYP peripheral in AES ECB encryption mode
  *         then encrypt pPlainData. The cypher data are available in pCypherData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value 
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{
  /* Process Locked */
  __HAL_LOCK(hcryp);

  /* Check that data aligned on u32 and Size multiple of 16*/
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if(hcryp->State != HAL_CRYP_STATE_RESET)
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES ECB mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_ENCRYPT);
      
      /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Write Plain Data and Get Cypher Data */
    if(CRYP_ProcessData(hcryp, pPlainData, Size, pCypherData, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);
	
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CBC encryption mode
  *         then encrypt pPlainData. The cypher data are available in pCypherData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{
  /* Process Locked */
  __HAL_LOCK(hcryp);
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if(hcryp->State != HAL_CRYP_STATE_RESET)
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CBC mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Write Plain Data and Get Cypher Data */
    if(CRYP_ProcessData(hcryp, pPlainData, Size, pCypherData, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR encryption mode
  *         then encrypt pPlainData. The cypher data are available in pCypherData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData, uint32_t Timeout)
{  
  /* Process Locked */
  __HAL_LOCK(hcryp);
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if(hcryp->State != HAL_CRYP_STATE_RESET)
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CTR mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Write Plain Data and Get Cypher Data */
    if(CRYP_ProcessData(hcryp, pPlainData, Size, pCypherData, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB decryption mode
  *         then decrypted pCypherData. The cypher data are available in pPlainData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value  
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{
  /* Process Locked */
  __HAL_LOCK(hcryp);
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if(hcryp->State != HAL_CRYP_STATE_RESET)
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES ECB decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_KEYDERDECRYPT);
      
      /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Write Cypher Data and Get Plain Data */
    if(CRYP_ProcessData(hcryp, pCypherData, Size, pPlainData, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB decryption mode
  *         then decrypted pCypherData. The cypher data are available in pPlainData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value  
  * @retval HAL status
  */
  
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{
  /* Process Locked */
  __HAL_LOCK(hcryp);
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if(hcryp->State != HAL_CRYP_STATE_RESET)
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
	  
      /* Set the CRYP peripheral in AES CBC decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_0);
	  
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }

	/*AES_CBC for HK32F39A special handling -----begin--*/	
	/* Get timeout */
	uint32_t  u32tickstart = 0;
	
	u32tickstart = HAL_GetTick();
	
	while(HAL_IS_BIT_CLR(hcryp->Instance->SR, AES_SR_CCF))
    {    
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0U)||((HAL_GetTick() - u32tickstart ) > Timeout))
        {
          /* Change state */
          hcryp->State = HAL_CRYP_STATE_TIMEOUT;
          
          /* Process Unlocked */          
          __HAL_UNLOCK(hcryp);
          
          return HAL_TIMEOUT;
        }
      }		
    }
	/* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_CCF);
	
	/* Disable CRYP */
    __HAL_CRYP_DISABLE(hcryp);
	
	/* Reset the CHMOD & MODE bits */
	CLEAR_BIT(hcryp->Instance->CR, AES_CR_MODE);	
	
	/* Set the CRYP peripheral in AES CBC decryption mode (with decryption) */
    __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_1);
	
	/* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);		 
	
	/*AES_CBC	for HK32F39A special handling-----end--*/
		
    /* Write Cypher Data and Get Plain Data */
    if(CRYP_ProcessData(hcryp, pCypherData, Size, pPlainData, Timeout) != HAL_OK)
    {
      return HAL_TIMEOUT;
    }
	
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR decryption mode
  *         then decrypted pCypherData. The cypher data are available in pPlainData
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Timeout Specify Timeout value   
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData, uint32_t Timeout)
{  
  /* Process Locked */
  __HAL_LOCK(hcryp);
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if initialization phase has already been performed */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->Phase == HAL_CRYP_PHASE_READY))
  {
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Set the key */
    CRYP_SetKey(hcryp, hcryp->Init.pKey);
    
    /* Reset the CHMOD & MODE bits */
    CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
    
    /* Set the CRYP peripheral in AES CTR decryption mode */
    __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_DECRYPT);
    
    /* Set the Initialization Vector */
    CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
    
    /* Set the phase */
    hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
  }
	
  /* Write Cypher Data and Get Plain Data */
  if(CRYP_ProcessData(hcryp, pCypherData, Size, pPlainData, Timeout) != HAL_OK)
  {
    return HAL_TIMEOUT;
  }
  
  /* Change the CRYP state */
  hcryp->State = HAL_CRYP_STATE_READY;
  
  /* Process Unlocked */
  __HAL_UNLOCK(hcryp);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB encryption mode using Interrupt.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pPlainData;
    hcryp->pCrypOutBuffPtr = pCypherData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES ECB mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_ENCRYPT);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Enable Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
    
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CBC encryption mode using Interrupt.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pPlainData;
    hcryp->pCrypOutBuffPtr = pCypherData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CBC mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Enable Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
    
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
   
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR encryption mode using Interrupt.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pPlainData;
    hcryp->pCrypOutBuffPtr = pCypherData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CTR mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Enable Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
    
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB decryption mode using Interrupt.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pCypherData;
    hcryp->pCrypOutBuffPtr = pPlainData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES ECB decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_KEYDERDECRYPT);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Enable Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
    
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;    
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CBC decryption mode using IT.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pCypherData;
    hcryp->pCrypOutBuffPtr = pPlainData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CBC decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_0);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);  

	  /* Enable CRYP */
      __HAL_CRYP_ENABLE(hcryp);
	  
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;	
    }
    
	/*AES_CBC for HK32F39A special handling -----begin--*/	
	/* Get timeout */
	
	uint32_t  u32tickstart = 0;
	uint32_t  u32Timeout = HK_TIMEOUT;
	u32tickstart = HAL_GetTick();
	while(HAL_IS_BIT_CLR(hcryp->Instance->SR, AES_SR_CCF))
    {    
      /* Check for the Timeout */
      if(u32Timeout != HAL_MAX_DELAY)
      {
        if((u32Timeout == 0U)||((HAL_GetTick() - u32tickstart ) > u32Timeout))
        {
          /* Change state */
          hcryp->State = HAL_CRYP_STATE_TIMEOUT;
          
          /* Process Unlocked */          
          __HAL_UNLOCK(hcryp);
          
          return HAL_TIMEOUT;
        }
      }		
    }
	
	 /* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_CCF);
	
	/* Disable CRYP */
    __HAL_CRYP_DISABLE(hcryp);
		
	/* Reset the CHMOD & MODE bits */
	  CLEAR_BIT(hcryp->Instance->CR, AES_CR_MODE);	
		
	/* Set the CRYP peripheral in AES CBC decryption mode (with decryption) */
    __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_1);
		
	/* Enable Interrupts */		
	__HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);		
		
	/* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);		 
	/*AES_CBC	for HK32F39A special handling-----end--*/
	
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;    
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR decryption mode using Interrupt.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt_IT(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  uint32_t inputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    /* Get the buffer addresses and sizes */
    hcryp->CrypInCount = Size;
    hcryp->pCrypInBuffPtr = pCypherData;
    hcryp->pCrypOutBuffPtr = pPlainData;
    hcryp->CrypOutCount = Size;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CTR decryption mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_DECRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Enable Interrupts */
    __HAL_CRYP_ENABLE_IT(hcryp, CRYP_IT_CC);
    
    /* Enable CRYP */
    __HAL_CRYP_ENABLE(hcryp);
	
    /* Get the last input data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;    
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB encryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pPlainData;
    outputaddr = (uint32_t)pCypherData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
			
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
			
      /* Set the CRYP peripheral in AES ECB mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_ENCRYPT);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {  
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CBC encryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pPlainData;
    outputaddr = (uint32_t)pCypherData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
	  
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
			
      /* Set the CRYP peripheral in AES CBC mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR encryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16.
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Encrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pPlainData, uint16_t Size, uint8_t *pCypherData)
{
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pPlainData;
    outputaddr = (uint32_t)pCypherData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);

	  /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
			
      /* Set the CRYP peripheral in AES CTR mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_ENCRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES ECB decryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESECB_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{  
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pCypherData;
    outputaddr = (uint32_t)pPlainData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES ECB decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_ECB_KEYDERDECRYPT);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CBC encryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16 bytes
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCBC_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pCypherData;
    outputaddr = (uint32_t)pPlainData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Reset the CHMOD & MODE bits */
      CLEAR_BIT(hcryp->Instance->CR, CRYP_ALGO_CHAIN_MASK);
      
      /* Set the CRYP peripheral in AES CBC decryption mode (with key derivation) */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_0);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
   
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);

	/*AES_CBC for HK32F39A special handling -----begin--*/	
	/* Get timeout */
	uint32_t	u32tickstart = 0;
	uint32_t	u32Timeout = HK_TIMEOUT;
	u32tickstart = HAL_GetTick();
	while(HAL_IS_BIT_CLR(hcryp->Instance->SR, AES_SR_CCF))
	{	 
	  /* Check for the Timeout */
	  if(u32Timeout != HAL_MAX_DELAY)
	  {
		if((u32Timeout == 0U)||((HAL_GetTick() - u32tickstart ) > u32Timeout))
		{
		  /* Change state */
		  hcryp->State = HAL_CRYP_STATE_TIMEOUT;
		  
		  /* Process Unlocked */		  
		  __HAL_UNLOCK(hcryp);
		  
		  return HAL_TIMEOUT;
		}
	  } 	
	  
	}
	/* Disable CRYP */
	__HAL_CRYP_DISABLE(hcryp);

	/* Reset the CHMOD & MODE bits */
	CLEAR_BIT(hcryp->Instance->CR, AES_CR_MODE);	

	/* Set the CRYP peripheral in AES CBC decryption mode (with decryption) */
	__HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CBC_KEYDERDECRYPT_HK_1);

	/* Enable CRYP */
	__HAL_CRYP_ENABLE(hcryp);		 
	/*AES_CBC	for HK32F39A special handling-----end--*/

    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}

/**
  * @brief  Initializes the CRYP peripheral in AES CTR decryption mode using DMA.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  pCypherData Pointer to the cyphertext buffer (aligned on u32)
  * @param  Size Length of the plaintext buffer, must be a multiple of 16
  * @param  pPlainData Pointer to the plaintext buffer (aligned on u32)
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRYP_AESCTR_Decrypt_DMA(CRYP_HandleTypeDef *hcryp, uint8_t *pCypherData, uint16_t Size, uint8_t *pPlainData)
{  
  uint32_t inputaddr = 0U, outputaddr = 0U;
  
  /* Check that data aligned on u32 */
  if((((uint32_t)pPlainData & (uint32_t)0x00000003U) != 0U) || (((uint32_t)pCypherData & (uint32_t)0x00000003U) != 0U) || ((Size & (uint16_t)0x000FU) != 0U))
  {
    /* Process Locked */
    __HAL_UNLOCK(hcryp);

    /* Return function status */
    return HAL_ERROR;
  }
  
  /* Check if HAL_CRYP_Init has been called */
  if ((hcryp->State != HAL_CRYP_STATE_RESET) && (hcryp->State == HAL_CRYP_STATE_READY))
  {
    /* Process Locked */
    __HAL_LOCK(hcryp);
    
    inputaddr  = (uint32_t)pCypherData;
    outputaddr = (uint32_t)pPlainData;
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_BUSY;
    
    /* Check if initialization phase has already been performed */
    if(hcryp->Phase == HAL_CRYP_PHASE_READY)
    {
      /* Set the key */
      CRYP_SetKey(hcryp, hcryp->Init.pKey);
      
      /* Set the CRYP peripheral in AES CTR mode */
      __HAL_CRYP_SET_MODE(hcryp, CRYP_CR_ALGOMODE_AES_CTR_DECRYPT);
      
      /* Set the Initialization Vector */
      CRYP_SetInitVector(hcryp, hcryp->Init.pInitVect);
      
      /* Set the phase */
      hcryp->Phase = HAL_CRYP_PHASE_PROCESS;
    }
    
    /* Set the input and output addresses and start DMA transfer */ 
    CRYP_SetDMAConfig(hcryp, inputaddr, Size, outputaddr);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Release Lock */
    __HAL_UNLOCK(hcryp);
  
    return HAL_ERROR;   
  }
}



/** @addtogroup CRYP_Exported_Functions_Group3
 *  @brief   DMA callback functions. 
 *
@verbatim   
  ==============================================================================
                      ##### DMA callback functions  #####
  ==============================================================================  
    [..]  This section provides DMA callback functions:
      (+) DMA Input data transfer complete
      (+) DMA Output data transfer complete
      (+) DMA error

@endverbatim
  * @{
  */

/**
  * @brief  CRYP error callback.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
 __weak void HAL_CRYP_ErrorCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed, 
            the HAL_CRYP_ErrorCallback can be implemented in the user file
   */ 
}

/**
  * @brief  Input transfer completed callback.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_InCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed, 
            the HAL_CRYP_InCpltCallback can be implemented in the user file
   */ 
}

/**
  * @brief  Output transfer completed callback.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYP_OutCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function should not be modified; when the callback is needed, 
            the HAL_CRYP_OutCpltCallback can be implemented in the user file
   */ 
}


/** @addtogroup CRYP_Exported_Functions_Group4
 *  @brief   CRYP IRQ handler.
 *
@verbatim   
  ==============================================================================
                ##### CRYP IRQ handler management #####
  ==============================================================================  
[..]  This section provides CRYP IRQ handler function.

@endverbatim
  * @{
  */

/**
  * @brief  This function handles CRYP interrupt request.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */

void HAL_CRYP_IRQHandler(CRYP_HandleTypeDef *hcryp)
{
  /* Check if error occurred*/
  if (__HAL_CRYP_GET_IT_SOURCE(hcryp, CRYP_IT_ERR) != RESET)
  {
    if (__HAL_CRYP_GET_FLAG(hcryp,CRYP_FLAG_RDERR) != RESET)
    {
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_RDERR);
    }
    
    if (__HAL_CRYP_GET_FLAG(hcryp,CRYP_FLAG_WRERR) != RESET)
    {
      __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_WRERR);
    }
    
    if (__HAL_CRYP_GET_FLAG(hcryp,CRYP_FLAG_CCF) != RESET)
    {
      __HAL_CRYP_CLEAR_FLAG(hcryp,CRYP_CLEARFLAG_CCF);
    }
    
    hcryp->State= HAL_CRYP_STATE_ERROR;
    /* Disable Computation Complete Interrupt */
    __HAL_CRYP_DISABLE_IT(hcryp,CRYP_IT_CC);
    __HAL_CRYP_DISABLE_IT(hcryp,CRYP_IT_ERR);
    
    HAL_CRYP_ErrorCallback(hcryp);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    return;
  }
  
  /* Check if computation complete interrupt was enabled*/
  if (__HAL_CRYP_GET_IT_SOURCE(hcryp, CRYP_IT_CC) != RESET)
  {
    /* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_CCF);

    CRYP_EncryptDecrypt_IT(hcryp);
	
  }
}

/**
  * @}
  */

/** @addtogroup CRYP_Exported_Functions_Group5
 *  @brief   Peripheral State functions. 
 *
@verbatim   
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================  
    [..]
    This subsection permits to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Returns the CRYP state.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL state
  */
HAL_CRYP_STATETypeDef HAL_CRYP_GetState(CRYP_HandleTypeDef *hcryp)
{
  return hcryp->State;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup CRYP_Private
  * @{
  */

/**
  * @brief  IT function called under interruption context to continue encryption or decryption
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval HAL status
  */
static HAL_StatusTypeDef CRYP_EncryptDecrypt_IT(CRYP_HandleTypeDef *hcryp)
{
  uint32_t inputaddr = 0U, outputaddr = 0U;

  /* Get the last Output data adress */
  outputaddr = (uint32_t)hcryp->pCrypOutBuffPtr;
  
  /* Read the Output block from the Output Register */
  *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
  outputaddr+=4U;
  *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
  
  hcryp->pCrypOutBuffPtr += 16U;
  hcryp->CrypOutCount -= 16U;
  
  /* Check if all input text is encrypted or decrypted */
  if(hcryp->CrypOutCount == 0U)
  {
    /* Disable Computation Complete Interrupt */
    __HAL_CRYP_DISABLE_IT(hcryp,CRYP_IT_CC);
    __HAL_CRYP_DISABLE_IT(hcryp,CRYP_IT_ERR);
    
    /* Process Unlocked */
    __HAL_UNLOCK(hcryp);
    
    /* Change the CRYP state */
    hcryp->State = HAL_CRYP_STATE_READY;

    /* Call computation complete callback */
    HAL_CRYPEx_ComputationCpltCallback(hcryp);
  }
  else /* Process the rest of input text */
  {
    /* Get the last Intput data adress */
    inputaddr = (uint32_t)hcryp->pCrypInBuffPtr;
    
    /* Write the Input block in the Data Input register */
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    hcryp->pCrypInBuffPtr += 16U;
    hcryp->CrypInCount -= 16U;      
  
  }
  return HAL_OK;
}
/**
  * @brief  DMA CRYP Input Data process complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void CRYP_DMAInCplt(DMA_HandleTypeDef *hdma)  
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  
  /* Disable the DMA transfer for input request  */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAINEN);
  
  /* Call input data transfer complete callback */
  HAL_CRYP_InCpltCallback(hcryp);
}

/**
  * @brief  DMA CRYP Output Data process complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void CRYP_DMAOutCplt(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  
  /* Disable the DMA transfer for output request by resetting the DMAOUTEN bit
     in the DMACR register */
  CLEAR_BIT(hcryp->Instance->CR, AES_CR_DMAOUTEN);

  /* Clear CCF Flag */
  __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_CCF);

  /* Disable CRYP */
  __HAL_CRYP_DISABLE(hcryp);
  
  /* Change the CRYP state to ready */
  hcryp->State = HAL_CRYP_STATE_READY;
  
  /* Call output data transfer complete callback */
  HAL_CRYP_OutCpltCallback(hcryp);
}

/**
  * @brief  DMA CRYP communication error callback. 
  * @param  hdma DMA handle
  * @retval None
  */
static void CRYP_DMAError(DMA_HandleTypeDef *hdma)
{
  CRYP_HandleTypeDef* hcryp = (CRYP_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;
  hcryp->State= HAL_CRYP_STATE_ERROR;
  HAL_CRYP_ErrorCallback(hcryp);
}

/**
  * @brief  Writes the Key in Key registers. 
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  Key Pointer to Key buffer
  * @note Key must be written as little endian.
  *         If Key pointer points at address n, 
  *         n[15:0] contains key[96:127], 
  *         (n+4)[15:0] contains key[64:95], 
  *         (n+8)[15:0] contains key[32:63] and 
  *         (n+12)[15:0] contains key[0:31]
  *         ....
  * @retval None
  */
static void CRYP_SetKey(CRYP_HandleTypeDef *hcryp, uint8_t *Key)
{  
	uint32_t keyaddr = (uint32_t)Key;
	
	/* ReSet the keySize */
	CLEAR_BIT(hcryp->Instance->CR2,CRYP_KEYSIZE_MASK);
	__HAL_CRYP_SET_KEY_SIZE(hcryp,hcryp->Init.KeySize);
	  
	if(hcryp->Init.KeySize == CRYP_KEYSIZE_128B)
	{
		hcryp->Instance->KEYR3 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR2 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR1 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR0 = __REV(*(uint32_t*)(keyaddr));

	}
	else if(hcryp->Init.KeySize == CRYP_KEYSIZE_192B)
	{			
		hcryp->Instance->KEYR5 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR4 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR3 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR2 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR1 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR0 = __REV(*(uint32_t*)(keyaddr));
	}
	else
	{
		hcryp->Instance->KEYR7 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR6 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR5 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR4 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR3 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR2 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR1 = __REV(*(uint32_t*)(keyaddr));
		keyaddr+=4U;
		hcryp->Instance->KEYR0 = __REV(*(uint32_t*)(keyaddr));
	}
		
}

/**
  * @brief  Writes the InitVector/InitCounter in IV registers. 
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  InitVector Pointer to InitVector/InitCounter buffer
  * @note Init Vector must be written as little endian.
  *         If Init Vector pointer points at address n, 
  *         n[15:0] contains Vector[96:127], 
  *         (n+4)[15:0] contains Vector[64:95], 
  *         (n+8)[15:0] contains Vector[32:63] and 
  *         (n+12)[15:0] contains Vector[0:31]
  * @retval None
  */
static void CRYP_SetInitVector(CRYP_HandleTypeDef *hcryp, uint8_t *InitVector)
{
  uint32_t ivaddr = (uint32_t)InitVector;
  
  hcryp->Instance->IVR3 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR2 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR1 = __REV(*(uint32_t*)(ivaddr));
  ivaddr+=4U;
  hcryp->Instance->IVR0 = __REV(*(uint32_t*)(ivaddr));
}

/**
  * @brief  Process Data: Writes Input data in polling mode and reads the output data
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  Input Pointer to the Input buffer
  * @param  Ilength Length of the Input buffer, must be a multiple of 16.
  * @param  Output Pointer to the returned buffer
  * @param  Timeout Specify Timeout value  
  * @retval None
  */

static HAL_StatusTypeDef CRYP_ProcessData(CRYP_HandleTypeDef *hcryp, uint8_t* Input, uint16_t Ilength, uint8_t* Output, uint32_t Timeout)
{
  uint32_t tickstart = 0U;
  
  uint32_t index = 0U;
  uint32_t inputaddr  = (uint32_t)Input;
  uint32_t outputaddr = (uint32_t)Output;

  for(index=0U; (index < Ilength); index += 16U)
  {
 
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR  = *(uint32_t*)(inputaddr);
    inputaddr+=4U;
    hcryp->Instance->DINR = *(uint32_t*)(inputaddr);
    inputaddr+=4U;

    /* Get timeout */
    tickstart = HAL_GetTick();
    
    while(HAL_IS_BIT_CLR(hcryp->Instance->SR, AES_SR_CCF))
    {    
			
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0U)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          /* Change state */
          hcryp->State = HAL_CRYP_STATE_TIMEOUT;
          
          /* Process Unlocked */          
          __HAL_UNLOCK(hcryp);
          
         // return HAL_TIMEOUT;
          return HAL_OK;
        }
      }		
    }

		
    /* Clear CCF Flag */
    __HAL_CRYP_CLEAR_FLAG(hcryp, CRYP_CLEARFLAG_CCF);

    /* Read the Output block from the Data Output Register */
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;
    *(uint32_t*)(outputaddr) = hcryp->Instance->DOUTR;
    outputaddr+=4U;

  }
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Set the DMA configuration and start the DMA transfer
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @param  inputaddr address of the Input buffer
  * @param  Size Size of the Input buffer, must be a multiple of 16.
  * @param  outputaddr address of the Output buffer
  * @retval None
  */
static void CRYP_SetDMAConfig(CRYP_HandleTypeDef *hcryp, uint32_t inputaddr, uint16_t Size, uint32_t outputaddr)
{
  /* Set the CRYP DMA transfer complete callback */
  hcryp->hdmain->XferCpltCallback = CRYP_DMAInCplt;
  /* Set the DMA error callback */
  hcryp->hdmain->XferErrorCallback = CRYP_DMAError;
  
  /* Set the CRYP DMA transfer complete callback */
  hcryp->hdmaout->XferCpltCallback = CRYP_DMAOutCplt;
  /* Set the DMA error callback */
  hcryp->hdmaout->XferErrorCallback = CRYP_DMAError;

  /* Enable the DMA In DMA Stream */
  HAL_DMA_Start_IT(hcryp->hdmain, inputaddr, (uint32_t)&hcryp->Instance->DINR, Size/4);

  /* Enable the DMA Out DMA Stream */
  HAL_DMA_Start_IT(hcryp->hdmaout, (uint32_t)&hcryp->Instance->DOUTR, outputaddr, Size/4);

  /* Enable In and Out DMA requests */
  SET_BIT(hcryp->Instance->CR, (AES_CR_DMAINEN | AES_CR_DMAOUTEN));

  /* Enable CRYP */
  __HAL_CRYP_ENABLE(hcryp);

}



/**
  * @brief  Pending configuration in interrupt mode , The aes operation will be suspended 
  *     when a higher priority interrupt occurs in aes operations. 
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
*/
void CRYP_SetPendingConfig_IT(CRYP_HandleTypeDef hcryp)
{
    //0.Save the value of the corresponding register
  sAesPar.u32Aes_cr = READ_REG(hcryp.Instance->CR);
  sAesPar.u32Aes_cr2 = READ_REG(hcryp.Instance->CR2);
  sAesPar.u32Aes_sr2 = READ_REG(hcryp.Instance->SR2);
  sAesPar.u32Aes_sr = READ_REG(hcryp.Instance->SR);
  
  //1.If dma is used to clear aes.dmainen
  if(READ_BIT(hcryp.Instance->CR, AES_CR_DMAINEN))
  {
    CLEAR_BIT(hcryp.Instance->CR, AES_CR_DMAINEN);
  }
  
  //2. wait AES_SR.CCF to be Seted
   /* Get timeout */
  sAesPar.u32tickstart = HAL_GetTick();

  while(HAL_IS_BIT_CLR(hcryp.Instance->SR, AES_SR_CCF))
  {  
      
    /* Check for the Timeout */
    if(PendingTimeout != HAL_MAX_DELAY)
    {
			if((PendingTimeout == 0U)||((HAL_GetTick() - sAesPar.u32tickstart ) > PendingTimeout))
			{
				/* Change state */
				hcryp.State = HAL_CRYP_STATE_TIMEOUT;
				
				return ;
			}
    }   
  }

  //3.If dma is used, the operation of dma read dout will still be done by removing the aes. 
  //dmaout.dmaouten clear hardware. if no dma is used, read the 4-times _ doutr register and press the result stack 
  if(READ_BIT(hcryp.Instance->CR, AES_CR_DMAOUTEN))
  {
    CLEAR_BIT(hcryp.Instance->CR, AES_CR_DMAOUTEN);
  }
  else
  {
    sAesPar.u32Aes_Doutr3 = hcryp.Instance->DOUTR;
    sAesPar.u32Aes_Doutr2 = hcryp.Instance->DOUTR;
    sAesPar.u32Aes_Doutr1 = hcryp.Instance->DOUTR;
    sAesPar.u32Aes_Doutr0 = hcryp.Instance->DOUTR;
  }
  
  //4.set bit cr register to clear ccf flag
  SET_BIT(hcryp.Instance->CR, AES_CR_CCFC);
  
  //5.set bit cr register to clear ccf flag
  CLEAR_BIT(hcryp.Instance->CR, AES_CR_EN);
  
  //6.Read the aes _ dinr for 4 consecutive times and press the stack
  sAesPar.u32Aes_Dinr3 = hcryp.Instance->DINR ;
  sAesPar.u32Aes_Dinr2 = hcryp.Instance->DINR ;
  sAesPar.u32Aes_Dinr1 = hcryp.Instance->DINR ;
  sAesPar.u32Aes_Dinr0 = hcryp.Instance->DINR ;
  
  //7.Read the aes _ cr, aes _ cr2, aes _ keyrx, aes _ ivrx registers and press the stack 
  sAesPar.u32Aes_cr =  hcryp.Instance->CR;
  sAesPar.u32Aes_cr2 =  hcryp.Instance->CR2;
  sAesPar.u32Aes_keyr3 =  hcryp.Instance->KEYR3;
  sAesPar.u32Aes_keyr2 =  hcryp.Instance->KEYR2;
  sAesPar.u32Aes_keyr1 =  hcryp.Instance->KEYR1;
  sAesPar.u32Aes_keyr0 =  hcryp.Instance->KEYR0;
  sAesPar.u32Aes_ivr3 =  hcryp.Instance->IVR3;
  sAesPar.u32Aes_ivr2 =  hcryp.Instance->IVR2;
  sAesPar.u32Aes_ivr1 =  hcryp.Instance->IVR1;
  sAesPar.u32Aes_ivr0 =  hcryp.Instance->IVR0;


}

/**
  * @brief  interrupt Resume , The aes operation will be suspended 
  *     when a higher priority interrupt occurs in aes operations. 
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
*/

void CRYP_SetResumeConfig_IT(CRYP_HandleTypeDef hcryp)
{
  //0.Set aes_cr.en to 0 and turn off aes
  //Resume AES_CR,AES_CR2 config
  SET_BIT(hcryp.Instance->CR, AES_CR_CCFC);
  CLEAR_BIT(hcryp.Instance->CR, AES_CR_EN);
  CLEAR_REG(hcryp.Instance->CR2);

  //1.Resume AES_KEYRx config
  hcryp.Instance->KEYR3 = sAesPar.u32Aes_keyr3 ;
  hcryp.Instance->KEYR2 = sAesPar.u32Aes_keyr2 ;
  hcryp.Instance->KEYR1 = sAesPar.u32Aes_keyr1 ;
  hcryp.Instance->KEYR0 = sAesPar.u32Aes_keyr0 ;

  //2. Resume AES_IVRx config
  hcryp.Instance->IVR3 = sAesPar.u32Aes_ivr3 ;
  hcryp.Instance->IVR2 = sAesPar.u32Aes_ivr2 ;
  hcryp.Instance->IVR1 = sAesPar.u32Aes_ivr1 ;
  hcryp.Instance->IVR0 = sAesPar.u32Aes_ivr0 ; 

  if( (sAesPar.u32Aes_sr & 0x00000001) != 0x01 )
  {
    WRITE_REG(hcryp.Instance->CR, sAesPar.u32Aes_cr);
  }
  if( ( sAesPar.u32Aes_sr2 >> 4 ) == 0x00 )
  {
    WRITE_REG(hcryp.Instance->CR2,  sAesPar.u32Aes_cr2 | 0x20) ;
  } 
  else
  {
    WRITE_REG(hcryp.Instance->CR2,  sAesPar.u32Aes_cr2 ) ;
  }

  // 3.Resume AES_DINR and AES_DOUTR 
  if( ((sAesPar.u32Aes_sr2 & 0xffffffef) >> 2 ) == 0x01 )
  {
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr0 ;
  } 
  else if( ((sAesPar.u32Aes_sr2 & 0xffffffef) >> 2 ) == 0x02 )
  {
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr1 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr0 ;
  } 
  else if( ((sAesPar.u32Aes_sr2 & 0xffffffef) >> 2 ) == 0x03 )
  {
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr2 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr1 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr0 ;
  } 
  else
  {
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr3 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr2 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr1 ;
    hcryp.Instance->DINR = sAesPar.u32Aes_Dinr0 ;
  }

  if(READ_BIT(hcryp.Instance->CR, AES_CR_EN) == 0x00)
  {
    hcryp.Instance->DOUTR = sAesPar.u32Aes_Doutr3 ;
    hcryp.Instance->DOUTR = sAesPar.u32Aes_Doutr2 ;
    hcryp.Instance->DOUTR = sAesPar.u32Aes_Doutr1 ;
    hcryp.Instance->DOUTR = sAesPar.u32Aes_Doutr0 ;
  }
  //set AES_CR.EN and AES_CR2.CCF_SET and AES_CR2.INT_RESUME
  if( (sAesPar.u32Aes_sr & 0x00000001) == 0x01 )
  {
    WRITE_REG(hcryp.Instance->CR,  sAesPar.u32Aes_cr) ;
    WRITE_REG(hcryp.Instance->CR2,  sAesPar.u32Aes_cr2 | 0x20);
  }

}



