#include "include.h"
#include "arm_arch.h"
#include "drv_model_pub.h"
#include "calendar_pub.h"
#include "calendar.h"

static SDD_OPERATIONS cal_op = {
            cal_ctrl
};
			
void cal_cfg_init_value(uint32_t val)
{
	REG_WRITE(CAL_CONFIG, val);
}

void cal_enable(void)
{
	REG_WRITE(CAL_ENABLE, 0x1);
}

void cal_disable(void)
{
	REG_WRITE(CAL_ENABLE, 0x0);
}

uint32_t cal_get_sec_field(void)
{
	uint32_t val;

	val = REG_READ(CAL_SECOND_VALUE);
	return val; 
}

uint32_t cal_get_usec_tu_field(void)
{
	uint32_t val;

	val = REG_READ(CAL_31_25US_VALUE) & CAL_31_25US_MASK;
	return val; 
}

uint32_t cal_get_time_us(void)
{
	uint32_t val;

	val = cal_get_sec_field() * 1000 + cal_get_usec_tu_field() * CAL_3125_TU_VAL / 100;
	return val; 
}

void cal_init(void)
{
	cal_cfg_init_value(0);
	cal_enable();
	
	sddev_register_dev(CAL_DEV_NAME, &cal_op);
}

void cal_exit(void)
{
	cal_disable();
}

UINT32 cal_ctrl(UINT32 cmd, void *param)
{
	switch(cmd)
	{		
		default:
			break;
	}
	
	return 0;
}

// eof

