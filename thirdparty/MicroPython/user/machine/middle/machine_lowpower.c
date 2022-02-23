#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include "usr_lpower.h"

STATIC mp_obj_t mplowpower (mp_obj_t mode) {
    
    //Enter_Lpower(mp_obj_get_int(mode));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mplowpower_obj, mplowpower);

STATIC const mp_rom_map_elem_t mp_module_lpower_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_Lpower) },
	{ MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mplowpower_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_lpower_globals, mp_module_lpower_globals_table);

const mp_obj_module_t mp_module_lpower = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_lpower_globals,
};
