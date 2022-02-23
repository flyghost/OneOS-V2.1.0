// Copyright (C) Firmwave Ltd., All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <oneos_config.h>
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#define DBG_EXT_TAG "baiduiot"
#include <dlog.h>

int platform_init(void)
{
    // TODO Add proper network events synchronization
    ThreadAPI_Sleep(100);
    return 0;
}

#ifdef BAIDUIOT_CLOUD_WITH_TLS
const IO_INTERFACE_DESCRIPTION *platform_get_default_tlsio(void)
{
    return (IO_INTERFACE_DESCRIPTION *)tlsio_mbedtls_get_interface_description();
}
#else
const IO_INTERFACE_DESCRIPTION *platform_get_default_tlsio(void)
{
    return (IO_INTERFACE_DESCRIPTION *)NULL;
}
#endif

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; freertos; undefined)");
}

void platform_deinit(void)
{
    LOG_I(DBG_EXT_TAG, "Deinitializing platform \r\n");
    while (1)
    {
    };
}
