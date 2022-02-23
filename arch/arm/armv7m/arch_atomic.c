
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include "arch_atomic.h"

#if defined(__CC_ARM)

__asm void os_atomic_add(os_atomic_t *mem, os_int32_t value)
{
Loop_add
    LDREX R2, [R0]
    ADD   R2, R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_add_exit
    B     Loop_add  
Loop_add_exit    
    BX    LR
}

__asm os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value)
{
Loop_add_ret
    LDREX R2, [R0]
    ADD   R2, R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_add_ret_exit
    B     Loop_add_ret
Loop_add_ret_exit
    MOV   R0, R2
    BX    LR
}

__asm void os_atomic_sub(os_atomic_t *mem, os_int32_t value)
{
Loop_sub
    LDREX R2, [R0]
    SUB   R2, R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_sub_exit
    B     Loop_sub
Loop_sub_exit
    BX    LR
}

__asm os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value)
{
Loop_sub_ret
    LDREX R2, [R0]
    SUB   R2, R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_sub_ret_exit
    B     Loop_sub_ret
Loop_sub_ret_exit
    MOV   R0, R2
    BX    LR
}

__asm void os_atomic_inc(os_atomic_t *mem)
{
Loop_inc
    LDREX R2, [R0]
    ADD   R2, R2, #1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_inc_exit
    B     Loop_inc
Loop_inc_exit
    BX    LR
}

__asm os_int32_t os_atomic_inc_return(os_atomic_t *mem)
{
Loop_inc_ret
    LDREX R2, [R0]
    ADD   R2, R2, #1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_inc_ret_exit
    B     Loop_inc_ret
Loop_inc_ret_exit
    MOV   R0, R2
    BX    LR
}

__asm void os_atomic_dec(os_atomic_t *mem)
{
Loop_dec
    LDREX R2, [R0]
    SUB   R2, R2, #1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_dec_exit
    B     Loop_dec
Loop_dec_exit
    BX    LR
}

__asm os_int32_t os_atomic_dec_return(os_atomic_t *mem)
{
Loop_dec_ret
    LDREX R2, [R0]
    SUB   R2, R2, #1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_dec_ret_exit
    B     Loop_dec_ret
Loop_dec_ret_exit
    MOV   R0, R2
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
Loop_xchg
    LDREX R2, [R0]    
    STREX R3, R1, [R0]
    CBZ   R3, Loop_xchg_exit
    B     Loop_xchg
Loop_xchg_exit
    MOV   R0, R2
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
    PUSH  {R4}
Loop_cmpxchg
    LDREX R3, [R0]
    MOV   R4, #0
    TEQ   R3, R1
    BNE   Loop_cmpxchg_exit
    STREX R3, R2, [R0]
    MOV   R4, #1
    CBZ   R3, Loop_cmpxchg_exit
    B     Loop_cmpxchg
Loop_cmpxchg_exit
    MOV   R0, R4
    POP   {R4}
    BX    LR
}

__asm void os_atomic_and(os_atomic_t* mem, os_int32_t value)
{
Loop_and
    LDREX R2, [R0]
    AND   R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_and_exit
    B     Loop_and
Loop_and_exit
    BX    LR
}

__asm void os_atomic_or(os_atomic_t* mem, os_int32_t value)
{
Loop_or
    LDREX R2, [R0]
    ORR   R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_or_exit
    B     Loop_or
Loop_or_exit
    BX    LR
}

__asm void os_atomic_nand(os_atomic_t* mem, os_int32_t value)
{
Loop_nand
    LDREX R2, [R0]
    AND   R2, R1
    MVN   R2, R2
    STREX R3, R2, [R0]
    CBZ   R3, Loop_nand_exit
    B     Loop_nand
Loop_nand_exit
    BX    LR
}

__asm void os_atomic_xor(os_atomic_t* mem, os_int32_t value)
{
Loop_xor
    LDREX R2, [R0]
    EOR   R2, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_xor_exit
    B     Loop_xor
Loop_xor_exit
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
    PUSH  {R4, R5}
    MOV   R4, #1
    LSL   R4, R1
Loop_test
    LDREX R2, [R0]
    AND   R5, R2, R4
    LSR   R5, R1
    STREX R3, R2, [R0]
    CBZ   R3, Loop_test_exit
    B     Loop_test
Loop_test_exit
    MOV   R0, R5
    POP   {R4, R5}
    BX    LR
}

__asm void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MOV   R4, #1
    LSL   R4, R1
Loop_set
    LDREX R2, [R0]
    ORR   R2, R4
    STREX R3, R2, [R0]
    CBZ   R3, Loop_set_exit
    B     Loop_set
Loop_set_exit
    POP   {R4}
    BX    LR
}

__asm void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MOV   R4, #1
    LSL   R4, R1
Loop_clear
    LDREX R2, [R0]
    BIC   R2, R4
    STREX R3, R2, [R0]
    CBZ   R3, Loop_clear_exit
    B     Loop_clear
Loop_clear_exit
    POP   {R4}
    BX    LR
}

__asm void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr)
{
    PUSH  {R4}
    MOV   R4, #1
    LSL   R4, R1
Loop_change
    LDREX R2, [R0]
    EOR   R2, R4
    STREX R3, R2, [R0]
    CBZ   R3, Loop_change_exit
    B     Loop_change
Loop_change_exit
    POP   {R4}
    BX    LR
}

#elif defined(__GNUC__) || defined(__CLANG_ARM)

void os_atomic_add(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_add:        \n"
        "   LDREX %0, [%2]  \n"
        "   ADD   %0, %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_add\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_add_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_add_ret:        \n"
        "   LDREX %0, [%2]  \n"
        "   ADD   %0, %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_add_ret\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_sub(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_sub:        \n"
        "   LDREX %0, [%2]  \n"
        "   SUB   %0, %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_sub\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return;
}

os_int32_t os_atomic_sub_return(os_atomic_t *mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_sub_ret:        \n"
        "   LDREX %0, [%2]  \n"
        "   SUB   %0, %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_sub_ret\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc","memory");

        return oldval;
}

void os_atomic_inc(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_inc:        \n"
        "   LDREX %0, [%2]  \n"
        "   ADD   %0, %0, #1\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_inc\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_inc_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_inc_ret:        \n"
        "   LDREX %0, [%2]  \n"
        "   ADD   %0, %0, #1\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_inc_ret\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return oldval;
}

void os_atomic_dec(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_dec:        \n"
        "   LDREX %0, [%2]  \n"
        "   SUB   %0, %0, #1\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_dec\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter)
        :   "cc", "memory");

        return;
}

os_int32_t os_atomic_dec_return(os_atomic_t *mem)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_dec_ret:        \n"
        "   LDREX %0, [%2]  \n"
        "   SUB   %0, %0, #1\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_dec_ret\n"
        :   "+r"(oldval),"+r"(exflag)
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
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_xchg:       \n"
        "   LDREX %0, [%2]  \n"
        "   STREX %1, %3, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_xchg\n"
        :   "+r"(oldval),"+r"(exflag)
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
    
    __asm__ __volatile__(
        "Loop_cmpxchg:    \n"
        "   LDREX %0, [%2]  \n"
        "   MOV   %1, #0    \n"
        "   TEQ   %0, %3    \n"
        "   BNE   Loop_cmpxchg_exit \n"
        "   STREX %0, %4, [%2] \n"
        "   MOV   %1, #1 \n"
        "   TEQ	  %0, #0 \n"
        "   BNE   Loop_cmpxchg \n"
        "Loop_cmpxchg_exit: \n"
        :   "+r"(tmpval),"=&r"(res)
        :   "r"(&mem->counter),"r"(old),"r"(new)
        :   "cc", "memory");

        return res;
}

void os_atomic_and(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_and:        \n"
        "   LDREX %0, [%2]  \n"
        "   AND   %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_and\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_or(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_or:         \n"
        "   LDREX %0, [%2]  \n"
        "   ORR   %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_or\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_nand(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_nand:       \n"
        "   LDREX %0, [%2]  \n"
        "   AND   %0, %3\n"
        "   MVN   %0, %0\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_nand\n"
        :   "+r"(oldval),"+r"(exflag)
        :   "r"(&mem->counter),"r"(value)
        :   "cc", "memory");

        return;
}

void os_atomic_xor(os_atomic_t* mem, os_int32_t value)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    
    __asm__ __volatile__(
        "Loop_xor:        \n"
        "   LDREX %0, [%2]  \n"
        "   EOR   %0, %3\n"
        "   STREX %1, %0, [%2]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_xor\n"
        :   "+r"(oldval),"+r"(exflag)
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
    __volatile__ os_int32_t exflag;
    __volatile__ os_int32_t tmpval;
    __volatile__ os_int32_t res;
    
    __asm__ __volatile__(
        "   MOV   %2, #1    \n"
        "   LSL   %2, %5    \n"
        "Loop_test:       \n"
        "   LDREX %0, [%4]  \n"
        "   AND   %3, %0, %2\n"
        "   LSR	  %3, %5\n"
        "   STREX %1, %0, [%4]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_test\n"        
        :   "+r"(oldval),"+r"(exflag),"+r"(tmpval),"=&r"(res)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return res;
}

void os_atomic_set_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    __volatile__ os_int32_t tmpval;
    
    __asm__ __volatile__(
        "   MOV   %2, #1    \n"
        "   LSL   %2, %4    \n"
        "Loop_set:        \n"
        "   LDREX %0, [%3]  \n"
        "   ORR   %0, %2\n"
        "   STREX %1, %0, [%3]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_set\n"        
        :   "+r"(oldval),"+r"(exflag),"+r"(tmpval)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_clear_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    __volatile__ os_int32_t tmpval;
    
    __asm__ __volatile__(
        "   MOV   %2, #1    \n"
        "   LSL   %2, %4    \n"
        "Loop_clear:      \n"
        "   LDREX %0, [%3]  \n"
        "   BIC   %0, %2\n"
        "   STREX %1, %0, [%3]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_clear\n"        
        :   "+r"(oldval),"+r"(exflag),"+r"(tmpval)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

void os_atomic_change_bit(os_atomic_t* mem, os_int32_t nr)
{
    __volatile__ os_int32_t oldval;
    __volatile__ os_int32_t exflag;
    __volatile__ os_int32_t tmpval;
    
    __asm__ __volatile__(
        "   MOV   %2, #1    \n"
        "   LSL   %2, %4    \n"
        "Loop_change:     \n"
        "   LDREX %0, [%3]  \n"
        "   EOR   %0, %2\n"
        "   STREX %1, %0, [%3]\n"
        "   TEQ	  %1, #0\n"
        "   BNE   Loop_change\n"        
        :   "+r"(oldval),"+r"(exflag),"+r"(tmpval)
        :   "r"(&mem->counter),"r"(nr)
        :   "cc", "memory");

        return;
}

#endif  /* defined(__CC_ARM) || defined(__GNUC__) || defined(__CLANG_ARM) */

