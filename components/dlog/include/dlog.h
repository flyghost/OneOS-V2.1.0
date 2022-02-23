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
 * @file        dlog.h
 *
 * @brief       The header for dlog.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __DLOG_H__
#define __DLOG_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_DLOG
/* Logger level, the number is compatible for syslog */
#define DLOG_ERROR                      3           /* Error conditions */
#define DLOG_WARNING                    4           /* Warning conditions */
#define DLOG_INFO                       6           /* Informational */
#define DLOG_DEBUG                      7           /* Debug-level messages */

#ifdef DLOG_WITH_FUNC_LINE

#if (DLOG_ERROR <= DLOG_COMPILE_LEVEL)
#define LOG_E(tag, fmt, ...)     \
    dlog_output(DLOG_ERROR,   tag, OS_TRUE, fmt " [%s][%d]", ##__VA_ARGS__, __FUNCTION__, __LINE__)
#else
#define LOG_E(tag, fmt, ...)
#endif

#if (DLOG_WARNING <= DLOG_COMPILE_LEVEL)
#define LOG_W(tag, fmt, ...)     \
    dlog_output(DLOG_WARNING, tag, OS_TRUE, fmt " [%s][%d]", ##__VA_ARGS__, __FUNCTION__, __LINE__)
#else
#define LOG_W(tag, fmt, ...)
#endif

#if (DLOG_INFO <= DLOG_COMPILE_LEVEL)
#define LOG_I(tag, fmt, ...)     \
    dlog_output(DLOG_INFO,    tag, OS_TRUE, fmt " [%s][%d]", ##__VA_ARGS__, __FUNCTION__, __LINE__)
#else
#define LOG_I(tag, fmt, ...)
#endif

#if (DLOG_DEBUG <= DLOG_COMPILE_LEVEL)
#define LOG_D(tag, fmt, ...)     \
    dlog_output(DLOG_DEBUG,   tag, OS_TRUE, fmt " [%s][%d]", ##__VA_ARGS__, __FUNCTION__, __LINE__)
#else
#define LOG_D(tag, fmt, ...)
#endif

/* Print logs without function name and file line number */
#else /* Not define OS_DEBUG_LOG_WITH_FUNC_LINE */

#if (DLOG_ERROR <= DLOG_COMPILE_LEVEL)
#define LOG_E(tag, fmt, ...)     \
    dlog_output(DLOG_ERROR,   tag, OS_TRUE, fmt, ##__VA_ARGS__)
#else
#define LOG_E(tag, fmt, ...)
#endif

#if (DLOG_WARNING <= DLOG_COMPILE_LEVEL)
#define LOG_W(tag, fmt, ...)     \
    dlog_output(DLOG_WARNING, tag, OS_TRUE, fmt, ##__VA_ARGS__)
#else
#define LOG_W(tag, fmt, ...)
#endif

#if (DLOG_INFO <= DLOG_COMPILE_LEVEL)
#define LOG_I(tag, fmt, ...)     \
    dlog_output(DLOG_INFO,    tag, OS_TRUE, fmt, ##__VA_ARGS__)
#else
#define LOG_I(tag, fmt, ...)
#endif

#if (DLOG_DEBUG <= DLOG_COMPILE_LEVEL)
#define LOG_D(tag, fmt, ...)     \
    dlog_output(DLOG_DEBUG,   tag, OS_TRUE, fmt, ##__VA_ARGS__)
#else
#define LOG_D(tag, fmt, ...)
#endif

#endif /* DLOG_WITH_FUNC_LINE */

#define LOG_RAW(fmt, ...)               dlog_raw(fmt, ##__VA_ARGS__)
#define LOG_HEX(tag, width, buf, size)  dlog_hexdump(tag, width, buf, size)

/**
 ***********************************************************************************************************************
 * @struct      dlog_backend
 *
 * @brief       Define backend structure.
 ***********************************************************************************************************************
 */
struct dlog_backend
{
    os_list_node_t  list_node;

    char            name[OS_NAME_MAX + 1];                      /* Name of log backend. */

    os_bool_t       support_color;                              /* Whether backend support color. */
    os_bool_t       support_isr;                                /* Whether backend support isr. */
    
    void          (*init)  (struct dlog_backend *backend);      /* Initialize backend. */
    void          (*deinit)(struct dlog_backend *backend);      /* De-initialize backend. */
    
    void          (*output)(struct dlog_backend *backend,
                            char                *log,
                            os_size_t            len);          /* Output log to backend */
                   
    void          (*flush) (struct dlog_backend *backend);      /* Flush logs in the buffer of backend */
};
typedef struct dlog_backend dlog_backend_t;

extern os_err_t    dlog_init(void);
extern void        dlog_output(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, ...);
extern void        dlog_raw(const char *format, ...);
extern void        dlog_hexdump(const char *tag, os_size_t width, os_uint8_t *buf, os_size_t size);
extern os_err_t    dlog_backend_register(dlog_backend_t *backend);
extern os_err_t    dlog_backend_unregister(dlog_backend_t *backend);
extern void        dlog_global_lvl_set(os_uint16_t level);
extern os_uint16_t dlog_global_lvl_get(void);
extern void        dlog_flush(void);

#else   /* Not define OS_USING_DLOG */

#define LOG_E(tag, fmt, ...)
#define LOG_W(tag, fmt, ...)
#define LOG_I(tag, fmt, ...)
#define LOG_D(tag, fmt, ...)

#define LOG_RAW(fmt, ...)
#define LOG_HEX(tag, width, buf, size)
#endif  /* OS_USING_DLOG */

#ifdef __cplusplus
}
#endif

#endif /* __DLOG_H__ */

