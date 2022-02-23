#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_misc.h"

#include <device.h>
#include <drv_cfg.h>
#include <os_clock.h>
#include <stdio.h>
#include <sensors/sensor.h>
#include "usr_als_ps.h"

#define LIGHT_SENSOR_NAME  		MICROPYTHON_DEVICE_LIGHTSENSOR_NAME
#define PROXIMITY_SENSOR_NAME	MICROPYTHON_DEVICE_PROXIMITY_NAME

typedef enum sta_result_light
{
    light_ok                      = 0,  ///< �޴��������ɹ�
    light_error                   = 1,  ///< ���ض�����
    light_timeout                 = 3,  ///< ��ʱ����
    light_framing                 = 9,  ///< ����֡����
	light_nnknow_error			  = 0x7F,
    light_no_response             = 0xFF
}sta_lightsensor_t;



os_device_t * g_li_sensor = NULL;
os_device_t * g_pr_sensor = NULL;


static sta_lightsensor_t light_read (float *light)
{

	float light_t = 0;
	int count = 0;
	struct os_sensor_data sensor_data = {0};

	if(g_li_sensor == NULL)
	{
		return light_error;
	}

	struct os_sensor_info sensor_info;
    os_device_control(g_li_sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	for (int i = 0; i < 10; i++)
    {
        os_device_read_nonblock(g_li_sensor, 0, &sensor_data, sizeof(struct os_sensor_data));

		if (sensor_info.unit == OS_SENSOR_UNIT_MLUX) 
		{
			//os_kprintf("lightsensor_read light MLUX [%d]: %d\n",i, sensor_data.data.light);
			count++;
			light_t += sensor_data.data.light/1000;
        }
		
    }
    *light = light_t/count;

    return light_ok;
}

static sta_lightsensor_t proximity_read (int *proximity)
{

	int proximity_t = 0;
	int count = 0;
	struct os_sensor_data sensor_data = {0};

	if(g_pr_sensor == NULL)
	{
		return light_error;
	}

	struct os_sensor_info sensor_info;
    os_device_control(g_pr_sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);
	for (int i = 0; i < 10; i++)
    {
        os_device_read_nonblock(g_pr_sensor, 0, &sensor_data, sizeof(struct os_sensor_data));

		if (sensor_info.unit == OS_SENSOR_UNIT_RAW) 
		{
			//os_kprintf("lightsensor_read light MLUX [%d]: %d\n",i, sensor_data.data.raw);
			count++;
			proximity_t += sensor_data.data.raw;
        }
    }
	
    *proximity = (proximity_t/count);

    return light_ok;
}

/**
 * @brief Function for handling data from light sensor.
 *
 * @param[in] p_data          light in Celsius degrees read from sensor.
 */
static sta_lightsensor_t lightsensor_read (float *light, int *proximity)
{
	sta_lightsensor_t ret;
    ret =light_read(light);
	if(ret != light_ok)
		return ret;
	ret = proximity_read(proximity);
	if(ret != light_ok)
		return ret;
    return light_ok;
}

static int als_ps_open(const char *dev_name)
{
    int ret = OS_ERROR;
    
    g_li_sensor = os_device_find(LIGHT_SENSOR_NAME);
    g_pr_sensor = os_device_find(PROXIMITY_SENSOR_NAME);
    if(!g_pr_sensor || !g_li_sensor)
    {
        return light_error;
    }

    ret = os_device_open(g_li_sensor);
    if (ret != OS_EOK)
    {
        mp_err("Open light sensor(%s) failed. \n", LIGHT_SENSOR_NAME);
        return light_error;
    }

    ret = os_device_open(g_pr_sensor);
    if (ret != OS_EOK)
    {
        mp_err("Open proximity sensor(%s) failed. \n", PROXIMITY_SENSOR_NAME);
        return light_error;
    }

    return light_ok;
}

static int als_ps_deinit(const char *dev_name)
{
	if(! g_li_sensor || ! g_pr_sensor)
	{
		mp_err("Could not found light sensor or proximity.");
		return light_error;
	}

	os_device_close(g_li_sensor);
	os_device_close(g_pr_sensor);

    return light_ok;
}


static int als_ps_ioctl(void *dev, int cmd, void *arg)
{
	if(! g_li_sensor || ! g_pr_sensor)
	{
		mp_err("Could not found light sensor or proximity \n");
		return light_error;
	}
    switch(cmd)
    {
        case  IOCTL_ALS_PS_READ:
            return lightsensor_read(&(((struct als_ps *)arg)->light), &(((struct als_ps *)arg)->proximitys));
        default:
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return light_error;
    }
}

struct operate als_ps_ops = {
    .open =  als_ps_open,
    .ioctl = als_ps_ioctl,
    .close = als_ps_deinit,
};

static int als_ps_register(void)
{
	device_info_t * lightsensor = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == lightsensor)
	{
		mp_err("malloc mem failed! \n");
		return MP_ERROR;
	}
    memset(lightsensor, 0, sizeof(device_info_t));
	

	lightsensor->owner.name = "als_ps";
	lightsensor->owner.type = DEV_LIGHT;
	
	lightsensor->ops = &als_ps_ops;
	

	mpycall_device_add(lightsensor);

	return MP_EOK;
}

OS_CMPOENT_INIT(als_ps_register, OS_INIT_SUBLEVEL_LOW);




