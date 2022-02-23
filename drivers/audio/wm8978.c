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
 * @file        wm8978.c
 *
 * @brief       This file implements audio driver for wm8978.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <arch_interrupt.h>

#include <stdint.h>
#include <string.h>
#include "drv_cfg.h"

#include "drv_audio.h"
#include <i2c/i2c.h>
#include <audio/sai.h>

#include <dlog.h>
#define DBG_EXT_TAG "audio.wm8960"

struct wm8978_dev
{
    struct os_i2c_client i2c;
    os_int16_t power_pin;
};

struct wm8978_player_device
{    
    struct            os_audio_device audio;
    struct            os_audio_configure replay_config;
    os_uint8_t         volume;  
    os_device_t     *cfg_bus;
    os_device_t     *data_bus;
    struct wm8978_dev wm8978;
};
 
#define EQ1_80Hz        0X00
#define EQ1_105Hz       0X01
#define EQ1_135Hz       0X02
#define EQ1_175Hz       0X03

#define EQ2_230Hz       0X00
#define EQ2_300Hz       0X01
#define EQ2_385Hz       0X02
#define EQ2_500Hz       0X03

#define EQ3_650Hz       0X00
#define EQ3_850Hz       0X01
#define EQ3_1100Hz      0X02
#define EQ3_14000Hz     0X03

#define EQ4_1800Hz      0X00
#define EQ4_2400Hz      0X01
#define EQ4_3200Hz      0X02
#define EQ4_4100Hz      0X03

#define EQ5_5300Hz      0X00
#define EQ5_6900Hz      0X01
#define EQ5_9000Hz      0X02
#define EQ5_11700Hz     0X03

/* wm8978 reg shadow */
static os_uint16_t wm8978_REGVAL_TBL[58]=
{
    0X0000, 0X0000, 0X0000, 0X0000, 0X0050, 0X0000, 0X0140, 0X0000,
    0X0000, 0X0000, 0X0000, 0X00FF, 0X00FF, 0X0000, 0X0100, 0X00FF,
    0X00FF, 0X0000, 0X012C, 0X002C, 0X002C, 0X002C, 0X002C, 0X0000,
    0X0032, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000,
    0X0038, 0X000B, 0X0032, 0X0000, 0X0008, 0X000C, 0X0093, 0X00E9,
    0X0000, 0X0000, 0X0000, 0X0000, 0X0003, 0X0010, 0X0010, 0X0100,
    0X0100, 0X0002, 0X0001, 0X0001, 0X0039, 0X0039, 0X0039, 0X0039,
    0X0001, 0X0001,
}; 

static os_uint8_t wm8978_Write_Reg(struct os_i2c_client *i2c, os_uint8_t reg, os_uint16_t val)
{
    os_i2c_client_write(i2c, (reg << 1) | ((val >> 8) & 1), 1, (os_uint8_t *)&val, 1);
    wm8978_REGVAL_TBL[reg] = val;
    return 0;   
}  

static os_uint16_t wm8978_Read_Reg(struct os_i2c_client *client, os_uint8_t reg)
{  
    return wm8978_REGVAL_TBL[reg];  
}

static void wm8978_init(struct wm8978_dev *wm8978)
{ 
    struct os_i2c_client *i2c = &wm8978->i2c;

    /* reset */
    wm8978_Write_Reg(i2c, 0, 0);
    
    /* init */
    wm8978_Write_Reg(i2c, 1, 0x1b);      // R1,  MICEN = 1, BIASEN = 1, VMIDSEL[1:0] == 11(5K)
    wm8978_Write_Reg(i2c, 2, 0x1b0);     // R2,  ROUT1, LOUT1 enable , BOOSTENR, BOOSTENL
    wm8978_Write_Reg(i2c, 3, 0x6c);      // R3,  LOUT2, ROUT2 enable , RMIX, LMIX
    wm8978_Write_Reg(i2c, 6, 0);         // R6,  EXTERN MCLK
    wm8978_Write_Reg(i2c, 43, 1 << 4);   // R43, INVROUT2
    wm8978_Write_Reg(i2c, 47, 1 << 8);   // R47, PGABOOSTL, LEFT MIC GAIN 20
    wm8978_Write_Reg(i2c, 48, 1 << 8);   // R48, PGABOOSTR, RIGHT MIC GAIN 20
    wm8978_Write_Reg(i2c, 49, 1 << 1);   // R49, TSDEN
    wm8978_Write_Reg(i2c, 49, 1 << 2);   // R49, SPEAKER BOOST,1.5x 
    wm8978_Write_Reg(i2c, 10, 1 << 3);   // R10, SOFTMUTE, 128x sample, best snr 
    wm8978_Write_Reg(i2c, 14, 1 << 3);   // R14, ADC 128x sample
} 

/* mic gain:0~63, -12dB~35.25dB, 0.75dB/Step */
OS_USED static void wm8978_MIC_Gain(struct wm8978_dev *wm8978, os_uint8_t gain)
{
    gain&=0X3F;
    wm8978_Write_Reg(&wm8978->i2c, 45, gain);
    wm8978_Write_Reg(&wm8978->i2c, 46, gain | 1 << 8);
}

/* L2/R2 Line In gain:0~7, 0: disable, 1~7: -12dB~6dB, 3dB/Step */
OS_USED static void wm8978_LINEIN_Gain(struct wm8978_dev *wm8978, os_uint8_t gain)
{
    os_uint16_t regval;
    
    gain &= 7;
    
    /* R47 */
    regval  = wm8978_Read_Reg(&wm8978->i2c, 47);
    regval &= ~(7 << 4);
    wm8978_Write_Reg(&wm8978->i2c, 47, regval|gain << 4);
    
    /* R48 */
    regval  = wm8978_Read_Reg(&wm8978->i2c, 48);
    regval &= ~(7 << 4);
    wm8978_Write_Reg(&wm8978->i2c, 48, regval | gain << 4);
}

/* AUXR,AUXL PWM AUDIO gain:0~7, 0: disbale, 1~7: -12dB~6dB,3dB/Step */
OS_USED static void wm8978_AUX_Gain(struct wm8978_dev *wm8978, os_uint8_t gain)
{
    os_uint16_t regval;
    
    gain &= 7;
    
    /* R47 */
    regval  = wm8978_Read_Reg(&wm8978->i2c, 47);
    regval &= ~(7 << 0);
    wm8978_Write_Reg(&wm8978->i2c, 47, regval | gain << 0);
    
    /* R48 */
    regval  = wm8978_Read_Reg(&wm8978->i2c, 48);
    regval &= ~(7 << 0);
    wm8978_Write_Reg(&wm8978->i2c, 48, regval | gain << 0);
}

/* DAC/ADC enable, 1: enable, 2: disable */
OS_USED static void wm8978_ADDA_Cfg(struct wm8978_dev *wm8978, os_uint8_t dacen, os_uint8_t adcen)
{
    os_uint16_t regval;
    
    /* R3 */
    regval = wm8978_Read_Reg(&wm8978->i2c, 3);
    
    /* DACR, DACL */
    if (dacen)
        regval |= 3 << 0;
    else 
        regval &= ~(3 << 0);
    
    wm8978_Write_Reg(&wm8978->i2c, 3, regval);
    
    /* R2 */
    regval = wm8978_Read_Reg(&wm8978->i2c, 2);
    
    /* ADCR, ADCL */
    if (adcen)
        regval |= 3 << 0;
    else 
        regval &= ~(3 << 0);
    
    wm8978_Write_Reg(&wm8978->i2c, 2, regval);
}

/* Input Config: micen, lineinen, auxen. 1: enable, 2: disable */
OS_USED static void wm8978_Input_Cfg(struct wm8978_dev *wm8978, os_uint8_t micen, os_uint8_t lineinen,os_uint8_t auxen)
{
    os_uint16_t regval;  
    
    /* R2 */
    regval = wm8978_Read_Reg(&wm8978->i2c, 2);
    
    /* INPPGAENR, INPPGAENL */
    if (micen)
        regval |= 3 << 2;
    else
        regval &= ~(3 << 2);
    
    wm8978_Write_Reg(&wm8978->i2c, 2, regval);
    
    /* R44 */
    regval = wm8978_Read_Reg(&wm8978->i2c, 44);
    
    /* LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA */
    if (micen)
        regval |= 3 << 4 | 3 << 0;
    else
        regval &= ~(3 << 4 | 3 << 0);
    
    wm8978_Write_Reg(&wm8978->i2c, 44, regval);
    
    /* LINE IN */
    if (lineinen)
        wm8978_LINEIN_Gain(wm8978, 5);
    else
        wm8978_LINEIN_Gain(wm8978, 0);
    
    /* AUX 6dB */
    if (auxen)
        wm8978_AUX_Gain(wm8978, 7);
    else
        wm8978_AUX_Gain(wm8978, 0);
}

/* Output Config: dacen. 1: enable, 2: disable */
/* bpsen: Bypass Output(MIC, LINE IN, AUX) 1: enable, 2: disable */
OS_USED static void wm8978_Output_Cfg(struct wm8978_dev *wm8978, os_uint8_t dacen, os_uint8_t bpsen)
{
    os_uint16_t regval = 0;
    
    if(dacen)
        regval |= 1 << 0;
    
    if(bpsen)
    {
        /* bypass en */
        regval |= 1 << 1;
        
        /* 0dB */
        regval |= 5 << 2;
    }
    
    wm8978_Write_Reg(&wm8978->i2c, 50, regval);
    wm8978_Write_Reg(&wm8978->i2c, 51, regval);
}

/* fmt:0 LSB, 1 MSB, 2 philips I2S, 3 PCM/DSP */
/* len:0 16b, 1 20b, 2 24b, 3 32b */
OS_USED static void wm8978_I2S_Cfg(struct wm8978_dev *wm8978, os_uint8_t fmt,os_uint8_t len)
{
    fmt &= 3;
    len &= 3;
    wm8978_Write_Reg(&wm8978->i2c, 4, (fmt << 3) | (len << 5));
}   

/* headset volume, voll(0-63) volr(0-63) */
OS_USED static void wm8978_HPvol_Set(struct wm8978_dev *wm8978, os_uint8_t voll,os_uint8_t volr)
{
    voll &= 0x3f;
    volr &= 0x3f;
    
    /* mute */
    if (voll == 0)
        voll |= 1 << 6;
    
    if (volr == 0)
        volr |= 1 << 6;
    
    wm8978_Write_Reg(&wm8978->i2c, 52, voll);
    wm8978_Write_Reg(&wm8978->i2c, 53, volr | (1 << 8));    /* HPVU=1 */
}

/* speaker volume, volx(0-63) */
OS_USED static void wm8978_SPKvol_Set(struct wm8978_dev *wm8978, os_uint8_t volx)
{ 
    volx &= 0x3f;
    
    /* mute */
    if (volx == 0)
        volx |= 1 << 6;
    
    wm8978_Write_Reg(&wm8978->i2c, 54, volx);
    wm8978_Write_Reg(&wm8978->i2c, 55, volx | (1 << 8));    /* SPKVU=1 */
}

/* 3D surround depth:0~15(0:weak, 15:strong) */
OS_USED static void wm8978_3D_Set(struct wm8978_dev *wm8978, os_uint8_t depth)
{ 
    depth &= 0xf;
    
    wm8978_Write_Reg(&wm8978->i2c, 41, depth); //R41,3D环绕设置    
}

/* 3D dir. 0: ADC, 1: DAC(default) */
OS_USED static void wm8978_EQ_3D_Dir(struct wm8978_dev *wm8978, os_uint8_t dir)
{
    os_uint16_t regval; 
    
    regval = wm8978_Read_Reg(&wm8978->i2c, 0x12);
    
    if(dir)
        regval |= 1 << 8;
    else
        regval &= ~(1 << 8);
    
    wm8978_Write_Reg(&wm8978->i2c, 18, regval);
}

/* EQ1 cfreq:cut-off frequency, 0~3: 80/105/135/175Hz */
/* gain:0~24, -12~+12dB */
OS_USED static void wm8978_EQ1_Set(struct wm8978_dev *wm8978, os_uint8_t cfreq, os_uint8_t gain)
{ 
    os_uint16_t regval;
    
    cfreq &= 3;
    
    if (gain > 24)
        gain = 24;
    
    gain    = 24 - gain;
    regval  = wm8978_Read_Reg(&wm8978->i2c, 18);
    regval &= 0x100;
    regval |= cfreq << 5;
    regval |= gain;
    wm8978_Write_Reg(&wm8978->i2c, 18, regval);
}

/* EQ2 cfreq:center frequency, 0~3: 230/300/385/500Hz */
/* gain:0~24, -12~+12dB */
OS_USED static void wm8978_EQ2_Set(struct wm8978_dev *wm8978, os_uint8_t cfreq, os_uint8_t gain)
{ 
    os_uint16_t regval=0;
    
    cfreq &= 3;
    
    if (gain > 24)
        gain = 24;
    
    gain    = 24 - gain;
    regval |= cfreq << 5;
    regval |= gain;
    wm8978_Write_Reg(&wm8978->i2c, 19, regval);
}

/* EQ3 cfreq:center frequency,0~3: 650/850/1100/1400Hz */
/* gain:0~24, -12~+12dB */
OS_USED static void wm8978_EQ3_Set(struct wm8978_dev *wm8978, os_uint8_t cfreq, os_uint8_t gain)
{ 
    os_uint16_t regval=0;
    
    cfreq &= 3;
    
    if (gain > 24)
        gain = 24;
    
    gain    = 24 - gain; 
    regval |= cfreq << 5;
    regval |= gain;
    wm8978_Write_Reg(&wm8978->i2c, 20, regval);
}

/* EQ4 cfreq:center frequency, 0~3: 1800/2400/3200/4100Hz */
/* gain:0~24, -12~+12dB */
OS_USED static void wm8978_EQ4_Set(struct wm8978_dev *wm8978, os_uint8_t cfreq, os_uint8_t gain)
{ 
    os_uint16_t regval=0;
    
    cfreq &= 3;
    
    if (gain > 24)
        gain = 24;
    
    gain    = 24 - gain; 
    regval |= cfreq << 5;
    regval |= gain;
    wm8978_Write_Reg(&wm8978->i2c, 21, regval);
}

/* EQ5 cfreq:center frequency, 0~3: 5300/6900/9000/11700Hz */
/* gain:0~24, -12~+12dB */
OS_USED static void wm8978_EQ5_Set(struct wm8978_dev *wm8978, os_uint8_t cfreq, os_uint8_t gain)
{ 
    os_uint16_t regval=0;
    
    cfreq &= 3;
    
    if (gain > 24)
        gain = 24;
    
    gain    = 24 - gain; 
    regval |= cfreq << 5;
    regval |= gain;
    wm8978_Write_Reg(&wm8978->i2c, 22, regval);
}

/* WM8978 Device Driver Interface */

static os_err_t audio_wm8978_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t result = OS_EOK;
    struct wm8978_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;

    switch (caps->config_type)
    {
        case AUDIO_VOLUME_CMD:
        {
            os_uint8_t volume = caps->udata.value;

            wm8978_HPvol_Set(&aduio_dev->wm8978, volume, volume);   /* headset volume */
            wm8978_SPKvol_Set(&aduio_dev->wm8978, volume);          /* speaker volume */
            
            aduio_dev->volume = volume;
            LOG_D(DBG_EXT_TAG, "set volume %d", volume);
            break;
        }

        case AUDIO_PARAM_CMD:
        {
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &caps->udata.config.samplerate);
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &caps->udata.config.channels);

            aduio_dev->replay_config.samplerate = caps->udata.config.samplerate;
            aduio_dev->replay_config.channels   = caps->udata.config.channels;
            aduio_dev->replay_config.samplebits = caps->udata.config.samplebits;
            LOG_D(DBG_EXT_TAG, "set samplerate %d", aduio_dev->replay_config.samplerate);
            break;
        }

        default:
            result = OS_ERROR;
            break;
        }

    return result;
}

static os_err_t audio_wm8978_init(struct os_audio_device *audio)  
{  
    os_err_t result = OS_EOK;
    struct wm8978_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;

    wm8978_init(&aduio_dev->wm8978);
    wm8978_HPvol_Set(&aduio_dev->wm8978, 20, 20);   /* headset volume */
    wm8978_SPKvol_Set(&aduio_dev->wm8978, 20);      /* speaker volume */

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_wm8978_start(struct os_audio_device *audio)
{
    struct wm8978_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;

    wm8978_ADDA_Cfg(&aduio_dev->wm8978, 1, 0);          /* enable dac */
    wm8978_Input_Cfg(&aduio_dev->wm8978, 0, 0, 0);      /* disable input */
    wm8978_Output_Cfg(&aduio_dev->wm8978, 1, 0);        /* start dac */
    
    LOG_D(DBG_EXT_TAG, "open sound device");
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_ENABLE, OS_NULL);
    
    return OS_EOK;
}

static os_err_t audio_wm8978_stop(struct os_audio_device *audio)
{
    struct wm8978_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_DISABLE, OS_NULL);

    wm8978_ADDA_Cfg(&aduio_dev->wm8978, 0, 0);          /* disable dac */
    wm8978_Input_Cfg(&aduio_dev->wm8978, 0, 0, 0);      /* disable input */
    wm8978_Output_Cfg(&aduio_dev->wm8978, 0, 0);        /* stop dac */
    
    LOG_D(DBG_EXT_TAG, "close sound device");
    
    return OS_EOK;
}

os_size_t audio_wm8978_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct wm8978_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;

    return os_device_write_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

os_size_t audio_wm8978_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct wm8978_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8978_player_device *)audio->parent.user_data;
    
    return os_device_read_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)readBuf, size);
}

os_err_t audio_wm8978_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct wm8978_player_device *wm8978_player_dev = dev->user_data;

        struct os_device_cb_info *dev_info = os_list_first_entry(&wm8978_player_dev->audio.parent.cb_heads[OS_DEVICE_CB_TYPE_TX], struct os_device_cb_info, list);

        dev_info->data = info->data;

        os_device_send_notify(&wm8978_player_dev->audio.parent);

        return OS_EOK;
    }
    
    return OS_ENOSYS;
}

static struct os_audio_ops wm8978_player_ops =
{
    .getcaps            = OS_NULL, 
    .configure          = audio_wm8978_config,
    .init               = audio_wm8978_init,
    .start              = audio_wm8978_start,
    .stop               = audio_wm8978_stop,
    .transmit           = audio_wm8978_transmit, 
    .receive            = audio_wm8978_receive,
};

int os_hw_audio_player_init(void)
{
    struct wm8978_player_device *wm8978_player;
    struct wm8978_dev *wm8978;

    wm8978_player = os_calloc(1, sizeof(struct wm8978_player_device));
    OS_ASSERT(wm8978_player != OS_NULL);
    
    wm8978 = &wm8978_player->wm8978;

    wm8978_player->replay_config.samplerate = 44100;
    wm8978_player->replay_config.channels   = 2;
    wm8978_player->replay_config.samplebits = 16;
    wm8978_player->volume                   = 50;

    /* register sound device */
    wm8978_player->audio.ops = &wm8978_player_ops;
    
    wm8978_player->cfg_bus = os_device_find(BSP_WM8978_I2C_BUS);
    if (wm8978_player->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    wm8978_player->data_bus = os_device_find(BSP_AUDIO_DATA_TX_BUS);
    if (wm8978_player->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    wm8978_player->data_bus->user_data = wm8978_player;
    os_device_open(wm8978_player->data_bus);
    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_wm8978_data_tx_done;
    os_device_control(wm8978_player->data_bus, OS_DEVICE_CTRL_SET_CB, info);
    
    wm8978->i2c.bus = (struct os_i2c_bus_device *)wm8978_player->cfg_bus;

    wm8978->i2c.client_addr = BSP_WM8978_I2C_ADDR;
    wm8978->power_pin = BSP_WM8978_POWER_PIN;

    os_audio_player_register(&wm8978_player->audio, "audio0", wm8978_player);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init, OS_INIT_SUBLEVEL_LOW);
