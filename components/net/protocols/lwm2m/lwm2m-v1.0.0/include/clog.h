/**
************************************* Copyright ******************************
******************************************************************************
*/


#ifndef __clog_LOG_H__
#define __clog_LOG_H__

#include <stdint.h>
#include <stdarg.h>
#include <os_util.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif


//log config
#define COMP_CFG_LOG_COLOR 1 //是否需要打印颜色
//#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_VERBOSE //日志输出水平

#if defined(LWM2M_LOG_ERROR)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_ERROR
#elif defined(LWM2M_LOG_WARN)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_WARN
#elif defined(LWM2M_LOG_INFO)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_INFO
#elif defined(LWM2M_LOG_DEBUG)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_DEBUG
#elif defined(LWM2M_LOG_VERBOSE)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_VERBOSE
#elif defined(LWM2M_LOG_NONE)
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_NONE
#endif







//log output interface
#define COMP_LOG_OUT(format,...) printf(format,##__VA_ARGS__)



//log level
typedef enum {
	COMP_LOG_NONE, /*no log print */
	COMP_LOG_ERROR, /*error print */
	COMP_LOG_WARN, /*warn print */
	COMP_LOG_INFO, /*formal print infomation*/
	COMP_LOG_DEBUG, /*normal use just for debugging */
	COMP_LOG_VERBOSE /*show all level print info */
} comp_log_level_t;

//如果没有定义日志等级就使用默认等级
#ifndef COMP_CFG_LOG_LOCAL_LEVEL
#define COMP_CFG_LOG_LOCAL_LEVEL COMP_LOG_INFO
#endif



// color define
#if COMP_CFG_LOG_COLOR
#define LOG_COLOR_BLACK "30"
#define LOG_COLOR_RED "31"
#define LOG_COLOR_GREEN "32"
#define LOG_COLOR_BROWN "33"
#define LOG_COLOR_BLUE "34"
#define LOG_COLOR_PURPLE "35"
#define LOG_COLOR_CYAN "36"
#define LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LOG_RESET_COLOR "\033[0m"
#define LOG_COLOR_E LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D LOG_COLOR(LOG_COLOR_BLUE)
#define LOG_COLOR_V LOG_COLOR(LOG_COLOR_BLACK)
#else //COMP_CFG_LOG_COLOR
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //COMP_CFG_LOG_COLOR

#define LOG_FORMAT(letter, format) LOG_COLOR_ ##letter #letter "[%s]:[%s][%d] " format LOG_RESET_COLOR "\r\n"
#define COMP_EARLY_LOGE( tag, format, ... ) COMP_LOG_EARLY_IMPL(tag, format, COMP_LOG_ERROR, E, ##__VA_ARGS__)
#define COMP_EARLY_LOGW( tag, format, ... ) COMP_LOG_EARLY_IMPL(tag, format, COMP_LOG_WARN, W, ##__VA_ARGS__)
#define COMP_EARLY_LOGI( tag, format, ... ) COMP_LOG_EARLY_IMPL(tag, format, COMP_LOG_INFO, I, ##__VA_ARGS__)
#define COMP_EARLY_LOGD( tag, format, ... ) COMP_LOG_EARLY_IMPL(tag, format, COMP_LOG_DEBUG, D, ##__VA_ARGS__)
#define COMP_EARLY_LOGV( tag, format, ... ) COMP_LOG_EARLY_IMPL(tag, format, COMP_LOG_VERBOSE, V, ##__VA_ARGS__)

#define COMP_LOG_EARLY_IMPL(tag, format, log_level, log_tag_letter, ...) do { \
if (COMP_CFG_LOG_LOCAL_LEVEL >= log_level) { \
COMP_LOG_OUT(LOG_FORMAT(log_tag_letter, format), tag,__FUNCTION__,__LINE__,##__VA_ARGS__); \
}} while(0)



//错误日志输出
#define COMP_LOGE( tag, format, ... ) COMP_EARLY_LOGE(tag, format, ##__VA_ARGS__)

//警告日志
#define COMP_LOGW( tag, format, ... ) COMP_EARLY_LOGW(tag, format, ##__VA_ARGS__)

//信息日志
#define COMP_LOGI( tag, format, ... ) COMP_EARLY_LOGI(tag, format, ##__VA_ARGS__)

//调试日志
#define COMP_LOGD( tag, format, ... ) COMP_EARLY_LOGD(tag, format, ##__VA_ARGS__)

//详细日志
#define COMP_LOGV( tag, format, ... ) COMP_EARLY_LOGV(tag, format, ##__VA_ARGS__)



#ifdef __cplusplus
}
#endif


#endif /* __COMP_LOG_H__ */


