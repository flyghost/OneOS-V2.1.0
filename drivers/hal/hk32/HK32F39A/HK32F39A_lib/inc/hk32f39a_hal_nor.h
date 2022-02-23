/**
  ******************************************************************************
  * @file    hk32f39a_hal_nor.h
  * @author  laura.c
  * @version V1.0  
  * @brief   Header file of NOR HAL module.
  */ 


#ifndef __HK32F39A_HAL_NOR_H
#define __HK32F39A_HAL_NOR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_fsmc.h"

/* NOR device IDs addresses */
#define MC_ADDRESS               ((uint16_t)0x0000)
#define DEVICE_CODE1_ADDR        ((uint16_t)0x0001)
#define DEVICE_CODE2_ADDR        ((uint16_t)0x000E)
#define DEVICE_CODE3_ADDR        ((uint16_t)0x000F)

/* NOR CFI IDs addresses */
#define CFI1_ADDRESS             ((uint16_t)0x10)
#define CFI2_ADDRESS             ((uint16_t)0x11)
#define CFI3_ADDRESS             ((uint16_t)0x12)
#define CFI4_ADDRESS             ((uint16_t)0x13)

/* NOR operation wait timeout */
#define NOR_TMEOUT               ((uint16_t)0xFFFF)
   
/* NOR memory data width */
#define NOR_MEMORY_8B            ((uint8_t)0x0)
#define NOR_MEMORY_16B           ((uint8_t)0x1)

/* NOR memory device read/write start address */
#define NOR_MEMORY_ADRESS1       FSMC_BANK1_1
#define NOR_MEMORY_ADRESS2       FSMC_BANK1_2
#define NOR_MEMORY_ADRESS3       FSMC_BANK1_3
#define NOR_MEMORY_ADRESS4       FSMC_BANK1_4


/**
  * @brief  NOR memory address shifting.
  * @param  __NOR_ADDRESS: NOR base address 
  * @param  __NOR_MEMORY_WIDTH_: NOR memory width
  * @param  __ADDRESS__: NOR memory address 
  * @retval NOR shifted address value
  */
#define NOR_ADDR_SHIFT(__NOR_ADDRESS, __NOR_MEMORY_WIDTH_, __ADDRESS__)       \
            ((uint32_t)(((__NOR_MEMORY_WIDTH_) == NOR_MEMORY_16B)?              \
              ((uint32_t)((__NOR_ADDRESS) + (2U * (__ADDRESS__)))):              \
              ((uint32_t)((__NOR_ADDRESS) + (__ADDRESS__)))))

/**
  * @brief  NOR memory write data to specified address.
  * @param  __ADDRESS__: NOR memory address 
  * @param  __DATA__: Data to write
  * @retval None
  */
#define NOR_WRITE(__ADDRESS__, __DATA__)  (*(__IO uint16_t *)((uint32_t)(__ADDRESS__)) = (__DATA__))


/* Exported typedef ----------------------------------------------------------*/ 
/** 
  * @brief  HAL SRAM State structures definition  
  */ 
typedef enum
{  
  HAL_NOR_STATE_RESET             = 0x00U,  /*!< NOR not yet initialized or disabled  */
  HAL_NOR_STATE_READY             = 0x01U,  /*!< NOR initialized and ready for use    */
  HAL_NOR_STATE_BUSY              = 0x02U,  /*!< NOR internal processing is ongoing   */
  HAL_NOR_STATE_ERROR             = 0x03U,  /*!< NOR error state                      */ 
  HAL_NOR_STATE_PROTECTED         = 0x04U   /*!< NOR NORSRAM device write protected  */
}HAL_NOR_StateTypeDef;    

/**
  * @brief  FSMC NOR Status typedef
  */
typedef enum
{
  HAL_NOR_STATUS_SUCCESS = 0U,
  HAL_NOR_STATUS_ONGOING,
  HAL_NOR_STATUS_ERROR,
  HAL_NOR_STATUS_TIMEOUT
}HAL_NOR_StatusTypeDef; 

/**
  * @brief  FSMC NOR ID typedef
  */
typedef struct
{
  uint16_t Manufacturer_Code;  /*!< Defines the device's manufacturer code used to identify the memory       */
  
  uint16_t Device_Code1;
  
  uint16_t Device_Code2;
        
  uint16_t Device_Code3;       /*!< Defines the device's codes used to identify the memory. 
                                    These codes can be accessed by performing read operations with specific 
                                    control signals and addresses set.They can also be accessed by issuing 
                                    an Auto Select command                                                   */    
}NOR_IDTypeDef;

/**
  * @brief  FSMC NOR CFI typedef
  */
typedef struct
{
  /*!< Defines the information stored in the memory's Common flash interface
       which contains a description of various electrical and timing parameters, 
       density information and functions supported by the memory                   */
  
  uint16_t CFI_1;
  
  uint16_t CFI_2;
  
  uint16_t CFI_3;
  
  uint16_t CFI_4;
}NOR_CFITypeDef;

/** 
  * @brief  NOR handle Structure definition  
  */ 
typedef struct
{
  FSMC_NORSRAM_TypeDef          *Instance;    /*!< Register base address                        */ 
  
  FSMC_NORSRAM_EXTENDED_TypeDef *Extended;    /*!< Extended mode register base address          */
  
  FSMC_NORSRAM_InitTypeDef      Init;         /*!< NOR device control configuration parameters  */

  HAL_LockTypeDef               Lock;         /*!< NOR locking object                           */ 
  
  __IO HAL_NOR_StateTypeDef     State;        /*!< NOR device access state                      */
   
}NOR_HandleTypeDef; 



/** @brief Reset NOR handle state
  * @param  __HANDLE__: NOR handle
  * @retval None
  */
#define __HAL_NOR_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_NOR_STATE_RESET)


/* Initialization/de-initialization functions  **********************************/  
HAL_StatusTypeDef HAL_NOR_Init(NOR_HandleTypeDef *hnor, FSMC_NORSRAM_TimingTypeDef *Timing, FSMC_NORSRAM_TimingTypeDef *ExtTiming);
HAL_StatusTypeDef HAL_NOR_DeInit(NOR_HandleTypeDef *hnor);
void              HAL_NOR_MspInit(NOR_HandleTypeDef *hnor);
void              HAL_NOR_MspDeInit(NOR_HandleTypeDef *hnor);
void              HAL_NOR_MspWait(NOR_HandleTypeDef *hnor, uint32_t Timeout);

/* I/O operation functions  ***************************************************/
HAL_StatusTypeDef HAL_NOR_Read_ID(NOR_HandleTypeDef *hnor, NOR_IDTypeDef *pNOR_ID);
HAL_StatusTypeDef HAL_NOR_ReturnToReadMode(NOR_HandleTypeDef *hnor);
HAL_StatusTypeDef HAL_NOR_Read(NOR_HandleTypeDef *hnor, uint32_t *pAddress, uint16_t *pData);
HAL_StatusTypeDef HAL_NOR_Program(NOR_HandleTypeDef *hnor, uint32_t *pAddress, uint16_t *pData);

HAL_StatusTypeDef HAL_NOR_ReadBuffer(NOR_HandleTypeDef *hnor, uint32_t uwAddress, uint16_t *pData, uint32_t uwBufferSize);
HAL_StatusTypeDef HAL_NOR_ProgramBuffer(NOR_HandleTypeDef *hnor, uint32_t uwAddress, uint16_t *pData, uint32_t uwBufferSize);

HAL_StatusTypeDef HAL_NOR_Erase_Block(NOR_HandleTypeDef *hnor, uint32_t BlockAddress, uint32_t Address);
HAL_StatusTypeDef HAL_NOR_Erase_Chip(NOR_HandleTypeDef *hnor, uint32_t Address);
HAL_StatusTypeDef HAL_NOR_Read_CFI(NOR_HandleTypeDef *hnor, NOR_CFITypeDef *pNOR_CFI);

/* NOR Control functions  *****************************************************/
HAL_StatusTypeDef HAL_NOR_WriteOperation_Enable(NOR_HandleTypeDef *hnor);
HAL_StatusTypeDef HAL_NOR_WriteOperation_Disable(NOR_HandleTypeDef *hnor);

/* NOR State functions ********************************************************/
HAL_NOR_StateTypeDef  HAL_NOR_GetState(NOR_HandleTypeDef *hnor);
HAL_NOR_StatusTypeDef HAL_NOR_GetStatus(NOR_HandleTypeDef *hnor, uint32_t Address, uint32_t Timeout);


#ifdef __cplusplus
}
#endif

#endif /* __HK32F39A_HAL_NOR_H */


