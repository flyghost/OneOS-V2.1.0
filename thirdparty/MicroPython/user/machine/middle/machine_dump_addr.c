#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/gc.h"


#define MP_VERIFY_PTR(ptr) ( \
        ((uintptr_t)(ptr) & (MICROPY_BYTES_PER_GC_BLOCK - 1)) == 0      /* must be aligned on a block */ \
        && ptr >= (void*)MP_STATE_MEM(gc_pool_start)     /* must be above start of pool */ \
        && ptr < (void*)MP_STATE_MEM(gc_pool_end)        /* must be below end of pool */ \
    )

#define MP_BLOCK_FROM_PTR(ptr) (((byte*)(ptr) - MP_STATE_MEM(gc_pool_start)) / MICROPY_BYTES_PER_GC_BLOCK)

#define MP_BLOCK_SHIFT(block) (2 * ((block) & (4 - 1)))
#define MP_ATB_GET_KIND(block) ((MP_STATE_MEM(gc_alloc_table_start)[(block) / 4] >> MP_BLOCK_SHIFT(block)) & 3)

#define dump_addr_info(info)	mp_hal_stdout_tx_strn(info, strlen(info))
		
STATIC mp_obj_t mp_dumpaddr (mp_obj_t inaddr) {
    unsigned int addr = mp_obj_get_int(inaddr);
	unsigned char status = 4;

	if (MP_VERIFY_PTR((void*)addr)) { 
		size_t block = MP_BLOCK_FROM_PTR(addr);
		status = MP_ATB_GET_KIND(block);
		switch (status){
			case 0: dump_addr_info("[free block addr]\n"); break;
			case 1: dump_addr_info("[head block addr]\n"); break;
			case 2: dump_addr_info("[tail block addr]\n"); break;
			case 3: dump_addr_info("[mark block addr]\n"); break;
			default: dump_addr_info("[error block addr]\n"); 
		}
		/*
						if(status == 0)
						{
							mp_hal_stdout_tx_strn("[free block addr]\n", strlen("[free block addr]\n"));
						}
						else if(status == 1)
						{
							mp_hal_stdout_tx_strn("[head block addr]\n", strlen("[head block addr]\n"));
						}
						else if(status == 2)
						{
							mp_hal_stdout_tx_strn("[tail block addr]\n", strlen("[tail block addr]\n"));
						}
						else if(status == 3)
						{
							mp_hal_stdout_tx_strn("[mark block addr]\n", strlen("[mark block addr]\n"));
						}
						else
						{
							mp_hal_stdout_tx_strn("[error block addr]\n", strlen("[error block addr]\n"));
						}
						*/
        } else {
			dump_addr_info("[addr out of range]\n");
			//mp_hal_stdout_tx_strn("[addr out of range]\n", strlen("[addr out of range]\n"));
		}
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mpdumpaddr_obj, mp_dumpaddr);

STATIC const mp_rom_map_elem_t mp_module_dumpaddr_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_dumpaddr) },
	{ MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mpdumpaddr_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_dumpaddr_globals, mp_module_dumpaddr_globals_table);

const mp_obj_module_t mp_module_dumpaddr = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_dumpaddr_globals,
};
