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
 * @brief       This file implements SPI driver for gd32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <gd32f4xx.h>
#include <gd32f4xx_spi.h>
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>
#include <os_stddef.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <drv_spi.h>
#include <string.h>
#include <pin/pin.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define LOG_TAG "drv.spi"
#include <drv_log.h>

struct gd32_spi
{
    struct os_spi_bus spi_bus;

    struct gd_spi_info *spi_info;
    struct gd32_spi_config     *config;
    struct os_spi_configuration *cfg;

    os_uint8_t     spi_dma_flag;
    os_list_node_t list;
};

static os_list_node_t gd32_spi_list = OS_LIST_INIT(gd32_spi_list);

static os_err_t gd32_spi_init(struct gd32_spi *spi_drv, struct os_spi_configuration *cfg)
{
    os_uint32_t spi_periph;
    spi_parameter_struct spi_init_struct;
    struct gd_spi_info *spi_info;

    OS_ASSERT(spi_drv != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    spi_info = spi_drv->spi_info;

    spi_periph = (os_uint32_t)spi_info->hspi;

    rcu_periph_clock_enable(spi_info->rcu_spi_base);
    rcu_periph_clock_enable(spi_info->rcu_spi_gpio_base);
    gpio_af_set(spi_info->spi_pin_base, spi_info->alternate_functions, spi_info->spi_sck_pin|spi_info->spi_miso_pin| spi_info->spi_mosi_pin);
    gpio_mode_set(spi_info->spi_pin_base, GPIO_MODE_AF, GPIO_PUPD_NONE, spi_info->spi_sck_pin|spi_info->spi_miso_pin| spi_info->spi_mosi_pin);
    gpio_output_options_set(spi_info->spi_pin_base, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, spi_info->spi_sck_pin|spi_info->spi_miso_pin| spi_info->spi_mosi_pin);
    //gpio_mode_set(spi_info->spi_pin_base, GPIO_MODE_INPUT, GPIO_PUPD_NONE, spi_info->spi_miso_pin);
    //gpio_output_options_set(spi_info->spi_pin_base, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, spi_info->spi_miso_pin);

    if(cfg->data_width <= 8)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    }
    else if(cfg->data_width <= 16)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        return OS_EIO;
    }

    {
        rcu_clock_freq_enum spi_src;
        os_uint32_t spi_apb_clock;
        os_uint32_t max_hz;

        max_hz = cfg->max_hz;

        LOG_EXT_D("sys   freq: %d\n", rcu_clock_freq_get(CK_SYS));
        LOG_EXT_D("CK_APB2 freq: %d\n", rcu_clock_freq_get(CK_APB2));
        LOG_EXT_D("max   freq: %d\n", max_hz);

        if (spi_periph == SPI1 || spi_periph == SPI2)
        {
            spi_src = CK_APB1;
        }
        else
        {
            spi_src = CK_APB2;
        }

        spi_apb_clock = rcu_clock_freq_get(spi_src);

        if(max_hz >= spi_apb_clock/2)
        {
            spi_init_struct.prescale = SPI_PSC_2;
        }
        else if (max_hz >= spi_apb_clock/4)
        {
            spi_init_struct.prescale = SPI_PSC_4;
        }
        else if (max_hz >= spi_apb_clock/8)
        {
            spi_init_struct.prescale = SPI_PSC_8;
        }
        else if (max_hz >= spi_apb_clock/16)
        {
            spi_init_struct.prescale = SPI_PSC_16;
        }
        else if (max_hz >= spi_apb_clock/32)
        {
            spi_init_struct.prescale = SPI_PSC_32;
        }
        else if (max_hz >= spi_apb_clock/64)
        {
            spi_init_struct.prescale = SPI_PSC_64;
        }
        else if (max_hz >= spi_apb_clock/128)
        {
            spi_init_struct.prescale = SPI_PSC_128;
        }
        else
        {
            /*  min prescaler 256 */
            spi_init_struct.prescale = SPI_PSC_256;
        }
    } /* baudrate */

    switch(cfg->mode & OS_SPI_MODE_3)
    {
        case OS_SPI_MODE_0:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
            break;
        case OS_SPI_MODE_1:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
            break;
        case OS_SPI_MODE_2:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_1EDGE;
            break;
        case OS_SPI_MODE_3:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
            break;
    }

    /* MSB or LSB */
    if(cfg->mode & OS_SPI_MSB)
    {
        spi_init_struct.endian = SPI_ENDIAN_MSB;
    }
    else
    {
        spi_init_struct.endian = SPI_ENDIAN_LSB;
    }

    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.nss = SPI_NSS_SOFT;

    spi_init(spi_periph, &spi_init_struct);

    spi_crc_off(spi_periph);

    spi_enable(spi_periph);

    return OS_EOK;
}

static os_err_t spi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct gd32_spi *spi_drv = os_container_of(device->bus, struct gd32_spi, spi_bus);
    spi_drv->cfg              = configuration;

    return gd32_spi_init(spi_drv, configuration);
}


uint32_t spi_send_buf(struct gd32_spi *spi_drv,uint8_t *buf,uint32_t len)
{
    uint32_t count=0;

    os_uint32_t spi_periph = spi_drv->spi_info->hspi;

    for(count=0;count<len;count++){


        /* loop while data register in not emplty */
        while (RESET == spi_i2s_flag_get(spi_periph,SPI_FLAG_TBE));

        if(spi_drv->cfg->data_width == 16){            
            /* send byte through the SPI peripheral */
            spi_i2s_data_transmit(spi_periph,*(uint16_t *)(buf+count*sizeof(uint16_t)));

        }else{
            /* send byte through the SPI peripheral */
            spi_i2s_data_transmit(spi_periph,*(uint8_t *)(buf+count*sizeof(uint8_t)));
        }
        /* wait to receive a byte */
        while(RESET == spi_i2s_flag_get(spi_periph,SPI_FLAG_RBNE));
        spi_i2s_data_receive(spi_periph);

    }
    /* return the byte read from the SPI bus */
    return OS_EOK;
}


uint32_t spi_receive_buf(struct gd32_spi *spi_drv,uint8_t *buf,uint32_t len)
{
    uint32_t count=0;    

    os_uint32_t spi_periph = spi_drv->spi_info->hspi;

    for(count=0;count<len;count++){

        /* loop while data register in not emplty */
        while (RESET == spi_i2s_flag_get(spi_periph,SPI_FLAG_TBE));

        /* send byte through the SPI peripheral */
        spi_i2s_data_transmit(spi_periph,DUMMY_BYTE);

        /* wait to receive a byte */
        while(RESET == spi_i2s_flag_get(spi_periph,SPI_FLAG_RBNE));

        if(spi_drv->cfg->data_width == 16)
            *(uint16_t *)(buf+count*sizeof(uint16_t))=(uint16_t)spi_i2s_data_receive(spi_periph);    
        else
            *(uint8_t *)(buf+count*sizeof(uint8_t))=(uint8_t)spi_i2s_data_receive(spi_periph);        
    }
    /* return the byte read from the SPI bus */
    return OS_EOK;
}

/*!
  \brief      send a byte through the SPI interface and return the byte received from the SPI bus
  \param[in]  byte: byte to send
  \param[out] none
  \retval     the value of the received byte
  */

static os_uint32_t spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_uint32_t state;
    os_size_t         message_length, already_send_length;
    os_uint32_t       send_length;
    os_uint8_t       *recv_buf;
    const os_uint8_t *send_buf;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    struct gd32_spi *spi_drv = os_container_of(device->bus, struct gd32_spi, spi_bus);

    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }

    message_length = message->length;
    recv_buf       = message->recv_buf;
    send_buf       = message->send_buf;
    while (message_length)
    {
        /* the GD32 library use uint32 to save the data length */
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

            state = spi_send_buf(spi_drv, (uint8_t *)send_buf, send_length);

            if(state != OS_EOK)
                goto done;

            state = spi_receive_buf(spi_drv, (uint8_t *)recv_buf, send_length);
        }
        else if (message->send_buf)
        {
            state = spi_send_buf(spi_drv, (uint8_t *)send_buf, send_length);
        }
        else
        {
            memset((uint8_t *)recv_buf, 0xff, send_length);
            state = spi_receive_buf(spi_drv, (uint8_t *)recv_buf, send_length);
        }
done:
        if (state != OS_EOK)
        {
            LOG_EXT_I("spi transfer error : %d", state);
            message->length   = 0;
        }


        else
        {
            LOG_EXT_D("transfer done");
        }

    }

    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }

    return message->length;
}


static const struct os_spi_ops gd32_spi_ops = {
    .configure = spi_configure,
    .xfer      = spixfer,
};

static int gd32_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_err_t    result  = 0;
    os_base_t   level;

    struct gd32_spi *gd_spi = os_calloc(1, sizeof(struct gd32_spi));

    OS_ASSERT(gd_spi);

    gd_spi->spi_info = (struct gd_spi_info *)dev->info;

    struct os_spi_bus *spi_bus = &gd_spi->spi_bus;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&gd32_spi_list, &gd_spi->list);
    os_hw_interrupt_enable(level);

    result = os_spi_bus_register(spi_bus, dev->name, &gd32_spi_ops);
    OS_ASSERT(result == OS_EOK);

    LOG_EXT_D("%s bus init done", dev->name);

    return result;
}

OS_DRIVER_INFO gd32_spi_driver = {
    .name   = "SPI_HandleTypeDef",
    .probe  = gd32_spi_probe,
};

OS_DRIVER_DEFINE(gd32_spi_driver, "1");

/*for test*/
#if 1

#include <stdint.h>
#include <oneos_config.h>
#include <spi.h>
#include <drv_spi.h>
#include <drv_gpio.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

#define W25Q16XX_BUS_NAME "w25q16xx_spi"


#define BSP_SPI_CS GET_PIN(E, 11)

void spi_test(void)
{
    const char *spi_device_name = "spi5";

    struct os_spi_device *os_spi_device;

    os_hw_spi_device_attach(spi_device_name, W25Q16XX_BUS_NAME, BSP_SPI_CS);

    os_spi_device = (struct os_spi_device *)os_device_find(W25Q16XX_BUS_NAME);
    if (os_spi_device == OS_NULL)
    {
        LOG_EXT_E("spi device %s not found!\r\n", W25Q16XX_BUS_NAME);
        return ;
    }

    /* config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
        cfg.max_hz     = 6000000;                   /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */
        os_spi_configure(os_spi_device, &cfg);
    }

    /* read JEDEC ID */
    uint8_t cmd[4] = {0x9f,0x0,0x0,0x0};
    uint8_t id_recv[6] = {0xff, 0xff, 0xff, 0xff, 0xff,0xff};

    os_spi_send_then_recv(os_spi_device, cmd, 4, id_recv, 6);

    LOG_EXT_E("JEDEC Read-ID Data : %02X %02X %02X %02X %02X %02X\r\n", id_recv[0], id_recv[1],id_recv[2], id_recv[3], id_recv[4], id_recv[5]);
    return;

}
    
SH_CMD_EXPORT(spi_test, spi_test, "spi_test");

#endif
#if 0
#include "gd25qxx.h"
#include <stdint.h>
#include <oneos_config.h>
#include <spi.h>
#include <drv_spi.h>
#include <drv_gpio.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

#define BUFFER_SIZE              256
#define TX_BUFFER_SIZE           (countof(tx_buffer) - 1)
#define RX_BUFFER_SIZE           0xFF

#define SFLASH_ID                0xC84015
#define FLASH_WRITE_ADDRESS      0x000000
#define FLASH_READ_ADDRESS       FLASH_WRITE_ADDRESS

uint8_t tx_buffer[256];
uint8_t rx_buffer[256];
uint32_t flash_id = 0;
uint32_t DeviceID = 0;
uint16_t i = 0;

void spi_test(void)
{
/* configure SPI5 GPIO and parameter */
    spi_flash_init();

    /* get flash id */
    flash_id = spi_flash_read_id();
    os_kprintf("\n\rThe Flash_ID:0x%X\n\r\n\r",flash_id);
#if 1
    /* flash id is correct */
        os_kprintf("\n\rWrite to tx_buffer:\n\r\n\r");

        /* printf tx_buffer value */
        for(i = 0; i < BUFFER_SIZE; i++){
            tx_buffer[i] = i;
            os_kprintf("0x%02X ",tx_buffer[i]);

            if(15 == i%16)
                os_kprintf("\n\r");
        }

        os_kprintf("\n\r\n\rRead from rx_buffer:\n\r\n\r");

        /* erase the specified flash sector */
        spi_flash_sector_erase(FLASH_WRITE_ADDRESS);

        /* write tx_buffer data to the flash */ 
        qspi_flash_buffer_write(tx_buffer,FLASH_WRITE_ADDRESS,256);

        /* read a block of data from the flash to rx_buffer */
        qspi_flash_buffer_read(rx_buffer,FLASH_READ_ADDRESS,256);

        /* printf rx_buffer value */
        for(i = 0; i < BUFFER_SIZE; i ++){
            os_kprintf("0x%02X ", rx_buffer[i]);
            if(15 == i%16)
                os_kprintf("\n\r");
        }
#endif
}

SH_CMD_EXPORT(spi_test, spi_test, "spi_test");

#endif
