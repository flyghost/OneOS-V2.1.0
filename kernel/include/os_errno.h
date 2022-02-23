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
 * @file        os_errno.h
 *
 * @brief       Error code macro definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_ERRNO_H__
#define __OS_ERRNO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OS_EOK           0      /* There is no error. */
#define OS_ERROR        -1      /* A generic error happens. */
#define OS_ETIMEOUT     -2      /* Timed out. */
#define OS_EFULL        -3      /* The resource is full. */
#define OS_EEMPTY       -4      /* The resource is empty. */
#define OS_ENOMEM       -5      /* No memory. */
#define OS_ENOSYS       -6      /* Function not implemented. */
#define OS_EBUSY        -7      /* Busy. */
#define OS_EIO          -8      /* IO error. */
#define OS_EINTR        -9      /* Interrupted system call. */
#define OS_EINVAL       -10     /* Invalid argument. */
#define OS_ENODEV       -11     /* No such device */
#define OS_EPERM        -12     /* Operation not permitted */

#ifdef __cplusplus
}
#endif

#endif

