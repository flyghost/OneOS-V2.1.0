/*
 * This file is paos of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial poosions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PAosICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TOos OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "py/runtime.h"



#if (MICROPY_PY_MACHINE_CAN)
#include "can.h"
#include "bus_can.h"
#include "usr_misc.h"
#include <string.h>
#include <device.h>
#include "middlelib.h"
#include "usr_general.h"

static uint32_t Byte2HexStr(uint8_t *buf, uint32_t bufsize, char *dest)
{	
	int i= 0;
	for (; i < bufsize; i++){
		byte2char(buf[i], dest+i*2, 1);
	}
	return i*2;
}


//static os_err_t can_rx_call(os_device_t *dev, os_size_t size)
//{
//    rx_count++;
//    return MP_CAN_OP_OK;
//}


static int can_read(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    can_msg_t *res_msg = (can_msg_t *)buf;
    struct os_can_msg msg = {0}; 
    uint8_t id_buf[4] = {0};

    if (mpy_usr_driver_read(dev_name, offset, &msg, sizeof(msg), MP_USR_DRIVER_READ_NONBLOCK) > 0)
    {
        id_buf[3] = msg.id & 0xff;
        id_buf[2] = (msg.id >> 8) & 0xff;
        id_buf[1] = (msg.id >> 16) & 0xff;
        id_buf[0] = (msg.id >> 24) & 0xff;

        Byte2HexStr(id_buf, 4, res_msg->id);
        res_msg->ide = msg.ide;
        res_msg->rtr = msg.rtr;
        res_msg->len = Byte2HexStr(msg.data, msg.len, res_msg->data);
        return res_msg->len;
    }

    return MP_CAN_OP_OK;
}


static struct os_can_msg *check_data(uint32_t *buf, uint32_t bufsize, struct os_can_msg *msg)
{
	int len ;
	uint8_t dest[30];
	if (bufsize <7 || bufsize > 30|| strlen((char *)buf) != bufsize ||  bufsize%2 != 0){
		mp_err("the parameter is invalid!\n");
		return NULL;
	}
	
	len = HexStr2Byte((char*)buf, bufsize, dest);
	if (!len){
		mp_err("[check_data] Failed to convert data! \n");
		return NULL;
	}
	msg->id  = dest[0] << 24 | dest[1] << 16 | dest[2] << 8 | dest[3];
	if (dest[4] > 1 || dest[5] > 1){
		mp_err("the parameter is invalid! id is 4 bytes of data, ide is wrong or rtr is wrong!\n"); 
		return NULL;
	} else {
		msg->ide = dest[4];//OS_CAN_STDID; // id 扩展帧
		msg->rtr = dest[5];//OS_CAN_DTR;   // 表示数据帧的有无
	}
	msg->len = dest[6];
	if (len > 7){
		memcpy(msg->data, dest+7, msg->len);
	}		
	return msg;
}

//static os_err_t can_tx_done(os_device_t *uart, void *buffer)
//{
//    tx_count++;
//    return MP_CAN_OP_OK;
//}

static int can_write(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    struct os_can_msg msg = {0};

    if (!check_data(buf, bufsize, &msg))
    {
        mp_err("can_write failed! \n");
        return MP_CAN_OP_ERROR;
    }

    return mpy_usr_driver_write(dev_name, offset, &msg, sizeof(msg), MP_USR_DRIVER_WRITE_NONBLOCK);
}



static int can_ioctl(void *device, int cmd, void* arg)
{
    os_device_t *can_device  =  os_device_find(((device_info_t *)device)->owner.name);
    if (can_device == NULL)
    {
        mp_err("can_ioctl find device error\n");
        return MP_CAN_OP_ERROR;
    }

    switch(cmd){
    case MP_MACHINE_OP_SET_PARAM:
        os_device_control(can_device, OS_CAN_CMD_SET_BAUD, (void *)((mp_can_obj_t *)arg)->baud);
        os_device_control(can_device, OS_CAN_CMD_SET_MODE, (void *)(((mp_can_obj_t *)arg)->mode));
        break;
    }

    return 0;
}

STATIC struct operate can_ops = {
    .open = mpy_usr_driver_open,
    .read = can_read,
    .write = can_write,
    .ioctl = can_ioctl,
    .close = mpy_usr_driver_close,
};

static int can_register(void)
{
    MP_SIMILAR_DEVICE_REGISTER(MICROPYTHON_MACHINE_CAN_PRENAME, DEV_BUS, &can_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(can_register, OS_INIT_SUBLEVEL_LOW);


#endif // MICROPY_PY_MACHINE_CAN

