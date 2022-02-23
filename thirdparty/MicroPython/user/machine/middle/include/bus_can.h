/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Damien P. George
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
#ifndef MICROPY_INCLUDED_DRIVERS_BUS_CAN_H
#define MICROPY_INCLUDED_DRIVERS_BUS_CAN_H

#include "py/mphal.h"
#include "usr_can.h"

enum {
    MP_SPI_IOCTL_INIT,
    MP_SPI_IOCTL_DEINIT,
};

struct _can_msg{
	char id[8];     	/* CAN ID */
	char ide;
	char rtr;
	uint16_t len;
	char  data[16];      /* Data segment */
};

typedef struct _can_msg can_msg_t;



typedef struct _mp_can_obj_t {
    uint32_t baud; // microsecond delay for half SCK period
    uint16_t prescale;
	uint32_t mode;
	uint8_t sjw;
    uint8_t bs1;
    uint8_t bs2;
} mp_can_obj_t;


#endif // MICROPY_INCLUDED_DRIVERS_BUS_SPI_H
