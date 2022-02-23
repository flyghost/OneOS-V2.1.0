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
 * @file        drv_i2c.c
 *
 * @brief       This file implements i2c driver for nrf5.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>

#include <drv_cfg.h>
#include <os_task.h>
#include <os_memory.h>

#define LOG_TAG              "drv.i2c"
#include <drv_log.h>

#include "drv_i2c.h"

#ifdef NRF52832_XXAA
    #define NRF_TWI_READ_MAX_LEN (0xff)
#elif NRF52840_XXAA
    #define NRF_TWI_READ_MAX_LEN (0xffff)
#else
    #define NRF_TWI_READ_MAX_LEN (0xf)
#endif


struct nrf5_i2c
{
    struct os_i2c_bus_device i2c;
    struct nrf5_i2c_info *i2c_info;
    os_list_node_t list;

    struct os_i2c_msg *msg;
    os_uint16_t msg_pos;
    
    char *device_name;

    os_sem_t sem;
};

static os_list_node_t nrf5_i2c_list = OS_LIST_INIT(nrf5_i2c_list);

uint32_t nrf5_i2c_tx(nrfx_twim_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint16_t               length,
                          bool                  no_stop)
{
    uint32_t result = 0;
    result = nrfx_twim_tx(p_instance,
                            address, p_data, length, no_stop);

    return result;
}

uint32_t nrf5_i2c_rx(nrfx_twim_t const * p_instance,
                        uint8_t               address,
                        uint8_t *             p_data,
                        uint16_t               length)
{
    uint32_t result = 0;
    result = nrfx_twim_rx(p_instance,
                          address, p_data, length);

    return result;
}

static os_err_t nrf5_i2c_read(nrfx_twim_t const * p_instance, unsigned char dev_addr, unsigned char *reg_addr, unsigned char *data, uint16_t len)
{
    ret_code_t err_code;
    os_uint32_t rx_len;    
    os_uint32_t buff_head = 0;
    os_int32_t last_len = len;
    
    err_code = nrf5_i2c_tx(p_instance,dev_addr,reg_addr,1,true);
    OS_ASSERT(err_code == 0);

    do
    {
        rx_len = last_len > NRF_TWI_READ_MAX_LEN ? NRF_TWI_READ_MAX_LEN : last_len;
        
        err_code = nrf5_i2c_rx(p_instance, dev_addr, data + buff_head, rx_len);
        OS_ASSERT(err_code == 0);
        
        last_len -= NRF_TWI_READ_MAX_LEN;
        buff_head += NRF_TWI_READ_MAX_LEN;
    }while(last_len > 0);
    
    return OS_EOK;
}

void nrf5_i2c_write(nrfx_twim_t const * p_instance, unsigned char dev_addr, unsigned char *reg_addr, unsigned char *data, uint16_t len)
{
    unsigned char *pMem;
    unsigned short i;
    ret_code_t err_code;
    
    pMem = malloc(len +1);
    
    pMem[0] = reg_addr[0];
    
    for(i = 0; i < len; i++)
    {
        pMem[1 + i] = data[i];
    }

    err_code = nrf5_i2c_tx(p_instance,dev_addr,pMem,len+1,false);
        
    if(err_code != 0)
        os_kprintf("err_code %d\r\n", err_code);
        
    OS_ASSERT(err_code == 0);
    free(pMem);
}

static os_size_t nrf5_i2c_master_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    os_uint32_t ret = 0;
    struct nrf5_i2c *nrf5_i2c;

    OS_ASSERT(bus != OS_NULL);
    nrf5_i2c = (struct nrf5_i2c *) bus;

    if(msgs[1].flags & OS_I2C_RD)
    {
        ret = nrf5_i2c_read(&nrf5_i2c->i2c_info->hi2c
            , msgs[0].addr, msgs[0].buf, (unsigned char *)msgs[1].buf, msgs[1].len);
        if (ret !=  OS_EOK)
            return 0;
    }
    else
    {
        nrf5_i2c_write(&nrf5_i2c->i2c_info->hi2c,msgs[0].addr, msgs[0].buf, (unsigned char *)msgs[1].buf, msgs[1].len);
    }
    
    return num;
}

static os_size_t nrf5_i2c_slave_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    return 0;
}
static os_err_t nrf5_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_ERROR;
}

static const struct os_i2c_bus_device_ops nrf5_i2c_ops =
{
    .i2c_transfer       = nrf5_i2c_master_xfer,
    .i2c_slave_transfer = nrf5_i2c_slave_xfer,
    .i2c_bus_control    = nrf5_i2c_bus_control,
};

void nrf5_i2c_config(struct nrf5_i2c_info * i2c_info)
{
    nrfx_twim_t const m_twi = i2c_info->hi2c;
    
    nrfx_twim_config_t config = NRFX_TWIM_DEFAULT_CONFIG;
    config.frequency = (nrf_twim_frequency_t)TWI_FREQUENCY_FREQUENCY_K100;
    config.scl = i2c_info->scl_pin;
    config.sda = i2c_info->sda_pin;

    nrfx_twi_twim_bus_recover(config.scl,config.sda);
    
    nrfx_twim_init(&m_twi, &config, NULL, NULL);
    nrfx_twim_enable(&m_twi);
}

static int nrf5_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct nrf5_i2c_info *i2c_info = (struct nrf5_i2c_info *)dev->info;
    struct nrf5_i2c *nrf5_i2c = os_calloc(1, sizeof(struct nrf5_i2c));

    OS_ASSERT(nrf5_i2c);
    
    char name[16] = "i2c_";
    strcat(name, dev->name);
    
    nrf5_i2c->i2c_info = i2c_info;
    
    nrf5_i2c_config(i2c_info);

    struct os_i2c_bus_device *dev_i2c = &nrf5_i2c->i2c;

    dev_i2c->ops = &nrf5_i2c_ops;

    level = os_irq_lock();
    os_list_add_tail(&nrf5_i2c_list, &nrf5_i2c->list);
    os_irq_unlock(level);
   
    result = os_i2c_bus_device_register(dev_i2c,
                                        dev->name,
                                        dev_i2c);
    
    OS_ASSERT(result == OS_EOK);
    
    return result;
}

OS_DRIVER_INFO nrf5_i2c_driver = {
    .name   = "I2C_Type",
    .probe  = nrf5_i2c_probe,
};

OS_DRIVER_DEFINE(nrf5_i2c_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);
