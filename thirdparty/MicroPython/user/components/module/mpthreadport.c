/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George on behalf of Pycom Ltd
 * Copyright (c) 2017 Pycom Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <os_task.h>
#include <os_event.h>
#include <os_mutex.h>
#include <os_clock.h>
#include <pin.h>
#include <drv_gpio.h>
#include "py/mpconfig.h"
#include "py/mpstate.h"
#include "py/gc.h"
#include "py/mpthread.h"
#include "mpthreadport.h"
#include "usr_pin.h"

#if MICROPY_PY_THREAD
unsigned char thread_priority = 19; /* same as main thd */
unsigned int thread_tick = 0xff;
mp_thread_event_t *test_event = NULL;

#define MP_THREAD_MIN_STACK_SIZE                        (5 * 1024)
#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + 1024)

#define MP_THREAD_STATUS_READY                   0
#define MP_THREAD_STATUS_RUNNING                 1
#define MP_THREAD_STATUS_FINISH                  2

static char mp_thread_name[16] = {0};
int  g_thread_counter = 0;
uint32_t g_thread_total_size = 0;
// the mutex controls access to the linked list
STATIC mp_thread_mutex_t g_thread_mutex;
STATIC thread_t g_thread_root_node;
STATIC thread_t *g_thread_root; // root pointer, handled by mp_thread_gc_others
#define MP_THREAD_EVENT_MAX             3
static mp_thread_event_t *g_thread_event[MP_THREAD_EVENT_MAX] = {NULL};

int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait) 
{
    return (OS_EOK == os_mutex_lock((struct os_mutex *)(mutex->mutex), (wait ? OS_WAIT_FOREVER : OS_NO_WAIT)));
}

void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex) 
{
    os_mutex_unlock((struct os_mutex *)(mutex->mutex));
}

void mp_thread_set_state(void *state) 
{
    os_task_self()->user_data = state;
    return;
}

void mp_thread_mutex_init(mp_thread_mutex_t *mutex) 
{
    //usr_CreateMutexStatic(mutex);
    static uint8_t count = 0;
    char name[OS_NAME_MAX];

    if (!mutex->is_init) 
    {
        /* build name */
        snprintf(name, sizeof(name), "mpl%02d", count++);
        mutex->mutex = os_mutex_create(name, OS_FALSE);
        if (!mutex->mutex)
        {
            mp_err("Created mutex falied!");
            while (1);
        }
        //os_mutex_init((struct os_mutex *)(mutex->mutex), name, OS_IPC_FLAG_FIFO, OS_FALSE);
        mutex->is_init = 1;
    }

    return;
}

void mp_thread_mutex_deinit(mp_thread_mutex_t *mutex) {
    if (mutex->is_init) 
    {
        os_mutex_destroy(mutex->mutex);
        mutex->is_init = 0;
    }
    
    return;
}


void *mp_thread_get_stack_addr(void)
{
    return os_task_self()->stack_begin;
}

uint32_t mp_thread_get_stack_size(void)
{
    os_task_t *task = os_task_self();
    return (uint32_t)((u32_t *)(task->stack_end) - (u32_t *)task->stack_begin);
}

void mp_thread_init(void *stack, uint32_t stack_len) {
    mp_thread_set_state(&mp_state_ctx.thread);
    // create the first entry in the linked list of all threads
    g_thread_root = &g_thread_root_node;
    g_thread_root->id = os_task_self();
    g_thread_root->status = MP_THREAD_STATUS_RUNNING;
    g_thread_root->arg = NULL;
    g_thread_root->stack = stack;
    g_thread_root->stack_len = stack_len;
	g_thread_root->next = NULL;
	
    mp_thread_mutex_init(&g_thread_mutex);
}

void mp_thread_gc_others(void) {
	
    mp_thread_mutex_lock(&g_thread_mutex, 1);
    for (thread_t *th = g_thread_root; th != NULL; th = th->next) {

        if (th->id != os_task_self()) {
            gc_collect_root((void**)&th, 1);
			gc_collect_root(&th->arg, 1); // probably not needed
        }

        if (th->status == MP_THREAD_STATUS_READY) {
            continue;
        }

		gc_collect_root((void**) &th->id, 1);
        gc_collect_root(th->stack, th->stack_len); // probably not needed
    }
    mp_thread_mutex_unlock(&g_thread_mutex);
}

mp_state_thread_t *mp_thread_get_state(void) 
{
    return (mp_state_thread_t *)(os_task_self()->user_data);
}

void mp_thread_start(void) {
    mp_thread_mutex_lock(&g_thread_mutex, 1);
	for (thread_t *th = g_thread_root; th != NULL; th = th->next) {
        if (th->id == os_task_self()) {
            th->status = MP_THREAD_STATUS_RUNNING;
            break;
        }
    }
    mp_thread_mutex_unlock(&g_thread_mutex);
}

void mp_thread_create(void *(*entry)(void*), void *arg, size_t *stack_size) 
{
    if (g_thread_counter >= MP_THREAD_MAX_NUM)
    {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Reach maximum."));
    }

    if (*stack_size == 0) 
    {
        *stack_size = MP_THREAD_DEFAULT_STACK_SIZE; // default stack size
    } else if (*stack_size < MP_THREAD_MIN_STACK_SIZE) {
        *stack_size = MP_THREAD_MIN_STACK_SIZE; // minimum stack size
    }
    
    if ((g_thread_total_size + *stack_size) >= MICROPY_HEAP_SIZE)
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, 
                                        "No mem for a new thread.Count:%d, size:%d", 
                                        g_thread_counter, g_thread_total_size));
    }

    // Allocate linked-list node (must be outside thread_mutex lock)
    thread_t *th = m_new_obj(thread_t);
    if (th == NULL) 
    {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "can't create thread obj"));
    }

    th->id = m_new_obj(struct os_task);
    if (th->id == NULL) 
    {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "can't create thread id"));
    }
    memset(mp_thread_name, 0, sizeof(mp_thread_name));
    snprintf(mp_thread_name, sizeof(mp_thread_name), "mp_thd_%d", th->id);
    th->stack = m_new(uint8_t, *stack_size);
    if (th->stack == NULL) 
    {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "can't create thread stack"));
    }
    
    mp_log("(mpthread : 0x%p) stack addr is :0x%p, size is %d", th, th->stack, *stack_size);
    mp_thread_mutex_lock(&g_thread_mutex, 1);

    // adjust the stack_size to provide room to recover from hitting the limit
    *stack_size -= 1024;

    // add thread to linked list of all threads
    th->status = MP_THREAD_STATUS_READY;
    th->arg = arg;

    th->stack_len = *stack_size / sizeof(usr_StackType_t);
    th->next = g_thread_root;
    g_thread_root = th;

    // create thread
    os_task_init(th->id, mp_thread_name, (void (*)(void *))entry, arg, 
                        th->stack, *stack_size, thread_priority);
    int result = os_task_startup(th->id);
    if (result != 0) 
    {
        mp_thread_mutex_unlock(&g_thread_mutex);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "can't create thread"));
    }
    g_thread_counter++;
    g_thread_total_size += *stack_size;
    mp_thread_mutex_unlock(&g_thread_mutex);
    mp_log("Create thread[%s] success!", mp_thread_name);
    return;
}

void mp_thread_finish(void) 
{
    thread_t *prev = NULL;

    mp_thread_mutex_lock(&g_thread_mutex, 1);
    for (thread_t *th = g_thread_root; th != NULL;prev = th, th = th->next) 
    {
        // unlink the node from the list
        if (th->id == os_task_self()) 
        {
            if (prev != NULL) 
            {
                prev->next = th->next;
            } 
            else 
            {
                // move the start pointer
                g_thread_root = th->next;
            }
            th->status = MP_THREAD_STATUS_FINISH;
            // explicitly release all its memory
            m_del_obj(struct os_task, th->id);
            m_del(uint8_t, th->stack, th->stack_len * sizeof(usr_StackType_t) + 1024);
            m_del_obj(thread_t, th);
            g_thread_counter--;
            if (g_thread_counter == 0 || 
                g_thread_total_size < (th->stack_len * sizeof(usr_StackType_t) + 1024))
            {
                g_thread_total_size = 0;
            } 
            else 
            {
                g_thread_total_size -= (th->stack_len * sizeof(usr_StackType_t) + 1024);
            }
            mp_log("The thread number left[%d].", g_thread_counter);
            break;
        }
    }
    mp_thread_mutex_unlock(&g_thread_mutex);
}

void mp_thread_deinit(void) 
{
    for (thread_t *th = g_thread_root; th != NULL; th = th->next) 
    {
        if ((th != &g_thread_root_node) && (th->status != MP_THREAD_STATUS_FINISH))
        {
            os_task_deinit(th->id);
        }
    }

    return;
}


typedef struct mp_thread_event_arg {
    void *event_name;
    int set;
    int timeout;
} mp_thread_event_arg_t;

static int mp_recv_thread_event(int event_index, void *event_arg)
{
    mp_thread_event_arg_t *arg = (mp_thread_event_arg_t *)event_arg;
    int ret = MP_ERROR;
    
    if (strcmp(g_thread_event[event_index]->event.name, arg->event_name) == 0)
    {
        ret = os_event_recv(&g_thread_event[event_index]->event, 
                        arg->set, 
                        OS_EVENT_OPTION_AND | OS_EVENT_OPTION_CLEAR, 
                        arg->timeout, 
                        NULL);
        if (ret == 0)
        {
            mp_log("Receive event[%s, %d, %d] success.", 
                    arg->event_name, arg->set, arg->timeout);
            return MP_EOK;
        }
        
        mp_err("Receive event[%s, %d, %d] failed.", 
                arg->event_name, arg->set, arg->timeout);
    }

    return MP_ERROR;
}

static int mp_send_thread_event(int event_index, void *event_arg)
{
    mp_thread_event_arg_t *arg = (mp_thread_event_arg_t *)event_arg;
    int ret = MP_ERROR;

    if (strcmp(g_thread_event[event_index]->event.name, arg->event_name) == 0)
    {
        ret = os_event_send(&g_thread_event[event_index]->event, arg->set);
        if (ret == 0)
        {
            mp_log("Send event[%s, %d] success.", 
                    arg->event_name, arg->set);
            return MP_EOK;
        }
        
        mp_err("Send event[%s, %d] failed.", 
                arg->event_name, arg->set);
    }

    return MP_ERROR;
}


static int mp_save_thread_event(int event_index, void *event_arg)
{
    if (g_thread_event[event_index] == NULL)
    {
        g_thread_event[event_index] = (mp_thread_event_t *)event_arg;
        mp_log("Save the event[%s].", ((mp_thread_event_t *)event_arg)->event.name);
        return MP_EOK;
    }
    
    return MP_ERROR;
}

static int mp_move_thread_event(int event_index, void *event_arg)
{
    if (strcmp(g_thread_event[event_index]->event.name, (const char *)event_arg) == 0)
    {   
        os_event_deinit(&g_thread_event[event_index]->event);
        m_del(mp_thread_event_t, g_thread_event[event_index], sizeof(mp_thread_event_t));
        g_thread_event[event_index] = NULL;
        mp_log("Move the event[%s].", event_arg);
        return MP_EOK;
    }

    return MP_ERROR;
}

static int mp_for_each_thread_event(int (*func)(int event_index, void *arg), void *event_arg)
{
    int i = 0;

    for (i = 0; i < MP_THREAD_EVENT_MAX; i++)
    {
        if (func(i, event_arg) == MP_EOK)
        {
            mp_log("Handle the evevt success.");
            break;
        }
    }

    return (i == MP_THREAD_EVENT_MAX) ? MP_ERROR : MP_EOK;
}

mp_thread_event_t *mp_creat_event(const char *event_name)
{ 
    mp_thread_event_t *event_obj = m_new_obj(mp_thread_event_t);
    if(event_obj == NULL)
    {
        mp_err("Malloc event memory failed!");
        return NULL;
    }
    
    if (mp_for_each_thread_event(mp_save_thread_event, event_obj) != MP_EOK)
    {
        m_del(mp_thread_event_t, event_obj, sizeof(mp_thread_event_t));
        mp_err("Reach the event maximum.");
        return NULL;
    }
    
    os_event_init(&event_obj->event, event_name);
    mp_log("Create a event[%s].", event_name);
    return event_obj;
}

int mp_send_event(const char *event_name, int set)
{
    mp_thread_event_arg_t send_arg = {0};
    
    send_arg.event_name = (void *)event_name;
    send_arg.set = set;
    if (mp_for_each_thread_event(mp_send_thread_event, (void *)&send_arg) != MP_EOK)
    {
        return -1;
    }

    return 0;
}

int mp_recv_event(const char *event_name, int set, int timeout)
{
    mp_thread_event_arg_t recv_arg = {0};
    
    recv_arg.set = set;
    recv_arg.timeout = timeout;
    recv_arg.event_name = (void *)event_name;
    if (mp_for_each_thread_event(mp_recv_thread_event, (void *)&recv_arg) != MP_EOK)
    {
        return -1;
    }

    return 0;
}

void mp_delete_event(const char *event_name)
{
    if (mp_for_each_thread_event(mp_move_thread_event, (void *)event_name) != MP_EOK)
    {
        mp_err("Delete event[%s] failed.", event_name);
    }
    return;
}

void mp_set_tick(int tick)
{
    if((tick * OS_TICK_PER_SECOND) >= 1000)
    {
        thread_tick = (OS_TICK_PER_SECOND * tick) / 1000;
    }
    else
    {
        thread_tick = 1;
    }

    return;
}

void mp_set_priority(unsigned char pri)
{
    thread_priority = pri;
    return;
}

void call_sendevent(void *args)
{
    mp_log("send event:%d\n", os_tick_get());
    os_event_send(&test_event->event, 0xff);
}


void mp_thread_event_test_event(void)
{
    test_event = m_new_obj(mp_thread_event_t);
    os_event_init(&test_event->event, "mpyevent");
    os_pin_mode(44, PIN_MODE_INPUT_PULLUP); //PA11
    os_pin_attach_irq(44, PIN_IRQ_MODE_FALLING, call_sendevent ,OS_NULL);
    os_pin_irq_enable(44, PIN_IRQ_ENABLE);

    os_event_recv(&test_event->event, 0xff, OS_EVENT_OPTION_AND | OS_EVENT_OPTION_CLEAR, 0xffffffff, NULL);
    mp_log("recv event:%d", os_tick_get());
    return;
}

#else

void vPortCleanUpTCB(void *tcb) {
}

/*
void vPortCleanUpTCB(void *tcb) {
    thread_t *prev = NULL;
    mp_thread_mutex_lock(&g_thread_mutex, 1);
    for (thread_t *th = thread; th != NULL; prev = th, th = th->next) {
        // unlink the node from the list
        if ((void*)th->id == tcb) {
            if (prev != NULL) {
                prev->next = th->next;
            } else {
                // move the start pointer
                thread = th->next;
            }
            // explicitly release all its memory
            m_del(thread_t, th, 1);
            break;
        }
    }
    mp_thread_mutex_unlock(&g_thread_mutex);
}
*/

#endif // MICROPY_PY_THREAD
