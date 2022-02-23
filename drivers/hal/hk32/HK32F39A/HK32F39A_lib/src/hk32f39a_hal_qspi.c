/**
  ******************************************************************************
  * @file    hk32f39a_hal_qspi.c
  * @author  laura.c
  * @version V1.0  
  * @brief   QSPI HAL module driver.
 **/


/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"
#ifdef HAL_QSPI_MODULE_ENABLED
 
/** QSPI_Private_Constants   */
#define QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE 0x00000000U                     /*!<Indirect write mode*/
#define QSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)QUADSPI_CCR_FMODE_0) /*!<Indirect read mode*/
#define QSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)QUADSPI_CCR_FMODE_1) /*!<Automatic polling mode*/
#define QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)QUADSPI_CCR_FMODE)   /*!<Memory-mapped mode*/

/*QSPI_Private_Macros QSPI Private Macros  */
#define IS_QSPI_FUNCTIONAL_MODE(MODE) (((MODE) == QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE) || \
                                       ((MODE) == QSPI_FUNCTIONAL_MODE_INDIRECT_READ)  || \
                                       ((MODE) == QSPI_FUNCTIONAL_MODE_AUTO_POLLING)   || \
                                       ((MODE) == QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED))
                               

/*QSPI_Private_Functions QSPI Private Functions  */
static void QSPI_DMARxCplt(DMA_HandleTypeDef *hdma);
static void QSPI_DMATxCplt(DMA_HandleTypeDef *hdma);
static void QSPI_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void QSPI_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void QSPI_DMAError(DMA_HandleTypeDef *hdma); 
static void QSPI_DMAAbortCplt(DMA_HandleTypeDef *hdma);
static HAL_StatusTypeDef QSPI_WaitFlagStateUntilTimeout(QSPI_HandleTypeDef *hqspi, uint32_t Flag, FlagStatus State, uint32_t tickstart, uint32_t Timeout);
static void QSPI_Config(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, uint32_t FunctionalMode);

  
/* QSPI_Exported_Functions QSPI Exported Functions  */

/*====================================================================
            ##### Initialization and Configuration functions #####
 =====================================================================
    [..]
    This subsection provides a set of functions allowing to :
      (+) Initialize the QuadSPI.
      (+) De-initialize the QuadSPI.
*/

/**
  * @brief Initializes the QSPI mode according to the specified parameters
  *        in the QSPI_InitTypeDef and creates the associated handle.
  * @param hqspi qspi handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Init(QSPI_HandleTypeDef *hqspi)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t tickstart = HAL_GetTick();
  
  /* Check the QSPI handle allocation */
  if(hqspi == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
//  assert_param(IS_QSPI_ALL_INSTANCE(hqspi->Instance));
  assert_param(IS_QSPI_CLOCK_PRESCALER(hqspi->Init.ClockPrescaler));
  assert_param(IS_QSPI_FIFO_THRESHOLD(hqspi->Init.FifoThreshold));
  assert_param(IS_QSPI_SSHIFT(hqspi->Init.SampleShifting));
  assert_param(IS_QSPI_FLASH_SIZE(hqspi->Init.FlashSize));
  assert_param(IS_QSPI_CS_HIGH_TIME(hqspi->Init.ChipSelectHighTime));
  assert_param(IS_QSPI_CLOCK_MODE(hqspi->Init.ClockMode));
  assert_param(IS_QSPI_DUAL_FLASH_MODE(hqspi->Init.DualFlash));

  if (hqspi->Init.DualFlash != QSPI_DUALFLASH_ENABLE )
  {
    assert_param(IS_QSPI_FLASH_ID(hqspi->Init.FlashID));
  }
  
  /* Process locked */
  __HAL_LOCK(hqspi);
    
  if(hqspi->State == HAL_QSPI_STATE_RESET)
  { 
    /* Allocate lock resource and initialize it */
    hqspi->Lock = HAL_UNLOCKED;
     
    /* Init the low level hardware : GPIO, CLOCK */
    HAL_QSPI_MspInit(hqspi);
             
    /* Configure the default timeout for the QSPI memory access */
    HAL_QSPI_SetTimeout(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
  }
  
  /* Configure QSPI FIFO Threshold */
  MODIFY_REG(hqspi->Instance->CR, QUADSPI_CR_FTHRES, ((hqspi->Init.FifoThreshold - 1U) << 8U));

  /* Wait till BUSY flag reset */
  status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_BUSY, RESET, tickstart, hqspi->Timeout);

  if(status == HAL_OK)
  {
                
    /* Configure QSPI Clock Prescaler and Sample Shift */
    MODIFY_REG(hqspi->Instance->CR,(QUADSPI_CR_PRESCALER | QUADSPI_CR_SSHIFT | QUADSPI_CR_FSEL | QUADSPI_CR_DFM), ((hqspi->Init.ClockPrescaler << 24U)| hqspi->Init.SampleShifting | hqspi->Init.FlashID| hqspi->Init.DualFlash ));
        
    /* Configure QSPI Flash Size, CS High Time and Clock Mode */
    MODIFY_REG(hqspi->Instance->DCR, (QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE), 
               ((hqspi->Init.FlashSize << 16U) | hqspi->Init.ChipSelectHighTime | hqspi->Init.ClockMode));
    
    /* Enable the QSPI peripheral */
    __HAL_QSPI_ENABLE(hqspi);
  
    /* Set QSPI error code to none */
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;  

    /* Initialize the QSPI state */
    hqspi->State = HAL_QSPI_STATE_READY;
  }
  
  /* Release Lock */
  __HAL_UNLOCK(hqspi);

  /* Return function status */
  return status;
}

/**
  * @brief DeInitializes the QSPI peripheral 
  * @param hqspi qspi handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_DeInit(QSPI_HandleTypeDef *hqspi)
{
  /* Check the QSPI handle allocation */
  if(hqspi == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hqspi);

  /* Disable the QSPI Peripheral Clock */
  __HAL_QSPI_DISABLE(hqspi);

  /* DeInit the low level hardware: GPIO, CLOCK, NVIC... */
  HAL_QSPI_MspDeInit(hqspi);

  /* Set QSPI error code to none */
  hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;

  /* Initialize the QSPI state */
  hqspi->State = HAL_QSPI_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hqspi);

  return HAL_OK;
}

/**
  * @brief QSPI MSP Init
  * @param hqspi QSPI handle
  * @retval None
  */
 __weak void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_QSPI_MspInit can be implemented in the user file
   */ 
}

/**
  * @brief QSPI MSP DeInit
  * @param hqspi QSPI handle
  * @retval None
  */
 __weak void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_QSPI_MspDeInit can be implemented in the user file
   */ 
}



/*==============================================================================
                      ##### IO operation functions #####
 ===============================================================================
       [..]
    This subsection provides a set of functions allowing to :
      (+) Handle the interrupts.
      (+) Handle the command sequence.
      (+) Transmit data in blocking, interrupt or DMA mode.
      (+) Receive data in blocking, interrupt or DMA mode.
      (+) Manage the auto-polling functional mode.
      (+) Manage the memory-mapped functional mode.
*/

/**
  * @brief This function handles QSPI interrupt request.
  * @param hqspi QSPI handle
  * @retval None.
  */
void HAL_QSPI_IRQHandler(QSPI_HandleTypeDef *hqspi)
{
  __IO uint32_t *data_reg;
  uint32_t flag = READ_REG(hqspi->Instance->SR);
  uint32_t itsource = READ_REG(hqspi->Instance->CR);

  /* QSPI Fifo Threshold interrupt occurred ----------------------------------*/
  if(((flag & QSPI_FLAG_FT)!= RESET) && ((itsource & QSPI_IT_FT)!= RESET))
  {
    data_reg = &hqspi->Instance->DR;

    if(hqspi->State == HAL_QSPI_STATE_BUSY_INDIRECT_TX)
    {
      /* Transmission process */
      while(__HAL_QSPI_GET_FLAG(hqspi, QSPI_FLAG_FT) != 0U)
      {
        if (hqspi->TxXferCount > 0U)
        {
          /* Fill the FIFO until it is full */
          *(__IO uint8_t *)data_reg = *hqspi->pTxBuffPtr++;
          hqspi->TxXferCount--;
        }
        else
        {
          /* No more data available for the transfer */
          /* Disable the QSPI FIFO Threshold Interrupt */
          __HAL_QSPI_DISABLE_IT(hqspi, QSPI_IT_FT);
          break;
        }
      }
    }
    else if(hqspi->State == HAL_QSPI_STATE_BUSY_INDIRECT_RX)
    {
#if QSPI_INTERRUPT_FT_ENABLE			
			FTCplt++;
#endif					
      /* Receiving Process */
      while(__HAL_QSPI_GET_FLAG(hqspi, QSPI_FLAG_FT) != 0U)
      {
        if (hqspi->RxXferCount > 0U)
        {
          /* Read the FIFO until it is empty */
          *hqspi->pRxBuffPtr++ = *(__IO uint8_t *)data_reg;
          hqspi->RxXferCount--;
        }
        else
        {
          /* All data have been received for the transfer */
          /* Disable the QSPI FIFO Threshold Interrupt */
          __HAL_QSPI_DISABLE_IT(hqspi, QSPI_IT_FT);
          break;
        }
      }
    }
    
    /* FIFO Threshold callback */
    HAL_QSPI_FifoThresholdCallback(hqspi);
  }

  /* QSPI Transfer Complete interrupt occurred -------------------------------*/
  else if(((flag & QSPI_FLAG_TC)!= RESET) && ((itsource & QSPI_IT_TC)!= RESET))
  {
    /* Clear interrupt */
    WRITE_REG(hqspi->Instance->FCR, QSPI_FLAG_TC);

    /* Disable the QSPI FIFO Threshold, Transfer Error and Transfer complete Interrupts */
    __HAL_QSPI_DISABLE_IT(hqspi, QSPI_IT_TC | QSPI_IT_TE | QSPI_IT_FT);
    
    /* Transfer complete callback */
    if(hqspi->State == HAL_QSPI_STATE_BUSY_INDIRECT_TX)
    {
      if ((hqspi->Instance->CR & QUADSPI_CR_DMAEN)!= RESET)
      {
        /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
        CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
        
        /* Disable the DMA channel */
        __HAL_DMA_DISABLE(hqspi->hdma);
      }

      /* Clear Busy bit */
      HAL_QSPI_Abort_IT(hqspi);
      
      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;

      /* TX Complete callback */
      HAL_QSPI_TxCpltCallback(hqspi);
    }
    else if(hqspi->State == HAL_QSPI_STATE_BUSY_INDIRECT_RX)
    {
      if ((hqspi->Instance->CR & QUADSPI_CR_DMAEN)!= RESET)
      {
        /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
        CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
        
        /* Disable the DMA channel */
        __HAL_DMA_DISABLE(hqspi->hdma);
      }
      else
      {
        data_reg = &hqspi->Instance->DR;
        while(READ_BIT(hqspi->Instance->SR, QUADSPI_SR_FLEVEL) != 0U)
        {
          if (hqspi->RxXferCount > 0U)
          {
            /* Read the last data received in the FIFO until it is empty */
            *hqspi->pRxBuffPtr++ = *(__IO uint8_t *)data_reg;
            hqspi->RxXferCount--;
          }
          else
          {
            /* All data have been received for the transfer */
            break;
          }
        }
      }
      /* Workaround - Extra data written in the FIFO at the end of a read transfer */
      HAL_QSPI_Abort_IT(hqspi);
      
      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;

      /* RX Complete callback */
      HAL_QSPI_RxCpltCallback(hqspi);
    }
    else if(hqspi->State == HAL_QSPI_STATE_BUSY)
    {
      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;

      /* Command Complete callback */
      HAL_QSPI_CmdCpltCallback(hqspi);
    }
    else if(hqspi->State == HAL_QSPI_STATE_ABORT)
    {
      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;

      if (hqspi->ErrorCode == HAL_QSPI_ERROR_NONE)
      {
        /* Abort called by the user */

        /* Abort Complete callback */
        HAL_QSPI_AbortCpltCallback(hqspi);
      }
      else 
      {
        /* Abort due to an error (eg :  DMA error) */

        /* Error callback */
        HAL_QSPI_ErrorCallback(hqspi);
      }
    }
  }

  /* QSPI Status Match interrupt occurred ------------------------------------*/
  else if(((flag & QSPI_FLAG_SM)!= RESET) && ((itsource & QSPI_IT_SM)!= RESET))
  {
    /* Clear interrupt */
    WRITE_REG(hqspi->Instance->FCR, QSPI_FLAG_SM);
   
    /* Check if the automatic poll mode stop is activated */
    if(READ_BIT(hqspi->Instance->CR, QUADSPI_CR_APMS) != 0U)
    {
      /* Disable the QSPI Transfer Error and Status Match Interrupts */
      __HAL_QSPI_DISABLE_IT(hqspi, (QSPI_IT_SM | QSPI_IT_TE));

      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;
    }

    /* Status match callback */
    HAL_QSPI_StatusMatchCallback(hqspi);
  }

  /* QSPI Transfer Error interrupt occurred ----------------------------------*/
  else if(((flag & QSPI_FLAG_TE)!= RESET) && ((itsource & QSPI_IT_TE)!= RESET))
  {
    /* Clear interrupt */
    WRITE_REG(hqspi->Instance->FCR, QSPI_FLAG_TE);
    
    /* Disable all the QSPI Interrupts */
    __HAL_QSPI_DISABLE_IT(hqspi, QSPI_IT_SM | QSPI_IT_TC | QSPI_IT_TE | QSPI_IT_FT);

    /* Set error code */
    hqspi->ErrorCode |= HAL_QSPI_ERROR_TRANSFER;
    
    if ((hqspi->Instance->CR & QUADSPI_CR_DMAEN)!= RESET)
    {
      /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
      CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
      
      /* Disable the DMA channel */
      hqspi->hdma->XferAbortCallback = QSPI_DMAAbortCplt;
      HAL_DMA_Abort_IT(hqspi->hdma);
    }
    else
    {
      /* Change state of QSPI */
      hqspi->State = HAL_QSPI_STATE_READY;
      
      /* Error callback */
      HAL_QSPI_ErrorCallback(hqspi);
    }
  }

  /* QSPI Timeout interrupt occurred -----------------------------------------*/
  else if(((flag & QSPI_FLAG_TO)!= RESET) && ((itsource & QSPI_IT_TO)!= RESET))
  {
    /* Clear interrupt */
    WRITE_REG(hqspi->Instance->FCR, QSPI_FLAG_TO);
    
    /* Time out callback */
    HAL_QSPI_TimeOutCallback(hqspi);
  }
}

/**
  * @brief Sets the command configuration. 
  * @param hqspi QSPI handle
  * @param cmd  structure that contains the command configuration information
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Command(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t tickstart = HAL_GetTick();
  
  /* Check the parameters */
  assert_param(IS_QSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_QSPI_INSTRUCTION(cmd->Instruction));
  }

  assert_param(IS_QSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != QSPI_ADDRESS_NONE)
  {
    assert_param(IS_QSPI_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_QSPI_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_QSPI_ALTERNATE_BYTES_SIZE(cmd->AlternateBytesSize));
  }

  assert_param(IS_QSPI_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_QSPI_DATA_MODE(cmd->DataMode));

  assert_param(IS_QSPI_DDR_MODE(cmd->DdrMode));
  assert_param(IS_QSPI_DDR_HHC(cmd->DdrHoldHalfCycle));
  assert_param(IS_QSPI_SIOO_MODE(cmd->SIOOMode));
  
  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    /* Update QSPI state */
    hqspi->State = HAL_QSPI_STATE_BUSY;   
    
    /* Wait till BUSY flag reset */
    status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_BUSY, RESET, tickstart, Timeout);
    
    if (status == HAL_OK)
    {
      /* Call the configuration function */
      QSPI_Config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);
      
      if (cmd->DataMode == QSPI_DATA_NONE)
      {
        /* When there is no data phase, the transfer start as soon as the configuration is done 
        so wait until TC flag is set to go back in idle state */
        status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
          
          /* Update QSPI state */
          hqspi->State = HAL_QSPI_STATE_READY; 
        }
	          
      }
      else
      {
        /* Update QSPI state */
        hqspi->State = HAL_QSPI_STATE_READY;   
      }
    }
  }
  else
  {
    status = HAL_BUSY;   
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hqspi);

  /* Return function status */
  return status;
}

/**
  * @brief Sets the command configuration in interrupt mode. 
  * @param hqspi QSPI handle
  * @param cmd  structure that contains the command configuration information
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Command_IT(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd)
{
  __IO uint32_t count = 0U;
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_QSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_QSPI_INSTRUCTION(cmd->Instruction));
  }

  assert_param(IS_QSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != QSPI_ADDRESS_NONE)
  {
    assert_param(IS_QSPI_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_QSPI_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_QSPI_ALTERNATE_BYTES_SIZE(cmd->AlternateBytesSize));
  }

  assert_param(IS_QSPI_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_QSPI_DATA_MODE(cmd->DataMode));

  assert_param(IS_QSPI_DDR_MODE(cmd->DdrMode));
  assert_param(IS_QSPI_DDR_HHC(cmd->DdrHoldHalfCycle));
  assert_param(IS_QSPI_SIOO_MODE(cmd->SIOOMode));
  
  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    /* Update QSPI state */
    hqspi->State = HAL_QSPI_STATE_BUSY;   
    
    /* Wait till BUSY flag reset */
   count = (hqspi->Timeout) * (SystemCoreClock / 16U / 1000U);
   do 
   {
     if (count-- == 0U)
     { 
        hqspi->State     = HAL_QSPI_STATE_ERROR;
        hqspi->ErrorCode |= HAL_QSPI_ERROR_TIMEOUT;
        status = HAL_TIMEOUT;
     }
   } 
   while ((__HAL_QSPI_GET_FLAG(hqspi, QSPI_FLAG_BUSY)) != RESET);
   
    if (status == HAL_OK)
    {
      if (cmd->DataMode == QSPI_DATA_NONE)
      {
        /* Clear interrupt */
        __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TE | QSPI_FLAG_TC);
      }
      
      /* Call the configuration function */
      QSPI_Config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);
      
      if (cmd->DataMode == QSPI_DATA_NONE)
      {
        /* When there is no data phase, the transfer start as soon as the configuration is done 
        so activate TC and TE interrupts */
        /* Process unlocked */
        __HAL_UNLOCK(hqspi);

        /* Enable the QSPI Transfer Error Interrupt */
        __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TE | QSPI_IT_TC);
      }
      else
      {
        /* Update QSPI state */
        hqspi->State = HAL_QSPI_STATE_READY;   

        /* Process unlocked */
        __HAL_UNLOCK(hqspi);
      }
    }
    else
    {
      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY;   

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }
  
  /* Return function status */
  return status;
}
//uint32_t SR=0;

/**
  * @brief Transmit an amount of data in blocking mode. 
  * @param hqspi QSPI handle
  * @param pData pointer to data buffer
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
//uint32_t fifo_cnt=0;  
HAL_StatusTypeDef HAL_QSPI_Transmit(QSPI_HandleTypeDef *hqspi, uint8_t *pData, uint32_t Timeout)
{
   HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();
  __IO uint32_t *data_reg = &hqspi->Instance->DR;

  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;

    if(pData != NULL )
    {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_TX;
      
      /* Configure counters and size of the handle */
      hqspi->TxXferCount = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->TxXferSize = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->pTxBuffPtr = pData;
    
      /* Configure QSPI: CCR register with functional as indirect write */
      MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);
//	  fifo_cnt=0;  

      while(hqspi->TxXferCount > 0U)
      {
#if QSPI_FIFO_LEVEL_TEST_ENABLE				
		uint32_t sr_data0=hqspi->Instance->SR;

		if(((sr_data0 & 0x4)==0x0)&&(SR==0x0))
		{
			SR= sr_data0;
		}

	        /* Wait until FT flag is set to send data */
//		uint32_t sr_data=hqspi->Instance->SR;
		if((sr_data0&0x4)==0x0)
			fifo_cnt++;			

#endif
        status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_FT, SET, tickstart, Timeout);

        if (status != HAL_OK)
        { 
          break;
        }

        *(__IO uint8_t *)data_reg = *hqspi->pTxBuffPtr++;
        hqspi->TxXferCount--;
	
      }
    
      if (status == HAL_OK)
      {
        /* Wait until TC flag is set to go back in idle state */
        status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Clear Transfer Complete bit */
          __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
          
          /* Clear Busy bit */
          status = HAL_QSPI_Abort(hqspi);

        }
      }
    
      /* Update QSPI state */

      hqspi->State = HAL_QSPI_STATE_READY;    
    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_BUSY;
  }

  /* Process unlocked */
  __HAL_UNLOCK(hqspi);

  return status;
}


/**
  * @brief Receive an amount of data in blocking mode 
  * @param hqspi QSPI handle
  * @param pData pointer to data buffer
  * @param Timeout  Time out duration
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Receive(QSPI_HandleTypeDef *hqspi, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();
  uint32_t addr_reg = READ_REG(hqspi->Instance->AR);
  __IO uint32_t *data_reg = &hqspi->Instance->DR;

  /* Process locked */
  __HAL_LOCK(hqspi);
  
  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    if(pData != NULL )
    {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_RX;
    
      /* Configure counters and size of the handle */
      hqspi->RxXferCount = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->RxXferSize = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->pRxBuffPtr = pData;

      /* Configure QSPI: CCR register with functional as indirect read */
      MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Start the transfer by re-writing the address in AR register */
     WRITE_REG(hqspi->Instance->AR, addr_reg);

#if QSPI_FIFO_LEVEL_TEST_ENABLE

		while(1)
		{
			if(((hqspi->Instance->SR & 0x2)!=0x2)&&((hqspi->Instance->SR & 0x4)==0x4))
			{
				SR= hqspi->Instance->SR;
				break;
			}
		}
#endif		

      while(hqspi->RxXferCount > 0U)
      {
        /* Wait until FT or TC flag is set to read received data */
        status = QSPI_WaitFlagStateUntilTimeout(hqspi, (QSPI_FLAG_FT | QSPI_FLAG_TC), SET, tickstart, Timeout);

        if  (status != HAL_OK)
        { 
          break;
        }

        *hqspi->pRxBuffPtr++ = *(__IO uint8_t *)data_reg;
        hqspi->RxXferCount--;
      }
    
      if (status == HAL_OK)
      {
        /* Wait until TC flag is set to go back in idle state */
        status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_TC, SET, tickstart, Timeout);

        if  (status == HAL_OK)
        {
          /* Clear Transfer Complete bit */
          __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
          
         /* Workaround - Extra data written in the FIFO at the end of a read transfer */
         status = HAL_QSPI_Abort(hqspi);
		
        }
      }

      /* Update QSPI state */
      hqspi->State = HAL_QSPI_STATE_READY;    
    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_BUSY;
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hqspi);

  return status;
}

/**
  * @brief  Send an amount of data in interrupt mode 
  * @param  hqspi QSPI handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Transmit_IT(QSPI_HandleTypeDef *hqspi, uint8_t *pData)
{  
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    if(pData != NULL )
    {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_TX;

      /* Configure counters and size of the handle */
      hqspi->TxXferCount = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->TxXferSize = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->pTxBuffPtr = pData;
    
      /* Configure QSPI: CCR register with functional as indirect write */
      MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);
    
      /* Clear interrupt */
      __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TE | QSPI_FLAG_TC);

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
      
      /* Enable the QSPI transfer error, FIFO threshold and transfer complete Interrupts */
      __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TE | QSPI_IT_FT | QSPI_IT_TC);
      
    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY;

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }

  return status;
}

/**
  * @brief  Receive an amount of data in no-blocking mode with Interrupt
  * @param  hqspi QSPI handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Receive_IT(QSPI_HandleTypeDef *hqspi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t addr_reg = READ_REG(hqspi->Instance->AR);
  
  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    if(pData != NULL )
    {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_RX;
    
      /* Configure counters and size of the handle */
      hqspi->RxXferCount = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->RxXferSize = READ_REG(hqspi->Instance->DLR) + 1U;
      hqspi->pRxBuffPtr = pData;

      /* Configure QSPI: CCR register with functional as indirect read */
      MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Start the transfer by re-writing the address in AR register */
      WRITE_REG(hqspi->Instance->AR, addr_reg);

      /* Clear interrupt */
      __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TE | QSPI_FLAG_TC);

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);

      /* Enable the QSPI transfer error, FIFO threshold and transfer complete Interrupts */
#if QSPI_INTERRUPT_FT_ENABLE
	  __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_FT);
#else
      __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TE | QSPI_IT_FT | QSPI_IT_TC);
#endif

    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY;   

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }

  return status;
}

/**
  * @brief  Sends an amount of data in non blocking mode with DMA. 
  * @param  hqspi QSPI handle
  * @param  pData pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @note   If DMA peripheral access is configured as halfword, the number 
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number 
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Transmit_DMA(QSPI_HandleTypeDef *hqspi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t *tmp;
  uint32_t data_size = (READ_REG(hqspi->Instance->DLR) + 1U);

  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    /* Clear the error code */                
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;

    if(pData != NULL ) 
    {
      /* Configure counters of the handle */
      if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_BYTE)
      {
        hqspi->TxXferCount = data_size;
      }
      else if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD)
      {
        if (((data_size % 2U) != 0U) || ((hqspi->Init.FifoThreshold % 2U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on halfword 
          => no transfer possible with DMA peripheral access configured as halfword */
          hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;

          /* Process unlocked */
          __HAL_UNLOCK(hqspi);
        }
        else
        {
          hqspi->TxXferCount = (data_size >> 1);
        }
      }
      else if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD)
      {
        if (((data_size % 4U) != 0U) || ((hqspi->Init.FifoThreshold % 4U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on word 
          => no transfer possible with DMA peripheral access configured as word */
          hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;

          /* Process unlocked */
          __HAL_UNLOCK(hqspi);
        }
        else
        {
          hqspi->TxXferCount = (data_size >> 2U);
        }
      }

      if (status == HAL_OK)
      {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_TX;

      /* Clear interrupt */
      __HAL_QSPI_CLEAR_FLAG(hqspi, (QSPI_FLAG_TE | QSPI_FLAG_TC));

      /* Configure size and pointer of the handle */
      hqspi->TxXferSize = hqspi->TxXferCount;
      hqspi->pTxBuffPtr = pData;

      /* Configure QSPI: CCR register with functional mode as indirect write */
      MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

      /* Set the QSPI DMA transfer complete callback */
      hqspi->hdma->XferCpltCallback = QSPI_DMATxCplt;

      /* Set the QSPI DMA Half transfer complete callback */
      hqspi->hdma->XferHalfCpltCallback = QSPI_DMATxHalfCplt;

      /* Set the DMA error callback */
      hqspi->hdma->XferErrorCallback = QSPI_DMAError;

      /* Clear the DMA abort callback */
      hqspi->hdma->XferAbortCallback = NULL;

      /* Configure the direction of the DMA */
      hqspi->hdma->Init.Direction = DMA_MEMORY_TO_PERIPH;

      /* Update direction mode bit */
      MODIFY_REG(hqspi->hdma->Instance->CCR, DMA_CCR_DIR, hqspi->hdma->Init.Direction);

      /* Enable the QSPI transmit DMA Channel */
      tmp = (uint32_t*)&pData;
      HAL_DMA_Start_IT(hqspi->hdma, *(uint32_t*)tmp, (uint32_t)&hqspi->Instance->DR, hqspi->TxXferSize);

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);

      /* Enable the QSPI transfer error Interrupt */
      __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TE);

      /* Enable the DMA transfer by setting the DMAEN bit in the QSPI CR register */
      SET_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
      }
    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;

      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY;   

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }

  return status;
}

/**
  * @brief  Receives an amount of data in non blocking mode with DMA. 
  * @param  hqspi QSPI handle
  * @param  pData pointer to data buffer.
  * @note   This function is used only in Indirect Read Mode
  * @note   If DMA peripheral access is configured as halfword, the number 
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number 
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_Receive_DMA(QSPI_HandleTypeDef *hqspi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t *tmp;
  uint32_t addr_reg = READ_REG(hqspi->Instance->AR);
  uint32_t data_size = (READ_REG(hqspi->Instance->DLR) + 1U);

  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;

    if(pData != NULL ) 
    {
      /* Configure counters of the handle */
      if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_BYTE)
      {
        hqspi->RxXferCount = data_size;
      }
      else if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD)
      {
        if (((data_size % 2U) != 0U) || ((hqspi->Init.FifoThreshold % 2U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on halfword 
          => no transfer possible with DMA peripheral access configured as halfword */
          hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;

          /* Process unlocked */
          __HAL_UNLOCK(hqspi);
        }
        else
        {
          hqspi->RxXferCount = (data_size >> 1U);
        }
      }
      else if (hqspi->hdma->Init.PeriphDataAlignment == DMA_PDATAALIGN_WORD)
      {
        if (((data_size % 4U) != 0U) || ((hqspi->Init.FifoThreshold % 4U) != 0U))
        {
          /* The number of data or the fifo threshold is not aligned on word 
          => no transfer possible with DMA peripheral access configured as word */
          hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
          status = HAL_ERROR;

          /* Process unlocked */
          __HAL_UNLOCK(hqspi);
        }
        else
        {
          hqspi->RxXferCount = (data_size >> 2U);
        }
      }

      if (status == HAL_OK)
      {
        /* Update state */
        hqspi->State = HAL_QSPI_STATE_BUSY_INDIRECT_RX;

        /* Clear interrupt */
        __HAL_QSPI_CLEAR_FLAG(hqspi, (QSPI_FLAG_TE | QSPI_FLAG_TC));

        /* Configure size and pointer of the handle */
        hqspi->RxXferSize = hqspi->RxXferCount;
        hqspi->pRxBuffPtr = pData;

        /* Set the QSPI DMA transfer complete callback */
        hqspi->hdma->XferCpltCallback = QSPI_DMARxCplt;

        /* Set the QSPI DMA Half transfer complete callback */
        hqspi->hdma->XferHalfCpltCallback = QSPI_DMARxHalfCplt;

        /* Set the DMA error callback */
        hqspi->hdma->XferErrorCallback = QSPI_DMAError;

        /* Clear the DMA abort callback */
        hqspi->hdma->XferAbortCallback = NULL;

        /* Configure the direction of the DMA */
        hqspi->hdma->Init.Direction = DMA_PERIPH_TO_MEMORY;

		MODIFY_REG(hqspi->hdma->Instance->CCR, DMA_CCR_DIR, hqspi->hdma->Init.Direction);

        /* Enable the DMA Channel */
        tmp = (uint32_t*)&pData;
        HAL_DMA_Start_IT(hqspi->hdma, (uint32_t)&hqspi->Instance->DR, *(uint32_t*)tmp, hqspi->RxXferSize);

        /* Configure QSPI: CCR register with functional as indirect read */
        MODIFY_REG(hqspi->Instance->CCR, QUADSPI_CCR_FMODE, QSPI_FUNCTIONAL_MODE_INDIRECT_READ);

        /* Start the transfer by re-writing the address in AR register */
        WRITE_REG(hqspi->Instance->AR, addr_reg);

        /* Process unlocked */
        __HAL_UNLOCK(hqspi);

        /* Enable the QSPI transfer error Interrupt */
        __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TE);

        /* Enable the DMA transfer by setting the DMAEN bit in the QSPI CR register */
        SET_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);

      }
    }
    else
    {
      hqspi->ErrorCode |= HAL_QSPI_ERROR_INVALID_PARAM;
      status = HAL_ERROR;

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY; 

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }

  return status;
}

/**
  * @brief  Configure the QSPI Automatic Polling Mode in blocking mode. 
  * @param  hqspi QSPI handle
  * @param  cmd structure that contains the command configuration information.
  * @param  cfg structure that contains the polling configuration information.
  * @param  Timeout  Time out duration
  * @note   This function is used only in Automatic Polling Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_AutoPolling(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t tickstart = HAL_GetTick();
  
  /* Check the parameters */
  assert_param(IS_QSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_QSPI_INSTRUCTION(cmd->Instruction));
  }
  
  assert_param(IS_QSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != QSPI_ADDRESS_NONE)
  {
    assert_param(IS_QSPI_ADDRESS_SIZE(cmd->AddressSize));
  }
  
  assert_param(IS_QSPI_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_QSPI_ALTERNATE_BYTES_SIZE(cmd->AlternateBytesSize));
  }
  
  assert_param(IS_QSPI_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_QSPI_DATA_MODE(cmd->DataMode));
  
  assert_param(IS_QSPI_DDR_MODE(cmd->DdrMode));
  assert_param(IS_QSPI_DDR_HHC(cmd->DdrHoldHalfCycle));
  assert_param(IS_QSPI_SIOO_MODE(cmd->SIOOMode));
  
  assert_param(IS_QSPI_INTERVAL(cfg->Interval));
  assert_param(IS_QSPI_STATUS_BYTES_SIZE(cfg->StatusBytesSize));
  assert_param(IS_QSPI_MATCH_MODE(cfg->MatchMode));
  
  /* Process locked */
  __HAL_LOCK(hqspi);
  
  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    /* Update state */
    hqspi->State = HAL_QSPI_STATE_BUSY_AUTO_POLLING;
    
    /* Wait till BUSY flag reset */
    status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_BUSY, RESET, tickstart, Timeout);
    
    if (status == HAL_OK)
    {
      /* Configure QSPI: PSMAR register with the status match value */
      WRITE_REG(hqspi->Instance->PSMAR, cfg->Match);
      
      /* Configure QSPI: PSMKR register with the status mask value */
      WRITE_REG(hqspi->Instance->PSMKR, cfg->Mask);
      
      /* Configure QSPI: PIR register with the interval value */
      WRITE_REG(hqspi->Instance->PIR, cfg->Interval);
      
      /* Configure QSPI: CR register with Match mode and Automatic stop enabled 
      (otherwise there will be an infinite loop in blocking mode) */
      MODIFY_REG(hqspi->Instance->CR, (QUADSPI_CR_PMM | QUADSPI_CR_APMS), 
               (cfg->MatchMode | QSPI_AUTOMATIC_STOP_ENABLE));
      
      /* Call the configuration function */
      cmd->NbData = cfg->StatusBytesSize;
      QSPI_Config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_AUTO_POLLING);
      
      /* Wait until SM flag is set to go back in idle state */
      status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_SM, SET, tickstart, Timeout);

      if (status == HAL_OK)
      {
        __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_SM);
        
        /* Update state */
        hqspi->State = HAL_QSPI_STATE_READY;
      }
    }
  }
  else
  {
    status = HAL_BUSY;   
  }
  /* Process unlocked */
  __HAL_UNLOCK(hqspi);
  
  /* Return function status */
  return status;  
}

/**
  * @brief  Configure the QSPI Automatic Polling Mode in non-blocking mode. 
  * @param  hqspi QSPI handle
  * @param  cmd structure that contains the command configuration information.
  * @param  cfg structure that contains the polling configuration information.
  * @note   This function is used only in Automatic Polling Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_AutoPolling_IT(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, QSPI_AutoPollingTypeDef *cfg)
{
  __IO uint32_t count = 0U;
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_QSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
    assert_param(IS_QSPI_INSTRUCTION(cmd->Instruction));
  }
  
  assert_param(IS_QSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != QSPI_ADDRESS_NONE)
  {
    assert_param(IS_QSPI_ADDRESS_SIZE(cmd->AddressSize));
  }
  
  assert_param(IS_QSPI_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_QSPI_ALTERNATE_BYTES_SIZE(cmd->AlternateBytesSize));
  }
  
  assert_param(IS_QSPI_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_QSPI_DATA_MODE(cmd->DataMode));
  
  assert_param(IS_QSPI_DDR_MODE(cmd->DdrMode));
  assert_param(IS_QSPI_DDR_HHC(cmd->DdrHoldHalfCycle));
  assert_param(IS_QSPI_SIOO_MODE(cmd->SIOOMode));
  
  assert_param(IS_QSPI_INTERVAL(cfg->Interval));
  assert_param(IS_QSPI_STATUS_BYTES_SIZE(cfg->StatusBytesSize));
  assert_param(IS_QSPI_MATCH_MODE(cfg->MatchMode));
  assert_param(IS_QSPI_AUTOMATIC_STOP(cfg->AutomaticStop));
  
  /* Process locked */
  __HAL_LOCK(hqspi);
  
  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    /* Update state */
    hqspi->State = HAL_QSPI_STATE_BUSY_AUTO_POLLING;

    /* Wait till BUSY flag reset */
    count = (hqspi->Timeout) * (SystemCoreClock / 16U / 1000U);
    do 
    {
      if (count-- == 0U)
      { 
        hqspi->State     = HAL_QSPI_STATE_ERROR;
        hqspi->ErrorCode |= HAL_QSPI_ERROR_TIMEOUT;
        status = HAL_TIMEOUT;
      }
    } 
    while ((__HAL_QSPI_GET_FLAG(hqspi, QSPI_FLAG_BUSY)) != RESET);
    
    if (status == HAL_OK)
    {
      /* Configure QSPI: PSMAR register with the status match value */
      WRITE_REG(hqspi->Instance->PSMAR, cfg->Match);
      
      /* Configure QSPI: PSMKR register with the status mask value */
      WRITE_REG(hqspi->Instance->PSMKR, cfg->Mask);
      
      /* Configure QSPI: PIR register with the interval value */
      WRITE_REG(hqspi->Instance->PIR, cfg->Interval);
      
      /* Configure QSPI: CR register with Match mode and Automatic stop mode */
      MODIFY_REG(hqspi->Instance->CR, (QUADSPI_CR_PMM | QUADSPI_CR_APMS), 
               (cfg->MatchMode | cfg->AutomaticStop));
      
      /* Clear interrupt */
      __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TE | QSPI_FLAG_SM);
      
      /* Call the configuration function */
      cmd->NbData = cfg->StatusBytesSize;
      QSPI_Config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_AUTO_POLLING);

      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
  
      /* Enable the QSPI Transfer Error and status match Interrupt */
      __HAL_QSPI_ENABLE_IT(hqspi, (QSPI_IT_SM | QSPI_IT_TE));

    }
    else
    {
      /* Process unlocked */
      __HAL_UNLOCK(hqspi);
    }
  }
  else
  {
    status = HAL_BUSY;   

    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
  }
  
  /* Return function status */
  return status;  
}

/**
  * @brief  Configure the Memory Mapped mode. 
  * @param  hqspi QSPI handle
  * @param  cmd structure that contains the command configuration information.
  * @param  cfg structure that contains the memory mapped configuration information.
  * @note   This function is used only in Memory mapped Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_MemoryMapped(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, QSPI_MemoryMappedTypeDef *cfg)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t tickstart = HAL_GetTick();
  
  /* Check the parameters */
  assert_param(IS_QSPI_INSTRUCTION_MODE(cmd->InstructionMode));
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
  assert_param(IS_QSPI_INSTRUCTION(cmd->Instruction));
  }

  assert_param(IS_QSPI_ADDRESS_MODE(cmd->AddressMode));
  if (cmd->AddressMode != QSPI_ADDRESS_NONE)
  {
    assert_param(IS_QSPI_ADDRESS_SIZE(cmd->AddressSize));
  }

  assert_param(IS_QSPI_ALTERNATE_BYTES_MODE(cmd->AlternateByteMode));
  if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
  {
    assert_param(IS_QSPI_ALTERNATE_BYTES_SIZE(cmd->AlternateBytesSize));
  }

  assert_param(IS_QSPI_DUMMY_CYCLES(cmd->DummyCycles));
  assert_param(IS_QSPI_DATA_MODE(cmd->DataMode));

  assert_param(IS_QSPI_DDR_MODE(cmd->DdrMode));
  assert_param(IS_QSPI_DDR_HHC(cmd->DdrHoldHalfCycle));
  assert_param(IS_QSPI_SIOO_MODE(cmd->SIOOMode));

  assert_param(IS_QSPI_TIMEOUT_ACTIVATION(cfg->TimeOutActivation));
  
  /* Process locked */
  __HAL_LOCK(hqspi);
  
  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    hqspi->ErrorCode = HAL_QSPI_ERROR_NONE;
    
    /* Update state */
    hqspi->State = HAL_QSPI_STATE_BUSY_MEM_MAPPED;
    
    /* Wait till BUSY flag reset */
    status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_BUSY, RESET, tickstart, hqspi->Timeout);
    
    if (status == HAL_OK)
    {
      /* Configure QSPI: CR register with timeout counter enable */
    MODIFY_REG(hqspi->Instance->CR, QUADSPI_CR_TCEN, cfg->TimeOutActivation);

    if (cfg->TimeOutActivation == QSPI_TIMEOUT_COUNTER_ENABLE)
      {
        assert_param(IS_QSPI_TIMEOUT_PERIOD(cfg->TimeOutPeriod));
        
        /* Configure QSPI: LPTR register with the low-power timeout value */
        WRITE_REG(hqspi->Instance->LPTR, cfg->TimeOutPeriod);
        
        /* Clear interrupt */
        __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TO);

        /* Enable the QSPI TimeOut Interrupt */
        __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TO);
      }
      
      /* Call the configuration function */
      QSPI_Config(hqspi, cmd, QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED);
    }
  }
  else
  {
    status = HAL_BUSY;   
  }

  /* Process unlocked */
  __HAL_UNLOCK(hqspi);
  
  /* Return function status */
  return status;  
}

/**
  * @brief  Transfer Error callbacks
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_QSPI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort completed callback.
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_AbortCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_QSPI_AbortCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Command completed callback.
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */

  
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_QSPI_CmdCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_QSPI_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
 __weak void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_QSPI_TxCpltCallback could be implemented in the user file
   */ 
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_RxHalfCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_QSPI_RxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hqspi QSPI handle
  * @retval None
  */
 __weak void HAL_QSPI_TxHalfCpltCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_QSPI_TxHalfCpltCallback could be implemented in the user file
   */ 
}

/**
  * @brief  FIFO Threshold callbacks
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_FifoThresholdCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_QSPI_FIFOThresholdCallback could be implemented in the user file
   */
}

/**
  * @brief  Status Match callbacks
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
    
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_QSPI_StatusMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Timeout callbacks
  * @param  hqspi QSPI handle
  * @retval None
  */
__weak void HAL_QSPI_TimeOutCallback(QSPI_HandleTypeDef *hqspi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hqspi);
  
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_QSPI_TimeOutCallback could be implemented in the user file
   */
}


/*=============================================================================
                  ##### Peripheral Control and State functions #####
 ===============================================================================  
    [..]
    This subsection provides a set of functions allowing to :
      (+) Check in run-time the state of the driver. 
      (+) Check the error code set during last operation.
      (+) Abort any operation.
*/

/**
  * @brief  Return the QSPI handle state.
  * @param  hqspi QSPI handle
  * @retval HAL state
  */
HAL_QSPI_StateTypeDef HAL_QSPI_GetState(QSPI_HandleTypeDef *hqspi)
{
  /* Return QSPI handle state */
  return hqspi->State;
}

/**
* @brief  Return the QSPI error code
* @param  hqspi QSPI handle
* @retval QSPI Error Code
*/
uint32_t HAL_QSPI_GetError(QSPI_HandleTypeDef *hqspi)
{
  return hqspi->ErrorCode;
}

/**
* @brief  Abort the current transmission
* @param  hqspi QSPI handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_QSPI_Abort(QSPI_HandleTypeDef *hqspi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();
  
  /* Check if the state is in one of the busy states */
  if ((hqspi->State & 0x2U) != 0U)
  {
    /* Process unlocked */
    __HAL_UNLOCK(hqspi);

    if ((hqspi->Instance->CR & QUADSPI_CR_DMAEN)!= RESET)
    {
      /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
      CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
      
      /* Abort DMA channel */
      status = HAL_DMA_Abort(hqspi->hdma);
      if(status != HAL_OK)
      {
        hqspi->ErrorCode |= HAL_QSPI_ERROR_DMA;
      }
    }  
    
    /* Configure QSPI: CR register with Abort request */
    SET_BIT(hqspi->Instance->CR, QUADSPI_CR_ABORT);

    /* Wait until TC flag is set to go back in idle state */
    status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_TC, SET, tickstart, hqspi->Timeout);

    if(status == HAL_OK)
    {
      __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
      
      /* Wait until BUSY flag is reset */
      status = QSPI_WaitFlagStateUntilTimeout(hqspi, QSPI_FLAG_BUSY, RESET, tickstart, hqspi->Timeout);
    }
    
    if (status == HAL_OK)
    {
      /* Update state */
      hqspi->State = HAL_QSPI_STATE_READY;
    }
  }

  return status;
}

/**
* @brief  Abort the current transmission (non-blocking function)
* @param  hqspi QSPI handle
* @retval HAL status
*/
HAL_StatusTypeDef HAL_QSPI_Abort_IT(QSPI_HandleTypeDef *hqspi)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check if the state is in one of the busy states */
  if ((hqspi->State & 0x2U) != 0U)
  {
    /* Process unlocked */
    __HAL_UNLOCK(hqspi);
    
    /* Update QSPI state */
    hqspi->State = HAL_QSPI_STATE_ABORT;   
    
    /* Disable all interrupts */
    __HAL_QSPI_DISABLE_IT(hqspi, (QSPI_IT_TO | QSPI_IT_SM | QSPI_IT_FT | QSPI_IT_TC | QSPI_IT_TE));
    
    if ((hqspi->Instance->CR & QUADSPI_CR_DMAEN)!= RESET)
    {
      /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
      CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
      
      /* Abort DMA channel */
      hqspi->hdma->XferAbortCallback = QSPI_DMAAbortCplt;
      HAL_DMA_Abort_IT(hqspi->hdma);
    }  
    else
    {
      /* Clear interrupt */
      __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
      
      /* Enable the QSPI Transfer Complete Interrupt */
      __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TC);
      
      /* Configure QSPI: CR register with Abort request */
      SET_BIT(hqspi->Instance->CR, QUADSPI_CR_ABORT);
    }
  }

  return status;
}

/** @brief Set QSPI timeout
  * @param  hqspi QSPI handle.
  * @param  Timeout Timeout for the QSPI memory access.
  * @retval None
  */
void HAL_QSPI_SetTimeout(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
  hqspi->Timeout = Timeout;
}

/** @brief Set QSPI Fifo threshold.
  * @param  hqspi QSPI handle.
  * @param  Threshold Threshold of the Fifo (value between 1 and 16).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_QSPI_SetFifoThreshold(QSPI_HandleTypeDef *hqspi, uint32_t Threshold)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hqspi);

  if(hqspi->State == HAL_QSPI_STATE_READY)
  {
    /* Synchronize init structure with new FIFO threshold value */
    hqspi->Init.FifoThreshold = Threshold;
    
    /* Configure QSPI FIFO Threshold */
    MODIFY_REG(hqspi->Instance->CR, QUADSPI_CR_FTHRES, 
               ((hqspi->Init.FifoThreshold - 1U) << QUADSPI_CR_FTHRES_Pos));
  }
  else
  {
    status = HAL_BUSY;   
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hqspi);

  /* Return function status */
  return status;
}

/** @brief Get QSPI Fifo threshold.
  * @param  hqspi QSPI handle.
  * @retval Fifo threshold (value between 1 and 16)
  */
uint32_t HAL_QSPI_GetFifoThreshold(QSPI_HandleTypeDef *hqspi)
{
  return ((READ_BIT(hqspi->Instance->CR, QUADSPI_CR_FTHRES) >> QUADSPI_CR_FTHRES_Pos) + 1U);
}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DMA QSPI receive process complete callback. 
  * @param  hdma DMA handle
  * @retval None
  */
static void QSPI_DMARxCplt(DMA_HandleTypeDef *hdma)  
{
  QSPI_HandleTypeDef* hqspi = ( QSPI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  hqspi->RxXferCount = 0U;
  
  /* Enable the QSPI transfer complete Interrupt */
  __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TC);
}

/**
  * @brief  DMA QSPI transmit process complete callback. 
  * @param  hdma DMA handle
  * @retval None
  */
static void QSPI_DMATxCplt(DMA_HandleTypeDef *hdma)     
{
  QSPI_HandleTypeDef* hqspi = ( QSPI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  hqspi->TxXferCount = 0U;
  
  /* Enable the QSPI transfer complete Interrupt */
  __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TC);
}

/**
  * @brief  DMA QSPI receive process half complete callback 
  * @param  hdma  DMA handle
  * @retval None
  */
static void QSPI_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
  QSPI_HandleTypeDef* hqspi = (QSPI_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

  HAL_QSPI_RxHalfCpltCallback(hqspi); 
}

/**
  * @brief  DMA QSPI transmit process half complete callback 
  * @param  hdma  DMA handle
  * @retval None
  */
static void QSPI_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  QSPI_HandleTypeDef* hqspi = (QSPI_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

  HAL_QSPI_TxHalfCpltCallback(hqspi);
}

/**
  * @brief  DMA QSPI communication error callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void QSPI_DMAError(DMA_HandleTypeDef *hdma)   
{
  QSPI_HandleTypeDef* hqspi = ( QSPI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  
  /* if DMA error is FIFO error ignore it */
//  if(HAL_DMA_GetError(hdma) != HAL_DMA_ERROR_FE)
  {
    hqspi->RxXferCount = 0U;
    hqspi->TxXferCount = 0U;
    hqspi->ErrorCode   |= HAL_QSPI_ERROR_DMA;
    
    /* Disable the DMA transfer by clearing the DMAEN bit in the QSPI CR register */
    CLEAR_BIT(hqspi->Instance->CR, QUADSPI_CR_DMAEN);
    
    /* Abort the QSPI */
    HAL_QSPI_Abort_IT(hqspi);
  }
}

/**
  * @brief  DMA QSPI abort complete callback.
  * @param  hdma DMA handle
  * @retval None
  */
static void QSPI_DMAAbortCplt(DMA_HandleTypeDef *hdma)   
{
  QSPI_HandleTypeDef* hqspi = ( QSPI_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

  hqspi->RxXferCount = 0U;
  hqspi->TxXferCount = 0U;

  if(hqspi->State == HAL_QSPI_STATE_ABORT)
  {
    /* DMA Abort called by QSPI abort */
    /* Clear interrupt */
    __HAL_QSPI_CLEAR_FLAG(hqspi, QSPI_FLAG_TC);
    
    /* Enable the QSPI Transfer Complete Interrupt */
    __HAL_QSPI_ENABLE_IT(hqspi, QSPI_IT_TC);
    
    /* Configure QSPI: CR register with Abort request */
    SET_BIT(hqspi->Instance->CR, QUADSPI_CR_ABORT);
  }
  else
  {
    /* DMA Abort called due to a transfer error interrupt */
    /* Change state of QSPI */
    hqspi->State = HAL_QSPI_STATE_READY;
    
    /* Error callback */
    HAL_QSPI_ErrorCallback(hqspi);
  }
}
/**
  * @brief  Wait for a flag state until timeout.
  * @param  hqspi QSPI handle
  * @param  Flag Flag checked
  * @param  State Value of the flag expected
  * @param  Timeout Duration of the time out
  * @param  tickstart tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef QSPI_WaitFlagStateUntilTimeout(QSPI_HandleTypeDef *hqspi, uint32_t Flag, 
                                                        FlagStatus State, uint32_t tickstart, uint32_t Timeout)
{
  /* Wait until flag is in expected state */    
  while((FlagStatus)(__HAL_QSPI_GET_FLAG(hqspi, Flag)) != State)
//	while(State1 != State)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U) || ((HAL_GetTick() - tickstart) > Timeout))
      {
        hqspi->State     = HAL_QSPI_STATE_ERROR;
        hqspi->ErrorCode |= HAL_QSPI_ERROR_TIMEOUT;
        
        return HAL_ERROR;
      }
    }
		
  }
  return HAL_OK;
}

/**
  * @brief  Configure the communication registers.
  * @param  hqspi QSPI handle
  * @param  cmd structure that contains the command configuration information
  * @param  FunctionalMode functional mode to configured
  *           This parameter can be one of the following values:
  *            @arg QSPI_FUNCTIONAL_MODE_INDIRECT_WRITE: Indirect write mode
  *            @arg QSPI_FUNCTIONAL_MODE_INDIRECT_READ: Indirect read mode
  *            @arg QSPI_FUNCTIONAL_MODE_AUTO_POLLING: Automatic polling mode
  *            @arg QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED: Memory-mapped mode
  * @retval None
  */
static void QSPI_Config(QSPI_HandleTypeDef *hqspi, QSPI_CommandTypeDef *cmd, uint32_t FunctionalMode)
{
  assert_param(IS_QSPI_FUNCTIONAL_MODE(FunctionalMode));

  if ((cmd->DataMode != QSPI_DATA_NONE) && (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED))
  {
    /* Configure QSPI: DLR register with the number of data to read or write */
    WRITE_REG(hqspi->Instance->DLR, (cmd->NbData - 1U));
  }
      
  if (cmd->InstructionMode != QSPI_INSTRUCTION_NONE)
  {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
    {
      /* Configure QSPI: ABR register with alternate bytes value */
      WRITE_REG(hqspi->Instance->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE)
      {
        /*---- Command with instruction, address and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateBytesSize |
                                         cmd->AlternateByteMode | cmd->AddressSize | cmd->AddressMode |
                                         cmd->InstructionMode | cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
        {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(hqspi->Instance->AR, cmd->Address);
        }
      }
      else
      {
        /*---- Command with instruction and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateBytesSize |
                                         cmd->AlternateByteMode | cmd->AddressMode | cmd->InstructionMode | 
                                         cmd->Instruction | FunctionalMode));
      }
    }
    else
    {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE)
      {
        /*---- Command with instruction and address ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateByteMode | 
                                         cmd->AddressSize | cmd->AddressMode | cmd->InstructionMode | 
                                         cmd->Instruction | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
        {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(hqspi->Instance->AR, cmd->Address);
        }
      }
      else
      {
        /*---- Command with only instruction ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateByteMode | 
                                         cmd->AddressMode | cmd->InstructionMode | cmd->Instruction  | 
                                         FunctionalMode));
      }
    }
  }
  else
  {
    if (cmd->AlternateByteMode != QSPI_ALTERNATE_BYTES_NONE)
    {
      /* Configure QSPI: ABR register with alternate bytes value */
      WRITE_REG(hqspi->Instance->ABR, cmd->AlternateBytes);

      if (cmd->AddressMode != QSPI_ADDRESS_NONE)
      {
        /*---- Command with address and alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateBytesSize |
                                         cmd->AlternateByteMode | cmd->AddressSize | cmd->AddressMode |
                                         cmd->InstructionMode | FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
        {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(hqspi->Instance->AR, cmd->Address);
        }
      }
      else
      {
        /*---- Command with only alternate bytes ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateBytesSize |
                                         cmd->AlternateByteMode | cmd->AddressMode | cmd->InstructionMode | 
                                         FunctionalMode));
      }
    }
    else
    {
      if (cmd->AddressMode != QSPI_ADDRESS_NONE)
      {
        /*---- Command with only address ----*/
        /* Configure QSPI: CCR register with all communications parameters */
        WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                         cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateByteMode | 
                                         cmd->AddressSize | cmd->AddressMode | cmd->InstructionMode | 
                                         FunctionalMode));

        if (FunctionalMode != QSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
        {
          /* Configure QSPI: AR register with address value */
          WRITE_REG(hqspi->Instance->AR, cmd->Address);
        }
      }
      else
      {
        /*---- Command with only data phase ----*/
        if (cmd->DataMode != QSPI_DATA_NONE)
        {
          /* Configure QSPI: CCR register with all communications parameters */
          WRITE_REG(hqspi->Instance->CCR, (cmd->DdrMode | cmd->DdrHoldHalfCycle | cmd->SIOOMode |
                                           cmd->DataMode | (cmd->DummyCycles << 18U) | cmd->AlternateByteMode | 
                                           cmd->AddressMode | cmd->InstructionMode | FunctionalMode));
        }
      }
    }
  }
}


#endif /* HAL_QSPI_MODULE_ENABLED */

