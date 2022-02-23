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
 * @file        ingenic_usart.c
 *
 * @brief       This file implements usart driver for x1000
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <ingenic_usart.h>
#include <os_errno.h>
#include <drv_common.h>
#include <ingenic_ost.h>
#include <interrupt.h>

static unsigned short quot1[3] = {0};

static struct baudtoregs_t baudtoregs[] = 
{
#if (BOARD_EXTAL_CLK == 24000000)
    {50,0x7530,0x10,0x0},
    {75,0x4e20,0x10,0x0},
    {110,0x3521,0x10,0x0},
    {134,0x2b9d,0x10,0x0},
    {150,0x2710,0x10,0x0},
    {200,0x1d4c,0x10,0x0},
    {300,0x1388,0x10,0x0},
    {600,0x9c4,0x10,0x0},
    {1200,0x4e2,0x10,0x0},
    {1800,0x340,0x10,0x0},
    {2400,0x271,0x10,0x0},
    {4800,0x138,0x10,0x0},
    {9600,0x9c,0x10,0x0},
    {19200,0x4e,0x10,0x0},
    {38400,0x27,0x10,0x0},
    {57600,0x1a,0x10,0x0},
    {115200,0xd,0x10,0x0},
    {230400,0x6,0x11,0x550},
    {460800,0x3,0x11,0x550},
    {500000,0x3,0x10,0x0},
    {576000,0x3,0xd,0x0},
    {921600,0x2,0xd,0x0},
    {1000000,0x2,0xc,0x0},
    {1152000,0x1,0x14,0x400},
    {1500000,0x1,0x10,0x0},
    {2000000,0x1,0xc,0x0},
    {2500000,0x1,0x9,0x780},
    {3000000,0x1,0x8,0x0},
    {3500000,0x1,0x6,0x400},
    {4000000,0x1,0x6,0x0},
#elif (BOARD_EXTAL_CLK == 26000000)
    {50,0x7ef4,0x10,0x0},
    {75,0x546b,0x10,0x0},
    {110,0x398f,0x10,0x0},
    {134,0x2f40,0x10,0x0},
    {150,0x2a36,0x10,0x0},
    {200,0x1fbd,0x10,0x0},
    {300,0x151b,0x10,0x0},
    {600,0xa8e,0x10,0x0},
    {1200,0x547,0x10,0x0},
    {1800,0x385,0x10,0x0},
    {2400,0x2a4,0x10,0x0},
    {4800,0x152,0x10,0x0},
    {9600,0xa9,0x10,0x0},
    {19200,0x54,0x10,0x2},
    {38400,0x2a,0x10,0x2},
    {57600,0x1c,0x10,0x2},
    {115200,0xe,0x10,0x2},
    {230400,0x7,0x10,0x2},
    {460800,0x4,0xe,0x2},
    {500000,0x3,0x11,0x550},
    {576000,0x3,0xf,0x2},
    {921600,0x2,0xe,0x2},
    {1000000,0x2,0xd,0x0},
    {1152000,0x2,0xb,0x248},
    {1500000,0x1,0x11,0x550},
    {2000000,0x1,0xd,0x0},
    {2500000,0x1,0xa,0x2a0},
    {3000000,0x1,0x8,0x700},
    {3500000,0x1,0x7,0x2a0},
    {4000000,0x1,0x6,0x7c0},
#elif (BOARD_EXTAL_CLK == 48000000)
    {50,0xea60,0x10,0x0},
    {75,0x9c40,0x10,0x0},
    {110,0x6a42,0x10,0x0},
    {134,0x573a,0x10,0x0},
    {150,0x4e20,0x10,0x0},
    {200,0x3a98,0x10,0x0},
    {300,0x2710,0x10,0x0},
    {600,0x1388,0x10,0x0},
    {1200,0x9c4,0x10,0x0},
    {1800,0x67f,0x10,0x0},
    {2400,0x4e2,0x10,0x0},
    {4800,0x271,0x10,0x0},
    {9600,0x138,0x10,0x0},
    {19200,0x9c,0x10,0x0},
    {38400,0x4e,0x10,0x0},
    {57600,0x34,0x10,0x0},
    {115200,0x1a,0x10,0x0},
    {230400,0xd,0x10,0x0},
    {460800,0x6,0x11,0x550},
    {500000,0x6,0x10,0x0},
    {576000,0x5,0x10,0x700},
    {921600,0x3,0x11,0x550},
    {1000000,0x3,0x10,0x0},
    {1152000,0x3,0xd,0x0},
    {1500000,0x2,0x10,0x0},
    {2000000,0x2,0xc,0x0},
    {2500000,0x1,0x13,0x84},
    {3000000,0x1,0x10,0x0},
    {3500000,0x1,0xd,0x600},
    {4000000,0x1,0xc,0x0},
#endif
};

uint8_t uart_receivedata(uint32_t uart_base)
{
    return UART_RDR(uart_base);
}

uint8_t uart_receivestate(uint32_t uart_base)
{
    return !!!(UART_LSR(uart_base) & UARTLSR_DR);
}

uint8_t uart_enableirq(uint32_t irq,uint32_t uart_base)
{

    os_hw_interrupt_umask(irq);
    UART_IER(uart_base) |= (UARTIER_RIE | UARTIER_RTIE);

    return 0;
}

uint8_t uart_disableirq(uint32_t irq,uint32_t uart_base)
{

    UART_IER(uart_base) &= ~(UARTIER_RIE | UARTIER_RTIE);
    os_hw_interrupt_mask(irq);

    return 0;
}

uint8_t uart_get_interrupt(uint32_t uart_base)
{
    return UART_ISR(uart_base);
}

uint8_t uart_senddatapoll(uint32_t uart_base, uint8_t u8)
{
    UART_TDR(uart_base) = u8;
    return 0;
}

uint8_t uart_sendstate(uint32_t uart_base)
{
    return !((UART_LSR(uart_base) & (UARTLSR_TDRQ | UARTLSR_TEMT)) == 0x60);
}

static uint16_t *get_divisor(uint32_t baud)
{
    struct baudtoregs_t *bt;
    int index;

    for (index = 0; index < sizeof(baudtoregs)/sizeof(baudtoregs[0]); index ++)
    {
        bt = &baudtoregs[index];
        if (bt->baud == baud)
        {
            break;
        }
    }

    if (index < sizeof(baudtoregs)/sizeof(baudtoregs[0])) 
    {
        quot1[0] = bt->div;
        quot1[1] = bt->umr;
        quot1[2] = bt->uacr;
        return quot1;
    }

    return NULL;
}

uint8_t uart_init (uint32_t uart_base, struct uart_hw_config *cfg)
{
    os_uint32_t baud_div;
    unsigned short *quot1; 

    UART_IER(uart_base) = 0; /* clear interrupt */
    UART_FCR(uart_base) = ~UARTFCR_UUE; /* disable UART unite */

    UART_SIRCR(uart_base) = ~(SIRCR_RSIRE | SIRCR_TSIRE);

    /* Set databits, stopbits and parity. (8-bit data, 1 stopbit, no parity) */
    UART_LCR(uart_base) = UARTLCR_WLEN_8;

    /* set baudrate */
    quot1 = get_divisor(cfg->baud_rate);
    if (quot1 == OS_NULL)
    {
        {
            baud_div = BOARD_EXTAL_CLK / 16 / cfg->baud_rate;
        }

        UART_DLHR(uart_base) = (baud_div >> 8) & 0xff;
        UART_DLLR(uart_base) = baud_div & 0xff;
        UART_LCR(uart_base) &= ~UARTLCR_DLAB;
    }
    else
    {       
        UART_LCR(uart_base) |= UARTLCR_DLAB;
        UART_DLHR(uart_base) = (quot1[0] >> 8) & 0xff;
        UART_DLLR(uart_base) = quot1[0] & 0xff;
        UART_LCR(uart_base) &= ~UARTLCR_DLAB;

        UART_UMR(uart_base)  = quot1[1] & 0xff;
        UART_UACR(uart_base) = quot1[2] & 0xff;
    }

    UART_FCR(uart_base) = UARTFCR_UUE | UARTFCR_FE | UARTFCR_TFLS | UARTFCR_RFLS;

    return 0;
}

void uart_gpio_func_init(uint32_t uart_base)
{
    if (uart_base == UART0_BASE){
        gpio_set_func(GPIO_PORT_B,GPIO_FUNC_0,0x9 << 19);
    }else if(uart_base == UART1_BASE){
        gpio_set_func(GPIO_PORT_B,GPIO_FUNC_0,0x3 << 23);
    }else if(uart_base == UART2_BASE){
        gpio_set_func(GPIO_PORT_A,GPIO_FUNC_2,0x3 << 10);
    }else{
        return;
    }
    return;
}

