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
 * @file        sht20.c
 *
 * @brief       This file provides functions for sht20.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_memory.h>
#include <sensors/sensor.h>
#include <math.h>

#define DBG_TAG "sensor.sensirion.sht20"
#include <drv_log.h>

#define SHT20_Measurement_RH_HM  0XE5
#define SHT20_Measurement_T_HM   0XE3
#define SHT20_Measurement_RH_NHM 0XF5
#define SHT20_Measurement_T_NHM  0XF3
#define SHT20_READ_REG           0XE7
#define SHT20_WRITE_REG          0XE6
#define SHT20_SOFT_RESET         0XFE

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    union
    {
        float tempreture;
        float humidity;
    } value;
} sht20_info_t;

static const int16_t POLYNOMIAL = 0x131;

static void sht20_reset(sht20_info_t *sht20)
{
    os_i2c_client_write(&sht20->i2c, SHT20_SOFT_RESET, 1, NULL, 0);
}

static unsigned char sht20_read_user_reg(sht20_info_t *sht20)
{
    return os_i2c_client_read_byte(&sht20->i2c, SHT20_READ_REG, 1);
}

static unsigned char sht20_write_user_reg(sht20_info_t *sht20, uint8_t value)
{
    return os_i2c_client_write_byte(&sht20->i2c, SHT20_WRITE_REG, 1, value);
}

static char SHT2x_CheckCrc(unsigned char *data, char nbrOfBytes)
{
    char crc     = 0;
    char bit     = 0;
    char byteCtr = 0;

    /* Calculates 8-Bit checksum with given polynomial */
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }

    return crc;
}

static float SHT2x_CalcTemperatureC(unsigned short u16sT)
{
    float temperatureC = 0;

    /* Clear bits [1..0] (status bits) */
    u16sT &= ~0x0003;

    /* T= -46.85 + 175.72 * ST/2^16 */
    temperatureC = -46.85 + 175.72 / 65536 * (float)u16sT;

    return temperatureC;
}

static float SHT2x_CalcRH(unsigned short u16sRH)
{
    float humidityRH = 0;

    /* Clear bits [1..0] (status bits) */
    u16sRH &= ~0x0003;

    /* HumidityRH = -6.0 + 125.0/65536 * (float)u16sRH */
    humidityRH = ((float)u16sRH * 0.00190735) - 6;

    return humidityRH;
}

static unsigned short SHT2x_MeasureHM(sht20_info_t *sht20, unsigned char cmd)
{
    char          crc;
    unsigned char buff[3];
    
    sht20_read_user_reg(sht20);
    
    /* Send cmd to sht20 and wait Measure */
    os_i2c_client_read(&sht20->i2c, cmd, 1, buff, 0);
    os_task_msleep(85);
    /* Get sht20 data */
    os_i2c_client_read(&sht20->i2c, cmd, 0, buff, 3);
    
    sht20_read_user_reg(sht20);
    sht20_write_user_reg(sht20, 0);
    sht20_read_user_reg(sht20);

    crc = SHT2x_CheckCrc(buff, 2);
    if (crc == buff[2])
        return ((buff[0] << 8) | buff[1]);
    
    LOG_D(DBG_TAG,"sht2x measure failed cmd:%02x data:%02x%02x crc:(%02x != %02x).", cmd, buff[0], buff[1], buff[2], crc);

    return (unsigned short)-1;
}

static void sht20_get_temp(sht20_info_t *sht20)
{
    unsigned short tmp;

    tmp                     = SHT2x_MeasureHM(sht20, SHT20_Measurement_T_NHM);
    sht20->value.tempreture = SHT2x_CalcTemperatureC(tmp);
}

static void sht20_get_humi(sht20_info_t *sht20)
{
    unsigned short tmp;

    tmp                   = SHT2x_MeasureHM(sht20, SHT20_Measurement_RH_NHM);
    sht20->value.humidity = SHT2x_CalcRH(tmp);
}

static sht20_info_t *sht20_init(const char *bus_name, os_uint16_t addr)
{
    sht20_info_t *sht20 = NULL;

    sht20 = os_calloc(1, sizeof(sht20_info_t));
    if (sht20 == OS_NULL)
    {
        LOG_E(DBG_TAG,"sht20 amlloc faile");
        return NULL;
    }

    LOG_D(DBG_TAG,"sht20:[%s][0x%02x]", bus_name, addr);

    sht20->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (sht20->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"sht20 i2c invalid.");
        os_free(sht20);
        return NULL;
    }

    sht20->i2c.client_addr = addr;

    sht20_reset(sht20);
    os_task_msleep(10);

    return sht20;
}

static os_size_t sht20_temp_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    sht20_info_t *sht20 = NULL;
    struct os_sensor_data *data  = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_TEMP);
    OS_ASSERT(buf);

    sht20 = (sht20_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    sht20_get_temp(sht20);

    data->type      = sensor->info.type;
    data->data.temp = sht20->value.tempreture;
    data->timestamp = os_sensor_get_ts();

    return 0;
}

static os_err_t sht20_temp_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    sht20_info_t *sht20  = (sht20_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = sht20->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops sht20_temp_ops =
{
    sht20_temp_fetch_data,
    sht20_temp_control,
};

static int os_hw_sht20_temp_init(void)
{
    os_int8_t     result;
    sht20_info_t *sht20;

    sht20 = sht20_init(OS_SHT20_I2C_BUS_NAME, OS_SHT20_I2C_ADDR);
    if (sht20 == NULL)
    {
        LOG_E(DBG_TAG,"sht20 temp init failed.");
        goto __exit;
    }

    /* Temp */
    sht20->sensor.info.type       = OS_SENSOR_CLASS_TEMP;
    sht20->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    sht20->sensor.info.model      = "sht20";
    sht20->sensor.info.unit       = OS_SENSOR_UNIT_DCELSIUS;
    sht20->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    sht20->sensor.info.range_max  = 1000;
    sht20->sensor.info.range_min  = 0;
    sht20->sensor.info.period_min = 300;
    sht20->sensor.ops             = &sht20_temp_ops;

    result = os_hw_sensor_register(&sht20->sensor, "sht20", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device temp register err code: %d", result);
        goto __exit;
    }

    LOG_D(DBG_TAG,"sht20 temp init success");
    return OS_EOK;

__exit:
    if (sht20)
        os_free(sht20);
    return OS_ERROR;
}
OS_DEVICE_INIT(os_hw_sht20_temp_init, OS_INIT_SUBLEVEL_LOW);

static os_size_t sht20_humi_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    sht20_info_t *sht20 = NULL;
    struct os_sensor_data *data  = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_HUMI);
    OS_ASSERT(buf);

    sht20 = (sht20_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    sht20_get_humi(sht20);

    data->type      = sensor->info.type;
    data->data.humi = sht20->value.humidity;
    data->timestamp = os_sensor_get_ts();

    return 0;
}

static os_err_t sht20_humi_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    sht20_info_t *sht20  = (sht20_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = sht20->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops sht20_humi_ops =
{
    sht20_humi_fetch_data,
    sht20_humi_control,
};

static int os_hw_sht20_humi_init(void)
{
    os_int8_t     result;
    sht20_info_t *sht20;

    sht20 = sht20_init(OS_SHT20_I2C_BUS_NAME, OS_SHT20_I2C_ADDR);
    if (sht20 == NULL)
    {
        LOG_E(DBG_TAG,"sht20 humi init failed.");
        goto __exit;
    }

    /* humi */
    sht20->sensor.info.type       = OS_SENSOR_CLASS_HUMI;
    sht20->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    sht20->sensor.info.model      = "sht20";
    sht20->sensor.info.unit       = OS_SENSOR_UNIT_PERMILLAGE;
    sht20->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    sht20->sensor.info.range_max  = 1000;
    sht20->sensor.info.range_min  = 0;
    sht20->sensor.info.period_min = 300;
    sht20->sensor.ops             = &sht20_humi_ops;

    result = os_hw_sensor_register(&sht20->sensor, "sht20", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device humi register err code: %d", result);
        goto __exit;
    }

    LOG_D(DBG_TAG,"sht20 humi init success");
    return OS_EOK;

__exit:
    if (sht20)
        os_free(sht20);
    return OS_ERROR;
}
OS_DEVICE_INIT(os_hw_sht20_humi_init, OS_INIT_SUBLEVEL_LOW);
