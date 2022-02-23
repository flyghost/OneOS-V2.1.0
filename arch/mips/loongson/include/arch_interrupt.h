#ifndef __ARCH_INTERRUPT_H__
#define __ARCH_INTERRUPT_H__

#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    extern os_ubase_t  os_irq_lock(void);
    extern void        os_irq_unlock(os_ubase_t irq_save);

    extern void        os_irq_disable(void);
    extern void        os_irq_enable(void);

    extern os_bool_t   os_is_irq_active(void);
    extern os_bool_t   os_is_irq_disabled(void);
    extern os_uint32_t os_irq_num(void);

    extern os_bool_t   os_is_fault_active(void);

    extern void os_interrupt_enter(void);

    extern void os_interrupt_leave(void);

    extern os_uint32_t os_interrupt_get_nest(void);

#ifdef __cplusplus
}
#endif

#endif /* __ARCH_INTERRUPT_H__ */

