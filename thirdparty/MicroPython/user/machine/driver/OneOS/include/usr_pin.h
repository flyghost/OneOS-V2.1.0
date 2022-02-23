#ifndef _USR_PIN_H
#define _USR_PIN_H
#include "py/obj.h"
#include "model_device.h"
#include "modmachine.h"

typedef signed long                     mp_base_t;

#define NAME_MAX    					8

//用户实现Pin操作时需要对应以下的Mode
#define PIN_LOW                 0x00
#define PIN_HIGH                0x01

#define PIN_MODE_OUTPUT         0x00
#define PIN_MODE_INPUT          0x01
#define PIN_MODE_INPUT_PULLUP   0x02
#define PIN_MODE_INPUT_PULLDOWN 0x03
#define PIN_MODE_OUTPUT_OD      0x04

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1




#define MP_PIN_OP_GET_PIN_NUM           1
#define MP_PIN_OP_SET_MODE              2
#define MP_PIN_OP_IRQ_EN                3
#define MP_PIN_OP_IRQ_DIS               4

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
	uint8_t	 open_flag;
    uint32_t pin;
	uint32_t mode;
	uint32_t irq;
	mp_obj_t pin_isr_cb;
	device_info_t *device;
	char name[NAME_MAX];
} machine_pin_obj_t;





extern const mp_obj_type_t machine_pin_type;

typedef struct mpy_pin_index
{
    int index;
    //void (*rcc)(void);
    //GPIO_TypeDef *gpio;
    uint32_t pin;
}Pin_index;


/**
*********************************************************************************************************
*                                      获取gpio的序列号
*
* @description: 这个函数用来获取gpio的序列号。
*
* @param      : device:         设备。
*
*				mesg:			gpio的信息，如['A', 13]
* @returns    : gpio 的序列号，（操作系统层的序列号）
*********************************************************************************************************
*/
int mp_pin_get_num(void *device, void *mesg);


//void  mpycall_pin_mode(mp_base_t pin, mp_base_t mode);
//i32_t mpycall_pin_irq_enable(mp_base_t pin, u32_t enable);
i32_t mpycall_pin_attach_irq(i32_t pin, u32_t mode, void (*hdr)(void *args), void *args);
#endif
