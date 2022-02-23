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
 * @file        drv_pdm.c
 *
 * @brief       This file implements pdm driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

#ifdef OS_USING_PDM

/* messagequeue define */
struct os_messagequeue pdm_mq;

static os_uint8_t am_pdm_buffer_pool[1024];

#define NWA_FRAME_SAMPLES  160 /* 8k, 16bit, mono audio data */
#define PDM_FIFO_THRESHOLD NWA_FRAME_SAMPLES

#define PDM_GPIO_CLK      22
#define PDM_GPIO_CFG_CLK  AM_HAL_PIN_22_PDM_CLK
#define PDM_GPIO_DATA     23
#define PDM_GPIO_CFG_DATA AM_HAL_PIN_23_PDM_DATA

static am_hal_pdm_config_t g_sPDMConfig = {
    AM_HAL_PDM_PCFG_LRSWAP_DISABLE | AM_HAL_PDM_PCFG_RIGHT_PGA_0DB | AM_HAL_PDM_PCFG_LEFT_PGA_0DB |
        AM_HAL_PDM_PCFG_MCLKDIV_DIV1 | AM_HAL_PDM_PCFG_SINC_RATE(48) | AM_HAL_PDM_PCFG_ADCHPD_ENABLE |
        AM_HAL_PDM_PCFG_HPCUTOFF(0x1) | AM_HAL_PDM_PCFG_CYCLES(0x1) | AM_HAL_PDM_PCFG_SOFTMUTE_DISABLE |
        AM_HAL_PDM_PCFG_PDMCORE_ENABLE, /* Set the PDM configuration */
    AM_HAL_PDM_IOCLK_750KHZ | AM_HAL_PDM_VCFG_RSTB_NORMAL | AM_HAL_PDM_VCFG_PDMCLK_ENABLE |
        AM_HAL_PDM_VCFG_I2SMODE_DISABLE | AM_HAL_PDM_VCFG_BCLKINV_DISABLE | AM_HAL_PDM_VCFG_DMICDEL_DISABLE |
        AM_HAL_PDM_VCFG_SELAP_INTERNAL | AM_HAL_PDM_VCFG_PACK_DISABLE |
        AM_HAL_PDM_VCFG_CHANNEL_LEFT, /* Set the Voice Configuration */
    PDM_FIFO_THRESHOLD,               /* Select the FIFO PCM sample threshold 0~256 */
};

os_uint8_t am_pdm_data_get(os_uint8_t *buff, os_uint16_t size)
{
    os_uint8_t pdm_rbufftemp[340];

    /* wait pdm message forever */
    os_mq_recv(&pdm_mq, pdm_rbufftemp, 340, OS_IPC_WAITING_FOREVER);

    /* copy the data */
    os_memcpy(buff, (char *)pdm_rbufftemp, size);

    return 0;
}

void am_pdm_start(void)
{
    /* Enable PDM */
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_PDM);
    am_hal_pdm_enable();
}

void am_pdm_stop(void)
{
    /* Disable PDM */
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_PDM);
    am_hal_pdm_disable();
}

uint8_t am_pdm_left_gain_get(void)
{
    /* get the left gain */
    return am_hal_pdm_left_gain_get();
}

void am_pdm_left_gain_set(uint8_t gain_val)
{
    /* set the left gain */
    am_hal_pdm_left_gain_set(gain_val);
}

uint8_t am_pdm_right_gain_get(void)
{
    /* get the right gain */
    return am_hal_pdm_right_gain_get();
}

void am_pdm_right_gain_set(uint8_t gain_val)
{
    /* set the right gain */
    am_hal_pdm_right_gain_set(gain_val);
}

void am_pdm_isr(void)
{
    int        i;
    os_int16_t pdm_sbufftemp[160];

    /* Clear the PDM interrupt */
    am_hal_pdm_int_clear(AM_HAL_PDM_INT_UNDFL | AM_HAL_PDM_INT_OVF | AM_HAL_PDM_INT_FIFO);

    for (i = 0; i < PDM_FIFO_THRESHOLD; i++) /* adjust as needed */
    {
        pdm_sbufftemp[i] = (os_int16_t)am_hal_pdm_fifo_data_read();
    }

    /* send the message */
    os_mq_send(&pdm_mq, pdm_sbufftemp, PDM_FIFO_THRESHOLD * sizeof(os_int16_t));
}

int os_hw_pdm_init(void)
{
    /* Enable power to modules used */
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PDM);

    /* Enable the PDM clock and data */
    am_hal_gpio_pin_config(PDM_GPIO_CLK, PDM_GPIO_CFG_CLK | AM_HAL_GPIO_HIGH_DRIVE);
    am_hal_gpio_pin_config(PDM_GPIO_DATA, PDM_GPIO_CFG_DATA);

    /* PDM setting */
    am_hal_pdm_config(&g_sPDMConfig);

    /* Enable PDM interrupts */
    am_hal_pdm_int_enable(AM_HAL_PDM_INT_FIFO);

    /* Clear PDM interrupts */
    am_hal_pdm_int_clear(AM_HAL_PDM_INT_UNDFL | AM_HAL_PDM_INT_OVF | AM_HAL_PDM_INT_FIFO);

    /* messagequeue init */
    os_mq_init(&pdm_mq,
               "mq_pdm",
               &am_pdm_buffer_pool[0],
               340 - sizeof(void *),
               sizeof(am_pdm_buffer_pool),
               OS_IPC_FLAG_FIFO);

    return 0;
}
#ifdef OS_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(os_hw_pdm_init);
#endif

#endif
