/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * modified from Copyright (c) 2018 SummerGift <zhangyuan@rt-thread.com>
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
#include "py/mperrno.h"

#ifdef MICROPY_PY_MACHINE_UART
#include "usr_misc.h"
#include "py/stream.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "machine_uart.h"
#include "usr_uart.h"


#define MP_UART_OPEN_CHECK(self)	MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, machine_uart_obj_t, "uart")

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
	uint8_t open_flag;
    mp_uart_device_handler *device;
}machine_uart_obj_t;

static middle_uart_config_t g_config = {0};

STATIC void machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) 
{
    mp_uart_device_handler *device = ((machine_uart_obj_t*)self_in)->device;
    middle_uart_config_t *cfg = device->other;
    
    mp_printf(print, "uart( port : %s | baudrate : %d | data bits : %d | stop bits : %d | parity : %d) ", 
        device->owner.name, cfg->baud_rate, cfg->data_bits, cfg->stop_bits, cfg->parity);
}


/// \method init(baudrate, bits=8, parity=None, stop=1, *, timeout=1000, timeout_char=0, flow=0, read_buf_len=64)
///
/// Initialise the UART bus with the given parameters:
///
///   - `baudrate` is the clock rate.
///   - `bits` is the number of bits per byte, 7, 8 or 9.
///   - `parity` is the parity, `None`, 0 (even) or 1 (odd).
///   - `stop` is the number of stop bits, 1 or 2.
///   - `timeout` is the timeout in milliseconds to wait for the first character.
///   - `timeout_char` is the timeout in milliseconds to wait between characters.
///   - `flow` is RTS | CTS where RTS == 256, CTS == 512
///   - `read_buf_len` is the character length of the read buffer (0 to disable).
///
STATIC mp_obj_t machine_uart_init_helper(machine_uart_obj_t *self, size_t n_args, 
                                                const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 9600} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_parity, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
        { MP_QSTR_timeout_char, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_read_buf_len, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 64} },
    };

    // parse args
    struct {
        mp_arg_val_t baudrate, bits, parity, stop, flow, timeout, timeout_char, read_buf_len;
    } args;
    mp_arg_parse_all(n_args, pos_args, kw_args,
    MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    // set the UART configuration values
    mp_uart_device_handler *uart_p = self->device;

    // baudrate
    g_config.baud_rate = args.baudrate.u_int;
    // parity
    mp_int_t bits = args.bits.u_int;
    if (args.parity.u_obj == mp_const_none) 
    {
        g_config.parity = MP_UART_PARITY_NONE;
    } else {
        mp_int_t parity = mp_obj_get_int(args.parity.u_obj);
        g_config.parity = (parity & 1) ? MP_UART_PARITY_ODD : MP_UART_PARITY_EVEN;
        //bits += 1; // STs convention has bits including parity, not all mcu
    }

    // number of bits
    if (bits == 8) 
    {
        g_config.data_bits = MP_UART_DATA_BITS_8;
    } else if (bits == 9) {
        g_config.data_bits = MP_UART_DATA_BITS_9;
    } else if (bits == 7) {
        g_config.data_bits = MP_UART_DATA_BITS_7;
    } else {
        mp_raise_ValueError("unsupported combination of bits and parity");
    }

    // stop bits
    switch (args.stop.u_int) 
    {
        case 1: g_config.stop_bits = MP_UART_STOP_BITS_1; break;
        default: g_config.stop_bits = MP_UART_STOP_BITS_2; break;
    }

    //buffer size
    g_config.bufsz = args.read_buf_len.u_int;
    g_config.flow = args.flow.u_int;
    g_config.timeout = args.timeout.u_int;
    g_config.timeout_char = args.timeout_char.u_int;

    uart_p->other = &g_config;
    if (self->open_flag == MP_MACHINE_INIT_FLAG) /* fix bug: 19773 */
    {
        uart_p->ops->open(uart_p->owner.name);
    }
    uart_p->ops->ioctl(uart_p, MP_UART_SET_USR_CFG, &g_config);
    return mp_const_none;
}


STATIC mp_obj_t machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    char uartname[8] = {0};
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    snprintf(uartname, sizeof(uartname)-1, "uart%d", mp_obj_get_int(args[0]));
    mp_uart_device_handler *device = NULL; 
    machine_uart_obj_t *self = NULL;

    device = mpycall_device_find(uartname);
    if (!device)
    {
         nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "uart(%d) doesn't exist", mp_obj_get_int(args[0])));
    }
    // create new uart object
    self = m_new_obj(machine_uart_obj_t);
    self->base.type = &machine_uart_type;
    self->device = device;

    if (n_args > 1 || n_kw > 0)
    {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_uart_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)self;
}

STATIC mp_obj_t machine_uart_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) 
{
    machine_uart_obj_t * self = args[0];
    
    MP_MACHINE_IS_ALREADY_OPENED(self);
    SET_MACHINE_OPEN_FLAG(self);
    return machine_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_uart_init_obj, 1, machine_uart_init);

STATIC mp_obj_t machine_uart_deinit(mp_obj_t self_in) 
{
    machine_uart_obj_t *self = self_in;

    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        self->device->ops->close, 
                        self->device->owner.name);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_deinit_obj, machine_uart_deinit);

#define RETRY_TIMES 500

STATIC mp_obj_t machine_uart_writechar(mp_obj_t self_in, mp_obj_t char_in) {
	MP_UART_OPEN_CHECK(self_in);
   
    uint16_t data = mp_obj_get_int(char_in);
    
	self->device->ops->write(self->device->owner.name, 0, &data, 1);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_uart_writechar_obj, machine_uart_writechar);


STATIC mp_obj_t machine_uart_readchar(mp_obj_t self_in) 
{
    MP_UART_OPEN_CHECK(self_in);

    unsigned char ch = 0xff; 
    self->device->ops->read(self->device->owner.name, 0, &ch, 1);

    if (ch == 0xff)
    {
        return mp_const_none;
    }

    return MP_OBJ_NEW_SMALL_INT(ch);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_readchar_obj, machine_uart_readchar);

STATIC const mp_rom_map_elem_t machine_uart_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_uart_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_uart_deinit_obj) },
//    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_uart_any_obj) },

    /// \method read([nbytes])
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    /// \method readline()
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj)},
    /// \method readinto(buf[, nbytes])
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    /// \method write(buf)
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },

    { MP_ROM_QSTR(MP_QSTR_writechar), MP_ROM_PTR(&machine_uart_writechar_obj) },
    { MP_ROM_QSTR(MP_QSTR_readchar), MP_ROM_PTR(&machine_uart_readchar_obj) },
//    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&pyb_uart_sendbreak_obj) },

//    class constants
    { MP_ROM_QSTR(MP_QSTR_FC_RTS), MP_ROM_INT(UART_HWCONTROL_RTS) },
    { MP_ROM_QSTR(MP_QSTR_FC_CTS), MP_ROM_INT(UART_HWCONTROL_CTS) },
	{ MP_ROM_QSTR(MP_QSTR_FC_NONE), MP_ROM_INT(UART_HWCONTROL_NONE) },
		
		
	{ MP_ROM_QSTR(MP_QSTR_BITS_7), MP_ROM_INT(7) },
	{ MP_ROM_QSTR(MP_QSTR_BITS_8), MP_ROM_INT(8) },
	{ MP_ROM_QSTR(MP_QSTR_BITS_9), MP_ROM_INT(9) },
	
	{ MP_ROM_QSTR(MP_QSTR_PARITY_NONE), MP_ROM_PTR(mp_const_none) },
	{ MP_ROM_QSTR(MP_QSTR_PARITY_EVEN), MP_ROM_INT(0) },
	{ MP_ROM_QSTR(MP_QSTR_PARITY_ODD), MP_ROM_INT(1) },
	
	{ MP_ROM_QSTR(MP_QSTR_STOP_1), MP_ROM_INT(1) },
	{ MP_ROM_QSTR(MP_QSTR_STOP_2), MP_ROM_INT(2) },
};
STATIC MP_DEFINE_CONST_DICT(machine_uart_locals_dict, machine_uart_locals_dict_table);

STATIC mp_uint_t machine_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) 
{
    mp_uart_device_handler *device = ((machine_uart_obj_t*) self_in)->device;
    byte *buf = buf_in;

    if (((machine_uart_obj_t*)self_in)->open_flag != MP_MACHINE_INIT_FLAG)
    {
        mp_raise_ValueError("Invalid operation, please open the device.");
        return 0;                                       
    }

    return device->ops->read(device->owner.name, 0, buf, size);
}

STATIC mp_uint_t machine_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) 
{
    mp_uart_device_handler *device = ((machine_uart_obj_t*) self_in)->device;
    const byte *buf = (const byte *)buf_in;
    
    if (((machine_uart_obj_t*)self_in)->open_flag != MP_MACHINE_INIT_FLAG)
    {
        //mp_raise_ValueError("Invalid operation, please the open device.");
        return 0;                                       
    }
    
    return device->ops->write(device->owner.name, 0, (void*)buf, size);
}

STATIC mp_uint_t machine_uart_ioctl(mp_obj_t obj, mp_uint_t request, uintptr_t arg, int *errcode) 
{   
    /* check the flag, then do your business */
    return 0;
}

STATIC const mp_stream_p_t uart_stream_p = {
    .read = machine_uart_read,
    .write = machine_uart_write,
    .ioctl = machine_uart_ioctl,
    .is_text = false,
};

const mp_obj_type_t machine_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = machine_uart_print,
    .make_new = machine_uart_make_new,
    .getiter = mp_identity_getiter,
    .iternext = mp_stream_unbuffered_iter,
    .protocol = &uart_stream_p,
    .locals_dict = (mp_obj_dict_t*)&machine_uart_locals_dict,
};

#endif // MICROPY_PY_MACHINE_UART
