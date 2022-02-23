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
 * @brief       This file implements i2c driver for gd.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <drv_cfg.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include "drv_i2c.h"

#define LOG_TAG              "drv.i2c"

struct gd_i2c
{
    struct os_i2c_bus_device i2c;
    struct gd_i2c_info *i2c_info;
    os_list_node_t list;

    struct os_i2c_msg *msg;
    os_uint16_t msg_pos;

    char *device_name;

    os_sem_t sem;
};

static os_list_node_t gd_i2c_list = OS_LIST_INIT(gd_i2c_list);

void gd32_i2c_read(uint32_t i2c_periph, uint32_t dev_addr, uint8_t* p_buffer, uint16_t number_of_byte)
{
    //send start flag
    //send device address first
    /* wait until BTC bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, (dev_addr<<1), I2C_RECEIVER);

    if(number_of_byte < 3){
        /* disable acknowledge */
        i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
    }

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

    if(1 == number_of_byte){
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(i2c_periph);
    }

    /* while there is data to be read */
    while(number_of_byte){
        if(3 == number_of_byte){
            /* wait until BTC bit is set */
            while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));

            /* disable acknowledge */
            i2c_ack_config(i2c_periph,I2C_ACK_DISABLE);
        }
        if(2 == number_of_byte){
            /* wait until BTC bit is set */
            while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));

            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(i2c_periph);
        }

        /* wait until the RBNE bit is set and clear it */
        if(i2c_flag_get(i2c_periph, I2C_FLAG_RBNE)){
            /* read a byte from the EEPROM */
            *p_buffer = i2c_data_receive(i2c_periph);

            /* point to the next location where the byte read will be saved */
            p_buffer++; 

            /* decrement the read bytes counter */
            number_of_byte--;
        } 
    }

    /* wait until the stop condition is finished */
    while(I2C_CTL0(i2c_periph)&0x0200);

    /* enable acknowledge */
    i2c_ack_config(i2c_periph,I2C_ACK_ENABLE);

    i2c_ackpos_config(i2c_periph,I2C_ACKPOS_CURRENT);

}
//first write£¬send device_addr and data
void gd32_i2c_write_start(uint32_t i2c_periph, uint32_t dev_addr, uint8_t* p_buffer)
{
    /* wait until I2C bus is idle */
    while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_periph);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(i2c_periph, (dev_addr<<1), I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(i2c_periph,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while( SET != i2c_flag_get(i2c_periph, I2C_FLAG_TBE));

    /* send the EEPROM's internal address to write to : only one byte address */
    i2c_data_transmit(i2c_periph, *p_buffer);

    /* wait until BTC bit is set */
    while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));
}

//not first write£¬only send data
void gd32_i2c_write_not_start(uint32_t i2c_periph, uint8_t* p_buffer, uint16_t number_of_byte)
{
    /* wait until the transmit data buffer is empty */
    while( SET != i2c_flag_get(i2c_periph, I2C_FLAG_TBE));

    /* while there is data to be written */
    while(number_of_byte--)
    {  
        i2c_data_transmit(i2c_periph, *p_buffer);

        /* point to the next byte to be written */
        p_buffer++; 

        /* wait until BTC bit is set */
        while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC));
    }

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(i2c_periph);

    /* wait until the stop condition is finished */
    while(I2C_CTL0(i2c_periph)&0x0200);
}


static os_size_t gd_i2c_mst_xfer(struct os_i2c_bus_device *bus,
        struct os_i2c_msg msgs[],
        os_uint32_t num)
{
    struct gd_i2c *gd_i2c;
    int i;

    OS_ASSERT(bus != OS_NULL);
    gd_i2c = (struct gd_i2c *) bus;

    for (i = 0; i < num; i++)
    {
        gd_i2c->msg = &msgs[i];
        gd_i2c->msg_pos = 0;

        if (msgs[i].flags & OS_I2C_RD)
        {
            gd32_i2c_read(gd_i2c->i2c_info->hi2c, msgs[i].addr, msgs[i].buf, msgs[i].len);
        }
        else
        {
            if (msgs[i].flags & OS_I2C_NO_START)
            {
                gd32_i2c_write_not_start(gd_i2c->i2c_info->hi2c, msgs[i].buf, msgs[i].len);
            }
            else
            {
                gd32_i2c_write_start(gd_i2c->i2c_info->hi2c, msgs[i].addr, msgs[i].buf);
            }
        }
    }


    gd_i2c->msg = OS_NULL;
    gd_i2c->msg_pos = 0;

    return i;
}

static os_size_t gd_i2c_slv_xfer(struct os_i2c_bus_device *bus,
        struct os_i2c_msg msgs[],
        os_uint32_t num)
{
    return 0;
}
static os_err_t gd_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_ERROR;
}

static const struct os_i2c_bus_device_ops gd_i2c_ops =
{
    .i2c_transfer       = gd_i2c_mst_xfer,
    .i2c_slave_transfer = gd_i2c_slv_xfer,
    .i2c_bus_control    = gd_i2c_bus_control,
};

void gd32_i2c_gpio_config(struct gd_i2c_info * i2c_info)
{
    uint32_t i2c_periph = i2c_info->hi2c;
    /* enable GPIOB clock */
    rcu_periph_clock_enable(i2c_info->periph);

    /* enable I2C clock */
    if (I2C0 == i2c_periph)
    {
        rcu_periph_clock_enable(RCU_I2C0);
    }
    else
    {
        rcu_periph_clock_enable(RCU_I2C1);
    }

    gpio_init(i2c_info->port, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, i2c_info->pin);
}

/*!
  \brief      configure the I2C interfaces
  \param[in]  none
  \param[out] none
  \retval     none
  */

#define I2C0_SLAVE_ADDRESS7     0xA0
void gd32_i2c_config(struct gd_i2c_info * i2c_info)
{
    /* enable I2C clock */
    uint32_t i2c_periph = i2c_info->hi2c;
    if (I2C0 == i2c_periph)
    {
        rcu_periph_clock_enable(RCU_I2C0);
    }
    else
    {
        rcu_periph_clock_enable(RCU_I2C1);
    }

    /* configure I2C clock */
    i2c_clock_config(i2c_periph, i2c_info->clk_speed, I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(i2c_periph,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,I2C0_SLAVE_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(i2c_periph);
    /* enable acknowledge */
    i2c_ack_config(i2c_periph,I2C_ACK_ENABLE);
}


static int gd_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct gd_i2c_info *i2c_info = (struct gd_i2c_info *)dev->info;
    struct gd_i2c *gd_i2c = os_calloc(1, sizeof(struct gd_i2c));

    OS_ASSERT(gd_i2c);

    char name[16] = "i2c_";
    strcat(name, dev->name);
    os_sem_init(&gd_i2c->sem, name, 0, OS_SEM_MAX_VALUE);

    gd_i2c->i2c_info = i2c_info;

    gd32_i2c_gpio_config(i2c_info);

    gd32_i2c_config(i2c_info);

    struct os_i2c_bus_device *dev_i2c = &gd_i2c->i2c;

    dev_i2c->ops = &gd_i2c_ops;

    level = os_irq_lock();
    os_list_add_tail(&gd_i2c_list, &gd_i2c->list);
    os_irq_unlock(level);

    result = os_i2c_bus_device_register(dev_i2c,
            dev->name,
            dev_i2c);

    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO gd_i2c_driver = {
    .name   = "I2C_Type",
    .probe  = gd_i2c_probe,
};

OS_DRIVER_DEFINE(gd_i2c_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

