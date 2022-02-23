#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_key.h"
#include "usr_pin.h"

#define MP_KEY_OPEN_CHECK(self)		MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, device_key_obj_t, "key")


//此模块主要实现按键的回调函数功能。但是不能输入变参,还是有缺陷，看看后面有时间能不能完善
//是否要添加按键的中断触发方式接口，暂时未添加（上升沿，下降沿等）
const mp_obj_type_t mp_key_type ;


STATIC mp_obj_t key_init_helper(device_key_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args, int flag) 
{
    int ret = -1;
    static const mp_arg_t allowed_args[] = 
    {
        { MP_QSTR_pin,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_mode,     MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = PIN_MODE_INPUT_PULLUP} },
        { MP_QSTR_irq,      MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = PIN_IRQ_MODE_FALLING} },
    };
    

    // parse args
    struct args_
    {
        mp_arg_val_t pin, mode, irq_mode;
    } args;

    ret = mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);
    if (ret != 0)
    {
        return mp_const_none;
    }

    if (!self->key.device)
    {
        return mp_const_none;
    }

    usr_pin_obj_t *pin = &self->key.pin;
    // set the pin configuration values
    if (args.pin.u_int != pin->pin )
    {
        pin->pin = args.pin.u_int;
    } 
    if (args.mode.u_int != pin->mode) 
    {
        pin->mode = args.mode.u_int;	
    } 
    if (args.irq_mode.u_int != pin->irq_mode)
    {
        pin->irq_mode = args.irq_mode.u_int;    
    } 

    if (0 != self->key.device->ops->ioctl(self->key.device, flag, self))
    {
        mp_raise_ValueError("Set parameters failed!\n");
        return mp_const_none;
    }

   return mp_obj_new_int(0);
}



mp_obj_t mp_key_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
	mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);        
	device_info_t *key_device = mpycall_device_find("KEY");

    if (!key_device)
    {
         nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "key doesn't exist", mp_obj_get_int(args[0])));
    }
	
    // create new uart object
    device_key_obj_t *self = m_new_obj(device_key_obj_t);
	if (!self){
		mp_raise_ValueError("malloc memory failed!\n");
	}
    self->key.base.type = &mp_key_type;
    self->key.device = key_device;

	self->id = mp_obj_get_int(args[0]);
	
	mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
	key_init_helper(self, n_args-1, args+1, &kw_args, MP_MACHINE_OP_SET_PARAM);
    return (mp_obj_t) self;
}

STATIC mp_obj_t  key_init(mp_obj_t self_in)
{
    device_key_obj_t *self = self_in;

    OPEN_MP_MACHINE_DEVICE(self->open_flag, 
                        self->key.device->ops->ioctl, 
                        self->key.device, MP_MACHINE_OP_INIT, self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_key_init_obj, key_init);

STATIC mp_obj_t  key_deinit(mp_obj_t self_in)
{
    device_key_obj_t *self = self_in;

    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        self->key.device->ops->ioctl, 
                        self->key.device, MP_MACHINE_OP_DEINIT, self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_key_deinit_obj, key_deinit);



STATIC void device_key_isr_handler(void *arg) {

    device_key_obj_t *self = arg;
	usr_key_obj_t *key = &self->key;
    mp_sched_schedule(key->pin_isr_cb, MP_OBJ_FROM_PTR(self));
}

STATIC mp_obj_t key_callback(mp_obj_t self_in, mp_obj_t func_ptr)
{
	MP_KEY_OPEN_CHECK(self_in);
	
	self->key.pin_isr_cb = func_ptr;
	self->isr_handler = device_key_isr_handler;
	self->key.device->ops->ioctl(self->key.device, MP_MACHINE_OP_CALLBACK, self);
	
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_key_callback, key_callback);

STATIC mp_obj_t key_setter(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
	device_key_obj_t *key_obj = (device_key_obj_t *)args[0];
	MP_KEY_OPEN_CHECK(key_obj);
	key_init_helper(key_obj, n_args-1, args+1, kw_args, MP_MACHINE_OP_SET_PARAM);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_key_setter_obj, 1, key_setter);

STATIC const mp_rom_map_elem_t mp_module_touchkey_globals_table[] =
{
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_key_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_key_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&mp_key_callback)},
	{ MP_ROM_QSTR(MP_QSTR_setter), 	 MP_ROM_PTR(&mp_key_setter_obj)},
};

STATIC MP_DEFINE_CONST_DICT(device_key_locals_dict, mp_module_touchkey_globals_table);


STATIC void mp_device_key_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    device_key_obj_t *self = MP_OBJ_TO_PTR(self_in);
	usr_pin_obj_t *key = &self->key.pin;
    mp_printf(print, "key (id=%u pin=%u, mode=%u, irq_mode=%u)", self->id, key->pin, key->mode, key->irq_mode);
}

const mp_obj_type_t mp_key_type =
{
    { &mp_type_type },
    .name = MP_QSTR_key,
    .make_new = mp_key_make_new,
	.print = mp_device_key_print,
    .locals_dict = (mp_obj_dict_t *)&device_key_locals_dict,
};


