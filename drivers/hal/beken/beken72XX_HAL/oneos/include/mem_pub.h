#ifndef _MEM_PUB_H_
#define _MEM_PUB_H_

#include <stdarg.h>
#include <os_types.h>

// oneos adapte for beken driver 
extern os_int32_t  os_memcmp(const void *str1, const void *str2, os_size_t count);
extern void       *os_memmove(void *dst, const void *src, os_size_t count);
extern void       *os_memcpy(void *dst, const void *src, os_size_t count);
extern void       *os_memset(void *src, os_uint8_t val, os_size_t count);
extern void *os_malloc(os_size_t nbytes);
extern void  os_free(void *ptr);
extern void *os_realloc(void *ptr, os_size_t nbytes);

// beken special function
void os_mem_init(void);
void *os_zalloc(size_t size);
int os_memcmp_const(const void *a, const void *b, size_t len);

#endif // _MEM_PUB_H_

// EOF
