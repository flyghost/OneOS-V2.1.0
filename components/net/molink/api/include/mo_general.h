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
 * @file        mo_general.h
 *
 * @brief       module link kit general api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_GENERAL_H__
#define __MO_GENERAL_H__

#include "mo_type.h"
#include "mo_object.h"
#include <time.h>

#ifdef MOLINK_USING_GENERAL_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MO_IMEI_LEN               (15)              /* International Mobile Equipment Identity */
#define MO_IMSI_LEN               (15)              /* International Mobile Subscriber Identity */
#define MO_ICCID_LEN              (20)              /* Integrate Circuit Card Identity */

/**
 ***********************************************************************************************************************
 * @struct      mo_firmware_version_t
 *
 * @brief       molink module firmware version infomation
 ***********************************************************************************************************************
 */
typedef struct mo_firmware_version
{
    os_size_t line_counts;  /* The number of lines of firmware version information */
    char    **ver_info;     /* The text of the firmware version information */
} mo_firmware_version_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_general_ops_t
 *
 * @brief       molink module general ops table
 ***********************************************************************************************************************
 */
typedef struct mo_general_ops
{
    os_err_t (*at_test)(mo_object_t *self);
    os_err_t (*get_imei)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_imsi)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_iccid)(mo_object_t *self, char *value, os_size_t len);
    os_err_t (*get_cfun)(mo_object_t *self, os_uint8_t *fun_lvl);
    os_err_t (*set_cfun)(mo_object_t *self, os_uint8_t fun_lvl);
    os_err_t (*get_firmware_version)(mo_object_t *self, mo_firmware_version_t *version);
    os_err_t (*sleep_mode_set)(mo_object_t *self, os_uint8_t fun_lvl);
    os_err_t (*get_eid)(mo_object_t *self, char *eid, os_size_t len);
    os_err_t (*gm_time)(mo_object_t *self, struct tm *l_tm);
} mo_general_ops_t;

os_err_t mo_at_test(mo_object_t *self);
os_err_t mo_get_imei(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_imsi(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_iccid(mo_object_t *self, char *value, os_size_t len);
os_err_t mo_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl);
os_err_t mo_set_cfun(mo_object_t *self, os_uint8_t fun_lvl);
os_err_t mo_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version);
void     mo_get_firmware_version_free(mo_firmware_version_t *version);
os_err_t mo_get_eid(mo_object_t *self, char *eid, os_size_t len);
os_err_t mo_gm_time(mo_object_t *self, struct tm *l_tm);
os_err_t mo_time(mo_object_t *self, time_t *timep);
os_err_t mo_sleep_mode_set(mo_object_t *self, os_uint8_t fun_lvl);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_GENERAL_OPS */

#endif /* __MO_GENERAL_H__ */
