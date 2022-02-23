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
 * @file        drv_adc.c
 *
 * @brief       This file implements adc driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <string.h>
#include "am_mcu_apollo.h"

#ifdef OS_USING_ADC

static unsigned int adc_value[ADC_CHANNEL_NUM];

static os_err_t am_adc_enabled(struct os_adc_device *device, os_uint32_t channel, os_bool_t enabled)
{
    if (enabled == OS_TRUE)
        am_hal_adc_enable();
    else
        am_hal_adc_disable();

    return OS_EOK;
}

static os_err_t am_adc_convert(struct os_adc_device *device, os_uint32_t channel, os_uint32_t *value)
{
    if (channel >= ADC_CHANNEL_NUM)
        return OS_ERROR;

    *value = adc_value[channel];
    return OS_EOK;
}

static struct os_adc_ops am_adc_ops =
{
    am_adc_enabled,
    am_adc_convert,
};

static struct os_adc_device am_adc;

/* ADC Configuration */
static am_hal_adc_config_t g_sADC_CfgA =
{
    /* Select the ADC Clock source using one of the clock source macros. */
    AM_HAL_ADC_CLOCK_1_5MHZ,

    /* Select the ADC trigger source using a trigger source macro. */
    AM_HAL_ADC_TRIGGER_SOFT,

    /* Use a macro to select the ADC reference voltage. */
    AM_HAL_ADC_REF_INT,

    /* Use a macro to choose a maximum sample rate setting. */
    AM_HAL_ADC_MODE_1MSPS,

    /* Use a macro to choose the power mode for the ADC's idle state. */
    AM_HAL_ADC_LPMODE_2,

    /* Use the Repeat macro to enable repeating samples using Timer3A. */
    AM_HAL_ADC_REPEAT,

    /* Power Off the temp sensor. */
    AM_HAL_ADC_PON_TEMP,

    /* Set the ADC window limits using the window limit macro. */
    AM_HAL_ADC_WINDOW(768, 256) /* arbitrary window setting, not used here. */
};

/* Timer configurations */
am_hal_ctimer_config_t g_sTimer3 =
{
    /* do not link A and B together to make a long 32-bit counter. */
    0,

    /* Set up timer 3A to drive the ADC */
    (AM_HAL_CTIMER_FN_PWM_REPEAT | AM_HAL_CTIMER_LFRC_32HZ),

    /* Timer 3B is not used in this example. */
    0,
};

void am_adc_isr(void)
{
    am_hal_adc_fifo_read_t fifo_info;

    /* Clear ADC Interrupt (write to clear). */
    AM_REGn(CTIMER, 0, INTCLR) = AM_REG_CTIMER_INTCLR_CTMRA0INT_M;

    /* Keep grabbing value from the ADC FIFO until it goes empty. */
    while (am_hal_adc_fifo_read(&fifo_info))
    {
        if (fifo_info.ui8Slot >= ADC_CHANNEL_NUM)
            continue;

        adc_value[fifo_info.ui8Slot] = fifo_info.ui16Data;

#if 0
        os_kprintf("<%u> adc isr:%d --> %u\n", (unsigned int)rt_tick_get(), 
            (int)fifo_info.ui8Slot, (unsigned int)fifo_info.ui16Data);
#endif
    }
}

static void adc_io_init(void)
{
#ifdef ADC_CHANNEL0_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL0_GPIO, ADC_CHANNEL0_PIN);
#endif

#ifdef ADC_CHANNEL1_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL1_GPIO, ADC_CHANNEL1_PIN);
#endif

#ifdef ADC_CHANNEL2_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL2_GPIO, ADC_CHANNEL2_PIN);
#endif

#ifdef ADC_CHANNEL3_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL3_GPIO, ADC_CHANNEL3_PIN);
#endif

#ifdef ADC_CHANNEL4_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL4_GPIO, ADC_CHANNEL4_PIN);
#endif

#ifdef ADC_CHANNEL5_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL5_GPIO, ADC_CHANNEL5_PIN);
#endif

#ifdef ADC_CHANNEL6_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL6_GPIO, ADC_CHANNEL6_PIN);
#endif

#ifdef ADC_CHANNEL7_GPIO
    am_hal_gpio_pin_config(ADC_CHANNEL7_GPIO, ADC_CHANNEL7_PIN);
#endif
}

void adc_init(void)
{
#if AM_PART_APOLLO

    /* We MUST turn on band gap to use the Temp Sensor. */
    /* The ADC hardware in mode 2 will cycle the power to the bandgap automatically. */
    am_hal_mcuctrl_bandgap_enable();
#endif

#if AM_PART_APOLLO2
    /* Enable the ADC power domain. */
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);
#endif

    /* Configure the ADC. */
    am_hal_adc_config(&g_sADC_CfgA);

    /* Initialize the slot control registers. */
#ifdef ADC_CHANNEL0_CFG
    am_hal_adc_slot_config(0, ADC_CHANNEL0_CFG);
#else
    am_hal_adc_slot_config(0, 0);
#endif

#ifdef ADC_CHANNEL1_CFG
    am_hal_adc_slot_config(1, ADC_CHANNEL1_CFG);
#else
    am_hal_adc_slot_config(1, 0);
#endif

#ifdef ADC_CHANNEL2_CFG
    am_hal_adc_slot_config(2, ADC_CHANNEL2_CFG);
#else
    am_hal_adc_slot_config(2, 0);
#endif

#ifdef ADC_CHANNEL3_CFG
    am_hal_adc_slot_config(3, ADC_CHANNEL3_CFG);
#else
    am_hal_adc_slot_config(3, 0);
#endif

#ifdef ADC_CHANNEL4_CFG
    am_hal_adc_slot_config(4, ADC_CHANNEL4_CFG);
#else
    am_hal_adc_slot_config(4, 0);
#endif

#ifdef ADC_CHANNEL5_CFG
    am_hal_adc_slot_config(5, ADC_CHANNEL5_CFG);
#else
    am_hal_adc_slot_config(5, 0);
#endif

#ifdef ADC_CHANNEL6_CFG
    am_hal_adc_slot_config(6, ADC_CHANNEL6_CFG);
#else
    am_hal_adc_slot_config(6, 0);
#endif

#ifdef ADC_CHANNEL7_CFG
    am_hal_adc_slot_config(7, ADC_CHANNEL7_CFG);
#else
    am_hal_adc_slot_config(7, 0);
#endif

    /* Enable the ADC. */
    am_hal_adc_enable();
}

void adc_init_timer3A(void)
{
    uint32_t ui32Period = 2000;    // Set for 2 second (2000ms) period

    /* LFRC has to be turned on for this example because we are running this
       timer off of the LFRC. */
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

    /* Set up timer 3A so start by clearing it. */
    am_hal_ctimer_clear(3, AM_HAL_CTIMER_TIMERA);

    /* Configure the timer to count 32Hz LFRC clocks but don't start it yet. */
    am_hal_ctimer_config(3, &g_sTimer3);

    /* Compute CMPR value needed for desired period based on a 32HZ clock. */
    ui32Period = ui32Period * 32 / 1000;
    am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, ui32Period, (ui32Period >> 1));

    /* Enable the timer output "pin". This refers to the pin as seen from
       inside the timer. The actual GPIO pin is neither enabled nor driven. */
    am_hal_ctimer_pin_enable(3, AM_HAL_CTIMER_TIMERA);

    /* Set up timer 3A as the trigger source for the ADC. */
    am_hal_ctimer_adc_trigger_enable();

    /* Start timer 3A. */
    am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
}

int os_hw_adc_init(void)
{
    memset(adc_value, 0, sizeof(adc_value));

    adc_init_timer3A();
    adc_io_init();
    adc_init();
    am_hal_adc_int_enable(AM_REG_ADC_INTEN_WCINC(1)     |
                          AM_REG_ADC_INTEN_WCEXC(1)     |
                          AM_REG_ADC_INTEN_FIFOOVR2(1)  |
                          AM_REG_ADC_INTEN_FIFOOVR1(1)  |
                          AM_REG_ADC_INTEN_SCNCMP(1)    |
                          AM_REG_ADC_INTEN_CNVCMP(1));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_ADC);
    am_hal_adc_trigger();

    os_hw_adc_register(&am_adc, "adc", &am_adc_ops, 0);

    os_kprintf("adc_init!\n");

    return 0;
}

#endif
