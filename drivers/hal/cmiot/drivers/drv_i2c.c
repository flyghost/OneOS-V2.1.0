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
 * @brief       This file implements i2c driver for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <os_memory.h>
#include <string.h>
#include "board.h"
#include "drv_i2c.h"

#define DBG_TAG "drv.i2c"
#include <dlog.h>

#define I2C_MASTER_ADDR   0x00
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2C_FLAG_TIMOUT))
static __IO uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

static struct cm32_i2c i2cs[] = {
#ifdef  BSP_USING_I2C1
    {
        .i2c_base   = I2C1,
        .slave_addr = 0,
        .scl_port   = I2C1_SCL_PORT,
        .scl_pin    = I2C1_SCL_PIN,
        .af_i2c_scl = AF_I2C1_SCL,
        .sda_port   = I2C1_SDA_PORT,
        .sda_pin    = I2C1_SDA_PIN,
        .af_i2c_sda = AF_I2C1_SDA,
        .name = "i2c1",
    },
#endif

#ifdef  BSP_USING_I2C2
    {
        .i2c_base   = I2C2,
        .slave_addr = 0,
        .scl_port   = I2C2_SCL_PORT,
        .scl_pin    = I2C2_SCL_PIN,
        .af_i2c_scl = AF_I2C2_SCL,
        .sda_port   = I2C2_SDA_PORT,
        .sda_pin    = I2C2_SDA_PIN,
        .af_i2c_sda = AF_I2C2_SDA,
        .name = "i2c2",
    },
#endif
};

void cm_hal_i2c_init(struct cm32_i2c *i2c_info)
{
    I2C_InitType i2cx_master;
    GPIO_InitType i2cx_scl, i2cx_sda;

    if (i2c_info->i2c_base == I2C1)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, ENABLE);
    }
    if (i2c_info->i2c_base == I2C2)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C2, ENABLE);
    }

    if ((i2c_info->scl_port == GPIOA) || (i2c_info->sda_port == GPIOA))
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    }
    if ((i2c_info->scl_port == GPIOB) || (i2c_info->sda_port == GPIOB))
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    }
    if ((i2c_info->scl_port == GPIOC) || (i2c_info->sda_port == GPIOC))
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
    }
    if ((i2c_info->scl_port == GPIOD) || (i2c_info->sda_port == GPIOD))
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOD, ENABLE);
    }

    i2cx_scl.Pin        = i2c_info->scl_pin;
    i2cx_scl.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    i2cx_scl.GPIO_Mode  = GPIO_Mode_AF_OD;
    i2cx_scl.GPIO_Alternate = i2c_info->af_i2c_scl;
    i2cx_scl.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitPeripheral(i2c_info->scl_port, &i2cx_scl);

    i2cx_sda.Pin        = i2c_info->sda_pin;
    i2cx_sda.GPIO_Slew_Rate = GPIO_Slew_Rate_High;
    i2cx_sda.GPIO_Mode  = GPIO_Mode_AF_OD;
    i2cx_sda.GPIO_Alternate = i2c_info->af_i2c_sda;
    i2cx_sda.GPIO_Pull = GPIO_Pull_Up;
    GPIO_InitPeripheral(i2c_info->sda_port, &i2cx_sda);

    I2C_DeInit(i2c_info->i2c_base);
    i2cx_master.BusMode     = I2C_BUSMODE_I2C;
    i2cx_master.FmDutyCycle = I2C_FMDUTYCYCLE_2;
    i2cx_master.OwnAddr1    = I2C_MASTER_ADDR;
    i2cx_master.AckEnable   = I2C_ACKEN;
    i2cx_master.AddrMode    = I2C_ADDR_MODE_7BIT;
    i2cx_master.ClkSpeed    = 100000; /* 100K */

    I2C_Init(i2c_info->i2c_base, &i2cx_master);
    I2C_Enable(i2c_info->i2c_base, ENABLE);
}

int cm_hal_i2c_send(I2C_Module* I2CX, uint8_t u8Addr, uint8_t* data, int len)
{
    uint8_t* sendBufferPtr = data;
    I2CTimeout             = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2CX, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return 4;
    };

    I2C_ConfigAck(I2CX, ENABLE);

    I2C_GenerateStart(I2CX, ENABLE);
    I2CTimeout = I2C_FLAG_TIMOUT;
    while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_MODE_FLAG)) /* EV5 */
    {
        if ((I2CTimeout--) == 0)
            return 5;
    };

    I2C_SendAddr7bit(I2CX, u8Addr, I2C_DIRECTION_SEND);
    I2CTimeout = I2C_FLAG_TIMOUT;
    while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_TXMODE_FLAG)) /* EV6 */
    {
        if ((I2CTimeout--) == 0)
            return 6;
    };

    /* send data */
    while (len-- > 0)
    {
        I2C_SendData(I2CX, *sendBufferPtr++);
        I2CTimeout = I2C_FLAG_TIMOUT;
        while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_DATA_SENDING)) /* EV8 */
        {
            if ((I2CTimeout--) == 0)
                return 7;
        };
    };

    I2CTimeout = I2C_FLAG_TIMOUT;
    while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_DATA_SENDED)) /* EV8-2 */
    {
        if ((I2CTimeout--) == 0)
            return 8;
    };
    I2C_GenerateStop(I2CX, ENABLE);
    return 0;
}

int cm_hal_i2c_recv(I2C_Module* I2CX, uint8_t u8Addr, uint8_t* data, int len)
{
    uint8_t* recvBufferPtr = data;
    I2CTimeout             = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlag(I2CX, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return 9;
    };

    I2C_ConfigAck(I2CX, ENABLE);

    /* send start */
    I2C_GenerateStart(I2CX, ENABLE);
    I2CTimeout = I2C_FLAG_TIMOUT;
    while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_MODE_FLAG)) /* EV5 */
    {
        if ((I2CTimeout--) == 0)
            return 10;
    };

    /* send addr */
    I2C_SendAddr7bit(I2CX, u8Addr, I2C_DIRECTION_RECV);
    I2CTimeout = I2C_FLAG_TIMOUT;
    while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_RXMODE_FLAG)) /* EV6 */
    {
        if ((I2CTimeout--) == 0)
            return 6;
    };

    /* recv data */
    while (len-- > 0)
    {
        if (len == 0)
        {
            I2C_ConfigAck(I2CX, DISABLE);
            I2C_GenerateStop(I2CX, ENABLE);
        }
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2CX, I2C_EVT_MASTER_DATA_RECVD_FLAG)) /* EV7 */
        {
            if ((I2CTimeout--) == 0)
                return 14;
        };
        *recvBufferPtr++ = I2C_RecvData(I2CX);
    };
    return 0;
}

os_size_t cm_i2c_master_xfer(struct os_i2c_bus_device *bus, struct os_i2c_msg *msgs, os_uint32_t num)
{
    os_err_t result = OS_EOK;
    os_uint32_t msgs_count = 0;
    os_uint32_t msgs_count_cur = 0;
    os_uint32_t msgs_buf_length = 0;
    os_uint16_t dev_addr = 0;
    os_uint16_t i2c_flag = 0;
    os_uint8_t *i2c_data_buf = OS_NULL;
    os_uint8_t *i2c_data_buf_cur = OS_NULL;

    struct cm32_i2c *i2c;

    i2c = os_container_of(bus, struct cm32_i2c, parent);

    while (msgs_count_cur < num)
    {
        msgs_buf_length = msgs[msgs_count_cur].len;
        i2c_flag = msgs[msgs_count_cur].flags;
        msgs_count = msgs_count_cur + 1;
        dev_addr = msgs[msgs_count_cur].addr;
        while (msgs_count < num)
        {
            if ((msgs[msgs_count].flags & OS_I2C_NO_START)
                    && (msgs[msgs_count].addr == msgs[msgs_count_cur].addr)
                    && ((i2c_flag & 0x01) == (msgs[msgs_count].flags & 0x01)))
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
                LOG_E(DBG_TAG, "i2c calloc failed! too many message with same condition to merge!");
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
                result = cm_hal_i2c_recv(i2c->i2c_base, dev_addr << 1, i2c_data_buf, msgs_buf_length);
                while (msgs_count_cur < msgs_count)
                {
                    memcpy(msgs[msgs_count_cur].buf, i2c_data_buf_cur, msgs[msgs_count_cur].len);
                    i2c_data_buf_cur += msgs[msgs_count_cur].len;
                    msgs_count_cur++;
                }

            }
            else
            {
                result = cm_hal_i2c_send(i2c->i2c_base, dev_addr << 1, i2c_data_buf, msgs_buf_length);
            }
            os_free(i2c_data_buf);
        }
        else
        {
            if (i2c_flag & OS_I2C_RD)
            {
                result = cm_hal_i2c_recv(i2c->i2c_base, dev_addr << 1, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
            }
            else
            {
                result = cm_hal_i2c_send(i2c->i2c_base, dev_addr << 1, msgs[msgs_count_cur].buf, msgs[msgs_count_cur].len);
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
        return 0;
    }
}

os_err_t cm_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_EOK;
}

static const struct os_i2c_bus_device_ops cm32_i2c_ops =
{
    .i2c_transfer       = cm_i2c_master_xfer,
    .i2c_slave_transfer = OS_NULL,
    .i2c_bus_control    = cm_i2c_bus_control
};

int os_hw_i2c_init(void)
{
    os_uint32_t idx = 0;

    for (idx = 0; idx < (sizeof(i2cs) / sizeof(i2cs[0])); idx++)
    {
        cm_hal_i2c_init(&i2cs[idx]);

        i2cs[idx].parent.ops = &cm32_i2c_ops;
        os_i2c_bus_device_register(&(i2cs[idx].parent), i2cs[idx].name, &(i2cs[idx].parent));
    }

    return 0;
}

OS_DEVICE_INIT(os_hw_i2c_init, OS_INIT_SUBLEVEL_HIGH);
