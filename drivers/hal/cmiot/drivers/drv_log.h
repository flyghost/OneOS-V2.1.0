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
 * @file        drv_log.h
 *
 * @brief       This file defines driver log with specific level and tag.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_EXT_TAG
#define DBG_EXT_TAG "drv"
#else
#define DBG_EXT_TAG DRV_EXT_TAG
#endif /* DRV_EXT_TAG */

#ifdef DRV_EXT_LVL
#define DBG_EXT_LVL DRV_EXT_LVL
#else
#define DBG_EXT_LVL DBG_EXT_INFO
#endif /* DRV_EXT_LVL */
