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
 * @file        cmiot_self.c
 *
 * @brief       Implement self functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "cmiot_type.h"
#include "cmiot_config.h"
#include "cmiot_user.h"
#include "cmiot_client.h"

cmiot_uint16 cmiot_get_recieve_buf_len()
{
    return CMIOT_RECIEVE_BUF_MAXLEN;
}

cmiot_uint16 cmiot_get_send_buf_len(void)
{
    return CMIOT_SEND_BUF_MAXLEN;
}

cmiot_uint8 cmiot_get_default_protocol(void)
{
    return CMIOT_DEFAULT_NETWORK_PROTOCOL;
}

cmiot_char *cmiot_get_manufacturer(void)
{
    return CMIOT_FOTA_SERVICE_OEM;
}

cmiot_char *cmiot_get_model_number(void)
{
    return CMIOT_FOTA_SERVICE_MODEL;
}

cmiot_char *cmiot_get_product_id(void)
{
    return CMIOT_FOTA_SERVICE_PRODUCT_ID;
}

cmiot_char *cmiot_get_product_sec(void)
{
    return CMIOT_FOTA_SERVICE_PRODUCT_SEC;
}

cmiot_char *cmiot_get_device_type(void)
{
    return CMIOT_FOTA_SERVICE_DEVICE_TYPE;
}

cmiot_char *cmiot_get_platform(void)
{
    return CMIOT_FOTA_SERVICE_PLATFORM;
}

cmiot_char *cmiot_get_apk_version(void)
{
    return CMIOT_FOTA_OS_VERSION;
}

cmiot_char *cmiot_get_firmware_version(void)
{
    return CMIOT_FIRMWARE_VERSION;
}

cmiot_uint8 cmiot_get_default_segment_size(void)
{
    return CMIOT_DEFAULT_SEGMENT_SIZE_INDEX;
}

cmiot_int8 cmiot_upgrade(void)
{
#if CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_COAP
    return cmiot_upgrade_coap();
#else
    return cmiot_upgrade_http();
#endif
}

cmiot_int8 cmiot_report_upgrade(void)
{
#if CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_COAP
    return cmiot_report_upgrade_coap();
#else
    return cmiot_report_upgrade_http();
#endif
}
