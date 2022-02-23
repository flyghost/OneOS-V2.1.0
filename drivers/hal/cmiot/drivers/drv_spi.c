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
 * @brief       This file implements spi driver for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_spi.h>

#define DBG_TAG "drv.spi"
#include <dlog.h>

#define SPI_DUMMY_BYTE 0xA5

struct cm32_spi spis[] = {
#ifdef  BSP_USING_SPI1
    {
        .spi_base = SPI1,
        .port_cs = SPI1_CS_PORT,
        .pin_cs = SPI1_CS_PIN,
        .cs_af = SPI1_CS_AF,
        .port_sck = SPI1_SCK_PORT,
        .pin_sck = SPI1_SCK_PIN,
        .sck_af = SPI1_SCK_AF,
        .port_miso = SPI1_MISO_PORT,
        .pin_miso = SPI1_MISO_PIN,
        .miso_af = SPI1_MISO_AF,
        .port_mosi = SPI1_MOSI_PORT,
        .pin_mosi = SPI1_MOSI_PIN,
        .mosi_af = SPI1_MOSI_AF,
        .name = "spi1",
    },
#endif

#ifdef  BSP_USING_SPI2
    {
        .spi_base = SPI2,
        .port_cs = SPI2_CS_PORT,
        .pin_cs = SPI2_CS_PIN,
        .cs_af = SPI2_CS_AF,
        .port_sck = SPI2_SCK_PORT,
        .pin_sck = SPI2_SCK_PIN,
        .sck_af = SPI2_SCK_AF,
        .port_miso = SPI2_MISO_PORT,
        .pin_miso = SPI2_MISO_PIN,
        .miso_af = SPI2_MISO_AF,
        .port_mosi = SPI2_MOSI_PORT,
        .pin_mosi = SPI2_MOSI_PIN,
        .mosi_af = SPI2_MOSI_AF,
        .name = "spi2",
    },
#endif
};

static os_err_t cm32_spi_init(struct cm32_spi *spi_drv)
{
    struct os_spi_configuration *cfg;
    SPI_InitType SPI_InitStructure;
    uint32_t spi_clk;
    RCC_ClocksType clks;

    cfg = spi_drv->cfg;

    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    /* SPI configuration */
    SPI_InitStructure.DataDirection = SPI_DIR_DOUBLELINE_FULLDUPLEX;

    if (cfg->mode & OS_SPI_SLAVE)
    {
        SPI_InitStructure.SpiMode = SPI_MODE_SLAVE;
    }
    else
    {
        SPI_InitStructure.SpiMode = SPI_MODE_MASTER;
    }

    if (cfg->data_width == 8)
    {
        SPI_InitStructure.DataLen = SPI_DATA_SIZE_8BITS;
    }
    else if (cfg->data_width == 16)
    {
        SPI_InitStructure.DataLen = SPI_DATA_SIZE_16BITS;
    }
    else
    {
        return OS_EIO;
    }

    if (cfg->mode & OS_SPI_CPOL)
    {
        SPI_InitStructure.CLKPOL = SPI_CLKPOL_HIGH;
    }
    else
    {
        SPI_InitStructure.CLKPOL = SPI_CLKPOL_LOW;
    }

    if (cfg->mode & OS_SPI_CPHA)
    {
        SPI_InitStructure.CLKPHA = SPI_CLKPHA_SECOND_EDGE;
    }
    else
    {
        SPI_InitStructure.CLKPHA = SPI_CLKPHA_FIRST_EDGE;
    }

    SPI_InitStructure.NSS = SPI_NSS_SOFT;

    RCC_GetClocksFreqValue(&clks);
    spi_clk = (uint32_t)clks.Pclk2Freq;

    if (cfg->max_hz >= spi_clk / 2)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_2;
    }
    else if (cfg->max_hz >= spi_clk / 4)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_4;
    }
    else if (cfg->max_hz >= spi_clk / 8)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_8;
    }
    else if (cfg->max_hz >= spi_clk / 16)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_16;
    }
    else if (cfg->max_hz >= spi_clk / 32)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_32;
    }
    else if (cfg->max_hz >= spi_clk / 64)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_64;
    }
    else if (cfg->max_hz >= spi_clk / 128)
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_128;
    }
    else
    {
        SPI_InitStructure.BaudRatePres = SPI_BR_PRESCALER_256;
    }

    if (cfg->mode & OS_SPI_MSB)
    {
        SPI_InitStructure.FirstBit = SPI_FB_MSB;
    }
    else
    {
        SPI_InitStructure.FirstBit = SPI_FB_LSB;
    }
    SPI_InitStructure.CRCPoly = 7;
    SPI_Init(spi_drv->spi_base, &SPI_InitStructure);

    /* Enable the SPI */
    SPI_Enable(spi_drv->spi_base, ENABLE);

    LOG_D(DBG_TAG, "%s init done", spi_drv->name);
    return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct cm32_spi *spi_drv = os_container_of(device->bus, struct cm32_spi, spi_bus);
    spi_drv->cfg              = configuration;

    return cm32_spi_init(spi_drv);
}

static os_err_t spi_sendbuf(SPI_Module* SPIx, uint8_t* pu8Buf, uint32_t u32Len)
{
    uint32_t u32Index = 0;

    for(u32Index = 0; u32Index < u32Len; u32Index++)
    {
        while (SPI_I2S_GetStatus(SPIx, SPI_I2S_TE_FLAG) == RESET)
            ;
        SPI_I2S_TransmitData(SPIx, pu8Buf[u32Index]);

        while (SPI_I2S_GetStatus(SPIx, SPI_I2S_RNE_FLAG) == RESET)
            ;
        SPI_I2S_ReceiveData(SPIx);
    }

    return OS_EOK;
}

static os_err_t spi_recvbuf(SPI_Module* SPIx, uint8_t* pu8Buf, uint32_t u32Len)
{
    while (u32Len--)
    {
        while (SPI_I2S_GetStatus(SPIx, SPI_I2S_TE_FLAG) == RESET)
            ;
        SPI_I2S_TransmitData(SPIx, SPI_DUMMY_BYTE);

        while (SPI_I2S_GetStatus(SPIx, SPI_I2S_RNE_FLAG) == RESET)
            ;
        *pu8Buf = SPI_I2S_ReceiveData(SPIx);
        pu8Buf++;
    }

    return OS_EOK;
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_err_t state;
    os_size_t         message_length, already_send_length;
    os_uint32_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct cm32_spi *spi_drv = os_container_of(device->bus, struct cm32_spi, spi_bus);

    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

    LOG_D(DBG_TAG, "%s transfer prepare and start", spi_drv->name);
    LOG_D(DBG_TAG, "%s sendbuf: %X, recvbuf: %X, length: %d",
              spi_drv->name,
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

            state = spi_sendbuf(spi_drv->spi_base, (uint8_t *)send_buf, send_length);

            if(state != OS_EOK)
                goto done;

            state = spi_recvbuf(spi_drv->spi_base, (uint8_t *)recv_buf, send_length);
        }
        else if (message->send_buf)
        {
            state = spi_sendbuf(spi_drv->spi_base, (uint8_t *)send_buf, send_length);
        }
        else
        {
            memset((uint8_t *)recv_buf, 0xff, send_length);
            state = spi_recvbuf(spi_drv->spi_base, (uint8_t *)recv_buf, send_length);
        }
done:
        if (state != OS_EOK)
        {
            LOG_I(DBG_TAG, "spi transfer error : %d", state);
            message->length   = 0;
        }

        else
        {
            LOG_D(DBG_TAG, "%s transfer done", spi_drv->name);
        }

    }

    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}

static const struct os_spi_ops cm32_spi_ops = {
    .configure = spi_configure,
    .xfer      = spixfer,
};

static inline void cm32_spi_pin_init(struct cm32_spi *spi_drv)
{
    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    uint32_t RCC_APB2_PERIPH_CLK_PORT, RCC_APB2_PERIPH_MISO_PORT, RCC_APB2_PERIPH_MOSI_PORT;

    /* sck */
    if (spi_drv->port_sck == GPIOA)
    {
        RCC_APB2_PERIPH_CLK_PORT = RCC_APB2_PERIPH_GPIOA;
    }
    else if (spi_drv->port_sck == GPIOB)
    {
        RCC_APB2_PERIPH_CLK_PORT = RCC_APB2_PERIPH_GPIOB;
    }
    else if (spi_drv->port_sck == GPIOC)
    {
        RCC_APB2_PERIPH_CLK_PORT = RCC_APB2_PERIPH_GPIOC;
    }
    else
    {
        RCC_APB2_PERIPH_CLK_PORT = RCC_APB2_PERIPH_GPIOD;
    }

    /* miso */
    if (spi_drv->port_miso == GPIOA)
    {
        RCC_APB2_PERIPH_MISO_PORT = RCC_APB2_PERIPH_GPIOA;
    }
    else if (spi_drv->port_miso == GPIOB)
    {
        RCC_APB2_PERIPH_MISO_PORT = RCC_APB2_PERIPH_GPIOB;
    }
    else if (spi_drv->port_miso == GPIOC)
    {
        RCC_APB2_PERIPH_MISO_PORT = RCC_APB2_PERIPH_GPIOC;
    }
    else
    {
        RCC_APB2_PERIPH_MISO_PORT = RCC_APB2_PERIPH_GPIOD;
    }

    /* mosi */
    if (spi_drv->port_mosi == GPIOA)
    {
        RCC_APB2_PERIPH_MOSI_PORT = RCC_APB2_PERIPH_GPIOA;
    }
    else if (spi_drv->port_miso == GPIOB)
    {
        RCC_APB2_PERIPH_MOSI_PORT = RCC_APB2_PERIPH_GPIOB;
    }
    else if (spi_drv->port_miso == GPIOC)
    {
        RCC_APB2_PERIPH_MOSI_PORT = RCC_APB2_PERIPH_GPIOC;
    }
    else
    {
        RCC_APB2_PERIPH_MOSI_PORT = RCC_APB2_PERIPH_GPIOD;
    }

    RCC_EnableAPB2PeriphClk(
        RCC_APB2_PERIPH_CLK_PORT | RCC_APB2_PERIPH_MISO_PORT | RCC_APB2_PERIPH_MOSI_PORT, ENABLE);

    if (spi_drv->spi_base == SPI1)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_SPI1 | RCC_APB2_PERIPH_AFIO, ENABLE);
    }
    else
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_SPI2 | RCC_APB2_PERIPH_AFIO, ENABLE);
    }

    GPIO_InitStructure.Pin        = spi_drv->pin_sck;
    //GPIO_InitStructure.GPIO_Pull  = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Current  = GPIO_DC_12mA;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = spi_drv->sck_af;
    GPIO_InitPeripheral(spi_drv->port_sck, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = spi_drv->pin_mosi;
    GPIO_InitStructure.GPIO_Alternate = spi_drv->mosi_af;
    GPIO_InitPeripheral(spi_drv->port_mosi, &GPIO_InitStructure);

    GPIO_InitStructure.Pin       = spi_drv->pin_miso;
    GPIO_InitStructure.GPIO_Alternate = spi_drv->miso_af;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitPeripheral(spi_drv->port_miso, &GPIO_InitStructure);
}

int os_hw_spi_init(void)
{
    os_err_t result  = OS_ERROR;
    os_uint32_t idx = 0;
    struct os_spi_bus *spi_bus;

    for(idx = 0; idx < (sizeof(spis) / sizeof(spis[0])); idx++)
    {
        spi_bus = &(spis[idx].spi_bus);

        cm32_spi_pin_init(&(spis[idx]));

        result = os_spi_bus_register(spi_bus, spis[idx].name, &cm32_spi_ops);

        if(result != OS_EOK)
        {
            LOG_D(DBG_TAG, "%s bus init failed", spis[idx].name);
            break;
        }
    }

    LOG_D(DBG_TAG, "spi bus init done");

    return result;
}

OS_DEVICE_INIT(os_hw_spi_init, OS_INIT_SUBLEVEL_HIGH);
