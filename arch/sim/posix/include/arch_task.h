#ifndef __ARCH_TASK_H__
#define __ARCH_TASK_H__

#include <os_types.h>
#include <os_task.h>

#ifdef __cplusplus
    extern "C" {
#endif

/* The minimum stack space of linux threads is 16K, and 16K alignment is required. */
#define OS_ARCH_STACK_ALIGN_SIZE   (16*1024)

extern void       *os_hw_stack_init(void        (*task_entry)(void *arg),
                                    void         *arg,
                                    void         *stack_begin,
                                    os_uint32_t   stack_size,
                                    void        (*task_exit)(void));

extern os_uint32_t os_hw_stack_max_used(void *stack_begin, os_uint32_t stack_size);

extern void        os_first_task_start(void);
extern void        os_task_switch(void);

extern void        disable_interrupts(void);
extern void        enable_interrupts(void);

#ifdef OS_USING_OVERFLOW_CHECK
extern os_bool_t   os_task_stack_is_overflow(void *stack_top, void *stack_begin, void *stack_end);
#endif 

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_TASK_H__ */

