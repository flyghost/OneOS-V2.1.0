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
 * @file        startup_gcc.S
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "../common/mips.h"

#define IRQ_STACK_SIZE		0x2000
#define EXC_STACK_SIZE		0x2000

.equ OS_TASK_STATE_RUNNING,        0x0004               /* Task Status Flag (RUNNING) */
.equ OS_TASK_STATE_RUNNING_CLR,    -5                   /* Task Status Flag (RUNNING) Clear */
.equ OS_TASK_STATE_OFFSET,         12                   /* Byte offset of task status in os_task_t */

.global g_os_current_task
.global g_os_next_task
.global os_task_switch_notify
.global irq_stack_top


    .section ".bss"
ALIGN(4)
    irq_stack_low:
    .space IRQ_STACK_SIZE
    irq_stack_top:
    .space 8

ALIGN(4)
    exc_stack_low:
    .space EXC_STACK_SIZE
    exc_stack_top:
    .space 8

#define SYSTEM_STACK		0x80003fe8

    .global	entry
    .global 	mips_vfp32_init

    .global	_start
    .section ".start", "ax"
    .set noreorder
    _start:
    .set    noreorder
    la      ra, _start

    li      t1, 0x00800000
    mtc0    t1, CP0_CAUSE

    /* init cp0 registers. */
    li      t0, 0x1000FC00 /* BEV = 0 and mask all interrupt */
    mtc0    t0, CP0_STATUS

#ifdef __mips_hard_float
    jal 	mips_vfp32_init
    nop
#endif

    /* setup stack pointer */
    li      sp, SYSTEM_STACK
    la      gp, _gp

    _cache_init:
    /* init caches, assumes a 4way * 128set * 32byte I/D cache */
    mtc0    zero, CP0_TAGLO  /* TAGLO reg */
    mtc0    zero, CP0_TAGHI  /* TAGHI reg */
    li      t0, 3            /* enable cache for kseg0 accesses */
    mtc0    t0, CP0_CONFIG   /* CONFIG reg */
    la      t0, 0x80000000   /* an idx op should use an unmappable address */
    ori     t1, t0, 0x4000   /* 16kB cache */

    _cache_loop:
    cache   0x8, 0(t0)       /* index store icache tag */
cache   0x9, 0(t0)       /* index store dcache tag */
    bne     t0, t1, _cache_loop
    addiu   t0, t0, 0x20     /* 32 bytes per cache line */
    nop

    /* invalidate BTB */
    mfc0    t0, CP0_CONFIG
    nop
    ori     t0, 2
    mtc0    t0, CP0_CONFIG
    nop


    /* jump to RT-Thread RTOS */
    jal     entry
    nop

    /* restart, never die */
    j       _start
    nop
    .set    reorder


    .section .vectors.1, "ax", %progbits
    j       mips_tlb_refill_entry
    nop

    /* 0x100 - Cache error handler */
    .section .vectors.2, "ax", %progbits
    j       mips_cache_error_entry
    nop

    /* 0x180 - Exception/Interrupt handler */
    .section .vectors.3, "ax", %progbits
    j       mips_exception_entry
    nop

    /* 0x200 - Special Exception Interrupt handler (when IV is set in CP0_CAUSE) */
    .section .vectors.4, "ax", %progbits
    j       mips_interrupt_entry
    nop
    .section .vectors, "ax", %progbits

    .global	mips_exception_handler
    //	.global	mips_syscall
LEAF(mips_exception_entry)
    .set    push
    .set    noat
    .set    noreorder
    .set    volatile

    mfc0	k0, C0_CAUSE
    andi	k0, k0, 0x7c
    beq		zero, k0, except_do_intr
    nop

andi	k0,(0x08 << 2)
    beq		zero,k0,except_do
    nop
    except_do_intr:
    la		k0,mips_interrupt_entry
    jr		k0
    nop
    except_do_syscall:
    //	la		k0,mips_syscall
    //	jr		k0
    nop
    except_do:
    //save sp
    move	k0,sp
    //la		sp,	exc_stack_top
    subu 	sp, sp, CONTEXT_SIZE
    //save context
    sw	$0, (4*0)(sp);
    sw	$1, (4*1)(sp);
    sw	$2, (4*2)(sp);
    sw	$3, (4*3)(sp);
    sw	$4, (4*4)(sp);
    sw	$5, (4*5)(sp);
    sw	$6, (4*6)(sp);
    sw	$7, (4*7)(sp);
    sw	$8, (4*8)(sp);
    sw	$9, (4*9)(sp);
    sw	$10, (4*10)(sp);
    sw	$11, (4*11)(sp);
    sw	$12, (4*12)(sp);
    sw	$13, (4*13)(sp);
    sw	$14, (4*14)(sp);
    sw	$15, (4*15)(sp);
    sw	$16, (4*16)(sp);
    sw	$17, (4*17)(sp);
    sw	$18, (4*18)(sp);
    sw	$19, (4*19)(sp);
    sw	$20, (4*20)(sp);
    sw	$21, (4*21)(sp);
    sw	$22, (4*22)(sp);
    sw	$23, (4*23)(sp);
    sw	$24, (4*24)(sp);
    sw	$25, (4*25)(sp);
    sw	$26, (4*26)(sp);
    sw	$27, (4*27)(sp);
    sw	$28, (4*28)(sp);
    sw	k0,  (4*29)(sp);	//old sp
    sw	$30, (4*30)(sp);
    sw	$31, (4*31)(sp);

    /* STATUS CAUSE EPC.... */
    mfc0	$2, CP0_STATUS
sw		$2, STK_OFFSET_SR(sp)

    mfc0	$2, CP0_CAUSE
sw		$2, STK_OFFSET_CAUSE(sp)

    mfc0	$2, CP0_BADVADDR
sw		$2, STK_OFFSET_BADVADDR(sp)

    MFC0	$2, CP0_EPC
sw		$2, STK_OFFSET_EPC(sp)

    mfhi	$2
sw		$2, STK_OFFSET_HI(sp)

    mflo	$2
sw		$2, STK_OFFSET_LO(sp)

    move	a0, sp
    la		k0,	mips_exception_handler
    j		k0
    nop

    //

    .set	pop
END(mips_exception_entry)

    .global	mips_tlb_refill_handler
LEAF(mips_tlb_refill_entry)
    .set    push
    .set    noat
    .set    noreorder
    .set    volatile

    la		k0,mips_tlb_refill_handler
    jr		k0

    nop
    eret
    nop

    .set	pop
END(mips_tlb_refill_entry)

    .global	mips_cache_error_handler
LEAF(mips_cache_error_entry)
    .set    push
    .set    noat
    .set    noreorder
    .set    volatile

    la		k0,mips_cache_error_handler
    jr		k0
    nop
    eret
    nop

    .set	pop
END(mips_cache_error_entry)



    .global os_interrupt_dispatch
    .global os_interrupt_enter
    .global os_interrupt_leave
LEAF(mips_interrupt_entry)
    .set    push
    .set    noat
    .set    noreorder
    .set    volatile

    //mfc0	k0,CP0_EPC
    SAVE_CONTEXT

    mfc0    t0, CP0_CAUSE
    mfc0    t1, CP0_STATUS
    and     t0, t1

    andi    t0, 0xff00
    beqz    t0, spurious_interrupt
    nop

    /* let k0 keep the current context sp */
    move    k0, sp

    /* switch to kernel stack */
    la      sp, irq_stack_top
    jal     os_interrupt_enter
    nop
    jal     os_interrupt_dispatch
    nop
    jal     os_interrupt_leave
    nop

    /* switch sp back to thread's context */
    move    sp, k0

    la      k0, g_os_task_switch_interrupt_flag /* k0 = &g_os_task_switch_interrupt_flag */
    lw      k1, 0(k0)
    beqz    k1, spurious_interrupt
    nop
    sw      zero, 0(k0)                         /* g_os_task_switch_interrupt_flag = 0 */
    nop

    /*
     * switch to the new thread
     */
    lw  t0 , g_os_current_task              /* t1 = g_os_current_task */
    sw  sp , 0(t0)                          /* store sp in preempted tasks TCB */

    li  t1 , OS_TASK_STATE_RUNNING_CLR      /* t1 = OS_TASK_STATE_RUNNING_CLR */
    lw  t2 , (OS_TASK_STATE_OFFSET)(t0)     /* t2 = g_os_current_task->state */
    and t2 , t2, t1                         /* t2 &= OS_TASK_STATE_RUNNING_CLR */
    sw  t2 , (OS_TASK_STATE_OFFSET)(t0)     /* g_os_current_task->state = t2 */

    /*task_switch_hook*/
    #if defined (OS_TASK_SWITCH_NOTIFY)
    subu    sp, sp, 4                       
    sw      ra, (0)(sp)                     /* push ra*/

    jal os_task_switch_notify
    nop

    lw      ra, (0)(sp)                     /* pop ra*/
    addu    sp, sp, 4
    #endif
    
    lw  t0 , g_os_next_task                 /* t0 = g_os_next_task */
    la  t1 , g_os_current_task              /* t1 = &g_os_current_task */
    sw  t0 , 0(t1)                          /* g_os_current_task = g_os_next_task */

    lw  t2 , 0(t1)                          /* t2 = g_os_current_task */
    lw  t3 , (OS_TASK_STATE_OFFSET)(t2)     /* t3 = g_os_current_task->state */
    ori t3 , t3, OS_TASK_STATE_RUNNING      /* t3 |= (OS_TASK_STATE_RUNNING) */
    sw  t3 , (OS_TASK_STATE_OFFSET)(t2)     /* g_os_current_task->state = t3 */

    
    lw  sp , 0(t2)                          /* get new task stack pointer */

                     
    j       spurious_interrupt
    nop
    spurious_interrupt:
    RESTORE_CONTEXT

    .set	pop
END(mips_interrupt_entry)
