/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Armink (armink.ztl@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>

#include "py/runtime.h"
#include "py/gc.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "lib/utils/pyexec.h"
#include "portmodules.h"
#include "vfs.h"
#include "utime_mphal.h"

//添加类必须要先extern
//extern const mp_obj_type_t mp_motor_type;
//extern const mp_obj_type_t mp_touchkey_type;
//extern const mp_obj_type_t mp_oled_type;
//extern const mp_obj_type_t mp_fontlibrary_type;
#ifdef MICROPY_DEVICE_HUMITURE
extern const mp_obj_type_t mp_humiture_type;
#endif
#ifdef MICROPY_DEVICE_ALS_PS
extern const mp_obj_type_t mp_als_ps_type;
#endif
#ifdef MICROPY_DEVICE_LED
extern const mp_obj_type_t mp_led_type;
#endif
#ifdef MICROPY_DEVICE_KEY
extern const mp_obj_type_t mp_key_type;
#endif
#ifdef MICROPY_DEVICE_SIX_AXIS
extern const mp_obj_type_t mp_six_axis_type;
#endif
#ifdef MICROPY_DEVICE_LCD
extern const mp_obj_type_t mp_lcd_type;
#endif
#ifdef MICROPY_DEVICE_BEEP
extern const mp_obj_type_t mp_beep_type;
#endif
#ifdef MICROPY_DEVICE_AUDIO
extern const mp_obj_type_t mp_audio_type;
#endif
//extern const mp_obj_type_t mp_voice_type;
//extern const mp_obj_type_t mp_sesecure_type;
//extern const mp_obj_type_t mp_ble_type;


STATIC mp_obj_t device_elapsed_millis(mp_obj_t start) {
    uint32_t startMillis = mp_obj_get_int(start);
    uint32_t currMillis = mp_hal_ticks_ms();
    return MP_OBJ_NEW_SMALL_INT((currMillis - startMillis) & 0x3fffffff);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(device_elapsed_millis_obj, device_elapsed_millis);

STATIC mp_obj_t device_elapsed_micros(mp_obj_t start) {
    uint32_t startMicros = mp_obj_get_int(start);
    uint32_t currMicros = mp_hal_ticks_us();
    return MP_OBJ_NEW_SMALL_INT((currMicros - startMicros) & 0x3fffffff);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(device_elapsed_micros_obj, device_elapsed_micros);

MP_DECLARE_CONST_FUN_OBJ_KW(pyb_main_obj); // defined in main.c

STATIC mp_obj_t machine_getall(size_t n_args, const mp_obj_t *args) {
	mpycall_device_listall();
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_getall_obj, 0, 1, machine_getall);


STATIC const mp_rom_map_elem_t device_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), 		MP_ROM_QSTR(MP_QSTR_machine) },
    { MP_ROM_QSTR(MP_QSTR_getall), 			MP_ROM_PTR(&machine_getall_obj) },
    { MP_ROM_QSTR(MP_QSTR_elapsed_micros), 	MP_ROM_PTR(&device_elapsed_micros_obj) },
    { MP_ROM_QSTR(MP_QSTR_elapsed_millis), 	MP_ROM_PTR(&device_elapsed_millis_obj) },

#ifdef MICROPY_DEVICE_HUMITURE
	{ MP_ROM_QSTR(MP_QSTR_Humiture), 		MP_ROM_PTR(&mp_humiture_type) },
#endif
#ifdef MICROPY_DEVICE_ALS_PS
	{ MP_ROM_QSTR(MP_QSTR_ALS_PS), 	MP_ROM_PTR(&mp_als_ps_type) },
#endif
#ifdef MICROPY_DEVICE_LED
	{ MP_ROM_QSTR(MP_QSTR_LED), 			MP_ROM_PTR(&mp_led_type) },
#endif
#ifdef MICROPY_DEVICE_KEY
	{ MP_ROM_QSTR(MP_QSTR_KEY), 			MP_ROM_PTR(&mp_key_type) },
#endif
#ifdef MICROPY_DEVICE_SIX_AXIS
	{ MP_ROM_QSTR(MP_QSTR_SIX_AXIS), 		MP_ROM_PTR(&mp_six_axis_type) },
#endif
#ifdef MICROPY_DEVICE_LCD
	{ MP_ROM_QSTR(MP_QSTR_LCD), 			MP_ROM_PTR(&mp_lcd_type) },
#endif
#ifdef MICROPY_DEVICE_BEEP
    { MP_ROM_QSTR(MP_QSTR_BEEP), 			MP_ROM_PTR(&mp_beep_type) },
#endif
#ifdef MICROPY_DEVICE_AUDIO
    { MP_ROM_QSTR(MP_QSTR_AUDIO), 			MP_ROM_PTR(&mp_audio_type) },
#endif
//    { MP_ROM_QSTR(MP_QSTR_OLED), MP_ROM_PTR(&mp_oled_type) },
//    { MP_ROM_QSTR(MP_QSTR_FontLib), MP_ROM_PTR(&mp_fontlibrary_type) },

//    { MP_ROM_QSTR(MP_QSTR_Battery), MP_ROM_PTR(&mp_battery_type) },
//    { MP_ROM_QSTR(MP_QSTR_Voice), MP_ROM_PTR(&mp_voice_type) },
//    { MP_ROM_QSTR(MP_QSTR_SESecure), MP_ROM_PTR(&mp_sesecure_type) },
//    { MP_ROM_QSTR(MP_QSTR_Bluetooth), MP_ROM_PTR(&mp_ble_type) },
};

STATIC MP_DEFINE_CONST_DICT(device_module_globals, device_module_globals_table);

const mp_obj_module_t mp_module_device = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&device_module_globals,
};
