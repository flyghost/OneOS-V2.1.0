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
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "machine_i2c.h"
#include "usr_i2c.h"

#ifdef MICROPY_PY_MACHINE_I2C

#define MP_I2C_OPEN_CHECK(self)     MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, machine_i2c_obj_t, "I2C")

static int machine_i2c_readfrom_func(mp_obj_t *self_in, uint16_t addr, uint8_t *dest, size_t len)
{
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    device_info_t *i2c_bus = (device_info_t *)(self->i2c_bus);
    
    return i2c_bus->ops->read(i2c_bus->owner.name, addr, dest, len);
}

static mp_obj_t machine_i2c_readfrom(mp_obj_t self_in, mp_obj_t addr_in, mp_obj_t rd_size) 
{
    machine_i2c_obj_t *self = self_in;
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->base.type->protocol;
    mp_int_t addr = mp_obj_get_int(addr_in);
    vstr_t vstr;
    int ret = MP_ERROR;

    if (self->open_flag != MP_MACHINE_INIT_FLAG)
    {
        mp_raise_ValueError("Invalid peripheral, please open ");
        return mp_const_none;
    }
    vstr_init_len(&vstr, mp_obj_get_int(rd_size));
    ret = i2c_p->readfrom(self_in, addr, (uint8_t*)vstr.buf, vstr.len);
    if (ret < 0) 
    {
        mp_raise_OSError(-ret);
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_3(machine_i2c_readfrom_obj, machine_i2c_readfrom);


int machine_i2c_writeto_func(mp_obj_t *self_in, uint16_t addr, const uint8_t *src, size_t len) 
{
    machine_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    device_info_t *i2c_bus = (device_info_t *)(self->i2c_bus);
    uint8_t buf[1] = {0};
    
    self->client_addr = addr;
    if (len == 0)
    {
        len = 1;
    }

    if (src == NULL)
    {
        src = buf;
    }
    
    return i2c_bus->ops->write(i2c_bus->owner.name, self->client_addr, (void *)src, len);
}

static mp_obj_t machine_i2c_writeto(mp_obj_t self_in, mp_obj_t addr_in, mp_obj_t wr_buf) 
{
    machine_i2c_obj_t *self = self_in;
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->base.type->protocol;
    mp_int_t addr = mp_obj_get_int(addr_in);
    mp_buffer_info_t bufinfo = {0};
    int ret = MP_ERROR;
    
    if (self->open_flag != MP_MACHINE_INIT_FLAG)
    {
        mp_raise_ValueError("Invalid peripheral, please open ");
        return mp_const_none;
    }
    mp_get_buffer_raise(wr_buf, &bufinfo, MP_BUFFER_READ);
    ret = i2c_p->writeto(self_in, addr,  bufinfo.buf, bufinfo.len);
    if (ret < 0) 
    {
        mp_raise_OSError(-ret);
    }
    
    return MP_OBJ_NEW_SMALL_INT(ret);
}
MP_DEFINE_CONST_FUN_OBJ_3(machine_i2c_writeto_obj, machine_i2c_writeto);


static mp_obj_t machine_i2c_deinit(mp_obj_t self_in) 
{
    machine_i2c_obj_t *self = self_in;
    device_info_t *i2c_bus = (device_info_t *)(self->i2c_bus);
	if (self->open_flag != MP_MACHINE_INIT_FLAG)
    {
        mp_raise_ValueError("Invalid peripheral, please open ");
        return mp_const_none;
    }
    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        i2c_bus->ops->close, 
                        i2c_bus->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_deinit_obj, machine_i2c_deinit);


static mp_obj_t machine_i2c_obj_init(mp_obj_t self_in) 
{
    machine_i2c_obj_t *self = self_in;
    device_info_t *i2c_bus = (device_info_t *)(self->i2c_bus);
    
    OPEN_MP_MACHINE_DEVICE(self->open_flag, 
                        i2c_bus->ops->open, 
                        i2c_bus->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_init_obj, machine_i2c_obj_init);

static mp_obj_t machine_i2c_make_new(const mp_obj_type_t *type, size_t n_args, 
                                            size_t n_kw, const mp_obj_t *args) 
{
    machine_i2c_obj_t *self = m_new_obj(machine_i2c_obj_t);
    char bus_name[IIC_NAME_MAX] = {0};

    if (n_args <= 0) 
    {
        mp_raise_ValueError("No I2C bus number.");
        return mp_const_none;
    }
    snprintf(bus_name, sizeof(bus_name), MICROPYTHON_MACHINE_I2C_PRENAME"%d", mp_obj_get_int(args[0]));
    self->i2c_bus = mpycall_device_find(bus_name);
    if (self->i2c_bus == NULL)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "%s doesn't exist", bus_name));
    }
    self->base.type = &machine_i2c_type;
    CLEAN_MACHINE_OPEN_FLAG(self);
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_arg_t machine_i2c_mem_allowed_args[] = {
    { MP_QSTR_addr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_memaddr, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_arg,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_addrsize, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
};

STATIC int read_mem(mp_obj_t self_in, uint16_t addr, uint32_t memaddr, 
                        uint8_t addrsize, uint8_t *buf, size_t len) 
 {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(self_in);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    uint8_t memaddr_buf[4];
    size_t memaddr_len = 0;
    
    for (int16_t i = addrsize - 8; i >= 0; i -= 8)
    {
        memaddr_buf[memaddr_len++] = memaddr >> i;
    }
    int ret = i2c_p->writeto(self_in, addr, memaddr_buf, memaddr_len);
    if (ret != memaddr_len) {
        // must generate STOP
        i2c_p->writeto(self_in, addr, NULL, 0);
        return ret;
    }
    return i2c_p->readfrom(self_in, addr, buf, len);
}

STATIC mp_obj_t machine_i2c_readfrom_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_n, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    machine_i2c_obj_t *self = pos_args[0];
    if (self->open_flag != MP_MACHINE_INIT_FLAG)
    {
        mp_raise_ValueError("Invalid peripheral, please open ");
        return mp_const_none;
    }
    
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // create the buffer to store data into
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[ARG_n].u_obj));

    // do the transfer
    int ret = read_mem(pos_args[0], args[ARG_addr].u_int, args[ARG_memaddr].u_int,
        args[ARG_addrsize].u_int, (uint8_t*)vstr.buf, vstr.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_readfrom_mem_obj, 1, machine_i2c_readfrom_mem);


static const mp_rom_map_elem_t machine_i2c_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_i2c_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom), MP_ROM_PTR(&machine_i2c_readfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto), MP_ROM_PTR(&machine_i2c_writeto_obj) },
    // memory operations
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem), MP_ROM_PTR(&machine_i2c_readfrom_mem_obj) },
};
MP_DEFINE_CONST_DICT(mp_machine_i2c_locals_dict, machine_i2c_locals_dict_table);

static const mp_machine_i2c_p_t mp_machine_i2c_p = {
    .readfrom = machine_i2c_readfrom_func,
    .writeto = machine_i2c_writeto_func,
};
const mp_obj_type_t machine_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .make_new = machine_i2c_make_new,
    .protocol = &mp_machine_i2c_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_i2c_locals_dict,
};

#endif /* MICROPY_PY_MACHINE_I2C */

