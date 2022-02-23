#ifndef __ARCH_CACHE_H__
#define __ARCH_CACHE_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

enum OS_HW_CACHE_OPS
{
    OS_HW_CACHE_FLUSH      = 0x01,
    OS_HW_CACHE_INVALIDATE = 0x02,
};

extern void        os_cpu_icache_enable(void);
extern void        os_cpu_icache_disable(void);
extern os_base_t   os_cpu_icache_status(void);
extern void        os_cpu_icache_ops(os_int32_t ops, void* addr, os_int32_t size);

extern void        os_cpu_dcache_enable(void);
extern void        os_cpu_dcache_disable(void);
extern os_base_t   os_cpu_dcache_status(void);
extern void        os_cpu_dcache_ops(os_int32_t ops, void* addr, os_int32_t size);

#ifdef __cplusplus
    }
#endif


#endif /* __ARCH_CACHE_H__ */

