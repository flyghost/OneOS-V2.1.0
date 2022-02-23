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
 * @file        drv_pwm.c
 *
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include <board.h>

#ifdef OS_USING_PWM
#include "drv_pwm.h"
#include <nrfx_pwm.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define LOG_TAG "drv.pwm"
#include <drv_log.h>

#define NRFX_PWM_PIN_INVERTED    0x80
#define _PRIO_PWM    6
#define PWM_CHANNEL_NUM_MAX (3)
#define PWM_PERIOD_MAX (262140000)//1/125khz*0xffff/2

struct nrf52_pwm
{
    struct os_pwm_device pwm_device;

    nrf_pwm_values_individual_t m_demo1_seq_values;
    nrf_pwm_sequence_t m_demo1_seq;
    nrfx_pwm_t *pwm_handle;

    char *name;
    os_uint8_t channel;
    os_uint64_t pwm_src_clk;
    uint32_t pwm_period;
    uint32_t pwm_pulse;
    uint8_t active_pin;
    uint8_t channel_0_pin;
    uint8_t channel_1_pin;
    uint8_t channel_2_pin;
    uint8_t channel_3_pin;
};

enum
{
#ifdef BSP_USING_PWM0
    PWM0_INDEX,
#endif
#ifdef BSP_USING_PWM1
    PWM1_INDEX,
#endif
#ifdef BSP_USING_PWM2
    PWM2_INDEX,
#endif
#ifdef BSP_USING_PWM3
    PWM3_INDEX,
#endif
};
#ifdef BSP_USING_PWM0
static nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);
#define PWM0_CONFIG                             \
    {                                           \
       .pwm_handle              =  &m_pwm0,     \
       .name                    = "pwm0",       \
       .pwm_src_clk             = 125000,      \
    }
#endif

#ifdef BSP_USING_PWM1
static nrfx_pwm_t m_pwm1 = NRFX_PWM_INSTANCE(1);
#define PWM1_CONFIG                             \
    {                                           \
       .pwm_handle              =  &m_pwm1,         \
       .name                    = "pwm1",       \
       .pwm_src_clk             = 125000,             \
    }
#endif

#ifdef BSP_USING_PWM2
static nrfx_pwm_t m_pwm2 = NRFX_PWM_INSTANCE(2);
#define PWM2_CONFIG                             \
    {                                           \
       .pwm_handle              =  &m_pwm2,         \
       .name                    = "pwm2",       \
       .pwm_src_clk             = 125000,             \
    }
#endif

#ifdef BSP_USING_PWM3
static nrfx_pwm_t m_pwm3 = NRFX_PWM_INSTANCE(3);
#define PWM3_CONFIG                             \
    {                                           \
       .pwm_handle              =  &m_pwm3,         \
       .name                    = "pwm3",       \
       .pwm_src_clk             = 125000,             \
    }
#endif

static struct nrf52_pwm nrf_pwm_obj[] =
{
#ifdef BSP_USING_PWM0
    PWM0_CONFIG,
#endif
#ifdef BSP_USING_PWM1
    PWM1_CONFIG,
#endif

#ifdef BSP_USING_PWM2
    PWM2_CONFIG,
#endif

#ifdef BSP_USING_PWM3
    PWM3_CONFIG,
#endif
};

static void nrf52_get_channel(void)
{
#ifdef BSP_USING_PWM0_CH0
        nrf_pwm_obj[PWM0_INDEX].channel |= 1 << 0;
        nrf_pwm_obj[PWM0_INDEX].channel_0_pin = BSP_USING_PWM0_CH0;
#endif
#ifdef BSP_USING_PWM0_CH1
        nrf_pwm_obj[PWM0_INDEX].channel |= 1 << 1;
        nrf_pwm_obj[PWM0_INDEX].channel_1_pin = BSP_USING_PWM0_CH1;
#endif
#ifdef BSP_USING_PWM0_CH2
        nrf_pwm_obj[PWM0_INDEX].channel |= 1 << 2;
        nrf_pwm_obj[PWM0_INDEX].channel_2_pin = BSP_USING_PWM0_CH2;
#endif
#ifdef BSP_USING_PWM0_CH3
        nrf_pwm_obj[PWM0_INDEX].channel |= 1 << 3;
        nrf_pwm_obj[PWM0_INDEX].channel_3_pin = BSP_USING_PWM0_CH3;
#endif
#ifdef BSP_USING_PWM1_CH0
        nrf_pwm_obj[PWM1_INDEX].channel |= 1 << 0;
        nrf_pwm_obj[PWM1_INDEX].channel_0_pin = BSP_USING_PWM1_CH0;
#endif
#ifdef BSP_USING_PWM1_CH1
        nrf_pwm_obj[PWM1_INDEX].channel |= 1 << 1;
        nrf_pwm_obj[PWM1_INDEX].channel_1_pin = BSP_USING_PWM1_CH1;
#endif
#ifdef BSP_USING_PWM1_CH2
        nrf_pwm_obj[PWM1_INDEX].channel |= 1 << 2;
        nrf_pwm_obj[PWM1_INDEX].channel_2_pin = BSP_USING_PWM1_CH2;
#endif
#ifdef BSP_USING_PWM1_CH3
        nrf_pwm_obj[PWM1_INDEX].channel |= 1 << 3;
        nrf_pwm_obj[PWM1_INDEX].channel_3_pin = BSP_USING_PWM1_CH3;
#endif
#ifdef BSP_USING_PWM2_CH0
        nrf_pwm_obj[PWM2_INDEX].channel |= 1 << 0;
        nrf_pwm_obj[PWM2_INDEX].channel_0_pin = BSP_USING_PWM2_CH0;
#endif
#ifdef BSP_USING_PWM2_CH1
        nrf_pwm_obj[PWM2_INDEX].channel |= 1 << 1;
        nrf_pwm_obj[PWM2_INDEX].channel_1_pin = BSP_USING_PWM2_CH1;
#endif
#ifdef BSP_USING_PWM2_CH2
        nrf_pwm_obj[PWM2_INDEX].channel |= 1 << 2;
        nrf_pwm_obj[PWM2_INDEX].channel_2_pin = BSP_USING_PWM2_CH2;
#endif
#ifdef BSP_USING_PWM2_CH3
        nrf_pwm_obj[PWM2_INDEX].channel |= 1 << 3;
        nrf_pwm_obj[PWM2_INDEX].channel_3_pin = BSP_USING_PWM2_CH3;
#endif
#ifdef BSP_USING_PWM3_CH0
        nrf_pwm_obj[PWM3_INDEX].channel |= 1 << 0;
        nrf_pwm_obj[PWM3_INDEX].channel_0_pin = BSP_USING_PWM3_CH0;
#endif
#ifdef BSP_USING_PWM3_CH1
        nrf_pwm_obj[PWM3_INDEX].channel |= 1 << 1;
        nrf_pwm_obj[PWM3_INDEX].channel_1_pin = BSP_USING_PWM3_CH1;
#endif
#ifdef BSP_USING_PWM3_CH2
        nrf_pwm_obj[PWM3_INDEX].channel |= 1 << 2;
        nrf_pwm_obj[PWM3_INDEX].channel_2_pin = BSP_USING_PWM3_CH2;
#endif
#ifdef BSP_USING_PWM3_CH3
        nrf_pwm_obj[PWM3_INDEX].channel |= 1 << 3;
        nrf_pwm_obj[PWM3_INDEX].channel_3_pin = BSP_USING_PWM3_CH3;
#endif

}

static os_err_t nrf52_pwm_enabled(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable)
{
    struct nrf52_pwm *nrf_pwm;
    
    nrf_pwm = os_container_of(dev, struct nrf52_pwm, pwm_device);
    
    if (!enable)
    {
        nrf_gpio_pin_write(nrf_pwm->active_pin, 0);
        while(nrfx_pwm_stop(nrf_pwm->pwm_handle, false) != true);
    }
    else
    {
        if(nrf_pwm->pwm_period > PWM_PERIOD_MAX || nrf_pwm->pwm_pulse > nrf_pwm->pwm_period)
        {
            return OS_ERROR;
        }
            
        if ((nrf_pwm->pwm_pulse == nrf_pwm->pwm_period) && (nrf_pwm->pwm_period != 0))
            nrf_gpio_pin_write(nrf_pwm->active_pin, 1);
        else if(nrf_pwm->pwm_pulse == 0)
            nrf_gpio_pin_write(nrf_pwm->active_pin, 0);
        else
            nrfx_pwm_simple_playback(nrf_pwm->pwm_handle, &nrf_pwm->m_demo1_seq, 1, NRFX_PWM_FLAG_LOOP);
    }

    return OS_EOK;
}

static os_err_t nrf52_pwm_set_period(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t period_ns)
{
    struct nrf52_pwm *nrf_pwm;

    nrf_pwm = os_container_of(dev, struct nrf52_pwm, pwm_device);

    if(channel > PWM_CHANNEL_NUM_MAX)
    {
        LOG_EXT_E("invalid pwm channel!\n");
        return OS_ERROR;
    }
    
    if(period_ns > PWM_PERIOD_MAX)
    {
        LOG_EXT_E("pwm period value over range, max value %d!\n", PWM_PERIOD_MAX);
        return OS_ERROR;
    }

    nrf_pwm->pwm_period = period_ns;
    nrf_pwm->pwm_handle->p_registers->COUNTERTOP = (period_ns / 1000 * nrf_pwm->pwm_src_clk / 1000000UL);//us

    return OS_EOK;
}

static os_err_t nrf52_pwm_set_pulse(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t pulse_ns)
{
    struct nrf52_pwm *nrf_pwm;
    uint32_t pulse;
    
    nrf_pwm = os_container_of(dev, struct nrf52_pwm, pwm_device);

    if(channel > PWM_CHANNEL_NUM_MAX)
    {
        LOG_EXT_E("invalid pwm channel!\n");
        return OS_ERROR;
    }
        
    if (pulse_ns > nrf_pwm->pwm_period)
    {
        LOG_EXT_E("pwm pulse value over range!\n");
        return OS_ERROR;
    }
    
    nrf_pwm->pwm_pulse = pulse_ns;
    pulse_ns = nrf_pwm->pwm_period - pulse_ns;
    pulse = (pulse_ns / 1000 * nrf_pwm->pwm_src_clk / 1000000UL);//us

    if (channel == 0)
    {
        nrf_pwm->m_demo1_seq_values.channel_0 = pulse;
        nrf_pwm->active_pin = nrf_pwm->channel_0_pin;
    }

    if (channel == 1)
    {
        nrf_pwm->m_demo1_seq_values.channel_1 = pulse;
        nrf_pwm->active_pin = nrf_pwm->channel_1_pin;
    }

    if (channel == 2)
    {
        nrf_pwm->m_demo1_seq_values.channel_2 = pulse;
        nrf_pwm->active_pin = nrf_pwm->channel_2_pin;
    }

    if (channel == 3)
    {
        nrf_pwm->m_demo1_seq_values.channel_3 = pulse;
        nrf_pwm->active_pin = nrf_pwm->channel_3_pin;
    }
    
    return OS_EOK;
}

static const struct os_pwm_ops nrf52_pwm_ops =
{
    .enabled = nrf52_pwm_enabled,
    .set_period = nrf52_pwm_set_period,
    .set_pulse = nrf52_pwm_set_pulse,
    .control  = OS_NULL,
};

void nrf52_pwm_init(void)
{
    int i = 0;

    nrf52_get_channel();

    nrfx_pwm_config_t config0 =
    {
        .irq_priority = _PRIO_PWM,
        .base_clock   = NRF_PWM_CLK_125kHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 65535,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    for (i = 0; i < sizeof(nrf_pwm_obj) / sizeof(nrf_pwm_obj[0]); i++)
    {
        if (nrf_pwm_obj[i].pwm_src_clk == 125000)
        {
            config0.base_clock = NRF_PWM_CLK_125kHz;
        }
        else if (nrf_pwm_obj[i].pwm_src_clk == 1000000)
        {
            config0.base_clock = NRF_PWM_CLK_1MHz;
        }
        else if (nrf_pwm_obj[i].pwm_src_clk == 2000000)
        {
            config0.base_clock = NRF_PWM_CLK_2MHz;
        }
        else if (nrf_pwm_obj[i].pwm_src_clk == 8000000)
        {
            config0.base_clock = NRF_PWM_CLK_8MHz;
        }
        else
        {
            config0.base_clock = NRF_PWM_CLK_1MHz;
        }

        if (nrf_pwm_obj[i].channel & 0x01)
        {
            config0.output_pins[0] = nrf_pwm_obj[i].channel_0_pin | NRFX_PWM_PIN_INVERTED;
        }

        if (nrf_pwm_obj[i].channel & 0x02)
        {
            config0.output_pins[1] = nrf_pwm_obj[i].channel_1_pin | NRFX_PWM_PIN_INVERTED;
        }

        if (nrf_pwm_obj[i].channel & 0x04)
        {
            config0.output_pins[2] = nrf_pwm_obj[i].channel_2_pin | NRFX_PWM_PIN_INVERTED;
        }

        if (nrf_pwm_obj[i].channel & 0x08)
        {
            config0.output_pins[3] = nrf_pwm_obj[i].channel_3_pin | NRFX_PWM_PIN_INVERTED;
        }

        nrf_pwm_obj[i].m_demo1_seq.values.p_individual = &nrf_pwm_obj[i].m_demo1_seq_values;
        nrf_pwm_obj[i].m_demo1_seq.length = NRF_PWM_VALUES_LENGTH(nrf_pwm_obj[i].m_demo1_seq_values),
                            nrfx_pwm_init(nrf_pwm_obj[i].pwm_handle, &config0, NULL);

        /* register pwm device */
        nrf_pwm_obj[i].pwm_device.ops = &nrf52_pwm_ops;
        if (os_device_pwm_register(&nrf_pwm_obj[i].pwm_device, nrf_pwm_obj[i].name) == OS_EOK)
        {
                os_kprintf("\r\n %s register success", nrf_pwm_obj[i].name);
        }
        else
        {
                os_kprintf("\r\n %s register failed", nrf_pwm_obj[i].name);
        } 
    }
}

#endif

