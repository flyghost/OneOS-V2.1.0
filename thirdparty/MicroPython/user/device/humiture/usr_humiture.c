#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_misc.h"

#include <device.h>
#include <sensors/sensor.h>
#include "user_humiture.h"


os_device_t *G_HUMI = NULL;
os_device_t *G_TEMP = NULL;




static int temperature_read(float *temperature)
{
	if (G_TEMP == NULL) return -1;
	struct os_sensor_data sensor_data = {0};
	struct os_sensor_info sensor_info;
	
    os_device_control(G_TEMP, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	os_device_read_nonblock(G_TEMP, 0, &sensor_data, sizeof(struct os_sensor_data));
   
    os_device_read_nonblock(G_TEMP, 0, &sensor_data, sizeof(struct os_sensor_data));

	if (sensor_info.unit == OS_SENSOR_UNIT_MDCELSIUS) {
		*temperature = sensor_data.data.temp/ 1000;
    } else if (sensor_info.unit == OS_SENSOR_UNIT_DCELSIUS) {
		*temperature = sensor_data.data.temp;
    } else {
        mp_err("invalid unit.");
    }
    
	return 0;
}

static int humidity_read(float *humidity)
{
	if (G_HUMI == NULL) return -1;
	struct os_sensor_data sensor_data = {0};

	struct os_sensor_info sensor_info;
    os_device_control(G_HUMI, OS_SENSOR_CTRL_GET_INFO, &sensor_info);

    os_device_read_nonblock(G_HUMI, 0, &sensor_data, sizeof(struct os_sensor_data));

	if (sensor_info.unit == OS_SENSOR_UNIT_MPERMILLAGE)  {
		*humidity = sensor_data.data.humi /1000;
    } else if (sensor_info.unit == OS_SENSOR_UNIT_PERMILLAGE) {
		*humidity = sensor_data.data.humi;
    } else {
        mp_err("invalid unit \n");
    }
   
	return 0;
}


static int humiture_init(const char *dev_name)
{
    int ret = 0;
    
    G_HUMI = os_device_find(MICROPYTHON_DEVICE_HUMIDITY_NAME);
	G_TEMP = os_device_find(MICROPYTHON_DEVICE_TEMPERATURE_NAME);
	if (G_HUMI == OS_NULL || G_TEMP == OS_NULL)
	{
		mp_raise_ValueError("Couldn't find humiture device(G_HUMI | G_TEMP)! \n");
		return MP_ERROR;
	}
	
    ret = os_device_open(G_HUMI);
	if (ret != 0)
	{
		mp_err("open humidity sensor(%s) failed \n", MICROPYTHON_DEVICE_HUMIDITY_NAME);
		return MP_ERROR;
	}

    ret = os_device_open(G_TEMP);
	if (ret != 0)
	{
		mp_err("open temperature sensor(%s) failed \n", MICROPYTHON_DEVICE_TEMPERATURE_NAME);
		return MP_ERROR;
	}
	
    return MP_EOK;
}


static int humiture_deinit(const char *dev)
{
	if(G_HUMI == OS_NULL || G_TEMP == OS_NULL){
		mp_raise_ValueError("Couldn't find humiture device(G_HUMI | G_TEMP)! \n");
		return -1;
	}
	os_device_close(G_HUMI);
    os_device_close(G_TEMP);

    return 0;
}


static int humiture_ioctl(void *dev, int cmd, void *arg)
{
	if(G_HUMI == OS_NULL || G_TEMP == OS_NULL){
		mp_raise_ValueError("Couldn't find humiture device(G_HUMI | G_TEMP)! \n");
		return -1;
	}
	struct humituredata *humiture = arg;
    switch(cmd)
    {
        case  IOCTL_HUMITURE_READ:
		{
			temperature_read(&humiture->temperature);
			humidity_read(&humiture->humidity);
            return MP_EOK; 
		}
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return MP_ERROR;
    }
}

struct operate humiture_ops = {
    .open =  humiture_init,
    .ioctl = humiture_ioctl,
    .close = humiture_deinit,
};

static int humiture_register(void)
{
	device_info_t * humiture = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == humiture)
	{
		os_kprintf("malloc mem failed!");
		return -1;
	}
    memset(humiture, 0, sizeof(device_info_t));
	

	humiture->owner.name = "humiture";
	humiture->owner.type = DEV_HUMITURE;
	
	humiture->ops = &humiture_ops;
	
    
	mpycall_device_add(humiture);

	return 0;
}

OS_CMPOENT_INIT(humiture_register, OS_INIT_SUBLEVEL_LOW);




