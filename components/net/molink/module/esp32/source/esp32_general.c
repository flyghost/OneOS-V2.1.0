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
 * @file        esop8266_general.c
 *
 * @brief       esp32 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp32_general.h"
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "esp32.general"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef ESP32_USING_GENERAL_OPS

os_err_t esp32_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t esp32_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GMR");
    if (result != OS_EOK)
    {
        return result;
    }

    version->ver_info = os_calloc(resp.line_counts - 2, sizeof(char *));
    if (version->ver_info == OS_NULL)
    {
        return OS_ENOMEM;
    }

    version->line_counts = 0;

    for (int i = 1; i <= resp.line_counts - 2; i++)
    {
        const char *source_line = at_resp_get_line(&resp, i);
        os_size_t   line_length = strlen(source_line);

        char **dest_line = &version->ver_info[version->line_counts];

        *dest_line = os_calloc(1, line_length + 1);
        if (OS_NULL == *dest_line)
        {
            mo_get_firmware_version_free(version);
            return OS_ENOMEM;
        }

        strncpy(*dest_line, source_line, line_length);
        version->line_counts ++;
    }

    return OS_EOK;
}

#endif /* ESP32_USING_GENERAL_OPS */
