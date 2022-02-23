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
 * @file        cms_id.h
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CMS_ID_H__
#define __CMS_ID_H__

#include "oneos_config.h"
#include "cms_config.h"
#include "cms_error.h"
#include <stdint.h>
#include <stddef.h>

typedef enum
{
	PID = 0,
	OID,
	DID,
	CTI_DEVICE,
	CTI_PLATFORM,
	CWT_DEVICE,
	CWT_PLATFORM,
	KEY_MASTER
} CMS_TYPE_T;


int cms_id_get_id_len(const CMS_TYPE_T type, size_t *len);

int cms_id_get_id(const CMS_TYPE_T type, uint8_t *buf, const size_t len);

int cms_id_cwt_check(uint8_t *cwt, const uint8_t len);

int cms_id_regist_check(void);

int cms_id_auth_check(void);

int cms_id_regist(void);

int cms_id_authorize(void);

#endif /* __CMS_ID_H__ */
