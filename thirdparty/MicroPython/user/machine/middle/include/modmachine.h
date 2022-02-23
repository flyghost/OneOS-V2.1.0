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

#ifndef _MODMACHINE_H
#define _MODMACHINE_H

#include "py/obj.h"


extern const mp_obj_type_t machine_pin_type;

MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(machine_info_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_unique_id_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_reset_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_bootloader_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_freq_obj);
MP_DECLARE_CONST_FUN_OBJ_0(pyb_wfi_obj);
MP_DECLARE_CONST_FUN_OBJ_0(pyb_disable_irq_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_enable_irq_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_sleep_obj);
//MP_DECLARE_CONST_FUN_OBJ_0(machine_deepsleep_obj);


#define MP_MACHINE_DEINIT_FLAG		0
#define MP_MACHINE_INIT_FLAG		1


/*
#define MP_MACHINE_OP_ERROR 		-1
#define MP_MACHINE_OP_EOK	 		0
#define MP_MACHINE_OP_INIT 			1
#define MP_MACHINE_OP_DEINIT 		2
#define MP_MACHINE_OP_OPEN 			3
#define MP_MACHINE_OP_CLOSE			4
#define MP_MACHINE_OP_SET_PARAM		5
#define MP_MACHINE_OP_CALLBACK		6
#define MP_MACHINE_OP_CREATE		7
#define MP_MACHINE_OP_DELETE		8
*/
typedef enum MP_MACHINE_OP_TYPE {
    MP_MACHINE_OP_ERROR = -1,
    MP_MACHINE_OP_EOK,
    MP_MACHINE_OP_INIT,
    MP_MACHINE_OP_DEINIT,
    MP_MACHINE_OP_OPEN,
    MP_MACHINE_OP_CLOSE,
    MP_MACHINE_OP_SET_PARAM,
    MP_MACHINE_OP_CALLBACK,
    MP_MACHINE_OP_CREATE,
    MP_MACHINE_OP_DELETE,
    MP_MACHINE_OP_ENABLE,
    MP_MACHINE_OP_DISABLE,
} MP_MACHINE_OP_TYPE_T; 




#define MP_MACHINE_CHECK_OPEN_FIRST_LEVEL(self_in, type, MSG)           \
    type *self = (type*)(self_in);                                      \
    do {                                                                \
        if (self->open_flag != MP_MACHINE_INIT_FLAG)                    \
        {                                                               \
            mp_raise_ValueError("Invalid peripheral, please open "MSG); \
            return mp_const_none;                                       \
        }                                                               \
    } while(0)

#define MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(self_in, type, MSG)		\
	type* self = (type*)(self_in);									\
	device_info_t* usr = self->device;								\
	if (!usr){														\
		mp_raise_ValueError(MSG" is null!");						\
		return mp_const_none;										\
	}																\
	if (usr->open_flag != MP_MACHINE_INIT_FLAG){					\
		mp_raise_ValueError("Invalid peripheral, please open "MSG);	\
		return mp_const_none;										\
	}

#define MP_MACHINE_IS_ALREADY_OPENED(self)                                  \
    do {                                                                    \
        if (self->open_flag == MP_MACHINE_INIT_FLAG)                        \
        {                                                                   \
            mp_raise_ValueError(" This device is already initialized.");    \
            return mp_const_none;                                           \
        }                                                                   \
    }                                                                       \
    while(0)

#define SET_MACHINE_OPEN_FLAG(self)									\
	(self)->open_flag = MP_MACHINE_INIT_FLAG

#define CLEAN_MACHINE_OPEN_FLAG(self)								\
	(self)->open_flag = MP_MACHINE_DEINIT_FLAG
	
#define USER_DEV(usr)	((device_info_t *)(usr->device))

#define OPEN_MP_MACHINE_DEVICE(open_flag, open_func, ...)                       \
    do {                                                                        \
        int ret = -1;                                                           \
        if (open_flag == MP_MACHINE_INIT_FLAG)                                  \
        {                                                                       \
            mp_raise_ValueError(" This device is already initialized.");        \
            break;                                                              \
        }                                                                       \
                                                                                \
        ret = open_func(__VA_ARGS__);                                         \
        if (0 != ret)                                                           \
        {                                                                       \
            mp_raise_ValueError(" Open device failed.");                        \
            break;                                                              \
        }                                                                       \
        open_flag = MP_MACHINE_INIT_FLAG;                                       \
    } while (0)

#define CLOSE_MP_MACHINE_DEVICE(open_flag, close_func, ...)                     \
    do {                                                                        \
        int ret = -1;                                                           \
                                                                                \
        open_flag = MP_MACHINE_DEINIT_FLAG;                                     \
        ret = close_func(__VA_ARGS__);                                          \
        if (0 != ret)                                                           \
        {                                                                       \
            mp_raise_ValueError(" Close device failed.");                       \
            break;                                                              \
        }                                                                       \
    } while (0)

#define MP_SINGLE_DEVICE_REGISTER(dev, dev_t, dev_ops)                  \
    do {                                                                \
        device_info_t * dev =  os_malloc(sizeof(device_info_t));        \
        if (!dev)                                                       \
        {                                                               \
            mp_err("Failed to allocate memory !");                      \
            return MP_ERROR;                                            \
        }                                                               \
        memset(dev, 0, sizeof(device_info_t));                          \
        dev->owner.name = #dev;                                         \
        dev->owner.type = dev_t;                                        \
        dev->ops = dev_ops;                                             \
        mpycall_device_add(dev);                                        \
    } while(0)
    
#define MP_SIMILAR_DEVICE_REGISTER(prename, dev_t, dev_ops)             \
    do {                                                                \
        device_info_t  *pos;                                            \
        struct similar_device *dev =                                    \
            mp_misc_find_similar_device(prename);                       \
        if (!dev->tail)                                                 \
        {                                                               \
            mp_err("No "prename" similar devices found! ");             \
            return MP_ERROR;                                            \
        }                                                               \
        DEV_LIST_LOOP_REVERSE(pos, dev->tail, dev->head)                \
        {                                                               \
            pos->owner.type = dev_t;                                    \
            pos->ops = dev_ops;                                         \
        }                                                               \
    } while(0)

#endif /* _MODMACHINE_H */

