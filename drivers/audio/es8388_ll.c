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
 * @file        drv_es8388.c
 *
 * @brief       This file provides es8388 driver functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>

#include "drv_cfg.h"
#include "es8388_ll.h"

#define DBG_SECTION_NAME "ES8388"
#include <dlog.h>

struct es8388_device
{
    struct os_i2c_bus_device *i2c;
    os_uint16_t pin;
};

static struct es8388_device es_dev = {0};

static os_uint16_t reg_read(os_uint8_t addr)
{
    struct os_i2c_msg msg[2] = {0};
    uint8_t val = 0xff;

    OS_ASSERT(es_dev.i2c != OS_NULL);

    msg[0].addr  = BSP_ES8388_I2C_ADDR;
    msg[0].flags = OS_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &addr;

    msg[1].addr  = BSP_ES8388_I2C_ADDR;
    msg[1].flags = OS_I2C_RD;
    msg[1].len   = 1;
    msg[1].buf   = &val;

    if (os_i2c_transfer(es_dev.i2c, msg, 2) != 2)
    {
        os_kprintf("I2C read data failed, reg = 0x%02x. \r\n", addr);
        return 0xff;
    }

    return val;
}

static void reg_write(os_uint8_t addr, os_uint8_t val)
{
    struct os_i2c_msg msgs[1] = {0};
    os_uint8_t buff[2] = {0};

    OS_ASSERT(es_dev.i2c != OS_NULL);

    buff[0] = addr;
    buff[1] = val;

    msgs[0].addr  = BSP_ES8388_I2C_ADDR;
    msgs[0].flags = OS_I2C_WR;
    msgs[0].buf   = buff;
    msgs[0].len   = 2;

    if (os_i2c_transfer(es_dev.i2c, msgs, 1) != 1)
    {
        os_kprintf("I2C write data failed, reg = 0x%2x. \r\n", addr);
        return;
    }
}

static int es8388_set_adc_dac_volume(int mode, int volume, int dot)
{
    int res = 0;
    if (volume < -96 || volume > 0)
    {
        if (volume < -96)
            volume = -96;
        else
            volume = 0;
    }
    dot = (dot >= 5 ? 1 : 0);
    volume = (-volume << 1) + dot;
    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_ADCCONTROL8, volume);
        reg_write(ES8388_ADCCONTROL9, volume);  
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACCONTROL5, volume);
        reg_write(ES8388_DACCONTROL4, volume);
    }
    return res;
}

void es8388_set_voice_mute(os_bool_t enable)
{
    uint8_t reg = 0;

    reg = reg_read(ES8388_DACCONTROL3);
    reg = reg & 0xFB;
    reg_write(ES8388_DACCONTROL3, reg | (((int)enable) << 2));
}

os_err_t es8388_init(os_device_t *dev, os_uint16_t pin)
{
    os_device_open(dev);
    
    es_dev.i2c = (struct os_i2c_bus_device *)dev;
    es_dev.pin = pin;

    /* enable es8388 PA */
    es8388_pa_power(OS_TRUE);

    os_task_msleep(50);

    reg_write(ES8388_DACCONTROL3, 0x04);  
    /* Chip Control and Power Management */
    reg_write(ES8388_CONTROL2, 0x50);
    reg_write(ES8388_CHIPPOWER, 0x00);  /* normal all and power up all */
    reg_write(ES8388_MASTERMODE, 0x00); /* TODO:CODEC IN I2S SLAVE MODE */

    /* dac */
    reg_write(ES8388_DACPOWER, 0xC0);     /* disable DAC and disable Lout/Rout/1/2 */
    reg_write(ES8388_CONTROL1, 0x12);     /* Enfr=0,Play&Record Mode,(0x17-both of mic&paly) */
    reg_write(ES8388_DACCONTROL1, 0x18);  /* 1a 0x18:16bit iis , 0x00:24 */
    reg_write(ES8388_DACCONTROL2, 0x02);  /* DACFsMode,SINGLE SPEED; DACFsRatio,256 */
    reg_write(ES8388_DACCONTROL16, 0x00); /* 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 */
    reg_write(ES8388_DACCONTROL17, 0x90); /* only left DAC to left mixer enable 0db */
    reg_write(ES8388_DACCONTROL20, 0x90); /* only right DAC to right mixer enable 0db */
    reg_write(ES8388_DACCONTROL21, 0x80); /* set internal ADC and DAC use the same LRCK clock, ADC LRCK as internal LRCK */
    reg_write(ES8388_DACCONTROL23, 0x00);   
    es8388_set_adc_dac_volume(ES_MODE_DAC, 0, 0);          

    reg_write(ES8388_DACPOWER, 0x3c);     /* 0x3c Enable DAC and Enable Lout/Rout/1/2 */
    /* adc */
    reg_write(ES8388_ADCPOWER, 0xFF);
    reg_write(ES8388_ADCCONTROL1, 0xbb);  /* MIC Left and Right channel PGA gain */
    reg_write(ES8388_ADCCONTROL2, 0x00);  /* 0x00 LINSEL & RINSEL, LIN1/RIN1 as ADC Input; DSSEL,use one DS Reg11; DSR, LINPUT1-RINPUT1 */
    reg_write(ES8388_ADCCONTROL3, 0x02);
    reg_write(ES8388_ADCCONTROL4, 0x0d);  /* Left/Right data, Left/Right justified mode, Bits length, I2S format */
    reg_write(ES8388_ADCCONTROL5, 0x02);  /* ADCFsMode,singel SPEED,RATIO=256 */
    /* ALC for Microphone */
    es8388_set_adc_dac_volume(ES_MODE_ADC, 0, 0);      
    reg_write(ES8388_ADCPOWER, 0x09);    /* Power on ADC, Enable LIN&RIN, Power off MICBIAS, set int1lp to low power mode */

    return OS_EOK;
}

os_err_t es8388_deinit(os_device_t *dev)
{
    os_device_close(dev);

    es8388_pa_power(OS_FALSE);

    return OS_EOK;
}

os_err_t es8388_start(enum es8388_mode mode)
{
    int res = 0;
    uint8_t prev_data = 0, data = 0;

    prev_data = reg_read(ES8388_DACCONTROL21);
    if (mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACCONTROL16, 0x09); /* 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 by pass enable */
        reg_write(ES8388_DACCONTROL17, 0x50); /* left DAC to left mixer enable  and  LIN signal to left mixer enable 0db  : bupass enable */
        reg_write(ES8388_DACCONTROL20, 0x50); /* right DAC to right mixer enable  and  LIN signal to right mixer enable 0db : bupass enable */
        reg_write(ES8388_DACCONTROL21, 0xC0); /* enable adc */
    }
    else
    {
        reg_write(ES8388_DACCONTROL21, 0x80);   /* enable dac */
    }
    data = reg_read(ES8388_DACCONTROL21);

    if (prev_data != data)
    {
        reg_write(ES8388_CHIPPOWER, 0xF0);   /* start state machine */ 
        reg_write(ES8388_CHIPPOWER, 0x00);   /* start state machine */
    }
    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC || mode == ES_MODE_LINE)
    {
        reg_write(ES8388_ADCPOWER, 0x00);   /* power up adc and line in */
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC || mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACPOWER, 0x3c);   /* power up dac and line out */
        es8388_set_voice_mute(OS_FALSE);
    }

    return res;
}

os_err_t es8388_stop(enum es8388_mode mode)
{
    int res = 0;
    if (mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACCONTROL21, 0x80); /* enable dac */
        reg_write(ES8388_DACCONTROL16, 0x00); /* 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 */
        reg_write(ES8388_DACCONTROL17, 0x90); /* only left DAC to left mixer enable 0db */
        reg_write(ES8388_DACCONTROL20, 0x90); /* only right DAC to right mixer enable 0db */
        return res;
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACPOWER, 0x00);
        es8388_set_voice_mute(OS_TRUE); 
    }
    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_ADCPOWER, 0xFF);  
    }
    if (mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACCONTROL21, 0x9C);
    }

    return OS_EOK;
}

os_err_t es8388_fmt_set(enum es8388_mode mode, enum es8388_format fmt)
{
    uint8_t reg = 0;

    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC)
    {
        reg = reg_read(ES8388_ADCCONTROL4);
        reg = reg & 0xfc;
        reg_write(ES8388_ADCCONTROL4, reg | fmt);
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC)
    {
        reg = reg_read(ES8388_DACCONTROL1);
        reg = reg & 0xf9;
        reg_write(ES8388_DACCONTROL1, reg | (fmt << 1));
    }

    return OS_EOK;
}

void es8388_volume_set(os_uint8_t volume)
{
    if (volume > 100)
        volume = 100;
    volume /= 3;

    reg_write(ES8388_DACCONTROL24, volume);
    reg_write(ES8388_DACCONTROL25, volume);
}

os_uint8_t es8388_volume_get(void)
{
    os_uint8_t volume;

    volume = reg_read(ES8388_DACCONTROL24);
    if (volume == 0xff)
    {
        volume = 0;
    }
    else
    {
        volume *= 3;
        if (volume == 99)
            volume = 100;
    }

    return volume;
}

void es8388_pa_power(os_bool_t enable)
{
    if (enable)
    {
        os_pin_write(es_dev.pin, PIN_HIGH);
    }
    else
    {
        os_pin_write(es_dev.pin, PIN_LOW);
    }
}

