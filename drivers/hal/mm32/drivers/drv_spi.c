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
 * @file        drv_spi.c
 *
 * @brief       This file implements SPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <drv_spi.h>
#include <string.h>


#define DBG_TAG "drv.spi"
#include <dlog.h>

struct stm32_hw_spi_cs
{
    GPIO_TypeDef *GPIOx;
    uint16_t      GPIO_Pin;
};

struct stm32_spi_config
{
    SPI_TypeDef *Instance;
    char        *bus_name;
    struct dma_config *dma_rx, *dma_tx;
};

struct stm32_spi_device
{
    os_uint32_t pin;
    char *bus_name;
    char *device_name;
};

#define SPI_USING_RX_DMA_FLAG (1 << 0)
#define SPI_USING_TX_DMA_FLAG (1 << 1)

struct stm32_spi
{
    struct os_spi_bus spi_bus;
    
    SPI_HandleTypeDef           *hspi;
    struct stm32_spi_config     *config;
    struct os_spi_configuration *cfg;

    struct
    {
        DMA_HandleTypeDef handle_rx;
        DMA_HandleTypeDef handle_tx;
    } dma;

    os_uint8_t     spi_dma_flag;
    os_list_node_t list;
};

static os_list_node_t stm32_spi_list = OS_LIST_INIT(stm32_spi_list);

static os_err_t stm32_spi_init(struct stm32_spi *spi_drv, struct os_spi_configuration *cfg)
{
    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    SPI_HandleTypeDef *spi_handle = spi_drv->hspi;

    if (cfg->mode & OS_SPI_SLAVE)
    {
        spi_handle->Init.Mode = SPI_MODE_SLAVE;
    }
    else
    {
        spi_handle->Init.Mode = SPI_MODE_MASTER;
    }

    if (cfg->mode & OS_SPI_3WIRE)
    {
        spi_handle->Init.Direction = SPI_DIRECTION_1LINE;
    }
    else
    {
        spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
    }

    if (cfg->data_width == 8)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
    }
    else if (cfg->data_width == 16)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_16BIT;
    }
    else
    {
        return OS_EIO;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_2EDGE;
    }
    else
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_1EDGE;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
    }
    else
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_LOW;
    }

    uint32_t SPI_APB_CLOCK;

#if defined(SERIES_STM32F0) || defined(SERIES_STM32G0)
    SPI_APB_CLOCK = HAL_RCC_GetPCLK1Freq();
#elif defined(SERIES_STM32H7)
    SPI_APB_CLOCK = HAL_RCC_GetSysClockFreq();
#elif defined(SERIES_STM32MP1)
    RCC_PeriphCLKInitTypeDef spi_clock_config;
    os_uint32_t spiclockselection;

    HAL_RCCEx_GetPeriphCLKConfig(&spi_clock_config);

    if(spi_handle->Instance == SPI1)
    {
        spiclockselection = spi_clock_config.Spi1ClockSelection;
    }
    else if((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3))
    {
        spiclockselection = spi_clock_config.Spi23ClockSelection;
    }
    else if((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5))
    {
        spiclockselection = spi_clock_config.Spi45ClockSelection;
    }
    else
    {
        return OS_ENODEV;
    }

    if(((spi_handle->Instance == SPI1) && (spiclockselection == RCC_SPI1CLKSOURCE_PLL4)) || \
        (((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3)) && (spiclockselection == RCC_SPI23CLKSOURCE_PLL4)))
    {
        PLL4_ClocksTypeDef pll4p_clocks;
        HAL_RCC_GetPLL4ClockFreq(&pll4p_clocks);
        SPI_APB_CLOCK = pll4p_clocks.PLL4_P_Frequency;
    }
    else if(((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5)) && (spiclockselection == RCC_SPI45CLKSOURCE_PLL4))
    {
        PLL4_ClocksTypeDef pll4q_clocks;
        HAL_RCC_GetPLL4ClockFreq(&pll4q_clocks);
        SPI_APB_CLOCK = pll4q_clocks.PLL4_Q_Frequency;
    }
    else if(((spi_handle->Instance == SPI1) && (spiclockselection == RCC_SPI1CLKSOURCE_PLL3_Q)) || \
        (((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3)) && (spiclockselection == RCC_SPI23CLKSOURCE_PLL3_Q)))
    {
        PLL3_ClocksTypeDef pll3q_clocks;
        HAL_RCC_GetPLL3ClockFreq(&pll3q_clocks);
        SPI_APB_CLOCK = pll3q_clocks.PLL3_Q_Frequency;
    }        
    else if(((spi_handle->Instance == SPI1) && (spiclockselection == RCC_SPI1CLKSOURCE_PLL3_R)) || \
        (((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3)) &&(spiclockselection == RCC_SPI23CLKSOURCE_PLL3_R)))
    {
        PLL3_ClocksTypeDef pll3r_clocks;
        HAL_RCC_GetPLL3ClockFreq(&pll3r_clocks);
        SPI_APB_CLOCK = pll3r_clocks.PLL3_R_Frequency;
    }
    else if(((spi_handle->Instance == SPI1) && (spiclockselection == RCC_SPI1CLKSOURCE_PER)) || \
        (((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3)) && (spiclockselection == RCC_SPI23CLKSOURCE_PER)))
    {
        SPI_APB_CLOCK = RCC_GetCKPERFreq();
    }    
    else if(((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5)) && (spiclockselection == RCC_SPI45CLKSOURCE_HSI))
    {
        SPI_APB_CLOCK = HSI_VALUE;
    }
    else if(((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5)) &&(spiclockselection == RCC_SPI45CLKSOURCE_CSI))
    {
        SPI_APB_CLOCK = CSI_VALUE;
    }
    else if(((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5)) && (spiclockselection == RCC_SPI45CLKSOURCE_HSE))
    {
        SPI_APB_CLOCK = HSE_VALUE;
    }    
    else if(((spi_handle->Instance == SPI4) || (spi_handle->Instance == SPI5)) && (spiclockselection == RCC_SPI45CLKSOURCE_PCLK2))
    {
        SPI_APB_CLOCK = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
        return OS_ENODEV;
    }
    
#elif defined (SPI2) && defined (SPI3)
    /* suitable for chips SPI number >=3,such as F7 series，most F4 series ...*/   
    if((spi_handle->Instance == SPI2) || (spi_handle->Instance == SPI3))
    {
        SPI_APB_CLOCK = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        SPI_APB_CLOCK = HAL_RCC_GetPCLK2Freq(); /* SPI1,4,5,6 */
    }
#else
     /* suitable for SPI number < 3...*/ 
    if(spi_handle->Instance == SPI1)
    {
        SPI_APB_CLOCK = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
        SPI_APB_CLOCK = HAL_RCC_GetPCLK1Freq();
    }
#endif

    if (cfg->max_hz >= SPI_APB_CLOCK / 2)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 4)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 8)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 16)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 32)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 64)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 128)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    }
    else
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    }

    LOG_D(DBG_TAG,"sys freq: %d, pclk2 freq: %d, SPI limiting freq: %d, BaudRatePrescaler: %d",
#if defined(SERIES_STM32MP1)
              HAL_RCC_GetSystemCoreClockFreq(),
#else
              HAL_RCC_GetSysClockFreq(),
#endif
              SPI_APB_CLOCK,
              cfg->max_hz,
              spi_handle->Init.BaudRatePrescaler);

    if (cfg->mode & OS_SPI_MSB)
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
    }
    else
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_LSB;
    }

    if (HAL_SPI_Init(spi_handle) != HAL_OK)
    {
        return OS_EIO;
    }

    LOG_D(DBG_TAG,"%s init done", spi_drv->config->bus_name);
    return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct stm32_spi *spi_drv = os_container_of(device->bus, struct stm32_spi, spi_bus);
    spi_drv->cfg              = configuration;

    return stm32_spi_init(spi_drv, configuration);
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    HAL_StatusTypeDef state;
    os_size_t         message_length, already_send_length;
    os_uint16_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct stm32_spi  *spi_drv    = os_container_of(device->bus, struct stm32_spi, spi_bus);
    SPI_HandleTypeDef *spi_handle = spi_drv->hspi;

    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

    LOG_D(DBG_TAG,"%s transfer prepare and start", spi_drv->config->bus_name);
    LOG_D(DBG_TAG,"%s sendbuf: %X, recvbuf: %X, length: %d",
              spi_drv->config->bus_name,
              (uint32_t)message->send_buf,
              (uint32_t)message->recv_buf,
              message->length);

    message_length = message->length;
    recv_buf       = message->recv_buf;
    send_buf       = message->send_buf;
    
    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > 65535)
        {
            send_length    = 65535;
            message_length = message_length - 65535;
        }
        else
        {
            send_length    = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = message->length - send_length - message_length;
        send_buf            = (os_uint8_t *)message->send_buf + already_send_length;
        recv_buf            = (os_uint8_t *)message->recv_buf + already_send_length;

        /* start once data exchange in DMA mode */
        if (message->send_buf && message->recv_buf)
        {
            if ((spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG) && (spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG))
            {
                state = HAL_SPI_TransmitReceive_DMA(spi_handle, (uint8_t *)send_buf, (uint8_t *)recv_buf, send_length);
            }
            else
            {
                state = HAL_SPI_TransmitReceive(spi_handle, (uint8_t *)send_buf, (uint8_t *)recv_buf, send_length, 1000);
            }
        }
        else if (message->send_buf)
        {
            if (spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
            {
                state = HAL_SPI_Transmit_DMA(spi_handle, (uint8_t *)send_buf, send_length);
            }
            else
            {
                state = HAL_SPI_Transmit(spi_handle, (uint8_t *)send_buf, send_length, 1000);
            }
        }
        else
        {
            memset((uint8_t *)recv_buf, 0xff, send_length);
            if (spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
            {
                state = HAL_SPI_Receive_DMA(spi_handle, (uint8_t *)recv_buf, send_length);
            }
            else
            {
                state = HAL_SPI_Receive(spi_handle, (uint8_t *)recv_buf, send_length, 1000);
            }
        }

        if (state != HAL_OK)
        {
            LOG_I(DBG_TAG,"spi transfer error : %d", state);
            message->length   = 0;
            spi_handle->State = HAL_SPI_STATE_READY;
        }
        else
        {
            LOG_D(DBG_TAG,"%s transfer done", spi_drv->config->bus_name);
        }

        /* For simplicity reasons, this example is just waiting till the end of the
           transfer, but application may perform other tasks while transfer operation
           is ongoing. */
        while (HAL_SPI_GetState(spi_handle) != HAL_SPI_STATE_READY);
    }

    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}

static const struct os_spi_ops stm32_spi_ops = {
    .configure = spi_configure,
    .xfer      = spixfer,
};

static int stm32_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;
    os_base_t   level;

    struct stm32_spi *st_spi = os_calloc(1, sizeof(struct stm32_spi));

    OS_ASSERT(st_spi);

    st_spi->hspi = (SPI_HandleTypeDef *)dev->info;

    struct os_spi_bus *spi_bus = &st_spi->spi_bus;

    level = os_irq_lock();
    os_list_add_tail(&stm32_spi_list, &st_spi->list);
    os_irq_unlock(level);

    result = os_spi_bus_register(spi_bus, dev->name, &stm32_spi_ops);
    OS_ASSERT(result == OS_EOK);

    LOG_D(DBG_TAG,"%s bus init done", dev->name);

    return result;
}

OS_DRIVER_INFO stm32_spi_driver = {
    .name   = "SPI_HandleTypeDef",
    .probe  = stm32_spi_probe,
};

OS_DRIVER_DEFINE(stm32_spi_driver,PREV,OS_INIT_SUBLEVEL_LOW);

