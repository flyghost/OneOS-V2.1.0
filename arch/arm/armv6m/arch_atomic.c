
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include "arch_atomic.h"

#if defined(__CC_ARM)

__asm void os_atomic_add(os_atomic_t *mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ADDS  R2, R2, R1
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

__asm os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ADDS  R2, R2, R1
    STR   R2, [R0]
    MOV   R0, R2
    MSR   PRIMASK, R3
    BX    LR
}

__asm void os_atomic_sub(os_atomic_t *mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    SUBS  R2, R2, R1
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

__asm os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    SUBS  R2, R2, R1
    STR   R2, [R0]
    MOV   R0, R2
    MSR   PRIMASK, R3
    BX    LR
}

__asm void os_atomic_inc(os_atomic_t *mem)
{
    MRS   R1, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ADDS  R2, R2, #1
    STR   R2, [R0]
    MSR   PRIMASK, R1
    BX    LR
}

__asm os_int32_t os_atomic_inc_return(os_atomic_t *mem)
{
    MRS   R1, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ADDS  R2, R2, #1
    STR   R2, [R0]
    MOV   R0, R2
    MSR   PRIMASK, R1
    BX    LR
}

__asm void os_atomic_dec(os_atomic_t *mem)
{
    MRS   R1, PRIMASK
    CPSID I
    LDR   R2, [R0]
    SUBS  R2, R2, #1
    STR   R2, [R0]
    MSR   PRIMASK, R1
    BX    LR
}

__asm os_int32_t os_atomic_dec_return(os_atomic_t *mem)
{
    MRS   R1, PRIMASK
    CPSID I
    LDR   R2, [R0]
    SUBS  R2, R2, #1
    STR   R2, [R0]
    MOV   R0, R2
    MSR   PRIMASK, R1
    BX    LR
}

/**
 ***********************************************************************************************************************
 * @brief           Storage the input "value" on the address "mem", and return old value in atomic operation.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       value           The value need storage on the address "mem".
 *
 * @return          The old value on the address "mem".
 * @retval          os_int32_t
 ***********************************************************************************************************************
 */
__asm os_int32_t os_atomic_xchg(os_atomic_t* mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]    
    STR   R1, [R0]
    MOV   R0, R2
    MSR   PRIMASK, R3
    BX    LR
}

/**
 ***********************************************************************************************************************
 * @brief           If old value on the address "mem" equal to the input "old", storage the input "new" on the address
 *                  "mem" in atomic operation, return result of storage operation.
 *
 * @detail          If old value isn`t equal to the "old", return 0 directly.
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       old             The value compare with the old one on the address "mem".
 * @param[in]       new             The value storage on the address "mem" as a new.
 *
 * @return          Storage success, return 1; storage failure, return 0.
 ***********************************************************************************************************************
 */
__asm os_bool_t os_atomic_cmpxchg(os_atomic_t* mem, os_int32_t old, os_int32_t new)
{
    PUSH  {R4, R5}
    MRS   R5, PRIMASK
    CPSID I
    LDR   R3, [R0]
    MOVS  R4, #0
    CMP   R3, R1
    BNE   cmpxchg_exit
    STR   R2, [R0]
    MOVS  R4, #1
cmpxchg_exit
    MOV   R0, R4    
    MSR   PRIMASK, R5
    POP   {R4, R5}
    BX    LR
}

__asm void os_atomic_and(os_atomic_t* mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ANDS  R2, R1
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

__asm void os_atomic_or(os_atomic_t* mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ORRS  R2, R1
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

__asm void os_atomic_nand(os_atomic_t* mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    ANDS  R2, R1
    MVNS  R2, R2
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

__asm void os_atomic_xor(os_atomic_t* mem, os_int32_t value)
{
    MRS   R3, PRIMASK
    CPSID I
    LDR   R2, [R0]
    EORS  R2, R1
    STR   R2, [R0]
    MSR   PRIMASK, R3
    BX    LR
}

/**
 ***********************************************************************************************************************
 * @brief           Test the bit of value on the address "mem" which indicate by the input "nr" in atomic operation.
 *                  Return the test result.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       nr              The number of bit in value which should be test.
 *
 * @return          The test result of bit.
 * @retval          0               The tested bit equal to 0;
 * @retval          1               The tested bit equal to 1;
 ***********************************************************************************************************************
 */
__asm os_bool_t os_atomic_test_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MRS   R4, PRIMASK
    CPSID I
    MOVS  R3, #1
    LSLS  R3, R1
    LDR   R2, [R0]
    ANDS  R2, R3
    LSRS  R2, R1
    MOV   R0, R2
    MSR   PRIMASK, R4
    POP   {R4}
    BX    LR
}

__asm void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MRS   R4, PRIMASK
    CPSID I
    MOVS  R3, #1
    LSLS  R3, R1
    LDR   R2, [R0]
    ORRS  R2, R3
    STR   R2, [R0]
    MSR   PRIMASK, R4
    POP   {R4}
    BX    LR
}

__asm void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MRS   R4, PRIMASK
    CPSID I
    MOVS  R3, #1
    LSLS  R3, R1
    LDR   R2, [R0]
    BICS  R2, R3
    STR   R2, [R0]
    MSR   PRIMASK, R4
    POP   {R4}
    BX    LR
}

__asm void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MRS   R4, PRIMASK
    CPSID I
    MOVS  R3, #1
    LSLS  R3, R1
    LDR   R2, [R0]
    EORS  R2, R3
    STR   R2, [R0]
    MSR   PRIMASK, R4
    POP   {R4}
    BX    LR
}

#elif defined(__CLANG_ARM)

void os_atomic_add(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADDS  %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADDS  %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_sub(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUBS  %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return;
}

os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUBS  %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return oldval;
}

void os_atomic_inc(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADDS  %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_inc_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADDS  %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_dec(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUBS  %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_dec_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUBS  %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return oldval;
}

/**
 ***********************************************************************************************************************
 * @brief           Storage the input "value" on the address "mem", and return old value in atomic operation.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       value           The value need storage on the address "mem".
 *
 * @return          The old value on the address "mem".
 * @retval          os_int32_t
 ***********************************************************************************************************************
 */
os_int32_t os_atomic_xchg(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   STR   %3, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return oldval;
}

/**
 ***********************************************************************************************************************
 * @brief           If old value on the address "mem" equal to the input "old", storage the input "new" on the address
 *                  "mem" in atomic operation, return result of storage operation.
 *
 * @detail          If old value isn`t equal to the "old", return 0 directly.
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       old             The value compare with the old one on the address "mem".
 * @param[in]       new             The value storage on the address "mem" as a new.
 *
 * @return          Storage success, return 1; storage failure, return 0.
 ***********************************************************************************************************************
 */
os_bool_t os_atomic_cmpxchg(os_atomic_t* mem, os_int32_t old, os_int32_t new)
{
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t res;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%3]  \n"
        "   MOVS  %1, #0    \n"
        "   CMP   %0, %4    \n"
        "   BNE   cmpxchg_exit\n"
        "   STR   %5, [%3]  \n"
        "   MOVS  %1, #1    \n"
        "cmpxchg_exit:    \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(tmpval),"=&r"(res),"+r"(primask)
        :   "r"(&mem->counter),"r"(old),"r"(new)
        :   "cc", "memory");

        return res;
}

void os_atomic_and(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ANDS  %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_or(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ORRS  %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_nand(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ANDS  %0, %3    \n"
        "   MVNS  %0, %0    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_xor(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   EORS  %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

/**
 ***********************************************************************************************************************
 * @brief           Test the bit of value on the address "mem" which indicate by the input "nr" in atomic operation.
 *                  Return the test result.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       nr              The number of bit in value which should be test.
 *
 * @return          The test result of bit.
 * @retval          0               The tested bit equal to 0;
 * @retval          1               The tested bit equal to 1;
 ***********************************************************************************************************************
 */
os_bool_t os_atomic_test_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t res;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %3, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSLS  %1, %5    \n"
        "   LDR   %0, [%4]  \n"
        "   MOVS  %2, %0    \n"
        "   ANDS  %2, %1    \n"
        "   LSRS  %2, %5    \n"
        "   MSR   PRIMASK, %3\n"
        :   "+r"(oldval),"+r"(tmpval),"=&r"(res),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return res;
}

void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSLS  %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   ORRS  %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSLS  %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   BICS  %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSLS  %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   EORS  %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

#elif defined(__GNUC__)

void os_atomic_add(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADD   %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADD   %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_sub(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUB   %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return;
}

os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUB   %0, %0, %3\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return oldval;
}

void os_atomic_inc(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADD   %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_inc_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ADD   %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_dec(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUB   %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_dec_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   SUB   %0, %0, #1\n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return oldval;
}

/**
 ***********************************************************************************************************************
 * @brief           Storage the input "value" on the address "mem", and return old value in atomic operation.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       value           The value need storage on the address "mem".
 *
 * @return          The old value on the address "mem".
 * @retval          os_int32_t
 ***********************************************************************************************************************
 */
os_int32_t os_atomic_xchg(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   STR   %3, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return oldval;
}

/**
 ***********************************************************************************************************************
 * @brief           If old value on the address "mem" equal to the input "old", storage the input "new" on the address
 *                  "mem" in atomic operation, return result of storage operation.
 *
 * @detail          If old value isn`t equal to the "old", return 0 directly.
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       old             The value compare with the old one on the address "mem".
 * @param[in]       new             The value storage on the address "mem" as a new.
 *
 * @return          Storage success, return 1; storage failure, return 0.
 ***********************************************************************************************************************
 */
os_bool_t os_atomic_cmpxchg(os_atomic_t* mem, os_int32_t old, os_int32_t new)
{
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t res;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%3]  \n"
        "   MOVS  %1, #0    \n"
        "   CMP   %0, %4    \n"
        "   BNE   cmpxchg_exit\n"
        "   STR   %5, [%3]  \n"
        "   MOVS  %1, #1    \n"
        "cmpxchg_exit:    \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(tmpval),"=&r"(res),"+r"(primask)
        :   "r"(&mem->counter),"r"(old),"r"(new)
        :   "cc", "memory");

        return res;
}

void os_atomic_and(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   AND   %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_or(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   ORR   %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_nand(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   AND   %0, %3    \n"
        "   MVN   %0, %0    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_xor(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %1, PRIMASK\n"
        "   CPSID I         \n"
        "   LDR   %0, [%2]  \n"
        "   EOR   %0, %3    \n"
        "   STR   %0, [%2]  \n"
        "   MSR   PRIMASK, %1\n"
        :   "+r"(oldval),"+r"(primask)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

/**
 ***********************************************************************************************************************
 * @brief           Test the bit of value on the address "mem" which indicate by the input "nr" in atomic operation.
 *                  Return the test result.
 *
 * @detail          
 *
 * @param[in/out]   mem             The Pointer cast to variate is defined by the "os_atomic_t".
 * @param[in]       nr              The number of bit in value which should be test.
 *
 * @return          The test result of bit.
 * @retval          0               The tested bit equal to 0;
 * @retval          1               The tested bit equal to 1;
 ***********************************************************************************************************************
 */
os_bool_t os_atomic_test_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t res;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %3, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSL   %1, %5    \n"
        "   LDR   %0, [%4]  \n"
        "   MOVS  %2, %0    \n"
        "   AND   %2, %1    \n"
        "   LSR   %2, %5    \n"
        "   MSR   PRIMASK, %3\n"
        :   "+r"(oldval),"+r"(tmpval),"=&r"(res),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return res;
}

void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSL   %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   ORR   %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSL   %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   BIC   %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t primask;
    
    __asm__ __volatile__(
        "   MRS   %2, PRIMASK\n"
        "   CPSID I         \n"
        "   MOVS  %1, #1    \n"
        "   LSL   %1, %4    \n"
        "   LDR   %0, [%3]  \n"
        "   EOR   %0, %1    \n"
        "   STR   %0, [%3]  \n"
        "   MSR   PRIMASK, %2\n"
        :   "+r"(oldval),"+r"(tmpval),"+r"(primask)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

#endif  /* defined(__CC_ARM) || defined(__GNUC__) || defined(__CLANG_ARM) */

