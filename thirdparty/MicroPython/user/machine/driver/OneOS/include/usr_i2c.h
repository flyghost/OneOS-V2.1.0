#ifndef _USR_I2C_H
#define _USR_I2C_H
#include <stdint.h>
#define IIC_NAME_MAX 16
#include "model_device.h"
#include "py/obj.h"
#include "device.h"
#ifdef BSP_USING_I2C
#include "i2c.h"
#include "i2c_dev.h"
#endif

typedef device_info_t mp_soft_i2c_device_handler;

#define _I2C1	0x01
#define _I2C2	0x02
#define _I2C3	0x03
#define _I2C4	0x04

#define OS_I2C_DEV_CTRL_TRANSFER OS_I2C_DEV_CTRL_RW

struct i2c_bus_device
{
    uint16_t  flags;
    uint16_t  addr;

    uint32_t  timeout;
    uint32_t  retries;
    char name[IIC_NAME_MAX];
    void *priv;
};

typedef struct _machine_i2c_obj_t {
    mp_obj_base_t base;
    uint8_t open_flag;
    uint16_t client_addr;
    device_info_t* i2c_bus;
} machine_i2c_obj_t;

#ifdef BSP_USING_I2C
struct os_i2c_msg_count
{
    struct os_i2c_msg msg;
    size_t count;
};
#endif

#ifdef BSP_USING_SOFT_I2C1
#define MPY_DEFAULT_SOFT_I2C_SCL_PIN    BSP_SOFT_I2C1_SCL_PIN
#define MPY_DEFAULT_SOFT_I2C_SDA_PIN    BSP_SOFT_I2C1_SDA_PIN
#elif defined BSP_USING_SOFT_I2C2
#define MPY_DEFAULT_SOFT_I2C_SCL_PIN    BSP_SOFT_I2C2_SCL_PIN
#define MPY_DEFAULT_SOFT_I2C_SDA_PIN    BSP_SOFT_I2C2_SDA_PIN
#else
#define MPY_DEFAULT_SOFT_I2C_SCL_PIN    BSP_SOFT_I2C3_SCL_PIN
#define MPY_DEFAULT_SOFT_I2C_SDA_PIN    BSP_SOFT_I2C3_SDA_PIN
#endif

#endif /* _USR_I2C_H */

