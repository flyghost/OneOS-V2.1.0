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
 * @file        m5310a_onenet_nb.h
 *
 * @brief       m5310-a module link kit onenet nb api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __M5310A_ONENET_H__
#define __M5310A_ONENET_H__

#include "mo_onenet_nb.h"
#include <oneos_config.h>

#define DEFINE_M5310A_ONENET_FUNC(name, args) os_err_t name args

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_create, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_createex, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_addobj, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_set_nmi, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_open, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_notify, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_update, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_get_write, ONENET_NB_FUNC_ARGS);
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_writersp, ONENET_NB_FUNC_ARGS);

/* for inner */
typedef struct
{
    int ref;
    int nnmi;
    int nsmi;
} m5310a_nmi_t;

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_get_nmi, ONENET_NB_FUNC_ARGS);

#ifdef OS_USING_SHELL
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_all, ONENET_NB_FUNC_ARGS);
#endif

#endif /* __M5310A_ONENET_H__ */
