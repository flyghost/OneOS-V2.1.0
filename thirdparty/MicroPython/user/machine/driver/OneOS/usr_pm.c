
#include "py/mperrno.h"

#if (MICROPY_PY_MACHINE_PM)
#include "usr_pm.h"
#include <stdio.h>
#include "usr_misc.h"
#include <string.h>
#include "device.h"

int pm_ctrl(void *device, int cmd, void* arg)
{
	os_device_t *pm_device = os_device_find(((device_info_t *)device)->owner.name);
	//int* data = arg;
	switch(cmd){
	
		case MP_PM_SLEEP_MODE_NONE:
			break;
		
		case MP_PM_SLEEP_MODE_IDLE:
			break;
		
		case MP_PM_SLEEP_MODE_LIGHT:
			
			break;
		
		case MP_PM_SLEEP_MODE_DEEP:
			break;
		
		case MP_PM_SLEEP_MODE_STANDBY:
			#ifdef OS_USING_LPMGR
			os_lpmgr_request(SYS_SLEEP_MODE_STANDBY);
			#endif
			break;
		
		case MP_PM_SLEEP_MODE_SHUTDOWN:
			#ifdef OS_USING_LPMGR
			os_lpmgr_request(SYS_SLEEP_MODE_SHUTDOWN);
			#endif
			break;
		
		default:
			break;
	}

	return 0;
}

struct operate pm_ops={
	.ioctl = pm_ctrl,
};

static int pm_register(void)
{
    MP_SINGLE_DEVICE_REGISTER(pm, DEV_PM, &pm_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(pm_register, OS_INIT_SUBLEVEL_LOW);

#endif //MICROPY_PY_MACHINE_PM
