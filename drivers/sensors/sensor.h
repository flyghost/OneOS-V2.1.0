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
 * @file        sensor.h
 *
 * @brief       This file provides struct/enum definition and sensor functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <os_task.h>
#include <os_mutex.h>
#include <os_clock.h>
#include <drv_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_RTC
#define os_sensor_get_ts() time(OS_NULL) /* API for the sensor to get the timestamp */
#else
#define os_sensor_get_ts() os_tick_get() /* API for the sensor to get the timestamp */
#endif

#define OS_PIN_NONE            0xFFFF /* PIN NONE */
#define OS_DEVICE_FLAG_FIFO_RX 0x200  /* Flag to use when the sensor is open by fifo mode */

#define OS_SENSOR_MODULE_MAX (3) /* The maximum number of members of a sensor module */

/* Sensor types */
#define OS_SENSOR_CLASS_NONE      (0)
#define OS_SENSOR_CLASS_ACCE      (1)  /* Accelerometer     */
#define OS_SENSOR_CLASS_GYRO      (2)  /* Gyroscope         */
#define OS_SENSOR_CLASS_MAG       (3)  /* Magnetometer      */
#define OS_SENSOR_CLASS_TEMP      (4)  /* Temperature       */
#define OS_SENSOR_CLASS_HUMI      (5)  /* Relative Humidity */
#define OS_SENSOR_CLASS_BARO      (6)  /* Barometer         */
#define OS_SENSOR_CLASS_LIGHT     (7)  /* Ambient light     */
#define OS_SENSOR_CLASS_PROXIMITY (8)  /* Proximity         */
#define OS_SENSOR_CLASS_HR        (9)  /* Heart Rate        */
#define OS_SENSOR_CLASS_TVOC      (10) /* TVOC Level        */
#define OS_SENSOR_CLASS_NOISE     (11) /* Noise Loudness    */
#define OS_SENSOR_CLASS_STEP      (12) /* Step sensor       */
#define OS_SENSOR_CLASS_FORCE     (13) /* Force sensor      */
#define OS_SENSOR_CLASS_ALTI      (14) /* Altitude sensor   */
#define OS_SENSOR_CLASS_IR        (15) /* IR intensity sensor   */


/* Sensor vendor types */
#define OS_SENSOR_VENDOR_UNKNOWN    (0)
#define OS_SENSOR_VENDOR_STM        (1) /* STMicroelectronics */
#define OS_SENSOR_VENDOR_BOSCH      (2) /* Bosch */
#define OS_SENSOR_VENDOR_INVENSENSE (3) /* Invensense */
#define OS_SENSOR_VENDOR_SEMTECH    (4) /* Semtech */
#define OS_SENSOR_VENDOR_GOERTEK    (5) /* Goertek */
#define OS_SENSOR_VENDOR_MIRAMEMS   (6) /* MiraMEMS */
#define OS_SENSOR_VENDOR_DALLAS     (7) /* Dallas */
#define OS_SENSOR_VENDOR_ADI        (8) /* ADI */
#define OS_SENSOR_VENDOR_SENSIRION  (9) /* Sensirion */

/* Sensor unit types */
#define OS_SENSOR_UNIT_NONE        (0)
#define OS_SENSOR_UNIT_MG          (1)  /* Accelerometer           unit: mG          */
#define OS_SENSOR_UNIT_MDPS        (2)  /* Gyroscope               unit: mdps        */
#define OS_SENSOR_UNIT_MGAUSS      (3)  /* Magnetometer            unit: mGauss      */
#define OS_SENSOR_UNIT_MLUX        (4)  /* Ambient light           unit: mlux        */
#define OS_SENSOR_UNIT_LUX         (5)  /* Ambient light           unit: lux         */
#define OS_SENSOR_UNIT_M           (6)  /* Distance                unit: m           */
#define OS_SENSOR_UNIT_CM          (7)  /* Distance                unit: cm          */
#define OS_SENSOR_UNIT_MM          (8)  /* Distance                unit: mm          */
#define OS_SENSOR_UNIT_PA          (9)  /* Barometer               unit: pa          */
#define OS_SENSOR_UNIT_MPERMILLAGE (10) /* Relative Humidity       unit: mpermillage */
#define OS_SENSOR_UNIT_PERMILLAGE  (11) /* Relative Humidity       unit: permillage  */
#define OS_SENSOR_UNIT_MDCELSIUS   (12) /* Temperature             unit: mdCelsius   */
#define OS_SENSOR_UNIT_DCELSIUS    (13) /* Temperature             unit: dCelsius    */
#define OS_SENSOR_UNIT_HZ          (14) /* Frequency               unit: HZ          */
#define OS_SENSOR_UNIT_ONE         (15) /* Dimensionless quantity  unit: 1           */
#define OS_SENSOR_UNIT_BPM         (16) /* Heart rate              unit: bpm         */
#define OS_SENSOR_UNIT_MN          (17) /* Force                   unit: mN          */
#define OS_SENSOR_UNIT_RAW         (18) /* Raw Data                unit: count       */

/* Sensor communication interface types */
#define OS_SENSOR_INTF_I2C     (1 << 0)
#define OS_SENSOR_INTF_SPI     (1 << 1)
#define OS_SENSOR_INTF_UART    (1 << 2)
#define OS_SENSOR_INTF_ONEWIRE (1 << 3)
#define OS_SENSOR_INTF_ADC     (1 << 4)

/* Sensor power mode types */
#define OS_SENSOR_POWER_NONE   (0)
#define OS_SENSOR_POWER_DOWN   (1) /* power down mode   */
#define OS_SENSOR_POWER_NORMAL (2) /* normal-power mode */
#define OS_SENSOR_POWER_LOW    (3) /* low-power mode    */
#define OS_SENSOR_POWER_HIGH   (4) /* high-power mode   */

/* Sensor work mode types */
#define OS_SENSOR_MODE_NONE    (0)
#define OS_SENSOR_MODE_POLLING (1) /* One shot only read a data */
#define OS_SENSOR_MODE_INT     (2) /* TODO: One shot interrupt only read a data */
#define OS_SENSOR_MODE_FIFO    (3) /* TODO: One shot interrupt read all fifo data */

/* Sensor control cmd types */
#define OS_SENSOR_CTRL_GET_ID    (0) /* Get device id */
#define OS_SENSOR_CTRL_GET_INFO  (1) /* Get sensor info */
#define OS_SENSOR_CTRL_SET_RANGE (2) /* Set the measure range of sensor. unit is info of sensor */
#define OS_SENSOR_CTRL_SET_ODR   (3) /* Set output date rate. unit is HZ */
#define OS_SENSOR_CTRL_SET_MODE  (4) /* Set sensor's work mode. ex. OS_SENSOR_MODE_POLLING,OS_SENSOR_MODE_INT */
#define OS_SENSOR_CTRL_SET_POWER (5) /* Set power mode. ex. OS_SENSOR_POWER_DOWN,OS_SENSOR_POWER_NORMAL */
#define OS_SENSOR_CTRL_SELF_TEST (6) /* Take a self test */

struct os_sensor_info
{
    os_uint8_t  type;       /* The sensor type */
    os_uint8_t  vendor;     /* Vendor of sensors */
    const char *model;      /* model name of sensor */
    os_uint8_t  unit;       /* unit of measurement */
    os_uint8_t  intf_type;  /* Communication interface type */
    os_int32_t  range_max;  /* maximum range of this sensor's value. unit is 'unit' */
    os_int32_t  range_min;  /* minimum range of this sensor's value. unit is 'unit' */
    os_uint32_t period_min; /* Minimum measurement period,unit:ms. zero = not a constant rate */
    os_uint8_t  fifo_max;
};

struct os_sensor_intf
{
    char      *dev_name;  /* The name of the communication device */
    os_uint8_t type;      /* Communication interface type */
    void      *user_data; /* Private data for the sensor. ex. i2c addr,spi cs,control I/O */
};

struct os_sensor_config
{
    struct os_sensor_intf     intf;    /* sensor interface config */
    struct os_device_pin_mode irq_pin; /* Interrupt pin, The purpose of this pin is to notification read data */
    os_uint8_t                mode;    /* sensor work mode */
    os_uint8_t                power;   /* sensor power mode */
    os_uint16_t               odr;     /* sensor out data rate */
    os_int32_t                range;   /* sensor range of measurement */
};

typedef struct os_sensor_device *os_sensor_t;

struct os_sensor_device
{
    struct os_device parent; /* The standard device */

    struct os_sensor_info   info;   /* The sensor info data */
    struct os_sensor_config config; /* The sensor config data */

    void     *data_buf; /* The buf of the data received */
    os_size_t data_len; /* The size of the data received */

    const struct os_sensor_ops *ops; /* The sensor ops */

    os_err_t (*irq_handle)(os_sensor_t sensor); /* Called when an interrupt is generated, registered by the driver */
};

struct sensor_3_axis
{
    os_int32_t x;
    os_int32_t y;
    os_int32_t z;
};

struct os_sensor_data
{
    os_uint32_t timestamp; /* The timestamp when the data was received */
    os_uint8_t  type;      /* The sensor type of the data */
    union
    {
        struct sensor_3_axis acce;      /* Accelerometer.       unit: mG          */
        struct sensor_3_axis gyro;      /* Gyroscope.           unit: mdps        */
        struct sensor_3_axis mag;       /* Magnetometer.        unit: mGauss      */
        os_int32_t           temp;      /* Temperature.         unit: dCelsius    */
        os_int32_t           humi;      /* Relative humidity.   unit: permillage  */
        os_int32_t           baro;      /* Pressure.            unit: pascal (Pa) */
        os_int32_t           light;     /* Light.               unit: lux         */
        os_int32_t           proximity; /* Distance.            unit: centimeters */
        os_int32_t           hr;        /* Heart rate.          unit: bpm         */
        os_int32_t           tvoc;      /* TVOC.                unit: permillage  */
        os_int32_t           noise;     /* Noise Loudness.      unit: HZ          */
        os_uint32_t          step;      /* Step sensor.         unit: 1           */
        os_int32_t           force;     /* Force sensor.        unit: mN          */
        os_int32_t           alti;      /* Altitude sensor.     unit: m           */
        os_uint32_t          raw;       /* Raw Data.            unit: 1           */
    } data;
};

struct os_sensor_ops
{
    os_size_t (*fetch_data)(struct os_sensor_device *sensor, void *buf, os_size_t len);
    os_err_t (*control)(struct os_sensor_device *sensor, int cmd, void *arg);
};

int os_hw_sensor_register(os_sensor_t sensor, const char *name, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H__ */
