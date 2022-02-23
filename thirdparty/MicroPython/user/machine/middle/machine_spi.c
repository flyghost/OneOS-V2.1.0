/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "machine_spi.h"
#include "usr_spi.h"
#include "spi.h"
#include "mphalport.h"

#if (MICROPY_PY_MACHINE_SPI && MICROPY_PY_MACHINE_PIN)

// if a port didn't define MSB/LSB constants then provide them
#ifndef MICROPY_PY_MACHINE_SPI_MSB
#define MICROPY_PY_MACHINE_SPI_MSB (0)
#define MICROPY_PY_MACHINE_SPI_LSB (1)
#endif

#define MICROPY_PY_MACHINE_SPI_POLARITY_LOW             (0)
#define MICROPY_PY_MACHINE_SPI_POLARITY_HIGH            (1)

#define MICROPY_PY_MACHINE_SPI_PHASE_0                  (0)
#define MICROPY_PY_MACHINE_SPI_PHASE_1                  (1)


static void machine_spi_transfer(mp_obj_base_t *self_in, size_t len, 
                                        char *src, char *dest) 
{
    machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;
    machine_spi_dev_msg_buf_t arg = {0};
    
    if (self->open_flag != MP_MACHINE_INIT_FLAG) 
    {
        mp_raise_msg(&mp_type_OSError, "transfer on deinitialized SPI");
        return;
    }

    if(src && dest)
    {
        arg.length = len;
        arg.recv_buf = dest;
        arg.send_buf = src;
        self->spi_bus->ops->ioctl((void *)self->spi_dev_name, SPI_DEV_SEND_RECV_CMD, (void *)&arg);
    }
    else if(src)
    {
        self->spi_bus->ops->write(self->spi_dev_name, 0, (void*)src, len);
    }
    else if(dest)
    {
        self->spi_bus->ops->read(self->spi_dev_name, 0, (void*)dest, len);
    }

    return;
}

static mp_obj_t mp_machine_spi_write(mp_obj_t self, mp_obj_t wr_buf) 
{
    mp_buffer_info_t src;
    
    mp_get_buffer_raise(wr_buf, &src, MP_BUFFER_READ);
    machine_spi_transfer(self, src.len, src.buf, NULL);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_machine_spi_write_obj, mp_machine_spi_write);

static mp_obj_t mp_machine_spi_read(size_t n_args, const mp_obj_t *args)
{
    vstr_t vstr = {0};

    vstr_init_len(&vstr, mp_obj_get_int(args[1]));
    memset(vstr.buf, n_args >= 3 ? mp_obj_get_int(args[2]) : 0, vstr.len);
    machine_spi_transfer(args[0], vstr.len, vstr.buf, vstr.buf);

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_machine_spi_read_obj, 3, 4, mp_machine_spi_read);

static mp_obj_t mp_machine_spi_write_readinto(size_t n_args, const mp_obj_t *args) 
{
    mp_buffer_info_t src;
    mp_buffer_info_t dest;

    mp_get_buffer_raise(args[1], &src, MP_BUFFER_READ);
    mp_get_buffer_raise(args[2], &dest, MP_BUFFER_WRITE);
    
    if (src.len != dest.len) 
    {
        mp_raise_ValueError("buffers must be the same length");
    }
    machine_spi_transfer(args[0], src.len, src.buf, dest.buf);
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_machine_spi_write_readinto_obj, 3, 4, mp_machine_spi_write_readinto);

static mp_obj_t machine_spi_deinit(mp_obj_t self_in) 
{
    machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;
    
    CLEAN_MACHINE_OPEN_FLAG(self);
    return  mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_deinit_obj, machine_spi_deinit);

static mp_obj_t machine_spi_init_helper(machine_spi_obj_t *self, size_t n_args, 
                                        const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 500000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int mode = 0;
    int baudrate = args[ARG_baudrate].u_int;;
    int polarity = args[ARG_polarity].u_int;
    int phase = args[ARG_phase].u_int;
    int bits = args[ARG_bits].u_int;
    int firstbit = args[ARG_firstbit].u_int;
    int ret;
    int b_buf[3];
    
    b_buf[0] = baudrate;
    b_buf[1] = bits;

    if(!polarity && !phase)
    {
        mode = MP_SPI_MODE_0;
    }

    if(!polarity && phase)
    {
        mode = MP_SPI_MODE_1;
    }

    if(polarity && !phase)
    {
        mode = MP_SPI_MODE_2;
    }

    if(polarity && phase)
    {
        mode = MP_SPI_MODE_3;
    }

    if(firstbit == 0)
    {
        mode |= MP_SPI_MSB;
    } else {
        mode |= MP_SPI_LSB;
    }

    b_buf[2] = mode;
    /* config spi */
    {
        ret = self->spi_bus->ops->ioctl((void *)self->spi_dev_name, SPI_CONFIG_CMD, b_buf);
    }

    if(ret < 0)
    {
        mp_raise_OSError(-1);
    }

    return mp_const_none;
}


STATIC mp_obj_t machine_spi_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    machine_spi_obj_t *self = args[0];
    
    MP_MACHINE_IS_ALREADY_OPENED(self);
    SET_MACHINE_OPEN_FLAG(self);
    return machine_spi_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_init_obj, 1, machine_spi_init);

STATIC mp_obj_t mp_machine_spi_readinto(size_t n_args, const mp_obj_t *args) 
{
    mp_buffer_info_t bufinfo;
    
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);
    memset(bufinfo.buf, n_args == 3 ? mp_obj_get_int(args[2]) : 0, bufinfo.len);
    
    machine_spi_transfer(args[0], bufinfo.len, bufinfo.buf, bufinfo.buf);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_machine_spi_readinto_obj, 3, 4, mp_machine_spi_readinto);

STATIC const mp_rom_map_elem_t machine_spi_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_spi_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_machine_spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_machine_spi_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_machine_spi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_readinto), MP_ROM_PTR(&mp_machine_spi_write_readinto_obj) },

    { MP_ROM_QSTR(MP_QSTR_MSB), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_MSB) },
    { MP_ROM_QSTR(MP_QSTR_LSB), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_LSB) },

    { MP_ROM_QSTR(MP_QSTR_POLARITY_LOW), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_POLARITY_LOW) },
    { MP_ROM_QSTR(MP_QSTR_POLARITY_HIGH), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_POLARITY_HIGH) },

    { MP_ROM_QSTR(MP_QSTR_PHASE_0), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_PHASE_0) },
    { MP_ROM_QSTR(MP_QSTR_PHASE_1), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_PHASE_1) },
};
MP_DEFINE_CONST_DICT(machine_spi_locals_dict, machine_spi_locals_dict_table);

static mp_obj_t machine_spi_make_new(const mp_obj_type_t *type, size_t n_args, 
                                                    size_t n_kw, const mp_obj_t *all_args) 
{
    char spi_bus_name[MPY_SPI_BUS_NAME_MAX] = {0};
    machine_spi_obj_t *self = m_new_obj(machine_spi_obj_t);
    
    if(n_args != 3)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "SPI param number must be 3!"));
    }

    snprintf(spi_bus_name, sizeof(spi_bus_name), "spi%d", mp_obj_get_int(all_args[0]));
    self->spi_bus = mpycall_device_find(spi_bus_name);
    if (NULL == self->spi_bus)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Can't find spi bus[%s]!", spi_bus_name));
    }
    strcpy(self->spi_dev_name, (char *)mp_obj_str_get_str(all_args[1]));
    self->cs_pin_num = mp_obj_get_int(all_args[2]);
    if (self->spi_bus->ops->ioctl((void *)self, SPI_DEV_ATTACH_CMD, NULL) != 0)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Operate spi dev[%s] failed!", self->spi_dev_name));
    }

    self->base.type = &machine_spi_type;
    return (mp_obj_t)self;
}

const mp_obj_type_t machine_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .make_new = machine_spi_make_new, // delegate to master constructor
    .locals_dict = (mp_obj_dict_t*)&machine_spi_locals_dict,
};

#endif // MICROPY_PY_MACHINE_SPI


