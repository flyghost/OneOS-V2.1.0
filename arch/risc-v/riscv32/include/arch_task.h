#ifndef __ARCH_TASK_H__
#define __ARCH_TASK_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define OS_ARCH_STACK_ALIGN_SIZE   4

struct os_hw_stack_frame
{
    os_ubase_t mstatus;    /*              - machine status register             */
    os_ubase_t epc;        /* epc - epc    - program counter                     */
    os_ubase_t ra;         /* x1  - ra     - return address for jumps            */
    os_ubase_t gp;         /* x3  - gp     - global pointer                      */
    os_ubase_t tp;         /* x4  - tp     - task pointer                        */
    os_ubase_t t0;         /* x5  - t0     - temporary register 0                */
    os_ubase_t t1;         /* x6  - t1     - temporary register 1                */
    os_ubase_t t2;         /* x7  - t2     - temporary register 2                */
    os_ubase_t s0_fp;      /* x8  - s0/fp  - saved register 0 or frame pointer   */
    os_ubase_t s1;         /* x9  - s1     - saved register 1                    */
    os_ubase_t a0;         /* x10 - a0     - return value or function argument 0 */
    os_ubase_t a1;         /* x11 - a1     - return value or function argument 1 */
    os_ubase_t a2;         /* x12 - a2     - function argument 2                 */
    os_ubase_t a3;         /* x13 - a3     - function argument 3                 */
    os_ubase_t a4;         /* x14 - a4     - function argument 4                 */
    os_ubase_t a5;         /* x15 - a5     - function argument 5                 */
    os_ubase_t a6;         /* x16 - a6     - function argument 6                 */
    os_ubase_t a7;         /* x17 - a7     - function argument 7                 */
    os_ubase_t s2;         /* x18 - s2     - saved register 2                    */
    os_ubase_t s3;         /* x19 - s3     - saved register 3                    */
    os_ubase_t s4;         /* x20 - s4     - saved register 4                    */
    os_ubase_t s5;         /* x21 - s5     - saved register 5                    */
    os_ubase_t s6;         /* x22 - s6     - saved register 6                    */
    os_ubase_t s7;         /* x23 - s7     - saved register 7                    */
    os_ubase_t s8;         /* x24 - s8     - saved register 8                    */
    os_ubase_t s9;         /* x25 - s9     - saved register 9                    */
    os_ubase_t s10;        /* x26 - s10    - saved register 10                   */
    os_ubase_t s11;        /* x27 - s11    - saved register 11                   */
    os_ubase_t t3;         /* x28 - t3     - temporary register 3                */
    os_ubase_t t4;         /* x29 - t4     - temporary register 4                */
    os_ubase_t t5;         /* x30 - t5     - temporary register 5                */
    os_ubase_t t6;         /* x31 - t6     - temporary register 6                */
};

extern void       *os_hw_stack_init(void         *tentry,
                                    void         *parameter,
                                    void         *stack_begin,
                                    os_uint32_t   stack_size,
                                    void         *texit);

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

