#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "user_six_axis.h"

#define GYRO (0)
#define ACCE (1)

#define MP_SIX_AXIS_OPEN_CHECK(self) 	MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(self, device_six_axis_obj_t, "six-axis");

const mp_obj_type_t mp_six_axis_type;

typedef struct _device_six_axis_obj_t {
    mp_obj_base_t base;
    device_info_t *device;
}device_six_axis_obj_t;

mp_obj_t mp_six_axis_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * six_axis = mpycall_device_find("six_axis");
    if (NULL == six_axis) {
        mp_raise_ValueError("six_axis can not find!\n");
    }
    device_six_axis_obj_t *self = m_new_obj(device_six_axis_obj_t);
    self->base.type = &mp_six_axis_type;
    self->device = six_axis;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_six_axis_init(mp_obj_t self)
{
    device_six_axis_obj_t *six_axis = (device_six_axis_obj_t *)self;

    OPEN_MP_MACHINE_DEVICE(six_axis->device->open_flag, 
                        six_axis->device->ops->open, 
                        six_axis->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mpsix_axis_init_obj, mp_six_axis_init);

STATIC mp_obj_t mp_six_axis_deinit (mp_obj_t self)
{
    device_six_axis_obj_t *six_axis = (device_six_axis_obj_t *)self;
    
    CLOSE_MP_MACHINE_DEVICE(six_axis->device->open_flag, 
                        six_axis->device->ops->close, 
                        six_axis->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mpsix_axis_deinit_obj, mp_six_axis_deinit);

STATIC mp_obj_t mp_six_axis_read(size_t n_args, const mp_obj_t *args){

    struct os_sensor_data six_axis_data[2] = {0};
 
	MP_SIX_AXIS_OPEN_CHECK(args[0]);
	
    if (0 != usr->ops->ioctl(usr, MPY_GYRO_ACCE_INFO_READ, six_axis_data)) {
        mp_raise_ValueError("failed to read six axis data !\n");
    }
    if (1 == n_args) {
        mp_obj_module_t *gyro_dict = mp_obj_new_dict(3);
        mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_X), mp_obj_new_int(six_axis_data[0].data.gyro.x));
        mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Y), mp_obj_new_int(six_axis_data[0].data.gyro.y));
		mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Z), mp_obj_new_int(six_axis_data[0].data.gyro.z));
				
		mp_obj_module_t *acce_dict = mp_obj_new_dict(3);
        mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_X), mp_obj_new_int(six_axis_data[1].data.acce.x));
        mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Y), mp_obj_new_int(six_axis_data[1].data.acce.y));
		mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Z), mp_obj_new_int(six_axis_data[1].data.acce.z));
		
		mp_obj_module_t *o = mp_obj_new_dict(2);
		mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_GYRO), gyro_dict);
        mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_ACCE), acce_dict);
				  
		return MP_OBJ_FROM_PTR(o);
        
    
    } else if (2 == n_args) {
        mp_int_t data = mp_obj_get_int(args[1]);
        if ((GYRO != data) && (ACCE != data)) {
            mp_raise_ValueError("Parameters is error!\n");
        }
        
        if (GYRO == data) {
			mp_obj_module_t *gyro_dict = mp_obj_new_dict(3);
			mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_X), mp_obj_new_int(six_axis_data[0].data.gyro.x));
			mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Y), mp_obj_new_int(six_axis_data[0].data.gyro.y));
			mp_obj_dict_store(gyro_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Z), mp_obj_new_int(six_axis_data[0].data.gyro.z));
			return MP_OBJ_FROM_PTR(gyro_dict);
        } else {
            mp_obj_module_t *acce_dict = mp_obj_new_dict(3);
			mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_X), mp_obj_new_int(six_axis_data[1].data.acce.x));
			mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Y), mp_obj_new_int(six_axis_data[1].data.acce.y));
			mp_obj_dict_store(acce_dict, MP_OBJ_NEW_QSTR(MP_QSTR_Z), mp_obj_new_int(six_axis_data[1].data.acce.z));
			return MP_OBJ_FROM_PTR(acce_dict);
        }
    } 
    
    return mp_obj_new_int(0);
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpsix_axis_read_obj, 1, 2, mp_six_axis_read);



STATIC const mp_rom_map_elem_t mp_module_six_axis_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mpsix_axis_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mpsix_axis_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), 	MP_ROM_PTR(&mpsix_axis_read_obj) },
	{ MP_ROM_QSTR(MP_QSTR_GYRO), 	MP_ROM_INT(GYRO) },
	{ MP_ROM_QSTR(MP_QSTR_ACCE), 	MP_ROM_INT(ACCE) },
};

STATIC MP_DEFINE_CONST_DICT(device_six_axis_locals_dict, mp_module_six_axis_globals_table);

const mp_obj_type_t mp_six_axis_type =
{
    { &mp_type_type },
    .name = MP_QSTR_SIX_AXIS,
    .make_new = mp_six_axis_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_six_axis_locals_dict,
};


