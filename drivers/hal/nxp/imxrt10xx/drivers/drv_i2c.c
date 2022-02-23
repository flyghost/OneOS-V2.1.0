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
 * @brief       This file implements i2c driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_task.h>
#include <os_memory.h>

#define LOG_TAG              "drv.i2c"
#include <drv_log.h>

#include "fsl_lpi2c.h"
#include "drv_i2c.h"

struct imxrt_i2c
{
    struct os_i2c_bus_device i2c;
    struct nxp_lpi2c_info *i2c_info;
    os_list_node_t list;

    struct os_i2c_msg *msg;
    os_uint16_t msg_pos;
    
    char *device_name;

    os_sem_t sem;
};

static os_list_node_t imxrt_i2c_list = OS_LIST_INIT(imxrt_i2c_list);

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

static void i2c_irq_callback(struct imxrt_i2c *imxrt_i2c)
{
    uint32_t flags = 0U;
    status_t reVal = kStatus_Fail;                                           
                                                                                                                                                  
    /* Get interrupt status flags */
    flags = LPI2C_MasterGetStatusFlags(imxrt_i2c->i2c_info->hi2c);

    if ((flags & kLPI2C_MasterTxReadyFlag) && !(imxrt_i2c->msg->flags & OS_I2C_RD))
    {
        /* If tx Index < LPI2C_DATA_LENGTH, master send->slave receive transfer is ongoing. */
        if (imxrt_i2c->msg_pos < imxrt_i2c->msg->len)
        {
            reVal = LPI2C_MasterSend(imxrt_i2c->i2c_info->hi2c, &imxrt_i2c->msg->buf[imxrt_i2c->msg_pos++], 1);
            if ((reVal != kStatus_Success) || (imxrt_i2c->msg_pos == imxrt_i2c->msg->len))
            {
                LPI2C_MasterDisableInterrupts(imxrt_i2c->i2c_info->hi2c, kLPI2C_MasterTxReadyFlag);
                os_sem_post(&imxrt_i2c->sem);
            }
        }
    }

    if ((flags & kLPI2C_MasterRxReadyFlag) && (imxrt_i2c->msg->flags & OS_I2C_RD))
    {
        /* If rx Index < LPI2C_DATA_LENGTH, master receive->slave send transfer is ongoing. */
        if (imxrt_i2c->msg_pos < imxrt_i2c->msg->len)
        {
            //reVal = LPI2C_MasterReceive(imxrt_i2c->i2c_info->hi2c, &imxrt_i2c->msg->buf[imxrt_i2c->msg_pos++], 1);

            imxrt_i2c->msg->buf[imxrt_i2c->msg_pos++] = imxrt_i2c->i2c_info->hi2c->MRDR;
            reVal = kStatus_Success;
            
            if ((reVal != kStatus_Success) || (imxrt_i2c->msg_pos == imxrt_i2c->msg->len))
            {
                LPI2C_MasterDisableInterrupts(imxrt_i2c->i2c_info->hi2c, kLPI2C_MasterTxReadyFlag);
                os_sem_post(&imxrt_i2c->sem);
            }
        }
    }                                                                     
}

#define LPI2C_IRQHandler_DEFINE(__index)                                        \
void LPI2C##__index##_IRQHandler(void)                                          \
{                                                                               \
    struct imxrt_i2c *imxrt_i2c;                                                \
                                                                                \
    os_list_for_each_entry(imxrt_i2c, &imxrt_i2c_list, struct imxrt_i2c, list)  \
    {                                                                           \
        if (imxrt_i2c->i2c_info->hi2c == LPI2C##__index)                        \
        {                                                                       \
            break;                                                              \
        }                                                                       \
    }                                                                           \
                                                                                \
    if (imxrt_i2c->i2c_info->hi2c == LPI2C##__index)                            \
        i2c_irq_callback(imxrt_i2c);                                            \
}

LPI2C_IRQHandler_DEFINE(1);
LPI2C_IRQHandler_DEFINE(2);
LPI2C_IRQHandler_DEFINE(3);
LPI2C_IRQHandler_DEFINE(4);

static os_size_t imxrt_i2c_mst_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    struct imxrt_i2c *imxrt_i2c;
    int i;
    status_t  ret;
    OS_ASSERT(bus != OS_NULL);
    imxrt_i2c = (struct imxrt_i2c *) bus;

    for (i = 0; i < num; i++)
    {
        imxrt_i2c->msg = &msgs[i];
        imxrt_i2c->msg_pos = 0;
    
        if (!(msgs[i].flags & OS_I2C_NO_START))
        {
            if (msgs[i].flags & OS_I2C_RD)
            {
                ret = LPI2C_MasterStart(imxrt_i2c->i2c_info->hi2c, msgs[i].addr, kLPI2C_Read);
                imxrt_i2c->i2c_info->hi2c->MTDR = LPI2C_MTDR_CMD(0X1U) | LPI2C_MTDR_DATA(msgs[i].len - 1);
            }
            else
            {
                ret = LPI2C_MasterStart(imxrt_i2c->i2c_info->hi2c, msgs[i].addr, kLPI2C_Write);
            }
        
            if (ret != kStatus_Success)
            {
                i = 0;
                break;
            }
        }

        if (msgs[i].flags & OS_I2C_RD)
        {
            LPI2C_MasterEnableInterrupts(imxrt_i2c->i2c_info->hi2c, kLPI2C_MasterRxReadyFlag);
        }
        else
        {
            LPI2C_MasterEnableInterrupts(imxrt_i2c->i2c_info->hi2c, kLPI2C_MasterTxReadyFlag);
        }

        os_sem_wait(&imxrt_i2c->sem, OS_IPC_WAITING_FOREVER);
    }

    LPI2C_MasterStop(imxrt_i2c->i2c_info->hi2c);

    imxrt_i2c->msg = OS_NULL;
    imxrt_i2c->msg_pos = 0;

    return i;
}

static os_size_t imxrt_i2c_slv_xfer(struct os_i2c_bus_device *bus,
                                    struct os_i2c_msg msgs[],
                                    os_uint32_t num)
{
    return 0;
}
static os_err_t imxrt_i2c_bus_control(struct os_i2c_bus_device *bus, void *arg)
{
    return OS_ERROR;
}

static const struct os_i2c_bus_device_ops imxrt_i2c_ops =
{
    .i2c_transfer       = imxrt_i2c_mst_xfer,
    .i2c_slave_transfer = imxrt_i2c_slv_xfer,
    .i2c_bus_control    = imxrt_i2c_bus_control,
};

static int imxrt_i2c_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct nxp_lpi2c_info *i2c_info = (struct nxp_lpi2c_info *)dev->info;
    struct imxrt_i2c *imxrt_i2c = os_calloc(1, sizeof(struct imxrt_i2c));

    OS_ASSERT(imxrt_i2c);

    char name[16] = "i2c_";
    strcat(name, dev->name);
    os_sem_init(&imxrt_i2c->sem, name, 0, OS_IPC_FLAG_FIFO);

    /*Clock setting for LPI2C*/
    CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    imxrt_i2c->i2c_info = i2c_info;

    struct os_i2c_bus_device *dev_i2c = &imxrt_i2c->i2c;

    dev_i2c->ops = &imxrt_i2c_ops;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&imxrt_i2c_list, &imxrt_i2c->list);
    os_hw_interrupt_enable(level);
   
    result = os_i2c_bus_device_register(dev_i2c,
                                        dev->name,
                                        dev_i2c);
    
    OS_ASSERT(result == OS_EOK);
    
    return result;
}

OS_DRIVER_INFO imxrt_i2c_driver = {
    .name   = "LPI2C_Type",
    .probe  = imxrt_i2c_probe,
};

OS_DRIVER_DEFINE(imxrt_i2c_driver, "2");

