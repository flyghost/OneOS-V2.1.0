#ifndef __ARCH_MISC_H__
#define __ARCH_MISC_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

extern os_int32_t  os_ffs(os_uint32_t value);
extern os_int32_t  os_fls(os_uint32_t value);
extern void       *os_get_current_task_sp(void);

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_MISC_H__ */

