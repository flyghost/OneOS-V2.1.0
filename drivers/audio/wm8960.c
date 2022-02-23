/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_wm8960.h"
#include "sai.h"
#include <dlog.h>
#define DBG_EXT_TAG "audio.wm8960"

wm8960_config_t wm8960Config = {
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .i2cBase          = LPI2C1,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format = {.mclk_HZ = 6144000U, .sampleRate = kWM8960_AudioSampleRate16KHz, .bitWidth = kWM8960_AudioBitWidth16bit},
    .master_slave = false,
};

const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

struct wm8960_dev
{
    struct os_i2c_client i2c;
    os_int16_t power_pin;
};

struct wm8960_player_device
{
    struct os_audio_device      audio;
    struct os_audio_configure   replay_config;

    os_uint8_t                  volume;  
    os_device_t     *cfg_bus;
    os_device_t     *data_bus;

    wm8960_handle_t wm8960Handle;
    struct wm8960_dev wm8960;
};

static os_err_t audio_wm8960_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t result = OS_EOK;
    struct wm8960_player_device *aduio_dev;

    LOG_E(DBG_EXT_TAG, "audio_wm8960_config\r\n");
    
    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;

    switch (caps->config_type)
    {
        case AUDIO_VOLUME_CMD:
        {
            os_uint8_t volume = caps->udata.value;
            
            WM8960_SetVolume(&aduio_dev->wm8960Handle, kWM8960_ModuleHP, volume);
            WM8960_SetVolume(&aduio_dev->wm8960Handle, kWM8960_ModuleSpeaker, volume);
            
            aduio_dev->volume = volume;
            LOG_E(DBG_EXT_TAG, "set volume %d\r\n", volume);
            break;
        }

        case AUDIO_PARAM_CMD:
        {
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

            /* save configs */
            aduio_dev->replay_config.samplerate = caps->udata.config.samplerate;
            aduio_dev->replay_config.channels   = caps->udata.config.channels;
            aduio_dev->replay_config.samplebits = caps->udata.config.samplebits;
            LOG_E(DBG_EXT_TAG, "set samplerate %d\r\n", aduio_dev->replay_config.samplerate);
            break;
        }

        default:
            result = OS_ERROR;
            break;
        }

    return result;
}

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        IOMUXC_GPR->GPR1 &= (~IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK);
    }
}

static os_err_t audio_wm8960_init(struct os_audio_device *audio)  
{  
    os_err_t result = OS_EOK;
    struct wm8960_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;
    
    /* Set SAI1_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, 1);
    /* Set SAI1_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai1Div, 63);
    /* Set Sai1 clock source. */
    CLOCK_SetMux(kCLOCK_Sai1Mux, 2);
    WM8960_Init(&aduio_dev->wm8960Handle, &wm8960Config);
    CLOCK_InitAudioPll(&audioPllConfig);
    BOARD_EnableSaiMclkOutput(true);

#if 0
    WM8960_SetVolume(&aduio_dev->wm8960Handle, kWM8960_ModuleHP, aduio_dev->volume);
    WM8960_SetVolume(&aduio_dev->wm8960Handle, kWM8960_ModuleSpeaker, aduio_dev->volume);
    
    /* set default params */ 
    os_sai_frequency_set(aduio_dev->sai, aduio_dev->replay_config.samplerate);
    os_sai_channel_set(aduio_dev->sai, aduio_dev->replay_config.channels);
#endif

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_wm8960_start(struct os_audio_device *audio)
{
    struct wm8960_player_device *aduio_dev;

    LOG_E(DBG_EXT_TAG, "audio_wm8960_start\r\n");
    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;

    

    //wm8960_ADDA_Cfg(&aduio_dev->wm8960, 1, 0);          /* enable dac */
    //wm8960_Input_Cfg(&aduio_dev->wm8960, 0, 0, 0);      /* disable input */
   // wm8960_Output_Cfg(&aduio_dev->wm8960, 1, 0);        /* start dac */
    
    LOG_E(DBG_EXT_TAG, "open sound device");
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_ENABLE, OS_NULL);
    
    return OS_EOK;
}

static os_err_t audio_wm8960_stop(struct os_audio_device *audio)
{
    struct wm8960_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_DISABLE, OS_NULL);

    //wm8960_ADDA_Cfg(&aduio_dev->wm8960, 0, 0);          /* disable dac */
    //wm8960_Input_Cfg(&aduio_dev->wm8960, 0, 0, 0);      /* disable input */
    //wm8960_Output_Cfg(&aduio_dev->wm8960, 0, 0);        /* stop dac */
    
    LOG_E(DBG_EXT_TAG, "close sound device");
    
    return OS_EOK;
}

os_size_t audio_wm8960_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct wm8960_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;

    return os_device_write_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

os_size_t audio_wm8960_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct wm8960_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct wm8960_player_device *)audio->parent.user_data;
    
    return os_device_read_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)readBuf, size);
}

os_err_t audio_wm8960_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct wm8960_player_device *wm8960_player_dev = dev->user_data;

        os_device_send_notify(&wm8960_player_dev->audio.parent);
        
        return OS_EOK;
    }
    return OS_ENOSYS;
}

static struct os_audio_ops wm8960_player_ops =
{
    .getcaps            = OS_NULL, 
    .configure          = audio_wm8960_config,
    .init               = audio_wm8960_init,
    .start              = audio_wm8960_start,
    .stop               = audio_wm8960_stop,
    .transmit           = audio_wm8960_transmit, 
    .receive            = audio_wm8960_receive,
};

int os_hw_audio_player_init(void)
{
    struct wm8960_player_device *wm8960_player;
    struct wm8960_dev *wm8960;

    wm8960_player = os_calloc(1, sizeof(struct wm8960_player_device));
    OS_ASSERT(wm8960_player != OS_NULL);

    /* init default confsiguration */
    wm8960_player->replay_config.samplerate = 44100;
    wm8960_player->replay_config.channels   = 2;
    wm8960_player->replay_config.samplebits = 16;
    wm8960_player->volume                   = 20;

    /* register sound device */
    wm8960_player->audio.ops = &wm8960_player_ops;

    wm8960_player->cfg_bus = os_device_find(BSP_WM8960_I2C_BUS);
    if (wm8960_player->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    wm8960_player->data_bus = os_device_find(BSP_WM8960_DATA_BUS);
    if (wm8960_player->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    wm8960_player.data_bus->user_data = wm8960_player;
    os_device_open(wm8960_player->data_bus);
    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_wm8960_data_tx_done;
    os_device_control(wm8960_player->data_bus, OS_DEVICE_CTRL_SET_CB, info);
    
    wm8960->i2c.bus = (struct os_i2c_bus_device *)wm8960_player->cfg_bus;

    wm8960->i2c.client_addr = BSP_WM8960_I2C_ADDR;
    wm8960->power_pin = BSP_WM8978_POWER_PIN;


    os_audio_player_register(&wm8960_player->audio, "audio0", wm8960_player);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init, OS_INIT_SUBLEVEL_LOW);

