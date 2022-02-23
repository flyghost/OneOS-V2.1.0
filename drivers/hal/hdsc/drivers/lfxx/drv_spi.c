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
 * @brief       This file implements spi driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_spi.h>

#define DBG_TAG "drv.spi"
#include <dlog.h>

static os_list_node_t hc32_spi_list = OS_LIST_INIT(hc32_spi_list);

typedef struct hc32_spi
{
    struct os_spi_bus spi_bus;

    struct hc32_spi_info *info;
    struct os_spi_configuration *cfg;

    os_list_node_t list;
} hc32_spi_t;

static os_err_t hc32_spi_init(struct hc32_spi *spi_drv)
{
    stc_spi_cfg_t  SpiInitStruct;
    struct os_spi_configuration *cfg;
    uint32_t u32Pclk;

    cfg = spi_drv->cfg;

    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    Reset_RstPeripheral0(spi_drv->info->reset);

    if (cfg->mode & OS_SPI_SLAVE)
    {
        SpiInitStruct.enSpiMode = SpiMskSlave;
    }
    else
    {
        SpiInitStruct.enSpiMode = SpiMskMaster;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        SpiInitStruct.enCPHA    = SpiMskCphasecond;
    }
    else
    {
        SpiInitStruct.enCPHA    = SpiMskCphafirst;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        SpiInitStruct.enCPOL    = SpiMskcpolhigh;
    }
    else
    {
        SpiInitStruct.enCPOL    = SpiMskcpollow;
    }

    u32Pclk = Sysctrl_GetPClkFreq();

    if (cfg->max_hz >= u32Pclk / 4)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv4;
    }
    else if (cfg->max_hz >= u32Pclk / 8)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv8;
    }
    else if (cfg->max_hz >= u32Pclk / 16)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv16;
    }
    else if (cfg->max_hz >= u32Pclk / 32)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv32;
    }
    else if (cfg->max_hz >= u32Pclk / 64)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv64;
    }
    else if (cfg->max_hz >= u32Pclk / 128)
    {
        SpiInitStruct.enPclkDiv = SpiClkMskDiv128;
    }
    else
    {
        return OS_ERROR;
    }

    LOG_D(DBG_TAG, "sys freq: %d, pclk2 freq: %d, SPI limiting freq: %d, BaudRatePrescaler: %d",
          Sysctrl_GetPClkFreq(),
          u32Pclk,
          cfg->max_hz,
          SpiInitStruct.enPclkDiv);

    if(Spi_Init(spi_drv->info->spi_base, &SpiInitStruct) != Ok)
    {
        return OS_EIO;
    }

    LOG_D(DBG_TAG, "%s init done", spi_drv->info->name);
    return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct hc32_spi *spi_drv = os_container_of(device->bus, struct hc32_spi, spi_bus);
    spi_drv->cfg              = configuration;

    return hc32_spi_init(spi_drv);
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    en_result_t state;
    os_size_t         message_length, already_send_length;
    os_uint32_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct hc32_spi  *spi_drv    = os_container_of(device->bus, struct hc32_spi, spi_bus);
        
    if (message->cs_take)
    {
        /*must called,otherwise spi driver will loop*/
        Spi_SetCS(spi_drv->info->spi_base, FALSE);
        
        os_pin_write(device->cs_pin, PIN_LOW);
    }
    
    LOG_D(DBG_TAG, "%s transfer prepare and start", spi_drv->info->name);
    LOG_D(DBG_TAG, "%s sendbuf: %X, recvbuf: %X, length: %d",
          spi_drv->info->name,
          (uint32_t)message->send_buf,
          (uint32_t)message->recv_buf,
          message->length);

    message_length = message->length;
    recv_buf       = message->recv_buf;
    send_buf       = message->send_buf;
    while (message_length)
    {
        /* the HC library use uint32 to save the data length */
        if (message_length > (uint32_t)(~0))
        {
            send_length    = (uint32_t)(~0);
            message_length = message_length - (uint32_t)(~0);
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
            memset((uint8_t *)recv_buf, 0xff, send_length);

            state = Spi_SendBuf(spi_drv->info->spi_base, (uint8_t *)send_buf, send_length);

            if(state != Ok)
                goto done;

            state = Spi_ReceiveBuf(spi_drv->info->spi_base, (uint8_t *)recv_buf, send_length);
        }
        else if (message->send_buf)
        {
            state = Spi_SendBuf(spi_drv->info->spi_base, (uint8_t *)send_buf, send_length);
        }
        else
        {
            memset((uint8_t *)recv_buf, 0xff, send_length);
            state = Spi_ReceiveBuf(spi_drv->info->spi_base, (uint8_t *)recv_buf, send_length);
        }
done:
        if (state != Ok)
        {
            LOG_I(DBG_TAG, "spi transfer error : %d", state);
            message->length   = 0;
        }
        else
        {
            LOG_D(DBG_TAG, "%s transfer done", spi_drv->info->name);
        }

    }

    if (message->cs_release)
    {
        /*must called,otherwise spi driver will loop*/
        Spi_SetCS(spi_drv->info->spi_base, TRUE);
        
        os_pin_write(device->cs_pin, PIN_HIGH);
    }
    
    return message->length;
}

static const struct os_spi_ops hc32_spi_ops = {
    .configure = spi_configure,
    .xfer      = spixfer,
};

static inline void __os_hw_spi_init(struct hc32_spi *spi_drv)
{
    stc_gpio_cfg_t GpioInitStruct;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    Sysctrl_SetPeripheralGate(spi_drv->info->peripheral, TRUE);

    DDL_ZERO_STRUCT(GpioInitStruct);

    GpioInitStruct.enDrv = GpioDrvH;
    GpioInitStruct.enDir = GpioDirOut;

    Gpio_Init(spi_drv->info->port_sck, spi_drv->info->pin_sck, &GpioInitStruct);
    Gpio_SetAfMode(spi_drv->info->port_sck, spi_drv->info->pin_sck, spi_drv->info->gpio_af);

    Gpio_Init(spi_drv->info->port_mosi, spi_drv->info->pin_mosi, &GpioInitStruct);
    Gpio_SetAfMode(spi_drv->info->port_mosi, spi_drv->info->pin_mosi, spi_drv->info->gpio_af);

    GpioInitStruct.enDir = GpioDirIn;
    Gpio_Init(spi_drv->info->port_miso, spi_drv->info->pin_miso, &GpioInitStruct);
    Gpio_SetAfMode(spi_drv->info->port_miso, spi_drv->info->pin_miso, spi_drv->info->gpio_af);
}+

static int hc32_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t result  = OS_ERROR;
    os_base_t   level;

    struct hc32_spi *hc_spi = os_calloc(1, sizeof(struct hc32_spi));

    OS_ASSERT(hc_spi);

    hc_spi->info = (struct hc32_spi_info *)dev->info;

    struct os_spi_bus *spi_bus = &hc_spi->spi_bus;

    level = os_irq_lock();
    os_list_add_tail(&hc32_spi_list, &hc_spi->list);
    os_irq_unlock(level);

    __os_hw_spi_init(hc_spi);

    result = os_spi_bus_register(spi_bus, dev->name, &hc32_spi_ops);
    OS_ASSERT(result == OS_EOK);

    LOG_D(DBG_TAG, "%s spi bus init done", dev->name);

    return result;
}

OS_DRIVER_INFO hc32_spi_driver = {
    .name   = "SPI_HandleTypeDef",
    .probe  = hc32_spi_probe,
};

OS_DRIVER_DEFINE(hc32_spi_driver, PREV, OS_INIT_SUBLEVEL_HIGH);
