#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "usr_misc.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include <device.h>
#include "user_six_axis.h"



static os_device_t *g_gyro_sensor = NULL;
static os_device_t *g_acce_sensor = NULL;

static int gyro_sensor_read(struct os_sensor_data *sensor_data)
{
	os_device_read_nonblock(g_gyro_sensor, 0, sensor_data, sizeof(struct os_sensor_data));
	 
	return 0;

}

static int acce_sensor_read(struct os_sensor_data *sensor_data)
{
	 os_device_read_nonblock(g_acce_sensor, 0, sensor_data, sizeof(struct os_sensor_data));
	 return 0;
}

static int six_axis_init(const char *dev_name)
{
    int ret = MP_ERROR;
    
    g_gyro_sensor = os_device_find(MICROPYTHON_DEVICE_GGYROSCOPE_NAME);
    g_acce_sensor = os_device_find(MICROPYTHON_DEVICE_ACCELEROMETER_NAME);
    
    if (g_gyro_sensor == OS_NULL || g_acce_sensor == OS_NULL)
    {
        mp_raise_ValueError("Couldn't find six_axis device(gyro_sensor | acce_sensor)! \n");
        return MP_ERROR;
    }

    ret = os_device_open(g_acce_sensor);
    if (ret != 0)
    {
        mp_err("Open accelerometer sensor(%s) failed.", 
                MICROPYTHON_DEVICE_ACCELEROMETER_NAME);
        return MP_ERROR;
    }

    ret = os_device_open(g_gyro_sensor);
    if (ret != 0)
    {
        mp_err("Open gyroscope sensor(%s) failed.", 
                MICROPYTHON_DEVICE_GGYROSCOPE_NAME);
        return MP_ERROR;
    }

    return 0;
}

static int six_axis_deinit(const char *dev_name)
{
    if((!g_gyro_sensor) || (!g_acce_sensor))
    {
        mp_err("Could not found accelerometer or gyroscope.");
    }
    os_device_close(g_gyro_sensor);
    os_device_close(g_acce_sensor);

    return 0;
}

static int six_axis_ioctl(void *dev, int cmd, void *arg)
{
    struct os_sensor_data *sensor_data = (struct os_sensor_data *)arg;

    if (! g_gyro_sensor || ! g_acce_sensor)
    {
        mp_err("Could not found accelerometer or gyroscope.");
    }
    switch(cmd)
    {
    case  MPY_GYRO_ACCE_INFO_READ:
        gyro_sensor_read(&sensor_data[0]);
        acce_sensor_read(&sensor_data[1]);
        return 0;
    default:
        mp_raise_ValueError("the cmd is wrong, please check!");
        return -1;
    }
}

struct operate six_axis_ops = {
    .open = six_axis_init,
    .ioctl = six_axis_ioctl,
    .close = six_axis_deinit,
};


static int six_axis_register(void)
{
	device_info_t * six_axis = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == six_axis)
	{
		os_kprintf("malloc mem failed!");
		return -1;
	}
    memset(six_axis, 0, sizeof(device_info_t));
	

	six_axis->owner.name = "six_axis";
	six_axis->owner.type = DEV_SIX_AXIS;
	
	six_axis->ops = &six_axis_ops;
	
	mpycall_device_add(six_axis);

	return 0;
}

OS_CMPOENT_INIT(six_axis_register, OS_INIT_SUBLEVEL_LOW);




