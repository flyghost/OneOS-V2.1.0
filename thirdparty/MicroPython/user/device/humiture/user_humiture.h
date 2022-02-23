#ifndef __USER_HUMITURE_H__
#define __USER_HUMITURE_H__

#define IOCTL_HUMITURE_READ             (1)

#define HUMITURE_INIT_FLAG 				(1)
#define HUMITURE_DEINIT_FLAG 			(0)

struct humituredata
{
    float temperature;
    float humidity;
};

#endif
