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

#define DBG_TAG "I2C"
#ifdef OS_I2C_BITOPS_DEBUG
#define DRV_EXT_LVL DBG_LOG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif

#include <drv_log.h>

#define STM32_I2C_SET_SDA(bus, val) os_pin_write(bus->config.sda, val)
#define STM32_I2C_SET_SCL(bus, val) os_pin_write(bus->config.scl, val)
#define STM32_I2C_GET_SDA(bus)      os_pin_read(bus->config.sda)
#define STM32_I2C_GET_SCL(bus)      os_pin_read(bus->config.scl)

#define STM32_I2C_SDA_L(bus) SET_SDA(bus, 0)
#define STM32_I2C_SDA_H(bus) SET_SDA(bus, 1)
#define STM32_I2C_SCL_L(bus) SET_SCL(bus, 0)

struct i2c_config
{
    os_uint8_t  scl;
    os_uint8_t  sda;
};

struct stm32_i2c
{
    struct os_i2c_bus_device i2c;
    I2C_HandleTypeDef *hi2c;
    struct i2c_config config;
    os_sem_t   i2c_sem;
    os_list_node_t list;
};

static os_err_t stm32_check_and_solve_bus_busy(struct stm32_i2c *dev);

static os_list_node_t stm32_i2c_list = OS_LIST_INIT(stm32_i2c_list);

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    struct stm32_i2c *i2c;

    os_list_for_each_entry(i2c, &stm32_i2c_list, struct stm32_i2c, list)
    {
        if (i2c->hi2c == hi2c)
        {
            os_sem_post(&i2c->i2c_sem);
            break;
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    struct stm32_i2c *i2c;

    os_list_for_each_entry(i2c, &stm32_i2c_list, struct stm32_i2c, list)
    {
        if (i2c->hi2c == hi2c)
        {
            os_sem_post(&i2c->i2c_sem);
            break;
        }
    }
}
static os_size_t stm32_i2c_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    os_err_t result = HAL_OK;
    os_uint32_t msgs_count = 0;
    os_uint32_t msgs_count_cur = 0;
    os_uint32_t msgs_buf_length = 0;
    os_uint16_t dev_addr = 0;
    os_uint16_t i2c_flag = 0;
    os_uint8_t *i2c_data_buf = OS_NULL;
    os_uint8_t *i2c_data_buf_cur = OS_NULL;
    
    struct stm32_i2c *st_i2c;
    
    st_i2c = os_container_of(bus, struct stm32_i2c, i2c);
    
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
                if (st_i2c->hi2c->hdmarx != OS_NULL)
                {
                    result = HAL_I2C_Master_Receive_DMA(st_i2c->hi2c, dev_addr, i2c_data_buf, msgs_buf_length);
                    os_sem_wait(&st_i2c->i2c_sem, OS_WAIT_FOREVER);
                    while (msgs_count_cur < msgs_count)
                    {
                        memcpy(msgs[msgs_count_cur].buf, i2c_data_buf_cur, msgs[msgs_count_cur].len);
                        i2c_data_buf_cur += msgs[msgs_count_cur].len; 
                        msgs_count_cur++;
                    }
                }
                else
                {
                    result = HAL_I2C_Master_Receive(st_i2c->hi2c, dev_addr, i2c_data_buf, msgs_buf_length, bus->timeout);
                    while (msgs_count_cur < msgs_count)
                    {
                        memcpy(msgs[msgs_count_cur].buf, i2c_data_buf_cur, msgs[msgs_count_cur].len);
                        i2c_data_buf_cur += msgs[msgs_count_cur].len; 
                        msgs_count_cur++;
                    }
                }

            }
            else
            {
                if (st_i2c->hi2c->hdmatx != OS_NULL)
                {
                    result = HAL_I2C_Master_Transmit_DMA(st_i2c->hi2c, dev_addr, i2c_data_buf, msgs_buf_length);
                    os_sem_wait(&st_i2c->i2c_sem, OS_WAIT_FOREVER);
                }
                else
                {
                    result = HAL_I2C_Master_Transmit(st_i2c->hi2c, dev_addr, i2c_data_buf, msgs_buf_length, bus->timeout);
                }
            }
            os_free(i2c_data_buf);
        }
        else
        {
            if (i2c_flag & OS_I2C_RD)
            {
                if (st_i2c->hi2c->hdmarx != OS_NULL)
                {
                    result = HAL_I2C_Master_Receive_DMA(st_i2c->hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
                    os_sem_wait(&st_i2c->i2c_sem, OS_WAIT_FOREVER);
                }
                else
                {
                    result = HAL_I2C_Master_Receive(st_i2c->hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len, bus->timeout);
                }
            }
            else
            {
                if (st_i2c->hi2c->hdmatx != OS_NULL)
                {
                    result = HAL_I2C_Master_Transmit_DMA(st_i2c->hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
                    os_sem_wait(&st_i2c->i2c_sem, OS_WAIT_FOREVER);
                }
                else
                {
                    result = HAL_I2C_Master_Transmit(st_i2c->hi2c, dev_addr, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len, bus->timeout);
                }
            }
            msgs_count_cur++;
        }
        
        if (result != HAL_OK)
        {
            break;
        }
    }

    if (result == HAL_OK)
    {
        return num;
    }
    else
    {
        result = stm32_check_and_solve_bus_busy(st_i2c);
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

static os_size_t stm32_i2c_slave_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
#if 0
    HAL_StatusTypeDef status = HAL_OK;
    
    struct stm32_i2c *st_i2c;

    st_i2c = os_container_of(bus, struct stm32_i2c, i2c);

    if (msgs->flags & OS_I2C_RD)
    {
        status = HAL_I2C_Slave_Receive_IT(st_i2c->hi2c, (uint8_t *)msgs.buf , msgs.len);
    }
    else
    {
        status = HAL_I2C_Slave_Transmit_IT(st_i2c->hi2c, (uint8_t *)msgs.buf , msgs.len);
    }  

    if (status == HAL_OK)
    {
        return num;
    }
    else
    {
        if (status == HAL_BUSY)
        {
            result = stm32_check_and_solve_bus_busy(st_i2c);
            if (result == OS_ERROR)
            {
                return 0;
            }
        }
    }
#endif
      return 0;
}

static os_err_t stm32_i2c_bus_control(struct os_i2c_bus_device *bus, void *args)
{   
    return 0;
}

static const struct os_i2c_bus_device_ops i2c_bus_ops =
{
    .i2c_transfer       = stm32_i2c_transfer,
    .i2c_slave_transfer = stm32_i2c_slave_transfer,
    .i2c_bus_control    = stm32_i2c_bus_control,
};

os_err_t i2c_bus_unlock(struct stm32_i2c *bus)
{
    os_uint8_t i = 0;

    while (PIN_LOW == os_pin_read(bus->config.sda) && (i < 16))
    {
        os_pin_write(bus->config.scl, PIN_HIGH);
        os_clocksource_ndelay(200000);
        os_pin_write(bus->config.scl, PIN_LOW);
        os_clocksource_ndelay(200000);
        i++;
    }
    LOG_I(DBG_TAG,"i2c bus unlock times %d!", i);
    if (PIN_LOW == os_pin_read(bus->config.sda))
    {
        LOG_E(DBG_TAG,"i2c bus unlock failed : sda still low!");
        return OS_ERROR;
    }
    os_pin_write(bus->config.scl, PIN_HIGH);
    os_clocksource_ndelay(200000);
    os_pin_write(bus->config.sda, PIN_LOW);
    os_clocksource_ndelay(200000);
    os_pin_write(bus->config.sda, PIN_HIGH);
    os_clocksource_ndelay(200000);
    
    return OS_EOK;
}


static os_err_t stm32_check_and_solve_bus_busy(struct stm32_i2c *dev)
{
    os_err_t status = HAL_OK;
    
    if (__HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BUSY) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_ARLO) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BERR))
    {
        LOG_I(DBG_TAG,"i2c bus error and reset!");
#ifdef I2C_CR1_PE
        CLEAR_BIT(dev->hi2c->Instance->CR1, I2C_CR1_PE);
        os_clocksource_ndelay(200000);
        SET_BIT(dev->hi2c->Instance->CR1, I2C_CR1_PE);
#endif 
        os_clocksource_ndelay(200000);
        if (__HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BUSY) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_ARLO) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BERR))
        {
            if (HAL_I2C_DeInit(dev->hi2c) != HAL_OK)
            {
                LOG_E(DBG_TAG,"i2c bus deinit failed!");
                return OS_ERROR;
            }
            
#ifdef I2C_CR1_SWRST
            SET_BIT(dev->hi2c->Instance->CR1, I2C_CR1_SWRST);
            os_clocksource_ndelay(200000);
            CLEAR_BIT(dev->hi2c->Instance->CR1, I2C_CR1_SWRST);
#endif

            os_pin_mode(dev->config.scl, PIN_MODE_OUTPUT_OD);
            os_pin_mode(dev->config.sda, PIN_MODE_OUTPUT_OD);
            
            os_pin_write(dev->config.scl, PIN_HIGH);
            os_pin_write(dev->config.sda, PIN_HIGH);

            status = i2c_bus_unlock(dev);
            
            os_pin_write(dev->config.scl, PIN_HIGH);
            os_pin_write(dev->config.sda, PIN_HIGH);

            if (status == OS_EOK)
            {
                if (HAL_I2C_Init(dev->hi2c) != HAL_OK)
                {
                    LOG_E(DBG_TAG,"i2c bus init failed!");
                    return OS_ERROR;
                }
#ifdef I2C_CR1_PE
                CLEAR_BIT(dev->hi2c->Instance->CR1, I2C_CR1_PE);
                os_clocksource_ndelay(200000);
                SET_BIT(dev->hi2c->Instance->CR1, I2C_CR1_PE);
#endif 
                if (__HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BUSY) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_ARLO) | __HAL_I2C_GET_FLAG(dev->hi2c, I2C_FLAG_BERR))
                {
                    LOG_E(DBG_TAG,"i2c bus still busy/arlo/berr!");
                    return OS_ERROR;
                }
            }
            else
            {
                return OS_ERROR;
            }
        }
    }
    return OS_EOK;
}

static int stm32_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;
    char sem_name[16] = "sem_";
    
    struct stm32_i2c_info *i2c_info = (struct stm32_i2c_info *)dev->info;
    struct stm32_i2c *st_i2c = os_calloc(1, sizeof(struct stm32_i2c));

    OS_ASSERT(st_i2c);
        
    strcat(sem_name, dev->name);
    os_sem_init(&st_i2c->i2c_sem, sem_name, 0, OS_SEM_MAX_VALUE);

    st_i2c->hi2c = i2c_info->instance;

    if (dev->info != OS_NULL)
    {
        st_i2c->config.scl = i2c_info->scl;
        st_i2c->config.sda = i2c_info->sda;
    }

    result = stm32_check_and_solve_bus_busy(st_i2c);
    if (result == OS_ERROR)
    {
        LOG_E(DBG_TAG,"unlock i2c bus error!");
        return result;
    }
    
    struct os_i2c_bus_device *dev_i2c = &st_i2c->i2c;

    dev_i2c->ops    = &i2c_bus_ops;

    level = os_irq_lock();
    os_list_add_tail(&stm32_i2c_list, &st_i2c->list);
    os_irq_unlock(level);
   
    result = os_i2c_bus_device_register(dev_i2c,
                                dev->name,
                                dev_i2c);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO stm32_i2c_driver = {
    .name   = "I2C_HandleTypeDef",
    .probe  = stm32_i2c_probe,
};

OS_DRIVER_DEFINE(stm32_i2c_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);

