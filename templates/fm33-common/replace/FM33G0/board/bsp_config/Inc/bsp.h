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
 * @file        bps.h
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 #ifndef _BSP_H_
 #define _BSP_H_
 
#include "os_types.h"
#include "board.h"

typedef struct __UART_HandleTypeDef
{
    UARTx_Type *instance;
}UART_HandleTypeDef;

typedef struct __LPUART_HandleTypeDef
{
    LPUART_Type *instance;
}LPUART_HandleTypeDef;

typedef struct __ADC_HandleTypeDef
{
    ANAC_Type *instance;
}ADC_HandleTypeDef;

typedef struct __IWDG_HandleTypeDef
{
    IWDT_Type *instance;
}IWDG_HandleTypeDef;

typedef struct __SPI_HandleTypeDef
{
    SPIx_Type *instance;
}SPI_HandleTypeDef;

typedef struct __LCD_HandleTypeDef
{
    DISP_Type *instance;
}LCD_HandleTypeDef;

typedef struct __TIM_HandleTypeDef
{
    void *instance;
}TIM_HandleTypeDef;

#if 0
typedef struct __LPUART_HandleTypeDef
{
    LPUART_Type *instance;
}LPUART_HandleTypeDef;

typedef struct __TIM_HandleTypeDef
{
    void *instance;
}TIM_HandleTypeDef;







#endif


#endif /*_BSP_H_*/
