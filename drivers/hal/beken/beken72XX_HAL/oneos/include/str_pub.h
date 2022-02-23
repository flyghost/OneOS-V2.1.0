#ifndef _STR_PUB_H_
#define _STR_PUB_H_

#include <stdarg.h>
#include <os_types.h>
#include "typedef.h"

/* Adpt oneos string functions for beken drivers. */

extern os_size_t   os_strlen(const char *str);
extern os_int32_t  os_strcmp(const char *str1, const char *str2);
extern os_int32_t  os_strncmp(const char *str1, const char *str2, os_size_t count);
extern os_int32_t  os_snprintf(char *buf, os_size_t size, const char *fmt, ...);
extern os_int32_t  os_vsnprintf(char *buf, os_size_t size, const char *fmt, va_list args);
extern char       *os_strncpy(char *dst, const char *src, os_size_t count);
extern char       *os_strcpy(char *dst, const char *src);
extern char       *os_strchr(const char *str, char ch);
extern char       *os_strstr(const char *str1, const char *str2);

#ifdef OS_USING_HEAP
extern char *os_strdup(const char *str);
#endif /* OS_USING_HEAP */

// beken special functions
UINT32 os_strtoul(const char *nptr, char **endptr, int base);
int os_strcasecmp(const char *s1, const char *s2);
int os_strncasecmp(const char *s1, const char *s2, size_t n);
char *os_strrchr(const char *s, int c);
size_t os_strlcpy(char *dest, const char *src, size_t siz);

#endif // _STR_PUB_H_

// EOF
