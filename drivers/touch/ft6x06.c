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
 * @file        ft6x06.c
 *
 * @brief       ft6x06
 *
 * @details     ft6x06
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_clock.h>
#include <i2c.h>
#include <string.h>
#include <stdlib.h>
#include <os_errno.h>
#include <os_memory.h>
#include <pin/pin.h>
#include <string.h>
#include <drv_gpio.h>
#define LOG_TAG "ft6x06"
#define DBG_LVL DBG_INFO
#include <drv_log.h>

#include "touch.h"
#include "ft6x06.h"

#define FT6x06_MAX_INSTANCE 2

struct ft6x06_touch {
    os_touch_t touch_device;
    struct os_i2c_bus_device *i2c_bus;
    os_uint16_t               i2c_addr;
    os_uint16_t id;
    /* field holding the current number of simultaneous active touches */
    os_uint8_t  currActiveTouchNb;
    /* field holding the touch index currently managed */
    os_uint8_t  currActiveTouchIdx;
};

static void TS_IO_Write(struct ft6x06_touch *ft6x06, uint8_t reg, uint8_t value)
{
    unsigned char buff[2] = {reg, value};
    os_i2c_master_send(ft6x06->i2c_bus, ft6x06->i2c_addr, 0, buff, 2);
}

static uint8_t TS_IO_Read(struct ft6x06_touch *ft6x06, uint8_t reg)
{
    unsigned char value;
    os_i2c_master_send(ft6x06->i2c_bus, ft6x06->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(ft6x06->i2c_bus, ft6x06->i2c_addr, 0, &value, 1);
    return value;
}

uint16_t TS_IO_ReadMultiple(struct ft6x06_touch *ft6x06, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    os_i2c_master_send(ft6x06->i2c_bus, ft6x06->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(ft6x06->i2c_bus, ft6x06->i2c_addr, 0, buffer, length);
    return 0;
}

static void TS_IO_Delay(uint32_t delay)
{
    os_task_msleep(delay);
}

void ft6x06_Init(struct ft6x06_touch *ft6x06);

void ft6x06_Reset(struct ft6x06_touch *ft6x06);

uint16_t ft6x06_ReadID(struct ft6x06_touch *ft6x06);

void ft6x06_TS_Start(struct ft6x06_touch *ft6x06);

uint8_t ft6x06_TS_DetectTouch(struct ft6x06_touch *ft6x06);

void ft6x06_TS_GetXY(struct ft6x06_touch *ft6x06, uint16_t *X, uint16_t *Y);

void ft6x06_TS_EnableIT(struct ft6x06_touch *ft6x06);

void ft6x06_TS_DisableIT(struct ft6x06_touch *ft6x06);

uint8_t ft6x06_TS_ITStatus(struct ft6x06_touch *ft6x06);

void ft6x06_TS_ClearIT(struct ft6x06_touch *ft6x06);

void ft6x06_TS_GetGestureID(struct ft6x06_touch *ft6x06, uint32_t *pGestureId);

void ft6x06_TS_GetTouchInfo(uint16_t  DeviceAddr,
                            uint32_t  touchIdx,
                            uint32_t *pWeight,
                            uint32_t *pArea,
                            uint32_t *pEvent);

#if (TS_AUTO_CALIBRATION_SUPPORTED == 1)
static uint32_t ft6x06_TS_Calibration(struct ft6x06_touch *ft6x06);
#endif

static uint32_t ft6x06_TS_Configure(struct ft6x06_touch *ft6x06);

void ft6x06_Reset(struct ft6x06_touch *ft6x06)
{
    /* Do nothing */
    /* No software reset sequence available in FT6206 IC */
}

uint16_t ft6x06_ReadID(struct ft6x06_touch *ft6x06)
{
    return (TS_IO_Read(ft6x06, FT6206_CHIP_ID_REG));
}

void ft6x06_TS_Start(struct ft6x06_touch *ft6x06)
{
#if (TS_AUTO_CALIBRATION_SUPPORTED == 1)
    /* Hw Calibration sequence start : should be done once after each power up */
    /* This is called internal calibration of the touch screen                 */
    ft6x06_TS_Calibration(ft6x06);
#endif
    /* Minimum static configuration of FT6206 */
    ft6x06_TS_Configure(ft6x06);

    /* By default set FT6206 IC in Polling mode : no INT generation on FT6206 for new touch available */
    /* Note TS_INT is active low                                                                      */
    ft6x06_TS_DisableIT(ft6x06);

    TS_IO_Delay(100);
}

uint8_t ft6x06_TS_DetectTouch(struct ft6x06_touch *ft6x06)
{
    volatile uint8_t nbTouch = 0;

    /* Read register FT6206_TD_STAT_REG to check number of touches detection */
    nbTouch = TS_IO_Read(ft6x06, FT6206_TD_STAT_REG);
    nbTouch &= FT6206_TD_STAT_MASK;

    if (nbTouch > FT6206_MAX_DETECTABLE_TOUCH)
    {
        /* If invalid number of touch detected, set it to zero */
        nbTouch = 0;
    }

    /* Update ft5336 driver internal global : current number of active touches */
    ft6x06->currActiveTouchNb = nbTouch;

    /* Reset current active touch index on which to work on */
    ft6x06->currActiveTouchIdx = 0;

    return nbTouch;
}

void ft6x06_TS_GetXY(struct ft6x06_touch *ft6x06, uint16_t *X, uint16_t *Y)
{
    uint8_t regAddress = 0;
    uint8_t dataxy[4];

    if (ft6x06->currActiveTouchIdx >= ft6x06->currActiveTouchNb)
        return;

    switch (ft6x06->currActiveTouchIdx)
    {
    case 0:
        regAddress = FT6206_P1_XH_REG;
        break;
    case 1:
        regAddress = FT6206_P2_XH_REG;
        break;

    default:
        break;
    }

    /* Read X and Y positions */
    TS_IO_ReadMultiple(ft6x06, regAddress, dataxy, sizeof(dataxy));

    /* Send back ready X position to caller */
    *X = ((dataxy[0] & FT6206_MSB_MASK) << 8) | (dataxy[1] & FT6206_LSB_MASK);

    /* Send back ready Y position to caller */
    *Y = ((dataxy[2] & FT6206_MSB_MASK) << 8) | (dataxy[3] & FT6206_LSB_MASK);

    ft6x06->currActiveTouchIdx++;
}

void ft6x06_TS_EnableIT(struct ft6x06_touch *ft6x06)
{
    uint8_t regValue = 0;
    regValue = (FT6206_G_MODE_INTERRUPT_TRIGGER & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT))
               << FT6206_G_MODE_INTERRUPT_SHIFT;

    /* Set interrupt trigger mode in FT6206_GMODE_REG */
    TS_IO_Write(ft6x06, FT6206_GMODE_REG, regValue);
}

void ft6x06_TS_DisableIT(struct ft6x06_touch *ft6x06)
{
    uint8_t regValue = 0;
    regValue = (FT6206_G_MODE_INTERRUPT_POLLING & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT))
               << FT6206_G_MODE_INTERRUPT_SHIFT;

    /* Set interrupt polling mode in FT6206_GMODE_REG */
    TS_IO_Write(ft6x06, FT6206_GMODE_REG, regValue);
}

uint8_t ft6x06_TS_ITStatus(struct ft6x06_touch *ft6x06)
{
    /* Always return 0 as feature not applicable to FT6206 */
    return 0;
}

void ft6x06_TS_ClearIT(struct ft6x06_touch *ft6x06)
{
    /* Nothing to be done here for FT6206 */
}

/**** NEW FEATURES enabled when Multi-touch support is enabled ****/

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

void ft6x06_TS_GetGestureID(struct ft6x06_touch *ft6x06, uint32_t *pGestureId)
{
    volatile uint8_t ucReadData = 0;

    ucReadData = TS_IO_Read(ft6x06, FT6206_GEST_ID_REG);

    *pGestureId = ucReadData;
}

void ft6x06_TS_GetTouchInfo(struct ft6x06_touch *ft6x06,
                            uint32_t             touchIdx,
                            uint32_t *           pWeight,
                            uint32_t *           pArea,
                            uint32_t *           pEvent)
{
    uint8_t regAddress = 0;
    uint8_t dataxy[3];

    if (touchIdx < ft6x06_handle.currActiveTouchNb)
    {
        switch (touchIdx)
        {
        case 0:
            regAddress = FT6206_P1_WEIGHT_REG;
            break;

        case 1:
            regAddress = FT6206_P2_WEIGHT_REG;
            break;

        default:
            break;

        } /* end switch(touchIdx) */

        /* Read weight, area and Event Id of touch index */
        TS_IO_ReadMultiple(ft6x06, regAddress, dataxy, sizeof(dataxy));

        /* Return weight of touch index */
        *pWeight = (dataxy[0] & FT6206_TOUCH_WEIGHT_MASK) >> FT6206_TOUCH_WEIGHT_SHIFT;
        /* Return area of touch index */
        *pArea = (dataxy[1] & FT6206_TOUCH_AREA_MASK) >> FT6206_TOUCH_AREA_SHIFT;
        /* Return Event Id  of touch index */
        *pEvent = (dataxy[2] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;

    } /* of if(touchIdx < ft6x06_handle.currActiveTouchNb) */
}

#endif

#if (TS_AUTO_CALIBRATION_SUPPORTED == 1)

static uint32_t ft6x06_TS_Calibration(struct ft6x06_touch *ft6x06)
{
    uint32_t         nbAttempt = 0;
    volatile uint8_t ucReadData;
    volatile uint8_t regValue;
    uint32_t         status          = FT6206_STATUS_OK;
    uint8_t          bEndCalibration = 0;

    /* >> Calibration sequence start */

    /* Switch FT6206 back to factory mode to calibrate */
    regValue = (FT6206_DEV_MODE_FACTORY & FT6206_DEV_MODE_MASK) << FT6206_DEV_MODE_SHIFT;
    TS_IO_Write(ft6x06, FT6206_DEV_MODE_REG, regValue); /* 0x40 */

    /* Read back the same register FT6206_DEV_MODE_REG */
    ucReadData = TS_IO_Read(ft6x06, FT6206_DEV_MODE_REG);
    TS_IO_Delay(300); /* Wait 300 ms */

    if (((ucReadData & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT) != FT6206_DEV_MODE_FACTORY)
    {
        /* Return error to caller */
        return (FT6206_STATUS_NOT_OK);
    }

    /* Start calibration command */
    TS_IO_Write(ft6x06, FT6206_TD_STAT_REG, 0x04);
    TS_IO_Delay(300); /* Wait 300 ms */

    /* 100 attempts to wait switch from factory mode (calibration) to working mode */
    for (nbAttempt = 0; ((nbAttempt < 100) && (!bEndCalibration)); nbAttempt++)
    {
        ucReadData = TS_IO_Read(ft6x06, FT6206_DEV_MODE_REG);
        ucReadData = (ucReadData & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT;
        if (ucReadData == FT6206_DEV_MODE_WORKING)
        {
            /* Auto Switch to FT6206_DEV_MODE_WORKING : means calibration have ended */
            bEndCalibration = 1; /* exit for loop */
        }

        TS_IO_Delay(200); /* Wait 200 ms */
    }

    /* Calibration sequence end << */

    return (status);
}
#endif /* TS_AUTO_CALIBRATION_SUPPORTED == 1 */

static uint32_t ft6x06_TS_Configure(struct ft6x06_touch *ft6x06)
{
    uint32_t status = FT6206_STATUS_OK;

    /* Nothing special to be done for FT6206 */

    return (status);
}

static os_size_t ft6x06_read_point(struct os_touch_device *touch, struct os_touch_data *data, os_size_t read_num)
{
    uint16_t input_x  = 0;
    uint16_t input_y  = 0;
    uint16_t input_w  = 0;
    uint16_t new_data = 0;

    struct ft6x06_touch *ft6x06 = (struct ft6x06_touch *)touch;

    new_data = ft6x06_TS_DetectTouch(ft6x06);
    if (new_data == 0)
        return 0;

    ft6x06_TS_GetXY(ft6x06, &input_x, &input_y);

    data->event        = OS_TOUCH_EVENT_DOWN;
    data->timestamp    = os_touch_get_ts();
    data->width        = input_w;
    data->x_coordinate = input_x;
    data->y_coordinate = input_y;
    data->track_id     = 0;

    return 1;
}

static os_err_t ft6x06_control(struct os_touch_device *device, int cmd, void *data)
{
    struct ft6x06_touch *ft6x06 = (struct ft6x06_touch *)device;

    switch (cmd)
    {
    case OS_TOUCH_CTRL_GET_ID:
    {
        uint16_t id  = ft6x06_ReadID(ft6x06);
        uint8_t *idp = data;
        *idp++       = id & 0xff;
        *idp++       = id >> 8;
        break;
    }
    case OS_TOUCH_CTRL_GET_INFO:
    {
        struct os_touch_info *info = data;
        *info                      = device->info;
        break;
    }
    case OS_TOUCH_CTRL_SET_MODE: /* change int trig type */
        break;
    default:
        break;
    }

    return OS_EOK;
}

static struct os_touch_ops ft6x06_touch_ops =
{
    .touch_readpoint = ft6x06_read_point,
    .touch_control   = ft6x06_control,
};

#define TS_I2C_ADDRESS                   ((uint16_t)0x2a)
#define TS_I2C_ADDRESS_A02               ((uint16_t)0x38)

static int os_hw_ft6x06_init(void)
{
    struct ft6x06_touch *ft6x06 = os_calloc(1, sizeof(struct ft6x06_touch));
    OS_ASSERT(ft6x06);

    ft6x06->i2c_bus  = os_i2c_bus_device_find(OS_FT6X060_I2C_BUS_NAME);
    OS_ASSERT(ft6x06->i2c_bus);

    ft6x06->i2c_addr = TS_I2C_ADDRESS;
    ft6x06->id = ft6x06_ReadID(ft6x06);
    if (ft6x06->id != FT6206_ID_VALUE)
    {
        ft6x06->i2c_addr = TS_I2C_ADDRESS_A02;
        ft6x06->id = ft6x06_ReadID(ft6x06);
        if (ft6x06->id != FT6206_ID_VALUE)
        {
            os_kprintf("invalid ft6x06\r\n");
        }
    }

    ft6x06_Reset(ft6x06);
    ft6x06_TS_Start(ft6x06);

    os_touch_t *touch_device = &ft6x06->touch_device;

    /* register touch device */
    touch_device->info.type      = OS_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor    = OS_TOUCH_VENDOR_UNKNOWN;
    touch_device->info.point_num = 1;
    touch_device->info.range_x   = 480;
    touch_device->info.range_y   = 800;
    touch_device->ops            = &ft6x06_touch_ops;

    os_hw_touch_register(touch_device, "touch", ft6x06);

    return 0;
}

OS_CMPOENT_INIT(os_hw_ft6x06_init, OS_INIT_SUBLEVEL_LOW);
