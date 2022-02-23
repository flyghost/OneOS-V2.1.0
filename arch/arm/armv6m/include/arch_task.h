#ifndef __ARCH_TASK_H__
#define __ARCH_TASK_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define OS_ARCH_STACK_ALIGN_SIZE   8


struct cpu_stack_frame
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;
};

struct manual_stack_frame
{   
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
    os_uint32_t exc_return;
};

struct stack_frame
{
    /* Push or pop stack manually */
    struct manual_stack_frame   manual_frame;

    /* Push or pop stack automatically */
    struct cpu_stack_frame      cpu_frame;

};

extern void       *os_hw_stack_init(void        (*task_entry)(void *arg),
                                    void         *arg,
                                    void         *stack_begin,
                                    os_uint32_t   stack_size,
                                    void        (*task_exit)(void));

extern os_uint32_t os_hw_stack_max_used(void *stack_begin, os_uint32_t stack_size);

extern void        os_first_task_start(void);
extern void        os_task_switch(void);

#ifdef OS_USING_OVERFLOW_CHECK
extern os_bool_t   os_task_stack_is_overflow(void *stack_top, void *stack_begin, void *stack_end);            
#endif 

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_TASK_H__ */

