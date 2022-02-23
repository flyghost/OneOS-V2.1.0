/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 ChenYong (chenyong@rt-thread.com)
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


#include "mphalport.h"


#ifdef MICROPY_PY_MACHINE_TIMER
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "usr_misc.h"
#include "py/obj.h"
#include "py/runtime.h"

#include "machine_timer.h"

#define  MAX_TIMER  17

typedef enum machine_timer_param_type {
    PARAM_MODE = 0,
    PARAM_PERIOD,
    PARAM_CALLBACK,
    PARAM_TYPE_END,
} machine_timer_param_type_t;

static const mp_arg_t machine_timer_allowed_args[] = {
    { MP_QSTR_mode,         MP_ARG_INT, {.u_int = 1} },
    { MP_QSTR_period,       MP_ARG_INT, {.u_int = 0xffffffff} },
    { MP_QSTR_callback,     MP_ARG_OBJ, {.u_obj = mp_const_none} },
};


const mp_obj_type_t machine_timer_type;

STATIC void error_check(bool status, const char *msg) {
    if (!status) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, msg));
    }
}

STATIC void machine_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_timer_obj_t *self = self_in;

    mp_printf(print, "Timer(%p; ", self);

    if (self->timer->id >= 0) {
        mp_printf(print, "timer_id=%d, ", self->timer->id);
    } else {
        mp_printf(print, "timer_name=%s, ", self->dev_name);
    }
    mp_printf(print, "period=%d, ", self->timeout);
    mp_printf(print, "auto_reload=%d)", self->is_repeat);
}

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    machine_timer_obj_t *self = m_new_obj(machine_timer_obj_t);

    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
	
	self->timer = mpycall_device_find("timer");
	if (!self->timer){
		mp_raise_ValueError("Could not find timer");
		return mp_const_none;
	}
    // check input timer device name or ID
    if (mp_obj_is_qstr(args[0])) {
        self->timer->id = -1;
        strncpy(self->dev_name, mp_obj_str_get_str(args[0]), OS_NAME_MAX);
    } else {
        error_check(0, "Input time device name or ID error.");
    }

    // initialize timer device
    self->base.type = &machine_timer_type;
    self->timeout = 0;
    self->timeout_cb = MP_NULL;
    self->is_repeat = MP_TRUE;
    self->is_init = MP_FALSE;

    // return constant object
    return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;

    if (self->is_init == MP_TRUE) 
    {
        self->timer->ops->ioctl(self, MP_MACHINE_OP_DISABLE, 0);
        self->timer->ops->ioctl(self, MP_MACHINE_OP_DELETE, 0);
        self->is_init = MP_FALSE;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

static void callback_handler(void* parameter) {
    machine_timer_obj_t *self = (machine_timer_obj_t *)parameter;

    mp_sched_schedule(self->timeout_cb, MP_OBJ_FROM_PTR(self));
    return;
}

static mp_obj_t machine_timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) 
{
    machine_timer_obj_t *self = (machine_timer_obj_t *)args[0];  
    int mode = 0;
    mp_arg_val_t dargs[MP_ARRAY_SIZE(machine_timer_allowed_args)] = {0};
    
    mp_arg_parse_all(n_args - 1, 
                        args + 1, 
                        kw_args, 
                        MP_ARRAY_SIZE(machine_timer_allowed_args), 
                        machine_timer_allowed_args, 
                        dargs);
    if (4 == n_args) {
        self->is_repeat = dargs[PARAM_MODE].u_int;
        self->timeout = dargs[PARAM_PERIOD].u_int;
        self->timeout_cb = dargs[PARAM_CALLBACK].u_obj;
    } else {
        mp_raise_ValueError("invalid format");
    }

    error_check(self->timeout > 0, "Set timeout value error");

    if (self->is_init != MP_FALSE)
    {
		mp_raise_ValueError("timer is already init");
    }

    if (self->timeout_cb == MP_NULL) {
		mp_raise_ValueError("invalid callback");
    }

    // set timer mode
    mode = (self->is_repeat == MP_TRUE)? MP_TIMER_FLAG_PERIODIC:MP_TIMER_FLAG_ONE_SHOT;
    self->timer->owner.flag = mode;
    self->timer->ops->ioctl(self, MP_MACHINE_OP_CREATE, callback_handler);

	if (!self->timer->other)
    {
        mp_err("%s create error!", self->dev_name);
        return mp_const_none;
    }
	self->timer->ops->ioctl(self, MP_MACHINE_OP_ENABLE, 0);

    self->is_init = MP_TRUE;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 1, machine_timer_init);

STATIC mp_obj_t machine_timer_tick_per_second(void)
{
    return mp_obj_new_int(OS_TICK_PER_SECOND);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_timer_tick_per_second_obj, machine_timer_tick_per_second);

STATIC const mp_rom_map_elem_t machine_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick_per_second), MP_ROM_PTR(&machine_timer_tick_per_second_obj) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(MP_FALSE) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(MP_TRUE) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals_dict, machine_timer_locals_dict_table);

const mp_obj_type_t machine_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .print = machine_timer_print,
    .make_new = machine_timer_make_new,
    .locals_dict = (mp_obj_t) &machine_timer_locals_dict,
};

#endif // MICROPYTHON_USING_MACHINE_TIMER

