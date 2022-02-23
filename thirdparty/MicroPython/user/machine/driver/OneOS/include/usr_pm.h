#ifndef _USR_PM_H_
#define _USR_PM_H_

#include "model_device.h"
#include "py/runtime.h"
#ifdef OS_USING_LPMGR
#include <lpmgr.h>
#endif
enum{
	MP_PM_SLEEP_MODE_NONE = 0,
	MP_PM_SLEEP_MODE_IDLE,
	MP_PM_SLEEP_MODE_LIGHT,
	MP_PM_SLEEP_MODE_DEEP,
	MP_PM_SLEEP_MODE_STANDBY,
	MP_PM_SLEEP_MODE_SHUTDOWN
};
extern const mp_obj_type_t machine_hard_pm_type;

#endif
