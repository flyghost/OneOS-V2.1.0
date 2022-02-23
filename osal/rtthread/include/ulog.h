/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        ulog.h
 *
 * @brief       RT-Thread adaper log function header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __RT_ULOG__H__
#define __RT_ULOG__H__

#include <rtconfig.h>

#ifdef RT_USING_ULOG

#ifdef __cplusplus
extern "C" {
#endif

/* Logger level, the number is compatible for syslog */
#define LOG_LVL_ASSERT                 0
#define LOG_LVL_ERROR                  3
#define LOG_LVL_WARNING                4
#define LOG_LVL_INFO                   6
#define LOG_LVL_DBG                    7

/* Compatible for rtdbg */
#undef LOG_D
#undef LOG_I
#undef LOG_W
#undef LOG_E
#undef LOG_RAW

#undef DBG_ERROR
#undef DBG_WARNING
#undef DBG_INFO
#undef DBG_LOG

#undef dbg_log

#define DBG_ERROR                      LOG_LVL_ERROR
#define DBG_WARNING                    LOG_LVL_WARNING
#define DBG_INFO                       LOG_LVL_INFO
#define DBG_LOG                        LOG_LVL_DBG

#if !defined(LOG_TAG)
    /* Compatible for rtdbg */
    #if defined(DBG_TAG)
        #define LOG_TAG                DBG_TAG
    #elif defined(DBG_SECTION_NAME)
        #define LOG_TAG                DBG_SECTION_NAME
    #else
        #define LOG_TAG                "NO_TAG"
    #endif
#endif /* !defined(LOG_TAG) */

#if !defined(LOG_LVL)
    /* Compatible for rtdbg */
    #if defined(DBG_LVL)
        #define LOG_LVL                DBG_LVL
    #elif defined(DBG_LEVEL)
        #define LOG_LVL                DBG_LEVEL
    #else
        #define LOG_LVL                LOG_LVL_DBG
    #endif
#endif /* !defined(LOG_LVL) */

#define dbg_log(level, ...)                                     \
    do                                                          \
    {                                                           \
        if ((level) <= LOG_LVL)                                 \
        {                                                       \
            ulog_output(level, LOG_TAG, RT_FALSE, __VA_ARGS__); \
        }                                                       \
    } while (0)

#if (LOG_LVL_DBG <= LOG_LVL)
    #define ulog_d(TAG, ...)                    ulog_output(LOG_LVL_DBG, TAG, RT_TRUE, __VA_ARGS__)
#else
    #define ulog_d(TAG, ...)
#endif

#if (LOG_LVL_INFO <= LOG_LVL)
    #define ulog_i(TAG, ...)                    ulog_output(LOG_LVL_INFO, TAG, RT_TRUE, __VA_ARGS__)
#else
    #define ulog_i(TAG, ...)
#endif

#if (LOG_LVL_WARNING <= LOG_LVL)
    #define ulog_w(TAG, ...)                    ulog_output(LOG_LVL_WARNING, TAG, RT_TRUE, __VA_ARGS__)
#else
    #define ulog_w(TAG, ...)
#endif

#if (LOG_LVL_ERROR <= LOG_LVL)
    #define ulog_e(TAG, ...)                    ulog_output(LOG_LVL_ERROR, TAG, RT_TRUE, __VA_ARGS__)
#else
    #define ulog_e(TAG, ...)
#endif

#if (LOG_LVL_DBG <= LOG_LVL)
    #define ulog_hex(TAG, width, buf, size)     ulog_hexdump(TAG, width, buf, size)
#else
    #define ulog_hex(TAG, width, buf, size)
#endif  

#define LOG_E(...)                              ulog_e(LOG_TAG, __VA_ARGS__)
#define LOG_W(...)                              ulog_w(LOG_TAG, __VA_ARGS__)
#define LOG_I(...)                              ulog_i(LOG_TAG, __VA_ARGS__)
#define LOG_D(...)                              ulog_d(LOG_TAG, __VA_ARGS__)
#define LOG_RAW(...)                            ulog_raw(__VA_ARGS__)
#define LOG_HEX(name, width, buf, size)         ulog_hex(name, width, buf, size)

/* assert for developer. */
#ifdef ULOG_ASSERT_ENABLE
    #define ULOG_ASSERT(EXPR)                                                   \
    if (!(EXPR))                                                                \
    {                                                                           \
        ulog_output(LOG_LVL_ASSERT, LOG_TAG, RT_TRUE,                           \
                    "(%s) has assert failed at %s:%ld.",                        \
                    #EXPR, __FUNCTION__, __LINE__);                             \
        ulog_flush();                                                           \
        while (1);                                                              \
    }
#else
    #define ULOG_ASSERT(EXPR)
#endif

/* ASSERT API definition */
#if !defined(ASSERT)
#define ASSERT           ULOG_ASSERT
#endif

/* Compatible for elog */
#undef ELOG_LVL_ASSERT
#undef ELOG_LVL_ERROR
#undef ELOG_LVL_WARN
#undef ELOG_LVL_INFO
#undef ELOG_LVL_DEBUG
#undef ELOG_LVL_VERBOSE

#define ELOG_LVL_ASSERT                LOG_LVL_ASSERT
#define ELOG_LVL_ERROR                 LOG_LVL_ERROR
#define ELOG_LVL_WARN                  LOG_LVL_WARNING
#define ELOG_LVL_INFO                  LOG_LVL_INFO
#define ELOG_LVL_DEBUG                 LOG_LVL_DBG
#define ELOG_LVL_VERBOSE               LOG_LVL_DBG

#undef assert
#undef log_e
#undef log_w
#undef log_i
#undef log_d
#undef log_v

#define assert                         ASSERT
#define log_e                          LOG_E
#define log_w                          LOG_W
#define log_i                          LOG_I
#define log_d                          LOG_D
#define log_v                          LOG_D
#define log_raw                        LOG_RAW
#define log_hex                        LOG_HEX

extern void ulog_output(rt_uint32_t level, const char *tag, rt_bool_t newline, const char *format, ...);
extern void ulog_raw(const char *format, ...);
extern void ulog_hexdump(const char *tag, rt_size_t width, rt_uint8_t *buf, rt_size_t size);
extern void ulog_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* RT_USING_ULOG */

#endif /* __RT_ULOG__H__ */

