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
 * @file        lsm6dsl.c
 *
 * @brief       This file provides functions for lsm6dsl.
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

#define DBG_EXT_TAG "sensor.adi.lsm6dsl"
#define DBG_EXT_LVL DBG_EXT_INFO

/* Who am I */

#define LSM6DSL_ACC_GYRO_WHO_AM_I 0x6A

/* Device Register */

#define LSM6DSL_ACC_GYRO_FUNC_CFG_ACCESS 0x01

#define LSM6DSL_ACC_GYRO_SENSOR_SYNC_TIME 0x04
#define LSM6DSL_ACC_GYRO_SENSOR_RES_RATIO 0x05

#define LSM6DSL_ACC_GYRO_FIFO_CTRL1 0x06
#define LSM6DSL_ACC_GYRO_FIFO_CTRL2 0x07
#define LSM6DSL_ACC_GYRO_FIFO_CTRL3 0x08
#define LSM6DSL_ACC_GYRO_FIFO_CTRL4 0x09
#define LSM6DSL_ACC_GYRO_FIFO_CTRL5 0x0A

#define LSM6DSL_ACC_GYRO_DRDY_PULSE_CFG_G 0x0B
#define LSM6DSL_ACC_GYRO_INT1_CTRL        0x0D
#define LSM6DSL_ACC_GYRO_INT2_CTRL        0x0E
#define LSM6DSL_ACC_GYRO_WHO_AM_I_REG     0x0F
#define LSM6DSL_ACC_GYRO_CTRL1_XL         0x10
#define LSM6DSL_ACC_GYRO_CTRL2_G          0x11
#define LSM6DSL_ACC_GYRO_CTRL3_C          0x12
#define LSM6DSL_ACC_GYRO_CTRL4_C          0x13
#define LSM6DSL_ACC_GYRO_CTRL5_C          0x14
#define LSM6DSL_ACC_GYRO_CTRL6_C          0x15
#define LSM6DSL_ACC_GYRO_CTRL7_G          0x16
#define LSM6DSL_ACC_GYRO_CTRL8_XL         0x17
#define LSM6DSL_ACC_GYRO_CTRL9_XL         0x18
#define LSM6DSL_ACC_GYRO_CTRL10_C         0x19

#define LSM6DSL_ACC_GYRO_MASTER_CONFIG 0x1A
#define LSM6DSL_ACC_GYRO_WAKE_UP_SRC   0x1B
#define LSM6DSL_ACC_GYRO_TAP_SRC       0x1C
#define LSM6DSL_ACC_GYRO_D6D_SRC       0x1D
#define LSM6DSL_ACC_GYRO_STATUS_REG    0x1E

#define LSM6DSL_ACC_GYRO_OUT_TEMP_L      0x20
#define LSM6DSL_ACC_GYRO_OUT_TEMP_H      0x21
#define LSM6DSL_ACC_GYRO_OUTX_L_G        0x22
#define LSM6DSL_ACC_GYRO_OUTX_H_G        0x23
#define LSM6DSL_ACC_GYRO_OUTY_L_G        0x24
#define LSM6DSL_ACC_GYRO_OUTY_H_G        0x25
#define LSM6DSL_ACC_GYRO_OUTZ_L_G        0x26
#define LSM6DSL_ACC_GYRO_OUTZ_H_G        0x27
#define LSM6DSL_ACC_GYRO_OUTX_L_XL       0x28
#define LSM6DSL_ACC_GYRO_OUTX_H_XL       0x29
#define LSM6DSL_ACC_GYRO_OUTY_L_XL       0x2A
#define LSM6DSL_ACC_GYRO_OUTY_H_XL       0x2B
#define LSM6DSL_ACC_GYRO_OUTZ_L_XL       0x2C
#define LSM6DSL_ACC_GYRO_OUTZ_H_XL       0x2D
#define LSM6DSL_ACC_GYRO_SENSORHUB1_REG  0x2E
#define LSM6DSL_ACC_GYRO_SENSORHUB2_REG  0x2F
#define LSM6DSL_ACC_GYRO_SENSORHUB3_REG  0x30
#define LSM6DSL_ACC_GYRO_SENSORHUB4_REG  0x31
#define LSM6DSL_ACC_GYRO_SENSORHUB5_REG  0x32
#define LSM6DSL_ACC_GYRO_SENSORHUB6_REG  0x33
#define LSM6DSL_ACC_GYRO_SENSORHUB7_REG  0x34
#define LSM6DSL_ACC_GYRO_SENSORHUB8_REG  0x35
#define LSM6DSL_ACC_GYRO_SENSORHUB9_REG  0x36
#define LSM6DSL_ACC_GYRO_SENSORHUB10_REG 0x37
#define LSM6DSL_ACC_GYRO_SENSORHUB11_REG 0x38
#define LSM6DSL_ACC_GYRO_SENSORHUB12_REG 0x39
#define LSM6DSL_ACC_GYRO_FIFO_STATUS1    0x3A
#define LSM6DSL_ACC_GYRO_FIFO_STATUS2    0x3B
#define LSM6DSL_ACC_GYRO_FIFO_STATUS3    0x3C
#define LSM6DSL_ACC_GYRO_FIFO_STATUS4    0x3D
#define LSM6DSL_ACC_GYRO_FIFO_DATA_OUT_L 0x3E
#define LSM6DSL_ACC_GYRO_FIFO_DATA_OUT_H 0x3F
#define LSM6DSL_ACC_GYRO_TIMESTAMP0_REG  0x40
#define LSM6DSL_ACC_GYRO_TIMESTAMP1_REG  0x41
#define LSM6DSL_ACC_GYRO_TIMESTAMP2_REG  0x42

#define LSM6DSL_ACC_GYRO_TIMESTAMP_L 0x49
#define LSM6DSL_ACC_GYRO_TIMESTAMP_H 0x4A

#define LSM6DSL_ACC_GYRO_STEP_COUNTER_L 0x4B
#define LSM6DSL_ACC_GYRO_STEP_COUNTER_H 0x4C

#define LSM6DSL_ACC_GYRO_SENSORHUB13_REG 0x4D
#define LSM6DSL_ACC_GYRO_SENSORHUB14_REG 0x4E
#define LSM6DSL_ACC_GYRO_SENSORHUB15_REG 0x4F
#define LSM6DSL_ACC_GYRO_SENSORHUB16_REG 0x50
#define LSM6DSL_ACC_GYRO_SENSORHUB17_REG 0x51
#define LSM6DSL_ACC_GYRO_SENSORHUB18_REG 0x52

#define LSM6DSL_ACC_GYRO_FUNC_SRC    0x53
#define LSM6DSL_ACC_GYRO_TAP_CFG1    0x58
#define LSM6DSL_ACC_GYRO_TAP_THS_6D  0x59
#define LSM6DSL_ACC_GYRO_INT_DUR2    0x5A
#define LSM6DSL_ACC_GYRO_WAKE_UP_THS 0x5B
#define LSM6DSL_ACC_GYRO_WAKE_UP_DUR 0x5C
#define LSM6DSL_ACC_GYRO_FREE_FALL   0x5D
#define LSM6DSL_ACC_GYRO_MD1_CFG     0x5E
#define LSM6DSL_ACC_GYRO_MD2_CFG     0x5F

#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_X_L 0x66
#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_X_H 0x67
#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_Y_L 0x68
#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_Y_H 0x69
#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_Z_L 0x6A
#define LSM6DSL_ACC_GYRO_OUT_MAG_RAW_Z_H 0x6B

#define LSM6DSL_ACC_GYRO_X_OFS_USR 0x73
#define LSM6DSL_ACC_GYRO_Y_OFS_USR 0x74
#define LSM6DSL_ACC_GYRO_Z_OFS_USR 0x75

/* Embedded functions register mapping */
#define LSM6DSL_ACC_GYRO_SLV0_ADD                    0x02
#define LSM6DSL_ACC_GYRO_SLV0_SUBADD                 0x03
#define LSM6DSL_ACC_GYRO_SLAVE0_CONFIG               0x04
#define LSM6DSL_ACC_GYRO_SLV1_ADD                    0x05
#define LSM6DSL_ACC_GYRO_SLV1_SUBADD                 0x06
#define LSM6DSL_ACC_GYRO_SLAVE1_CONFIG               0x07
#define LSM6DSL_ACC_GYRO_SLV2_ADD                    0x08
#define LSM6DSL_ACC_GYRO_SLV2_SUBADD                 0x09
#define LSM6DSL_ACC_GYRO_SLAVE2_CONFIG               0x0A
#define LSM6DSL_ACC_GYRO_SLV3_ADD                    0x0B
#define LSM6DSL_ACC_GYRO_SLV3_SUBADD                 0x0C
#define LSM6DSL_ACC_GYRO_SLAVE3_CONFIG               0x0D
#define LSM6DSL_ACC_GYRO_DATAWRITE_SRC_MODE_SUB_SLV0 0x0E
#define LSM6DSL_ACC_GYRO_CONFIG_PEDO_THS_MIN         0x0F

#define LSM6DSL_ACC_GYRO_SM_STEP_THS      0x13
#define LSM6DSL_ACC_GYRO_PEDO_DEB_REG     0x14
#define LSM6DSL_ACC_GYRO_STEP_COUNT_DELTA 0x15

#define LSM6DSL_ACC_GYRO_MAG_SI_XX  0x24
#define LSM6DSL_ACC_GYRO_MAG_SI_XY  0x25
#define LSM6DSL_ACC_GYRO_MAG_SI_XZ  0x26
#define LSM6DSL_ACC_GYRO_MAG_SI_YX  0x27
#define LSM6DSL_ACC_GYRO_MAG_SI_YY  0x28
#define LSM6DSL_ACC_GYRO_MAG_SI_YZ  0x29
#define LSM6DSL_ACC_GYRO_MAG_SI_ZX  0x2A
#define LSM6DSL_ACC_GYRO_MAG_SI_ZY  0x2B
#define LSM6DSL_ACC_GYRO_MAG_SI_ZZ  0x2C
#define LSM6DSL_ACC_GYRO_MAG_OFFX_L 0x2D
#define LSM6DSL_ACC_GYRO_MAG_OFFX_H 0x2E
#define LSM6DSL_ACC_GYRO_MAG_OFFY_L 0x2F
#define LSM6DSL_ACC_GYRO_MAG_OFFY_H 0x30
#define LSM6DSL_ACC_GYRO_MAG_OFFZ_L 0x31
#define LSM6DSL_ACC_GYRO_MAG_OFFZ_H 0x32

/* Accelero Full_ScaleSelection */
#define LSM6DSL_ACC_FULLSCALE_2G  ((uint8_t)0x00) /*!< ? g */
#define LSM6DSL_ACC_FULLSCALE_4G  ((uint8_t)0x08) /*!< ? g */
#define LSM6DSL_ACC_FULLSCALE_8G  ((uint8_t)0x0C) /*!< ? g */
#define LSM6DSL_ACC_FULLSCALE_16G ((uint8_t)0x04) /*!< ?6 g */

/* Accelero Full Scale Sensitivity */
#define LSM6DSL_ACC_SENSITIVITY_2G     ((float)0.061f)  /*!< accelerometer sensitivity with 2 g full scale */
#define LSM6DSL_ACC_SENSITIVITY_4G     ((float)0.122f)  /*!< accelerometer sensitivity with 4 g full scale */
#define LSM6DSL_ACC_SENSITIVITY_8G     ((float)0.244f)  /*!< accelerometer sensitivity with 8 g full scale */
#define LSM6DSL_ACC_SENSITIVITY_16G    ((float)0.488f)  /*!< accelerometer sensitivity with 12 g full scale */

/* Accelero Power Mode selection */
#define LSM6DSL_ACC_GYRO_LP_XL_DISABLED ((uint8_t)0x00) /* LP disabled*/
#define LSM6DSL_ACC_GYRO_LP_XL_ENABLED  ((uint8_t)0x10) /* LP enabled*/

/* Output Data Rate */
#define LSM6DSL_ODR_BITPOSITION ((uint8_t)0xF0) /*!< Output Data Rate bit position */
#define LSM6DSL_ODR_POWER_DOWN  ((uint8_t)0x00) /* Power Down mode       */
#define LSM6DSL_ODR_13Hz        ((uint8_t)0x10) /* Low Power mode        */
#define LSM6DSL_ODR_26Hz        ((uint8_t)0x20) /* Low Power mode        */
#define LSM6DSL_ODR_52Hz        ((uint8_t)0x30) /* Low Power mode        */
#define LSM6DSL_ODR_104Hz       ((uint8_t)0x40) /* Normal mode           */
#define LSM6DSL_ODR_208Hz       ((uint8_t)0x50) /* Normal mode           */
#define LSM6DSL_ODR_416Hz       ((uint8_t)0x60) /* High Performance mode */
#define LSM6DSL_ODR_833Hz       ((uint8_t)0x70) /* High Performance mode */
#define LSM6DSL_ODR_1660Hz      ((uint8_t)0x80) /* High Performance mode */
#define LSM6DSL_ODR_3330Hz      ((uint8_t)0x90) /* High Performance mode */
#define LSM6DSL_ODR_6660Hz      ((uint8_t)0xA0) /* High Performance mode */

/* Gyro Full Scale Selection */
#define LSM6DSL_GYRO_FS_245  ((uint8_t)0x00)
#define LSM6DSL_GYRO_FS_500  ((uint8_t)0x04)
#define LSM6DSL_GYRO_FS_1000 ((uint8_t)0x08)
#define LSM6DSL_GYRO_FS_2000 ((uint8_t)0x0C)

/* Gyro Full Scale Sensitivity */
#define LSM6DSL_GYRO_SENSITIVITY_245DPS  ((float)8.750f) /* Sensitivity value for 245 dps full scale  [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_500DPS  ((float)17.50f) /* Sensitivity value for 500 dps full scale  [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_1000DPS ((float)35.00f) /* Sensitivity value for 1000 dps full scale [mdps/LSB] */
#define LSM6DSL_GYRO_SENSITIVITY_2000DPS ((float)70.00f) /* Sensitivity value for 2000 dps full scale [mdps/LSB] */

/* Gyro Power Mode selection */
#define LSM6DSL_ACC_GYRO_LP_G_DISABLED ((uint8_t)0x00) /* LP disabled*/
#define LSM6DSL_ACC_GYRO_LP_G_ENABLED  ((uint8_t)0x80) /* LP enabled*/

/* Block Data Update */
#define LSM6DSL_BDU_CONTINUOS    ((uint8_t)0x00)
#define LSM6DSL_BDU_BLOCK_UPDATE ((uint8_t)0x40)

/* Auto-increment */
#define LSM6DSL_ACC_GYRO_IF_INC_DISABLED ((uint8_t)0x00)
#define LSM6DSL_ACC_GYRO_IF_INC_ENABLED  ((uint8_t)0x04)

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    union
    {
        struct
        {
            short x;
            short y;
            short z;
        } acce;

        struct
        {
            short x;
            short y;
            short z;
        } gyro;
    } value;
} lsm6dsl_info_t;

static os_err_t lsm6dsl_write(lsm6dsl_info_t *lsm6dsl, uint8_t Reg, uint8_t Value)
{
    return os_i2c_client_write_byte(&lsm6dsl->i2c, Reg, 1, Value);
}

static uint8_t lsm6dsl_read(lsm6dsl_info_t *lsm6dsl, uint8_t Reg)
{
    return os_i2c_client_read_byte(&lsm6dsl->i2c, Reg, 1);
}

static os_err_t lsm6dsl_read_buff(lsm6dsl_info_t *lsm6dsl, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
    return os_i2c_client_read(&lsm6dsl->i2c, Reg, 1, Buffer, Length);
}

void lsm6dsl_acc_low_power(lsm6dsl_info_t *lsm6dsl, uint16_t status)
{
    uint8_t ctrl = 0x00;

    /* Read CTRL6_C value */
    ctrl = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL6_C);

    /* Clear Low Power Mode bit */
    ctrl &= ~(0x10);

    /* Set Low Power Mode */
    if (status)
    {
        ctrl |= LSM6DSL_ACC_GYRO_LP_XL_ENABLED;
    }
    else
    {
        ctrl |= LSM6DSL_ACC_GYRO_LP_XL_DISABLED;
    }

    /* Write back control register */
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL6_C, ctrl);
}

void lsm6dsl_acc_init(lsm6dsl_info_t *lsm6dsl, uint16_t InitStruct)
{
    uint8_t ctrl = 0x00;
    uint8_t tmp;

    /* Read CTRL1_XL */
    tmp = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL);

    /* Write value to ACC MEMS CTRL1_XL register: FS and Data Rate */
    ctrl = (uint8_t)InitStruct;
    tmp &= ~(0xFC);
    tmp |= ctrl;
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL, tmp);

    /* Read CTRL3_C */
    tmp = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL3_C);

    /* Write value to ACC MEMS CTRL3_C register: BDU and Auto-increment */
    ctrl = ((uint8_t)(InitStruct >> 8));
    tmp &= ~(0x44);
    tmp |= ctrl;
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}

void lsm6dsl_acc_deinit(lsm6dsl_info_t *lsm6dsl)
{
    uint8_t ctrl = 0x00;

    /* Read control register 1 value */
    ctrl = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL);

    /* Clear ODR bits */
    ctrl &= ~(LSM6DSL_ODR_BITPOSITION);

    /* Set Power down */
    ctrl |= LSM6DSL_ODR_POWER_DOWN;

    /* write back control register */
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL, ctrl);
}

uint8_t lsm6dsl_acc_read_id(lsm6dsl_info_t *lsm6dsl)
{
    /* Read value at Who am I register address */
    return lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);
}

void lsm6dsl_acc_get_value(lsm6dsl_info_t *lsm6dsl, int16_t *pData)
{
    int16_t pnRawData[3];
    uint8_t ctrlx = 0;
    uint8_t buffer[6];
    uint8_t i           = 0;
    float   sensitivity = 0;

    volatile uint8_t read_bak = 0;

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G, 0X1C);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL10_C, 0x38);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL, 0x60);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_TAP_CFG1, 0x90);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_WAKE_UP_DUR, 0x00);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_WAKE_UP_THS, 0x02);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_MD1_CFG, 0x20);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    /* 6D Orientation Configuration */
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_TAP_THS_6D, 0x40);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL8_XL, 0x01);
    read_bak = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    uint8_t status = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_STATUS_REG);

    /* Read the acceleration control register content */
    ctrlx = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL1_XL);

    /* Read output register X, Y & Z acceleration */
    lsm6dsl_read_buff(lsm6dsl, LSM6DSL_ACC_GYRO_OUTX_L_XL, buffer, 6);

    for (i = 0; i < 3; i++)
    {
        pnRawData[i] = ((((uint16_t)buffer[2 * i + 1]) << 8) + (uint16_t)buffer[2 * i]);
    }

    /* Switch the sensitivity value set in the CRTL1_XL */
    switch (ctrlx & 0x0C)
    {
    case LSM6DSL_ACC_FULLSCALE_2G:
        sensitivity = LSM6DSL_ACC_SENSITIVITY_2G;
        break;
    case LSM6DSL_ACC_FULLSCALE_4G:
        sensitivity = LSM6DSL_ACC_SENSITIVITY_4G;
        break;
    case LSM6DSL_ACC_FULLSCALE_8G:
        sensitivity = LSM6DSL_ACC_SENSITIVITY_8G;
        break;
    case LSM6DSL_ACC_FULLSCALE_16G:
        sensitivity = LSM6DSL_ACC_SENSITIVITY_16G;
        break;
    }

    /* Obtain the mg value for the three axis */
    for (i = 0; i < 3; i++)
    {
        pData[i] = (int16_t)(pnRawData[i] * sensitivity);
    }
}

void lsm6dsl_gyro_low_power(lsm6dsl_info_t *lsm6dsl, uint16_t status)
{
    uint8_t ctrl = 0x00;

    /* Read CTRL7_G value */
    ctrl = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL7_G);

    /* Clear Low Power Mode bit */
    ctrl &= ~(0x80);

    /* Set Low Power Mode */
    if (status)
    {
        ctrl |= LSM6DSL_ACC_GYRO_LP_G_ENABLED;
    }
    else
    {
        ctrl |= LSM6DSL_ACC_GYRO_LP_G_DISABLED;
    }

    /* Write back control register */
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL7_G, ctrl);
}

void lsm6dsl_gyro_init(lsm6dsl_info_t *lsm6dsl, uint16_t InitStruct)
{
    uint8_t ctrl = 0x00;
    uint8_t tmp;

    /* Read CTRL2_G */
    tmp = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    /* Write value to GYRO MEMS CTRL2_G register: FS and Data Rate */
    ctrl = (uint8_t)InitStruct;
    tmp &= ~(0xFC);
    tmp |= ctrl;
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G, tmp);

    /* Read CTRL3_C */
    tmp = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL3_C);

    /* Write value to GYRO MEMS CTRL3_C register: BDU and Auto-increment */
    ctrl = ((uint8_t)(InitStruct >> 8));
    tmp &= ~(0x44);
    tmp |= ctrl;
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}

void lsm6dsl_gyro_deinit(lsm6dsl_info_t *lsm6dsl)
{
    uint8_t ctrl = 0x00;

    /* Read control register 1 value */
    ctrl = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    /* Clear ODR bits */
    ctrl &= ~(LSM6DSL_ODR_BITPOSITION);

    /* Set Power down */
    ctrl |= LSM6DSL_ODR_POWER_DOWN;

    /* Write back control register */
    lsm6dsl_write(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G, ctrl);
}

uint8_t lsm6dsl_gyro_read_id(lsm6dsl_info_t *lsm6dsl)
{
    /* IO interface initialization */
    /* Read value at Who am I register address */
    return lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);
}

void lsm6dsl_gyro_get_value(lsm6dsl_info_t *lsm6dsl, float *pfData)
{
    int16_t pnRawData[3];
    uint8_t ctrlg = 0;
    uint8_t buffer[6];
    uint8_t i           = 0;
    float   sensitivity = 0;

    /* Read the gyro control register content */
    ctrlg = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_CTRL2_G);

    /* Read output register X, Y & Z acceleration */
    lsm6dsl_read_buff(lsm6dsl, LSM6DSL_ACC_GYRO_OUTX_L_G, buffer, 6);

    for (i = 0; i < 3; i++)
    {
        pnRawData[i] = ((((uint16_t)buffer[2 * i + 1]) << 8) + (uint16_t)buffer[2 * i]);
    }

    /* Normal mode */
    /* Switch the sensitivity value set in the CRTL2_G */
    switch (ctrlg & 0x0C)
    {
    case LSM6DSL_GYRO_FS_245:
        sensitivity = LSM6DSL_GYRO_SENSITIVITY_245DPS;
        break;
    case LSM6DSL_GYRO_FS_500:
        sensitivity = LSM6DSL_GYRO_SENSITIVITY_500DPS;
        break;
    case LSM6DSL_GYRO_FS_1000:
        sensitivity = LSM6DSL_GYRO_SENSITIVITY_1000DPS;
        break;
    case LSM6DSL_GYRO_FS_2000:
        sensitivity = LSM6DSL_GYRO_SENSITIVITY_2000DPS;
        break;
    }

    /* Obtain the mg value for the three axis */
    for (i = 0; i < 3; i++)
    {
        pfData[i] = (float)(pnRawData[i] * sensitivity);
    }
}

static lsm6dsl_info_t *lsm6dsl_sensor_init(const char *bus_name, os_uint16_t addr, int acc_or_gyro)
{
    unsigned char   devid   = 0;
    lsm6dsl_info_t *lsm6dsl = NULL;

    LOG_I(DBG_EXT_TAG,"lsm6dsl:[%s][0x%02x]", bus_name, addr);

    lsm6dsl = os_calloc(1, sizeof(lsm6dsl_info_t));
    if (lsm6dsl == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"lsm6dsl amlloc faile");
        return NULL;
    }

    lsm6dsl->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (lsm6dsl->i2c.bus == NULL)
    {
        LOG_E(DBG_EXT_TAG,"lsm6dsl i2c invalid.");
        os_free(lsm6dsl);
        return NULL;
    }

    lsm6dsl->i2c.client_addr = OS_LSM6DSL_I2C_ADDR;

    devid = lsm6dsl_read(lsm6dsl, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);

    LOG_I(DBG_EXT_TAG,"lsm6dsl devid:0x%02x", devid);
    if (devid != LSM6DSL_ACC_GYRO_WHO_AM_I)
    {
        LOG_E(DBG_EXT_TAG,"lsm6dsl devid invalid.");
        os_free(lsm6dsl);
        return NULL;
    }

    lsm6dsl->id = devid;

    uint16_t InitStruct;

    if (acc_or_gyro)
    {
        InitStruct = LSM6DSL_ODR_52Hz 
                   | LSM6DSL_ACC_FULLSCALE_2G 
                   | (LSM6DSL_BDU_BLOCK_UPDATE << 8)
                   | (LSM6DSL_ACC_GYRO_IF_INC_ENABLED << 8);
        lsm6dsl_acc_init(lsm6dsl, InitStruct);
    }
    else
    {
        InitStruct = LSM6DSL_ODR_52Hz 
                   | LSM6DSL_GYRO_FS_2000 
                   | (LSM6DSL_BDU_BLOCK_UPDATE << 8)
                   | (LSM6DSL_ACC_GYRO_IF_INC_ENABLED << 8);
        lsm6dsl_gyro_init(lsm6dsl, InitStruct);
    }

    return lsm6dsl;
}

static uint8_t lsm6dsl_acce_sensor_get_value(lsm6dsl_info_t *lsm6dsl)
{
    int16_t pData[3];

    lsm6dsl_acc_get_value(lsm6dsl, pData);

    lsm6dsl->value.acce.x = pData[0];
    lsm6dsl->value.acce.y = pData[1];
    lsm6dsl->value.acce.z = pData[2];

    LOG_D(DBG_EXT_TAG, "(%d, %d, %d).", (int)lsm6dsl->value.acce.x, (int)lsm6dsl->value.acce.y, (int)lsm6dsl->value.acce.z);

    return 0;
}

static os_size_t lsm6dsl_acce_sensor_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    lsm6dsl_info_t *lsm6dsl = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_ACCE);
    OS_ASSERT(buf);

    lsm6dsl = (lsm6dsl_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    lsm6dsl_acce_sensor_get_value(lsm6dsl);

    data->type        = sensor->info.type;
    data->data.acce.x = lsm6dsl->value.acce.x;
    data->data.acce.y = lsm6dsl->value.acce.y;
    data->data.acce.z = lsm6dsl->value.acce.z;
    data->timestamp   = os_sensor_get_ts();

    return 0;
}

static os_err_t lsm6dsl_acce_sensor_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    lsm6dsl_info_t *lsm6dsl = (lsm6dsl_info_t *)sensor;
    os_err_t        result  = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = lsm6dsl->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops lsm6dsl_acce_ops =
{
    lsm6dsl_acce_sensor_fetch_data,
    lsm6dsl_acce_sensor_control
};

static int os_hw_lsm6dsl_acce_init(void)
{
    os_int8_t       result;
    lsm6dsl_info_t *lsm6dsl = NULL;

    lsm6dsl = lsm6dsl_sensor_init(OS_LSM6DSL_I2C_BUS_NAME, OS_LSM6DSL_I2C_ADDR, 1);
    if (lsm6dsl == NULL)
    {
        LOG_E(DBG_EXT_TAG,"lsm6dsl init failed.");
        goto __exit;
    }

    lsm6dsl->sensor.info.type       = OS_SENSOR_CLASS_ACCE;
    lsm6dsl->sensor.info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
    lsm6dsl->sensor.info.model      = "lsm6dsl";
    lsm6dsl->sensor.info.unit       = OS_SENSOR_UNIT_MG;
    lsm6dsl->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    lsm6dsl->sensor.info.range_max  = 16000;
    lsm6dsl->sensor.info.range_min  = -16000;
    lsm6dsl->sensor.info.period_min = 5;
    lsm6dsl->sensor.ops             = &lsm6dsl_acce_ops;

    result = os_hw_sensor_register(&lsm6dsl->sensor, "lsm6dsl", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_EXT_TAG,"lsm6dsl init success");
    return OS_EOK;

__exit:
    if (lsm6dsl)
    {
        os_free(lsm6dsl);
    }
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_lsm6dsl_acce_init, OS_INIT_SUBLEVEL_LOW);

static uint8_t lsm6dsl_gyro_sensor_get_value(lsm6dsl_info_t *lsm6dsl)
{
    float pData[3];

    lsm6dsl_gyro_get_value(lsm6dsl, pData);

    lsm6dsl->value.gyro.x = pData[0];
    lsm6dsl->value.gyro.y = pData[1];
    lsm6dsl->value.gyro.z = pData[2];

    LOG_D(DBG_EXT_TAG, "(%d, %d, %d).", (int)lsm6dsl->value.gyro.x, (int)lsm6dsl->value.gyro.y, (int)lsm6dsl->value.gyro.z);

    return 0;
}

static os_size_t lsm6dsl_gyro_sensor_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    lsm6dsl_info_t *lsm6dsl = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_GYRO);
    OS_ASSERT(buf);

    lsm6dsl = (lsm6dsl_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    lsm6dsl_gyro_sensor_get_value(lsm6dsl);

    data->type        = sensor->info.type;
    data->data.acce.x = lsm6dsl->value.gyro.x;
    data->data.acce.y = lsm6dsl->value.gyro.y;
    data->data.acce.z = lsm6dsl->value.gyro.z;
    data->timestamp   = os_sensor_get_ts();

    return 0;
}

static os_err_t lsm6dsl_gyro_sensor_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    lsm6dsl_info_t *lsm6dsl = (lsm6dsl_info_t *)sensor;
    os_err_t        result  = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = lsm6dsl->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops lsm6dsl_gyro_ops =
{
    lsm6dsl_gyro_sensor_fetch_data,
    lsm6dsl_gyro_sensor_control
};

static int os_hw_lsm6dsl_gyro_init(void)
{
    os_int8_t       result;
    lsm6dsl_info_t *lsm6dsl = NULL;

    lsm6dsl = lsm6dsl_sensor_init(OS_LSM6DSL_I2C_BUS_NAME, OS_LSM6DSL_I2C_ADDR, 0);
    if (lsm6dsl == NULL)
    {
        LOG_E(DBG_EXT_TAG,"lsm6dsl init failed.");
        goto __exit;
    }

    lsm6dsl->sensor.info.type       = OS_SENSOR_CLASS_GYRO;
    lsm6dsl->sensor.info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
    lsm6dsl->sensor.info.model      = "lsm6dsl";
    lsm6dsl->sensor.info.unit       = OS_SENSOR_UNIT_MDPS;
    lsm6dsl->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    lsm6dsl->sensor.info.range_max  = 16000;
    lsm6dsl->sensor.info.range_min  = -16000;
    lsm6dsl->sensor.info.period_min = 5;
    lsm6dsl->sensor.ops             = &lsm6dsl_gyro_ops;

    result = os_hw_sensor_register(&lsm6dsl->sensor, "lsm6dsl", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_EXT_TAG,"lsm6dsl init success");
    return OS_EOK;

__exit:
    if (lsm6dsl)
    {
        os_free(lsm6dsl);
    }
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_lsm6dsl_gyro_init, OS_INIT_SUBLEVEL_LOW);
