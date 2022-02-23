/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        cms_con_def.h
 *
 * @brief       Provides CMS connect component basic definition.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CNS_CON_DEF_H__
#define __CNS_CON_DEF_H__
#include "oneos_config.h"
#include "cms_error.h"
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    cms_connect_success       = 0,
    cms_connect_failure       = -1,
    cms_connect_param_err     = -2,
    cms_connect_not_connected = -3,
    cms_connect_auth_err      = -4,
    cms_connect_heart_timeout = -5,
    cms_connect_timeout       = -6,
} cms_connect_err_code;

typedef enum
{
    cms_con_state_disconnect = 0,
    cms_con_state_connect
} cms_con_state;

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#if defined(__cplusplus)
}
#endif

#endif
