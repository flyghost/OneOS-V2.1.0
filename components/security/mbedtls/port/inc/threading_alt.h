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
 * @file        threading_alt.h
 *
 * @brief       mbedtls alternate threading functions header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __THREADING_ALT_H__
#define __THREADING_ALT_H__

#include "os_types.h"
#include "os_mutex.h"

#ifndef OS_USING_MUTEX
#error "MBEDTLS_THREADING_ALT need to define OS_USING_MUTEX!"
#endif

typedef struct mbedtls_threading_mutex
{
    os_bool_t   valid;
    os_mutex_t *mutex;
} mbedtls_threading_mutex_t;

extern void mbedtls_mutex_alt_init( mbedtls_threading_mutex_t *mutex );
extern void mbedtls_mutex_alt_free( mbedtls_threading_mutex_t *mutex );
extern int mbedtls_mutex_alt_lock( mbedtls_threading_mutex_t *mutex );
extern int mbedtls_mutex_alt_unlock( mbedtls_threading_mutex_t *mutex );

#endif /* __THREADING_ALT_H__ */
