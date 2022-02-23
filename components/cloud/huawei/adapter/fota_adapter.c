/*
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
 * @file        fota_adapter.c
 *
 * @brief       huawei cloud sdk file "fota.c" adaptation, to do.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
/* stake: flag_manager.h */
#include "flag_manager.h"
int flag_init(flag_op_s *flag)
{
    return 0;
}

int flag_read(flag_type_e flag_type, void *buf, int32_t len)
{
    return 0;
}

int flag_write(flag_type_e flag_type, const void *buf, int32_t len)
{
    return 0;
}
/* end: flag_manager.h */

/* stake: upgrade_flag.h */
#include "upgrade_flag.h"
int flag_upgrade_init(void)
{
    return 0;
}

int flag_set_info(upgrade_type_e upgrade_type, uint32_t image_size)
{
    return 0;
}

void flag_get_info(upgrade_type_e  *upgrade_type,
                   uint32_t        *image_size,
                   uint32_t        *old_image_size,
                   upgrade_state_e *upgrade_state)
{
    return;
}

int flag_upgrade_set_result(upgrade_state_e state, uint32_t image_size)
{
    return 0;
}

int flag_upgrade_get_result(upgrade_state_e *state)
{
    return 0;
}

int flag_set_recover_verify(uint32_t recover_verify, uint32_t verify_length)
{
    return 0;
}

void flag_get_recover_verify(uint32_t *recover_verify, uint32_t *verify_length)
{
    return;
}

int flag_enable_hwpatch(const uint8_t *patch_head, int32_t patch_len)
{
    return 0;
}
/* end: upgrade_flag.h */

/* stake: package.h */
#include "ota/package.h"
/**
 *@ingroup agenttiny
 *@brief get storage device.
 *
 *@par Description:
 *This API is used to get storage device.
 *@attention none.
 *
 *@param none.
 *
 *@retval #pack_storage_device_api_s *     storage device.
 *@par Dependency: none.
 *@see none
 */
pack_storage_device_api_s *pack_get_device(void)
{
    return NULL;
}

/**
 *@ingroup agenttiny
 *@brief initiate storage device.
 *
 *@par Description:
 *This API is used to initiate storage device.
 *@attention none.
 *
 *@param ato_opt        [IN] Ota option.
 *
 *@retval #int          0 if succeed, or error.
 *@par Dependency: none.
 *@see none
 */
int pack_init_device(const pack_params_s *params)
{
    return 0;
}
/* end: flag_manager.h */
