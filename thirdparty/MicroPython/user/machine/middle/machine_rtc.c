#include "usr_rtc.h"
#include <stdio.h>
#include <string.h>

#include "py/runtime.h"

typedef struct _machine_hard_rtc_obj_t {
    mp_obj_base_t base;
    device_info_t* rtc;
} machine_hard_rtc_obj_t;

mp_obj_t machine_hard_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {

	device_info_t * rtc = mpycall_device_find("rtc");
    if (rtc == NULL) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "rtc doesn't exist"));
        return mp_const_none;
    }

    // create new hard I2C object
    machine_hard_rtc_obj_t *self = m_new_obj(machine_hard_rtc_obj_t);
    self->base.type = &machine_hard_rtc_type;
    self->rtc = rtc;
    return (mp_obj_t) self;
}


STATIC mp_obj_t machine_rtc_settime(size_t n_args, const mp_obj_t *args){
    machine_hard_rtc_obj_t *self = (machine_hard_rtc_obj_t*)MP_OBJ_TO_PTR(args[0]);
		int timebuf[3];
		if(n_args != 4)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "rtc n_args error"));
		}
		
		timebuf[0] = mp_obj_get_int(args[1]);
		timebuf[1] = mp_obj_get_int(args[2]);
		timebuf[2] = mp_obj_get_int(args[3]);
	
		self->rtc->ops->ioctl(self->rtc, SET_TIME, timebuf);

    // return number of acks received
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_settime_obj, 3, 4, machine_rtc_settime);

STATIC mp_obj_t machine_rtc_setdate(size_t n_args, const mp_obj_t *args){
    machine_hard_rtc_obj_t *self = (machine_hard_rtc_obj_t*)MP_OBJ_TO_PTR(args[0]);
		int date[3];
		if(n_args != 4)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "rtc n_args error"));
		}
		
		date[0] = mp_obj_get_int(args[1]);
		date[1] = mp_obj_get_int(args[2]);
		date[2] = mp_obj_get_int(args[3]);
	
		self->rtc->ops->ioctl(self->rtc, SET_DATE, date);

    // return number of acks received
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_setdate_obj, 3, 4, machine_rtc_setdate);

/*
STATIC mp_obj_t machine_rtc_setalarm(size_t n_args, const mp_obj_t *args){
    machine_hard_rtc_obj_t *self = (machine_hard_rtc_obj_t*)MP_OBJ_TO_PTR(args[0]);
		int time_temp;
		if(n_args != 2)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "rtc n_args error"));
		}
		
		time_temp = mp_obj_get_int(args[1]);

		self->rtc->ops->ioctl(self->rtc, SET_ALARM, &time_temp);

    // return number of acks received
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_setalarm_obj, 1, 2, machine_rtc_setalarm);
*/

STATIC mp_obj_t machine_rtc_now(mp_obj_t self){

    device_info_t *device_rtc = ((machine_hard_rtc_obj_t*) self)->rtc;
	
	char gettime[25] = {0};

	device_rtc->ops->ioctl(device_rtc, GET_TIME, gettime);

    return mp_obj_new_str(gettime,strlen(gettime));
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_rtc_now_obj,  machine_rtc_now);





STATIC const mp_rom_map_elem_t mp_machine_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_settime), MP_ROM_PTR(&machine_rtc_settime_obj) },
    { MP_ROM_QSTR(MP_QSTR_setdate), MP_ROM_PTR(&machine_rtc_setdate_obj) },
    /*
    { MP_ROM_QSTR(MP_QSTR_setalarm), MP_ROM_PTR(&machine_rtc_setalarm_obj) },
    */
    { MP_ROM_QSTR(MP_QSTR_now), MP_ROM_PTR(&machine_rtc_now_obj) },
};
MP_DEFINE_CONST_DICT(mp_machine_rtc_locals_dict, mp_machine_rtc_locals_dict_table);


const mp_obj_type_t machine_hard_rtc_type = {
    { &mp_type_type },
    .name = MP_QSTR_RTC,
    .make_new = machine_hard_rtc_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_rtc_locals_dict,
};


