/**
******************************************************************************
* @file    HAL_aes.h
* @author  AE team
* @version V1.1.0
* @date    28/08/2019
* @brief   This file contains all the functions prototypes for the WWDG
*          firmware library.
******************************************************************************
* @copy
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MindMotion SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2019 MindMotion</center></h2>
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HAL_AES_H
#define __HAL_AES_H

/* Includes ------------------------------------------------------------------*/
#include "HAL_device.h"

/** @addtogroup AES
  * @{
  */

/**
  * @brief   AES Init structure definition
  */
typedef struct
{
    uint32_t AES_Operation; /*!< Specifies the AES mode of operation.
                               This parameter can be a value of @ref AES_possible_Operation_modes */
    uint32_t AES_Chaining;  /*!< Specifies the AES Chaining modes: ECB, CBC or CTR.
                               This parameter can be a value of @ref AES_possible_chaining_modes */
    uint32_t AES_DataType;  /*!< Specifies the AES data swapping: 32-bit, 16-bit, 8-bit or 1-bit.
                               This parameter can be a value of @ref AES_Data_Types */
} AES_InitTypeDef;

/**
  * @brief   AES Key(s) structure definition
  */
typedef struct
{
    uint32_t AES_Key0;  /*!< Key[31:0]   */
    uint32_t AES_Key1;  /*!< Key[63:32]  */
    uint32_t AES_Key2;  /*!< Key[95:64]  */
    uint32_t AES_Key3;  /*!< Key[127:96] */
    uint32_t AES_Key4;  /*!< Key[157:127] */
    uint32_t AES_Key5;  /*!< Key[191:160] */
    uint32_t AES_Key6;  /*!< Key[223:192] */
    uint32_t AES_Key7;  /*!< Key[255:224] */
} AES_KeyInitTypeDef;

/**
  * @brief   AES Initialization Vectors (IV) structure definition
  */
typedef struct
{
    uint32_t AES_IV0;  /*!< Init Vector IV[31:0]   */
    uint32_t AES_IV1;  /*!< Init Vector IV[63:32]  */
    uint32_t AES_IV2;  /*!< Init Vector IV[95:64]  */
    uint32_t AES_IV3;  /*!< Init Vector IV[127:96] */
} AES_IVInitTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup AES_Exported_Constants
  * @{
  */

/** @defgroup AES_possible_Operation_modes
  * @{
  */
#define AES_Operation_Encryp               ((uint32_t)0x00000000)          /*!< AES in Encryption mode */
#define AES_Operation_KeyDeriv             ((uint32_t)0x00000008)          /*!< AES in Key Derivation mode */
#define AES_Operation_Decryp               ((uint32_t)0x00000010)          /*!< AES in Decryption mode */
#define AES_Operation_KeyDerivAndDecryp    ((uint32_t)0x00000018)          /*!< AES in Key Derivation and Decryption mode */

#define IS_AES_MODE(OPERATION) (((OPERATION) == AES_Operation_Encryp)    || \
                                ((OPERATION) == AES_Operation_KeyDeriv)  || \
                                ((OPERATION) == AES_Operation_Decryp)    || \
                                ((OPERATION) == AES_Operation_KeyDerivAndDecryp))

/** @defgroup AES_possible_chaining_modes
  * @{
  */
#define AES_Chaining_ECB                   ((uint32_t)0x00000000)           /*!< AES in ECB chaining mode */
#define AES_Chaining_CBC                   ((uint32_t)(0x0<<16)|(0x1<<5))   /*!< AES in CBC chaining mode */
#define AES_Chaining_CTR                   ((uint32_t)(0x0<<16)|(0x2<<5))   /*!< AES in CTR chaining mode */
#define AES_Chaining_CFB                   ((uint32_t)(0x1<<16)|(0x0<<5))   /*!< AES in CFB chaining mode */
#define AES_Chaining_OFB                   ((uint32_t)(0x1<<16)|(0x1<<5))   /*!< AES in OFB chaining mode */

#define IS_AES_CHAINING(CHAINING) (((CHAINING) == AES_Chaining_ECB) || \
                                   ((CHAINING) == AES_Chaining_CBC) || \
                                   ((CHAINING) == AES_Chaining_CTR))

/** @defgroup AES_Data_Types
  * @{
  */
#define AES_DataType_32b                   ((uint32_t)0x00000000)           /*!< 32-bit data. No swapping */
#define AES_DataType_16b                   AES_CR_DATATYPE_0                /*!< 16-bit data. Each half word is swapped */
#define AES_DataType_8b                    AES_CR_DATATYPE_1                /*!< 8-bit data. All bytes are swapped */
#define AES_DataType_1b                    AES_CR_DATATYPE                  /*!< 1-bit data. In the word all bits are swapped */

#define IS_AES_DATATYPE(DATATYPE) (((DATATYPE) == AES_DataType_32b) || \
                                   ((DATATYPE) == AES_DataType_16b)|| \
                                   ((DATATYPE) == AES_DataType_8b) || \
                                   ((DATATYPE) == AES_DataType_1b))

/** @defgroup AES_Flags
  * @{
  */
#define AES_FLAG_CCF                       ((uint32_t)0x00000001)           /*!< Computation Complete Flag */
#define AES_FLAG_RDERR                     AES_SR_RDERR                     /*!< Read Error Flag           */
#define AES_FLAG_WRERR                     AES_SR_WRERR                     /*!< Write Error Flag          */

#define IS_AES_FLAG(FLAG) (((FLAG) == AES_FLAG_CCF)    || \
                           ((FLAG) == AES_FLAG_RDERR)  || \
                           ((FLAG) == AES_FLAG_WRERR))
/**
  * @}
  */

/** @defgroup AES_Interrupts
  * @{
  */
#define AES_IT_CC                          AES_CR_CCFIE  /*!< Computation Complete interrupt */
#define AES_IT_ERR                         AES_CR_ERRIE /*!< Error interrupt                */

#define IS_AES_IT(IT) ((((IT) & (uint32_t)0xFFFFF9FF) == 0x00) && ((IT) != 0x00))
#define IS_AES_GET_IT(IT) (((IT) == AES_IT_CC) || ((IT) == AES_IT_ERR))

/**
  * @}
  */

/** @defgroup AES_DMA_Transfer_modes
  * @{
  */
#define AES_DMATransfer_In                 AES_CR_DMAINEN                     /*!< DMA requests enabled for input transfer phase */
#define AES_DMATransfer_Out                AES_CR_DMAOUTEN                    /*!< DMA requests enabled for input transfer phase */
#define AES_DMATransfer_InOut              (AES_CR_DMAINEN | AES_CR_DMAOUTEN) /*!< DMA requests enabled for both input and output phases */

#define IS_AES_DMA_TRANSFER(TRANSFER)   (((TRANSFER) == AES_DMATransfer_In)  || \
        ((TRANSFER) == AES_DMATransfer_Out)  || \
        ((TRANSFER) == AES_DMATransfer_InOut))


/* The size of the keyprior to calling this function. Instead of the size of the key**********************************/
#define CRL_AES128_KEY               (0x0<<18)
#define CRL_AES192_KEY               (0x1<<18)
#define CRL_AES256_KEY               (0x2<<18)

/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Initialization and configuration functions *********************************/
void AES_DeInit(void);
void AES_Init(AES_InitTypeDef *AES_InitStruct);
void AES_KeyInit(AES_KeyInitTypeDef *AES_KeyInitStruct);
void AES_IVInit(AES_IVInitTypeDef *AES_IVInitStruct);
void AES_Cmd(FunctionalState NewState);
void AES_KeySize(uint32_t CRL_AESxxx_KEY);

/* Structures initialization functions ****************************************/
void AES_StructInit(AES_InitTypeDef *AES_InitStruct);
void AES_KeyStructInit(AES_KeyInitTypeDef *AES_KeyInitStruct);
void AES_IVStructInit(AES_IVInitTypeDef *AES_IVInitStruct);

/* AES Read and Write functions **********************************************/
void AES_WriteSubData(uint32_t Data);
uint32_t AES_ReadSubData(void);
void AES_ReadKey(AES_KeyInitTypeDef *AES_KeyInitStruct);
void AES_ReadIV(AES_IVInitTypeDef *AES_IVInitStruct);

/* DMA transfers management function ******************************************/
void AES_DMAConfig(uint32_t AES_DMATransfer, FunctionalState NewState);

/* Interrupts and flags management functions **********************************/
void AES_ITConfig(uint32_t AES_IT, FunctionalState NewState);
FlagStatus AES_GetFlagStatus(uint32_t AES_FLAG);
void AES_ClearFlag(uint32_t AES_FLAG);
ITStatus AES_GetITStatus(uint32_t AES_IT);
void AES_ClearITPendingBit(uint32_t AES_IT);

ErrorStatus AES_ECB_Encrypt(uint8_t *Key, uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_ECB_Decrypt(uint8_t *Key, uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CBC_Encrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CBC_Decrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CTR_Encrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CTR_Decrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CFB_Encrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_CFB_Decrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_OFB_Encrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
ErrorStatus AES_OFB_Decrypt(uint8_t *Key, uint8_t InitVectors[16], uint8_t *Input, uint32_t Ilength, uint8_t *Output);
#endif /* __HAL_AES_H */

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/*-------------------------(C) COPYRIGHT 2019 MindMotion ----------------------*/
