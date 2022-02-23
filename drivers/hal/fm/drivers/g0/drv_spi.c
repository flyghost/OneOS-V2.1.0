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
 * @brief       This file implements SPI driver for fm33.
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

#define HAL_SPI_WAIT_TX_FLAG  0x00
#define HAL_SPI_WAIT_RX_FLAG  0x01

struct fm33_spi_config
{
    SPI_Type    *Instance;
    char        *bus_name;
};

struct fm33_spi_device
{
    os_uint32_t     pin;
    char           *bus_name;
    char           *device_name;
};

struct fm33_spi
{
    struct os_spi_bus             spi_bus;

    SPI_HandleTypeDef            *hspi;
    FL_SPI_InitTypeDef            InitStruct;
    struct fm33_spi_config       *config;
    struct os_spi_configuration  *cfg;

    os_list_node_t                list;
};

struct fm33_spi_gpio
{
    GPIO_Type    *GPIOx;
    uint32_t      GPIO_SSN;
    uint32_t      GPIO_SCK;
    uint32_t      GPIO_MISO;
    uint32_t      GPIO_MOSI;
};

static const struct fm33_spi_gpio spi_io[] =
{
    /*spi1*/
    {GPIOB, FL_GPIO_PIN_8, FL_GPIO_PIN_9, FL_GPIO_PIN_10, FL_GPIO_PIN_11},
    /*spi2*/
    {GPIOC, FL_GPIO_PIN_7, FL_GPIO_PIN_8, FL_GPIO_PIN_9,  FL_GPIO_PIN_10},
};

static os_list_node_t fm33_spi_list = OS_LIST_INIT(fm33_spi_list);

/**
 ***********************************************************************************************************************
 * HAL SPI APIs
 ***********************************************************************************************************************
**/
static os_err_t HAL_SPI_Wait_Timeout(SPI_Type *hspi, os_uint8_t flag, os_uint32_t timeout)
{
    os_uint32_t wait_time = timeout * 1000;
    if (flag == HAL_SPI_WAIT_TX_FLAG)
    {
        while (!(FL_SPI_IsActiveFlag_TXBuffEmpty(hspi)))
        {
            wait_time--;
            if (wait_time == 0)
                break;
        }
    }
    else if (flag == HAL_SPI_WAIT_RX_FLAG)
    {
        while (!(FL_SPI_IsActiveFlag_RXBuffFull(hspi)))
        {
            wait_time--;
            if (wait_time == 0)
                break;
        }
    }

    if (wait_time == 0)
        return OS_ETIMEOUT;
    else
        return OS_EOK;
}

static os_err_t HAL_SPI_Write_and_Read_Byte(SPI_Type *hspi, os_uint8_t *write_byte, os_uint8_t *recv_byte, os_uint32_t timeout)
{
    os_uint8_t dummy_byte = 0xff;

    /*write byte*/
    if (write_byte != NULL)
        FL_SPI_WriteTXBuff(hspi, *write_byte);
    else
        FL_SPI_WriteTXBuff(hspi, dummy_byte);

    if (HAL_SPI_Wait_Timeout(hspi, HAL_SPI_WAIT_TX_FLAG, timeout) != OS_EOK)
        return OS_ERROR;

    if (HAL_SPI_Wait_Timeout(hspi, HAL_SPI_WAIT_RX_FLAG, timeout) != OS_EOK)
        return OS_ERROR;

    /*read byte*/
    if (recv_byte != NULL)
        *recv_byte = (os_uint8_t)FL_SPI_ReadRXBuff(hspi);
    else
        FL_SPI_ReadRXBuff(hspi);

    return OS_EOK;
}
static os_err_t HAL_SPI_TransmitReceive(SPI_Type *hspi, os_uint8_t *send_buf, os_uint8_t *recv_buf, os_uint16_t length, os_uint32_t timeout)
{
    os_uint16_t i;

    for (i = 0; i < length; i++)
    {
        if (HAL_SPI_Write_and_Read_Byte(hspi, &send_buf[i], &recv_buf[i], timeout) != OS_EOK)
            return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t HAL_SPI_Transmit(SPI_Type *hspi, os_uint8_t *send_buf, os_uint16_t length, os_uint32_t timeout)
{
    os_uint16_t i;

    for (i = 0; i < length; i++)
    {
        if (HAL_SPI_Write_and_Read_Byte(hspi, &send_buf[i], NULL, timeout) != OS_EOK)
            return OS_ERROR;
    }
    return OS_EOK;
}

static os_err_t HAL_SPI_Receive(SPI_Type *hspi, os_uint8_t *recv_buf, os_uint16_t length, os_uint32_t timeout)
{
    os_uint16_t i;

    for (i = 0; i < length; i++)
    {
        if (HAL_SPI_Write_and_Read_Byte(hspi, NULL, &recv_buf[i], timeout) != OS_EOK)
            return OS_ERROR;
    }
    return OS_EOK;
}
static void fm33_spi_gpio_init(SPI_Type *hspi)
{
    FL_GPIO_InitTypeDef             gpioInitStruct = {0};
    const struct fm33_spi_gpio     *gpio_info;

    if (hspi == SPI1)
    {
        gpio_info = &spi_io[0];
    }
    else if (hspi == SPI2)
    {
        gpio_info = &spi_io[1];
    }
    gpioInitStruct.pin = gpio_info->GPIO_SSN | gpio_info->GPIO_SCK | gpio_info->GPIO_MISO | gpio_info->GPIO_MOSI;
    gpioInitStruct.mode = FL_GPIO_MODE_DIGITAL;
    gpioInitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpioInitStruct.pull = FL_DISABLE;
    gpioInitStruct.remapPin = FL_DISABLE;

    FL_GPIO_Init(gpio_info->GPIOx, &gpioInitStruct);

}
static os_err_t fm33_spi_init(struct fm33_spi *spi_drv, struct os_spi_configuration *cfg)
{
    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    os_uint32_t SPI_APB_CLOCK;

    SPI_HandleTypeDef  *spi_handle  = spi_drv->hspi;
    FL_SPI_InitTypeDef *init_struct = &spi_drv->InitStruct;

    if (spi_handle->instance == SPI1)
        SPI_APB_CLOCK = FL_RCC_GetAPB2ClockFreq();
    else if (spi_handle->instance == SPI2)
        SPI_APB_CLOCK = FL_RCC_GetAPB1ClockFreq();
    
    fm33_spi_gpio_init(spi_handle->instance);

    if (cfg->mode & OS_SPI_SLAVE)
    {
        init_struct->mode = FL_SPI_WORK_MODE_SLAVE;
    }
    else
    {
        init_struct->mode = FL_SPI_WORK_MODE_MASTER;
    }

    if (cfg->mode & OS_SPI_3WIRE)
    {
        LOG_E(DBG_TAG, "3 wire mode is not supported.");
        return OS_EIO;
    }

    switch (cfg->data_width)
    {
        case 8:
            init_struct->dataWidth = FL_SPI_DATA_WIDTH_8B;
            break;
        case 16:
            init_struct->dataWidth = FL_SPI_DATA_WIDTH_16B;
            break;
        case 24:
            init_struct->dataWidth = FL_SPI_DATA_WIDTH_24B;
            break;
        case 32:
            init_struct->dataWidth = FL_SPI_DATA_WIDTH_32B;
            break;
        default:
            return OS_EIO;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        init_struct->clockPhase = FL_SPI_PHASE_EDGE2;
    }
    else
    {
        init_struct->clockPhase = FL_SPI_PHASE_EDGE1;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        init_struct->clockPolarity = FL_SPI_POLARITY_INVERT;
    }
    else
    {
        init_struct->clockPolarity = FL_SPI_POLARITY_NORMAL;
    }

    if (cfg->mode & OS_SPI_MSB)
    {
        init_struct->bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    }
    else
    {
        init_struct->bitOrder = FL_SPI_BIT_ORDER_LSB_FIRST;
    }

    if (cfg->max_hz >= SPI_APB_CLOCK / 2)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV2;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 4)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV4;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 8)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV8;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 16)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV16;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 32)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV32;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 64)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV64;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 128)
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV128;
    }
    else
    {
        init_struct->baudRate = FL_SPI_BAUDRATE_DIV256;
    }

    init_struct->transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    init_struct->softControl = FL_ENABLE;

    if (FL_SPI_Init(spi_handle->instance, init_struct) != FL_PASS)
    {
        return OS_EIO;
    }

    FL_SPI_ClearTXBuff(spi_handle->instance);
    FL_SPI_ClearRXBuff(spi_handle->instance);

    LOG_I(DBG_TAG, "%s init done", spi_drv->config->bus_name);
    return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct fm33_spi *spi_drv = os_container_of(device->bus, struct fm33_spi, spi_bus);
    spi_drv->cfg              = configuration;

    return fm33_spi_init(spi_drv, configuration);
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_err_t          state;
    os_size_t         message_length, already_send_length;
    os_uint16_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct fm33_spi  *spi_drv     = os_container_of(device->bus, struct fm33_spi, spi_bus);
    SPI_HandleTypeDef *spi_handle = spi_drv->hspi;

    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

    LOG_D(DBG_TAG, "%s transfer prepare and start", spi_drv->config->bus_name);
    LOG_D(DBG_TAG, "%s sendbuf: %X, recvbuf: %X, length: %d",
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
            state = HAL_SPI_TransmitReceive(spi_handle->instance, (uint8_t *)send_buf, (uint8_t *)recv_buf, send_length, 1000);
        }
        else if (message->send_buf)
        {
            state = HAL_SPI_Transmit(spi_handle->instance, (uint8_t *)send_buf, send_length, 1000);
        }
        else
        {
            memset((uint8_t *)recv_buf, 0xff, send_length);
            state = HAL_SPI_Receive(spi_handle->instance, (uint8_t *)recv_buf, send_length, 1000);
        }

        if (state != OS_EOK)
        {
            LOG_D(DBG_TAG, "spi transfer error : %d\r\n", state);
            message->length   = 0;
        }
        else
        {
            LOG_D(DBG_TAG, "%s transfer done\r\n", spi_drv->config->bus_name);
        }

        /* For simplicity reasons, this example is just waiting till the end of the
           transfer, but application may perform other tasks while transfer operation
           is ongoing. */
        /*while (HAL_SPI_GetState(spi_handle) != HAL_SPI_STATE_READY);*/
    }

    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}

static const struct os_spi_ops fm33_spi_ops =
{
    .configure = spi_configure,
    .xfer      = spixfer,
};

static int fm33_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;

    struct fm33_spi *fm_spi = os_calloc(1, sizeof(struct fm33_spi));

    OS_ASSERT(fm_spi);

    fm_spi->hspi = (SPI_HandleTypeDef *)dev->info;

    struct os_spi_bus *spi_bus = &fm_spi->spi_bus;

    level = os_irq_lock();
    os_list_add_tail(&fm33_spi_list, &fm_spi->list);
    os_irq_unlock(level);

    result = os_spi_bus_register(spi_bus, dev->name, &fm33_spi_ops);
    OS_ASSERT(result == OS_EOK);
    
    LOG_I(DBG_TAG, "%s bus init done.", dev->name);
    return result;
}

OS_DRIVER_INFO fm33_spi_driver =
{
    .name   = "SPI_HandleTypeDef",
    .probe  = fm33_spi_probe,
};

OS_DRIVER_DEFINE(fm33_spi_driver, PREV, OS_INIT_SUBLEVEL_LOW);


