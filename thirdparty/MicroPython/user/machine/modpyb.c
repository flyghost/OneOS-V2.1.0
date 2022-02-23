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
#include "middle/include/portmodules.h"
//#include "modmachine.h"

#if (MICROPY_PY_MACHINE)
#include "vfs.h"
#include "utime_mphal.h"
#include "usr_pin.h"
#include "middle/include/machine_spi.h"
#include "middle/include/machine_i2c.h"
#include "middle/include/machine_uart.h"
#include "middle/include/machine_wdt.h"
#include "middle/include/machine_timer.h"
#include "middle/include/machine_can.h"
#include "middle/include/machine_dac.h"
#include "usr_adc.h"
#include "usr_pwm.h"
#include "usr_rtc.h"
#include "usr_pm.h"


/// \function elapsed_millis(start)
/// Returns the number of milliseconds which have elapsed since `start`.
///
/// This function takes care of counter wrap, and always returns a positive
/// number. This means it can be used to measure periods upto about 12.4 days.
///
/// Example:
///     start = pyb.millis()
///     while pyb.elapsed_millis(start) < 1000:
///         # Perform some operation

//STATIC mp_obj_t pyb_elapsed_millis(mp_obj_t start) {
//    uint32_t startMillis = mp_obj_get_int(start);
//    uint32_t currMillis = mp_hal_ticks_ms();
//    return MP_OBJ_NEW_SMALL_INT((currMillis - startMillis) & 0x3fffffff);
//}
//STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_elapsed_millis_obj, pyb_elapsed_millis);


/// \function elapsed_micros(start)
/// Returns the number of microseconds which have elapsed since `start`.
///
/// This function takes care of counter wrap, and always returns a positive
/// number. This means it can be used to measure periods upto about 17.8 minutes.
///
/// Example:
///     start = pyb.micros()
///     while pyb.elapsed_micros(start) < 1000:
///         # Perform some operation
//STATIC mp_obj_t pyb_elapsed_micros(mp_obj_t start) {
//    uint32_t startMicros = mp_obj_get_int(start);
//    uint32_t currMicros = mp_hal_ticks_us();
//    return MP_OBJ_NEW_SMALL_INT((currMicros - startMicros) & 0x3fffffff);
//}
//STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_elapsed_micros_obj, pyb_elapsed_micros);

MP_DECLARE_CONST_FUN_OBJ_KW(pyb_main_obj); // defined in main.c

STATIC mp_obj_t machine_getall(size_t n_args, const mp_obj_t *args) {
	mpycall_device_listall();
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_getall_obj, 0, 1, machine_getall);

STATIC mp_obj_t machine_deepsleep(void) 
{
//#ifdef OS_USING_LPMGR
//	os_lpmgr_request(SYS_SLEEP_MODE_SHUTDOWN);
//#endif
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(machine_deepsleep_obj, machine_deepsleep);


STATIC const mp_rom_map_elem_t pyb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },
/*
    { MP_ROM_QSTR(MP_QSTR_hard_reset), MP_ROM_PTR(&machine_reset_obj) },

    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&machine_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_unique_id), MP_ROM_PTR(&machine_unique_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_repl_info), MP_ROM_PTR(&pyb_set_repl_info_obj) },

    { MP_ROM_QSTR(MP_QSTR_wfi), MP_ROM_PTR(&pyb_wfi_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable_irq), MP_ROM_PTR(&pyb_disable_irq_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_irq), MP_ROM_PTR(&pyb_enable_irq_obj) },

    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&machine_sleep_obj) },
    */
    { MP_ROM_QSTR(MP_QSTR_standby), MP_ROM_PTR(&machine_deepsleep_obj) },
//    { MP_ROM_QSTR(MP_QSTR_main), MP_ROM_PTR(&pyb_main_obj) },
//    { MP_ROM_QSTR(MP_QSTR_repl_uart), MP_ROM_PTR(&pyb_repl_uart_obj) },
/*
    { MP_ROM_QSTR(MP_QSTR_millis), MP_ROM_PTR(&mp_utime_ticks_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_elapsed_millis), MP_ROM_PTR(&pyb_elapsed_millis_obj) },
    { MP_ROM_QSTR(MP_QSTR_micros), MP_ROM_PTR(&mp_utime_ticks_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_elapsed_micros), MP_ROM_PTR(&pyb_elapsed_micros_obj) },
    { MP_ROM_QSTR(MP_QSTR_delay), MP_ROM_PTR(&mp_utime_sleep_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_udelay), MP_ROM_PTR(&mp_utime_sleep_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_mount), MP_ROM_PTR(&mp_os_mount_obj) },
*/

//#if MICROPY_HW_ENABLE_RNG
//    { MP_ROM_QSTR(MP_QSTR_rng), MP_ROM_PTR(&pyb_rng_get_obj) },
//#endif
//
	
//
#if MICROPY_PY_MACHINE_PIN
    { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },
#endif

#if (MICROPY_PY_MACHINE_SPI && MICROPY_PY_MACHINE_PIN)
	{ MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_spi_type) },
#endif

#if MICROPY_PY_MACHINE_UART
	{ MP_ROM_QSTR(MP_QSTR_UART), MP_ROM_PTR(&machine_uart_type) },
#endif

#if MICROPY_PY_MACHINE_WDT
	{ MP_ROM_QSTR(MP_QSTR_WDT), MP_ROM_PTR(&machine_wdt_type) },
#endif
	
#if MICROPY_PY_MACHINE_ADC
	{ MP_ROM_QSTR(MP_QSTR_ADC), MP_ROM_PTR(&mp_module_adc) },
#endif

#if MICROPY_PY_MACHINE_PWM
	{ MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_PTR(&mp_module_pwm) },
#endif

#if MICROPY_PY_MACHINE_RTC
	  { MP_ROM_QSTR(MP_QSTR_RTC), MP_ROM_PTR(&machine_hard_rtc_type) },
#endif

#if MICROPY_PY_MACHINE_TIMER
    { MP_ROM_QSTR(MP_QSTR_Timer), MP_ROM_PTR(&machine_timer_type) },
#endif

{ MP_ROM_QSTR(MP_QSTR_getall), MP_ROM_PTR(&machine_getall_obj) },

//    { MP_ROM_QSTR(MP_QSTR_ExtInt), MP_ROM_PTR(&extint_type) },
//
//#if MICROPY_HW_ENABLE_SERVO
//    { MP_ROM_QSTR(MP_QSTR_pwm), MP_ROM_PTR(&pyb_pwm_set_obj) },
//    { MP_ROM_QSTR(MP_QSTR_servo), MP_ROM_PTR(&pyb_servo_set_obj) },
//    { MP_ROM_QSTR(MP_QSTR_Servo), MP_ROM_PTR(&pyb_servo_type) },
//#endif
//
//#if MICROPY_HW_HAS_SWITCH
//    { MP_ROM_QSTR(MP_QSTR_Switch), MP_ROM_PTR(&pyb_switch_type) },
//#endif
//
//#if MICROPY_HW_HAS_FLASH
//    { MP_ROM_QSTR(MP_QSTR_Flash), MP_ROM_PTR(&pyb_flash_type) },
//#endif
//
//#if MICROPY_HW_HAS_SDCARD
//    { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&pyb_sdcard_obj) }, // now obsolete
//    { MP_ROM_QSTR(MP_QSTR_SDCard), MP_ROM_PTR(&pyb_sdcard_type) },
//#endif
//
//#if defined(MICROPY_HW_LED1)
//    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&pyb_led_type) },
//#endif
#if MICROPY_PY_MACHINE_I2C
	  { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_i2c_type) },
#endif

#if MICROPY_PY_MACHINE_PM
	  { MP_ROM_QSTR(MP_QSTR_pm), MP_ROM_PTR(&machine_hard_pm_type) },
#endif

#if MICROPY_PY_MACHINE_CAN
    { MP_ROM_QSTR(MP_QSTR_CAN), MP_ROM_PTR(&mp_machine_can_type) },
#endif
//
//    { MP_ROM_QSTR(MP_QSTR_ADCAll), MP_ROM_PTR(&pyb_adc_all_type) },
//
#if MICROPY_PY_MACHINE_DAC
    { MP_ROM_QSTR(MP_QSTR_DAC), MP_ROM_PTR(&machine_dac_type) },
#endif
//
//#if MICROPY_HW_HAS_MMA7660
//    { MP_ROM_QSTR(MP_QSTR_Accel), MP_ROM_PTR(&pyb_accel_type) },
//#endif
//
//#if MICROPY_HW_HAS_LCD
//    { MP_ROM_QSTR(MP_QSTR_LCD), MP_ROM_PTR(&pyb_lcd_type) },
//#endif
};

STATIC MP_DEFINE_CONST_DICT(pyb_module_globals, pyb_module_globals_table);

const mp_obj_module_t pyb_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&pyb_module_globals,
};
#endif
