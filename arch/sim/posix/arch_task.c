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
 * @file        arch_task.c
 *
 * @brief       This file provides functions related to the SIM.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_types.h>
#include <os_stddef.h>
#include <os_task.h>
#include <os_errno.h>
#include <arch_task.h>
#include <os_clock.h>
#include <os_assert.h>
#include <os_memory.h>
#include <os_assert.h>

#include <errno.h>

#define __USE_GNU
#include <sched.h>

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>
#include <wait_for_event.h>

/*-----------------------------------------------------------*/

extern os_task_t        *g_os_current_task;
extern os_task_t        *g_os_next_task;
volatile os_ubase_t      g_os_interrupt_nest = 0;
extern volatile os_uint32_t g_interrupt_nesting;

os_uint32_t os_task_switch_interrupt_flag = 0;

#define SIG_RESUME SIGUSR1

typedef struct os_thread
{
    pthread_t pthread;
    void    (*task_entry)(void *arg) ;
    void     *arg;
    void    (*task_exit)(void);
    os_ubase_t dying;
    struct event*     thread_event;
} os_thread_t;

os_uint32_t os_hw_stack_max_used(void *stack_begin, os_uint32_t stack_size)
{
    os_uint8_t *addr;
    os_uint32_t max_used;

    addr = (os_uint8_t *)stack_begin;
    addr += sizeof(os_thread_t); 
    while (*addr == '$')
    {
        addr++;
    }

    max_used = (os_uint32_t)((os_ubase_t)stack_begin + stack_size - (os_ubase_t)addr);

    return max_used;
}


/*
 * The additional per-thread data is stored at the beginning of the
 * task's stack.
 */
os_thread_t *get_thread_from_task(os_task_t* task)
{
    return (os_thread_t *)(task->stack_begin);
}

/*-----------------------------------------------------------*/

static sigset_t gs_resume_signals;
static sigset_t gs_all_signals;
static sigset_t gs_scheduler_original_signal_mask;
static pthread_t gs_main_thread = ( pthread_t )NULL;
static int      gs_cpuid = 0;

/*-----------------------------------------------------------*/

static os_ubase_t gs_scheduler_end = OS_FALSE;
/*-----------------------------------------------------------*/

static void setup_signals_and_scheduler_policy(void);
static void setup_timer_interrupt(void);
static void *task_wait_for_start(void * arg);
static void switch_thread(os_thread_t *thread_to_resume, os_thread_t *thread_to_suspend);
static void thread_suspend(os_thread_t * thread);
static void thread_resume(os_thread_t * xThreadId );
static void SysTickHandler(int sig);

/*-----------------------------------------------------------*/

static void fatal_error( const char *call, int error_code )
{
    fprintf(stdout, "%s: %s\n", call, strerror(error_code));
    fflush(stdout);
    abort();
}

/* Stack down */
void *os_hw_stack_init(void        (*task_entry)(void *arg),
                       void         *arg,
                       void         *stack_begin,
                       os_uint32_t   stack_size,
                       void        (*task_exit)(void))
{
    os_thread_t *thread;
    pthread_attr_t thread_attributes;
    struct sched_param param;
    int ret;
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    CPU_SET(gs_cpuid, &cpuset);
    /*
     * Store the additional thread data at the start of the stack.
     */
    memset(stack_begin, '$', stack_size);
    thread = (os_thread_t *)stack_begin;

    thread->task_entry = task_entry;
    thread->arg = arg;
    thread->task_exit = task_exit;
    thread->dying = OS_FALSE;

    pthread_attr_init(&thread_attributes);
    //pthread_attr_setaffinity_np(&thread_attributes, sizeof(cpu_set_t), &cpuset);
                          
    ret = pthread_attr_setstack(&thread_attributes, stack_begin, stack_size);
    if (ret)
    {
        fatal_error("pthread_attr_setstack,the minimum stack space of linux threads is 16K", ret);
    }
    //param.sched_priority = OS_TASK_PRIORITY_MAX - 1 - task->current_priority;
    //pthread_attr_setschedparam(&thread_attributes, &param);

    thread->thread_event = event_create();

    os_ubase_t level = os_irq_lock();
    ret = pthread_create(&thread->pthread, &thread_attributes, task_wait_for_start, thread);
    if (ret)
    {
        fatal_error("pthread_create", ret);
    }
    os_irq_unlock(level);
    return stack_begin;
}

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
void os_first_task_start( void )
{
    int signal_tmp;
    sigset_t signals_set_tmp;

    gs_main_thread = pthread_self();

    /* Start the timer that generates the tick ISR(SIGALRM).
       Interrupts are disabled here already. */
    setup_timer_interrupt();

    /* Start the first task. */
    g_os_current_task = g_os_next_task;
    g_os_current_task->state  |= OS_TASK_STATE_RUNNING;
    os_thread_t *first_thread = get_thread_from_task(os_task_self());

    /* Start the first task. */
    disable_interrupts();
    thread_resume(first_thread);

    /* Wait until signaled by end_scheduler(). */
    sigemptyset(&signals_set_tmp);
    sigaddset(&signals_set_tmp, SIG_RESUME);

    sigwait( &signals_set_tmp, &signal_tmp);
}
/*-----------------------------------------------------------*/

void end_scheduler(void )
{
    struct itimerval itimer;
    struct sigaction sigtick;
    os_thread_t *current_thread;

    /* Stop the timer and ignore any pending SIGALRMs that would end
     * up running on the main thread when it is resumed. */
    itimer.it_value.tv_sec = 0;
    itimer.it_value.tv_usec = 0;

    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 0;
    (void)setitimer( ITIMER_REAL, &itimer, NULL );

    sigtick.sa_flags = 0;
    sigtick.sa_handler = SIG_IGN;
    sigemptyset( &sigtick.sa_mask );
    sigaction( SIGALRM, &sigtick, NULL );

    /* Signal the scheduler to exit its loop. */
    gs_scheduler_end = OS_TRUE;
    (void)pthread_kill( gs_main_thread, SIG_RESUME );

    current_thread = get_thread_from_task(os_task_self());
    thread_suspend(current_thread);
}

void os_task_switch(void)
{
    os_thread_t *thread_to_suspend;
    os_thread_t *thread_to_resume;

    #if 0
    if(g_os_interrupt_nest > 0)
    {        
        os_task_switch_interrupt_flag = 1;
        return ;
    }
    #endif
        
    thread_to_suspend = get_thread_from_task(os_task_self());

    //printf("g_os_current_task<%s> g_os_next_task<%s>\r\n",g_os_current_task->name,g_os_next_task->name);
    
    #ifdef OS_TASK_SWITCH_NOTIFY
    extern void os_task_switch_notify(void);
    os_task_switch_notify();
    #endif
    
    g_os_current_task->state &= ~OS_TASK_STATE_RUNNING;
    g_os_current_task = g_os_next_task;
    g_os_current_task->state |= OS_TASK_STATE_RUNNING;

    thread_to_resume = get_thread_from_task( os_task_self() );

    switch_thread(thread_to_resume, thread_to_suspend); 

}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void disable_interrupts(void)
{
    pthread_sigmask(SIG_BLOCK, &gs_all_signals, NULL);   
}
/*-----------------------------------------------------------*/

void enable_interrupts(void)
{
    pthread_sigmask(SIG_UNBLOCK, &gs_all_signals, NULL);
}
/*-----------------------------------------------------------*/


static os_uint64_t get_time_ns(void)
{
struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000000000ull + t.tv_nsec;
}

static os_uint64_t start_time_ns;
/* commented as part of the code below in SysTickHandler,
 * to adjust timing according to full demo requirements */
/* static uint64_t prvTickCount; */

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void setup_timer_interrupt( void )
{
    struct itimerval itimer;
    int ret;

    /* Initialise the structure with the current timer information. */
    ret = getitimer(ITIMER_REAL, &itimer);
    if (ret)
    {
        fatal_error("getitimer", errno);
    }

    OS_ASSERT(OS_TICK_PER_SECOND > 1);
    
    /* Set the interval between timer events. */
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 1000 * 1000 / OS_TICK_PER_SECOND;
    
    /* Set the current count-down. */
    itimer.it_value.tv_sec = 0;
    itimer.it_value.tv_usec = 1000 * 1000 / OS_TICK_PER_SECOND;

    /* Set-up the timer interrupt. */
    ret = setitimer(ITIMER_REAL, &itimer, NULL);
    if (ret)
    {
        fatal_error("setitimer", errno);
    }

    start_time_ns = get_time_ns();
}
/*-----------------------------------------------------------*/

static void SysTickHandler(int sig)
{
    os_thread_t *thread_to_suspend;
    os_thread_t *thread_to_resume;
    os_ubase_t   level = 0;
    
    level = os_irq_lock();
    
    g_os_interrupt_nest++; /* Signals are blocked in this signal handler. */
    os_tick_increase();
    g_os_interrupt_nest--;

    #if 0
    if(os_task_switch_interrupt_flag == 1)
    {
        os_task_switch_interrupt_flag = 0;        

        thread_to_suspend = get_thread_from_task(os_task_self());

        //printf("irq g_os_current_task<%s> g_os_next_task<%s>\r\n",g_os_current_task->name,g_os_next_task->name);

        #ifdef OS_TASK_SWITCH_NOTIFY
        extern void os_task_switch_notify(void);
        os_task_switch_notify();
        #endif
        
        g_os_current_task->state &= ~OS_TASK_STATE_RUNNING;
        g_os_current_task = g_os_next_task;
        g_os_current_task->state |= OS_TASK_STATE_RUNNING;
       
        thread_to_resume = get_thread_from_task(os_task_self());       

        switch_thread(thread_to_resume, thread_to_suspend); 
    }
    #endif
    
    os_irq_unlock(level);
}

/*-----------------------------------------------------------*/

void vPortThreadDying( void *pxTaskToDelete, volatile os_ubase_t *pxPendYield )
{
    os_thread_t *thread = get_thread_from_task( pxTaskToDelete );

    thread->dying = OS_TRUE;
}

void cancel_thread(void *task_to_delete)
{
    os_thread_t *thread = task_to_delete;

    /*
     * The thread has already been suspended so it can be safely cancelled.
     */
    pthread_cancel(thread->pthread);
    pthread_join(thread->pthread, NULL);
}
/*-----------------------------------------------------------*/

static void *task_wait_for_start( void * arg )
{
    os_thread_t *thread = arg;
    os_base_t level;

    thread_suspend(thread);

    /* Resumed for the first time, unblocks all signals. */
    g_interrupt_nesting = 0;
    enable_interrupts();


    level = os_irq_lock();
    os_task_set_cleanup_callback(os_task_self(), cancel_thread, arg);
    os_irq_unlock(level);

    /* Call the task's entry point. */
    thread->task_entry(thread->arg);
    
    thread->task_exit();
    //os_kprintf("g_os_current_task<%s> g_os_next_task<%s>\r\n",g_os_current_task->name,g_os_next_task->name);
    //os_kprintf("########thread->thread_id %d#########\r\n",thread->thread_id);
    OS_ASSERT(OS_FALSE);

    return NULL;
}
/*-----------------------------------------------------------*/

static void switch_thread(os_thread_t *thread_to_resume, os_thread_t *thread_to_suspend)
{
    os_ubase_t saved_interrupt_nesting;

    if ( thread_to_suspend != thread_to_resume )
    {
        /*
         * Switch tasks.
         *
         * The critical section nesting is per-task, so save it on the
         * stack of the current (suspending thread), restoring it when
         * we switch back to this task.
         */
         #if 0
         OS_ASSERT(g_interrupt_nesting > 0);
         #else
        if(g_interrupt_nesting <= 0)
        {
            printf("fun<%s> line<%d> g_interrupt_nesting %d\r\n",__FUNCTION__,__LINE__,g_interrupt_nesting);
            printf("g_os_current_task<%s> g_os_next_task<%s>\r\n",g_os_current_task->name,g_os_next_task->name);
            printf("os_task_switch_interrupt_flag %d \r\n",os_task_switch_interrupt_flag);
            OS_ASSERT(0);
        }
        #endif
        
        saved_interrupt_nesting = g_interrupt_nesting;

        thread_resume( thread_to_resume );
        if ( thread_to_suspend->dying )
        {
            printf("fun<%s> line<%d> g_interrupt_nesting %d\r\n",__FUNCTION__,__LINE__,g_interrupt_nesting);
            pthread_exit( NULL );
        }
        //printf("g_os_current_thread<0x%x> g_os_next_thread<0x%x> \r\n",thread_to_suspend->pthread,thread_to_resume->pthread);
        
        thread_suspend(thread_to_suspend);
        g_interrupt_nesting = saved_interrupt_nesting;
    }
}
/*-----------------------------------------------------------*/

static void thread_suspend(os_thread_t *thread)
{
    /*
     * Suspend this thread by waiting for a pthread_cond_signal event.
     *
     * A suspended thread must not handle signals (interrupts) so
     * all signals must be blocked by calling this from:
     *
     * - Inside a critical section (vPortEnterCritical() /
     *   vPortExitCritical()).
     *
     * - From a signal handler that has all signals masked.
     *
     * - A thread with all signals blocked with pthread_sigmask().
        */
    event_wait(thread->thread_event);
}

/*-----------------------------------------------------------*/

static void thread_resume( os_thread_t *thread )
{
    if ( pthread_self() != thread->pthread )
    {
        event_signal(thread->thread_event);
    }
}
/*-----------------------------------------------------------*/

static void setup_signals_and_scheduler_policy(void)
{
    struct sigaction sigresume, sigtick;
    int ret;

    gs_main_thread = pthread_self();
    //printf("gs_main_thread = 0x%x\r\n",(os_uint32_t)gs_main_thread);

    /* Initialise common signal masks. */
    sigemptyset( &gs_resume_signals );
    sigaddset( &gs_resume_signals, SIG_RESUME );
    sigfillset( &gs_all_signals );
    /* Don't block SIGINT so this can be used to break into GDB while
     * in a critical section. */
    sigdelset( &gs_all_signals, SIGINT );

    /*
     * Block all signals in this thread so all new threads
     * inherits this mask.
     *
     * When a thread is resumed for the first time, all signals
     * will be unblocked.
     */
    (void)pthread_sigmask( SIG_SETMASK, &gs_all_signals,
                           &gs_scheduler_original_signal_mask );

    /* SIG_RESUME is only used with sigwait() so doesn't need a
       handler. */
    sigresume.sa_flags = 0;
    sigresume.sa_handler = SIG_IGN;
    sigfillset( &sigresume.sa_mask);
    ret = sigaction(SIG_RESUME, &sigresume, NULL);

    if (ret)
    {
        fatal_error( "sigaction", errno );
    }
    

    sigtick.sa_flags = 0;
    sigtick.sa_handler = SysTickHandler;
    sigfillset(&sigtick.sa_mask);
    
    ret = sigaction( SIGALRM, &sigtick, NULL );
    if (ret)
    {
        fatal_error( "sigaction", errno );
    }
}
/*-----------------------------------------------------------*/

unsigned long get_run_time( void )
{
    struct tms times_tmp;

    times(&times_tmp);

    return (unsigned long)times_tmp.tms_utime;
}
/*-----------------------------------------------------------*/

 int system_init(void)
{
    #define HEAP_SIZE 100*1024*1024
    static char heap_base[HEAP_SIZE];
    os_sys_heap_init();
    os_sys_heap_add(heap_base, HEAP_SIZE, OS_MEM_ALG_FIRSTFIT);
    gs_cpuid = sched_getcpu();
    //printf("cpuid is %d\r\n", gs_cpuid);

    setup_signals_and_scheduler_policy();
    return OS_EOK;
}
OS_CORE_INIT(system_init,OS_INIT_SUBLEVEL_HIGH);

#ifdef OS_USING_OVERFLOW_CHECK
/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of task is overflow or not.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          Whether the stack of this task overflows.
 * @retval          OS_TRUE         The stack of this task is overflow.
 * @retval          OS_FALSE        The stack of this task is not overflow.
 ***********************************************************************************************************************
 */
os_bool_t os_task_stack_is_overflow(void *stack_top, void *stack_begin, void *stack_end)
{
    os_bool_t  is_overflow;
    
    OS_ASSERT(OS_NULL != stack_top);
    OS_ASSERT(OS_NULL != stack_begin);
    OS_ASSERT(OS_NULL != stack_end);

    if ((*((os_uint8_t *)stack_begin) != '$')
        || ((os_ubase_t)stack_top < (os_ubase_t)stack_begin)
        || ((os_ubase_t)stack_top >= (os_ubase_t)stack_end))
    {
        is_overflow = OS_TRUE;
    }
    else
    {
        is_overflow = OS_FALSE;
    }

    return is_overflow;
}
#endif /* OS_USING_OVERFLOW_CHECK */

