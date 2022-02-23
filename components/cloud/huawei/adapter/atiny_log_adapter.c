#include "log/atiny_log.h"

#ifdef ATINY_DEBUG
atiny_log_e atiny_get_log_level(void)
{
    // LOG_DEBUG = 0,
    // LOG_INFO,
    // LOG_WARNING,
    // LOG_ERR,
    // LOG_FATAL,
    // LOG_MAX
    return LOG_FATAL;
}

static const char *log_name[] = {"debug", "info", "warn", "error", "fatal"};

const char *atiny_get_log_level_name(atiny_log_e log_level)
{
    switch (log_level)
    {
    case LOG_DEBUG:
        return log_name[0];

    case LOG_INFO:
        return log_name[1];

    case LOG_WARNING:
        return log_name[2];

    case LOG_ERR:
        return log_name[3];

    case LOG_FATAL:
        return log_name[4];

    default:
        return NULL;
    }
}
#endif
