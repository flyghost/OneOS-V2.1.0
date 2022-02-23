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
 * @file        soft_i2c_bus.c
 *
 * @brief       this file implements i2c-bit-bus related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_types.h>
#include <device.h>
#include <arch_interrupt.h>
#include <os_assert.h>
#include <drv_common.h>
#include <os_memory.h>
#include <string.h>
#include <timer/clocksource.h>
#include <drv_i2c.h>

#define DBG_TAG "drv.i2c"
#include <drv_log.h>


static os_err_t fm33_check_and_solve_bus_busy(struct fm33_i2c *dev);

static os_list_node_t fm33_i2c_list = OS_LIST_INIT(fm33_i2c_list);

static os_uint8_t I2C_Send_Byte(os_uint8_t x_byte)
{
    FL_I2C_Master_WriteTXBuff(I2C, x_byte);

    while(!FL_I2C_Master_IsActiveFlag_TXComplete(I2C));

    FL_I2C_Master_ClearFlag_TXComplete(I2C);

    if(!FL_I2C_Master_IsActiveFlag_NACK(I2C))
    {
        return 0;
    }

    else
    {
        FL_I2C_Master_ClearFlag_NACK(I2C);
        return 1;
    }

}

static os_uint8_t I2C_Receive_Byte(os_uint8_t *x_byte)
{
    FL_I2C_Master_EnableRX(I2C);

    while(!FL_I2C_Master_IsActiveFlag_RXComplete(I2C));

    FL_I2C_Master_ClearFlag_RXComplete(I2C);
    *x_byte = FL_I2C_Master_ReadRXBuff(I2C);
    return 0;
}

os_err_t HAL_I2C_Master_Receive(I2C_Type *inst, os_uint16_t DevAddress, os_uint8_t *pData, os_uint16_t Size, os_uint32_t Timeout)
{
    os_uint8_t ret;
    os_uint16_t i;
    
    FL_I2C_Master_EnableI2CStart(I2C);
    while(!FL_I2C_Master_IsActiveFlag_Start(I2C));
    
    FL_I2C_Master_DisableRX(I2C);
    ret = I2C_Send_Byte((os_uint8_t)DevAddress);
    if(ret != 0)
    {
        return OS_ERROR;
    }
    
    FL_I2C_Master_EnableI2CRestart(I2C);
    ret = I2C_Send_Byte(DevAddress | 1);
    if(ret != 0)
    {
        return OS_ERROR;
    }

    for(i = 0; i < Size; i++)
    {
        if(i < Size - 1)
        {
            FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_ACK);
        }
        else
        {
            FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_NACK);
        }
        
        ret = I2C_Receive_Byte(pData + i);
        if(ret != 0)
        {
            return OS_ERROR;
        }        
    }

    FL_I2C_Master_EnableI2CStop(I2C);
    
    return OS_EOK;
}

os_err_t HAL_I2C_Master_Transmit(I2C_Type *inst, os_uint16_t DevAddress, os_uint8_t *pData, os_uint16_t Size, uint32_t Timeout)
{
    os_uint8_t ret;
    os_uint16_t i;
    
    FL_I2C_Master_EnableI2CStart(I2C);
    while(!FL_I2C_Master_IsActiveFlag_Start(I2C));
    
    FL_I2C_Master_DisableRX(I2C);
    ret = I2C_Send_Byte((os_uint8_t)DevAddress);
    if(ret != 0)
    {
        return OS_ERROR;
    }

    for(i = 0; i < Size; i++)
    {
        ret = I2C_Send_Byte(*(pData + i));
        if(ret != 0)
        {
            return OS_ERROR;
        }        
    }

    FL_I2C_Master_EnableI2CStop(I2C);

    return OS_EOK;    
}

static os_err_t fm33_i2c_init(struct fm33_i2c *dev)
{
    FL_I2C_MasterMode_InitTypeDef   IICInitStructer;
    FL_GPIO_InitTypeDef    GPIO_InitStruct = {0};

    GPIO_InitStruct.pin = dev->info->scl.gpio;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    FL_GPIO_Init(dev->info->scl.port,&GPIO_InitStruct);


    GPIO_InitStruct.pin = dev->info->sda.gpio;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    FL_GPIO_Init(dev->info->sda.port,&GPIO_InitStruct);


    IICInitStructer.clockSource = dev->info->cfg.clockSource;
    IICInitStructer.baudRate = dev->info->cfg.baudRate;
    FL_I2C_MasterMode_Init(dev->info->inst, &IICInitStructer);

    return OS_EOK;
}
#ifdef OS_USING_LPMGR
static os_err_t fm33_i2c_deinit(struct fm33_i2c *dev)
{
    FL_I2C_DeInit(dev->info->inst);
    return OS_EOK;
}
#endif
static os_size_t fm33_i2c_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    os_err_t result = OS_EOK;
    os_uint32_t msgs_count = 0;
    os_uint32_t msgs_count_cur = 0;
    os_uint32_t msgs_buf_length = 0;
    os_uint16_t dev_addr = 0;
    os_uint16_t i2c_flag = 0;
    os_uint8_t *i2c_data_buf = OS_NULL;
    os_uint8_t *i2c_data_buf_cur = OS_NULL;
    I2C_Type   *hi2c;
    
    struct fm33_i2c *fm_i2c;
    
    fm_i2c = os_container_of(bus, struct fm33_i2c, i2c);
    hi2c = fm_i2c->info->inst;
    
    while (msgs_count_cur < num)
    {
        msgs_buf_length = msgs[msgs_count_cur].len;
        i2c_flag = msgs[msgs_count_cur].flags;
        msgs_count = msgs_count_cur + 1;
        dev_addr = msgs[msgs_count_cur].addr << 1;
        while (msgs_count < num)
        {
            if ((msgs[msgs_count].flags & OS_I2C_NO_START) && (msgs[msgs_count].addr == msgs[msgs_count_cur].addr) && ((i2c_flag & 0x01) == (msgs[msgs_count].flags & 0x01) ))
            {
                msgs_buf_length += msgs[msgs_count].len;
                msgs_count++;
            }
            else
            {
                break;
            }
        }
        
        if (msgs_count - msgs_count_cur > 1)
        {
            i2c_data_buf = (os_uint8_t *)os_calloc(1, msgs_buf_length);
            if (i2c_data_buf == OS_NULL)
            {
                LOG_E(DBG_TAG,"i2c calloc failed! too many message with same condition to merge!");
            }
            
            i2c_data_buf_cur = i2c_data_buf;

            if ((i2c_flag & 0x01) == OS_I2C_WR)
            {
                while (msgs_count_cur < msgs_count)
                {
                    memcpy(i2c_data_buf_cur, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
                    i2c_data_buf_cur += msgs[msgs_count_cur].len; 
                    msgs_count_cur++;
                }
            }
            
            if (i2c_flag & OS_I2C_RD)
            {
                result = HAL_I2C_Master_Receive(hi2c, dev_addr, i2c_data_buf, msgs_buf_length, bus->timeout);
                while (msgs_count_cur < msgs_count)
                {
                    memcpy(msgs[msgs_count_cur].buf, i2c_data_buf_cur, msgs[msgs_count_cur].len);
                    i2c_data_buf_cur += msgs[msgs_count_cur].len; 
                    msgs_count_cur++;
                }

            }
            else
            {
                result = HAL_I2C_Master_Transmit(hi2c, dev_addr, i2c_data_buf, msgs_buf_length, bus->timeout);
            }
            os_free(i2c_data_buf);
        }
        else
        {
            if (i2c_flag & OS_I2C_RD)
            {
                result = HAL_I2C_Master_Receive(hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len, bus->timeout);
            }
            else
            {
                result = HAL_I2C_Master_Transmit(hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len, bus->timeout);
            }
            msgs_count_cur++;
        }
        
        if (result != OS_EOK)
        {
            break;
        }
    }

    if (result == OS_EOK)
    {
        return num;
    }
    else
    {
        result = fm33_check_and_solve_bus_busy(fm_i2c);
        if (result == OS_ERROR)
        {
            LOG_E(DBG_TAG,"unlock i2c bus error!");
        }
        else
        {
            LOG_I(DBG_TAG,"unlock i2c bus success!");
        }
        return 0;
    }
}

static os_size_t fm33_i2c_slave_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
      return 0;
}

static os_err_t fm33_i2c_bus_control(struct os_i2c_bus_device *bus, void *args)
{   
    return 0;
}

static const struct os_i2c_bus_device_ops i2c_bus_ops =
{
    .i2c_transfer       = fm33_i2c_transfer,
    .i2c_slave_transfer = fm33_i2c_slave_transfer,
    .i2c_bus_control    = fm33_i2c_bus_control,
};

os_err_t i2c_bus_unlock(struct fm33_i2c *bus)
{
    os_uint8_t i = 0;

    while (PIN_LOW == os_pin_read(bus->info->sda.pin) && (i < 16))
    {
        os_pin_write(bus->info->scl.pin, PIN_HIGH);
        os_clocksource_ndelay(200000);
        os_pin_write(bus->info->scl.pin, PIN_LOW);
        os_clocksource_ndelay(200000);
        i++;
    }
    LOG_I(DBG_TAG,"i2c bus unlock times %d!", i);
    if (PIN_LOW == os_pin_read(bus->info->sda.pin))
    {
        LOG_E(DBG_TAG,"i2c bus unlock failed : sda still low!");
        return OS_ERROR;
    }
    os_pin_write(bus->info->scl.pin, PIN_HIGH);
    os_clocksource_ndelay(200000);
    os_pin_write(bus->info->sda.pin, PIN_LOW);
    os_clocksource_ndelay(200000);
    os_pin_write(bus->info->sda.pin, PIN_HIGH);
    os_clocksource_ndelay(200000);
    
    return OS_EOK;
}


static os_err_t fm33_check_and_solve_bus_busy(struct fm33_i2c *dev)
{
    return OS_EOK;
}

static int fm33_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct fm33_i2c_info *i2c_info = (struct fm33_i2c_info *)dev->info;
    struct fm33_i2c *fm_i2c = os_calloc(1, sizeof(struct fm33_i2c));

    OS_ASSERT(fm_i2c);

    fm_i2c->info = i2c_info;

    fm33_i2c_init(fm_i2c);
    result = fm33_check_and_solve_bus_busy(fm_i2c);
    if (result == OS_ERROR)
    {
        LOG_E(DBG_TAG,"unlock i2c bus error!");
        return result;
    }
    
    struct os_i2c_bus_device *dev_i2c = &fm_i2c->i2c;

    dev_i2c->ops    = &i2c_bus_ops;

    level = os_irq_lock();
    os_list_add_tail(&fm33_i2c_list, &fm_i2c->list);
    os_irq_unlock(level);
   
    result = os_i2c_bus_device_register(dev_i2c,
                                dev->name,
                                dev_i2c);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO fm33_i2c_driver = {
    .name   = "I2C_Type",
    .probe  = fm33_i2c_probe,
};

OS_DRIVER_DEFINE(fm33_i2c_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);

