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
 * @file        os_version.h
 *
 * @brief       Query OneOS version and kernel version.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_version.h>
#include <os_errno.h>
#include <os_types.h>
#include <os_util.h>
#include <shell.h>

#ifdef OS_USING_SHELL
static os_err_t sh_query_oneos_version(os_int32_t argc, char **argv)
{
    os_kprintf("\r\n%s\r\n\r\n", ONEOS_VERSION);

    return OS_EOK;
}
SH_CMD_EXPORT(version, sh_query_oneos_version, "show oneos version");

#endif /* OS_USING_SHELL */

