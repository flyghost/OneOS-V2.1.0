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
 * @file        rg500q.h
 *
 * @brief       rg500q factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __RG500Q_H__
#define __RG500Q_H__

#include "mo_object.h"
#include "oneos_config.h"

#ifdef RG500Q_USING_GENERAL_OPS
#include "rg500q_general.h"
#endif

#ifdef RG500Q_USING_NETSERV_OPS
#include "rg500q_netserv.h"
#endif

#ifdef RG500Q_USING_PPP_OPS
#include "rg500q_ppp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_RG500Q

#ifndef RG500Q_NAME
#define RG500Q_NAME          "rg500q"
#endif

#ifndef RG500Q_DEVICE_NAME
#define RG500Q_DEVICE_NAME   "usbh_cdc_hs"
#endif

#ifndef RG500Q_RECV_BUFF_LEN
#define RG500Q_RECV_BUFF_LEN (512)
#endif

typedef struct mo_rg500q
{
    mo_object_t parent;
#ifdef RG500Q_USING_PPP_OPS
    os_task_t  *rx_task;
    os_event_t *ppp_evt;
    os_mb_t    *ppp_mb;
    void       *ppp_ctlblock;
    void       *ppp_netif;
#endif /* RG500Q_USING_PPP_OPS */
} mo_rg500q_t;

mo_object_t *module_rg500q_create(const char *name, void *parser_config);
os_err_t     module_rg500q_destroy(mo_object_t *self);

#endif /* MOLINK_USING_RG500Q */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RG500Q_H__ */
