#include "usr_pm.h"
#include <stdio.h>
#include <string.h>

#include "py/runtime.h"

#if (MICROPY_PY_MACHINE_PM)

typedef struct _machine_hard_pm_obj_t {
    mp_obj_base_t base;
    device_info_t* pm;
} machine_hard_pm_obj_t;

mp_obj_t machine_hard_pm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    //char pm_device[4];

    //snprintf(pm_device, sizeof(pm_device), "i2c%d", mp_obj_get_int(all_args[0]));
    //struct i2c_bus_device *i2c_bus = _i2c_open(mp_obj_get_int(all_args[0]));
		void * pm = mpycall_device_find("pm");
    if (pm == NULL) {
        //printf("can't find %s device\r\n", iic_device);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pm doesn't exist"));
    }

    // create new hard I2C object
    machine_hard_pm_obj_t *self = m_new_obj(machine_hard_pm_obj_t);
    self->base.type = &machine_hard_pm_type;
    self->pm = pm;
    return (mp_obj_t) self;
}


//STATIC mp_obj_t machine_pm_settime(size_t n_args, const mp_obj_t *args){
//    machine_hard_pm_obj_t *self = (machine_hard_pm_obj_t*)MP_OBJ_TO_PTR(args[0]);
//		int timebuf[3];
//		if(n_args != 4)
//		{
//			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pm n_args error"));
//		}
//		
//		timebuf[0] = mp_obj_get_int(args[1]);
//		timebuf[1] = mp_obj_get_int(args[2]);
//		timebuf[2] = mp_obj_get_int(args[3]);
//	
//		self->pm->ops->ioctl(self->pm, SET_TIME, timebuf);

//    // return number of acks received
//    return mp_const_none;
//}
//MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pm_settime_obj, 3, 4, machine_pm_settime);

//STATIC mp_obj_t machine_pm_setdate(size_t n_args, const mp_obj_t *args){
//    machine_hard_pm_obj_t *self = (machine_hard_pm_obj_t*)MP_OBJ_TO_PTR(args[0]);
//		int date[3];
//		if(n_args != 4)
//		{
//			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pm n_args error"));
//		}
//		
//		date[0] = mp_obj_get_int(args[1]);
//		date[1] = mp_obj_get_int(args[2]);
//		date[2] = mp_obj_get_int(args[3]);
//	
//		self->pm->ops->ioctl(self->pm, SET_DATE, date);

//    // return number of acks received
//    return mp_const_none;
//}
//MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pm_setdate_obj, 3, 4, machine_pm_setdate);

STATIC mp_obj_t machine_pm_request(size_t n_args, const mp_obj_t *args){
    machine_hard_pm_obj_t *self = (machine_hard_pm_obj_t*)MP_OBJ_TO_PTR(args[0]);
		int time_temp;
		if(n_args != 2)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pm n_args error"));
		}
		
		time_temp = mp_obj_get_int(args[1]);

		self->pm->ops->ioctl(self->pm, time_temp, NULL);

    // return number of acks received
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pm_request_obj, 1, 2, machine_pm_request);


STATIC const mp_rom_map_elem_t mp_machine_pm_locals_dict_table[] = {
    //{ MP_ROM_QSTR(MP_QSTR_settime), MP_ROM_PTR(&machine_pm_settime_obj) },
    //{ MP_ROM_QSTR(MP_QSTR_setdate), MP_ROM_PTR(&machine_pm_setdate_obj) },
		{ MP_ROM_QSTR(MP_QSTR_pm_request), MP_ROM_PTR(&machine_pm_request_obj) },
		{ MP_ROM_QSTR(MP_QSTR_STANDBY), MP_ROM_INT(MP_PM_SLEEP_MODE_STANDBY) },
		{ MP_ROM_QSTR(MP_QSTR_SHUTDOWN), MP_ROM_INT(MP_PM_SLEEP_MODE_SHUTDOWN) },

};
MP_DEFINE_CONST_DICT(mp_machine_pm_locals_dict, mp_machine_pm_locals_dict_table);


const mp_obj_type_t machine_hard_pm_type = {
    { &mp_type_type },
    .name = MP_QSTR_pm,
    .make_new = machine_hard_pm_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_pm_locals_dict,
};
#endif

