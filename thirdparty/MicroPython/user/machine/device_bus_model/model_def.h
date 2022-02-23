#ifndef __MODEL_DEF_H__
#define __MODEL_DEF_H__

#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM Compiler 							*/
#include <stdarg.h>
#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))

#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#elif defined(__GNUC__) /* GNU GCC Compiler */
#include <stdarg.h>

#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))
#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#elif defined(_WIN32)

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))
#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#endif


/* Similar as the NULL in C library */
#ifdef __cplusplus
#define MP_NULL                         0
#else
#define MP_NULL                         ((void *)0)
#endif

/* Boolean value definitions */
#define MP_FALSE                        0
#define MP_TRUE                         1


//#define SIZEOF(x) ((char*)(&(x) + 1) - (char*)&(x))

typedef unsigned  char	u8_t;
typedef unsigned  short	u16_t;
#ifndef LWIP_HDR_ARCH_H
typedef unsigned  int	u32_t;
#endif
typedef signed int		i32_t;
typedef unsigned long   ubase_t;


typedef ubase_t               mp_size_t;              /* Type for size */

/**
 *********************************************************************************************************
 *                                      define function pointer
 * @note	the pointer contain four parts: function _  return type (0: void) _ parameters number _t
 *********************************************************************************************************
*/
typedef void  (*fun_0_0_t)(void);
typedef int   (*fun_i_0_t)(void);
typedef u32_t (*fun_u32_1_t)(void *);
typedef int   (*fun_i_1_t)(void *);		// funcion _  return type _ param _ t
typedef void  (*fun_0_1_t)(void *);
typedef void  *(*fun_p_0_t)(void);
typedef void  *(*fun_p_1_t)(void *);
typedef void  *(*fun_p_1_t)(void *);
typedef void  *(*fun_p_2_t)(void *, void *);
typedef void  (*fun_0_2x_t)(const char *, ...);
typedef int   (*fun_i_2_t)(void *, void *);
typedef int   (*fun_i_3_t)(void *, void *, void *);
typedef int   (*fun_i_4_t)(void *, void *, void *, void *);
typedef int   (*fun_i_4x_t)(char *, ubase_t, const char *, va_list);
typedef void  *(*fun_p_5_t)(void *, void *, void *, void *, void *);
typedef void  *(*fun_p_6_t)(void *, void *, void *, void *, void *, void *);

#endif
