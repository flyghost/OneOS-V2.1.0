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
 * @file        mo_common.c
 *
 * @brief       module link kit common api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_common.h"

/**
 ***********************************************************************************************************************
 * @brief           Get molink module instance by name.
 *
 * @param[in]       name            The name of molink module instance
 *
 * @return          On success, return a molink module instance descriptor; on error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_object_t *mo_get_by_name(const char *name)
{
    return mo_object_get_by_name(name);
}

/**
 ***********************************************************************************************************************
 * @brief           Get default molink module instance.
 *
 * @return          On success, return a molink module instance descriptor; on error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_object_t *mo_get_default(void)
{
    return mo_object_get_default();
}

/**
 ***********************************************************************************************************************
 * @brief           Set default molink module instance.
 *
 * @param[in]       self            The molink module instance descriptor
 ***********************************************************************************************************************
 */
void mo_set_default(mo_object_t *self)
{
    mo_object_set_default(self);
}
