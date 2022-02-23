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
 * @file        ingenic_usart.h
 *
 * @brief        This file provides functions declaration for usart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __INGENIC_UART_H__
#define __INGENIC_UART_H__

#include <stdint.h>

typedef enum {RESET = 0, SET = !RESET} FlagStatus;

struct uart_hw_config{

    uint32_t baud_rate;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t bit_order;
    uint32_t invert;
};

struct baudtoregs_t
{
    unsigned int baud;
    unsigned short div;
    unsigned int umr:5;
    unsigned int uacr:12;
};

/* Uart Register */
#define UART_RDR(base)      REG8((base) + 0x00) /* R  8b H'xx */
#define UART_TDR(base)      REG8((base) + 0x00) /* W  8b H'xx */
#define UART_DLLR(base)     REG8((base) + 0x00) /* RW 8b H'00 */
#define UART_DLHR(base)     REG8((base) + 0x04) /* RW 8b H'00 */
#define UART_IER(base)      REG8((base) + 0x04) /* RW 8b H'00 */
#define UART_ISR(base)      REG8((base) + 0x08) /* R  8b H'01 */
#define UART_FCR(base)      REG8((base) + 0x08) /* W  8b H'00 */
#define UART_LCR(base)      REG8((base) + 0x0C) /* RW 8b H'00 */
#define UART_MCR(base)      REG8((base) + 0x10) /* RW 8b H'00 */
#define UART_LSR(base)      REG8((base) + 0x14) /* R  8b H'00 */
#define UART_MSR(base)      REG8((base) + 0x18) /* R  8b H'00 */
#define UART_SPR(base)      REG8((base) + 0x1C) /* RW 8b H'00 */
#define UART_MCR(base)      REG8((base) + 0x10) /* RW 8b H'00 */
#define UART_SIRCR(base)    REG8((base) + 0x20) /* RW 8b H'00 */
#define UART_UMR(base)		REG8((base) + 0x24)	/* W  8b H'00 */
#define UART_UACR(base)		REG8((base) + 0x28)	/* W  8b H'00 */

/*
 * Define macros for UARTIER
 * UART Interrupt Enable Register
 */
#define UARTIER_RIE     (1 << 0)    /* 0: receive fifo "full" interrupt disable */
#define UARTIER_TIE     (1 << 1)    /* 0: transmit fifo "empty" interrupt disable */
#define UARTIER_RLIE    (1 << 2)    /* 0: receive line status interrupt disable */
#define UARTIER_MIE     (1 << 3)    /* 0: modem status interrupt disable */
#define UARTIER_RTIE    (1 << 4)    /* 0: receive timeout interrupt disable */

/*
 * Define macros for UARTISR
 * UART Interrupt Status Register
 */
#define UARTISR_IP          (1 << 0)    /* 0: interrupt is pending  1: no interrupt */
#define UARTISR_IID         (7 << 1)    /* Source of Interrupt */
#define UARTISR_IID_MSI     (0 << 1)    /* Modem status interrupt */
#define UARTISR_IID_THRI    (1 << 1)    /* Transmitter holding register empty */
#define UARTISR_IID_RDI     (2 << 1)    /* Receiver data interrupt */
#define UARTISR_IID_RLSI    (3 << 1)    /* Receiver line status interrupt */
#define UARTISR_FFMS        (3 << 6)    /* FIFO mode select, set when UARTFCR.FE is set to 1 */
#define UARTISR_FFMS_NO_FIFO    (0 << 6)
#define UARTISR_FFMS_FIFO_MODE  (3 << 6)

/*
 * Define macros for UARTFCR
 * UART FIFO Control Register
 */
#define UARTFCR_FE      (1 << 0)    /* 0: non-FIFO mode  1: FIFO mode */
#define UARTFCR_RFLS    (1 << 1)    /* write 1 to flush receive FIFO */
#define UARTFCR_TFLS    (1 << 2)    /* write 1 to flush transmit FIFO */
#define UARTFCR_DMS     (1 << 3)    /* 0: disable DMA mode */
#define UARTFCR_UUE     (1 << 4)    /* 0: disable UART */
#define UARTFCR_RTRG    (3 << 6)    /* Receive FIFO Data Trigger */
#define UARTFCR_RTRG_1  (0 << 6)
#define UARTFCR_RTRG_4  (1 << 6)
#define UARTFCR_RTRG_8  (2 << 6)
#define UARTFCR_RTRG_15 (3 << 6)

/*
 * Define macros for UARTLCR
 * UART Line Control Register
 */
#define UARTLCR_WLEN    (3 << 0)    /* word length */
#define UARTLCR_WLEN_5  (0 << 0)
#define UARTLCR_WLEN_6  (1 << 0)
#define UARTLCR_WLEN_7  (2 << 0)
#define UARTLCR_WLEN_8  (3 << 0)
#define UARTLCR_STOP    (1 << 2)    /* 0: 1 stop bit when word length is 5,6,7,8
1: 1.5 stop bits when 5; 2 stop bits when 6,7,8 */
#define UARTLCR_PE      (1 << 3)    /* 0: parity disable */
#define UARTLCR_PROE    (1 << 4)    /* 0: even parity  1: odd parity */
#define UARTLCR_SPAR    (1 << 5)    /* 0: sticky parity disable */
#define UARTLCR_SBRK    (1 << 6)    /* write 0 normal, write 1 send break */
#define UARTLCR_DLAB    (1 << 7)    /* 0: access UARTRDR/TDR/IER  1: access UARTDLLR/DLHR */

/*
 * Define macros for UARTLSR
 * UART Line Status Register
 */
#define UARTLSR_DR      (1 << 0)    /* 0: receive FIFO is empty  1: receive data is ready */
#define UARTLSR_ORER    (1 << 1)    /* 0: no overrun error */
#define UARTLSR_PER     (1 << 2)    /* 0: no parity error */
#define UARTLSR_FER     (1 << 3)    /* 0; no framing error */
#define UARTLSR_BRK     (1 << 4)    /* 0: no break detected  1: receive a break signal */
#define UARTLSR_TDRQ    (1 << 5)    /* 1: transmit FIFO half "empty" */
#define UARTLSR_TEMT    (1 << 6)    /* 1: transmit FIFO and shift registers empty */
#define UARTLSR_RFER    (1 << 7)    /* 0: no receive error  1: receive error in FIFO mode */

/*
 * Define macros for UARTMCR
 * UART Modem Control Register
 */
#define UARTMCR_DTR     (1 << 0)    /* 0: DTR_ ouput high */
#define UARTMCR_RTS     (1 << 1)    /* 0: RTS_ output high */
#define UARTMCR_OUT1    (1 << 2)    /* 0: UARTMSR.RI is set to 0 and RI_ input high */
#define UARTMCR_OUT2    (1 << 3)    /* 0: UARTMSR.DCD is set to 0 and DCD_ input high */
#define UARTMCR_LOOP    (1 << 4)    /* 0: normal  1: loopback mode */
#define UARTMCR_FCM     (1 << 6)    /* 0: flow control by software; 1: hardware */
#define UARTMCR_MCE     (1 << 7)    /* 0: modem function is disable */

/*
 * Define macros for UARTMSR
 * UART Modem Status Register
 */
#define UARTMSR_DCTS    (1 << 0)    /* 0: no change on CTS_ pin since last read of UARTMSR */
#define UARTMSR_DDSR    (1 << 1)    /* 0: no change on DSR_ pin since last read of UARTMSR */
#define UARTMSR_DRI     (1 << 2)    /* 0: no change on RI_ pin since last read of UARTMSR */
#define UARTMSR_DDCD    (1 << 3)    /* 0: no change on DCD_ pin since last read of UARTMSR */
#define UARTMSR_CTS     (1 << 4)    /* 0: CTS_ pin is high */
#define UARTMSR_DSR     (1 << 5)    /* 0: DSR_ pin is high */
#define UARTMSR_RI      (1 << 6)    /* 0: RI_ pin is high */
#define UARTMSR_DCD     (1 << 7)    /* 0: DCD_ pin is high */

/*
 * Define macros for SIRCR
 * Slow IrDA Control Register
 */
#define SIRCR_TSIRE (1 << 0)    /* 0: transmitter is in UART mode  1: IrDA mode */
#define SIRCR_RSIRE (1 << 1)    /* 0: receiver is in UART mode  1: IrDA mode */
#define SIRCR_TPWS  (1 << 2)    /* 0: transmit 0 pulse width is 3/16 of bit length
1: 0 pulse width is 1.6us for 115.2Kbps */
#define SIRCR_TXPL  (1 << 3)    /* 0: encoder generates a positive pulse for 0 */
#define SIRCR_RXPL  (1 << 4)    /* 0: decoder interprets positive pulse as 0 */

uint8_t uart_receivedata(uint32_t uart_base);
uint8_t uart_receivestate(uint32_t uart_base);
uint8_t uart_enableirq(uint32_t irq,uint32_t uart_base);
uint8_t uart_disableirq(uint32_t irq,uint32_t uart_base);
uint8_t uart_senddatapoll(uint32_t uart_base, uint8_t u8);
uint8_t uart_sendstate(uint32_t uart_base);
uint8_t uart_init (uint32_t uart_base, struct uart_hw_config *cfg);
uint8_t uart_get_interrupt(uint32_t uart_base);
void uart_gpio_func_init(uint32_t uart_base);

#endif /* __INGENIC_USART_H__ */

