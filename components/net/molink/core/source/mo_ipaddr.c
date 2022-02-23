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
 * @file        mo_ipaddr.c
 *
 * @brief       IP address function implementation
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ipaddr.h"
#include <os_types.h>
#include <os_assert.h>
#include <os_stddef.h>

#include <string.h>

/* Here for now until needed in other places in lwIP */
#ifndef isprint
#define in_range(c, lo, up) ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isprint(c)          in_range(c, 0x20, 0x7f)
#define isdigit(c)          in_range(c, '0', '9')
#define isxdigit(c)         (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)          in_range(c, 'a', 'z')
#define isspace(c)          (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#define xchar(i)            ((i) < 10 ? '0' + (i) : 'A' + (i)-10)
#endif

#if defined(MOLINK_USING_IPV4)
/**
 ***********************************************************************************************************************
 * @brief           Check whether "cp" is a valid ascii representation of
 *                  an Internet address and convert to a binary address.
 *
 * @details         This replaces inet_addr, the return value from which
 *                  cannot distinguish between failure and a local
 *                  broadcast address.
 *
 * @param[in]       cp              IP address in ascii representation (e.g. "127.0.0.1")
 * @param[in]       addr            pointer to which to save the ip address in network order
 *
 * @return          1 if cp could be converted to addr, 0 on failure
 ***********************************************************************************************************************
 */
int net_ip4addr_aton(const char *cp, ip4_addr_t *addr)
{
    uint32_t  val      = 0;
    uint8_t   base     = 0;
    uint32_t  parts[4] = {0};
    uint32_t *pp       = parts;
    char      c        = *cp;

    for (;;)
    {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
        {
            return 0;
        }
        val  = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c    = *++cp;
            }
            else
            {
                base = 8;
            }
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (uint32_t)(c - '0');
                c   = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (uint32_t)(c + 10 - (islower(c) ? 'a' : 'A'));
                c   = *++cp;
            }
            else
            {
                break;
            }
        }
        if (c == '.')
        {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
            {
                return 0;
            }
            *pp++ = val;
            c     = *++cp;
        }
        else
        {
            break;
        }
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
    {
        return 0;
    }
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1)
    {

    case 0:
        return 0; /* initial nondigit */

    case 1: /* a -- 32 bits */
        break;

    case 2: /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
        {
            return 0;
        }
        if (parts[0] > 0xff)
        {
            return 0;
        }
        val |= parts[0] << 24;
        break;

    case 3: /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    default:
        OS_ASSERT(0);

    }
    if (addr)
    {
        ip4_addr_set_u32(addr, htonl(val));
    }
    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param[in]       addr            IP address in network order to convert
 * @param[in]       buf             Target buffer where the string is stored
 * @param[in]       buflen          The length of buf
 *
 * @return          either pointer to buf which now holds the ASCII representation
 *                  of addr or OS_NULL if buf was too small
 ***********************************************************************************************************************
 */
char *net_ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen)
{
    char     inv[3] = {0};
    char    *rp     = OS_NULL;
    uint32_t s_addr = 0;
    uint8_t *ap     = OS_NULL;
    uint8_t  rem    = 0;
    uint8_t  n      = 0;
    uint8_t  i      = 0;
    int32_t  len    = 0;

    s_addr = ip4_addr_get_u32(addr);

    rp = buf;
    ap = (uint8_t *)&s_addr;
    for (n = 0; n < 4; n++)
    {
        i = 0;
        do
        {
            rem = *ap % (uint8_t)10;
            *ap /= (uint8_t)10;
            inv[i++] = (char)('0' + rem);
        } while (*ap);
        while (i--)
        {
            if (len++ >= buflen)
            {
                return OS_NULL;
            }
            *rp++ = inv[i];
        }
        if (len++ >= buflen)
        {
            return OS_NULL;
        }
        *rp++ = '.';
        ap++;
    }
    *--rp = 0;
    return buf;
}

/**
 ***********************************************************************************************************************
 * @brief           Convert numeric IP address into decimal dotted ASCII representation.
 *
 * @attention       returns ptr to static buffer; not reentrant!
 *
 * @param[in]       addr            IP address in network order to convert
 *
 * @return          pointer to a global static (!) buffer that holds the ASCII representation of addr
 ***********************************************************************************************************************
 */
char *net_ip4addr_ntoa(const ip4_addr_t *addr)
{
    static char str[IP4ADDR_STRLEN_MAX];
    return net_ip4addr_ntoa_r(addr, str, IP4ADDR_STRLEN_MAX);
}

/**
 ***********************************************************************************************************************
 * @brief           Ascii internet address interpretation routine
 *
 * @attention       The value returned is in network order
 *
 * @param[in]       cp              IP address in ascii representation (e.g. "127.0.0.1")
 *
 * @return          IP address in network order
 ***********************************************************************************************************************
 */
in_addr_t net_ipaddr_addr(const char *cp)
{
    ip4_addr_t val;

    if (net_ip4addr_aton(cp, &val))
    {
        return ip4_addr_get_u32(&val);
    }
    return (IPADDR_NONE);
}

#endif /* MOLINK_USING_IPV4 */

#if defined(MOLINK_USING_IPV6)

OS_WEAK const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;

/**
 ***********************************************************************************************************************
 * @brief           Check whether "cp" is a valid ascii representation of an IPv6 address
 *                  and convert to a binary address.
 *
 * @param[in]       cp              IPv6 address in ascii representation (e.g. "FF01::1")
 * @param[in]       addr            Pointer to which to save the ip address in network order
 *
 * @return          1 if cp could be converted to addr, 0 on failure
 ***********************************************************************************************************************
 */
int net_ip6addr_aton(const char *cp, ip6_addr_t *addr)
{
    uint32_t addr_index          = 0;
    uint32_t zero_blocks         = 8;
    uint32_t current_block_index = 0;
    uint32_t current_block_value = 0;
    const char *s;
    /**
     * Count the number of colons, to count the number of blocks in a "::" sequence
     * zero_blocks may be 1 even if there are no :: sequences
     */
    for (s = cp; *s != 0; s++)
    {
        if (*s == ':')
        {
            zero_blocks--;
        }
        else if (!isxdigit(*s))
        {
            break;
        }
    }

    /* parse each block */
    for (s = cp; *s != 0; s++)
    {
        if (*s == ':')
        {
            if (addr)
            {
                if (current_block_index & 0x1)
                {
                    addr->addr[addr_index++] |= current_block_value;
                }
                else
                {
                    addr->addr[addr_index] = current_block_value << 16;
                }
            }
            current_block_index++;
            current_block_value = 0;
            if (current_block_index > 7)
            {
                /* address too long! */
                return 0;
            }
            if (s[1] == ':')
            {
                if (s[2] == ':')
                {
                    /* invalid format: three successive colons */
                    return 0;
                }
                s++;
                /* "::" found, set zeros */
                while (zero_blocks > 0)
                {
                    zero_blocks--;
                    if (current_block_index & 0x1)
                    {
                        addr_index++;
                    }
                    else
                    {
                        if (addr)
                        {
                            addr->addr[addr_index] = 0;
                        }
                    }
                    current_block_index++;
                    if (current_block_index > 7)
                    {
                        /* address too long! */
                        return 0;
                    }
                }
            }
        }
        else if (isxdigit(*s))
        {
            /* add current digit */
            current_block_value =
                (current_block_value << 4) +
                (isdigit(*s) ? (uint32_t)(*s - '0') : (uint32_t)(10 + (islower(*s) ? *s - 'a' : *s - 'A')));
        }
        else
        {
            /* unexpected digit, space? CRLF? */
            break;
        }
    }

    if (addr)
    {
        if (current_block_index & 0x1)
        {
            addr->addr[addr_index++] |= current_block_value;
        }
        else
        {
            addr->addr[addr_index] = current_block_value << 16;
        }
    }

    /* convert to network byte order. */
    if (addr)
    {
        for (addr_index = 0; addr_index < 4; addr_index++)
        {
            addr->addr[addr_index] = htonl(addr->addr[addr_index]);
        }
    }

    if (current_block_index != 7)
    {
        return 0;
    }

    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param[in]       addr            IP6 address in network order to convert
 * @param[in]       buf             Target buffer where the string is stored
 * @param[in]       buflen          Length of buf.
 *
 * @return          Either pointer to buf which now holds the ASCII representation of addr
 *                  or OS_NULL if buf was too small
 ***********************************************************************************************************************
 */
char *net_ip6addr_ntoa_r(const ip6_addr_t *addr, char *buf, int buflen)
{
    uint32_t current_block_index = 0;
    uint32_t current_block_value = 0;
    uint32_t next_block_value    = 0;
    int32_t  i                   = 0;
    uint8_t  zero_flag           = 0;
    uint8_t  empty_block_flag    = 0; /* used to indicate a zero chain for "::' */

    for (current_block_index = 0; current_block_index < 8; current_block_index++)
    {
        /* get the current 16-bit block */
        current_block_value = htonl(addr->addr[current_block_index >> 1]);
        if ((current_block_index & 0x1) == 0)
        {
            current_block_value = current_block_value >> 16;
        }
        current_block_value &= 0xffff;

        /* Check for empty block. */
        if (current_block_value == 0)
        {
            if (current_block_index == 7 && empty_block_flag == 1)
            {
                /* special case, we must render a ':' for the last block. */
                buf[i++] = ':';
                if (i >= buflen)
                {
                    return OS_NULL;
                }
                break;
            }
            if (empty_block_flag == 0)
            {
                /* generate empty block "::", but only if more than one contiguous zero block,
                 * according to current formatting suggestions RFC 5952. */
                next_block_value = htonl(addr->addr[(current_block_index + 1) >> 1]);
                if ((current_block_index & 0x1) == 0x01)
                {
                    next_block_value = next_block_value >> 16;
                }
                next_block_value &= 0xffff;
                if (next_block_value == 0)
                {
                    empty_block_flag = 1;
                    buf[i++]         = ':';
                    if (i >= buflen)
                    {
                        return OS_NULL;
                    }
                    continue; /* move on to next block. */
                }
            }
            else if (empty_block_flag == 1)
            {
                /* move on to next block. */
                continue;
            }
        }
        else if (empty_block_flag == 1)
        {
            /* Set this flag value so we don't produce multiple empty blocks. */
            empty_block_flag = 2;
        }

        if (current_block_index > 0)
        {
            buf[i++] = ':';
            if (i >= buflen)
            {
                return OS_NULL;
            }
        }

        if ((current_block_value & 0xf000) == 0)
        {
            zero_flag = 1;
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf000) >> 12));
            zero_flag = 0;
            if (i >= buflen)
            {
                return OS_NULL;
            }
        }

        if (((current_block_value & 0xf00) == 0) && (zero_flag))
        {
            /* do nothing */
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf00) >> 8));
            zero_flag = 0;
            if (i >= buflen)
            {
                return OS_NULL;
            }
        }

        if (((current_block_value & 0xf0) == 0) && (zero_flag))
        {
            /* do nothing */
        }
        else
        {
            buf[i++]  = xchar(((current_block_value & 0xf0) >> 4));
            zero_flag = 0;
            if (i >= buflen)
            {
                return OS_NULL;
            }
        }

        buf[i++] = xchar((current_block_value & 0xf));
        if (i >= buflen)
        {
            return OS_NULL;
        }
    }

    buf[i] = 0;

    return buf;
}

/**
 ***********************************************************************************************************************
 * @brief           Convert numeric IPv6 address into ASCII representation
 *
 * @attention       returns ptr to static buffer; not reentrant!
 *
 * @param[in]       addr            IP6 address in network order to convert
 *
 * @return          pointer to a global static (!) buffer that holds the ASCII
 *                  representation of addr
 ***********************************************************************************************************************
 */
char *net_ip6addr_ntoa(const ip6_addr_t *addr)
{
    static char str[40];
    return net_ip6addr_ntoa_r(addr, str, 40);
}

#endif /* MOLINK_USING_IPV6 */

const char *net_inet_ntop(int af, const void *src, char *dst, int32_t size)
{
#define AF_INET  2
#define AF_INET6 10

    const char *ret      = OS_NULL;
    int         size_int = (int)size;
    if (size_int < 0)
    {
        return OS_NULL;
    }
    switch (af)
    {
#if NTEDEV_IPV4
    case AF_INET:
        return net_ip4addr_ntoa_r((const ip4_addr_t*)src, dst, size_int);
#endif
#if defined(MOLINK_USING_IPV6)
    case AF_INET6:
        return net_ip6addr_ntoa_r((const ip6_addr_t*)src, dst, size_int);
#endif
    default:
        break;
    }
    return ret;
}

int net_inet_pton(int af, const char *src, void *dst)
{
#define AF_INET  2
#define AF_INET6 10

    int err = 0;
    switch (af)
    {
#if defined(MOLINK_USING_IPV4)
    case AF_INET:
        err = net_ip4addr_aton(src, (ip4_addr_t*)dst);
        break;
#endif
#if defined(MOLINK_USING_IPV6)
    case AF_INET6:
    {
        /* convert into temporary variable since ip6_addr_t might be larger
         than in6_addr when scopes are enabled */
        ip6_addr_t addr;
        err = net_ip6addr_aton(src, &addr);
        if (err)
        {
            memcpy(dst, &addr.addr, sizeof(addr.addr));
        }
        break;
    }
#endif
    default:
        err = -1;
        break;
    }
    return err;
}
