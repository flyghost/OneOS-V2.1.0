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
 * @file        cs43l22.c
 *
 * @brief       This file implements audio driver for cs43l22.
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

#define DBG_EXT_TAG "audio.CS43L22"

struct cs43l22_dev
{
    struct os_i2c_client i2c;
    os_int16_t           rst_pin;
};

struct cs43l22_player_device
{
    struct os_audio_device    audio;
    struct os_audio_configure replay_config;
    os_uint8_t                volume;
    os_device_t     *cfg_bus;
    os_device_t     *data_bus;
    struct cs43l22_dev cs43l22;
};

/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER   1
#define OUTPUT_DEVICE_HEADPHONE 2
#define OUTPUT_DEVICE_BOTH      3
#define OUTPUT_DEVICE_AUTO      4

/* Volume Levels values */
#define DEFAULT_VOLMIN  0x00
#define DEFAULT_VOLMAX  0xFF
#define DEFAULT_VOLSTEP 0x04

#define AUDIO_PAUSE  0
#define AUDIO_RESUME 1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW 1
#define CODEC_PDWN_SW 2

/* MUTE commands */
#define AUDIO_MUTE_ON  1
#define AUDIO_MUTE_OFF 0

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K ((uint32_t)192000)
#define AUDIO_FREQUENCY_96K  ((uint32_t)96000)
#define AUDIO_FREQUENCY_48K  ((uint32_t)48000)
#define AUDIO_FREQUENCY_44K  ((uint32_t)44100)
#define AUDIO_FREQUENCY_32K  ((uint32_t)32000)
#define AUDIO_FREQUENCY_22K  ((uint32_t)22050)
#define AUDIO_FREQUENCY_16K  ((uint32_t)16000)
#define AUDIO_FREQUENCY_11K  ((uint32_t)11025)
#define AUDIO_FREQUENCY_8K   ((uint32_t)8000)

/** CS43l22 Registers  ***/
#define CS43L22_REG_ID                0x01
#define CS43L22_REG_POWER_CTL1        0x02
#define CS43L22_REG_POWER_CTL2        0x04
#define CS43L22_REG_CLOCKING_CTL      0x05
#define CS43L22_REG_INTERFACE_CTL1    0x06
#define CS43L22_REG_INTERFACE_CTL2    0x07
#define CS43L22_REG_PASSTHR_A_SELECT  0x08
#define CS43L22_REG_PASSTHR_B_SELECT  0x09
#define CS43L22_REG_ANALOG_ZC_SR_SETT 0x0A
#define CS43L22_REG_PASSTHR_GANG_CTL  0x0C
#define CS43L22_REG_PLAYBACK_CTL1     0x0D
#define CS43L22_REG_MISC_CTL          0x0E
#define CS43L22_REG_PLAYBACK_CTL2     0x0F
#define CS43L22_REG_PASSTHR_A_VOL     0x14
#define CS43L22_REG_PASSTHR_B_VOL     0x15
#define CS43L22_REG_PCMA_VOL          0x1A
#define CS43L22_REG_PCMB_VOL          0x1B
#define CS43L22_REG_BEEP_FREQ_ON_TIME 0x1C
#define CS43L22_REG_BEEP_VOL_OFF_TIME 0x1D
#define CS43L22_REG_BEEP_TONE_CFG     0x1E
#define CS43L22_REG_TONE_CTL          0x1F
#define CS43L22_REG_MASTER_A_VOL      0x20
#define CS43L22_REG_MASTER_B_VOL      0x21
#define CS43L22_REG_HEADPHONE_A_VOL   0x22
#define CS43L22_REG_HEADPHONE_B_VOL   0x23
#define CS43L22_REG_SPEAKER_A_VOL     0x24
#define CS43L22_REG_SPEAKER_B_VOL     0x25
#define CS43L22_REG_CH_MIXER_SWAP     0x26
#define CS43L22_REG_LIMIT_CTL1        0x27
#define CS43L22_REG_LIMIT_CTL2        0x28
#define CS43L22_REG_LIMIT_ATTACK_RATE 0x29
#define CS43L22_REG_OVF_CLK_STATUS    0x2E
#define CS43L22_REG_BATT_COMPENSATION 0x2F
#define CS43L22_REG_VP_BATTERY_LEVEL  0x30
#define CS43L22_REG_SPEAKER_STATUS    0x31
#define CS43L22_REG_TEMPMONITOR_CTL   0x32
#define CS43L22_REG_THERMAL_FOLDBACK  0x33
#define CS43L22_REG_CHARGE_PUMP_FREQ  0x34

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/**
 * @brief  CS43L22 ID
 */
#define CS43L22_ID      0xE0
#define CS43L22_ID_MASK 0xF8
/**
 * @brief Chip ID Register: Chip I.D. and Revision Register
 *  Read only register
 *  Default value: 0x01
 *  [7:3] CHIPID[4:0]: I.D. code for the CS43L22.
 *        Default value: 11100b
 *  [2:0] REVID[2:0]: CS43L22 revision level.
 *        Default value:
 *        000 - Rev A0
 *        001 - Rev A1
 *        010 - Rev B0
 *        011 - Rev B1
 */
#define CS43L22_CHIPID_ADDR 0x01

#define VOLUME_CONVERT(Volume) (((Volume) > 100) ? 255 : ((uint8_t)(((Volume)*255) / 100)))

static uint8_t Is_cs43l22_Stop = 1;
static uint8_t OutputDev       = 0;

static os_uint16_t cs43l22_read_reg(struct os_i2c_client *i2c, os_uint16_t reg)
{
    return os_i2c_client_read_byte(i2c, reg, 1);
}

static os_uint8_t cs43l22_write_reg(struct os_i2c_client *i2c, os_uint16_t reg, os_uint8_t val)
{
    int ret;

    ret = os_i2c_client_write_byte(i2c, reg, 1, val);

    if (ret != 0)
        return (os_uint8_t)-1;

    if (val != cs43l22_read_reg(i2c, reg))
        return (os_uint8_t)-2;

    return 0;
}

/**
 * @brief Sets higher or lower the codec volume level.
 * @param cs43l22: Device address on communication Bus.
 * @param Volume: a byte value from 0 to 255 (refer to codec registers
 *                description for more details).
 *
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_set_volume(struct cs43l22_dev *cs43l22, uint8_t Volume)
{
    uint32_t counter      = 0;
    uint8_t  convertedvol = VOLUME_CONVERT(Volume);

    /* Set the Master volume */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_MASTER_A_VOL, convertedvol + 0x19);
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_MASTER_B_VOL, convertedvol + 0x19);

    return counter;
}

/**
 * @brief Sets new frequency.
 * @param cs43l22: Device address on communication Bus.
 * @param AudioFreq: Audio frequency used to play the audio stream.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_set_frequency(struct cs43l22_dev *cs43l22, uint32_t AudioFreq)
{
    return 0;
}

/**
 * @brief Enables or disables the mute feature on the audio codec.
 * @param cs43l22: Device address on communication Bus.
 * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
 *             mute mode.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_set_mute(struct cs43l22_dev *cs43l22, uint32_t Cmd)
{
    uint32_t counter = 0;

    /* Set the Mute mode */
    if (Cmd == AUDIO_MUTE_ON)
    {
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0xFF);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_HEADPHONE_A_VOL, 0x01);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_HEADPHONE_B_VOL, 0x01);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_A_VOL, 0x01);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_B_VOL, 0x01);
    }
    else /* AUDIO_MUTE_OFF Disable the Mute */
    {
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_HEADPHONE_A_VOL, 0x00);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_HEADPHONE_B_VOL, 0x00);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_A_VOL, 0x00);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_B_VOL, 0x00);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, OutputDev);
    }    
    
    return counter;
}

/**
 * @brief Switch dynamically (while audio file is played) the output target
 *         (speaker or headphone).
 * @note This function modifies a global variable of the audio codec driver: OutputDev.
 * @param cs43l22: Device address on communication Bus.
 * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
 *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_set_output_mode(struct cs43l22_dev *cs43l22, uint8_t Output)
{
    uint32_t counter = 0;

    switch (Output)
    {
    case OUTPUT_DEVICE_SPEAKER:
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0xFA); /* SPK always ON & HP always OFF */
        OutputDev = 0xFA;
        break;

    case OUTPUT_DEVICE_HEADPHONE:
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0xAF); /* SPK always OFF & HP always ON */
        OutputDev = 0xAF;
        break;

    case OUTPUT_DEVICE_BOTH:
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0xAA); /* SPK always ON & HP always ON */
        OutputDev = 0xAA;
        break;

    case OUTPUT_DEVICE_AUTO:
        counter +=
            cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0x05); /* Detect the HP or the SPK automatically */
        OutputDev = 0x05;
        break;

    default:
        counter +=
            cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, 0x05); /* Detect the HP or the SPK automatically */
        OutputDev = 0x05;
        break;
    }
    return counter;
}

/**
 * @brief  Get the CS43L22 ID.
 * @param cs43l22: Device address on communication Bus.
 * @retval The CS43L22 ID
 */
uint32_t cs43l22_read_id(struct cs43l22_dev *cs43l22)
{
    uint8_t Value;

    Value = cs43l22_read_reg(&cs43l22->i2c, CS43L22_CHIPID_ADDR);
    Value = (Value & CS43L22_ID_MASK);

    return ((uint32_t)Value);
}

/**
 * @brief Start the audio Codec play feature.
 * @note For this codec no Play options are required.
 * @param cs43l22: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_play(struct cs43l22_dev *cs43l22, uint16_t *pBuffer, uint16_t Size)
{
    uint32_t counter = 0;

    if (Is_cs43l22_Stop == 1)
    {
        /* Enable the digital soft ramp */
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_MISC_CTL, 0x06);

        /* Enable Output device */
        counter += cs43l22_set_mute(cs43l22, AUDIO_MUTE_OFF);

        /* Power on the Codec */
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL1, 0x9E);
        Is_cs43l22_Stop = 0;
    }

    /* Return communication control value */
    return counter;
}

/**
 * @brief Pauses playing on the audio codec.
 * @param cs43l22: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_pause(struct cs43l22_dev *cs43l22)
{
    uint32_t counter = 0;

    /* Pause the audio file playing */
    /* Mute the output first */
    counter += cs43l22_set_mute(cs43l22, AUDIO_MUTE_ON);

    /* Put the Codec in Power save mode */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL1, 0x01);

    return counter;
}

/**
 * @brief Resumes playing on the audio codec.
 * @param cs43l22: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_resume(struct cs43l22_dev *cs43l22)
{
    uint32_t          counter = 0;
    volatile uint32_t index   = 0x00;
    /* Resumes the audio file playing */
    /* Unmute the output first */
    counter += cs43l22_set_mute(cs43l22, AUDIO_MUTE_OFF);

    for (index = 0x00; index < 0xFF; index++)
        ;

    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, OutputDev);

    /* Exit the Power save mode */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL1, 0x9E);

    return counter;
}

/**
 * @brief Stops audio Codec playing. It powers down the codec.
 * @param cs43l22: Device address on communication Bus.
 * @param CodecPdwnMode: selects the  power down mode.
 *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
 *                           mode, the codec is set to default configuration
 *                           (user should re-Initialize the codec in order to
 *                            play again the audio stream).
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_stop(struct cs43l22_dev *cs43l22, uint32_t CodecPdwnMode)
{
    uint32_t counter = 0;

    /* Mute the output first */
    counter += cs43l22_set_mute(cs43l22, AUDIO_MUTE_ON);

    /* Disable the digital soft ramp */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_MISC_CTL, 0x04);

    /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL1, 0x9F);

    Is_cs43l22_Stop = 1;
    return counter;
}

/**
 * @brief Resets cs43l22 registers.
 * @param cs43l22: Device address on communication Bus.
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_reset(struct cs43l22_dev *cs43l22)
{
    os_pin_write(cs43l22->rst_pin, PIN_LOW);
    os_task_msleep(20);
    os_pin_write(cs43l22->rst_pin, PIN_HIGH);

    return 0;
}

/**
 * @brief Initializes the audio codec and the control interface.
 * @param cs43l22: Device address on communication Bus.
 * @param OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
 *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
 * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @retval 0 if correct communication, else wrong communication
 */
uint32_t cs43l22_init(struct cs43l22_dev *cs43l22, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    uint32_t counter = 0;
    uint32_t id;

    os_pin_mode(cs43l22->rst_pin, PIN_MODE_OUTPUT);

    cs43l22_reset(cs43l22);

    id = cs43l22_read_id(cs43l22);
    if (id != CS43L22_ID)
    {
        LOG_E(DBG_EXT_TAG, "cs43l22 id: %02x invalid, expect: %02x", id, CS43L22_ID);
        return (uint32_t)-1;
    }

    /* Keep Codec powered OFF */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL1, 0x01);

    /*Save Output device for mute ON/OFF procedure*/
    switch (OutputDevice)
    {
    case OUTPUT_DEVICE_SPEAKER:
        OutputDev = 0xFA;
        break;

    case OUTPUT_DEVICE_HEADPHONE:
        OutputDev = 0xAF;
        break;

    case OUTPUT_DEVICE_BOTH:
        OutputDev = 0xAA;
        break;

    case OUTPUT_DEVICE_AUTO:
        OutputDev = 0x05;
        break;

    default:
        OutputDev = 0x05;
        break;
    }

    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_POWER_CTL2, OutputDev);

    /* Clock configuration: Auto detection */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_CLOCKING_CTL, 0x81);

    /* Set the Slave Mode and the audio Standard */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_INTERFACE_CTL1, CODEC_STANDARD);

    /* Set the Master volume */
    counter += cs43l22_set_volume(cs43l22, Volume);

    /* If the Speaker is enabled, set the Mono mode and volume attenuation level */
    if (OutputDevice != OUTPUT_DEVICE_HEADPHONE)
    {
        /* Set the Speaker Mono mode */
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_PLAYBACK_CTL2, 0x06);

        /* Set the Speaker attenuation level */
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_A_VOL, 0x00);
        counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_SPEAKER_B_VOL, 0x00);
    }

    /* Additional configuration for the CODEC. These configurations are done to reduce
    the time needed for the Codec to power off. If these configurations are removed,
    then a long delay should be added between powering off the Codec and switching
    off the I2S peripheral MCLK clock (which is the operating clock for Codec).
    If this delay is not inserted, then the codec will not shut down properly and
    it results in high noise after shut down. */

    /* Disable the analog soft ramp */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_ANALOG_ZC_SR_SETT, 0x00);
    /* Disable the digital soft ramp */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_MISC_CTL, 0x04);
    /* Disable the limiter attack level */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_LIMIT_CTL1, 0x00);
    /* Adjust Bass and Treble levels */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_TONE_CTL, 0x0F);
    /* Adjust PCM volume level */
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_PCMA_VOL, 0x0A);
    counter += cs43l22_write_reg(&cs43l22->i2c, CS43L22_REG_PCMB_VOL, 0x0A);

    if (counter == 0)
    {
        LOG_I(DBG_EXT_TAG, "cs43l22 init success");
    }

    /* Return communication control value */
    return counter;
}

/**
 * @brief  Deinitializes the audio codec.
 * @param  None
 * @retval  None
 */
void cs43l22_deInit(void)
{
}

/* CS43L22 Device Driver Interface */

static os_err_t audio_cs43l22_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t                      result = OS_EOK;
    struct cs43l22_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;

    switch (caps->config_type)
    {
    case AUDIO_VOLUME_CMD:
    {
        os_uint8_t volume = caps->udata.value;

        volume *= 2;

        cs43l22_set_volume(&aduio_dev->cs43l22, volume);

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

static os_err_t audio_cs43l22_init(struct os_audio_device *audio)
{
    os_err_t                      result = OS_EOK;
    struct cs43l22_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);

    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;

    cs43l22_init(&aduio_dev->cs43l22, OUTPUT_DEVICE_HEADPHONE, 70, 0);

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_cs43l22_start(struct os_audio_device *audio)
{
    struct cs43l22_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;

    cs43l22_play(&aduio_dev->cs43l22, OS_NULL, 0);

    LOG_D(DBG_EXT_TAG, "open sound device");
	os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_ENABLE, OS_NULL);

    return OS_EOK;
}

static os_err_t audio_cs43l22_stop(struct os_audio_device *audio)
{
    struct cs43l22_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_DISABLE, OS_NULL);

    cs43l22_stop(&aduio_dev->cs43l22, 0);
    audio_cs43l22_init(audio);

    LOG_D(DBG_EXT_TAG, "close sound device");

    return OS_EOK;
}

os_size_t audio_cs43l22_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct cs43l22_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;

    return os_device_write_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

os_size_t audio_cs43l22_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct cs43l22_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct cs43l22_player_device *)audio->parent.user_data;
    
    return os_device_read_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)readBuf, size);
}

os_err_t audio_cs43l22_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct cs43l22_player_device *cs43l22_player_dev = dev->user_data;

        os_device_send_notify(&cs43l22_player_dev->audio.parent);

        return OS_EOK;
    }
    
    return OS_ENOSYS;
}

static struct os_audio_ops cs43l22_player_ops = {
    .getcaps            = OS_NULL,
    .configure          = audio_cs43l22_config,
    .init               = audio_cs43l22_init,
    .start              = audio_cs43l22_start,
    .stop               = audio_cs43l22_stop,
    .transmit           = audio_cs43l22_transmit, 
    .receive            = audio_cs43l22_receive,
};

int os_hw_audio_player_init(void)
{
    struct cs43l22_player_device *cs43l22_player;
    struct cs43l22_dev *cs43l22;

    cs43l22_player = os_calloc(1, sizeof(struct cs43l22_player_device));
    OS_ASSERT(cs43l22_player != OS_NULL);

    cs43l22 = &cs43l22_player->cs43l22;

    cs43l22_player->replay_config.samplerate = 44100;
    cs43l22_player->replay_config.channels   = 2;
    cs43l22_player->replay_config.samplebits = 16;
    cs43l22_player->volume                   = 50;

    cs43l22_player->audio.ops = &cs43l22_player_ops;

    cs43l22_player->cfg_bus = os_device_find(BSP_CS43L22_I2C_BUS);
    if (cs43l22_player->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    cs43l22_player->data_bus = os_device_find(BSP_AUDIO_DATA_TX_BUS);
    if (cs43l22_player->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    cs43l22_player->data_bus->user_data = cs43l22_player;
    os_device_open(cs43l22_player->data_bus);
    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_cs43l22_data_tx_done;
    os_device_control(cs43l22_player->data_bus, OS_DEVICE_CTRL_SET_CB, info);
    
    cs43l22->i2c.bus = (struct os_i2c_bus_device *)cs43l22_player->cfg_bus;

    cs43l22->i2c.client_addr = BSP_CS43L22_I2C_ADDR;
    cs43l22->rst_pin         = BSP_CS43L22_RST_PIN;

    os_audio_player_register(&cs43l22_player->audio, "audio0", cs43l22_player);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init, OS_INIT_SUBLEVEL_LOW);
