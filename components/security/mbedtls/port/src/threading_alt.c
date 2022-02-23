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
 * @file        thread_alt.c
 * 
 * @brief       Implementation of the functions when MBEDTLS_THREADING_ALT defined for mbedtls. 
 * 
 * @details
 * 
 * @revision
 * Date         Author          Notes
 * 2020-08-25   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef MBEDTLS_THREADING_ALT

#include "threading_alt.h"
#include "oneos_config.h"
#include "os_util.h"

void mbedtls_mutex_alt_init( mbedtls_threading_mutex_t *mutex )
{
    static os_uint16_t tls_alt_mutex_cnt =  0;
    char               name[OS_NAME_MAX] = {0};

    os_snprintf(name, sizeof(name), "tlsMu%02d", tls_alt_mutex_cnt++);

    mutex->mutex = os_mutex_create(name, OS_FALSE);
    mutex->valid = mutex->mutex? OS_TRUE : OS_FALSE;
}

void mbedtls_mutex_alt_free( mbedtls_threading_mutex_t *mutex )
{
    os_mutex_destroy(mutex->mutex);
    mutex->valid = 0;
}

int mbedtls_mutex_alt_lock( mbedtls_threading_mutex_t *mutex )
{
    return os_mutex_lock(mutex->mutex, OS_WAIT_FOREVER);
}

int mbedtls_mutex_alt_unlock( mbedtls_threading_mutex_t *mutex )
{
    return os_mutex_unlock(mutex->mutex);
}

#endif
