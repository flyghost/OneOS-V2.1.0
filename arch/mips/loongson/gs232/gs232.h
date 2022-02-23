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
 * @file        gs232.h
 *
 * @brief       gs232 head.
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __GS232_H__
#define __GS232_H__

#include <oneos_config.h>
#include "../common/mips.h"

#define INTC_BASE           0xBFD01040

#ifdef ARCH_MIPS_SOC_LS1B
#define GS232_INTC_CELLS        4
#endif

#ifdef ARCH_MIPS_SOC_LS1C300
#define GS232_INTC_CELLS        5
#endif

#define GS232_NR_IRQS           (32 * GS232_INTC_CELLS)

#define GMAC0_BASE          0xBFE10000
#define GMAC0_DMA_BASE      0xBFE11000
#define GMAC1_BASE          0xBFE20000
#define GMAC1_DMA_BASE      0xBFE21000
#define I2C0_BASE           0xBFE58000
#define PWM0_BASE           0xBFE5C000
#define PWM1_BASE           0xBFE5C010
#define PWM2_BASE           0xBFE5C020
#define PWM3_BASE           0xBFE5C030
#define WDT_BASE            0xBFE5C060
#define RTC_BASE            0xBFE64000
#define I2C1_BASE           0xBFE68000
#define I2C2_BASE           0xBFE70000
#define AC97_BASE           0xBFE74000
#define NAND_BASE           0xBFE78000
#define SPI_BASE            0xBFE80000
#define CAN1_BASE           0xBF004300
#define CAN0_BASE           0xBF004400

#ifndef __ASSEMBLY__
#include <arch_hw.h>

/* Watch Dog registers */
#define WDT_EN              HWREG32(WDT_BASE + 0x00)
#define WDT_SET             HWREG32(WDT_BASE + 0x04)
#define WDT_TIMER           HWREG32(WDT_BASE + 0x08)

#define PLL_FREQ            HWREG32(0xbfe78030)
#define PLL_DIV_PARAM       HWREG32(0xbfe78034)

struct gs232_intc_regs
{
    volatile unsigned int int_isr;
    volatile unsigned int int_en;
    volatile unsigned int int_set;
    volatile unsigned int int_clr;        /* offset 0x10*/
    volatile unsigned int int_pol;
    volatile unsigned int int_edge;        /* offset 0 */
};

extern void os_hw_timer_init(void);

#endif

#endif

