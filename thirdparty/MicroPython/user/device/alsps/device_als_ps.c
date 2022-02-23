#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_als_ps.h"


/****************************采样类型*******************************/
#define LIGHT                    		  (0x1)
#define PROXIMITYS                        (0x2)


#define MP_ALSPS_OPEN_CHECK(self)	MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(self, device_als_ps_obj_t, "als-ps")

const mp_obj_type_t mp_als_ps_type;

typedef struct _device_als_ps_obj_t {
    mp_obj_base_t base;
    device_info_t *device;
}device_als_ps_obj_t;

mp_obj_t mp_als_ps_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * als_ps = mpycall_device_find("als_ps");
    if (NULL == als_ps){
        mp_raise_ValueError("als_ps can not find!\n");
    }
    device_als_ps_obj_t *self = m_new_obj(device_als_ps_obj_t);
    self->base.type = &mp_als_ps_type;
    self->device = als_ps;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_als_ps_init(mp_obj_t self)
{
    device_als_ps_obj_t *als_ps = (device_als_ps_obj_t *)self;

    OPEN_MP_MACHINE_DEVICE(als_ps->device->open_flag, 
                        als_ps->device->ops->open, 
                        als_ps->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_als_ps_init_obj, mp_als_ps_init);

STATIC mp_obj_t mp_als_ps_deinit (mp_obj_t self)
{
    device_als_ps_obj_t *als_ps = (device_als_ps_obj_t *)self;

    CLOSE_MP_MACHINE_DEVICE(als_ps->device->open_flag, 
                        als_ps->device->ops->close, 
                        als_ps->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_als_ps_deinit_obj, mp_als_ps_deinit);


STATIC mp_obj_t mp_als_ps_read(size_t n_args, const mp_obj_t *args){
	
    struct als_ps value = {0};
	
    MP_ALSPS_OPEN_CHECK(args[0]);

	if (0 != usr->ops->ioctl(usr, IOCTL_ALS_PS_READ, &value)) {
		mp_raise_ValueError("mp_als_ps_read failed!\n");
		return mp_const_none;
	}
	if (1 == n_args) {
		mp_obj_module_t *o = mp_obj_new_dict(2);
		mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_LIGHT), mp_obj_new_float(value.light));
		mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_PROXIMITYS), mp_obj_new_int(value.proximitys));
		return MP_OBJ_FROM_PTR(o);

	}  else if (2 == n_args)  {
		mp_int_t data = mp_obj_get_int(args[1]);
		if ((LIGHT != data) && (PROXIMITYS != data)) {
			mp_raise_ValueError("als_ps_deinit failed!\n");
		}

		return (LIGHT == data)? mp_obj_new_float(value.light): mp_obj_new_int(value.proximitys);
	} 
	
	return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_als_ps_read_obj, 1, 2, mp_als_ps_read);



STATIC const mp_rom_map_elem_t mp_module_als_ps_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 		MP_ROM_PTR(&mp_als_ps_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 		MP_ROM_PTR(&mp_als_ps_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), 		MP_ROM_PTR(&mp_als_ps_read_obj) },

	{ MP_ROM_QSTR(MP_QSTR_LIGHT), 		MP_ROM_INT(LIGHT) },
    { MP_ROM_QSTR(MP_QSTR_PROXIMITYS), 	MP_ROM_INT(PROXIMITYS) },
};

STATIC MP_DEFINE_CONST_DICT(device_als_ps_locals_dict, mp_module_als_ps_globals_table);

const mp_obj_type_t mp_als_ps_type =
{
    { &mp_type_type },
    .name = MP_QSTR_ALS_PS,
    .make_new = mp_als_ps_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_als_ps_locals_dict,
};


