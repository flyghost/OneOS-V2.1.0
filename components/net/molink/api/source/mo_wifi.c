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
 * @file        mo_wifi.c
 *
 * @brief       module link kit wifi api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_wifi.h"

#include <stdio.h>
#include <stdlib.h>

#define MO_LOG_TAG "molink.wifi"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_WIFI_OPS

static mo_wifi_ops_t *get_wifi_ops(mo_object_t *self)
{
    mo_wifi_ops_t *ops = (mo_wifi_ops_t *)self->ops_table[MODULE_OPS_WIFI];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support wifi operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set module wifi mode
 *
 * @param[in]       module          The descriptor of molink module instance
 * @param[in]       mode            The wifi mode @see mo_wifi_mode_t
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module functionality level successfully
 * @retval          OS_ERROR        Set module functionality level error
 * @retval          OS_ETIMEOUT     Set module functionality level timeout
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(mode > MO_WIFI_MODE_NULL && mode < MO_WIFI_MODE_MAX);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_mode)
    {
        ERROR("Module %s does not support set wifi module mode operate", module->name);
        return OS_ERROR;
    }

    return ops->set_mode(module, mode);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module wifi mode
 *
 * @param[in]       module          The descriptor of molink module instance
 * 
 * @return          On success, return wifi mode; on error, return MO_WIFI_MODE_NULL. @see mo_wifi_mode_t
 ***********************************************************************************************************************
 */
mo_wifi_mode_t mo_wifi_get_mode(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return MO_WIFI_MODE_NULL;
    }

    if (OS_NULL == ops->get_mode)
    {
        ERROR("Module %s does not support get wifi module mode operate", module->name);
        return MO_WIFI_MODE_NULL;
    }

    return ops->get_mode(module);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module wifi state
 *
 * @param[in]       module          The descriptor of molink module instance
 * 
 * @return          On success, return wifi state; on error, return MO_WIFI_STAT_NULL. @see mo_wifi_stat_t
 ***********************************************************************************************************************
 */
mo_wifi_stat_t mo_wifi_get_stat(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return MO_WIFI_STAT_NULL;
    }

    if (OS_NULL == ops->get_stat)
    {
        ERROR("Module %s does not support get wifi module state operate", module->name);
        return MO_WIFI_STAT_NULL;
    }

    return ops->get_stat(module);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command get sta cip infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[out]      ip                The ip to store
 * @param[out]      gw                The gw to store
 * @param[out]      mask              The mask to store
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             get sta cip infomation successfully
 * @retval          OS_ERROR           get sta cip infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_get_sta_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask, ip_addr_t *ip6_ll, ip_addr_t *ip6_gl)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_sta_cip)
    {
        ERROR("Module %s does not support get sta cip ", module->name);
        return OS_ERROR;
    }

    return ops->get_sta_cip(module, ip, gw, mask, ip6_ll, ip6_gl);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command get ap cip infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[in]       ip                The ip to set
 * @param[in]       gw                The gw to set
 * @param[in]       mask              The mask to set
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             get ap cip infomation successfully
 * @retval          OS_ERROR           get ap cip infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_set_ap_cip(mo_object_t *module, char *ip, char *gw, char *mask)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != ip);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_ap_cip)
    {
        ERROR("Module %s does not support get ap cip ", module->name);
        return OS_ERROR;
    }

    return ops->set_ap_cip(module, ip, gw, mask);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command get ap cip infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[out]      ip                The ip to store
 * @param[out]      gw                The gw to store
 * @param[out]      mask              The mask to store
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             get ap cip infomation successfully
 * @retval          OS_ERROR           get ap cip infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_get_ap_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != ip);
    OS_ASSERT(OS_NULL != gw);
    OS_ASSERT(OS_NULL != mask);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_ap_cip)
    {
        ERROR("Module %s does not support get ap cip ", module->name);
        return OS_ERROR;
    }

    return ops->get_ap_cip(module, ip, gw, mask);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command get sta mac infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[out]      mac               The mac to store
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             get sta mac infomation successfully
 * @retval          OS_ERROR           get sta mac infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_get_sta_mac(mo_object_t *module, char mac[])
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != mac);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_sta_mac)
    {
        ERROR("Module %s does not support get sta mac", module->name);
        return OS_ERROR;
    }

    return ops->get_sta_mac(module, mac);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command get ap mac infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[out]      mac               The mac to store
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             get ap mac infomation successfully
 * @retval          OS_ERROR           get ap mac infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_get_ap_mac(mo_object_t *module, char mac[])
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != mac);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_ap_mac)
    {
        ERROR("Module %s does not support get ap mac", module->name);
        return OS_ERROR;
    }

    return ops->get_ap_mac(module, mac);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to scan ap infomation
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[in]       ssid              The ssid of ap to scan, if ssid is NULL, scan all available ap
 * @param[out]      scan_result       The buffer to store scan ap infomation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             Scan infomation successfully
 * @retval          OS_ETIMEOUT        Scan infomation timeout
 * @retval          OS_ERROR           Scan infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != scan_result);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->scan_info)
    {
        ERROR("Module %s does not support scan wifi ap info operate", module->name);
        return OS_ERROR;
    }

    return ops->scan_info(module, ssid, scan_result);
}

/**
 ***********************************************************************************************************************
 * @brief           Free scan ap infomation
 *
 * @param[in]       scan_result          The buffer that holds the scan information
 ***********************************************************************************************************************
 */
void mo_wifi_scan_info_free(mo_wifi_scan_result_t *scan_result)
{
    OS_ASSERT(OS_NULL != scan_result);

    os_free(scan_result->info_array);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to connect to ap 
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[in]       ssid              The ssid of ap to connect
 * @param[in]       password          The password of ap to connect
 * 
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK             Connect successfully
 * @retval          OS_ETIMEOUT        Connect timeout
 * @retval          OS_ERROR           Connect error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != ssid);
    //OS_ASSERT(OS_NULL != password);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->connect_ap)
    {
        ERROR("Module %s does not support connect wifi ap operate", module->name);
        return OS_ERROR;
    }

    return ops->connect_ap(module, ssid, password);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to disconnect to ap 
 *
 * @param[in]       module            The descriptor of molink module instance
 * 
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK             Disconnect successfully
 * @retval          OS_ETIMEOUT        Disconnect timeout
 * @retval          OS_ERROR           Disconnect error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_disconnect_ap(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->disconnect_ap)
    {
        ERROR("Module %s does not support disconnect wifi ap operate", module->name);
        return OS_ERROR;
    }

    return ops->disconnect_ap(module);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to start ap 
 *
 * @param[in]       module            The descriptor of molink module instance
 * @param[in]       ssid              The ssid of ap to start
 * @param[in]       password          The password of ap to start
 * @param[in]       channel           The channel of ap to start
 * @param[in]       ecn               The encryption method of ap to start
 * 
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK             start successfully
 * @retval          OS_ETIMEOUT        start timeout
 * @retval          OS_ERROR           start error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_start_ap(mo_object_t *module, const char *ssid, const char *password, os_uint8_t channel, os_uint8_t ecn)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != ssid);
    OS_ASSERT(OS_NULL != password);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->start_ap)
    {
        ERROR("Module %s does not support start ap operate", module->name);
        return OS_ERROR;
    }

    return ops->start_ap(module, ssid, password, channel, ecn);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to stop ap 
 *
 * @param[in]       module            The descriptor of molink module instance
 * 
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK             stop successfully
 * @retval          OS_ETIMEOUT        stop timeout
 * @retval          OS_ERROR           stop error
 ***********************************************************************************************************************
 */
os_err_t mo_wifi_stop_ap(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_wifi_ops_t *ops = get_wifi_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->stop_ap)
    {
        ERROR("Module %s does not support stop ap operate", module->name);
        return OS_ERROR;
    }

    return ops->stop_ap(module);
}

#endif /* MOLINK_USING_WIFI_OPS */
