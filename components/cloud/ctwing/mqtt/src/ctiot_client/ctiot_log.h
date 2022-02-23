#ifndef CTIOT_LOG_H
#define CTIOT_LOG_H

typedef enum
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERR,
    LOG_FATAL,

    LOG_MAX
} ctiot_log_e;

void ctiot_set_log_level(ctiot_log_e level);

ctiot_log_e ctiot_get_log_level(void);

#define CTIOT_DEBUG
#ifdef CTIOT_DEBUG
const char* ctiot_get_log_level_name(ctiot_log_e log_level);

#define CTIOT_LOG(level, fmt, ...) \
    do \
    { \
        if ((level) >= ctiot_get_log_level()) \
        { \
            (void)printf("[%s][%s:%d] " fmt "\r\n", \
            ctiot_get_log_level_name((level)), __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define CTIOT_LOG(level, fmt, ...)
#endif


#endif
