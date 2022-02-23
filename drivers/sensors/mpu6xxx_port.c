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
 * @file        mpu6xxx_port.c
 *
 * @brief       This file provides port for mpu6xxx.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stddef.h>
#include <sensor_inven_mpu6xxx.h>

static int os_hw_mpu6xxx_port(void)
{
    const char *name = NULL;
    struct os_sensor_config cfg;

    cfg.irq_pin.pin    = OS_PIN_NONE;
    cfg.intf.dev_name  = OS_MPU6XXX_BUS_NAME;
    
    #if defined(OS_MPU6XXX_ADDR) 
    cfg.intf.user_data = (void *)OS_MPU6XXX_ADDR;
    #endif

#ifdef OS_USING_ICM20602
    name = "icm20602";
#elif defined(OS_USING_ICM20608)
    name = "icm20608";
#elif defined(OS_USING_MPU9250)
    name = "mpu9250";
#elif defined(OS_USING_MPU6050)
    name = "mpu6050";
#else
#error :unknown mpu6xxx device
#endif

    OS_ASSERT(name);

    return os_hw_mpu6xxx_init(name, &cfg);
}

OS_ENV_INIT(os_hw_mpu6xxx_port, OS_INIT_SUBLEVEL_HIGH);
