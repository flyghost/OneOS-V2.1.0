#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include "usr_adc.h"
#include "stdio.h"

#if MICROPY_PY_MACHINE_ADC

#define MP_ADC_CHECK_OPEN(self)     MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, machine_hard_adc_obj_t, "adc")

typedef struct _machine_hard_adc_obj_t {
    mp_obj_base_t base;
    void* adc_device;
	int resolution;
	uint8_t channel;
	uint8_t	open_flag;
} machine_hard_adc_obj_t;


mp_obj_t machine_hard_adc_make_new(const mp_obj_type_t *type, size_t n_args, 
                                            size_t n_kw, const mp_obj_t *all_args) 
{
    char adc_dev_name[6]={0};
    int id = 0;
    device_info_t* adc_device = NULL;
    
    if (n_args != 2)
    {
        mp_raise_ValueError("The amount of parameters are wrong! Input two arguments please !!\n");
        return mp_const_none;
    }
    
    id = mp_obj_get_int(all_args[0]);
    if (id == -1)
    {
        strcpy(adc_dev_name, "adc");
    } 
    else 
    {
        snprintf(adc_dev_name, sizeof(adc_dev_name), "adc%d", id);
    }

    adc_device = mpycall_device_find(adc_dev_name);
    if (adc_device == NULL)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) doesn't exist", adc_dev_name));
        return mp_const_none;
    }

    // create new adc object
    machine_hard_adc_obj_t *self = m_new_obj(machine_hard_adc_obj_t);
    self->base.type = &mp_module_adc;
    self->adc_device = adc_device;
    self->channel = mp_obj_get_int(all_args[1]);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_adc_read(size_t n_args, const mp_obj_t *args) 
{
    MP_ADC_CHECK_OPEN(args[0]);
    device_info_t *device = (device_info_t *)(self->adc_device);
    int vol = 0;

    if (n_args == 2)
    {
        self->channel = mp_obj_get_int(args[1]);
    }

    device->ops->read(device->owner.name, self->channel, &vol, sizeof(vol));
    if (vol <= 0)
    {
        mp_raise_ValueError("ADC channel is invalid.");
    }
    return mp_obj_new_int(vol);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_read_obj, 1, 2, machine_adc_read);

STATIC mp_obj_t machine_adc_init(size_t n_args, const mp_obj_t *args) 
{
    machine_hard_adc_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    device_info_t *device = (device_info_t *)(self->adc_device);
    
    if (n_args == 2)
    {
        self->channel = mp_obj_get_int(args[1]);
        /* the channel is need to-do */
    }

    OPEN_MP_MACHINE_DEVICE(self->open_flag, 
                        device->ops->open, 
                        device->owner.name);
    device->ops->ioctl(device, MP_MACHINE_OP_ENABLE, NULL);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_adc_init_obj, 1, 2, machine_adc_init);


STATIC mp_obj_t machine_adc_deinit(mp_obj_t self_in) 
{
    machine_hard_adc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    device_info_t *device = (device_info_t *)(self->adc_device);
    
    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        device->ops->ioctl, 
                        device, MP_MACHINE_OP_DISABLE, NULL);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_adc_deinit_obj, machine_adc_deinit);

STATIC const mp_rom_map_elem_t mp_module_adc_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_adc_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_adc_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_adc_read_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_adc_globals, mp_module_adc_globals_table);

const mp_obj_type_t mp_module_adc = {
    { &mp_type_type },
	.name = MP_QSTR_ADC,
	.make_new = machine_hard_adc_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_module_adc_globals,
};
#endif

