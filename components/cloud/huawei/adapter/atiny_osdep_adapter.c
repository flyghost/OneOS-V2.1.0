/*
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
 * @file        atiny_osdep_adapter.c
 *
 * @brief       huawei cloud sdk file "atiny_osdep.c" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdlib.h>
#include "osdepends/atiny_osdep.h"

/**
 *@ingroup atiny_adapter
 *@brief get time since the system was boot.
 *
 *@par Description:
 *This API is used to retrieves the number of milliseconds that have elapsed since the system was boot.
 *@attention none.
 *
 *@param none.
 *
 *@retval #uint64_t     the number of milliseconds.
 *@par Dependency: none.
 *@see atiny_usleep
 */
#include <os_clock.h>
uint64_t atiny_gettime_ms(void)
{
    volatile uint64_t tick = (uint64_t)os_tick_get();
    return (tick * (1000 / os_tick_from_ms(1000)));
}

/**
 *@ingroup atiny_adapter
 *@brief sleep thread itself.
 *
 *@par Description:
 *This API is used to sleep thread itself, it could be implemented in the user file.
 *@attention none.
 *
 *@param usec           [IN] the time interval for which execution is to be suspended, in microseconds.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_gettime_ms
 */
#include <os_task.h>
void atiny_usleep(unsigned long usec)
{
    os_task_msleep(usec / 1000);
}

/**
 *@ingroup atiny_adapter
 *@brief get len bytes of entropy.
 *
 *@par Description:
 *This API is used to get len bytes of entropy, it could be implemented in the user file.
 *@attention none.
 *
 *@param output         [OUT] buffer to store the entropy
 *@param len            [IN] length of the entropy
 *
 *@retval #0            Succeed.
 *@retval #-1           Failed.
 *@par Dependency: none.
 *@see none.
 */
int atiny_random(void *output, size_t len)
{
#ifdef OS_USING_RTC
    srand((int)time(0));
#else
    srand((int)os_tick_get());
#endif
    uint8_t *buff = (uint8_t *)output;
    for (int i = 0; i <= len / 4; i++)
    {
        uint32_t ran = rand();
        for (int j = 0; j < 4; j++)
        {
            if (i * 4 + j >= len)
            {
                return 0;
            }
            buff[i * 4 + j] = (uint8_t)((ran >> (j * 8)) & 0xFF);
        }
    }
    return 0;
}

/**
 *@ingroup atiny_adapter
 *@brief allocates a block of size bytes of memory
 *
 *@par Description:
 *This API is used to allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *@attention none.
 *
 *@param size           [IN] specify block size in bytes.
 *
 *@retval #pointer      pointer to the beginning of the block.
 *@par Dependency: none.
 *@see atiny_free
 */
#include <stdlib.h>
void *atiny_malloc(size_t size)
{
    return malloc(size);
}

/**
 *@ingroup atiny_adapter
 *@brief deallocate memory block.
 *
 *@par Description:
 *This API is used to deallocate memory block.
 *@attention none.
 *
 *@param size           [IN] pointer to the beginning of the block previously allocated with atiny_malloc.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_malloc
 */
void atiny_free(void *ptr)
{
    free(ptr);
}

/**
 *@ingroup atiny_adapter
 *@brief writes formatted data to string.
 *
 *@par Description:
 *This API is used to writes formatted data to string.
 *@attention none.
 *
 *@param buf            [OUT] string that holds written text.
 *@param size           [IN] maximum length of character will be written.
 *@param format         [IN] format that contains the text to be written, it can optionally contain embedded
                             format specifiers that specifies how subsequent arguments are converted for output.
 *@param ...            [IN] the variable argument list, for formatted and inserted in the resulting string
                             replacing their respective specifiers.
 *
 *@retval #int          bytes of character successfully written into string.
 *@par Dependency: none.
 *@see none.
 */
#include <stdio.h>
int atiny_snprintf(char *buf, unsigned int size, const char *format, ...)
{
    int     len = 0;
    va_list argp;
    va_start(argp, format);
    len = vsnprintf(buf, size, format, argp);
    len = len > size - 1 ? size - 1 : len;
    va_end(argp);
    return len;
}

/**
 *@ingroup atiny_adapter
 *@brief writes formatted data to stream.
 *
 *@par Description:
 *This API is used to writes formatted data to stream.
 *@attention none.
 *
 *@param format         [IN] string that contains the text to be written, it can optionally contain embedded
                             format specifiers that specifies how subsequent arguments are converted for output.
 *@param ...            [IN] the variable argument list, for formatted and inserted in the resulting string
                             replacing their respective specifiers.
 *
 *@retval #int          bytes of character successfully written into stream.
 *@par Dependency: none.
 *@see none.
 */
int atiny_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return 0;
}

/**
 *@ingroup atiny_strdup
 *@brief returns a pointer to a new string which is a duplicate of the string ch
 *
 *@par Description:
 *This API returns a pointer to a new string which is a duplicate of the string ch.
   Memory for the new string is obtained with atiny_malloc, and can be freed with atiny_free
 *@attention none.
 *
 *@param format         [IN] const char pointer
 *@param ch            [IN] const char pointer points to the string to be duplicated
 *
 *@retval #char pointer          a pointer to a new string which is a duplicate of the string ch
 *@par Dependency: none.
 *@see atiny_malloc atiny_free.
 */
char *atiny_strdup(const char *ch)
{
    size_t len    = strlen(ch) + 1;
    char  *new_ch = (char *)atiny_malloc(len);
    if (new_ch == NULL)
        return NULL;
    memcpy(new_ch, ch, len);
    new_ch[len - 1] = 0;
    return new_ch;
}

/**
 *@ingroup atiny_adapter
 *@brief create a mutex.
 *
 *@par Description:
 *This API is used to create a mutex.
 *@attention none.
 *
 *@param none.
 *
 *@retval #pointer      the mutex handle.
 *@retval NULL          create mutex failed.
 *@par Dependency: none.
 *@see atiny_mutex_destroy | atiny_mutex_lock | atiny_mutex_unlock
 */
#include <os_mutex.h>
void *atiny_mutex_create(void)
{
    static os_uint32_t index = 0;
    char               mutex_name[OS_NAME_MAX];

    snprintf(mutex_name, OS_NAME_MAX, "hw_%d", index++);
    os_mutex_t *mutex = os_mutex_create(mutex_name, OS_FALSE);

    if (mutex == NULL)
        return NULL;
    return mutex;
}

/**
 *@ingroup atiny_adapter
 *@brief destroy the specified mutex object.
 *
 *@par Description:
 *This API is used to destroy the specified mutex object, it will release related resource.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_lock | atiny_mutex_unlock
 */
void atiny_mutex_destroy(void *mutex)
{
    (void)os_mutex_destroy((os_mutex_t *)mutex);
}

/**
 *@ingroup atiny_adapter
 *@brief waits until the specified mutex is in the signaled state.
 *
 *@par Description:
 *This API is used to waits until the specified mutex is in the signaled state.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_destroy | atiny_mutex_unlock
 */
void atiny_mutex_lock(void *mutex)
{
    (void)os_mutex_lock((os_mutex_t *)mutex, OS_WAIT_FOREVER);
}

/**
 *@ingroup atiny_adapter
 *@brief releases ownership of the specified mutex object.
 *
 *@par Description:
 *This API is used to releases ownership of the specified mutex object.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_destroy | atiny_mutex_lock
 */
void atiny_mutex_unlock(void *mutex)
{
    (void)os_mutex_unlock((os_mutex_t *)mutex);
}

/**
 *@ingroup atiny_adapter
 *@brief reboot.
 *
 *@par Description:
 *This API is used to reboot, it could be implemented in the user file.
 *@attention none.
 *
 *@retval none.
 *@par Dependency: none.
 *@see none.
 */
void atiny_reboot(void)
{
    /* To do */
}

/**
 *@ingroup atiny_adapter
 *@brief task delay.
 *
 *@par Description:
 *This API is used to delay some seconds.
 *@attention none.
 *
 *@param second          [IN] the specified second.
 *
 *@retval none.
 *@par Dependency: none.
 *@see none.
 */
void atiny_delay(uint32_t second)
{
    (void)os_task_msleep(second * 1000);
}

#if (LOSCFG_BASE_IPC_MUX == YES)

static char atiny_task_mutex_is_valid(const atiny_task_mutex_s *mutex)
{
    return (mutex != NULL) && (mutex->valid);
}

int atiny_task_mutex_create(atiny_task_mutex_s *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR;
    }

    (void)memset(mutex, 0, sizeof(atiny_task_mutex_s));
    mutex->mutex = atiny_mutex_create();
    if (mutex->mutex == NULL)
    {
        return OS_ERROR;
    }
    mutex->valid = TRUE;
    return LOS_OK;
}

#define ATINY_DESTROY_MUTEX_WAIT_INTERVAL 100
int atiny_task_mutex_delete(atiny_task_mutex_s *mutex)
{
    if (!atiny_task_mutex_is_valid(mutex))
    {
        return ERR;
    }
    atiny_mutex_destroy(mutex->mutex);

    (void)memset(mutex, 0, sizeof(atiny_task_mutex_s));

    return LOS_OK;
}
int atiny_task_mutex_lock(atiny_task_mutex_s *mutex)
{
    if (mutex == NULL)
    {
        return ERR;
    }

    if (!atiny_task_mutex_is_valid(mutex))
    {
        return ERR;
    }

    atiny_mutex_lock(mutex->mutex);
    return 0;
}
int atiny_task_mutex_unlock(atiny_task_mutex_s *mutex)
{
    if (mutex == NULL)
    {
        return ERR;
    }

    if (!atiny_task_mutex_is_valid(mutex))
    {
        return ERR;
    }
    atiny_mutex_unlock(mutex->mutex);
    return 0;
}
#endif /* LOSCFG_BASE_IPC_MUX == YES */
