#include <oneos_config.h>
#include <os_types.h>

#if defined(__CC_ARM)
const os_uint8_t gs_lowest_bit_bitmap[] =
{
    /* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};
os_int32_t os_ffs(os_uint32_t value)
{
    if (value == 0)
    {
        return 0;
    }
    
    if (value & 0xff)
    {
        return gs_lowest_bit_bitmap[value & 0xff] + 1;
    }
    
    if (value & 0xff00)
    {
        return gs_lowest_bit_bitmap[(value & 0xff00) >> 8] + 9;
    }
    
    if (value & 0xff0000)
    {
        return gs_lowest_bit_bitmap[(value & 0xff0000) >> 16] + 17;
    }
    
    return gs_lowest_bit_bitmap[(value & 0xff000000) >> 24] + 25;
}

__asm void *os_get_current_task_sp(void) 
{
    MRS     R0, PSP
    BX      LR
}

#elif defined(__GNUC__) || defined(__CLANG_ARM)
os_int32_t os_ffs(os_uint32_t value)
{
    return __builtin_ffs(value);
}

void *os_get_current_task_sp(void) 
{
    register void *result;

    __asm__ __volatile__(
        "   MRS %0, PSP\n"
        :   "=r"(result));

    return result;
}

#endif

os_int32_t os_fls(os_uint32_t value)
{
    os_int32_t pos;

    pos = 32;
     
    if (!value)
    {
        pos = 0;
    }
    else
    {
        if (!(value & 0xFFFF0000U))
        {
            value <<= 16;
            pos    -= 16;
        }
        
        if (!(value & 0xFF000000U))
        {
            value <<= 8;
            pos    -= 8;
        }
        
        if (!(value & 0xF0000000U))
        {
            value <<= 4;
            pos -= 4;
        }
        
        if (!(value & 0xC0000000U))
        {
            value <<= 2;
            pos -= 2;
        }
        
        if (!(value & 0x80000000U))
        {
            value <<= 1;
            pos -= 1;
        }
    }
    
    return pos;
}

