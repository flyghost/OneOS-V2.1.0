#include "usr_misc.h"
#include "os_util.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/objlist.h"
#include "cmiot_user.h"
#include "board.h"

STATIC mp_obj_t fota_upgrade(size_t n_args, const mp_obj_t *args) 
{
    mp_log("Start upgrade.");
    mp_arg_check_num(n_args, 0, 0, 1, false);
    
    cmiot_int8 rst = cmiot_upgrade();
    mp_log("Upgrade rst:%d.", rst);
    if(0 == rst)
    {
        mp_log("Ready to reset.");
        //os_shutdown(SHUTDOWN_RESET);
        HAL_NVIC_SystemReset();
    }
    
    return mp_const_none;
}

STATIC mp_obj_t fota_upgrade_report(size_t n_args, const mp_obj_t *args) 
{
    mp_log("Start upgrade report.");
    mp_arg_check_num(n_args, 0, 0, 1, false);
    
    cmiot_int8 rst = cmiot_report_upgrade();
    mp_log("Report upgrade rst:%d.", rst);
    
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fota_upgrade_obj, 0, 1, fota_upgrade);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fota_upgrade_report_obj, 0, 1, fota_upgrade_report);


STATIC const mp_rom_map_elem_t mp_module_fota_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_fota) },
    { MP_ROM_QSTR(MP_QSTR_upgrade), MP_ROM_PTR(&fota_upgrade_obj) },
    { MP_ROM_QSTR(MP_QSTR_upgrade_report), MP_ROM_PTR(&fota_upgrade_report_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_fota_globals, mp_module_fota_globals_table);

const mp_obj_module_t mp_module_fota = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_fota_globals,
};

