#include <extension/string_ext.h>
#include <os_stddef.h>
#include <os_memory.h>

char *strdup(const char *s)
{
    size_t  len;
    char    *str_tmp;

    len = strlen(s) + 1;
#if defined (OS_USING_SYS_HEAP)
    str_tmp = (char *)os_malloc(len);
#else
    str_tmp = OS_NULL;
#endif
    if (!str_tmp)
    {
        return OS_NULL;
    }

    memcpy(str_tmp, s, len);

    return str_tmp;
}

