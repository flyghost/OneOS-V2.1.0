/**
  ******************************************************************************
  * @file    hk32f39a_hal_cryp_ex.c
  * @author  Rakan
  * @date    2019/10/30
  * @brief   CRYP  module driver.
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_conf.h"

/** @addtogroup CRYPEx
  * @brief CRYP HAL Extended module driver.
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @addtogroup CRYPEx_Exported_Functions
  * @{
  */


/** @addtogroup CRYPEx_Exported_Functions_Group1
 *  @brief    Extended features functions. 
 *
@verbatim   
 ===============================================================================
                 ##### Extended features functions #####
 =============================================================================== 
    [..]  This section provides callback functions:
      (+) Computation completed.

@endverbatim
  * @{
  */

/**
  * @brief  Computation completed callbacks.
  * @param  hcryp pointer to a CRYP_HandleTypeDef structure that contains
  *         the configuration information for CRYP module
  * @retval None
  */
__weak void HAL_CRYPEx_ComputationCpltCallback(CRYP_HandleTypeDef *hcryp)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcryp);

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CRYP_ComputationCpltCallback could be implemented in the user file
   */ 
}



