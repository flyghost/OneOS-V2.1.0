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
 * @file        mo_mqttc_priv.h
 *
 * @brief       module link kit mqtt client private api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-04   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_MQTTC_PRIV_H__
#define __MO_MQTTC_PRIV_H__

#include "mo_mqttc.h"

#ifdef MOLINK_USING_MQTTC_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

os_err_t mo_mqttc_data_queue_push_notice(mo_mqttc_t *client, mqttc_msg_data_t *msg);
void     mo_mqttc_data_queue_disconnect_notice(mo_mqttc_t *client);
void     mo_mqttc_data_queue_destroy(mo_mqttc_t *client);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_MQTTC_OPS */

#endif /* __MO_MQTTC_PRIV_H__ */
