#ifndef __MPCONFIGPORT_H__
#define __MPCONFIGPORT_H__
#include <stdint.h>
#include <stdio.h>

//config_tools 
#include <oneos_config.h>
#include <os_errno.h>
#include <os_task.h>
/*
*********************************************************************************************************
*                                        Manually define area
*********************************************************************************************************
*/
//user-defined config
//end user-defined config


/*
*********************************************************************************************************
*                                        automatic define area module 
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                        machine module 
*********************************************************************************************************
*/
#ifdef PKG_MICROPYTHON_MACHINE_BUS
#define MICROPY_PY_MACHINE          (1)
#endif

#ifdef MICROPY_PY_ADC
#define MICROPY_PY_MACHINE_ADC 		(1)
#endif

#ifdef MICROPY_PY_CAN
#define MICROPY_PY_MACHINE_CAN 		(1)
#endif

#ifdef MICROPY_PY_DAC
#define MICROPY_PY_MACHINE_DAC      (1)
#endif

#ifdef MICROPY_PY_I2C
#define MICROPY_PY_MACHINE_I2C      (1)
#define MICROPY_PY_MACHINE_I2C_MAKE_NEW machine_hard_i2c_make_new
#endif

#ifdef MICROPY_PY_SPI
#define MICROPY_PY_MACHINE_SPI      (1)
#endif

#ifdef MICROPY_PY_UART
#define MICROPY_PY_MACHINE_UART     (1)
#endif

#ifdef MICROPY_PY_PIN
#define MICROPY_PY_MACHINE_PIN 		(1)
#define MICROPY_PY_MACHINE_PIN_MAKE_NEW mp_pin_make_new
#endif

#ifdef MICROPY_PY_PWM
#define MICROPY_PY_MACHINE_PWM      (1)
#endif

#ifdef MICROPY_PY_WDT
#define MICROPY_PY_MACHINE_WDT      (1)
#endif

#ifdef MICROPY_PY_RTC
#define MICROPY_PY_MACHINE_RTC		(1)
#endif

#ifdef MICROPY_PY_TIMER
#define MICROPY_PY_MACHINE_TIMER	(1)
#endif

#ifdef MICROPY_PY_PM
#define MICROPY_PY_MACHINE_PM		(1)
#endif

#ifdef PKG_MICROPYTHON_DEVICE
#define MICROPY_PY_DEVICE			(1)
#endif

/*
*********************************************************************************************************
*                                        std-libraries and micro-libraries
*********************************************************************************************************
*/
#ifdef MICROPY_USING_THREAD
#define MICROPY_PY_THREAD 			(1)
#define MICROPY_PY_THREAD_GIL		(0)

#ifdef MP_THREAD_MAX_NUM_32
#define MP_THREAD_MAX_NUM_20
#endif
#ifndef MP_THREAD_MAX_NUM_10
#define MP_THREAD_MAX_NUM_10
#endif
#endif

#ifdef MICROPY_USING_MATH
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (1)
#endif

#ifdef MICROPY_USING_MATH_SPECIAL_FUNCTIONS
#define MICROPY_PY_MATH_SPECIAL_FUNCTIONS (1)
#endif

#ifdef MICROPY_USING_MATH_FACTORIAL
#define MICROPY_PY_MATH_FACTORIAL (1)
#endif

#ifdef MICROPY_USING_MATH_ISCLOSE
#define MICROPY_PY_MATH_ISCLOSE (1)
#endif

#ifdef MICROPY_USING_ARRAY
#define MICROPY_PY_ARRAY (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#else
#define MICROPY_PY_ARRAY (0)
#endif

#ifdef MICROPY_USING_UBINASCII
#define MICROPY_PY_UBINASCII (1)
#endif

#ifdef MICROPY_USING_UBINASCII_CRC32
#define MICROPY_PY_UBINASCII_CRC32 (1)
#endif

#ifdef MICROPY_USING_COLLECTIONS
#define MICROPY_PY_COLLECTIONS      (1)
#endif

#ifdef MICROPY_USING_COLLECTIONS_DEQUE
#define MICROPY_PY_COLLECTIONS_DEQUE (1)
#endif

#ifdef MICROPY_USING_COLLECTIONS_ORDEREDDICT
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#else
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (0)
#endif

#ifdef MICROPY_USING_COLLECTIONS_NAMEDTUPLE__ASDICT
#define MICROPY_PY_COLLECTIONS_NAMEDTUPLE__ASDICT (1)
#endif

#ifdef MICROPY_USING_UHEAPQ
#define MICROPY_PY_UHEAPQ (1)
#endif

#ifdef MICROPY_USING_UHASHLIB
#define MICROPY_PY_UHASHLIB			 (1)
#ifdef MICROPY_USING_UHASHLIB_SHA256
#define MICROPY_PY_UHASHLIB_SHA256   (1)
#endif
#ifdef MICROPY_USING_UHASHLIB_MD5
#define MICROPY_PY_UHASHLIB_MD5   (1)
#endif
#ifdef MICROPY_USING_UHASHLIB_SHA1
#define MICROPY_PY_UHASHLIB_SHA1   (1)
#endif
#endif

#ifdef MICROPY_USING_UCTYPES
#define MICROPY_PY_UCTYPES           (1)
#endif
#ifdef MICROPY_USING_UCRYPTOLIB
#define MICROPY_PY_UCRYPTOLIB        (1)
#endif
#ifdef MICROPY_USING_URANDOM
#define MICROPY_PY_URANDOM           (1)
#endif

#ifdef MICROPY_USING_MO_NETWORK
#define MICROPY_PY_MO_NETWORK           (1)
#else
#define MICROPY_PY_MO_NETWORK           (0)
#endif
#ifdef MICROPY_USING_WIFISCAN
#define MICROPY_PY_WIFISCAN           (1)
#else
#define MICROPY_PY_WIFISCAN           (0)
#endif

#ifdef MICROPY_USING_USSL
#define MICROPY_PY_USSL              (1)
#ifdef MICROPY_USING_SSL_AXTLS
#define MICROPY_SSL_AXTLS            (1)
#endif
#ifdef MICROPY_USING_SSL_MBEDTLS
#define MICROPY_SSL_MBEDTLS          (1)
#endif
#endif

#ifdef MICROPY_USING_USELECT
#define MICROPY_PY_USELECT           (1)
#endif

#ifdef MICROPY_USING_USOCKET
#define MICROPY_PY_USOCKET          (1)
#endif

#ifdef MICROPY_USING_UJSON
#define MICROPY_PY_UJSON (1)
#else
#define MICROPY_PY_UJSON (0)
#endif

#ifdef MICROPY_USING_URE
#define MICROPY_PY_URE (1)
#endif

#ifdef MICROPY_USING_URE_DEBUG
#define MICROPY_PY_URE_DEBUG (1)
#endif

#ifdef MICROPY_USING_URE_MATCH_GROUPS
#define MICROPY_PY_URE_MATCH_GROUPS (1)
#endif

#ifdef MICROPY_USING_URE_MATCH_SPAN_START_END
#define MICROPY_PY_URE_MATCH_SPAN_START_END (1)
#endif

#ifdef MICROPY_USING_URE_SUB
#define MICROPY_PY_URE_SUB (1)
#endif

#ifdef MICROPY_USING_UZLIB
#define MICROPY_PY_UZLIB   (1)
#endif

#if defined(MICROPY_PY_RTC) && defined(MICROPY_USING_UTIME)
#define MICROPY_PY_UTIME            (1)
#endif

#ifdef MICROPY_USING_FOTA
#define MICROPY_PY_FOTA          (1)
#endif

#ifdef MICROPY_USING_SYSMISC
#define MICROPY_PY_SYSMISC          (1)
#endif

#ifdef MICROPY_USING_VIRTUAL_AT
#define MICROPY_PY_VIRTUAL_AT        (1)
#endif

#ifdef MICROPY_USING_CODECS
#define MICROPY_PY_CODECS          (1)
#endif

/*
*********************************************************************************************************
*                                        Micropython components
*********************************************************************************************************
*/
#ifdef MICROPY_USING_FILESYSTEM
#define MICROPY_VFS_FAT             (1)
#endif

// options to control how MicroPython is built

// print debugging info
//#define MICROPY_DEBUG_VERBOSE		(1)

//use low power
#define MICROPY_PY_LPOWER      		(1)

//add by zxc
#define MICROPY_KBD_EXCEPTION       (1)
#define MICROPY_HELPER_REPL         (1)


#define MP_ENDIANNESS_LITTLE        (1)

#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_ALLOC_PATH_MAX      (512)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_COMP_MODULE_CONST   (0)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (1)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (0)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_DICT_FROMKEYS (0)
#define MICROPY_PY_BUILTINS_STR_COUNT (0)
#define MICROPY_PY_BUILTINS_STR_OP_MODULO (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_SYS              (1)

#define MICROPY_PY_UERRNO 			 (1)
#define MICROPY_PY_UHASHLIB			 (1)

//#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_NONE)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_USE_INTERNAL_PRINTF (0)

// control over Python builtins
#define MICROPY_PY_FUNCTION_ATTRS   (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_STR_CENTER (1)
#define MICROPY_PY_BUILTINS_STR_PARTITION (1)
#define MICROPY_PY_BUILTINS_STR_SPLITLINES (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_SLICE_ATTRS (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS (1)
#define MICROPY_PY_BUILTINS_INPUT (1)
#define MICROPY_PY_BUILTINS_POW3 (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_HELP    (1)

#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY___FILE__         (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ATTRTUPLE        (1)

#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_STREAMS_NON_BLOCK   (1)
#define MICROPY_MODULE_WEAK_LINKS   (1)
#define MICROPY_CAN_OVERRIDE_BUILTINS (1)
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_USE_INTERNAL_PRINTF (0)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_MODULE_FROZEN_MPY   (1)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)
#define MICROPY_READER_VFS          (0)

#define MICROPY_PY_ONENET			(0)
#define MICROPY_DUMP_ADDR			(1)



#define MICROPY_PY_OS_DUPTERM       (0)
#define MICROPY_VFS                 (0)




#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_PY_UTIMEQ           (0)
#ifdef MICROPY_USING_POSITION
#define MICROPY_PY_POSITION          (1)
#endif

/*****************************************************************************/

// type definitions for the specific machine
#define MICROPYTHON_USING_UOS

#ifdef MICROPYTHON_USING_UOS
#define MICROPY_PY_IO_FILEIO         (1)
#define MICROPY_PY_MODUOS            (1)
#define MICROPY_PY_MODUOS_FILE       (1)
#define MICROPY_PY_SYS_STDFILES      (1)
#define MICROPY_READER_POSIX         (1)
#define MICROPY_PY_BUILTINS_COMPILE  (1)
#define MICROPY_PY_BUILTINS_EXECFILE (1)
#endif


#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define MICROPY_HEAP_SIZE	(MICROPYTHON_RAM_SIZE * 1024)
//#define MICROPY_HEAP_ADDR	MICROPYTHON_RAM_START //(MICROPYTHON_RAM_START + MICROPYTHON_RAM_SIZE * 1024)

#if defined  (STM32_SRAM2_START) 
#define MICROPY_HEAP_ADDR   (STM32_SRAM2_START)
#else
#define MICROPY_HEAP_ADDR   (HEAP_END - MICROPY_HEAP_SIZE)
#endif

#define UINT_FMT "%lu"
#define INT_FMT "%ld"

#ifndef errno
#define errno
#endif

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

#define MP_ERROR		-1
#define MP_EOK			0
// dummy print
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_stream(str, len)


// We need to provide a declaration/definition of alloca()

//by zxc
#define MICROPY_HW_BOARD_NAME          "MicroPython board"

//#if (MICROPY_PY_IO && (!MICROPY_VFS)) 
//#ifndef OS_USING_VFS
//#error MicroPython need filesystem, enable virtual-file-system please!
//#endif
//#endif


// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS 		{ MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

extern const struct _mp_obj_module_t mp_module_os;
#define MODUOS_PORT_BUILTIN_MODULES         { MP_ROM_QSTR(MP_QSTR_uos), MP_ROM_PTR(&mp_module_os ) },



#if MICROPY_PY_DEVICE
extern const struct _mp_obj_module_t mp_module_device;
#define MODUDEVICE_PORT_BUILTIN_MODULES            { MP_ROM_QSTR(MP_QSTR_device), MP_ROM_PTR(&mp_module_device ) },
#else
#define MODUDEVICE_PORT_BUILTIN_MODULES
#endif


#if MICROPY_PY_UJSON
extern const struct _mp_obj_module_t mp_module_ujson;
//#define MODUJSON_PORT_BUILTIN_MODULE_WEAK_LINKS            { MP_ROM_QSTR(MP_QSTR_json), MP_ROM_PTR(&mp_module_ujson ) },
#endif

#if MICROPY_PY_UHASHLIB
extern const struct _mp_obj_module_t mp_module_uhashlib;
//#define MODUHASHLIB_PORT_BUILTIN_MODULE_WEAK_LINKS         { MP_ROM_QSTR(MP_QSTR_hashlib), MP_ROM_PTR(&mp_module_uhashlib ) },
#endif

#if MICROPY_DUMP_ADDR
extern const struct _mp_obj_module_t mp_module_dumpaddr;
#define MODDUMPADDR_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_dumpaddr), MP_ROM_PTR(&mp_module_dumpaddr) },
#else
#define MODDUMPADDR_PORT_BUILTIN_MODULES
#endif

#if MICROPY_PY_LPOWER
extern const struct _mp_obj_module_t mp_module_lpower;
#define MODULPOWER_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_Lpower), MP_ROM_PTR(&mp_module_lpower) },
#else
#define MODULPOWER_PORT_BUILTIN_MODULES
#endif

#if MICROPY_PY_USOCKET
extern const struct _mp_obj_module_t mp_module_usocket;
#define MODUSOCKET_PORT_BUILTIN_MODULES { MP_ROM_QSTR(MP_QSTR_usocket), MP_ROM_PTR(&mp_module_usocket)},
#else 
#define MODUSOCKET_PORT_BUILTIN_MODULES 
#endif


#if MICROPY_PY_UTIME
extern const struct _mp_obj_module_t mp_module_time;
#define MODUTIME_PORT_BUILTIN_MODULES		{ MP_ROM_QSTR(MP_QSTR_utime), MP_ROM_PTR(&mp_module_time)},
#else
#define MODUTIME_PORT_BUILTIN_MODULES
#endif

#if MICROPY_PY_ONENET
extern const struct _mp_obj_module_t mp_module_onenet;
#define MODUTIME_ONENET_BUILTIN_MODULES		{ MP_ROM_QSTR(MP_QSTR_OneNET), MP_ROM_PTR(&mp_module_onenet)},
#else
#define MODUTIME_ONENET_BUILTIN_MODULES
#endif

#if MICROPY_PY_FOTA
extern const struct _mp_obj_module_t mp_module_fota;
#define MODFOTA_FOTA_BUILTIN_MODULES	{ MP_ROM_QSTR(MP_QSTR_fota), MP_ROM_PTR(&mp_module_fota)},
#else
#define MODFOTA_FOTA_BUILTIN_MODULES
#endif

// #if MICROPY_PY_SYSMISC
// extern const struct _mp_obj_module_t mp_module_sysmisc;
// #define MODFOTA_SYSMISC_BUILTIN_MODULES	{ MP_ROM_QSTR(MP_QSTR_sysmisc), MP_ROM_PTR(&mp_module_sysmisc)},
// #else
// #define MODFOTA_SYSMISC_BUILTIN_MODULES
// #endif

#if MICROPY_PY_VIRTUAL_AT
extern const struct _mp_obj_module_t mp_module_virat;
#define MODFOTA_VIRTUAL_AT_BUILTIN_MODULES	{ MP_ROM_QSTR(MP_QSTR_virat), MP_ROM_PTR(&mp_module_virat)},
#else
#define MODFOTA_VIRTUAL_AT_BUILTIN_MODULES
#endif

#if MICROPY_PY_POSITION
extern const struct _mp_obj_module_t mp_module_position;
#define MODPOSITION_BUILTIN_MODULES	{ MP_ROM_QSTR(MP_QSTR_position), MP_ROM_PTR(&mp_module_position)},
#else
#define MODPOSITION_BUILTIN_MODULES
#endif

#if MICROPY_PY_MACHINE
extern const struct _mp_obj_module_t pyb_module;
#define MODMACHINE_MACHINE_BUILTIN_MODULES 	{ MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&pyb_module) }, 
#else
#define MODMACHINE_MACHINE_BUILTIN_MODULES
#endif

#if MICROPY_PY_NETWORK
extern const struct _mp_obj_module_t mp_module_network;
#define NETWORK_BUILTIN_MODULE              { MP_ROM_QSTR(MP_QSTR_net), MP_ROM_PTR(&mp_module_network) },
#else
#define NETWORK_BUILTIN_MODULE
#endif

#if MICROPY_PY_MO_NETWORK
extern const struct _mp_obj_module_t mp_module_mo_network;
#define MO_NETWORK_BUILTIN_MODULE              { MP_ROM_QSTR(MP_QSTR_mo_net), MP_ROM_PTR(&mp_module_mo_network) },
#else
#define MO_NETWORK_BUILTIN_MODULE
#endif


#define MICROPY_PORT_BUILTIN_MODULES 		\
		MODMACHINE_MACHINE_BUILTIN_MODULES	\
		MODULPOWER_PORT_BUILTIN_MODULES 	\
		MODUTIME_PORT_BUILTIN_MODULES		\
		MODUOS_PORT_BUILTIN_MODULES 		\
		MODDUMPADDR_PORT_BUILTIN_MODULES 	\
		MODUDEVICE_PORT_BUILTIN_MODULES 	\
		MODUSOCKET_PORT_BUILTIN_MODULES		\
		MODUTIME_ONENET_BUILTIN_MODULES 	\
		MODFOTA_FOTA_BUILTIN_MODULES        \
        NETWORK_BUILTIN_MODULE              \
        MODFOTA_VIRTUAL_AT_BUILTIN_MODULES  \
        MODPOSITION_BUILTIN_MODULES         \
        MO_NETWORK_BUILTIN_MODULE
		
		
		
#define MP_STATE_PORT                  MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS     const char *readline_hist[8];

#if MICROPY_PY_THREAD
#define MICROPY_EVENT_POLL_HOOK 			\
    do { 									\
        extern void mp_handle_pending(void);\
        mp_handle_pending(); 				\
        MP_THREAD_GIL_EXIT(); 				\
        MP_THREAD_GIL_ENTER(); 				\
    } while (0);
#else
#define MICROPY_EVENT_POLL_HOOK 			\
    do { 									\
        extern void mp_handle_pending(void);\
        mp_handle_pending(); 				\
    } while (0);
#endif

		
#include <alloca.h>
#endif
