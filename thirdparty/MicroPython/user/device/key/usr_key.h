#include "py/obj.h"
#include "model_device.h"
#include "device.h"




typedef struct usr_pin_cfg{
	uint32_t 	pin;
	uint32_t 	mode;
	uint32_t 	irq_mode;
}usr_pin_obj_t;

typedef struct usr_key_cfg{
	mp_obj_base_t 	base;
	mp_obj_t 		pin_isr_cb;
	device_info_t 	*device;
	usr_pin_obj_t 	pin;
}usr_key_obj_t;

typedef struct _device_key_obj_t {
    usr_key_obj_t	key;
	uint8_t			id;
	uint8_t			open_flag;
    mp_obj_t 		isr_handler;
}device_key_obj_t;




