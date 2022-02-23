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
 * @file        drv_usart.h
 *
 * @brief        This file provides functions declaration for usart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <board.h>

struct hk32_usart_info {
    USART_TypeDef *huart;           /* USART1 */

    IRQn_Type irq;                  /* USART1_IRQn */

    enum hk32_rcc_type rcc_tpye;
    uint32_t  rcc;                  /* RCC_APB2Periph_USART1 */

    GPIO_TypeDef *gpio_tx_port;     /* GPIOA */
    uint16_t gpio_tx_pin;           /* GPIO_Pin_9 */
    uint32_t gpio_tx_rcc;           /* RCC_APB2Periph_GPIOA */

    GPIO_TypeDef *gpio_rx_port;     /* GPIOA */
    uint16_t gpio_rx_pin;           /* GPIO_Pin_10 */
    uint32_t gpio_rx_rcc;           /* RCC_APB2Periph_GPIOA */

    DMA_Channel_TypeDef *dma_channel;   /* DMA1_Channel6 */
    uint32_t dma_rcc;                   /* RCC_AHBPeriph_DMA1 */
    IRQn_Type dma_irq;                  /* DMA1_Channel5_IRQn */
};

#endif /* __DRV_USART_H__ */

