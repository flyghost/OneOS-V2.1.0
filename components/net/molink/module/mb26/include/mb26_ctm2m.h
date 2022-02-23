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
 * @file        mb26_ctm2m.h
 *
 * @brief       mb26 module link kit ctm2m api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MB26_CTM2M_H__
#define __MB26_CTM2M_H__

#include "mo_ctm2m.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MB26_USING_CTM2M_OPS

ctm2m_t *mb26_ctm2m_create(mo_object_t *module, ctm2m_create_parm_t parm);
os_err_t mb26_ctm2m_destroy(ctm2m_t *handle);
os_err_t mb26_ctm2m_set_ue_cfg(ctm2m_t *handle, ctm2m_ue_cfg_t cfg);
os_err_t mb26_ctm2m_get_ue_cfg(ctm2m_t *handle, ctm2m_ue_info_t *cfg_info);
os_err_t mb26_ctm2m_register(ctm2m_t *handle);
os_err_t mb26_ctm2m_deregister(ctm2m_t *handle);
os_err_t mb26_ctm2m_send(ctm2m_t *handle, ctm2m_send_t send, os_int32_t *send_msg_id);
os_err_t mb26_ctm2m_resp(ctm2m_t *handle, ctm2m_resp_t resp);
os_err_t mb26_ctm2m_update(ctm2m_t *handle, ctm2m_update_t update);

#endif /* MB26_USING_CTM2M_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MB26_CTM2M_H__ */
