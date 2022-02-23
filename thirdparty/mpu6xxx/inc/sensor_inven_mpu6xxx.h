/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-08     flybreak     the first version
 * 2020-03-20     OneOS Team   adapt the code to OneOS
 */

#ifndef SENSOR_INVEN_MPU6XXX_H__
#define SENSOR_INVEN_MPU6XXX_H__

#include "sensors/sensor.h"
#include "mpu6xxx.h"

#define MPU6XXX_ADDRESS_AD0_LOW                 0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6XXX_ADDRESS_AD0_HIGH                0x69 // address pin high (VCC)

#define MPU6XXX_ADDR_DEFAULT  MPU6XXX_ADDRESS_AD0_LOW

int os_hw_mpu6xxx_init(const char *name, struct os_sensor_config *cfg);

#endif
