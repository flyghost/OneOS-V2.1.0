#ifndef __ACW_DEBUG_H__
#define __ACW_DEBUG_H__

#define DBG_EXT_TAG "acw"
#define DBG_EXT_LVL DBG_EXT_DEBUG

#ifdef OS_USING_DLOG
#include <dlog.h>

#define DBG_EXT_ERROR               DLOG_ERROR
#define DBG_EXT_WARNING             DLOG_WARNING
#define DBG_EXT_INFO                DLOG_INFO
#define DBG_EXT_DEBUG               DLOG_DEBUG

/* Print logs with function name and file line number, and use dlog interface */
#if (DBG_EXT_ERROR <= DBG_EXT_LVL)
#define ACW_PRINT_E(fmt, ...)     \
    dlog_output(DBG_EXT_ERROR, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_E(fmt, ...)
#endif

#if (DBG_EXT_WARNING <= DBG_EXT_LVL)
#define ACW_PRINT_W(fmt, ...)     \
    dlog_output(DBG_EXT_WARNING, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_W(fmt, ...)
#endif

#if (DBG_EXT_INFO <= DBG_EXT_LVL)
#define ACW_PRINT_I(fmt, ...)     \
    dlog_output(DBG_EXT_INFO, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_I(fmt, ...)
#endif

#if (DBG_EXT_DEBUG <= DBG_EXT_LVL)
#define ACW_PRINT_D(fmt, ...)     \
    dlog_output(DBG_EXT_DEBUG, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_D(fmt, ...)
#endif

#else /* Not define OS_USING_DLOG */

#define DBG_EXT_ERROR               0   /* Equal to KERN_ERROR */
#define DBG_EXT_WARNING             1   /* Equal to KERN_WARNING */
#define DBG_EXT_INFO                2   /* Equal to KERN_INFO */
#define DBG_EXT_DEBUG               3   /* Equal to KERN_DEBUG */

/* Print logs with function name and file line number, and use kernel log interface */
#if (DBG_EXT_ERROR <= DBG_EXT_LVL)
#define ACW_PRINT_E(fmt, ...)     \
    os_kernel_print(DBG_EXT_ERROR, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_E(fmt, ...)
#endif

#if (DBG_EXT_WARNING <= DBG_EXT_LVL)
#define ACW_PRINT_W(fmt, ...)     \
    os_kernel_print(DBG_EXT_WARNING, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_W(fmt, ...)
#endif

#if (DBG_EXT_INFO <= DBG_EXT_LVL)
#define ACW_PRINT_I(fmt, ...)     \
    os_kernel_print(DBG_EXT_INFO, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_I(fmt, ...)
#endif

#if (DBG_EXT_DEBUG <= DBG_EXT_LVL)
#define ACW_PRINT_D(fmt, ...)     \
    os_kernel_print(DBG_EXT_DEBUG, DBG_EXT_TAG, OS_TRUE, "[%s][%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ACW_PRINT_D(fmt, ...)
#endif

#endif /* define OS_USING_DLOG */

#endif /* end of __ACW_DEBUG_H__ */
