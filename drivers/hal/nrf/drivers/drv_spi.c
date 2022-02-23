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
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "board.h"
#include "drv_spi.h"
#include <os_memory.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define LOG_TAG "drv.spi"
#include <drv_log.h>

struct nrf5_spi
{
    struct os_spi_bus spi_bus;

    struct nrf5_spi_info *spi_info;
    struct nrf5_spi_config     *config;
    struct os_spi_configuration *cfg;

    os_uint8_t     spi_dma_flag;
    os_list_node_t list;
};

static os_list_node_t nrf5_spi_list = OS_LIST_INIT(nrf5_spi_list);


#ifdef BSP_USING_SPI0
#define NRFX_SPI0_CONFIG         \
{                                \
    .bus_name = "spi0",          \
    .spi = NRFX_SPI_INSTANCE(0)  \
}
#endif
#ifdef BSP_USING_SPI1
#define NRFX_SPI1_CONFIG         \
{                                \
    .bus_name = "spi1",          \
    .spi = NRFX_SPI_INSTANCE(1)  \
}
#endif

#ifdef BSP_USING_SPI2
#define NRFX_SPI2_CONFIG         \
{                                \
    .bus_name = "spi2",          \
    .spi = NRFX_SPI_INSTANCE(2)  \
}
#endif

struct nrfx_drv_spi_config
{
    char *bus_name;
    nrfx_spi_t spi;
};

struct nrfx_drv_spi
{
    nrfx_spi_t spi;    /* nrfx spi driver instance. */
    nrfx_spi_config_t   spi_config; /* nrfx spi config Configuration */
    struct os_spi_configuration *cfg;
    struct os_spi_bus spi_bus;
};

static struct nrfx_drv_spi_config spi_config[] =
{
#ifdef BSP_USING_SPI0
    NRFX_SPI0_CONFIG,
#endif

#ifdef BSP_USING_SPI1
    NRFX_SPI1_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    NRFX_SPI2_CONFIG,
#endif

};

/**
 * spi event handler function
 */
static void spi0_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    return;
}

static void spi1_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    return;
}

static void spi2_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    return;
}
nrfx_spi_evt_handler_t spi_handler[] = {spi0_handler, spi1_handler, spi2_handler};

/**
  * @brief  This function config spi bus
  * @param  device
  * @param  configuration
  * @retval OS_EOK / OS_ERROR
  */
static os_err_t spi_configure(struct os_spi_device *device,
                              struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct nrf5_spi *spi_drv = os_container_of(device->bus, struct nrf5_spi, spi_bus);
    spi_drv->cfg = configuration;
    struct nrf5_spi_info *spi_info;
    spi_info = spi_drv->spi_info;

    nrfx_spi_t spi = spi_info->spi_dev;
    nrfx_spi_config_t config = NRFX_SPI_DEFAULT_CONFIG;
    config.sck_pin = spi_info->sck_pin;
    config.mosi_pin = spi_info->mosi_pin;
    config.miso_pin = spi_info->miso_pin;
    config.ss_pin = NRFX_SPI_PIN_NOT_USED;

    /* spi config bit order */
    if(configuration->mode & OS_SPI_MSB)
    {
        config.bit_order = NRF_SPI_BIT_ORDER_MSB_FIRST;
    }
    else
    {
        config.bit_order = NRF_SPI_BIT_ORDER_LSB_FIRST;
    }
    /* spi mode config */
    switch (configuration->mode & OS_SPI_MODE_3 )
    {
    case OS_SPI_MODE_0/* OS_SPI_CPOL:0 , OS_SPI_CPHA:0 */:
        config.mode = NRF_SPI_MODE_0;
        break;
    case OS_SPI_MODE_1/* OS_SPI_CPOL:0 , OS_SPI_CPHA:1 */:
        config.mode = NRF_SPI_MODE_1;
        break;
    case OS_SPI_MODE_2/* OS_SPI_CPOL:1 , OS_SPI_CPHA:0 */:
        config.mode = NRF_SPI_MODE_2;
        break;
    case OS_SPI_MODE_3/* OS_SPI_CPOL:1 , OS_SPI_CPHA:1 */:
        config.mode = NRF_SPI_MODE_3;
        break;
    default:
        os_kprintf("spi_configure mode error %x\n",configuration->mode);
        return OS_ERROR;
    }
    /* spi frequency config */
    switch (configuration->max_hz / 1000)
    {
    case 125:
        config.frequency = NRF_SPI_FREQ_125K;
        break;
    case 250:
        config.frequency = NRF_SPI_FREQ_250K;
        break;
    case 500:
        config.frequency = NRF_SPI_FREQ_500K;
        break;
    case 1000:
        config.frequency = NRF_SPI_FREQ_1M;
        break;
    case 2000:
        config.frequency = NRF_SPI_FREQ_2M;
        break;
    case 4000:
        config.frequency = NRF_SPI_FREQ_4M;
        break;
    case 8000:
        config.frequency = NRF_SPI_FREQ_8M;
        break;
    default:
        config.frequency = NRF_SPI_FREQ_4M;
        break;
    }

    nrfx_spi_evt_handler_t handler = OS_NULL;    //spi send callback handler ,default NULL
    void * context = OS_NULL;
    nrfx_err_t nrf_ret = nrfx_spi_init(&spi, &config, handler, context);
    if(NRFX_SUCCESS == nrf_ret)
        return OS_EOK;

    return OS_ERROR;
}

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    nrfx_err_t nrf_ret;

    struct nrf5_spi *spi_drv = os_container_of(device->bus, struct nrf5_spi, spi_bus);
    struct nrf5_spi_info *spi_info;
    spi_info = spi_drv->spi_info;

    nrfx_spi_t * p_instance =  &spi_info->spi_dev;
    nrfx_spi_xfer_desc_t p_xfer_desc;

    if(message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }
    
    p_xfer_desc.p_rx_buffer = message->recv_buf;
    p_xfer_desc.rx_length = message->length;
    p_xfer_desc.p_tx_buffer = message->send_buf;
    p_xfer_desc.tx_length = message->length ;
    
    if(message->send_buf == OS_NULL)
    {
        p_xfer_desc.tx_length = 0;
    }
    
    if(message->recv_buf == OS_NULL)
    {
        p_xfer_desc.rx_length = 0;
    }

    nrf_ret = nrfx_spi_xfer(p_instance, &p_xfer_desc, 0);
    
    if(message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    if( NRFX_SUCCESS != nrf_ret)
    {
        return 0;
    }
    else
    {
        return message->length;
    }
}

/* spi bus callback function  */
static const struct os_spi_ops nrf5_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};

static int nrf5_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;
    os_base_t   level;

    struct nrf5_spi *nrf_spi = os_calloc(1, sizeof(struct nrf5_spi));

    OS_ASSERT(nrf_spi);

    nrf_spi->spi_info = (struct nrf5_spi_info *)dev->info;

    struct os_spi_bus *spi_bus = &nrf_spi->spi_bus;

    level = os_irq_lock();
    os_list_add_tail(&nrf5_spi_list, &nrf_spi->list);
    os_irq_unlock(level);

    result = os_spi_bus_register(spi_bus, dev->name, &nrf5_spi_ops);
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO nrf5_spi_driver = {
    .name   = "SPI_HandleTypeDef",
    .probe  = nrf5_spi_probe,
};

OS_DRIVER_DEFINE(nrf5_spi_driver,PREV,OS_INIT_SUBLEVEL_LOW);
