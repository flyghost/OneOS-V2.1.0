/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        os_util.c
 *
 * @brief       This file provides some utility functions similar to C library, so kernel does not depend on any
 *              C library.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_stddef.h>
#include <arch_interrupt.h>
#include <string.h>
#include <board.h>


/* Private function */
#define isdigit(c)          ((unsigned int)((c) - '0') < 10)

OS_INLINE int divide(long long *n, int base)
{
    int res;

    /* Optimized for processor which does not support divide instructions. */
    if (10 == base)
    {
        res = (int)(((unsigned long long)(*n)) % 10U);
        *n  = (long long)(((unsigned long long)(*n)) / 10U);
    }
    else
    {
        res = (int)(((unsigned long long)(*n)) % base);
        *n  = (long long)(((unsigned long long)(*n)) / base);
    }

    return res;
}

OS_INLINE int skip_atoi(const char **s)
{
    register int i = 0;
    
    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    
    return i;
}

#define ZEROPAD     (1 << 0)    /* Pad with zero */
#define SIGN        (1 << 1)    /* Unsigned/signed long */
#define PLUS        (1 << 2)    /* Show plus */
#define SPACE       (1 << 3)    /* Space if plus */
#define LEFT        (1 << 4)    /* Left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* Use 'ABCDEF' instead of 'abcdef' */

static char *print_number(char      *buf,
                          char      *end,
                          long long num,
                          int       base,
                          int       s,
                          int       precision,
                          int       type)
{
    char              c;
    char              sign;
    char              tmp[32];
    int               precision_bak;
    const char        *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int      i;
    register int      size;

    precision_bak = precision;
    size          = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
    {
        type &= ~ZEROPAD;
    }
    
    c = (type & ZEROPAD) ? '0' : ' ';

    /* Get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num  = -num;
        }
        else if (type & PLUS)
        {
            sign = '+';
        }
        else if (type & SPACE)
        {
            sign = ' ';
        }
        else
        {
            ;
        }
    }

    if (type & SPECIAL)
    {
        if (base == 16)
        {
            size -= 2;
        }
        else if (base == 8)
        {
            size--;
        }
        else
        {
            ;
        }
    }

    i = 0;
    if (num == 0)
    {
        tmp[i] = '0';
        i++;
    }
    else
    {
        while (num != 0)
        {
            tmp[i] = digits[divide(&num, base)];
            i++;
        }
    }

    if (i > precision)
    {
        precision = i;
    }
    size -= precision;

    if (!(type & (ZEROPAD | LEFT)))
    {
        if ((sign) && (size > 0))
        {
            size--;
        }
        
        while (size-- > 0)
        {
            if (buf < end)
            {
                *buf = ' ';
            }
            
            ++buf;
        }
    }

    if (sign)
    {
        if (buf < end)
        {
            *buf = sign;
        }
        
        --size;
        ++buf;
    }

    if (type & SPECIAL)
    {
        if (base == 8)
        {
            if (buf < end)
            {
                *buf = '0';
            }
            
            ++buf;
        }
        else if (base == 16)
        {
            if (buf < end)
            {
                *buf = '0';
            }
            ++buf;

            if (buf < end)
            {
                *buf = type & LARGE ? 'X' : 'x';
            }
            ++buf;
        }
        else
        {
            ;
        }
    }

    /* No align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf < end)
            {
                *buf = c;
            }
            
            ++buf;
        }
    }

    while (i < precision--)
    {
        if (buf < end)
        {
            *buf = '0';
        }
        
        ++buf;
    }

    /* Put number in the temporary buffer */
    while ((i-- > 0) && (precision_bak != 0))
    {
        if (buf < end)
        {
            *buf = tmp[i];
        }
        ++buf;
    }

    while (size-- > 0)
    {
        if (buf < end)
        {
            *buf = ' ';
        }
        ++buf;
    }

    return buf;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will fill a formatted string to buffer.
 *
 * @param[out]      buf             The buffer to save formatted string.
 * @param[in]       size            The size of buffer.
 * @param[in]       fmt             The format.
 * @param[in]       args            The arguments.
 *
 * @return          The length of filling string to buffer.
 * @retval          >= 0            The actual fill length.
 * @retval          < 0             Fill buffer failed.
 ***********************************************************************************************************************
 */
os_int32_t os_vsnprintf(char *buf, os_size_t size, const char *fmt, va_list args)
{
    unsigned long long num;
    int i;
    int len;
    char *str;
    char *end;
    char c;
    const char *s;

    os_uint8_t base;            /* The base of number */
    os_uint8_t flags;           /* Flags to print number */
    os_uint8_t qualifier;       /* 'h', 'l', or 'L' for integer fields */
    os_int32_t field_width;     /* Width of output field */
    int precision;              /* min. # of digits for integers and max for a string */

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *) - 1);
        size = end - buf;
    }

    for ( ; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str < end)
            {
                *str = *fmt;
            }
            ++str;
            
            continue;
        }

        /* Process flags */
        flags = 0;
        while (1)
        {
            /* Skips the first '%' also */
            ++fmt;
            if (*fmt == '-')
            {
                flags |= LEFT;
            }
            else if (*fmt == '+')
            {
                flags |= PLUS;
            }
            else if (*fmt == ' ')
            {
                flags |= SPACE;
            }
            else if (*fmt == '#')
            {
                flags |= SPECIAL;
            }
            else if (*fmt == '0')
            {
                flags |= ZEROPAD;
            }
            else
            {
                break;
            }
        }

        /* Get field width */
        field_width = -1;
        if (isdigit(*fmt))
        {
            field_width = skip_atoi(&fmt);
        }
        else if (*fmt == '*')
        {
            ++fmt;
            
            /* It's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }
        else
        {
            ;
        }

        /* Get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++fmt;
            if (isdigit(*fmt))
            {
                precision = skip_atoi(&fmt);
            }
            else if (*fmt == '*')
            {
                ++fmt;
                
                /* It's the next argument */
                precision = va_arg(args, int);
            }
            else
            {
                ;
            }
            
            if (precision < 0)
            {
                precision = 0;
            }
        }
        
        /* Get the conversion qualifier */
        qualifier = 0;
        if ((*fmt == 'h') || (*fmt == 'l') || (*fmt == 'L'))
        {
            qualifier = *fmt;
            ++fmt;
            
            if ((qualifier == 'l') && (*fmt == 'l'))
            {
                qualifier = 'L';
                ++fmt;
            }
        }

        /* The default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str < end)
                    {
                        *str = ' ';
                    }
                    
                    ++str;
                }
            }

            /* Get character */
            c = (os_uint8_t)va_arg(args, int);
            if (str < end)
            {
                *str = c;
            }
            ++str;

            /* Put width */
            while (--field_width > 0)
            {
                if (str < end)
                {
                    *str = ' ';
                }
                ++str;
            }
            
            continue;
        case 's':
            s = va_arg(args, char *);
            if (!s)
            {
                s = "(NULL)";
            }
            
            len = strlen(s);
            if (precision > 0 && len > precision)
            {
                len = precision;
            }
            
            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str < end)
                    {
                        *str = ' ';
                    }
                    ++str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str < end)
                {
                    *str = *s;
                }
                
                ++str;
                ++s;
            }

            while (len < field_width--)
            {
                if (str < end)
                {
                    *str = ' ';
                }
                ++str;
            }
            
            continue;
        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags       |= ZEROPAD;
            }

            str = print_number(str,
                               end,
                               (unsigned long)va_arg(args, void *),
                               16,
                               field_width,
                               precision,
                               flags);

            continue;
        case '%':
            if (str < end)
            {
                *str = '%';
            }
            ++str;
            
            continue;
        /* Integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;
            
        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str < end)
            {
                *str = '%';
            }
            ++str;

            if (*fmt)
            {
                if (str < end)
                {
                    *str = *fmt;
                }
                ++str;
            }
            else
            {
                --fmt;
            }
            
            continue;
        }

        if (qualifier == 'L')
        {
            num = va_arg(args, long long);
        }
        else if (qualifier == 'l')
        {
            num = va_arg(args, os_uint32_t);
            if (flags & SIGN)
            {
                num = (os_int32_t)num;
            }
        }
        else if (qualifier == 'h')
        {
            num = (os_uint16_t)va_arg(args, os_int32_t);
            if (flags & SIGN)
            {
                num = (os_int16_t)num;
            }
        }
        else
        {
            num = va_arg(args, os_uint32_t);
            if (flags & SIGN)
            {
                num = (os_int32_t)num;
            }
        }

        str = print_number(str, end, num, base, field_width, precision, flags);
    }

    if (size > 0)
    {
        if (str < end)
        {
            *str = '\0';
        }
        else
        {
            end[-1] = '\0';
        }
    }

    /* The trailing null byte doesn't count towards the total ++str;*/
    return str - buf;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will fill a formatted string to buffer.
 *
 * @param[out]      buf             The buffer to save formatted string.
 * @param[in]       size            The size of buffer.
 * @param[in]       fmt             The format.
 *
 * @return          The length of filling string to buffer.
 * @retval          >= 0            The actual fill length.
 * @retval          < 0             Fill buffer failed.
 ***********************************************************************************************************************
 */
os_int32_t os_snprintf(char *buf, os_size_t size, const char *fmt, ...)
{
    os_int32_t n;
    va_list    args;

    va_start(args, fmt);
    n = os_vsnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

#ifdef OS_DEBUG
/**
 ***********************************************************************************************************************
 * @brief           This function will print a formatted string on system console.
 *
 * @param[in]       fmt             The format.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_kprintf(const char *fmt, ...)
{
    va_list     args;
    os_size_t   length;
    static char log_buff[OS_LOG_BUFF_SIZE];
    os_ubase_t  irq_save;

    irq_save = os_irq_lock();

    va_start(args, fmt);
    /* 
     * The return value of os_vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the log_buf, we have to adjust the output
     * length.
     */
    length = os_vsnprintf(log_buff, sizeof(log_buff), fmt, args);
    if (length > sizeof(log_buff) - 1)
    {
        length = sizeof(log_buff) - 1;
        log_buff[length] = '\0';
    }
    
    va_end(args);

    os_hw_console_output(log_buff);

    os_irq_unlock(irq_save);

    return;
}
#else
void os_kprintf(const char *fmt, ...)
{
    /* Disable debug, do nothing */
    return;
}
#endif /* OS_DEBUG */

