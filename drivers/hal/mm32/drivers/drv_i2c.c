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
 * @brief       This file implements i2c driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>

#include <drv_cfg.h>
#include <os_task.h>
#include <os_memory.h>

#define LOG_TAG              "drv.i2c"
//#include <drv_log.h>

//#include "fsl_lpi2c.h"
#include "bsp.h"
#include "drv_rtc.h"
#include "mm32_hal.h"
#include "drv_i2c.h"

struct mm32_i2c
{
    struct os_i2c_bus_device i2c;
    I2C_TypeDef * hi2c;
    os_list_node_t list;

    struct os_i2c_msg *msg;
    os_uint16_t msg_pos;
    
    char *device_name;

    os_sem_t sem;
};

static os_list_node_t mm32_i2c_list = OS_LIST_INIT(mm32_i2c_list);

void mm32_i2c_read(I2C_TypeDef * i2c_periph, uint32_t dev_addr, uint8_t* p_buffer, uint16_t number_of_byte)
{
    I2C_GenerateSTART(i2c_periph, 1);
    #if 0
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
    #endif
}
//first write£¬send device_addr and data
void mm32_i2c_write_start(I2C_TypeDef * i2c_periph, uint32_t dev_addr, uint8_t* p_buffer)
{
    #if 0
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
    #endif
}

//not first write£¬only send data
void mm32_i2c_write_not_start(I2C_TypeDef * i2c_periph, uint8_t* p_buffer, uint16_t number_of_byte)
{
    #if 0
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
    #endif
}


static os_size_t mm32_i2c_mst_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    struct mm32_i2c *mm32_i2c;
    int i;

    OS_ASSERT(bus != OS_NULL);
    mm32_i2c = (struct mm32_i2c *) bus;
    
    for (i = 0; i < num; i++)
    {
        mm32_i2c->msg = &msgs[i];
        mm32_i2c->msg_pos = 0;

        if (msgs[i].flags & OS_I2C_RD)
        {
            mm32_i2c_read(mm32_i2c->hi2c, msgs[i].addr, msgs[i].buf, msgs[i].len);
        }
        else
        {
            if (msgs[i].flags & OS_I2C_NO_START)
            {
                mm32_i2c_write_not_start(mm32_i2c->hi2c, msgs[i].buf, msgs[i].len);
            }
            else
            {
                mm32_i2c_write_start(mm32_i2c->hi2c, msgs[i].addr, msgs[i].buf);
            }
        }
    }


    mm32_i2c->msg = OS_NULL;
    mm32_i2c->msg_pos = 0;

    return i;
}

static os_size_t mm32_i2c_slv_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    return 0;
}
static os_err_t mm32_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_ERROR;
}

static const struct os_i2c_bus_device_ops mm32_i2c_ops =
{
    .i2c_transfer       = mm32_i2c_mst_xfer,
    .i2c_slave_transfer = mm32_i2c_slv_xfer,
    .i2c_bus_control    = mm32_i2c_bus_control,
};

void mm32_i2c_gpio_config(struct mm32_i2c_info * i2c_info)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHBPeriphClockCmd(i2c_info->pin_clk, ENABLE);
    GPIO_PinAFConfig(i2c_info->pin_port, i2c_info->scl_pin_source, i2c_info->gpio_af_idx);
    GPIO_PinAFConfig(i2c_info->pin_port, i2c_info->sda_pin_source, i2c_info->gpio_af_idx);
    GPIO_InitStruct.GPIO_Pin  = i2c_info->scl_pin;
    //Set GPIO spped
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_20MHz;
    //Keep the bus free which means SCK & SDA is high
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(i2c_info->pin_port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = i2c_info->sda_pin;
    GPIO_Init(i2c_info->pin_port, &GPIO_InitStruct);
    
}

/*!
    \brief      configure the I2C interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/

#define I2C0_SLAVE_ADDRESS7     0xA0
void mm32_i2c_config(struct mm32_i2c_info * i2c_info)
{
    I2C_InitTypeDef I2C_InitStruct;

    //Enable I2C clock state
    i2c_info->rcc_init_func(i2c_info->i2c_clk, ENABLE);

    I2C_StructInit(&I2C_InitStruct);
    //Configure I2C as master mode
    I2C_InitStruct.Mode = I2C_CR_MASTER;
    I2C_InitStruct.OwnAddress = 0;
    I2C_InitStruct.Speed = I2C_CR_FAST;
    I2C_InitStruct.ClockSpeed = i2c_info->speed;

    //Initializes the I2Cx peripheral according to the specified
    I2C_Init(i2c_info->hi2c, &I2C_InitStruct);
    I2C_Cmd(i2c_info->hi2c, ENABLE);
}

static void I2C_SetDeviceAddr(struct mm32_i2c_info * i2c_info, u8 deviceaddr)
{
    //Disable I2C
    I2C_Cmd(i2c_info->hi2c, DISABLE);
    //Set the device address
    I2C_Send7bitAddress(i2c_info->hi2c, deviceaddr, I2C_Direction_Transmitter);
    //Enable I2C
    I2C_Cmd(i2c_info->hi2c, ENABLE);
}

static int mm32_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct mm32_i2c_info * i2c_info = (struct mm32_i2c_info *)dev->info;
    struct mm32_i2c *mm32_i2c = os_calloc(1, sizeof(struct mm32_i2c));

    OS_ASSERT(mm32_i2c);

    char name[16] = "i2c_";
    strcat(name, dev->name);
    os_sem_init(&mm32_i2c->sem, name, 0, 1);
    
    mm32_i2c->hi2c = i2c_info->hi2c;
    
    mm32_i2c_gpio_config(i2c_info);
    
    mm32_i2c_config(i2c_info);

    struct os_i2c_bus_device *dev_i2c = &mm32_i2c->i2c;

    dev_i2c->ops = &mm32_i2c_ops;

    level = os_irq_lock();
    os_list_add_tail(&mm32_i2c_list, &mm32_i2c->list);
    os_irq_unlock(level);
   
    result = os_i2c_bus_device_register(dev_i2c, dev->name, dev_i2c);
    
    OS_ASSERT(result == OS_EOK);
    
    return result;
}

OS_DRIVER_INFO mm32_i2c_driver = {
    .name   = "I2C_Type",
    .probe  = mm32_i2c_probe,
};

OS_DRIVER_DEFINE(mm32_i2c_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

