#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "user_beep.h"


#define MP_BEEP_OPEN_CHECK(self)	MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(self, device_beep_obj_t, "beep")


const mp_obj_type_t mp_beep_type;

typedef struct _device_beep_obj_t {
    mp_obj_base_t base;
    device_info_t *device;
}device_beep_obj_t;


mp_obj_t mp_beep_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * beep = mpycall_device_find("beep");
    if (! beep){
        mp_raise_ValueError("beep can not find!\n");
    }
    device_beep_obj_t *self = m_new_obj(device_beep_obj_t);
    self->base.type = &mp_beep_type;
    self->device = beep;

	self->device->id = (n_args == 1) ? mp_obj_get_int(args[0]) : -1;
	
    return (mp_obj_t) self;
}

STATIC mp_obj_t  beep_init(size_t n_args, const mp_obj_t *args)
{
    device_info_t *beep_device = ((device_beep_obj_t*) args[0])->device;

    if (n_args == 2)
    {
        beep_device->id = mp_obj_get_int(args[1]);
    }
    if (beep_device->id == -1)
    {
        mp_raise_ValueError("Beep pin is null, Please set it first!");
    }

    OPEN_MP_MACHINE_DEVICE(beep_device->open_flag, 
                        beep_device->ops->ioctl, 
                        beep_device, MP_MACHINE_OP_OPEN, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(beep_init_obj, 1, 2, beep_init);

STATIC mp_obj_t  beep_deinit(mp_obj_t self_in)
{
    device_info_t *beep_device = ((device_beep_obj_t*) self_in)->device;

    CLOSE_MP_MACHINE_DEVICE(beep_device->open_flag, 
                        beep_device->ops->ioctl, 
                        beep_device, MP_MACHINE_OP_CLOSE, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_deinit_obj, beep_deinit);

STATIC mp_obj_t beep_on(mp_obj_t self_in)
{
    MP_BEEP_OPEN_CHECK(self_in);
    
    usr->ops->ioctl(usr, MP_MACHINE_OP_ENABLE, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_on_obj, beep_on);

STATIC mp_obj_t beep_off(mp_obj_t self_in)
{
	MP_BEEP_OPEN_CHECK(self_in);
    usr->ops->ioctl(usr, MP_MACHINE_OP_DISABLE, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(beep_off_obj, beep_off);



STATIC const mp_rom_map_elem_t mp_module_beep_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&beep_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&beep_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_on), 	 MP_ROM_PTR(&beep_on_obj) },
	{ MP_ROM_QSTR(MP_QSTR_off),  MP_ROM_PTR(&beep_off_obj) },
};

STATIC MP_DEFINE_CONST_DICT(device_beep_locals_dict, mp_module_beep_globals_table);

const mp_obj_type_t mp_beep_type =
{
    { &mp_type_type },
    .name = MP_QSTR_BEEP,
    .make_new = mp_beep_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_beep_locals_dict,
};


