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
 * @file        x1000_intc.h
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef _X1000_INTC_H_
#define _X1000_INTC_H_


/*
 * INTC (Interrupt Controller)
 */
#define INTC_ISR(n)             (INTC_BASE + 0x00 + (n) * 0x20)
#define INTC_IMR(n)             (INTC_BASE + 0x04 + (n) * 0x20)
#define INTC_IMSR(n)            (INTC_BASE + 0x08 + (n) * 0x20)
#define INTC_IMCR(n)            (INTC_BASE + 0x0c + (n) * 0x20)
#define INTC_IPR(n)             (INTC_BASE + 0x10 + (n) * 0x20)

#define REG_INTC_ISR(n)         REG32(INTC_ISR((n)))
#define REG_INTC_IMR(n)         REG32(INTC_IMR((n)))
#define REG_INTC_IMSR(n)        REG32(INTC_IMSR((n)))
#define REG_INTC_IMCR(n)        REG32(INTC_IMCR((n)))
#define REG_INTC_IPR(n)         REG32(INTC_IPR((n)))

// interrupt controller interrupts
#define IRQ_DMIC                0
#define IRQ_AIC0                1
#define IRQ_RESERVED2           2
#define IRQ_RESERVED3           3
#define IRQ_RESERVED4           4
#define IRQ_RESERVED5           5
#define IRQ_RESERVED6           6
#define IRQ_SFC                 7
#define IRQ_SSI0                8
#define IRQ_RESERVED9           9
#define IRQ_PDMA                10
#define IRQ_PDMAD               11
#define IRQ_RESERVED12          12
#define IRQ_RESERVED13          13
#define IRQ_GPIO3               14
#define IRQ_GPIO2               15
#define IRQ_GPIO1               16
#define IRQ_GPIO0               17
#define IRQ_RESERVED18          18
#define IRQ_RESERVED19          19
#define IRQ_RESERVED20          20
#define IRQ_OTG                 21
#define IRQ_RESERVED22          22
#define IRQ_AES                 23
#define IRQ_RESERVED24          24
#define IRQ_TCU2                25
#define IRQ_TCU1                26
#define IRQ_TCU0                27
#define IRQ_RESERVED28          28
#define IRQ_RESERVED29          29
#define IRQ_CIM                 30
#define IRQ_LCD                 31
#define IRQ_RTC                 32
#define IRQ_RESERVED33          33
#define IRQ_RESERVED34          34
#define IRQ_RESERVED35          35
#define IRQ_MSC1                36
#define IRQ_MSC0                37
#define IRQ_SCC                 38
#define IRQ_RESERVED39          39
#define IRQ_PCM0                40
#define IRQ_RESERVED41          41
#define IRQ_RESERVED42          42
#define IRQ_RESERVED43          43
#define IRQ_HARB2               44
#define IRQ_RESERVED45          45
#define IRQ_HARB0               46
#define IRQ_CPM                 47
#define IRQ_RESERVED48          48
#define IRQ_UART2               49
#define IRQ_UART1               50
#define IRQ_UART0               51
#define IRQ_DDR                 52
#define IRQ_RESERVED53          53
#define IRQ_EFUSE               54
#define IRQ_MAC                 55
#define IRQ_RESERVED56          56
#define IRQ_RESERVED57          57
#define IRQ_I2C2                58
#define IRQ_I2C1                59
#define IRQ_I2C0                60
#define IRQ_PDMAM               61
#define IRQ_JPEG                62
#define IRQ_RESERVED63          63

#define IRQ_INTC_MAX            63

#ifndef __ASSEMBLY__

#define __intc_unmask_irq(n)    (REG_INTC_IMCR((n)/32) = (1 << ((n)%32)))
#define __intc_mask_irq(n)      (REG_INTC_IMSR((n)/32) = (1 << ((n)%32)))
#define __intc_ack_irq(n)       (REG_INTC_IPR((n)/32) = (1 << ((n)%32)))        /* A dummy ack, as the Pending Register is Read Only. Should we remove __intc_ack_irq() */

#endif /* !__ASSEMBLY__ */

#endif /* _X1000_INTC_H_ */
