/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Armink (armink.ztl@gmail.com)
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
#include "py/mphal.h"

#if MICROPY_PY_MACHINE_PIN
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "usr_pin.h"
#include "py/runtime.h"
#include "py/gc.h"

#include "errno.h"
#include "usr_pin.h"


#define GPIO_MODE_IN                           ((uint32_t)0x00000000)   /*!< Input Floating Mode                   */
#define GPIO_MODE_OUT_PP                       ((uint32_t)0x00000001)   /*!< Output Push Pull Mode                 */
#define GPIO_MODE_OUT_OD                       ((uint32_t)0x00000011)   /*!< Output Open Drain Mode                */
#define GPIO_MODE_AF_PP                        ((uint32_t)0x00000002)   /*!< Alternate Function Push Pull Mode     */
#define GPIO_MODE_AF_OD                        ((uint32_t)0x00000012)   /*!< Alternate Function Open Drain Mode    */
#define GPIO_MODE_ANALOG                       ((uint32_t)0x00000003)   /*!< Analog Mode  */
#define GPIO_NOPULL                            ((uint32_t)0x00000000)   /*!< No Pull-up or Pull-down activation  */
#define GPIO_PULLUP                            ((uint32_t)0x00000001)   /*!< Pull-up activation                  */
#define GPIO_PULLDOWN                          ((uint32_t)0x00000002)   /*!< Pull-down activation                */
//#define GPIO_MODE_IT_RISING                    ((uint32_t)0x10110000)   /*!< External Interrupt Mode with Rising edge trigger detection          */
//#define GPIO_MODE_IT_FALLING                   ((uint32_t)0x10210000)   /*!< External Interrupt Mode with Falling edge trigger detection         */
//#define GPIO_MODE_IT_RISING_FALLING            ((uint32_t)0x10310000)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */




#define MP_PIN_OPEN_CHECK(self)		MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, machine_pin_obj_t, "pin")

	

	
const mp_obj_base_t machine_pin_obj_template = {&machine_pin_type};

void mp_pin_od_write(void *machine_pin, int stat) 
{
    machine_pin_obj_t *self = machine_pin;
    uint32_t mode = 0;
    
    if (stat == PIN_LOW) 
    {   
        mode = PIN_MODE_OUTPUT;
        self->device->ops->ioctl(self, MP_PIN_OP_SET_MODE, (void *)&mode);
        self->device->ops->write(self->device->owner.name, self->pin, &stat, 0);
    } 
    else 
    {
        mode = PIN_MODE_INPUT_PULLUP;
        self->device->ops->ioctl(self, MP_PIN_OP_SET_MODE, (void *)&mode);
    }
}

void mp_hal_pin_open_set(void *machine_pin, int mode) 
{
    machine_pin_obj_t *self = machine_pin;
    if(self->device == NULL)
    {
        mp_raise_ValueError("no pin model\n");
        return;
    }
    self->device->ops->ioctl(self, MP_PIN_OP_SET_MODE, (void *)&mode);
}

char* mp_hal_pin_get_name(void *machine_pin) {
	machine_pin_obj_t *self = machine_pin;
    return self->name;
}

STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "<Pin %d>", self->pin);
}

// constructor(drv_name, pin, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted port
    if (!MP_OBJ_IS_TYPE(args[0], &mp_type_tuple)) {
        mp_raise_ValueError("Pin id must be tuple of (\"GPIO_x\", pin#)");
    }
    mp_obj_t *items;
    mp_obj_get_array_fixed_n(args[0], 2, &items);
    const char *pin_name = mp_obj_str_get_str(items[0]);
    int wanted_pin = mp_obj_get_int(items[1]);

    machine_pin_obj_t *pin = m_new_obj(machine_pin_obj_t);
    if (!pin) {
        mp_raise_OSError(ENOMEM);
    }
		
	device_info_t* device = mpycall_device_find("pin");
		
	if(device == NULL)
	{
		mp_raise_ValueError("no pin model\n");
	}

    strncpy(pin->name, pin_name, sizeof(pin->name));
    pin->base = machine_pin_obj_template;
    pin->pin = wanted_pin;
	pin->device = device;

    if (n_args > 1 || n_kw > 0) 
    {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        if (machine_pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args) == mp_const_none)
        {
            return mp_const_none;
        }
    }

    return (mp_obj_t)pin;
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(machine_pin_obj_t *self, size_t n_args, 
                                                    const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };
    int ret = -1;
    
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    ret = mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if (ret != 0)
    {
        return mp_const_none;
    }
    // get io mode
    uint mode = args[ARG_mode].u_int;

    // get pull mode
    uint pull = GPIO_NOPULL;

    if (args[ARG_pull].u_obj != mp_const_none) 
    {
        pull = mp_obj_get_int(args[ARG_pull].u_obj);
    }

    if(mode == PIN_MODE_INPUT) 
    {
        if (pull == PIN_MODE_INPUT_PULLUP)
        {
            mode = PIN_MODE_INPUT_PULLUP;
        } 
        else if (pull == PIN_MODE_INPUT_PULLDOWN) 
        {
            mode = PIN_MODE_INPUT_PULLDOWN;
        } 
        else
        {
            mode = PIN_MODE_INPUT;
        }
    }

    self->device->ops->ioctl(self, MP_PIN_OP_SET_MODE, (void *)&mode);
    self->mode = mode;
    // get initial value
    if (args[ARG_value].u_obj != MP_OBJ_NULL) 
    {
        int value = mp_obj_is_true(args[ARG_value].u_obj);
        self->device->ops->write(self->device->owner.name, self->pin, &value, 0);
    }

    return self;
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
	MP_PIN_OPEN_CHECK(self_in);
	
	//device_info_t * dev = USER_DEV(self);
	
    if (n_args == 0) {
		return mp_obj_new_bool(self->device->ops->read(self->device->owner.name, self->pin, NULL, 0));
    } else {
        int value = mp_obj_is_true(args[0]);
		self->device->ops->write(self->device->owner.name, self->pin, &value, 0);
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) 
{
    machine_pin_obj_t *self = (machine_pin_obj_t *)args[0];

    MP_MACHINE_IS_ALREADY_OPENED(self);
    
    if (machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args) != mp_const_none)
    {
        SET_MACHINE_OPEN_FLAG(self);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

// pin.name()
STATIC mp_obj_t machine_pin_name(size_t n_args, const mp_obj_t *args) {
	MP_PIN_OPEN_CHECK(args[0]);
    return mp_obj_new_str(self->name, strlen(self->name));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_name_obj, 1, 2, machine_pin_name);

// pin.pin()
STATIC mp_obj_t machine_pin_pin(size_t n_args, const mp_obj_t *args) {
	MP_PIN_OPEN_CHECK(args[0]);
    return MP_OBJ_NEW_SMALL_INT(self->pin);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_pin_obj, 1, 2, machine_pin_pin);

STATIC mp_uint_t machine_pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;
	
    switch (request) {
        case MP_PIN_READ: {
			return self->device->ops->read(self->device->owner.name, self->pin, NULL, 0);
        }
        case MP_PIN_WRITE: {
            self->device->ops->write(self->device->owner.name, self->pin, &arg, 0);
            return 0;
        }
    }
    return 1;
}

// pin.port()
STATIC mp_obj_t machine_pin_port(size_t n_args, const mp_obj_t *args) {
	MP_PIN_OPEN_CHECK(args[0]);
    return MP_OBJ_NEW_SMALL_INT(self->pin);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_port_obj, 1, 2, machine_pin_port);

// pin.port()
STATIC mp_obj_t machine_pin_mode(size_t n_args, const mp_obj_t *args) {
	MP_PIN_OPEN_CHECK(args[0]);
	return MP_OBJ_NEW_SMALL_INT(self->mode);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_mode_obj, 1, 2, machine_pin_mode);

STATIC mp_obj_t machine_pin_deinit(mp_obj_t self_in) 
{
    machine_pin_obj_t *self = self_in;

    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        self->device->ops->ioctl, 
                        self, MP_PIN_OP_IRQ_DIS, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_deinit_obj, machine_pin_deinit);

STATIC void machine_pin_isr_handler(void *arg) {
    machine_pin_obj_t *self = arg;
    mp_sched_schedule(self->pin_isr_cb, MP_OBJ_FROM_PTR(self));

    return;
}

STATIC mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
    enum { ARG_handler, ARG_trigger, ARG_wake };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_handler, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_trigger, MP_ARG_INT, {.u_int = PIN_IRQ_MODE_RISING} },
    };
    MP_PIN_OPEN_CHECK(pos_args[0]);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    uint32_t mode = 0;
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (n_args > 1 || kw_args->used != 0) 
    {
        // configure irq
        self->pin_isr_cb = args[ARG_handler].u_obj;
        uint32_t trigger = args[ARG_trigger].u_int;

        //mpycall_pin_mode(self->pin, PIN_MODE_INPUT_PULLUP);
        mode = PIN_MODE_INPUT_PULLUP;
        self->device->ops->ioctl(self, MP_PIN_OP_SET_MODE, (void *)&mode);
        mpycall_pin_attach_irq(self->pin, trigger, machine_pin_isr_handler, (void*)self);
        self->device->ops->ioctl(self, MP_PIN_OP_IRQ_EN, 0);
        //mpycall_pin_irq_enable(self->pin, PIN_IRQ_ENABLE);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);

STATIC mp_obj_t get_pin_num(size_t n_args, const mp_obj_t *args){
	char msg[3] = {0};
	int index = 0;
	if (3 == n_args){
		MP_PIN_OPEN_CHECK(args[0]);
		//device_info_t * dev = USER_DEV(self);
		msg[0] =  (mp_obj_str_get_str(args[1]))[0];
		msg[1] = mp_obj_get_int(args[2]);
		index = self->device->ops->ioctl(self, MP_PIN_OP_GET_PIN_NUM,  msg);
	} else {
		msg[0] =  (mp_obj_str_get_str(args[0]))[0];
		msg[1] = mp_obj_get_int(args[1]);
		index  = mp_pin_get_num(NULL, msg); 
	}
	return MP_OBJ_NEW_SMALL_INT(index);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_get_num_obj,2, 3, get_pin_num);

STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init),      MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value),     MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_name),      MP_ROM_PTR(&machine_pin_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_pin),       MP_ROM_PTR(&machine_pin_pin_obj) },
	{ MP_ROM_QSTR(MP_QSTR_port),      MP_ROM_PTR(&machine_pin_port_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mode),      MP_ROM_PTR(&machine_pin_mode_obj) },
	{ MP_ROM_QSTR(MP_QSTR_irq), 	  MP_ROM_PTR(&machine_pin_irq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit),    MP_ROM_PTR(&machine_pin_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_index),     MP_ROM_PTR(&machine_pin_get_num_obj) },
	

	{ MP_ROM_QSTR(MP_QSTR_ALT_OD),    MP_ROM_INT(GPIO_MODE_AF_OD) },
    { MP_ROM_QSTR(MP_QSTR_ALT_PP),    MP_ROM_INT(GPIO_MODE_AF_PP) },
    { MP_ROM_QSTR(MP_QSTR_ANALOG),    MP_ROM_INT(GPIO_MODE_ANALOG) },
    { MP_ROM_QSTR(MP_QSTR_IN),        MP_ROM_INT(PIN_MODE_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT_PP),    MP_ROM_INT(PIN_MODE_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT_OD),    MP_ROM_INT(PIN_MODE_OUTPUT_OD) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(PIN_MODE_INPUT_PULLDOWN) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP),   MP_ROM_INT(PIN_MODE_INPUT_PULLUP) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(PIN_IRQ_MODE_RISING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(PIN_IRQ_MODE_FALLING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING_FALLING), MP_ROM_INT(PIN_IRQ_MODE_RISING_FALLING) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_LOW_LEVEL), MP_ROM_INT(PIN_IRQ_MODE_LOW_LEVEL) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_HIGH_LEVEL), MP_ROM_INT(PIN_IRQ_MODE_HIGH_LEVEL) },
		
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

STATIC const mp_pin_p_t machine_pin_pin_p = {
    .ioctl = machine_pin_ioctl,
};

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
    .call = machine_pin_call,
    .protocol = &machine_pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};

#endif // MICROPY_PY_MACHINE_PIN
