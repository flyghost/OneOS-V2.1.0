#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "user_humiture.h"
#include "modmachine.h"

/**************************define the const args*********************/

#define TEMPERATURE                     (12)
#define HUMIDITY                        (13)


#define MP_HUMITURE_CHECK_OPEN(self) 	MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(self, device_humiture_obj_t, "humiture");


const mp_obj_type_t mp_humiture_type;

typedef struct _device_humiture_obj_t {
    mp_obj_base_t base;
    device_info_t *device;
}device_humiture_obj_t;

mp_obj_t mp_humiture_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * humiture = mpycall_device_find("humiture");
    if (NULL == humiture)
    {
        mp_raise_ValueError("humiture can not find!\n");
    }
    device_humiture_obj_t *self = m_new_obj(device_humiture_obj_t);
    self->base.type = &mp_humiture_type;
    self->device = humiture;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_humiture_init(mp_obj_t self)
{
    device_humiture_obj_t *humiture = (device_humiture_obj_t *)self;
    
    OPEN_MP_MACHINE_DEVICE(humiture->device->open_flag, 
                        humiture->device->ops->open, 
                        humiture->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mphumiture_init_obj, mp_humiture_init);

STATIC mp_obj_t mp_humiture_deinit (mp_obj_t self)
{
    device_humiture_obj_t *humiture = (device_humiture_obj_t *)self;

    CLOSE_MP_MACHINE_DEVICE(humiture->device->open_flag, 
                        humiture->device->ops->close, 
                        humiture->device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mphumiture_deinit_obj, mp_humiture_deinit);


STATIC mp_obj_t mp_humiture_read(size_t n_args, const mp_obj_t *args)
{
 
    struct humituredata value = {0};
	
	MP_HUMITURE_CHECK_OPEN(args[0]);
	
    if (0 != usr->ops->ioctl(usr, IOCTL_HUMITURE_READ, &value)) {
        mp_raise_ValueError("failed to read humiture!\n");
		return mp_obj_new_int(-1);
    }
    if (1 == n_args) {
        mp_obj_module_t *o = mp_obj_new_dict(2);
        mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_TEMPERATURE), mp_obj_new_float(value.temperature));
        mp_obj_dict_store(o, MP_OBJ_NEW_QSTR(MP_QSTR_HUMIDITY), mp_obj_new_float(value.humidity));
        return MP_OBJ_FROM_PTR(o);
    
    } else if (2 == n_args) {
        mp_int_t data = mp_obj_get_int(args[1]);
        if ((TEMPERATURE != data) && (HUMIDITY != data)) {
            mp_raise_ValueError("failed to read humiture!\n");
        }
		
        return (TEMPERATURE == data)? mp_obj_new_float(value.temperature):mp_obj_new_float(value.humidity);
    } 
    
    return mp_obj_new_float(0);
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mphumiture_read_obj, 1, 2, mp_humiture_read);


STATIC const mp_rom_map_elem_t mp_module_humiture_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mphumiture_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mphumiture_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), 	MP_ROM_PTR(&mphumiture_read_obj) },

    // class constants
	{ MP_ROM_QSTR(MP_QSTR_TEMPERATURE), MP_ROM_INT(TEMPERATURE) },
    { MP_ROM_QSTR(MP_QSTR_HUMIDITY), MP_ROM_INT(HUMIDITY) },

};

STATIC MP_DEFINE_CONST_DICT(device_humiture_locals_dict, mp_module_humiture_globals_table);

const mp_obj_type_t mp_humiture_type =
{
    { &mp_type_type },
    .name = MP_QSTR_Humiture,
    .make_new = mp_humiture_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_humiture_locals_dict,
};


