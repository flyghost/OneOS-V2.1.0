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
 * @brief       This file implements spi driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "drv_spi.h"

#include "am_mcu_apollo.h"

static am_hal_iom_config_t g_sIOMConfig =
{
    .ui32InterfaceMode  = AM_HAL_IOM_SPIMODE,
    .ui32ClockFrequency = AM_HAL_IOM_100KHZ,
    .bSPHA              = 0,
    .bSPOL              = 0,
    .ui8WriteThreshold  = 4,
    .ui8ReadThreshold   = 60,
};

/* AM spi driver */
struct am_spi_bus
{
    struct os_spi_bus parent;
    os_uint32_t       u32Module;
};

static os_err_t configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    struct am_spi_bus *am_spi_bus = (struct am_spi_bus *)device->bus;
    os_uint32_t        max_hz     = configuration->max_hz;

#if 0
    if(max_hz >= 24000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_24MHZ;
    }
    else if(max_hz >= 16000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_16MHZ;
    }
    else if(max_hz >= 12000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_12MHZ;
    }
    else
#endif
    if (max_hz >= 8000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_8MHZ;
    }
    else if (max_hz >= 6000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_6MHZ;
    }
    else if (max_hz >= 4000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_4MHZ;
    }
    else if (max_hz >= 3000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_3MHZ;
    }
    else if (max_hz >= 2000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_2MHZ;
    }
    else if (max_hz >= 1500000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_1_5MHZ;
    }
    else if (max_hz >= 1000000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_1MHZ;
    }
    else if (max_hz >= 750000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_750KHZ;
    }
    else if (max_hz >= 500000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_500KHZ;
    }
    else if (max_hz >= 400000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_400KHZ;
    }
    else if (max_hz >= 375000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_375KHZ;
    }
    else if (max_hz >= 250000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_250KHZ;
    }
    else if (max_hz >= 100000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_100KHZ;
    }
    else if (max_hz >= 50000)
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_50KHZ;
    }
    else
    {
        g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_10KHZ;
    }

    /* CPOL */
    if (configuration->mode & OS_SPI_CPOL)
    {
        g_sIOMConfig.bSPOL = 1;
    }
    else
    {
        g_sIOMConfig.bSPOL = 0;
    }

    /* CPHA */
    if (configuration->mode & OS_SPI_CPHA)
    {
        g_sIOMConfig.bSPHA = 1;
    }
    else
    {
        g_sIOMConfig.bSPHA = 0;
    }

    /* init SPI */
    am_hal_iom_disable(am_spi_bus->u32Module);
    am_hal_iom_config(am_spi_bus->u32Module, &g_sIOMConfig);
    am_hal_iom_enable(am_spi_bus->u32Module);

    return OS_EOK;
};

static os_uint32_t xfer(struct os_spi_device *device, struct os_spi_message *message)
{
    struct am_spi_bus *am_spi_bus = (struct am_spi_bus *)device->bus;
    /* struct os_spi_configuration * config = &device->config; */
    os_uint32_t *     send_ptr          = (os_uint32_t *)message->send_buf;
    os_uint32_t *     recv_ptr          = message->recv_buf;
    os_uint32_t       u32BytesRemaining = message->length;
    os_uint32_t       u32TransferSize   = 0;

    /* take CS */
    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

    if (recv_ptr != OS_NULL)
    {
        while (u32BytesRemaining)
        {
            /* Set the transfer size to either 64, or the number of remaining
               bytes, whichever is smaller */
            if (u32BytesRemaining > 64)
            {
                u32TransferSize = 64;
                am_hal_iom_spi_read(am_spi_bus->u32Module,
                                    device->cs_pin,
                                    (uint32_t *)recv_ptr,
                                    u32TransferSize,
                                    AM_HAL_IOM_RAW);
            }
            else
            {
                u32TransferSize = u32BytesRemaining;
                {
                    am_hal_iom_spi_read(am_spi_bus->u32Module,
                                        device->cs_pin,
                                        (uint32_t *)recv_ptr,
                                        u32TransferSize,
                                        AM_HAL_IOM_RAW);
                }
            }

            u32BytesRemaining -= u32TransferSize;
            recv_ptr = (os_uint32_t *)((os_uint32_t)recv_ptr + u32TransferSize);
        }
    }
    else
    {
        while (u32BytesRemaining)
        {
            /* Set the transfer size to either 32, or the number of remaining
               bytes, whichever is smaller */
            if (u32BytesRemaining > 64)
            {
                u32TransferSize = 64;
                am_hal_iom_spi_write(am_spi_bus->u32Module,
                                     device->cs_pin,
                                     (uint32_t *)send_ptr,
                                     u32TransferSize,
                                     AM_HAL_IOM_RAW);
            }
            else
            {
                u32TransferSize = u32BytesRemaining;
                {
                    am_hal_iom_spi_write(am_spi_bus->u32Module,
                                         device->cs_pin,
                                         (uint32_t *)send_ptr,
                                         u32TransferSize,
                                         AM_HAL_IOM_RAW);
                }
            }

            u32BytesRemaining -= u32TransferSize;
            send_ptr = (os_uint32_t *)((os_uint32_t)send_ptr + u32TransferSize);
        }
    }

    /* release CS */
    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}

static const struct os_spi_ops am_spi_ops =
{
    configure,
    xfer
};

#ifdef BSP_USING_SPI0
static struct am_spi_bus am_spi_bus_0 = 
{
    {0},
    AM_SPI0_IOM_INST
};
#endif /* #ifdef RT_USING_SPI0 */

#ifdef BSP_USING_SPI1
static struct am_spi_bus am_spi_bus_1 =
{
    {0},
    AM_SPI1_IOM_INST
};
#endif /* #ifdef RT_USING_SPI1 */

int os_hw_spi_init(void)
{
    struct am_spi_bus *am_spi;

#ifdef BSP_USING_SPI0
    /* init spi gpio */
    am_hal_gpio_pin_config(SPI0_GPIO_SCK, SPI0_GPIO_CFG_SCK);
    am_hal_gpio_pin_config(SPI0_GPIO_MISO, SPI0_GPIO_CFG_MISO | AM_HAL_GPIO_PULL6K);
    am_hal_gpio_pin_config(SPI0_GPIO_MOSI, SPI0_GPIO_CFG_MOSI | AM_HAL_GPIO_PULL6K);

    /* Initialize IOM 0 in SPI mode at 100KHz */
    am_hal_iom_pwrctrl_enable(AM_SPI0_IOM_INST);
    am_hal_iom_config(AM_SPI0_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_SPI0_IOM_INST);

    /* init spi bus device */
    am_spi = &am_spi_bus_0;
    os_spi_bus_register(&am_spi->parent, "spi0", &am_spi_ops);
#endif

#ifdef BSP_USING_SPI1
    /* init spi gpio */
    am_hal_gpio_pin_config(SPI1_GPIO_SCK, SPI1_GPIO_CFG_SCK);
    am_hal_gpio_pin_config(SPI1_GPIO_MISO, SPI1_GPIO_CFG_MISO);
    am_hal_gpio_pin_config(SPI1_GPIO_MOSI, SPI1_GPIO_CFG_MOSI);
    am_hal_gpio_pin_config(SPI1_GPIO_CE0, SPI1_GPIO_CFG_CE0);

    /* Initialize IOM 0 in SPI mode at 100KHz */
    am_hal_iom_pwrctrl_enable(AM_SPI1_IOM_INST);
    am_hal_iom_config(AM_SPI1_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_SPI1_IOM_INST);

    /* init spi bus device */
    am_spi = &am_spi_bus_1;
    os_spi_bus_register(&am_spi->parent, "spi1", &am_spi_ops);
#endif

    os_kprintf("spi init!\n");

    return 0;
}
