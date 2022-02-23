#ifndef _USR_LPOWER_H_
#define _USR_LPOWER_H_
#include "py/obj.h"
//#include "stm32l476xx.h"

enum{
	SLEEP_MODE = 0,
	STOP_MODE,
	STAND_MODE
};

void Enter_Lpower(int mode);

#endif
