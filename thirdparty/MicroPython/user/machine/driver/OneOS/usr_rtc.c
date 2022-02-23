#include "device.h"
#include "usr_rtc.h"
#include <string.h>
#include <stdlib.h>
#if (MICROPY_PY_MACHINE_RTC)
#include <rtc/rtc.h>
#include <os_memory.h>
#include "usr_misc.h"
#include "modmachine.h"

static int rtc_ctrl(void *device, int cmd, void* arg)
{
	//os_device_t *rtc_device = os_device_find(((device_info_t *)device)->owner.name);
	
	int* data = arg;
	switch(cmd){
	
		case SET_DATE:
			set_date(data[0], data[1], data[2]);
			break;
		
		case SET_TIME:
			set_time(data[0], data[1], data[2]);
			break;
		#if 0
		case SET_ALARM:
			os_device_control(rtc_device, RT_DEVICE_CTRL_RTC_SET_ALARM, arg);
			break;
		#endif
		case GET_TIME:
		{
			time_t now;
			now = rtc_get();
			char *strtime = ctime(&now);
			memcpy(arg,strtime, strlen(strtime)-1);
			break;
		}
		default:
			break;
	}
	
	
	return 0;
}

static struct operate rtc_ops = {
    .ioctl = rtc_ctrl,
};

static int rtc_register(void)
{
    MP_SINGLE_DEVICE_REGISTER(rtc, DEV_RTC, &rtc_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(rtc_register, OS_INIT_SUBLEVEL_LOW);
#endif //MICROPY_PY_MACHINE_RTC
