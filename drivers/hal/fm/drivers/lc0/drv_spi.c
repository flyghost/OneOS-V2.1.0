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
#include <drv_spi.h>


#define HAL_SPI_WAIT_TX_FLAG  0x00
#define HAL_SPI_WAIT_RX_FLAG  0x01

static os_list_node_t fm33_spi_list = OS_LIST_INIT(fm33_spi_list);

static os_err_t fm33_spi_init(struct fm33_spi *spi_drv, struct os_spi_configuration *cfg)
{
    os_uint32_t         SPI_APB_CLOCK = 0;
    FL_SPI_InitTypeDef  init_struct = {0};

    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    FL_GPIO_Init(spi_drv->info->port, &spi_drv->info->gpio_init_struct);

    if (spi_drv->info->instance == SPI1)
    {
        SPI_APB_CLOCK = FL_RCC_GetAPB2ClockFreq();
    }
    else if (spi_drv->info->instance == SPI2)
    {
        SPI_APB_CLOCK = FL_RCC_GetAPB1ClockFreq();
    }

    if (cfg->mode & OS_SPI_3WIRE)
    {
        LOG_E(DBG_TAG, "3 wire mode is not supported.");
        return OS_EIO;
    }

    if (cfg->mode & OS_SPI_SLAVE)
    {
        init_struct.mode = FL_SPI_WORK_MODE_SLAVE;
    }
    else
    {
        init_struct.mode = FL_SPI_WORK_MODE_MASTER;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        init_struct.clockPhase = FL_SPI_PHASE_EDGE2;
    }
    else
    {
        init_struct.clockPhase = FL_SPI_PHASE_EDGE1;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        init_struct.clockPolarity = FL_SPI_POLARITY_INVERT;
    }
    else
    {
        init_struct.clockPolarity = FL_SPI_POLARITY_NORMAL;
    }

    if (cfg->mode & OS_SPI_MSB)
    {
        init_struct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    }
    else
    {
        init_struct.bitOrder = FL_SPI_BIT_ORDER_LSB_FIRST;
    }

    switch (cfg->data_width)
    {
        case 8:
            init_struct.dataWidth = FL_SPI_DATA_WIDTH_8B;
            break;
        case 16:
            init_struct.dataWidth = FL_SPI_DATA_WIDTH_16B;
            break;
        case 24:
            init_struct.dataWidth = FL_SPI_DATA_WIDTH_24B;
            break;
        case 32:
            init_struct.dataWidth = FL_SPI_DATA_WIDTH_32B;
            break;
        default:
            return OS_EIO;
    }

    if (cfg->max_hz >= SPI_APB_CLOCK / 2)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV2;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 4)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV4;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 8)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV8;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 16)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV16;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 32)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV32;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 64)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV64;
    }
    else if (cfg->max_hz >= SPI_APB_CLOCK / 128)
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV128;
    }
    else
    {
        init_struct.baudRate = FL_SPI_BAUDRATE_DIV256;
    }

    init_struct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    init_struct.softControl = FL_ENABLE;

    if (FL_SPI_Init(spi_drv->info->instance, &init_struct) != FL_PASS)
    {
        return OS_EIO;
    }

    FL_SPI_ClearTXBuff(spi_drv->info->instance);
    FL_SPI_ClearRXBuff(spi_drv->info->instance);

    return OS_EOK;
}

static os_err_t fm33_spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct fm33_spi *spi_drv = os_container_of(device->bus, struct fm33_spi, spi_bus);
    spi_drv->cfg             = configuration;

    return fm33_spi_init(spi_drv, configuration);
}

os_uint32_t fm33_spi_send_buf(struct fm33_spi *spi_drv, os_uint8_t *buf, os_uint32_t len)
{
    os_uint32_t  count = 0;
    os_uint16_t *buf16 = OS_NULL;
    os_uint32_t *buf32 = OS_NULL;

    switch (spi_drv->cfg->data_width)
    {
        case 8:
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, buf[count]);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
        case 16:
            buf16 = (os_uint16_t *)buf;
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, buf16[count]);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
        case 24:
        case 32:
            buf32 = (os_uint32_t *)buf;
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, buf32[count]);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
    }

    return OS_EOK;
}

os_uint32_t fm33_spi_receive_buf(struct fm33_spi *spi_drv, os_uint8_t *buf, os_uint32_t len)
{
    os_uint32_t  count = 0;
    os_uint16_t *buf16 = OS_NULL;
    os_uint32_t *buf32 = OS_NULL;
    os_uint8_t   dummy = 0;

    switch (spi_drv->cfg->data_width)
    {
        case 8:
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, dummy);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                buf[count] = FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
        case 16:
            buf16 = (os_uint16_t *)buf;
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, dummy);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                buf16[count] = FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
        case 24:
        case 32:
            buf32 = (os_uint32_t *)buf;
            for(count=0;count<len;count++)
            {
                FL_SPI_WriteTXBuff(spi_drv->info->instance, dummy);
                while(!(FL_SPI_IsActiveFlag_TXBuffEmpty(spi_drv->info->instance)));

                while(!(FL_SPI_IsActiveFlag_RXBuffFull(spi_drv->info->instance)));
                buf32[count] = FL_SPI_ReadRXBuff(spi_drv->info->instance);
            }
            break;
    }

    return OS_EOK;
}

static os_uint32_t fm33_spi_xfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_size_t         message_length      = 0;
    os_size_t         already_send_length = 0;
    os_uint16_t       send_length         = 0;
    os_err_t          state               = OS_ERROR;
    os_uint8_t       *recv_buf = OS_NULL;
    const os_uint8_t *send_buf = OS_NULL;
    struct fm33_spi  *spi_drv  = OS_NULL;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    spi_drv = os_container_of(device->bus, struct fm33_spi, spi_bus);

    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

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

        if (message->send_buf && message->recv_buf)
        {
            memset((os_uint8_t *)recv_buf, 0xff, send_length);

            state = fm33_spi_send_buf(spi_drv, (os_uint8_t *)send_buf, send_length);

            if (state != OS_EOK)
            {
                goto done;
            }

            state = fm33_spi_receive_buf(spi_drv, recv_buf, send_length);
        }
        else if (message->send_buf)
        {
            state = fm33_spi_send_buf(spi_drv, (os_uint8_t *)send_buf, send_length);
        }
        else if (message->recv_buf)
        {
            memset((os_uint8_t *)recv_buf, 0xff, send_length);
            state = fm33_spi_receive_buf(spi_drv, recv_buf, send_length);
        }
done:
        if (state != OS_EOK)
        {
            LOG_D(DBG_TAG, "spi transfer error : %d\r\n", state);
            message->length   = 0;
        }
        else
        {
            LOG_D(DBG_TAG, "%s transfer done\r\n", spi_drv->config->bus_name);
        }
    }

    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}

static const struct os_spi_ops fm33_spi_ops =
{
    .configure = fm33_spi_configure,
    .xfer      = fm33_spi_xfer,
};

static int fm33_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t         result  = 0;
    os_base_t        level;
    struct fm33_spi *fm_spi = OS_NULL;

    fm_spi = os_calloc(1, sizeof(struct fm33_spi));
    OS_ASSERT(fm_spi);

    fm_spi->info = (struct fm33_spi_info *)dev->info;

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
    .name   = "SPI_Type",
    .probe  = fm33_spi_probe,
};

OS_DRIVER_DEFINE(fm33_spi_driver, PREV, OS_INIT_SUBLEVEL_LOW);


