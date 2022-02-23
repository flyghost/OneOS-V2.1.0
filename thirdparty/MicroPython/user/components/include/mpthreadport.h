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

#ifndef MICROPY_INCLUDED_MPTHREADPORT_H
#define MICROPY_INCLUDED_MPTHREADPORT_H
#include <os_event.h>
#include "py/obj.h"
#include "usr_misc.h"

typedef unsigned long usr_BaseType_t;
typedef unsigned int usr_StackType_t;

typedef struct mp_thread_event
{
    mp_obj_base_t base;
    os_event_t event;
} mp_thread_event_t;

typedef void *thread_id;
// this structure forms a linked list, one node per active thread
typedef struct _thread_t {
    thread_id id;        // system id of thread
    int status;             // whether the thread is ready and running
    void *arg;              // thread Python args, a GC root pointer
    void *stack;            // pointer to the stack
    size_t stack_len;       // number of words in the stack
    struct _thread_t *next;
} thread_t;

typedef void * mp_mutex;
typedef struct _mp_thread_mutex_t {
    mp_mutex mutex;
    int is_init;
} mp_thread_mutex_t;

void *mp_thread_get_stack_addr(void);
uint32_t mp_thread_get_stack_size(void);
void mp_thread_init(void *stack, uint32_t stack_len);
void mp_thread_gc_others(void);
void mp_thread_deinit(void);

struct _mp_state_thread_t;  /* _mp_state_thread_t 在mpstate.h声明, 如果直接include，则循环引用 */
struct _mp_state_thread_t *mp_thread_get_state(void);

void mp_thread_set_state(void *state);
void mp_thread_create(void *(*entry)(void*), void *arg, size_t *stack_size);
void mp_thread_start(void);
void mp_thread_finish(void);

void mp_thread_mutex_init(mp_thread_mutex_t *mutex);
int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait);
void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex);


#if defined(__CC_ARM) || defined(__CLANG_ARM)
mp_thread_event_t *mp_creat_event(const char *event_name);
int mp_send_event(const char *event_name, int set);
int mp_recv_event(const char *event_name, int set, int timeout);
void mp_delete_event(const char *event_name);
void mp_set_tick(int tick);
void mp_set_priority(unsigned char pri);
void mp_thread_event_test_event(void);
#endif


#endif // MICROPY_INCLUDED_ESP32_MPTHREADPORT_H
