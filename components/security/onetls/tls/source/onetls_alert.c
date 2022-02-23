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
 * @file        onetls_alert.c
 *
 * @brief       onetls_alert functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_alert.h"
#include "onetls_lib.h"
#include "onetls_socket.h"

// 收发告警
uint32_t onetls_send_alert(onetls_ctx *ctx, uint8_t level, uint8_t desc)
{
    uint32_t ret = 0;
    ctx->send_msg[0] = level;
    ctx->send_msg[1] = desc;
    ctx->send_msg_len = 2;
    ctx->send_msg_type = ONETLS_MT_ALERT;
    if (ctx->state < ONETLS_STATE_OK) {
        ret = onetls_send_record_not_encrypt(ctx);
    } else {
        ret = onetls_send_record(ctx);
    }
    onetls_fflush(ctx);

    ctx->shutdown = ONETLS_SHUTDOWN_CODE(desc);
    return ret;
}

uint32_t onetls_recv_alert(onetls_ctx *ctx, uint8_t level, uint8_t desc)
{
    onetls_check_errlog(1, "recv an alert:level[%d], desc[%d]", level, desc);
    if ((level == ONETLS_ALERT_LEVEL_FATAL) || (desc == ONETLS_ALERT_DESCRIPTION_CLOSE_NOTIFY)) {
        if (ctx->state > ONETLS_STATE_INIT) {
            onetls_shutdown_ex(ctx);
        }
        ctx->shutdown = ONETLS_SHUTDOWN_CODE(desc);
        ctx->state = ONETLS_STATE_INIT;
    }
    return ONETLS_SUCCESS;
}

void onetls_notify_close(onetls_ctx *ctx)
{
    uint32_t ret = onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_WARNING, ONETLS_ALERT_DESCRIPTION_CLOSE_NOTIFY);
    onetls_check_errlog(ret, "onetls_send_alert");
}
