/**
  ******************************************************************************
  * @file    hk32f39a_hal_cryp_ex.h
  * @author  Rakan
  * @date    2019/10/30
  * @brief   CRYP  module driver.
  ******************************************************************************  
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_HAL_CRYP_EX_H
#define __HK32F39A_HAL_CRYP_EX_H

#ifdef __cplusplus
 extern "C" {
#endif
   

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_hal_def.h"


/** @defgroup CRYPEx CRYPEx
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup CRYPEx_Exported_Functions CRYPEx Exported Functions
  * @{
  */

/** @defgroup CRYPEx_Exported_Functions_Group1 Extended features functions
  * @{
  */

/* CallBack functions  ********************************************************/
void HAL_CRYPEx_ComputationCpltCallback(CRYP_HandleTypeDef *hcryp);

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */ 
  
#ifdef __cplusplus
}
#endif

#endif /* __HK32F39A_HAL_CRYP_EX_H */
 

