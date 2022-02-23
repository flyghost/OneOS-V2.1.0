#ifndef __USER_HUMITURE_H__
#define __USER_HUMITURE_H__

#include "modmachine.h"

#ifndef IOCTL_ALS_PS_READ
#define IOCTL_ALS_PS_READ             (15)
#endif

#define ALS_PS_INIT_FLAG 		MP_MACHINE_INIT_FLAG
#define ALS_PS_DEINIT_FLAG 		MP_MACHINE_DEINIT_FLAG

struct als_ps
{
    float light;
    int proximitys;
};

#endif
