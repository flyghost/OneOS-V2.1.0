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
 * @file        bps.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <board.h>

#if defined(BSP_USING_UART0)
UART_HandleTypeDef huart0 =
{
    UART0,
};
#endif

#if defined(BSP_USING_UART1)
UART_HandleTypeDef huart1 =
{
    UART1,
};
#endif

#if defined(BSP_USING_UART2)
UART_HandleTypeDef huart2 =
{
    UART2,
};
#endif


#if defined(BSP_USING_UART3)
UART_HandleTypeDef huart3 =
{
    UART3,
};
#endif

#if defined(BSP_USING_UART4)
UART_HandleTypeDef huart4 =
{
    UART4,
};
#endif

#if defined(BSP_USING_UART5)
UART_HandleTypeDef huart5 =
{
    UART5,
};
#endif

#if defined(BSP_USING_LPUART0)
LPUART_HandleTypeDef hlpuart0 =
{
    LPUART,
};
#endif


#if defined (OS_USING_ADC)
ADC_HandleTypeDef hadc =
{
    ANAC,
};
#endif

#if defined (OS_USING_WDG)
IWDG_HandleTypeDef hiwdt =
{
    IWDT,
};
#endif

#if defined (BSP_USING_SPI1)
SPI_HandleTypeDef hspi1 =
{
    SPI1,
};
#endif

#if defined (BSP_USING_SPI2)
SPI_HandleTypeDef hspi2 =
{
    SPI2,
};
#endif

#if defined (OS_USING_LCD)
LCD_HandleTypeDef hlcd =
{
    LCD,
};
#endif

#if defined (BSP_USING_LPTIM32)
TIM_HandleTypeDef hlptim = 
{
    0,
};
#endif

#if 0
#if defined(BSP_USING_UART4)
UART_HandleTypeDef huart4 =
{
    UART4,
};
#endif

#if defined(BSP_USING_UART5)
UART_HandleTypeDef huart5 =
{
    UART5,
};
#endif

#if defined(BSP_USING_LPUART0)
LPUART_HandleTypeDef hlpuart0 =
{
    LPUART0,
};
#endif

#if defined(BSP_USING_LPUART1)
LPUART_HandleTypeDef hlpuart1 =
{
    LPUART1,
};
#endif

#if defined(OS_USING_TIMER_DRIVER)

#if defined(BSP_USING_ATIM)
TIM_HandleTypeDef hatim =
{
    ATIM,
};
#endif

#if defined(BSP_USING_BSTIM32)

TIM_HandleTypeDef hbtim =
{
    BSTIM32,
};
#endif

#if defined(BSP_USING_GPTIM0)
TIM_HandleTypeDef hgptim0 =
{
    GPTIM0,
};
#endif

#if defined(BSP_USING_GPTIM1)
TIM_HandleTypeDef hgptim1 =
{
    GPTIM1,
};
#endif

#if defined(BSP_USING_LPTIM32)
TIM_HandleTypeDef hlptim =
{
    LPTIM32,
};
#endif

#endif

#endif
