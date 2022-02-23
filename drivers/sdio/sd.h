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
 * @file        sd.h
 *
 * @brief       This file provides operation functions declaration for sd.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SD_H__
#define __SD_H__

#include <arch_interrupt.h>
#include <sdio/mmcsd_host.h>

#ifdef __cplusplus
extern "C" {
#endif

os_err_t mmcsd_send_if_cond(struct os_mmcsd_host *host, os_uint32_t ocr);
os_err_t mmcsd_send_app_op_cond(struct os_mmcsd_host *host, os_uint32_t ocr, os_uint32_t *rocr);

os_err_t   mmcsd_get_card_addr(struct os_mmcsd_host *host, os_uint32_t *rca);
os_int32_t mmcsd_get_scr(struct os_mmcsd_card *card, os_uint32_t *scr);

os_int32_t init_sd(struct os_mmcsd_host *host, os_uint32_t ocr);

#ifdef __cplusplus
}
#endif

#endif
