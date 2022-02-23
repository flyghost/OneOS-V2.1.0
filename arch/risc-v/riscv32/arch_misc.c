#include <os_types.h>

/**
 ***********************************************************************************************************************
 * @brief           Find the first bit set in value.
 *
 * @param[in]       value
 *
 * @return          Returns lowest bit equal to 1 in value, or returns zero if value is zero.
 ***********************************************************************************************************************
 */
os_int32_t os_ffs(os_uint32_t value)
{
    return __builtin_ffs(value);
}

/**
 ***********************************************************************************************************************
 * @brief           Find the last bit set in value.
 *
 * @param[in]       value
 *
 * @return          Returns highest bit equal to 1 in value, or returns zero if value is zero.
 ***********************************************************************************************************************
 */
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

/**
 ***********************************************************************************************************************
 * @brief           Get the sp of the current task.
 *
 * @param           None
 *
 * @return          sp pointer
 ***********************************************************************************************************************
 */
void *os_get_current_task_sp(void) 
{
    register void *result;

    __asm__ __volatile__(
        "   addi %0, sp, 0\n"
        :   "=r"(result));

    return result;
}

