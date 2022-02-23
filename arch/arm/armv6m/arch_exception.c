#include <os_types.h>
#include <os_task.h>
#include <os_util.h>
#include <os_errno.h>
#include <os_assert.h>
#include <arch_misc.h>
#include <arch_task.h>
#include <arch_exception.h>
#include <os_safety.h>

/* exception hook */
os_err_t (*os_exception_hook)(void *context, os_size_t *msp, os_size_t *psp) = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           This function will set a hook function, the hook will be called in os_hw_hard_fault_exception function
 *
 * @param[in]       hook             The hook function complemented by user.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hw_exception_install(os_err_t (*exception_handle)(void *context, os_size_t *msp, os_size_t *psp))
{
    os_exception_hook = exception_handle;
}

static void _os_arch_usage_fault_track(void)
{
    os_kprintf("usage fault:\r\n");
    os_kprintf("SCB_CFSR_UFSR: 0x%02X\r\n", SCB_CFSR_UFSR);

    if (SCB_CFSR_UFSR & (1 << 0))
    {
        os_kprintf("UNDEFINSTR\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 1))
    {
        os_kprintf("INVSTATE\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 2))
    {
        os_kprintf("INVPC\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 3))
    {
        os_kprintf("NOCP\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 8))
    {
        os_kprintf("UNALIGNED\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 9))
    {
        os_kprintf("DIVBYZERO\r\n");
    }
}

static void _os_arch_bus_fault_track(void)
{
    os_kprintf("bus fault:\r\n");
    os_kprintf("SCB_CFSR_BFSR: 0x%02X\r\n", SCB_CFSR_BFSR);

    if (SCB_CFSR_BFSR & (1 << 0))
    {
        os_kprintf("IBUSERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 1))
    {
        os_kprintf("PRECISERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 2))
    {
        os_kprintf("IMPRECISERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 3))
    {
        os_kprintf("UNSTKERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 4))
    {
        os_kprintf("STKERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 7))
    {
        os_kprintf("SCB->BFAR:%08X\r\n", SCB_BFAR);
    }

    return;
}

static void _os_arch_mem_manage_fault_track(void)
{
    os_kprintf("mem manage fault:\r\n");
    os_kprintf("SCB_CFSR_MFSR: 0x%02X\r\n", SCB_CFSR_MFSR);

    if (SCB_CFSR_MFSR & (1 << 0))
    {
        os_kprintf("IACCVIOL\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 1))
    {
        os_kprintf("DACCVIOL\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 3))
    {
        os_kprintf("MUNSTKERR\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 4))
    {
        os_kprintf("MSTKERR\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 7))
    {
        os_kprintf("SCB->MMAR: %08X\r\n", SCB_MMAR);
    }

    return;
}

static void _os_arch_hard_fault_track(void)
{
    if (SCB_HFSR & (1UL << 1))
    {
        os_kprintf("Failed vector fetch\r\n");
    }

    if (SCB_HFSR & (1UL << 30))
    {
        if (SCB_CFSR_BFSR)
        {
            _os_arch_bus_fault_track();
        }

        if (SCB_CFSR_MFSR)
        { 
            _os_arch_mem_manage_fault_track();
        }

        if (SCB_CFSR_UFSR)
        {
            _os_arch_usage_fault_track();
        }
    }

    if(SCB_HFSR & (1UL << 31))
    {
        os_kprintf("Debug event\r\n");
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       frame           The start address of the stack frame when the exception occurs.
 * @param[in]       msp             Interrupt stack pointer.
 * @param[in]       psp             Currently running task stack pointer.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_arch_fault_exception(void *frame, os_size_t *msp, os_size_t *psp)
{
    if (OS_NULL != os_exception_hook)
    {
        os_err_t result;

        result = os_exception_hook(frame, msp, psp);
        if (OS_EOK == result)
        {
            return;
        }
    }

#ifdef  STACK_TRACE_EN
        _arch_exception_stack_show(frame, msp, psp);

        while(1);
#else

    struct stack_frame      *stack_frame_pointer;

    stack_frame_pointer = (struct stack_frame *)frame;

    os_kprintf("R0 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r0);
    os_kprintf("R1 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r1);
    os_kprintf("R2 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r2);
    os_kprintf("R3 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r3);
    os_kprintf("R4 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r4);
    os_kprintf("R5 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r5);
    os_kprintf("R6 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r6);
    os_kprintf("R7 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r7);
    os_kprintf("R8 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r8);
    os_kprintf("R9 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r9);
    os_kprintf("R10: 0x%08x\r\n", stack_frame_pointer->manual_frame.r10);
    os_kprintf("R11: 0x%08x\r\n", stack_frame_pointer->manual_frame.r11);
    os_kprintf("R12: 0x%08x\r\n", stack_frame_pointer->cpu_frame.r12);
    os_kprintf("LR : 0x%08x\r\n", stack_frame_pointer->cpu_frame.lr);
    os_kprintf("PC : 0x%08x\r\n", stack_frame_pointer->cpu_frame.pc);
    os_kprintf("PSR: 0x%08x\r\n", stack_frame_pointer->cpu_frame.psr);

    /* Hard fault is generated in task context. */
    if (stack_frame_pointer->manual_frame.exc_return & EXCEPTION_RETURN_MODE_MASK)
    {
        os_kprintf("hard fault in task: %s\r\n", os_task_self()->name);
    }
    /* Hard fault is generated in interrupt context. */
    else
    {
        os_kprintf("hard fault in interrupt\r\n");
    }
    
    _os_arch_hard_fault_track();
    
    os_safety_exception_process();
#endif  /* STACK_TRACE_EN */

}

#ifdef  STACK_TRACE_EN

#include <stack_trace.h>
/**
 ***********************************************************************************************************************
 * @brief           This function displays a list of the nested routine calls that the specified task. 
 *
 * @attention       This function is used in task context and interrupt context, and cannot be used in exception 
 *                  context. The processing method of the currently running task and other tasks is different.
 *
 * @param[in]       name            Pointer to task name string
 * @param[in]       context         1:the exception context 0:other context
 *
 * @return          On success, return OS_EOK; on error, OS_ERROR is returned.
 * @retval          OS_EOK           Success.
 * @retval          OS_ERROR         There is no task with this name in the system.
 ***********************************************************************************************************************
 */
os_err_t task_stack_show(char *name, os_uint32_t context)
{
    /* The place where the function call backtrace starts after removing the registers in the stack. */
    os_size_t          *sp_func_call;
    
    os_size_t          *sp_top;
    os_size_t          *sp_bottom;
    os_task_t          *task;
    os_uint32_t         i;
    call_back_trace_t   record;

    struct stack_frame          *stack_frame_pointer;
    struct cpu_stack_frame      *cpu_frame;

    if (OS_NULL == name)
    {   
        return OS_ERROR;
    }

    if (OS_TRUE == task_is_protected(name))
    {
        os_kprintf("The task %s is protected and does not allow stack backtracking.\r\n", name);
        return OS_ERROR;
    }
       
    task = os_task_find(name);

    if (OS_NULL == task)
    {
        os_kprintf("The system does not have a task with this name %s\r\n", name);
        return OS_ERROR;
    }

    os_kprintf("=================Task %s stack trace======================\r\n", name);
    
    if (task != os_task_self())
    {
        /* After the task switch occurs, task->sp records the stack pointer of the task. */
        sp_top = task->stack_top;

        sp_func_call =  sp_top + sizeof(struct stack_frame) / sizeof(os_size_t);

        stack_frame_pointer = (struct stack_frame *)sp_top;
        
        os_kprintf("R0 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r0);
        os_kprintf("R1 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r1);
        os_kprintf("R2 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r2);
        os_kprintf("R3 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r3);
        os_kprintf("R4 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r4);
        os_kprintf("R5 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r5);
        os_kprintf("R6 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r6);
        os_kprintf("R7 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r7);
        os_kprintf("R8 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r8);
        os_kprintf("R9 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r9);
        os_kprintf("R10: 0x%08x\r\n", stack_frame_pointer->manual_frame.r10);
        os_kprintf("R11: 0x%08x\r\n", stack_frame_pointer->manual_frame.r11);
        os_kprintf("R12: 0x%08x\r\n", stack_frame_pointer->cpu_frame.r12);
        os_kprintf("LR : 0x%08x\r\n", stack_frame_pointer->cpu_frame.lr);
        os_kprintf("PC : 0x%08x\r\n", stack_frame_pointer->cpu_frame.pc);
        os_kprintf("PSR: 0x%08x\r\n", stack_frame_pointer->cpu_frame.psr);

        /* Task overflow. */
        if ((os_size_t)sp_top >= ((os_size_t)task->stack_end) 
            || (sp_top < (os_size_t*)(task->stack_begin)))
        {
            sp_bottom = sp_func_call + (TASK_STACK_OVERFLOW_STACK_SIZE ) / sizeof(os_size_t);
            
            os_kprintf("The stack of task %s is overflow!\r\n", name);
        }
        else
        {
            sp_bottom = task->stack_end;
        }

        record.depth = 0;
        record.back_trace[record.depth++] = stack_frame_pointer->cpu_frame.pc;
        if ((stack_frame_pointer->cpu_frame.lr - 1) != stack_frame_pointer->cpu_frame.pc)
        {
            record.back_trace[record.depth++] = stack_frame_pointer->cpu_frame.lr - 1;
        }
    }
    else
    {
        if (1 == context)
        {
            os_kprintf("Task %s stack trace has been processed by hard fault exception\r\n", name);
            return OS_EOK;
        }
        else if(0 == context)
        {
            /* The running task in interrupt context. */
            if (os_is_irq_active() > 0)
            {
                /* The running task always uses psp, task->sp is not used. */
                sp_top = (os_size_t*)os_get_current_task_sp();
                cpu_frame = (struct cpu_stack_frame *)sp_top;
                os_kprintf("SP : 0x%08x\r\n", sp_top);

                os_kprintf("R0 : 0x%08x\r\n", cpu_frame->r0);
                os_kprintf("R1 : 0x%08x\r\n", cpu_frame->r1);
                os_kprintf("R2 : 0x%08x\r\n", cpu_frame->r2);
                os_kprintf("R3 : 0x%08x\r\n", cpu_frame->r3);
                os_kprintf("R12: 0x%08x\r\n", cpu_frame->r12);
                os_kprintf("LR : 0x%08x\r\n", cpu_frame->lr);
                os_kprintf("PC : 0x%08x\r\n", cpu_frame->pc);
                os_kprintf("PSR: 0x%08x\r\n", cpu_frame->psr);
                
                record.depth = 0;
                record.back_trace[record.depth++] = cpu_frame->pc;
                if ((cpu_frame->lr - 1) != cpu_frame->pc)
                {
                    record.back_trace[record.depth++] = cpu_frame->lr - 1;
                }

                sp_func_call = sp_top + sizeof(struct cpu_stack_frame) / sizeof(os_size_t);

                /* Task overflow. */
                if ((os_size_t)sp_top >= ((os_size_t)task->stack_end) 
                    || (sp_top < (os_size_t*)(task->stack_begin)))
                {
                   
                    sp_bottom = sp_top 
                                  + (sizeof(struct cpu_stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE )
                                  / sizeof(os_size_t);
                    
                    os_kprintf("The stack of task %s is overflow!\r\n", name);
                   
                }
                else
                {
                    sp_bottom = task->stack_end;
                }
            }
            /* The running task in task context. */
            else
            {
                /* The running task always uses psp, task->sp is not used. */
                sp_top = (os_size_t*)os_get_current_task_sp();
                os_kprintf("SP : 0x%08x\r\n", sp_top);
                record.depth = 0;
                sp_func_call = sp_top;
                
                /*Task overflow.*/
                if((os_size_t)sp_top >= ((os_size_t)task->stack_end) 
                    || (sp_top < (os_size_t*)(task->stack_begin)))
                {
                    sp_bottom = sp_top + TASK_STACK_OVERFLOW_STACK_SIZE / sizeof(os_size_t);
                    os_kprintf("The stack of task %s is overflow!\r\n", name);
                }
                else
                {
                    sp_bottom = task->stack_end;
                }
            }
        }
    }

    trace_stack(sp_func_call, sp_bottom, &record);
    
    #ifdef EXC_DUMP_STACK
    dump_stack((os_uint32_t)sp_top, (os_uint32_t)sp_bottom - (os_uint32_t)sp_top, (os_size_t*)sp_top);
    #endif
    
    os_kprintf("You can use follow command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf/*.elf -a -f ");
    for (i = 0; i < record.depth; i++)
    {
        os_kprintf("0x%08x ", record.back_trace[i]);
    }
    os_kprintf("\r\n");

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       frame           The start address of the stack frame when the exception occurs.
 * @param[in]       msp             Interrupt stack pointer.
 * @param[in]       psp             Currently running task stack pointer.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */ 
void _arch_exception_stack_show(void *frame, os_size_t *msp, os_size_t *psp)
{
    /* The place where the function call backtrace starts after removing the registers in the stack. */
    os_size_t   *sp_func_call;
    
    os_size_t   *sp_top;    /* Stack position to start backtracking. */
    os_size_t   *sp_bottom;
    os_uint16_t  i;
    os_uint32_t  exc_return;
    os_uint32_t  thread_mode;
    
    call_back_trace_t               record;
    struct stack_frame              *stack_frame_pointer;
    
    exc_return = ((struct stack_frame *)frame)->manual_frame.exc_return;
    sp_top = (os_size_t*)frame;

    /* According to the exc_return to get the exception context is the interrupt or task. */
    if (exc_return & EXCEPTION_RETURN_MODE_MASK)
    {
        thread_mode = 1;
    }
    else
    {
        thread_mode = 0;
    }

    sp_func_call =  sp_top + sizeof(struct stack_frame) / sizeof(os_size_t);
    
    /* Hard fault is generated in interrupt context. */
    if (0 == thread_mode)
    {
        os_kprintf("Hard fault in interrupt\r\n");
        
        sp_bottom = sp_func_call;

        while((os_size_t)sp_bottom < g_main_stack_end_addr)
        {
            /*Find interrupt push exc return.*/
            if(OS_TRUE == disassembly_ins_is_exc_return(*sp_bottom))
            {
                break;
            }
            sp_bottom++;
        }
    }
    /* Hard fault is generated in task context. */
    else
    {
        os_kprintf("Hard fault in task: %s\r\n", (os_task_name(os_task_self())));

        /* Task overflow. */
        if ((os_size_t)sp_top >= ((os_size_t)os_task_self()->stack_end) 
            || (sp_top < (os_size_t*)(os_task_self()->stack_begin)))
        {
            sp_bottom = sp_top + (sizeof(struct stack_frame) + TASK_STACK_OVERFLOW_STACK_SIZE)
                                 / sizeof(os_size_t);
            
            os_kprintf("The stack of task %s is overflow!\r\n", os_task_name(os_task_self()));
        }
        else
        {
            sp_bottom = (os_size_t *)(os_task_self()->stack_end);
        }
    }

    stack_frame_pointer = (struct stack_frame *)frame;

    os_kprintf("R0 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r0);
    os_kprintf("R1 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r1);
    os_kprintf("R2 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r2);
    os_kprintf("R3 : 0x%08x\r\n", stack_frame_pointer->cpu_frame.r3);
    os_kprintf("R4 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r4);
    os_kprintf("R5 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r5);
    os_kprintf("R6 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r6);
    os_kprintf("R7 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r7);
    os_kprintf("R8 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r8);
    os_kprintf("R9 : 0x%08x\r\n", stack_frame_pointer->manual_frame.r9);
    os_kprintf("R10: 0x%08x\r\n", stack_frame_pointer->manual_frame.r10);
    os_kprintf("R11: 0x%08x\r\n", stack_frame_pointer->manual_frame.r11);
    os_kprintf("R12: 0x%08x\r\n", stack_frame_pointer->cpu_frame.r12);
    os_kprintf("LR : 0x%08x\r\n", stack_frame_pointer->cpu_frame.lr);
    os_kprintf("PC : 0x%08x\r\n", stack_frame_pointer->cpu_frame.pc);
    os_kprintf("PSR: 0x%08x\r\n", stack_frame_pointer->cpu_frame.psr);

    record.depth = 0;
    record.back_trace[record.depth++] = stack_frame_pointer->cpu_frame.pc;
    if ((stack_frame_pointer->cpu_frame.lr - 1) != stack_frame_pointer->cpu_frame.pc)
    {
        record.back_trace[record.depth++] = stack_frame_pointer->cpu_frame.lr - 1;
    }

    _os_arch_hard_fault_track();

    trace_stack(sp_func_call, sp_bottom, &record);

    os_kprintf("You can user follow command for backtrace:\r\n");
    os_kprintf("addr2line -e *.axf/*.elf -a -f ");
    for (i = 0; i < record.depth; i++)
    {
        os_kprintf("0x%08x ", record.back_trace[i]);
    }
    os_kprintf("\r\n");

    #ifdef EXC_DUMP_STACK
    dump_stack((os_uint32_t)sp_top, (os_uint32_t)sp_bottom - (os_uint32_t)sp_top, (os_size_t*)sp_top);
    #endif

#ifdef OS_USING_SHELL
    
    os_kprintf("=======================   Heap Info   =======================\r\n");
    extern os_err_t sh_memshow(os_int32_t argc, char **argv);
    sh_memshow(0, OS_NULL);

#endif /* OS_USING_SHELL */
}

#define BL_INS_MASK         0xF800
#define BL_INS_HIGH         0xF800
#define BL_INS_LOW          0xF000
#define BLX_INX_MASK        0xFF00
#define BLX_INX             0x4700

/**
 ***********************************************************************************************************************
 * @brief           This function check the disassembly instruction is exception return.
 *
 * @param[in]       ins    Instruction.
 *
 * @return          On success, return OS_TRUE; on error, return OS_FALSE.
 ***********************************************************************************************************************
 */
os_bool_t disassembly_ins_is_exc_return(os_size_t ins)
{
    if ((ins == 0xFFFFFFF1) 
     || (ins == 0xFFFFFFF9)
     || (ins == 0xFFFFFFFD))
    {
        return OS_TRUE;
    }

    return OS_FALSE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function check the disassembly instruction is 'BL' or 'BLX'.
 *
 * @param[in]       addr    Address of instruction
 *
 * @return          On success, return OS_TRUE; on error, return OS_FALSE.
 ***********************************************************************************************************************
 */
os_bool_t disassembly_ins_is_bl_blx(os_uint32_t addr) 
{
    os_uint16_t ins1 = *((os_uint16_t *)addr);
    os_uint16_t ins2 = *((os_uint16_t *)(addr + 2));

    /* instruction is 'BL' */
    if ((ins2 & BL_INS_MASK) == BL_INS_HIGH && (ins1 & BL_INS_MASK) == BL_INS_LOW) 
    {
        return OS_TRUE;
    }
    /* instruction is 'BLX' */
    else if ((ins2 & BLX_INX_MASK) == BLX_INX) 
    {
        return OS_TRUE;
    } 
    else 
    {
        return OS_FALSE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           The function will find and record all function calls in the stack.
 *
 * @attention       
 *
 * @param[in]       stack_top       The top of stack.
 * @param[in]       stack_bottom    The bottom of stack.
 * @param[out]      trace           a memory area is used to record function calls
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void trace_stack(os_size_t *stack_top, os_size_t *stack_bottom, call_back_trace_t *trace)
{
    os_uint32_t pc;
    os_size_t *sp;
    
    pc = 0;
    for (sp = stack_top; sp < stack_bottom; sp ++) 
    {
        /* the *sp value may be LR, so need decrease a word to PC */
        pc = *((os_uint32_t *) sp) - sizeof(os_size_t);
        
        /* the Cortex-M using thumb instruction, so the pc must be an odd number */
        if (pc % 2 == 0) 
        {
            continue;
        }

        /* fix the PC address in thumb mode */
        pc = *((os_uint32_t *) sp) - 1;
        if (trace->depth < CALL_BACK_TRACE_MAX_DEPTH)
        {
            if ((pc >= g_code_start_addr)
                && (pc < g_code_end_addr)
                && disassembly_ins_is_bl_blx(pc - sizeof(os_size_t)))
            {

                /* ignore repeat */
                if ((2 == trace->depth)
                    && (pc == trace->back_trace[1]))
                {
                    continue;
                }
            
                trace->back_trace[trace->depth++] = pc;
            }
        }
        else
        {
            break;
        }
    }
}

/**
***********************************************************************************************************************
* @brief           Initialize address range of code segment and main stack.
*
* @param           None
*
* @return          Will only return OS_EOK.
***********************************************************************************************************************
*/
os_err_t call_back_trace_init(void)
{

#if defined(__CC_ARM)

    g_main_stack_start_addr = (os_uint32_t)&CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    g_main_stack_end_addr   = (os_uint32_t)&CSTACK_BLOCK_END(CSTACK_BLOCK_NAME);
    g_code_start_addr       = (os_uint32_t)&CODE_SECTION_START(CODE_SECTION_NAME);
    g_code_end_addr         = (os_uint32_t)&CODE_SECTION_END(CODE_SECTION_NAME);
    
#elif defined(__ICCARM__)

    g_main_stack_start_addr = (os_uint32_t)__section_begin(CSTACK_BLOCK_NAME);
    g_main_stack_end_addr   = (os_uint32_t)__section_end(CSTACK_BLOCK_NAME);
    g_code_start_addr       = (os_uint32_t)__section_begin(CODE_SECTION_NAME);
    g_code_end_addr         = (os_uint32_t)__section_end(CODE_SECTION_NAME);
    
#elif defined(__GNUC__)

    g_main_stack_start_addr = (os_uint32_t)(&CSTACK_BLOCK_START);
    g_main_stack_end_addr   = (os_uint32_t)(&CSTACK_BLOCK_END);
    g_code_start_addr       = (os_uint32_t)(&CODE_SECTION_START);
    g_code_end_addr         = (os_uint32_t)(&CODE_SECTION_END);
    
#endif

    if (g_main_stack_start_addr >= g_main_stack_end_addr)
    {
        os_kprintf("ERROR: Unable to get the main stack information!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}
OS_APP_INIT(call_back_trace_init, OS_INIT_SUBLEVEL_LOW);

#endif /* STACK_TRACE_EN */

