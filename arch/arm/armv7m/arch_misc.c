#include <oneos_config.h>
#include <os_types.h>

#if defined(__CC_ARM)
__asm os_int32_t os_ffs(os_uint32_t value)
{
    CMP     R0, #0x00
    BEQ     exit

    RBIT    R0, R0
    CLZ     R0, R0
    ADDS    R0, R0, #0x01

exit
    BX      LR
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

