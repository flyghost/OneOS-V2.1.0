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
 * @file        posix_types.h
 *
 * @brief       Posix common data type definitions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __POSIX_TYPES_H__
#define __POSIX_TYPES_H__

#include <oneos_config.h>
#include <os_stddef.h>

#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>

#if defined(__CC_ARM) || defined(__IAR_SYSTEMS_ICC__)
#include <sys/errno.h>
#else
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

#endif

