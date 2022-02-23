#ifndef __ARCH_TASK_H__
#define __ARCH_TASK_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define OS_ARCH_STACK_ALIGN_SIZE   4

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

struct cpu_stack_frame_fpu
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;

    /* FPU registers */
    os_uint32_t S0;
    os_uint32_t S1;
    os_uint32_t S2;
    os_uint32_t S3;
    os_uint32_t S4;
    os_uint32_t S5;
    os_uint32_t S6;
    os_uint32_t S7;
    os_uint32_t S8;
    os_uint32_t S9;
    os_uint32_t S10;
    os_uint32_t S11;
    os_uint32_t S12;
    os_uint32_t S13;
    os_uint32_t S14;
    os_uint32_t S15;
    os_uint32_t FPSCR;
    os_uint32_t NO_NAME;
};


struct stack_frame_common
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

struct stack_frame_nofpu
{
    /* Push or pop stack manually */
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
    os_uint32_t exc_return;

    /* Push or pop stack automatically */
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;
};

struct stack_frame_fpu
{
    /* begin: Push or pop stack manually */
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
    os_uint32_t exc_return;

    /* FPU registers */
    os_uint32_t s16;
    os_uint32_t s17;
    os_uint32_t s18;
    os_uint32_t s19;
    os_uint32_t s20;
    os_uint32_t s21;
    os_uint32_t s22;
    os_uint32_t s23;
    os_uint32_t s24;
    os_uint32_t s25;
    os_uint32_t s26;
    os_uint32_t s27;
    os_uint32_t s28;
    os_uint32_t s29;
    os_uint32_t s30;
    os_uint32_t s31;
    /* end: Push or pop stack manually */

    /* begin: Push or pop stack automatically */
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;

    /* FPU registers */
    os_uint32_t S0;
    os_uint32_t S1;
    os_uint32_t S2;
    os_uint32_t S3;
    os_uint32_t S4;
    os_uint32_t S5;
    os_uint32_t S6;
    os_uint32_t S7;
    os_uint32_t S8;
    os_uint32_t S9;
    os_uint32_t S10;
    os_uint32_t S11;
    os_uint32_t S12;
    os_uint32_t S13;
    os_uint32_t S14;
    os_uint32_t S15;
    os_uint32_t FPSCR;
    os_uint32_t NO_NAME;
    /* end: Push or pop stack automatically */
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

