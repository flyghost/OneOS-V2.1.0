#ifndef _USR_TIMER_H
#define _USR_TIMER_H

#include "mpconfigport.h"
#include "modmachine.h"
#include "model_device.h"

#define MP_TIMER_FLAG_ONE_SHOT      0x0     /* One shot timer. */
#define MP_TIMER_FLAG_PERIODIC      0x1     /* Periodic timer. */

//os_timer_t *
typedef void * TimerHandle_t;

void mp_hal_delay_ms(mp_uint_t ms);


typedef struct _machine_timer_obj_t {
    mp_obj_base_t 	base;
	device_info_t	*timer;
    mp_obj_t 		timeout_cb;
    uint32_t 		timeout;
    signed int 		is_repeat;
    signed int 		is_init;
	char dev_name[OS_NAME_MAX];
} machine_timer_obj_t;

#endif /* _USR_TIMER_H */

