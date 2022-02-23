/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef MICROPY_INCLUDED_MIDDLE_UART_H
#define MICROPY_INCLUDED_MIDDLE_UART_H

#include "model_device.h"
#include "py/mpprint.h"
#include "serial/serial.h"
#include "modmachine.h"

#define MP_UART_SET_DEAULT_CFG      0
#define MP_UART_SET_USR_CFG         1

#define MP_SPI_NAME_MAX (8)
typedef device_info_t mp_uart_device_handler;

typedef long mp_uart_result;

typedef struct middle_uart_config
{
    int baud_rate;
    int parity;
    int data_bits;
    int stop_bits;
    int bufsz;
	int flow;
	int timeout_char;
	unsigned int  timeout;
} middle_uart_config_t;

#define MP_UART_PARITY_NONE (0)
#define MP_UART_PARITY_ODD  (1)
#define MP_UART_PARITY_EVEN (2)

#define MP_UART_DATA_BITS_7 (0)
#define MP_UART_DATA_BITS_8  DATA_BITS_8  //(1)
#define MP_UART_DATA_BITS_9  DATA_BITS_9  //(2)

#define MP_UART_STOP_BITS_1  STOP_BITS_1  //(0)
#define MP_UART_STOP_BITS_2  STOP_BITS_2  //(1)


#define UART_HWCONTROL_RTS  (0x1UL << 8U)            /*!< 0x00000100 */
#define UART_HWCONTROL_CTS  (0x1UL << 9U)            /*!< 0x00000200 */
#define UART_HWCONTROL_NONE 0x00000000U              /*!< No hardware control       */



#endif
