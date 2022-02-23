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


#include "py/runtime.h"

#if MICROPY_PY_MACHINE_CAN
#include <stdio.h>
#include <string.h>
#include "mphalport.h"
#include "machine_can.h"
#include "bus_can.h"

#define MP_CAN_CHECK_OPEN(self)			MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, mp_machine_can_obj_t, "can")

/******************************************************************************/
// MicroPython bindings for generic machine.CAN

mp_obj_t mp_machine_can_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
	char can_name[8] = {0};
	int id = mp_obj_get_int(args[0]);
	mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
	if (id == -1){
		strcpy(can_name, "can");
	} else {
		snprintf(can_name, sizeof(can_name)-1, "can%d", id);
	}
    device_info_t * can = mpycall_device_find(can_name);
    if (NULL == can){
        mp_raise_ValueError("can can not find!\n");
		return mp_const_none;
    }

	// create new object
    mp_machine_can_obj_t *self = m_new_obj(mp_machine_can_obj_t);
    self->base.type = &mp_machine_can_type;
    self->can = can;
	self->can->id = id;
    return (mp_obj_t) self;
}

STATIC mp_obj_t machine_can_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) 
{
    mp_machine_can_obj_t *self = args[0];
    mp_obj_base_t *s = (mp_obj_base_t*)MP_OBJ_TO_PTR(self);
    mp_machine_can_p_t *can_p = (mp_machine_can_p_t*)s->type->protocol;

    SET_MACHINE_OPEN_FLAG(self);
    can_p->init(s, n_args - 1, args + 1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_can_init_obj, 1, machine_can_init);

STATIC mp_obj_t machine_can_deinit(mp_obj_t self_in) 
{
    mp_machine_can_obj_t *self = self_in;
    mp_obj_base_t *s = (mp_obj_base_t*)MP_OBJ_TO_PTR(self);
    mp_machine_can_p_t *can_p = (mp_machine_can_p_t*)s->type->protocol;
    
    can_p->deinit(s);
    CLEAN_MACHINE_OPEN_FLAG(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_can_deinit_obj, machine_can_deinit);

STATIC int mp_machine_can_transfer(mp_obj_t self, uint32_t offset, const uint8_t *buf, uint32_t bufsize) {
	mp_obj_base_t *can_obj = (mp_obj_base_t*)MP_OBJ_TO_PTR(self);
    mp_machine_can_p_t *can_p = (mp_machine_can_p_t*)can_obj->type->protocol;
    return can_p->transfer(can_obj, offset, buf, bufsize);
}


STATIC mp_obj_t mp_machine_can_read(size_t n_args, const mp_obj_t *args) {
	MP_CAN_CHECK_OPEN(args[0]);
	if (!self->can){
		mp_raise_ValueError("Can device is null!");
		return mp_const_none;
	}

	can_msg_t msg = {0};
	mp_obj_module_t *can_dict =  mp_obj_new_dict(4); 
    if (self->can->ops->read(self->can->owner.name, 0, &msg, sizeof(msg)) != MP_CAN_OP_ERROR){
		mp_obj_dict_store(can_dict, MP_OBJ_NEW_QSTR(MP_QSTR_id), mp_obj_new_str(msg.id, 8));
		mp_obj_dict_store(can_dict, MP_OBJ_NEW_QSTR(MP_QSTR_ide), mp_obj_new_int(msg.ide));
		mp_obj_dict_store(can_dict, MP_OBJ_NEW_QSTR(MP_QSTR_rtr), mp_obj_new_int(msg.rtr));
		if (msg.rtr != 1){ //·ÇÔ¶³ÌÖ¡
			mp_obj_dict_store(can_dict, MP_OBJ_NEW_QSTR(MP_QSTR_value), mp_obj_new_str((char *)msg.data, msg.len));
		}
		return MP_OBJ_FROM_PTR(can_dict);
	} 
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_machine_can_read_obj, 1, 2, mp_machine_can_read);


STATIC mp_obj_t mp_machine_can_write(mp_obj_t self_in, mp_obj_t wr_buf) {
	MP_CAN_CHECK_OPEN(self_in);
	if (!self->can){
		mp_raise_ValueError("Can device is null!");
		return mp_const_none;
	}

	// get the buffer to write from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(wr_buf, &bufinfo, MP_BUFFER_READ);
    if (mp_machine_can_transfer(self, 0, bufinfo.buf, bufinfo.len) == MP_CAN_OP_ERROR){
		return mp_const_none;
	}
    return MP_OBJ_NEW_SMALL_INT(bufinfo.len);
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_machine_can_write_obj, mp_machine_can_write);

STATIC const mp_rom_map_elem_t machine_can_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_can_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_can_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_machine_can_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_machine_can_write_obj) },
		
};

MP_DEFINE_CONST_DICT(mp_machine_can_locals_dict, machine_can_locals_dict_table);


STATIC void mp_machine_can_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_machine_can_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "CAN (id=%u baudrate=%u, prescale=%u, sjw=%u, bs1=%u, bs1=%u)", 
		self->can->id, self->cfg.baud, self->cfg.prescale, self->cfg.sjw, self->cfg.bs1, self->cfg.bs2);
}

STATIC mp_obj_t mp_machine_can_helper(mp_machine_can_obj_t *self, size_t n_args, int cmd, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	
	enum { ARG_baudrate, ARG_mode, ARG_prescale, ARG_sjw, ARG_bs1, ARG_bs2 };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 500000} },
		{ MP_QSTR_mode, 	MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_prescale, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_sjw,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bs1,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_bs2,		MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
	
	
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
	
	if (!self->can || !self->can->ops || !self->can->ops->ioctl || !self->can->ops->open){
		mp_raise_ValueError("invalid can peripheral");
		return mp_const_none;
	}
	
	mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set parameters
    self->cfg.baud = args[ARG_baudrate].u_int;
	self->cfg.mode = args[ARG_mode].u_int;
    self->cfg.prescale = args[ARG_prescale].u_int;
    self->cfg.sjw = args[ARG_sjw].u_int;
	self->cfg.bs1 = args[ARG_bs1].u_int;
	self->cfg.bs2 = args[ARG_bs2].u_int;
	
	
	self->can->ops->open(self->can->owner.name);
	
    // configure bus
    self->can->ops->ioctl(self->can, cmd, &self->cfg);

    return MP_OBJ_FROM_PTR(self);
}

STATIC void mp_machine_can_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
	mp_machine_can_helper((mp_machine_can_obj_t *)self_in, n_args, MP_MACHINE_OP_SET_PARAM, pos_args, kw_args);
}

STATIC int mp_machine_soft_can_transfer(mp_obj_base_t *self_in, uint32_t offset, const uint8_t *buf, uint32_t bufsize) {
    mp_machine_can_obj_t *self = (mp_machine_can_obj_t*)self_in;
	if (!self->can){
		mp_raise_ValueError("invalid can peripheral");
		return MP_CAN_OP_ERROR;
	}
    return self->can->ops->write(self->can->owner.name, 0, (void *)buf, bufsize);
}

STATIC void can_deinit(mp_obj_base_t *self_in)
{
    mp_machine_can_obj_t *self = (mp_machine_can_obj_t*)self_in;
    if (!self->can )
    {
        mp_raise_ValueError("invalid can peripheral");
        return ;
    }
    self->can->ops->close(self->can->owner.name);
}

const mp_machine_can_p_t mp_machine_can_p = {
    .init = mp_machine_can_init,
    .deinit = can_deinit,
    .transfer = mp_machine_soft_can_transfer,
};

const mp_obj_type_t mp_machine_can_type = {
    { &mp_type_type },
    .name = MP_QSTR_CAN,
    .print = mp_machine_can_print,
    .make_new = mp_machine_can_make_new, // delegate to master constructor
    .protocol = &mp_machine_can_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_can_locals_dict,
};

#endif // MICROPY_PY_MACHINE_CAN
