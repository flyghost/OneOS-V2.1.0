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
 * @file        nrf5x_isr.c
 *
 * @brief       IRQ functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
// #include "os_irq.h"
// #include <arch_interrupt.h>
#include "nimble/nimble_npl.h"
#include "nrfx.h"

static void (*radio_isr_addr)(void);
static void (*rng_isr_addr)(void);
static void (*rtc0_isr_addr)(void);

void RADIO_IRQHandler(void)
{
    // os_interrupt_enter();
    radio_isr_addr();
    // os_interrupt_leave();
}

void RNG_IRQHandler(void)
{
    // os_interrupt_enter();
    rng_isr_addr();
    // os_interrupt_leave();
}

void RTC0_IRQHandler(void)
{
    // os_interrupt_enter();
    rtc0_isr_addr();
    // os_interrupt_leave();
}

void ble_npl_hw_set_isr(int irqn, void (*addr)(void))
{
    switch (irqn)
    {
    case RADIO_IRQn:
        radio_isr_addr = addr;
        break;
    case RNG_IRQn:
        rng_isr_addr = addr;
        break;
    case RTC0_IRQn:
        rtc0_isr_addr = addr;
        break;
    }
}
