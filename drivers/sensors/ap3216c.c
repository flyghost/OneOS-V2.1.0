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
 * @file        ap3216c.c
 *
 * @brief       This file provides functions for ap3216c.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_util.h>
#include <os_memory.h>
#include <string.h>
#include <stdio.h>
#include <drv_gpio.h>
#include <sensors/sensor.h>

#define DBG_EXT_TAG "sensor.adi.ap3216c"
//#define DBG_EXT_LVL DBG_EXT_INFO
#include <dlog.h>

/* System Register */
#define AP3216C_SYS_CONFIGURATION_REG    0x00 /* Default */
#define AP3216C_SYS_INT_STATUS_REG       0x01
#define AP3216C_SYS_INT_CLEAR_MANNER_REG 0x02
#define AP3216C_IR_DATA_L_REG            0x0A
#define AP3216C_IR_DATA_H_REG            0x0B
#define AP3216C_ALS_DATA_L_REG           0x0C
#define AP3216C_ALS_DATA_H_REG           0x0D
#define AP3216C_PS_DATA_L_REG            0x0E
#define AP3216C_PS_DATA_H_REG            0x0F

/* ALS Register */
#define AP3216C_ALS_CONFIGURATION_REG    0x10 /* Range 5:4,persist 3:0 */
#define AP3216C_ALS_CALIBRATION_REG      0x19
#define AP3216C_ALS_THRESHOLD_LOW_L_REG  0x1A /* Bit 7:0 */
#define AP3216C_ALS_THRESHOLD_LOW_H_REG  0x1B /* Bit 15:8 */
#define AP3216C_ALS_THRESHOLD_HIGH_L_REG 0x1C /* Bit 7:0 */
#define AP3216C_ALS_THRESHOLD_HIGH_H_REG 0x1D /* Bit 15:8 */

/* PS Register */
#define AP3216C_PS_CONFIGURATION_REG    0x20
#define AP3216C_PS_LED_DRIVER_REG       0x21
#define AP3216C_PS_INT_FORM_REG         0x22
#define AP3216C_PS_MEAN_TIME_REG        0x23
#define AP3216C_PS_LED_WAITING_TIME_REG 0x24
#define AP3216C_PS_CALIBRATION_L_REG    0x28
#define AP3216C_PS_CALIBRATION_H_REG    0x29
#define AP3216C_PS_THRESHOLD_LOW_L_REG  0x2A /* Bit 1:0 */
#define AP3216C_PS_THRESHOLD_LOW_H_REG  0x2B /* Bit 9:2 */
#define AP3216C_PS_THRESHOLD_HIGH_L_REG 0x2C /* Bit 1:0 */
#define AP3216C_PS_THRESHOLD_HIGH_H_REG 0x2D /* Bit 9:2 */

enum ap3216c_mode_value
{
    AP3216C_MODE_POWER_DOWN,      /* Power down (Default) */
    AP3216C_MODE_ALS,             /* ALS function active */
    AP3216C_MODE_PS,              /* PS+IR function active */
    AP3216C_MODE_ALS_AND_PS,      /* ALS and PS+IR functions active */
    AP3216C_MODE_SW_RESET,        /* SW reset */
    AP3216C_MODE_ALS_ONCE,        /* ALS function once */
    AP3216C_MODE_PS_ONCE,         /* PS+IR function once */
    AP3216C_MODE_ALS_AND_PS_ONCE, /* ALS and PS+IR functions once */
};

enum ap3216c_int_clear_manner
{
    AP3216C_INT_CLEAR_MANNER_BY_READING,  /* INT is automatically cleared by reading data registers(Default) */
    AP3216C_ALS_CLEAR_MANNER_BY_SOFTWARE, /* Software clear after writing 1 into address 0x01 each bit */
};

enum als_range
{
    AP3216C_ALS_RANGE_20661, /* Resolution = 0.35 lux/count(default). */
    AP3216C_ALS_RANGE_5162,  /* Resolution = 0.0788 lux/count. */
    AP3216C_ALS_RANGE_1291,  /* Resolution = 0.0197 lux/count. */
    AP3216C_ALS_RANGE_323,   /* Resolution = 0.0049 lux/count */
};
typedef enum als_range als_range_t;

enum als_gain
{
    AP3216C_ALS_GAIN1, /* detection distance *1. */
    AP3216C_ALS_GAIN2, /* detection distance *2 (default). */
    AP3216C_ALS_GAIN4, /* detection distance *4. */
    AP3216C_ALS_GAIN8, /* detection distance *8. */
};
typedef enum als_gain als_gain_t;

enum ap3216c_cmd
{
    AP3216C_SYSTEM_MODE,          /* System  Configuration(Default : 000) */
    AP3216C_INT_PARAM,            /* INT Clear Manner(Default : 0) */
    AP3216C_ALS_RANGE,            /* ALS dynamic range(Default : 00) */
    AP3216C_ALS_PERSIST,          /* ALS persist(Default : 0000) */
    AP3216C_ALS_CALIBRATION,      /* ALS window loss calibration(Default : 0x40) */
    AP3216C_ALS_LOW_THRESHOLD_L,  /* Lower byte of low interrupt threshold for ALS(Default : 0x00) */
    AP3216C_ALS_LOW_THRESHOLD_H,  /* Higher byte of low interrupt threshold for ALS(Default : 0x00) */
    AP3216C_ALS_HIGH_THRESHOLD_L, /* Lower byte of high interrupt threshold for ALS (Default : 0xFF) */
    AP3216C_ALS_HIGH_THRESHOLD_H, /* Higher byte of high interrupt threshold for ALS(Default : 0xFF) */
    AP3216C_PS_INTEGRATED_TIME,   /* PS or IR Integrated time select(Default : 0000) */
    AP3216C_PS_GAIN,              /* PS gain (Default : 01) */
    AP3216C_PS_PERSIST,           /* Interrupt filter(Default : 01) */
    AP3216C_PS_LED_CONTROL,       /* LED pulse(Default : 01) */
    AP3216C_PS_LED_DRIVER_RATIO,  /* LED driver ratio(Default : 11) */
    AP3216C_PS_INT_MODE,          /* PS INT Mode(Default : 0x01) */
    AP3216C_PS_MEAN_TIME,         /* PS mean time(Default : 0x00) */
    AP3216C_PS_WAITING_TIME,      /* PS LED Waiting(Default : 0x00) */
    AP3216C_PS_CALIBRATION_L,     /* PS Calibration L(Default : 0x00) */
    AP3216C_PS_CALIBRATION_H,     /* PS Calibration H(Default : 0x00) */
    AP3216C_PS_LOW_THRESHOLD_L,   /* PS Low Threshold L(Default :0x00) */
    AP3216C_PS_LOW_THRESHOLD_H,   /* PS Low Threshold H(Default :0x00) */
    AP3216C_PS_HIGH_THRESHOLD_L,  /* PS high Threshold L(Default :0xff) */
    AP3216C_PS_HIGH_THRESHOLD_H,  /* PS high Threshold H(Default :0xff) */
};
typedef enum ap3216c_cmd ap3216c_cmd_t;

#ifdef AP3216C_USING_HW_INT

struct ap3216c_threshold
{
    os_uint16_t min;         /* Als 16 bits, ps 10 bits available(0-1 bit and 8-15 bit ) */
    os_uint16_t max;         /* Als 16 bits, ps 10 bits available(0-1 bit and 8-15 bit ) */
    os_uint8_t  noises_time; /* Filter special noises trigger interrupt */
};
typedef struct ap3216c_threshold ap3216c_threshold_t;

typedef void (*ap3216c_int_cb)(void *args);
#endif

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

#ifdef AP3216C_USING_HW_INT
    ap3216c_int_cb als_int_cb;
    ap3216c_int_cb ps_int_cb;
#endif
} ap3216c_info_t;

static os_err_t write_reg(ap3216c_info_t *ap3216c, os_uint8_t reg, os_uint8_t data)
{
    return os_i2c_client_write_byte(&ap3216c->i2c, reg, 1, data);
}

static os_err_t read_regs(ap3216c_info_t *ap3216c, os_uint8_t reg, os_uint8_t len, os_uint8_t *buf)
{
    return os_i2c_client_read(&ap3216c->i2c, reg, 1, buf, len);
}

static os_err_t reset_sensor(ap3216c_info_t *ap3216c)
{
    OS_ASSERT(ap3216c);

    write_reg(ap3216c, AP3216C_SYS_CONFIGURATION_REG, AP3216C_MODE_SW_RESET);

    return OS_EOK;
}

static os_uint32_t read_low_and_high(ap3216c_info_t *ap3216c, os_uint8_t reg, os_uint8_t len)
{
    os_uint32_t data;
    os_uint8_t  buf = 0;

    read_regs(ap3216c, reg, len, &buf);    // low
    data = buf;
    read_regs(ap3216c, reg + 1, len, &buf);    // high
    data = data + (buf << len * 8);

    return data;
}

#ifdef AP3216C_USING_HW_INT

static void set_threshold(ap3216c_info_t *ap3216c, ap3216c_cmd_t cmd, ap3216c_threshold_t threshold)
{
    ap3216c_set_param(ap3216c, cmd, (threshold.min & 0xff));
    ap3216c_set_param(ap3216c, (ap3216c_cmd_t)(cmd + 1), (threshold.min >> 8));
    ap3216c_set_param(ap3216c, (ap3216c_cmd_t)(cmd + 2), (threshold.max & 0xff));
    ap3216c_set_param(ap3216c, (ap3216c_cmd_t)(cmd + 3), threshold.max >> 8);
}

static void ap3216c_hw_interrupt(void *args)
{
    ap3216c_info_t *ap3216c = (ap3216c_info_t *)args;

    if (ap3216c->als_int_cb)
    {
        ap3216c->als_int_cb(ap3216c->als_int_cb);
    }
    if (ap3216c->ps_int_cb)
    {
        ap3216c->ps_int_cb(ap3216c->ps_int_cb);
    }
}

static void ap3216c_int_init(ap3216c_info_t *ap3216c)
{
    OS_ASSERT(ap3216c);

    os_pin_mode(AP3216C_INT_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(AP3216C_INT_PIN, PIN_IRQ_MODE_FALLING, ap3216c_hw_interrupt, (void *)ap3216c);
    os_pin_irq_enable(AP3216C_INT_PIN, PIN_IRQ_ENABLE);
}

void ap3216c_int_als_cb(ap3216c_info_t *    ap3216c,
                        os_bool_t           enabled,
                        ap3216c_threshold_t threshold,
                        ap3216c_int_cb      int_cb)
{
    OS_ASSERT(ap3216c);

    if (enabled)
    {
        ap3216c->als_int_cb = int_cb;
        set_threshold(ap3216c, AP3216C_ALS_LOW_THRESHOLD_L, threshold);
    }
    else
    {
        ap3216c->als_int_cb = OS_NULL;
    }
}

void ap3216c_int_ps_cb(ap3216c_info_t *ap3216c, os_bool_t enabled, ap3216c_threshold_t threshold, ap3216c_int_cb int_cb)
{
    OS_ASSERT(ap3216c);

    if (enabled)
    {
        ap3216c->ps_int_cb = int_cb;
        set_threshold(ap3216c, AP3216C_PS_LOW_THRESHOLD_L, threshold);
    }
    else
    {
        ap3216c->ps_int_cb = OS_NULL;
    }
}

#endif /* AP3216C_USING_HW_INT */

static os_err_t ap3216c_set_param(ap3216c_info_t *ap3216c, ap3216c_cmd_t cmd, os_uint8_t value)
{
    OS_ASSERT(ap3216c);

    switch (cmd)
    {
    case AP3216C_SYSTEM_MODE:
    {
        if (value > AP3216C_MODE_ALS_AND_PS_ONCE)
        {
            LOG_E(DBG_EXT_TAG, "Setting system mode parameter is wrong !");
            return OS_ERROR;
        }
        /* Default 000,power down */
        write_reg(ap3216c, AP3216C_SYS_CONFIGURATION_REG, value);

        break;
    }
    case AP3216C_INT_PARAM:
    {
        if (value > AP3216C_ALS_CLEAR_MANNER_BY_SOFTWARE)
        {
            LOG_E(DBG_EXT_TAG, "Setting int parameter is wrong !");
            return OS_ERROR;
        }
        write_reg(ap3216c, AP3216C_SYS_INT_CLEAR_MANNER_REG, value);

        break;
    }

    case AP3216C_ALS_RANGE:
    {
        os_uint8_t args;

        if (!(value == AP3216C_ALS_RANGE_20661 || value == AP3216C_ALS_RANGE_5162 || value == AP3216C_ALS_RANGE_1291 ||
              value == AP3216C_ALS_RANGE_323))
        {
            LOG_E(DBG_EXT_TAG, "Setting als dynamic range is wrong, please refer als_range");
            return OS_ERROR;
        }
        read_regs(ap3216c, AP3216C_ALS_CONFIGURATION_REG, 1, &args);
        args &= 0xcf;
        args |= value << 4;
        write_reg(ap3216c, AP3216C_ALS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_ALS_PERSIST:
    {
        os_uint8_t args = 0;

        if (value > 0x0f)
        {
            LOG_E(DBG_EXT_TAG, "Setting als persist overflows ");
            return OS_ERROR;
        }
        read_regs(ap3216c, AP3216C_ALS_CONFIGURATION_REG, 1, &args);
        args &= 0xf0;
        args |= value;
        write_reg(ap3216c, AP3216C_ALS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_L:
    {
        write_reg(ap3216c, AP3216C_ALS_THRESHOLD_LOW_L_REG, value);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_H:
    {
        write_reg(ap3216c, AP3216C_ALS_THRESHOLD_LOW_H_REG, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_L:
    {
        write_reg(ap3216c, AP3216C_ALS_THRESHOLD_HIGH_L_REG, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_H:
    {
        write_reg(ap3216c, AP3216C_ALS_THRESHOLD_HIGH_H_REG, value);

        break;
    }
    case AP3216C_PS_GAIN:
    {
        os_uint8_t args = 0;

        if (value > 0x3)
        {
            LOG_E(DBG_EXT_TAG, "Setting ps again overflows ");
            return OS_ERROR;
        }
        read_regs(ap3216c, AP3216C_PS_CONFIGURATION_REG, 1, &args);
        args &= 0xf3;
        args |= value;
        write_reg(ap3216c, AP3216C_PS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_PS_PERSIST:
    {
        os_uint8_t args = 0;

        if (value > 0x3)
        {
            LOG_E(DBG_EXT_TAG, "Setting ps persist overflows ");
            return OS_ERROR;
        }
        read_regs(ap3216c, AP3216C_PS_CONFIGURATION_REG, 1, &args);
        args &= 0xfc;
        args |= value;
        write_reg(ap3216c, AP3216C_PS_CONFIGURATION_REG, args);

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_L:
    {
        if (value > 0x3)
        {
            LOG_E(DBG_EXT_TAG, "Setting ps low threshold of low bit is wrong !");
            return OS_ERROR;
        }
        write_reg(ap3216c, AP3216C_PS_THRESHOLD_LOW_L_REG, value);

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_H:
    {
        write_reg(ap3216c, AP3216C_PS_THRESHOLD_LOW_H_REG, value);

        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_L:
    {
        if (value > 0x3)
        {
            LOG_E(DBG_EXT_TAG, "Setting ps high threshold of low bit is wrong !");
            return OS_ERROR;
        }
        write_reg(ap3216c, AP3216C_PS_THRESHOLD_HIGH_L_REG, value);

        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_H:
    {
        write_reg(ap3216c, AP3216C_PS_THRESHOLD_HIGH_H_REG, value);

        break;
    }

    default:
    {
        return OS_ERROR;
    }
    }

    return OS_EOK;
}

static os_err_t ap3216c_get_param(ap3216c_info_t *ap3216c, ap3216c_cmd_t cmd, os_uint8_t *value)
{
    OS_ASSERT(ap3216c);

    switch (cmd)
    {
    case AP3216C_SYSTEM_MODE:
    {
        read_regs(ap3216c, AP3216C_SYS_CONFIGURATION_REG, 1, value);

        if (*value > AP3216C_MODE_ALS_AND_PS_ONCE)
        {
            LOG_E(DBG_EXT_TAG, "Getting system mode parameter is wrong !");
            return OS_ERROR;
        }
        break;
    }
    case AP3216C_INT_PARAM:
    {
        read_regs(ap3216c, AP3216C_SYS_INT_CLEAR_MANNER_REG, 1, value);

        if (*value > AP3216C_ALS_CLEAR_MANNER_BY_SOFTWARE)
        {
            LOG_E(DBG_EXT_TAG, "Setting int parameter is wrong !");
            return OS_ERROR;
        }
        break;
    }
    case AP3216C_ALS_RANGE:
    {
        os_uint8_t temp;

        read_regs(ap3216c, AP3216C_ALS_CONFIGURATION_REG, 1, value);
        temp = (*value & 0xff) >> 4;

        if (!(temp == AP3216C_ALS_RANGE_20661 || temp == AP3216C_ALS_RANGE_5162 || temp == AP3216C_ALS_RANGE_1291 ||
              temp == AP3216C_ALS_RANGE_323))
        {
            LOG_E(DBG_EXT_TAG, "Getting als dynamic range is wrong, please refer als_range");
            return OS_ERROR;
        }

        *value = temp;

        break;
    }
    case AP3216C_ALS_PERSIST:
    {
        os_uint8_t temp;

        read_regs(ap3216c, AP3216C_ALS_CONFIGURATION_REG, 1, value);
        temp = *value & 0x0f;

        if (temp > 0x0f)
        {
            LOG_E(DBG_EXT_TAG, "Getting als persist is wrong, please refer als_range");
            return OS_ERROR;
        }
        *value = temp;

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_L:
    {
        read_regs(ap3216c, AP3216C_ALS_THRESHOLD_LOW_L_REG, 1, value);

        break;
    }
    case AP3216C_ALS_LOW_THRESHOLD_H:
    {
        read_regs(ap3216c, AP3216C_ALS_THRESHOLD_LOW_H_REG, 1, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_L:
    {
        read_regs(ap3216c, AP3216C_ALS_THRESHOLD_HIGH_L_REG, 1, value);

        break;
    }
    case AP3216C_ALS_HIGH_THRESHOLD_H:
    {
        read_regs(ap3216c, AP3216C_ALS_THRESHOLD_HIGH_H_REG, 1, value);

        break;
    }
    case AP3216C_PS_GAIN:
    {
        os_uint8_t temp;

        read_regs(ap3216c, AP3216C_PS_CONFIGURATION_REG, 1, &temp);

        *value = (temp & 0xc) >> 2;

        break;
    }
    case AP3216C_PS_PERSIST:
    {
        os_uint8_t temp;

        read_regs(ap3216c, AP3216C_PS_CONFIGURATION_REG, 1, &temp);

        *value = temp & 0x3;

        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_L:
    {
        read_regs(ap3216c, AP3216C_PS_THRESHOLD_LOW_L_REG, 1, value);
        if ((*value & 0xff) > 0x3)
        {
            LOG_E(DBG_EXT_TAG, "Getting ps low threshold of low bit is wrong !");
            return OS_ERROR;
        }
        break;
    }
    case AP3216C_PS_LOW_THRESHOLD_H:
    {
        read_regs(ap3216c, AP3216C_PS_THRESHOLD_LOW_H_REG, 1, value);
        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_L:
    {
        read_regs(ap3216c, AP3216C_PS_THRESHOLD_HIGH_L_REG, 1, value);

        if ((*value & 0xff) > 3)
        {
            LOG_E(DBG_EXT_TAG, "Getting ps high threshold of low bit is wrong !");
            return OS_ERROR;
        }
        break;
    }
    case AP3216C_PS_HIGH_THRESHOLD_H:
    {
        read_regs(ap3216c, AP3216C_PS_THRESHOLD_HIGH_H_REG, 1, value);

        break;
    }

    default:
    {
        return OS_ERROR;
    }
    }

    return OS_EOK;
}

float ap3216c_read_ambient_light(ap3216c_info_t *ap3216c)
{
    float       brightness = 0.0; /* default error data */
    os_uint32_t read_data;
    os_uint8_t  temp;

    OS_ASSERT(ap3216c);

    read_data = read_low_and_high(ap3216c, AP3216C_ALS_DATA_L_REG, 1);

    ap3216c_get_param(ap3216c, AP3216C_ALS_RANGE, &temp);
    if (temp == AP3216C_ALS_RANGE_20661)
    {
        brightness = (0.35 * read_data) * 1000; /* sensor ambient light converse to reality */
    }
    else if (temp == AP3216C_ALS_RANGE_5162)
    {
        brightness = (0.0788 * read_data) * 1000; /* sensor ambient light converse to reality */
    }
    else if (temp == AP3216C_ALS_RANGE_1291)
    {
        brightness = (0.0197 * read_data) * 1000; /* sensor ambient light converse to reality */
    }
    else if (temp == AP3216C_ALS_RANGE_323)
    {
        brightness = (0.0049 * read_data) * 1000; /* sensor ambient light converse to reality */
    }
    else
    {
        LOG_E(DBG_EXT_TAG, "Failed to get range of ap3216c");
    }

    return brightness;
}

os_uint16_t ap3216c_read_ps_data(ap3216c_info_t *ap3216c)
{
    os_uint16_t proximity = 0;
    OS_ASSERT(ap3216c);

    os_uint32_t read_data;
    read_data = read_low_and_high(ap3216c, AP3216C_PS_DATA_L_REG, 1); /* read two data */

    if (1 == ((read_data >> 6) & 0x01 || (read_data >> 14) & 0x01))
    {
        LOG_I(DBG_EXT_TAG, "The data of PS is invalid for high intensive IR light ");
    }

    proximity = (read_data & 0x000f) + (((read_data >> 8) & 0x3f) << 4); /* sensor proximity converse to reality */

    return proximity;
}

os_uint16_t ap3216c_read_ir_data(ap3216c_info_t *ap3216c)
{
    os_uint16_t ir_value = 0;
    OS_ASSERT(ap3216c);

    os_uint32_t read_data;
    read_data = read_low_and_high(ap3216c, AP3216C_IR_DATA_L_REG, 1); /* read two data */

    if(1 == (read_data & 0x0080))
    {
        LOG_I(DBG_EXT_TAG, "The data of IR is invalid ");
    }
    
    ir_value = ((read_data & 0xff00) >> 6)  + ((read_data & 0x0003)); /* sensor ir converse to reality */

    return ir_value;
}


static os_size_t ap3216c_fetch_light_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    ap3216c_info_t *ap3216c = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_LIGHT);
    OS_ASSERT(buf);

    ap3216c = (ap3216c_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    data->type       = sensor->info.type;
    data->timestamp  = os_sensor_get_ts();
    data->data.light = ap3216c_read_ambient_light(ap3216c);

    return 0;
}

static os_err_t ap3216c_light_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = 0x12;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static os_size_t ap3216c_fetch_proximity_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    ap3216c_info_t *ap3216c = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_PROXIMITY);
    OS_ASSERT(buf);

    ap3216c = (ap3216c_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    data->type           = sensor->info.type;
    data->timestamp      = os_sensor_get_ts();
    data->data.proximity = ap3216c_read_ps_data(ap3216c);

    return 0;
}

static os_err_t ap3216c_proximity_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = 0x12;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static os_size_t ap3216c_fetch_ir_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    ap3216c_info_t *ap3216c = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_IR);
    OS_ASSERT(buf);

    ap3216c = (ap3216c_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    data->type       = sensor->info.type;
    data->timestamp  = os_sensor_get_ts();
    data->data.light = ap3216c_read_ir_data(ap3216c);

    return 0;
}

static os_err_t ap3216c_ir_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = 0x12;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops ap3216c_light_ops = {ap3216c_fetch_light_data, ap3216c_light_control};

static struct os_sensor_ops ap3216c_proximity_ops = {ap3216c_fetch_proximity_data, ap3216c_proximity_control};

static struct os_sensor_ops ap3216c_ir_ops = {ap3216c_fetch_ir_data, ap3216c_ir_control};

static ap3216c_info_t *ap3216c_init(const char *bus_name, os_uint16_t addr)
{
    ap3216c_info_t *ap3216c = NULL;

    LOG_I(DBG_EXT_TAG, "ap3216c:[%s][0x%02x]", bus_name, addr);

    ap3216c = os_calloc(1, sizeof(ap3216c_info_t));
    if (ap3216c == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "ap3216c amlloc faile");
        return NULL;
    }

    ap3216c->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (ap3216c->i2c.bus == NULL)
    {
        LOG_E(DBG_EXT_TAG, "ap3216c i2c invalid.");
        os_free(ap3216c);
        return NULL;
    }

    ap3216c->i2c.client_addr = addr;

    /* Reset ap3216c */
    reset_sensor(ap3216c);
    os_task_msleep(100);

    ap3216c_set_param(ap3216c, AP3216C_SYSTEM_MODE, AP3216C_MODE_ALS_AND_PS);
    os_task_msleep(100);

#ifdef AP3216C_USING_HW_INT
    /* Init interrupt mode	*/
    ap3216c_int_init(ap3216c);
#endif

    return ap3216c;
}

static int os_hw_ap3216c_light_init(void)
{
    os_int8_t       result;
    ap3216c_info_t *ap3216c = NULL;

    ap3216c = ap3216c_init(OS_AP3216C_I2C_BUS_NAME, OS_AP3216C_I2C_ADDR);

    if (ap3216c == NULL)
    {
        LOG_E(DBG_EXT_TAG, "ap3216c init failed.");
        goto __exit;
    }

    ap3216c->sensor.info.type       = OS_SENSOR_CLASS_LIGHT;
    ap3216c->sensor.info.vendor     = OS_SENSOR_VENDOR_ADI;
    ap3216c->sensor.info.model      = "ap3216c";
    ap3216c->sensor.info.unit       = OS_SENSOR_UNIT_MLUX;
    ap3216c->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    ap3216c->sensor.info.range_max  = 200000;
    ap3216c->sensor.info.range_min  = 0;
    ap3216c->sensor.info.period_min = 300;
    ap3216c->sensor.ops             = &ap3216c_light_ops;

    result = os_hw_sensor_register(&ap3216c->sensor, "ap3216c", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_EXT_TAG, "AP3216C init success");
    return OS_EOK;

__exit:
    if (ap3216c)
        os_free(ap3216c);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_ap3216c_light_init, OS_INIT_SUBLEVEL_LOW);

static int os_hw_ap3216c_proximity_init(void)
{
    os_int8_t       result;
    ap3216c_info_t *ap3216c = NULL;

    ap3216c = ap3216c_init(OS_AP3216C_I2C_BUS_NAME, OS_AP3216C_I2C_ADDR);

    if (ap3216c == NULL)
    {
        LOG_E(DBG_EXT_TAG, "ap3216c init failed.");
        goto __exit;
    }

    ap3216c->sensor.info.type       = OS_SENSOR_CLASS_PROXIMITY;
    ap3216c->sensor.info.vendor     = OS_SENSOR_VENDOR_ADI;
    ap3216c->sensor.info.model      = "ap3216c";
    ap3216c->sensor.info.unit       = OS_SENSOR_UNIT_RAW;
    ap3216c->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    ap3216c->sensor.info.range_max  = 10000;
    ap3216c->sensor.info.range_min  = 0;
    ap3216c->sensor.info.period_min = 300;
    ap3216c->sensor.ops             = &ap3216c_proximity_ops;

    result = os_hw_sensor_register(&ap3216c->sensor, "ap3216c", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_EXT_TAG, "AP3216C init success");
    return OS_EOK;

__exit:
    if (ap3216c)
        os_free(ap3216c);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_ap3216c_proximity_init, OS_INIT_SUBLEVEL_LOW);

static int os_hw_ap3216c_ir_init(void)
{
    os_int8_t       result;
    ap3216c_info_t *ap3216c = NULL;

    ap3216c = ap3216c_init(OS_AP3216C_I2C_BUS_NAME, OS_AP3216C_I2C_ADDR);

    if (ap3216c == NULL)
    {
        LOG_E(DBG_EXT_TAG, "ap3216c init failed.");
        goto __exit;
    }

    ap3216c->sensor.info.type       = OS_SENSOR_CLASS_IR;
    ap3216c->sensor.info.vendor     = OS_SENSOR_VENDOR_ADI;
    ap3216c->sensor.info.model      = "ap3216c";
    ap3216c->sensor.info.unit       = OS_SENSOR_UNIT_RAW;
    ap3216c->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    ap3216c->sensor.info.range_max  = 10000;
    ap3216c->sensor.info.range_min  = 0;
    ap3216c->sensor.info.period_min = 300;
    ap3216c->sensor.ops             = &ap3216c_ir_ops;

    result = os_hw_sensor_register(&ap3216c->sensor, "ap3216c", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_EXT_TAG, "AP3216C init success");
    return OS_EOK;

__exit:
    if (ap3216c)
        os_free(ap3216c);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_ap3216c_ir_init, OS_INIT_SUBLEVEL_LOW);


