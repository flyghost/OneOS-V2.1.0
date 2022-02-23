#include "py/obj.h"

#if MICROPY_PY_MACHINE_DAC
#include "py/runtime.h"
#include "py/builtin.h"

#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "machine_dac.h"
#include "usr_dac.h"


#define MP_DAC_CHECK_OPEN(self)			MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, device_dac_obj_t, "dac")

typedef struct _device_dac_obj_t {
    mp_obj_base_t 	base;
	uint32_t 	  	channel;
	uint8_t  		open_flag;
    device_info_t 	*dac_device;
}device_dac_obj_t;


mp_obj_t mp_dac_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    char dac_name[8] = {0};
    device_info_t * dac_device = NULL;
    device_dac_obj_t *self = NULL;
    int id = 0;
    
    if (n_args != 2)
    {
        mp_raise_ValueError("The amount of parameters are wrong! Input two arguments please !!\n");
        return mp_const_none;
    }
    //mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    id = mp_obj_get_int(args[0]);
    if (id == -1)
    {
        strcpy(dac_name, "dac");
    } 
    else 
    {
        snprintf(dac_name, sizeof(dac_name)-1, "dac%d", id);
    }

    dac_device = mpycall_device_find(dac_name);
    if (NULL == dac_device)
    {
        mp_raise_ValueError("dac can not find!\n");
    }
    
    self = m_new_obj(device_dac_obj_t);
    self->base.type = &machine_dac_type;
    self->dac_device = dac_device;
    self->channel = mp_obj_get_int(args[1]);

    return (mp_obj_t)self;
}

STATIC mp_obj_t dac_init(size_t n_args, const mp_obj_t *args)
{
    device_dac_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    device_info_t *dac_device = ((device_dac_obj_t*) self)->dac_device;

    if (n_args == 2)
    {
        self->channel = mp_obj_get_int(args[1]);
    }

    OPEN_MP_MACHINE_DEVICE(self->open_flag, dac_device->ops->open, dac_device->owner.name);
    dac_device->ops->ioctl(dac_device, MP_MACHINE_OP_ENABLE, (void *)(self->channel));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(dac_init_obj, 1, 2, dac_init);


STATIC mp_obj_t dac_write(size_t n_args, const mp_obj_t *args)
{
    MP_DAC_CHECK_OPEN(args[0]);
    device_info_t *dac_device = ((device_dac_obj_t*) self)->dac_device;
    uint32_t  value = 0;

    if (n_args == 3)
    {
        self->channel = mp_obj_get_int(args[1]);
        value = mp_obj_get_int(args[2]);
    } 
    else
    {
        value = mp_obj_get_int(args[1]);
    }

    dac_device->ops->write(dac_device->owner.name, self->channel, &value, sizeof(value));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(dac_write_obj, 2, 3, dac_write);

STATIC mp_obj_t dac_deinit(mp_obj_t self_in)
{
    device_dac_obj_t *self = MP_OBJ_TO_PTR(self_in);
    device_info_t *dac_device = self->dac_device;

    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        dac_device->ops->ioctl, 
                        dac_device, MP_MACHINE_OP_DISABLE, (void *)(self->channel));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(dac_deinit_obj, dac_deinit);



STATIC const mp_rom_map_elem_t mp_module_dac_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&dac_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_write), 	 MP_ROM_PTR(&dac_write_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit),  MP_ROM_PTR(&dac_deinit_obj) },
};

STATIC MP_DEFINE_CONST_DICT(device_dac_locals_dict, mp_module_dac_globals_table);

const mp_obj_type_t machine_dac_type =
{
    { &mp_type_type },
    .name = MP_QSTR_DAC,
    .make_new = mp_dac_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_dac_locals_dict,
};
#endif // MICROPY_PY_MACHINE_DAC

