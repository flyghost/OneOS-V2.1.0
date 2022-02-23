#ifndef __MODEL_LOG_H__
#define __MODEL_LOG_H__

#include "model_def.h"

struct model_misc_fun{
	fun_p_2_t   calloc;
	fun_0_1_t 	free;
	fun_i_1_t 	mdelay;
	fun_0_2x_t	print;
	fun_i_4x_t	format;
	u32_t		model;
};

/**
 *********************************************************************************************************
 *                                      define log macro
 *********************************************************************************************************
*/
#define MODEL_LOG_LEVEL		model_get_misc_structure()->model
#define MODEL_LOG_BASE		0
#define MODEL_LOG_DEBUG		1
#define MODEL_LOG_ERROR		2

#define LOG_HANDLE			model_get_misc_structure()->print

#define model_kprintf(msg ,...)    LOG_HANDLE(msg, ##__VA_ARGS__)	 

#define model_printf(num, msg, ...) 			\
do {											\
	if ((num > MODEL_LOG_LEVEL) && LOG_HANDLE){	\
		model_kprintf(msg"\r\n", ##__VA_ARGS__);  	\
	}											\
}while(0);

#define MODEL_LOG(TAG, MSG,...)	            	\
    model_printf(MODEL_LOG_DEBUG, TAG "[LOG] [%s] "MSG, __FUNCTION__, ##__VA_ARGS__)
    
#define MODEL_ERR(TAG, MSG,...)	            	\
    model_printf(MODEL_LOG_ERROR, TAG "[ERROR] [%s | %d ] "MSG, __FUNCTION__, __LINE__, ##__VA_ARGS__)



struct model_misc_fun *model_get_misc_structure(void);
#endif
