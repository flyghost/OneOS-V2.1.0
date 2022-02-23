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
 * @brief       This file implements i2c driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

#include "am_mcu_apollo.h"

/* I2C0 */
#define AM_I2C0_IOM_INST 0

/* I2C2 */
#define AM_I2C2_IOM_INST 2

/* I2C3 */
#define AM_I2C3_IOM_INST 3

/* I2C4 */
#define AM_I2C4_IOM_INST 4

static am_hal_iom_config_t g_sIOMConfig =
{
    AM_HAL_IOM_I2CMODE, /* ui32InterfaceMode */
    AM_HAL_IOM_100KHZ,  /* ui32ClockFrequency */
    0,                  /* bSPHA */
    0,                  /* bSPOL */
    4,                  /* ui8WriteThreshold */
    60,                 /* ui8ReadThreshold */
};

/* AM i2c driver */
struct am_i2c_bus
{
    struct os_i2c_bus_device parent;
    os_uint32_t              u32Module;
};

os_size_t os_i2c_master_xfer(struct os_i2c_bus_device *bus, struct os_i2c_msg *msgs, os_uint32_t num)
{
    struct am_i2c_bus *am_i2c_bus = (struct am_i2c_bus *)bus;
    struct os_i2c_msg *msg;
    int                i;
    os_uint32_t        msg_len = 0;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        if (msg->flags == OS_I2C_RD)
        {
            am_hal_iom_i2c_read(am_i2c_bus->u32Module, msg->addr, (uint32_t *)msg->buf, msg->len, AM_HAL_IOM_RAW);
            msg_len += msg->len;
        }
        else if (msg->flags == OS_I2C_WR)
        {
            am_hal_iom_i2c_write(am_i2c_bus->u32Module, msg->addr, (uint32_t *)msg->buf, msg->len, AM_HAL_IOM_RAW);
            msg_len += (msg->len - 1);
        }
    }

    return msg_len;
}

os_err_t os_i2c_bus_control(struct os_i2c_bus_device *bus, os_uint32_t cmd, os_uint32_t arg)
{
    struct am_i2c_bus *am_i2c_bus = (struct am_i2c_bus *)bus;
    /* os_uint32_t ctrl_arg = (os_uint32_t)(arg); */

    OS_ASSERT(bus != OS_NULL);
    am_i2c_bus = (struct am_i2c_bus *)bus->parent.user_data;

    OS_ASSERT(am_i2c_bus != OS_NULL);

    switch (cmd)
    {
    /* I2C config */
    case OS_DEVICE_CTRL_CONFIG:
        break;
    }

    return OS_EOK;
}

static const struct os_i2c_bus_device_ops am_i2c_ops =
{
    os_i2c_master_xfer,
    OS_NULL,
    os_i2c_bus_control
};

#ifdef BSP_USING_I2CBB
static struct am_i2c_bus am_i2c_bus_bb = 
{
    {0},
    AM_HAL_IOM_I2CBB_MODULE
};
#endif

#ifdef BSP_USING_I2C0
static struct am_i2c_bus am_i2c_bus_0 = 
{
    {0},
    AM_I2C0_IOM_INST
};
#endif

#ifdef BSP_USING_I2C1
static struct am_i2c_bus am_i2c_bus_1 = 
{
    {1},
    AM_I2C1_IOM_INST
};
#endif

#ifdef BSP_USING_I2C2
static struct am_i2c_bus am_i2c_bus_2 = 
{
    {2},
    AM_I2C2_IOM_INST
};
#endif

#ifdef BSP_USING_I2C3
static struct am_i2c_bus am_i2c_bus_3 = 
{
    {3},
    AM_I2C3_IOM_INST
};
#endif

#ifdef BSP_USING_I2C4
static struct am_i2c_bus am_i2c_bus_4 = 
{
    {4},
    AM_I2C4_IOM_INST
};
#endif

int os_hw_i2c_init(void)
{
    struct am_i2c_bus *am_i2c;

#ifdef BSP_USING_I2CBB
    am_hal_i2c_bit_bang_init(I2CBB_GPIO_SCL, I2CBB_GPIO_SDA);

    /* init i2c bus device */
    am_i2c             = &am_i2c_bus_bb;
    am_i2c->parent.ops = &am_i2c_ops;
    os_i2c_bus_device_register(&am_i2c->parent, "i2cbb");
#endif

#ifdef BSP_USING_I2C0
    /* init i2c gpio */
    am_hal_gpio_pin_config(I2C0_GPIO_SCL, I2C0_GPIO_CFG_SCK | AM_HAL_GPIO_PULL6K);
    am_hal_gpio_pin_config(I2C0_GPIO_SDA, I2C0_GPIO_CFG_SDA | AM_HAL_GPIO_PULL6K);

    /* Initialize IOM 0 in I2C mode at 100KHz */
    am_hal_iom_pwrctrl_enable(AM_I2C0_IOM_INST);
    g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_100KHZ;
    am_hal_iom_config(AM_I2C0_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_I2C0_IOM_INST);

    /* init i2c bus device */
    am_i2c             = &am_i2c_bus_0;
    am_i2c->parent.ops = &am_i2c_ops;
    os_i2c_bus_device_register(&am_i2c->parent, "i2c0");
#endif

#ifdef BSP_USING_I2C2
    /* init i2c gpio */
    am_hal_gpio_pin_config(I2C2_GPIO_SCL, I2C2_GPIO_CFG_SCK | AM_HAL_GPIO_PULL6K);
    am_hal_gpio_pin_config(I2C2_GPIO_SDA, I2C2_GPIO_CFG_SDA | AM_HAL_GPIO_PULL6K);

    /* Initialize IOM 2 in I2C mode at 400KHz */
    am_hal_iom_pwrctrl_enable(AM_I2C2_IOM_INST);
    g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_400KHZ;
    am_hal_iom_config(AM_I2C2_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_I2C2_IOM_INST);

    /* init i2c bus device */
    am_i2c             = &am_i2c_bus_2;
    am_i2c->parent.ops = &am_i2c_ops;
    os_i2c_bus_device_register(&am_i2c->parent, "i2c2");
#endif

#ifdef BSP_USING_I2C3
    /* init i2c gpio */
    am_hal_gpio_pin_config(I2C3_GPIO_SCL, I2C3_GPIO_CFG_SCK | AM_HAL_GPIO_PULL6K);
    am_hal_gpio_pin_config(I2C3_GPIO_SDA, I2C3_GPIO_CFG_SDA | AM_HAL_GPIO_PULL6K);

    /* Initialize IOM 3 in I2C mode at 400KHz */
    am_hal_iom_pwrctrl_enable(AM_I2C3_IOM_INST);
    g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_400KHZ;
    am_hal_iom_config(AM_I2C3_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_I2C3_IOM_INST);

    /* init i2c bus device */
    am_i2c             = &am_i2c_bus_3;
    am_i2c->parent.ops = &am_i2c_ops;
    os_i2c_bus_device_register(&am_i2c->parent, "i2c3");
#endif

#ifdef BSP_USING_I2C4
    /* init i2c gpio */
    am_hal_gpio_pin_config(I2C4_GPIO_SCL, I2C4_GPIO_CFG_SCK | AM_HAL_GPIO_PULL6K);
    am_hal_gpio_pin_config(I2C4_GPIO_SDA, I2C4_GPIO_CFG_SDA | AM_HAL_GPIO_PULL6K);

    /* Initialize IOM 4 in I2C mode at 400KHz */
    am_hal_iom_pwrctrl_enable(AM_I2C4_IOM_INST);
    g_sIOMConfig.ui32ClockFrequency = AM_HAL_IOM_400KHZ;
    am_hal_iom_config(AM_I2C4_IOM_INST, &g_sIOMConfig);
    am_hal_iom_enable(AM_I2C4_IOM_INST);

    /* init i2c bus device */
    am_i2c             = &am_i2c_bus_4;
    am_i2c->parent.ops = &am_i2c_ops;
    os_i2c_bus_device_register(&am_i2c->parent, "i2c4");
#endif

    return 0;
}
