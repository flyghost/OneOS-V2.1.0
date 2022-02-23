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
 * @file        drv_qspi.c
 *
 * @brief       This file implements QSPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <string.h>
#include "board.h"
#include "drv_qspi.h"
#include <bus/bus.h>

#define DRV_EXT_LVL    DBG_EXT_INFO
#define DRV_EXT_TAG    "drv.qspi"
#include <drv_log.h>

static int stm32_qspi_init(struct os_qspi_device *device, struct os_qspi_configuration *qspi_cfg)
{
    int result = OS_EOK;
    unsigned int i = 1;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(qspi_cfg != OS_NULL);

    struct os_spi_configuration *cfg = &qspi_cfg->parent;
    QSPI_HandleTypeDef *QSPI_Handler = device->parent.bus->parent.user_data;

    while (cfg->max_hz < HAL_RCC_GetHCLKFreq() / (i + 1))
    {
        i++;
        if (i == 255)
        {
            LOG_EXT_E("QSPI init failed, QSPI frequency(%d) is too low.", cfg->max_hz);
            return OS_ERROR;
        }
    }
    /* 80/(1+i) */
    QSPI_Handler->Init.ClockPrescaler = i;

    if (!(cfg->mode & OS_SPI_CPOL))
    {
        /* QSPI MODE0 */
        QSPI_Handler->Init.ClockMode = QSPI_CLOCK_MODE_0;
    }
    else
    {
        /* QSPI MODE3 */
        QSPI_Handler->Init.ClockMode = QSPI_CLOCK_MODE_3;
    }

    /* flash size */
    QSPI_Handler->Init.FlashSize = POSITION_VAL(qspi_cfg->medium_size) - 1;

    result = HAL_QSPI_Init(QSPI_Handler);
    if (result == HAL_OK)
    {
        LOG_EXT_D("qspi init success!");
    }
    else
    {
        LOG_EXT_E("qspi init failed (%d)!", result);
    }

    return result;
}

static void qspi_send_cmd(QSPI_HandleTypeDef *QSPI_Handler, struct os_qspi_message *message)
{
    OS_ASSERT(message != OS_NULL);
    OS_ASSERT(QSPI_Handler != OS_NULL);

    QSPI_CommandTypeDef Cmdhandler;

    /* set QSPI cmd struct */
    Cmdhandler.Instruction = message->instruction.content;
    Cmdhandler.Address     = message->address.content;
    Cmdhandler.DummyCycles = message->dummy_cycles;
    if (message->instruction.qspi_lines == 0)
    {
        Cmdhandler.InstructionMode = QSPI_INSTRUCTION_NONE;
    }
    else if (message->instruction.qspi_lines == 1)
    {
        Cmdhandler.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    else if (message->instruction.qspi_lines == 2)
    {
        Cmdhandler.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    }
    else if (message->instruction.qspi_lines == 4)
    {
        Cmdhandler.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    }
    if (message->address.qspi_lines == 0)
    {
        Cmdhandler.AddressMode = QSPI_ADDRESS_NONE;
    }
    else if (message->address.qspi_lines == 1)
    {
        Cmdhandler.AddressMode = QSPI_ADDRESS_1_LINE;
    }
    else if (message->address.qspi_lines == 2)
    {
        Cmdhandler.AddressMode = QSPI_ADDRESS_2_LINES;
    }
    else if (message->address.qspi_lines == 4)
    {
        Cmdhandler.AddressMode = QSPI_ADDRESS_4_LINES;
    }
    if (message->address.size == 24)
    {
        Cmdhandler.AddressSize = QSPI_ADDRESS_24_BITS;
    }
    else
    {
        Cmdhandler.AddressSize = QSPI_ADDRESS_32_BITS;
    }
    if (message->qspi_data_lines == 0)
    {
        Cmdhandler.DataMode = QSPI_DATA_NONE;
    }
    else if (message->qspi_data_lines == 1)
    {
        Cmdhandler.DataMode = QSPI_DATA_1_LINE;
    }
    else if (message->qspi_data_lines == 2)
    {
        Cmdhandler.DataMode = QSPI_DATA_2_LINES;
    }
    else if (message->qspi_data_lines == 4)
    {
        Cmdhandler.DataMode = QSPI_DATA_4_LINES;
    }

    Cmdhandler.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Cmdhandler.DdrMode           = QSPI_DDR_MODE_DISABLE;
    Cmdhandler.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    Cmdhandler.NbData            = message->parent.length;
    HAL_QSPI_Command(QSPI_Handler, &Cmdhandler, 5000);
}

static os_uint32_t qspixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    os_size_t len = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    struct os_qspi_message *qspi_message = (struct os_qspi_message *)message;
    QSPI_HandleTypeDef *QSPI_Handler     = device->bus->parent.user_data;

    const os_uint8_t *sndb   = message->send_buf;
    os_uint8_t       *rcvb   = message->recv_buf;
    os_int32_t        length = message->length;

#ifdef BSP_QSPI_USING_SOFTCS
    if (message->cs_take)
    {
        os_pin_write(device->cs_pin, PIN_LOW);
    }
#endif

    /* send data */
    if (sndb)
    {
        qspi_send_cmd(QSPI_Handler, qspi_message);
        if (qspi_message->parent.length != 0)
        {
            if (HAL_QSPI_Transmit(QSPI_Handler, (os_uint8_t *)sndb, 5000) == HAL_OK)
            {
                len = length;
            }
            else
            {
                LOG_EXT_E("QSPI send data failed(%d)!", QSPI_Handler->ErrorCode);
                QSPI_Handler->State = HAL_QSPI_STATE_READY;
                goto __exit;
            }
        }
        else
        {
            len = 1;
        }
    }
    else if (rcvb) /* recv data */
    {
        qspi_send_cmd(QSPI_Handler, qspi_message);
#ifdef BSP_QSPI_USING_DMA
        if (HAL_QSPI_Receive_DMA(QSPI_Handler, rcvb) == HAL_OK)
#else
        if (HAL_QSPI_Receive(QSPI_Handler, rcvb, 5000) == HAL_OK)
#endif
        {
            len = length;
#ifdef BSP_QSPI_USING_DMA
            while (QSPI_Handler->RxXferCount != 0);
#endif
        }
        else
        {
            LOG_EXT_E("QSPI recv data failed(%d)!", QSPI_Handler->ErrorCode);
            QSPI_Handler->State = HAL_QSPI_STATE_READY;
            goto __exit;
        }
    }

__exit:
#ifdef BSP_QSPI_USING_SOFTCS
    if (message->cs_release)
    {
        os_pin_write(device->cs_pin, PIN_HIGH);
    }
#endif
    return len;
}

static os_err_t qspi_configure(struct os_spi_device *device, struct os_spi_configuration *configuration)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(configuration != OS_NULL);

    struct os_qspi_device *qspi_device = (struct os_qspi_device *)device;
    return stm32_qspi_init(qspi_device, &qspi_device->config);
}

static const struct os_spi_ops stm32_qspi_ops = {
    .configure = qspi_configure,
    .xfer      = qspixfer,
};

/**
 ***********************************************************************************************************************
 * @brief           Attach device to QSPI bus.
 *
 * @param[in]       bus_name         QSPI bus name.
 * @param[in]       device_name      QSPI device name.
 * @param[in]       pin              CS pin number.
 * @param[in]       data_line_width  QSPI data lines width, such as 1, 2, 4.
 * @param[in]       enter_qspi_mode  Callback function that lets FLASH enter QSPI mode.
 * @param[in]       exit_qspi_mode   Callback function that lets FLASH exit QSPI mode.

 *
 * @return
 * @retval          OS_EOK           Succeed.
 * @retval          Others           Fail.
 ***********************************************************************************************************************
 */
os_err_t stm32_qspi_bus_attach_device(const char *bus_name,
                                      const char *device_name,
                                      os_uint32_t pin,
                                      os_uint8_t  data_line_width,
                                      void (*enter_qspi_mode)(),
                                      void (*exit_qspi_mode)())
{
    struct os_qspi_device  *qspi_device = OS_NULL;
    os_err_t                result      = OS_EOK;

    OS_ASSERT(bus_name != OS_NULL);
    OS_ASSERT(device_name != OS_NULL);
    OS_ASSERT(data_line_width == 1 || data_line_width == 2 || data_line_width == 4);

    qspi_device = (struct os_qspi_device *)os_calloc(1, sizeof(struct os_qspi_device));
    if (qspi_device == OS_NULL)
    {
        LOG_EXT_E("no memory, qspi bus attach device failed!");
        result = OS_ENOMEM;
        goto __exit;
    }
    
    if (qspi_device == OS_NULL)
    {
        LOG_EXT_E("no memory, qspi bus attach device failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    qspi_device->enter_qspi_mode      = enter_qspi_mode;
    qspi_device->exit_qspi_mode       = exit_qspi_mode;
    qspi_device->config.qspi_dl_width = data_line_width;

#ifdef BSP_QSPI_USING_SOFTCS
    os_pin_mode(pin, PIN_MODE_OUTPUT);
    os_pin_write(pin, 1);
#endif

    result = os_spi_bus_attach_device(&qspi_device->parent, device_name, bus_name, pin);

__exit:
    if (result != OS_EOK)
    {
        if (qspi_device)
        {
            os_free(qspi_device);
        }
    }

    return result;
}

static int stm32_qspi_bus_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct os_spi_bus *spi_bus;

    spi_bus = os_calloc(1, sizeof(struct os_spi_bus));
    OS_ASSERT(spi_bus);

    spi_bus->parent.user_data = (QSPI_HandleTypeDef *)dev->info;
    return os_qspi_bus_register(spi_bus, dev->name, &stm32_qspi_ops);
}

OS_DRIVER_INFO stm32_qspi_driver = {
    .name   = "QSPI_HandleTypeDef",
    .probe  = stm32_qspi_bus_probe,
};

OS_DRIVER_DEFINE(stm32_qspi_driver, "1");

