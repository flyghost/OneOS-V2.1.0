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

#ifndef MICROPY_INCLUDED_MIDDLE_SPI_H
#define MICROPY_INCLUDED_MIDDLE_SPI_H
#include "model_device.h"
#include "mpconfigport.h"
#include "py/obj.h"

#if MICROPY_PY_MACHINE_SPI

#define MPY_SPI_BUS_NAME_MAX        (8)
#define MPY_SPI_DEV_NAME_MAX        (16)

typedef struct _machine_spi_obj_t {
    mp_obj_base_t base;
    uint8_t open_flag;
    int cs_pin_num;
    device_info_t *spi_bus;
    char spi_dev_name[MPY_SPI_DEV_NAME_MAX];
} machine_spi_obj_t;

typedef struct _machine_spi_dev_msg_buf_t {
    const void *send_buf;
    void *recv_buf;
    uint32_t length;
} machine_spi_dev_msg_buf_t;


#define MP_SPI_MASK		(0x3)
#define MP_SPI_MODE_0 (0)
#define MP_SPI_MODE_1 (1)
#define MP_SPI_MODE_2 (2)
#define MP_SPI_MODE_3 (3)

#define MP_SPI_MSB (0 << 2)
#define MP_SPI_LSB (1 << 2)

typedef enum MP_SPI_CMD_TYPE {
    SPI_CONFIG_CMD = 0,
    SPI_ADD_NEW_DEV_CMD,
    SPI_DEV_ATTACH_CMD,
    SPI_DEV_SEND_RECV_CMD,
} MP_SPI_CMD_TYPE_E;

void spi_dev_reg_ext(char *dev_name);

#endif
#endif

