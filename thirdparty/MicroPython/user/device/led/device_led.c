#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>
#include "model_device.h"
#include "usr_led.h"

/***********************led color************************/
typedef enum MP_DEVICE_LED_COLOR {
    LED_RED     =((uint32_t)0x00000000),
    LED_GREEN   =((uint32_t)0x00000001),
    LED_BLUE    =((uint32_t)0x00000002),
    LED_COLOR_ERROR,
}MP_DEVICE_LED_COLOR_T;

#define  MP_LED_OPEN_CHECK(self)    MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self, device_led_obj_t, "led");

int8_t g_led_map[LED_COLOR_ERROR] ={MP_ERROR};

const mp_obj_type_t mp_led_type ;

typedef struct _device_touchkey_obj_t {
    mp_obj_base_t   base;
    uint8_t         color;
    uint8_t         open_flag;
    int32_t         pin;
    device_info_t *device;
}device_led_obj_t;

mp_obj_t mp_led_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 2, 2, false);        
    device_info_t *led_device = mpycall_device_find("LED");

    if (led_device == NULL)
    {
         nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "LED doesn't exist", mp_obj_get_int(args[0])));
    }

    device_led_obj_t *self = m_new_obj(device_led_obj_t);
    self->base.type = &mp_led_type;
    self->device = led_device;
    self->pin = mp_obj_get_int(args[0]);
    self->color = mp_obj_get_int(args[1]);
    return (mp_obj_t)self;
}

STATIC mp_obj_t led_init(mp_obj_t self_in)
{
    device_led_obj_t *led = self_in;
    device_info_t *dev = USER_DEV(led);

    dev->id = led->pin;
    OPEN_MP_MACHINE_DEVICE(led->open_flag, 
                        dev->ops->ioctl, 
                        dev, MP_MACHINE_OP_INIT, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_init_obj, led_init);


STATIC mp_obj_t led_deinit(mp_obj_t self_in)
{
    device_led_obj_t *led = self_in;
    device_info_t *dev = USER_DEV(led);
    dev->id = led->pin;
    
    CLOSE_MP_MACHINE_DEVICE(led->open_flag, 
                        dev->ops->ioctl, 
                        dev, MP_MACHINE_OP_DEINIT, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_deinit_obj, led_deinit);

STATIC mp_obj_t led_setter(mp_obj_t self_in, mp_obj_t pin, mp_obj_t color_in)
{ 
    MP_LED_OPEN_CHECK(self_in);
    int color = mp_obj_get_int(color_in);
    
    if (color >= LED_COLOR_ERROR)
    {
        mp_raise_ValueError("the led num is wrong, please check!\n");
    }
    g_led_map[color] = mp_obj_get_int(pin);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(led_setter_obj, led_setter);

STATIC mp_obj_t led_on(mp_obj_t self_in)
{
    MP_LED_OPEN_CHECK(self_in);
    device_led_obj_t *p_self = MP_OBJ_TO_PTR(self_in);

    p_self->device->id = p_self->pin;
    p_self->device->ops->ioctl(p_self->device, MP_MACHINE_OP_ENABLE, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_on_obj, led_on);

STATIC mp_obj_t led_off(mp_obj_t self_in)
{
    MP_LED_OPEN_CHECK(self_in);
    device_led_obj_t *p_self = MP_OBJ_TO_PTR(self_in);

    p_self->device->id = p_self->pin;
    p_self->device->ops->ioctl(p_self->device, MP_MACHINE_OP_DISABLE, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(led_off_obj, led_off);

STATIC const mp_rom_map_elem_t mp_module_led_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init),    MP_ROM_PTR(&led_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit),  MP_ROM_PTR(&led_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_open),    MP_ROM_PTR(&led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_close),   MP_ROM_PTR(&led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_RED),     MP_ROM_INT(LED_RED) },
    { MP_ROM_QSTR(MP_QSTR_BLUE),    MP_ROM_INT(LED_BLUE) },
    { MP_ROM_QSTR(MP_QSTR_GREEN),   MP_ROM_INT(LED_GREEN) },
};

STATIC MP_DEFINE_CONST_DICT(device_led_locals_dict, mp_module_led_globals_table);

STATIC void mp_led_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    device_led_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "led (addr=%p, id=%u color=%u)", self, self->pin, self->color);
}

const mp_obj_type_t mp_led_type =
{
    {&mp_type_type},
    .name = MP_QSTR_LED,
    .print = mp_led_print,
    .make_new = mp_led_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_led_locals_dict,
};


