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
 * @file        onepos_external_service.h
 *
 * @brief       onepos user interface
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-28   OneOS Team      First version.
 ***********************************************************************************************************************
 */
 
#ifndef __ONEPOS_EXTERNAL_SERVICE_H__
#define __ONEPOS_EXTERNAL_SERVICE_H__

#include <os_types.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    double lat;
    double lon;
}onepos_position_res_t;

extern os_err_t    onepos_ble_position_start(void);
extern os_err_t    onepos_ble_position_exit(void);
extern os_err_t    onepos_ble_position_get_res(onepos_position_res_t *res);
extern os_uint32_t onepos_ble_obtain_position_interval(void);
extern os_err_t    onepos_ble_position_change_position_interval(os_uint32_t position_interval);

#ifdef __cplusplus
}
#endif


#endif /*__ONEPOS_EXTERNAL_SERVICE_H__ */

