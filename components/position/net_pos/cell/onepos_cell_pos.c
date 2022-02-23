/**
 ***********************************************************************************************************************
 * Copyright (c)2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        onepos_cell_pos.c
 *
 * @brief       cell^s location of onepos
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_assert.h>
#include "onepos_cell_pos.h"

#define ONEPOS_LOG_TAG "onepos.cell"
#define ONEPOS_LOG_LVL ONEPOS_LOG_INFO
#include <onepos_log.h>

static mo_object_t *cell_module = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           clean onepos cell info
 *
 * @param[in]       onepos_cell_info       opepos cell info
 ***********************************************************************************************************************
 */
static void onepos_cell_info_clean(onepos_cell_info_t *onepos_cell_info)
{
    os_free(onepos_cell_info->cell_info);
    onepos_cell_info->cell_num  = 0;
    onepos_cell_info->cell_info = OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           creat onepos cell loca publish message
 *
 * @param[in]       onepos_cell_info        scanned cell information
 * @param[in]       json_src                json data for add
 *
 * @return          json format data
 * @retval          os_err_t                error code
 ***********************************************************************************************************************
 */
static os_err_t onepos_cell_loca_pub_msg(onepos_cell_info_t *onepos_cell_info, cJSON* json_src) 
{
    char   		*cell_list_str = OS_NULL;
    os_uint32_t  cell_list_len = 0;
    os_uint32_t  index         = 0;
    os_uint32_t  i             = 0;

    if ((OS_NULL == onepos_cell_info) || (0 == onepos_cell_info->cell_num) || OS_NULL == json_src)
    {
        ONEPOS_LOG_E("input param is error");
        return OS_ERROR;
    }
    cell_list_len = (onepos_cell_info->cell_num * ONEPOS_CELL_INFO_LEN)
                    + (onepos_cell_info->cell_num - 1) * strlen(ONEPOS_MSG_SEPARATOR) + 1;
    cell_list_str = (char *)os_malloc(cell_list_len);
    if(OS_NULL != cell_list_str)
    {
        for (i = 0; i < (onepos_cell_info->cell_num); i++)
        {
            index += os_snprintf((cell_list_str + index), (cell_list_len - index),
                               ONEPOS_CELL_MSG_FORMAT,
                               onepos_cell_info->cell_info[i].mcc,
                               onepos_cell_info->cell_info[i].mnc,
                               onepos_cell_info->cell_info[i].lac,
                               onepos_cell_info->cell_info[i].cid,
                               onepos_cell_info->cell_info[i].ss);
            index += os_snprintf((cell_list_str + index), (cell_list_len - index), "%s",
                               ONEPOS_MSG_SEPARATOR);
        }
        os_snprintf(
            (cell_list_str + index), (cell_list_len - index), ONEPOS_CELL_MSG_FORMAT,
            onepos_cell_info->cell_info[i].mcc, onepos_cell_info->cell_info[i].mnc,
            onepos_cell_info->cell_info[i].lac, onepos_cell_info->cell_info[i].cid,
            onepos_cell_info->cell_info[i].ss);

        cJSON_AddItemToObject(json_src, "cell_list",
                              cJSON_CreateString(cell_list_str));
        cJSON_AddItemToObject(json_src, "net_type",
                              cJSON_CreateNumber(onepos_cell_info->net_type));

        os_free(cell_list_str);
        return OS_EOK;
    }
    else
    {
        ONEPOS_LOG_E("malloc onepos cell massage is NULL");
        return OS_ENOMEM;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           creat pulish message to cell positioning
 *
 * @param[in]       json_src                json data for add
 *
 * @return          os_err_t                error code
 ***********************************************************************************************************************
 */
os_err_t cell_pos_pub_msg(cJSON* json_src)
{
    os_err_t           ret              = OS_EOK;
    onepos_cell_info_t onepos_cell_info = {0,};

    if (OS_EOK == mo_get_cell_info(cell_module, &onepos_cell_info))
    {
        ret = onepos_cell_loca_pub_msg(&onepos_cell_info, json_src);
        onepos_cell_info_clean(&onepos_cell_info);
    }
    else
    {
        ONEPOS_LOG_E("get cell info is ERROR!");
        ret = OS_EIO;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           init cell module to work
 *
 * @return          os_err_t
 * @retval          OS_EOK         init cell modules is successful
 * @retval          OS_ERROR       init cell modules is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_init_cell_dev(void)
{
    cell_module = mo_object_get_by_name(ONEPOS_CELL_DEVICE_NAME);
    /* Check param */
    if (OS_NULL == cell_module)
    {
        ONEPOS_LOG_E("onepos device : %s is error", ONEPOS_CELL_DEVICE_NAME);
        return OS_ERROR;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           get cell module status
 *
 * @return          os_bool_t
 * @retval          OS_TRUE         cell is OK
 * @retval          OS_ERROR        cell is not ready
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_cell_sta(void)
{
    char ip[30] = {0,};

    if (OS_EOK != mo_get_ipaddr(cell_module, ip))
    {
        return OS_FALSE;
    }
    else
    {
        return OS_TRUE;
    }
}
