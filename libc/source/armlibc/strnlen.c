#include <extension/string_ext.h>

size_t strnlen(const char *s, size_t maxlen)
{
    const char *str_tmp;

    for (str_tmp = s; (*str_tmp != '\0') && (str_tmp - s < maxlen); str_tmp++)
    {
        ;
    }

    return (str_tmp - s);
}

