#include "ctiot_log.h"
#include <oneos_config.h>

#ifndef CTIOT_LOG_LEVEL_GLOBAL
#define CTIOT_LOG_LEVEL_GLOBAL LOG_DEBUG
#endif
static ctiot_log_e g_ctiot_log_level = (ctiot_log_e)CTIOT_LOG_LEVEL_GLOBAL;

static const char *g_log_names[] =
{
    "DEBUG",
    "INFO",
    "WARNING",
    "ERR",
    "FATAL",
};

void ctiot_set_log_level(ctiot_log_e level)
{
    g_ctiot_log_level = level;
}

ctiot_log_e ctiot_get_log_level(void)
{
    return g_ctiot_log_level;
}

const char *ctiot_get_log_level_name(ctiot_log_e log_level)
{
    if (log_level >= LOG_MAX)
    {
        return "UNKOWN";
    }

    return g_log_names[log_level];
}
