#ifndef __ARCH_ATOMIC_H__
#define __ARCH_ATOMIC_H__

#include <os_types.h>

#ifdef __cplusplus
    extern "C" {
#endif

struct os_atomic
{
    volatile os_int32_t counter;
};
typedef struct os_atomic os_atomic_t;

/*
 * Apply to initializtion of variate which is defined by the "os_atomic_t".
 */
#define OS_ATOMIC_INIT(val) {(val)}

#define os_atomic_read(ptr)	         ((ptr)->counter)
#define os_atomic_set(ptr, value)    (((ptr)->counter) = (value))

extern void os_atomic_add(os_atomic_t *mem, os_int32_t value);
extern void os_atomic_sub(os_atomic_t *mem, os_int32_t value);
extern void os_atomic_inc(os_atomic_t *mem);
extern void os_atomic_dec(os_atomic_t *mem);

extern os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value);
extern os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value);
extern os_int32_t os_atomic_inc_return(os_atomic_t *mem);
extern os_int32_t os_atomic_dec_return(os_atomic_t *mem);

extern os_int32_t os_atomic_xchg(os_atomic_t* mem, os_int32_t value);
extern os_bool_t os_atomic_cmpxchg(os_atomic_t* mem, os_int32_t old, os_int32_t new);

extern void os_atomic_and(os_atomic_t* mem, os_int32_t value);
extern void os_atomic_or(os_atomic_t* mem, os_int32_t value);
extern void os_atomic_nand(os_atomic_t* mem, os_int32_t value);
extern void os_atomic_xor(os_atomic_t* mem, os_int32_t value);

extern os_bool_t os_atomic_test_bit(os_atomic_t* mem, os_int32_t nr);
extern void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr);
extern void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr);
extern void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr);

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_ATOMIC_H__ */

