#ifndef _USR_RTC_H_
#define _USR_RTC_H_

#include "model_device.h"
#include "py/runtime.h"


enum{

	SET_DATE = 0,
	SET_TIME,
	SET_ALARM,
	GET_TIME
	
};
extern const mp_obj_type_t machine_hard_rtc_type;

#endif

