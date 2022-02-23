/**
  ******************************************************************************
  * @file    hk32f39a_hal_rng.h
  * @author  Rakan
  * @date    2019/10/30
  * @brief   RNG module driver.
  ******************************************************************************  
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_HAL_RNG_H
#define __HK32F39A_HAL_RNG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_hal_def.h"




/* Exported types ------------------------------------------------------------*/ 



/** @defgroup RNG_Exported_Types_Group1 RNG State Structure definition 
  * @{
  */
typedef enum
{
  HAL_RNG_STATE_RESET     = 0x00U,  /*!< RNG not yet initialized or disabled */
  HAL_RNG_STATE_READY     = 0x01U,  /*!< RNG initialized and ready for use   */
  HAL_RNG_STATE_BUSY      = 0x02U,  /*!< RNG internal process is ongoing     */ 
  HAL_RNG_STATE_TIMEOUT   = 0x03U,  /*!< RNG timeout state                   */
  HAL_RNG_STATE_ERROR     = 0x04U   /*!< RNG error state                     */
    
}HAL_RNG_StateTypeDef;

/** 
  * @}
  */

/** @defgroup RNG_Exported_Types_Group2 RNG Handle Structure definition   
  * @{
  */ 
typedef struct
{
  RNG_TypeDef                 *Instance;    /*!< Register base address   */ 
  
  HAL_LockTypeDef             Lock;         /*!< RNG locking object      */
  
  __IO HAL_RNG_StateTypeDef   State;        /*!< RNG communication state */
  
  uint32_t                    RandomNumber; /*!< Last Generated RNG Data */
  
}RNG_HandleTypeDef;

/** 
  * @}
  */

/**
  * @}
  */ 
   
/* Exported constants --------------------------------------------------------*/

/** @defgroup RNG_Exported_Constants RNG Exported Constants
  * @{
  */

/** @defgroup RNG_Exported_Constants_Group1 RNG Interrupt definition
  * @{
  */
#define RNG_IT_DRDY  RNG_SR_DRDY  /*!< Data Ready interrupt  */
#define RNG_IT_SEI   RNG_SR_SEIS  /*!< Seed error interrupt  */
/**
  * @}
  */

/** @defgroup RNG_Exported_Constants_Group2 RNG Flag definition
  * @{
  */
#define RNG_FLAG_DRDY   RNG_SR_DRDY  /*!< Data ready                 */
#define RNG_FLAG_SECS   RNG_SR_SECS  /*!< Seed error current status  */

#define RNG_IT_IE       RNG_CR_IE      /*!< RNG Interrupt enable */




/**
  * @}
  */

/**
  * @}
  */ 
  
/* Exported macros -----------------------------------------------------------*/

/** @defgroup RNG_Exported_Macros RNG Exported Macros
  * @{
  */

/** @brief Reset RNG handle state
  * @param  __HANDLE__ RNG Handle
  * @retval None
  */
#define __HAL_RNG_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_RNG_STATE_RESET)

/**
  * @brief  Enables the RNG peripheral.
  * @param  __HANDLE__ RNG Handle
  * @retval None
  */
#define __HAL_RNG_ENABLE(__HANDLE__) ((__HANDLE__)->Instance->CR |=  RNG_CR_RNGEN)

/**
  * @brief  Disables the RNG peripheral.
  * @param  __HANDLE__ RNG Handle
  * @retval None
  */
#define __HAL_RNG_DISABLE(__HANDLE__) ((__HANDLE__)->Instance->CR &= ~RNG_CR_RNGEN)

/**
  * @brief  Check the selected RNG flag status.
  * @param  __HANDLE__ RNG Handle
  * @param  __FLAG__ RNG flag
  *          This parameter can be one of the following values:
  *            @arg RNG_FLAG_DRDY: Data ready                
  *            @arg RNG_FLAG_SECS: Seed error current status 
  * @retval The new state of __FLAG__ (SET or RESET).
  */
#define __HAL_RNG_GET_FLAG(__HANDLE__, __FLAG__) (((__HANDLE__)->Instance->SR & (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clears the selected RNG flag status.
  * @param  __HANDLE__ RNG handle
  * @param  __FLAG__ RNG flag to clear  
  * @note   WARNING: This is a dummy macro for HAL code alignment,
  *         flags RNG_FLAG_DRDY, RNG_FLAG_CECS and RNG_FLAG_SECS are read-only.
  * @retval None
  */
#define __HAL_RNG_CLEAR_FLAG(__HANDLE__, __FLAG__)                      /* dummy  macro */

/**
  * @brief  Enables the RNG interrupts.
  * @param  __HANDLE__ RNG Handle
  * @retval None
  */
#define __HAL_RNG_ENABLE_IT(__HANDLE__) ((__HANDLE__)->Instance->CR |=  RNG_IT_IE)
    
/**
  * @brief  Disables the RNG interrupts.
  * @param  __HANDLE__ RNG Handle
  * @retval None
  */
#define __HAL_RNG_DISABLE_IT(__HANDLE__) ((__HANDLE__)->Instance->CR &= ~RNG_IT_IE)

/**
  * @brief  Checks whether the specified RNG interrupt has occurred or not.
  * @param  __HANDLE__ RNG Handle
  * @param  __INTERRUPT__ specifies the RNG interrupt status flag to check.
  *         This parameter can be one of the following values:
  *            @arg RNG_IT_DRDY: Data ready interrupt   
  *            @arg RNG_IT_SEI: Seed error interrupt
  * @retval The new state of __INTERRUPT__ (SET or RESET).
  */
#define __HAL_RNG_GET_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->SR & (__INTERRUPT__)) == (__INTERRUPT__))   

/**
  * @brief  Clear the RNG interrupt status flags.
  * @param  __HANDLE__ RNG Handle
  * @param  __INTERRUPT__ specifies the RNG interrupt status flag to clear.
  *          This parameter can be one of the following values:   
  *            @arg RNG_IT_SEI: Seed error interrupt
  * @note   RNG_IT_DRDY flag is read-only, reading RNG_DR register automatically clears RNG_IT_DRDY.          
  * @retval None
  */
#define __HAL_RNG_CLEAR_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->SR) = ~(__INTERRUPT__))

/**
  * @}
  */ 

/* Exported functions --------------------------------------------------------*/
/** @defgroup RNG_Exported_Functions RNG Exported Functions
  * @{
  */

/** @defgroup RNG_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */  
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng);
HAL_StatusTypeDef HAL_RNG_DeInit (RNG_HandleTypeDef *hrng);
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng);

/**
  * @}
  */ 

/** @defgroup RNG_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */
uint32_t HAL_RNG_GetRandomNumber(RNG_HandleTypeDef *hrng);    /* Obsolete, use HAL_RNG_GenerateRandomNumber() instead    */
uint32_t HAL_RNG_GetRandomNumber_IT(RNG_HandleTypeDef *hrng); /* Obsolete, use HAL_RNG_GenerateRandomNumber_IT() instead */

HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *hrng, uint32_t *random32bit);
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber_IT(RNG_HandleTypeDef *hrng);
uint32_t HAL_RNG_ReadLastRandomNumber(RNG_HandleTypeDef *hrng);

void HAL_RNG_IRQHandler(RNG_HandleTypeDef *hrng);
void HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng);
void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef* hrng, uint32_t random32bit);



HAL_RNG_StateTypeDef HAL_RNG_GetState(RNG_HandleTypeDef *hrng);

#define IS_RNG_IT(IT) ((IT) == RNG_IT_SEI))

#define IS_RNG_FLAG(FLAG) (((FLAG) == RNG_FLAG_DRDY) || \
                           ((FLAG) == RNG_FLAG_SECS))


#ifdef __cplusplus
}
#endif

#endif 


