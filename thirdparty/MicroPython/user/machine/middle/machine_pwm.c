
#include "py/runtime.h"

#if (MICROPY_PY_MACHINE_PWM)
#include "usr_pwm.h"
#include "py/obj.h"
#include <string.h>
#include "py/builtin.h"

#define MP_PWM_OPEN_CHECK(self)		MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, machine_hard_pwm_obj_t, "pwm") 

typedef struct _machine_hard_pwm_obj_t 
{
    mp_obj_base_t base;
    int duty;
    uint8_t channal;
    uint8_t	open_flag;
    float freq;
    device_info_t * device;
} machine_hard_pwm_obj_t;

mp_obj_t machine_hard_pwm_make_new(const mp_obj_type_t *type, size_t n_args, 
                                            size_t n_kw, const mp_obj_t *all_args) 
{
    char pwm_dev_name[10]={0};
    machine_hard_pwm_obj_t *self = NULL;
    
    snprintf(pwm_dev_name, sizeof(pwm_dev_name), "pwm_tim%d", mp_obj_get_int(all_args[0]));
    device_info_t * device = mpycall_device_find(pwm_dev_name);
    if (device == NULL)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) doesn't exist", 
                                                pwm_dev_name));
        return mp_const_none;
    }
    if (n_args < 4)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Insufficient Parameters!"));
        return mp_const_none;
    }
    self = m_new_obj(machine_hard_pwm_obj_t);
    self->base.type = &mp_module_pwm;
    self->device = device;
    self->channal = mp_obj_get_int(all_args[1]);
    self->freq = mp_obj_get_float(all_args[2]);
    self->duty = mp_obj_get_int(all_args[3]);

    /*
    period = 1e9 / self->freq;
    pulse = period * ((float)(self->duty) / 100);
    //os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",
    //              self->freq, period, self->duty, pulse);
    
    ret = device->ops->open(device, 0);
    if (0 != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not open", pwm_dev_name));
    }

    ret = device->ops->ioctl(device, USR_PWM_CMD_SET_PERIOD, &period);
    if (0 != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not set freq , please check if freq  is valid", pwm_dev_name));
    }

    ret = device->ops->write(device, self->channal, &pulse, sizeof(pulse));
    if (0 != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not set  duty, please check if  duty is valid", pwm_dev_name));
    }

    ret = device->ops->ioctl(device, USR_PWM_CMD_ENABLE, &self->channal);
    if (0 != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "(%s) can not enable", pwm_dev_name));
    }
    */
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t machine_pwm_init(mp_obj_t self_in, mp_obj_t freq, mp_obj_t duty) 
{
    int  ret = MP_ERROR;
    machine_hard_pwm_obj_t *self = MP_OBJ_TO_PTR(self_in);
    struct pwm_config pwm_args = {0};

    OPEN_MP_MACHINE_DEVICE(self->open_flag, 
                        self->device->ops->open,
                        self->device->owner.name);

    pwm_args.channel = self->channal;
    pwm_args.frequency = mp_obj_get_float(freq);
    pwm_args.duty = mp_obj_get_int(duty);

    //os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",pwm_args.frequency,period,pwm_args.duty,pulse);
    //os_kprintf("channel:%d\n",pwm_args.channel);

    ret = self->device->ops->ioctl(self->device, MPY_PWM_CMD_SET_PERIOD, &pwm_args);
    if (MP_EOK != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "freq = %f can not init\n", 
                                                pwm_args.frequency));
    }

    ret = self->device->ops->ioctl(self->device, MPY_PWM_CMD_SET_PULSE, &pwm_args);
    if (MP_EOK != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "duty = %d can not init\n", 
                                                pwm_args.duty));
    }
    
    ret = self->device->ops->ioctl(self->device, MP_MACHINE_OP_ENABLE, &pwm_args.channel);
    if (MP_EOK != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not enable"));
    }
    self->freq = pwm_args.frequency;
    self->duty = pwm_args.duty;
    
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(machine_pwm_init_obj, machine_pwm_init);

STATIC mp_obj_t machine_pwm_deinit(mp_obj_t self_in) 
{
    int  ret = MP_ERROR;
    machine_hard_pwm_obj_t *self = MP_OBJ_TO_PTR(self_in);

    struct pwm_config pwm_args = {0};
    pwm_args.channel = ((machine_hard_pwm_obj_t*)self)->channal;

    ret = self->device->ops->ioctl(self->device, MP_MACHINE_OP_DISABLE, &pwm_args.channel);
    if (MP_EOK != ret)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "channel = %d can not deinit\n", 
                                                ((machine_hard_pwm_obj_t*)self)->channal));
        return mp_const_none;
    }

    CLOSE_MP_MACHINE_DEVICE(self->open_flag, 
                        self->device->ops->close, 
                        self->device->owner.name);
    self->duty = 0;
    self->freq = 0;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_deinit_obj, machine_pwm_deinit);

STATIC mp_obj_t machine_pwm_freq(size_t n_args, const mp_obj_t *args) 
{
    int  ret = 0;
    struct pwm_config pwm_args = {0};
    
    MP_PWM_OPEN_CHECK(args[0]);
    //machine_hard_pwm_obj_t* pwm_obj = (machine_hard_pwm_obj_t*)self;

    if(n_args > 2 || !self || !self->device)
    {
        mp_raise_ValueError("args error\n");
    }

    pwm_args.channel = self->channal;
    if(n_args == 2)
    {
        pwm_args.frequency = mp_obj_get_float(args[1]);
        ret = self->device->ops->ioctl(self->device, MPY_PWM_CMD_SET_PERIOD, &pwm_args);
        if (0 != ret)
        {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set freq\n"));
        }
        self->freq = pwm_args.frequency;

        pwm_args.duty = self->duty;
        ret = self->device->ops->ioctl(self->device, MPY_PWM_CMD_SET_PULSE, &pwm_args);
        if (0 != ret)
        {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set duty\n"));
        }
    }
    else
    {
        return mp_obj_new_float(self->freq);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_freq_obj, 1, 2, machine_pwm_freq);


STATIC mp_obj_t machine_pwm_duty(size_t n_args, const mp_obj_t *args) 
{
    struct pwm_config pwm_args = {0};
    
    MP_PWM_OPEN_CHECK(args[0]);

    if(n_args > 2 || !self || !self->device)
    {
        mp_raise_ValueError("args error\n");
    }

    if(n_args == 2)
    {
        pwm_args.channel = self->channal;
        pwm_args.frequency = self->freq;
        pwm_args.duty = mp_obj_get_int(args[1]);
        //os_kprintf("freq:%d, period:%d; duty:%d, pluse:%d\n",pwm_obj->freq,period,duty,pulse);
        int ret = self->device->ops->ioctl(self->device, MPY_PWM_CMD_SET_PULSE, &pwm_args);
        if (0 != ret)
        {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pwm can not set duty\n"));
        }
        self->duty = pwm_args.duty;
    }
    else
    {
        return MP_OBJ_NEW_SMALL_INT(self->duty);
    }

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_obj, 1, 2, machine_pwm_duty);

STATIC const mp_rom_map_elem_t mp_module_pwm_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pwm_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_pwm_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_duty), MP_ROM_PTR(&machine_pwm_duty_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_pwm_globals, mp_module_pwm_globals_table);


const mp_obj_type_t mp_module_pwm = {
    { &mp_type_type },
		.name = MP_QSTR_PWM,
	  .make_new = machine_hard_pwm_make_new,
    .locals_dict = (mp_obj_dict_t*)&mp_module_pwm_globals,
};

#endif // MICROPY_PY_MACHINE_PWM
