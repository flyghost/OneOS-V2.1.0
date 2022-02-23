/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-23     flybreak     the first version
 * 2020-03-20     OneOS Team   adapt the code to OneOS
 */

#include <os_task.h>
#include <os_stddef.h>
#include <shell.h>
#include <os_types.h>
#include "mpu6xxx.h"

/* Default configuration, please change according to the actual situation, support i2c and spi device name */
#define MPU6XXX_DEVICE_NAME  "i2c1"

/* Test function */
static int mpu6xxx_test()
{
    struct mpu6xxx_device *dev;
    struct mpu6xxx_3axes accel, gyro;
    int i;

    /* Initialize mpu6xxx, The parameter is OS_NULL, means auto probing for i2c*/
    dev = mpu6xxx_init(MPU6XXX_DEVICE_NAME,(os_uint8_t)OS_NULL);

    if (dev == OS_NULL)
    {
        os_kprintf("mpu6xxx init failed\n");
        return -1;
    }
    os_kprintf("mpu6xxx init succeed\n");

    for (i = 0; i < 5; i++)
    {
        mpu6xxx_get_accel(dev, &accel);
        mpu6xxx_get_gyro(dev, &gyro);

        os_kprintf("accel.x = %3d, accel.y = %3d, accel.z = %3d ", accel.x, accel.y, accel.z);
        os_kprintf("gyro.x = %3d gyro.y = %3d, gyro.z = %3d\n", gyro.x, gyro.y, gyro.z);

        os_task_mdelay(100);
    }

    mpu6xxx_deinit(dev);

    return 0;
}
SH_CMD_EXPORT(mpu6xxx_test, mpu6xxx_test, "mpu6xxx sensor test function");

