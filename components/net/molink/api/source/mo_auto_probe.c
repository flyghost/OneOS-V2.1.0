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
 * @file        mo_auto_probe.c
 *
 * @brief       module link kit common api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_auto_probe.h"
#include <os_types.h>
#include <os_errno.h>
#include <serial.h>
#include <string.h>
#include <os_memory.h>
#include "at_parser.h"
#include "mo_object.h"

#ifdef MOLINK_USING_AUTO_PROBE_OPS

#define MO_LOG_TAG "molink.auto_probe"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define MO_AUTO_PROBE_PARSER_NAME "CONNECT_PARSER"
#define MO_AUTO_PROBE_PARSER_BUFF_LEN (256)
#define MO_AUTO_PROBE_RETRY_TIME_DFT  (5)

static mo_auto_probe_info_t *s_mo_auto_probe_tab_entry_p;
static int                   s_mo_auto_probe_tab_num;

#if defined(__ICCARM__) || defined(__ICCRX__)           /* For IAR compiler */
#pragma section="MolinkTab"
#endif

extern mo_object_t  *module_object_get_by_device(os_device_t *device);
static void         _mo_auto_probe_parser_deinit(at_parser_t *parser);

static void _mo_auto_probe_init(void)
{
    /* Initialize the atest commands table.*/
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM C Compiler */
    extern const int MolinkTab$$Base;
    extern const int MolinkTab$$Limit;
    
    s_mo_auto_probe_tab_entry_p = (mo_auto_probe_info_t *)&MolinkTab$$Base;
    s_mo_auto_probe_tab_num     = (mo_auto_probe_info_t *)&MolinkTab$$Limit - s_mo_auto_probe_tab_entry_p;
    
#elif defined(__ICCARM__) || defined(__ICCRX__)         /* For IAR Compiler */
    s_mo_auto_probe_tab_entry_p = (mo_auto_probe_info_t *)__section_begin("MolinkTab");
    s_mo_auto_probe_tab_num     = (mo_auto_probe_info_t *)__section_end("MolinkTab") - s_mo_auto_probe_tab_entry_p;
    
#elif defined(__GNUC__)                                 /* For GCC Compiler */
    extern const int __molink_module_tab_start;
    extern const int __molink_module_tab_end;
    
    s_mo_auto_probe_tab_entry_p = (mo_auto_probe_info_t *)&__molink_module_tab_start;
    s_mo_auto_probe_tab_num     = (mo_auto_probe_info_t *)&__molink_module_tab_end - s_mo_auto_probe_tab_entry_p;
#else
#error "[mo_auto_probe] Compilier not support!!!"
#endif
    
    return;
}

static os_bool_t _mo_auto_probe_is_device_avaliable(mo_auto_probe_info_t *info_ptr)
{
    os_device_t *_device = os_device_find(info_ptr->device_name);
    if (OS_NULL == _device)
    {
        DEBUG("%s device %s not found!", info_ptr->name, info_ptr->device_name);
        return OS_FALSE;
    }
    
    if (OS_NULL != module_object_get_by_device(_device))
    {
        DEBUG("Device has been used.");
        return OS_FALSE;
    }

    return OS_TRUE;
}

static os_err_t _mo_auto_probe_device(mo_auto_probe_info_t *info_ptr, os_device_t **device)
{
    struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;
    uart_config.baud_rate = info_ptr->uart_baud_rate;

    DEBUG("%s device %s baud rate %d!\r\n", info_ptr->name, info_ptr->device_name, info_ptr->uart_baud_rate);

    os_device_t *_device = os_device_find(info_ptr->device_name);
    if (OS_NULL == _device)
    {
        DEBUG("%s device %s not found!", info_ptr->name, info_ptr->device_name);
        return OS_EEMPTY;
    }
    
    /* Temporary open device for bsp1.2, device control seem credible when device opening */
    if (OS_EOK != os_device_open(_device))
    {
        ERROR("Temporary open device failed!");
        return OS_ERROR;
    }

    if (OS_EOK != os_device_control(_device, OS_DEVICE_CTRL_CONFIG, &uart_config))
    {
        ERROR("Unexpect error occured!");
        os_device_close(_device);
        return OS_ERROR;
    }

    os_device_close(_device);

    *device = _device;
    return OS_EOK;
}

static at_parser_t* _mo_auto_probe_parser_init(os_device_t *device)
{
    os_err_t     result = OS_EOK;
    at_parser_t *parser = os_calloc(1, sizeof(at_parser_t));

    if (OS_NULL == parser)
    {
        ERROR("MoLink auto probe session: no enough memory!");
        return OS_NULL;
    }

    result = at_parser_init(parser, MO_AUTO_PROBE_PARSER_NAME, device, MO_AUTO_PROBE_PARSER_BUFF_LEN);
    if (OS_EOK != result)
    {
        ERROR("MoLink auto probe session: create parser failed!");
        os_free(parser);
        return OS_NULL;
    }

    result = at_parser_startup(parser);
    if (OS_EOK != result)
    {
        ERROR("MoLink auto probe session: parser task startup failed!");
        _mo_auto_probe_parser_deinit(parser);
        return OS_NULL;
    }

    return parser;
}

static void _mo_auto_probe_parser_deinit(at_parser_t *parser)
{
    at_parser_deinit(parser);
    os_free(parser);
    return;
}

static os_err_t _mo_auto_probe_check_identifier(mo_auto_probe_info_t *info_ptr, at_parser_t *parser)
{
    os_err_t  result = OS_EOK;
    char resp_buff[MO_AUTO_PROBE_PARSER_BUFF_LEN] = {0};
    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = OS_TICK_PER_SECOND};

    result = at_parser_connect(parser, MO_AUTO_PROBE_RETRY_TIME_DFT);
    if (OS_EOK != result)
    {
        DEBUG("MoLink auto probe session: module unreachable!");
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, info_ptr->at_cmd);
    if (OS_EOK != result)
    {
        DEBUG("MoLink auto probe session: cmd exec failed!");
        goto __exit;
    }
    
    if (OS_NULL == at_resp_get_line_by_kw(&resp, info_ptr->identifier))
    {
        DEBUG("MoLink auto probe session: not match %s!", info_ptr->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

static os_err_t _mo_module_identify_process(mo_auto_probe_info_t *info_ptr)
{
    os_err_t     result = OS_EOK;
    os_device_t *device = OS_NULL;
    at_parser_t *parser = OS_NULL;

    result = _mo_auto_probe_device(info_ptr, &device);
    if (OS_NULL == device) /* TODO OS_EBUSY->skip this device at autoprobe table */
    {
        return result;
    }

    parser = _mo_auto_probe_parser_init(device);
    if (OS_NULL == parser)
    {
        return OS_ERROR;
    }

    result = _mo_auto_probe_check_identifier(info_ptr, parser);
    if (OS_EOK == result)
    {
        INFO("MoLink auto probe module: %s", info_ptr->name);
    }

    _mo_auto_probe_parser_deinit(parser);

    return result;
}

static os_err_t _mo_auto_probe_process(void)
{
    os_err_t result = OS_EOK;
    
    for (int index = 0; index < s_mo_auto_probe_tab_num; index++)
    {
        OS_ASSERT(OS_NULL != s_mo_auto_probe_tab_entry_p[index].init_fn);
        
        os_kprintf("MoLink auto probe check: %s\r\n", s_mo_auto_probe_tab_entry_p[index].identifier);

        if (!_mo_auto_probe_is_device_avaliable(s_mo_auto_probe_tab_entry_p + index)) continue;

        if (OS_NULL != s_mo_auto_probe_tab_entry_p[index].misc_fn)
        {
            result = s_mo_auto_probe_tab_entry_p[index].misc_fn();
            if (OS_EOK != result) continue;
        }
        
        if (OS_EOK == _mo_module_identify_process(s_mo_auto_probe_tab_entry_p + index))
        {
            result = s_mo_auto_probe_tab_entry_p[index].init_fn();
            if (OS_EOK != result) return result;
        }
        else
        {
            continue;
        }
    }
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function used to trigger the module autoprobe session.
 *
 * @param[in]       void
 * 
 * @retval          OS_EOK          Ping successfully
 * @retval          OS_ETIMEOUT     Ping timeout
 * @retval          OS_ERROR        Ping error
 ***********************************************************************************************************************
 */
os_err_t mo_auto_probe(void)
{
    _mo_auto_probe_init();

    return _mo_auto_probe_process();
}
OS_CMPOENT_INIT(mo_auto_probe, OS_INIT_SUBLEVEL_LOW);

#endif /* MOLINK_USING_AUTO_PROBE_OPS */

