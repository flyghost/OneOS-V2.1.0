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
 * @file        drv_sd.h
 *
 * @brief       This file provides sd card device register.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef DRV_SD_H__
#define DRV_SD_H__

#include "fsl_usdhc.h"
#ifdef __cplusplus
extern "C" {
#endif

os_err_t imxrt_usdhc_sd_register(struct os_imxrt_usdhc *imxrt_usdhc);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_SAI_H__ */


