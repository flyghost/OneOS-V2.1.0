/**
***********************************************************************************************************************
* Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
* @file        cms_id_atest.c
*
* @brief       This is mqtt test file based atest.
*
* @revision
* Date         Author          Notes
* 2020-10-13   OneOS Team      First Version
***********************************************************************************************************************
*/
#include <shell.h>
#include <atest.h>
#include <string.h>
#include <stdio.h>
#include "oneos_config.h"

#ifdef CMS_USING_ID
#include "cms_id.h"
static void hexdump(const char *tag, size_t width, const unsigned char *buf, size_t size)
{
    printf("%s\r\n", tag);
    int i = 0;
    for (; i < size;)
    {
        printf("%02x ", buf[i]);
        ++i;
        if (i % width == 0)
            printf("\r\n");
    }
    if (i % width)
        printf("\r\n");
}

static void strdump(const char *tag, size_t width, const unsigned char *buf, size_t size)
{
    printf("%s\r\n", tag);
    int i = 0;
    for (; i < size;)
    {
        printf("%c", buf[i]);
        ++i;
        if (i % width == 0)
            printf("\r\n");
    }
    if (i % width)
        printf("\r\n");
}

static void atest_cms_id_get_id(void)
{
    size_t len = 0;
    CMS_TYPE_T TYPE = PID;
    char *name = NULL;
    uint8_t id[128];
    for (; TYPE <= CWT_PLATFORM; TYPE++)
    {
        cms_id_get_id_len(TYPE, &len);
        cms_id_get_id(TYPE, id, len);

        switch (TYPE)
        {
        case PID:
            name = "PID";
            break;
        case OID:
            name = "OID";
            break;
        case DID:
            name = "DID";
            break;
        case CTI_DEVICE:
            name = "CTI_DEVICE";
            break;
        case CTI_PLATFORM:
            name = "CTI_PLATFORM";
            break;
        case CWT_DEVICE:
            name = "CWT_DEVICE";
            break;
        case CWT_PLATFORM:
            name = "CWT_PLATFORM";
            break;
        default:
            break;
        }
        if (TYPE > DID)
            hexdump(name, 16, id, len);
        else
            strdump(name, 64, id, len);
        memset(id, 0, sizeof(id));
    }
}

static void atest_cms_id_regist_check(void)
{
    int rc = CMS_ERROR_GENERIC_ERROR;
    rc = cms_id_regist_check();
    if (rc != CMS_ERROR_SUCCESS)
    {
        printf("Device is don't regist.\r\n");
    }
    else
    {
        printf("Device has registed.\r\n");
    }
}

static void atest_cms_id_regist(void)
{
    cms_id_regist();
}

static void atest_cms_id_auth_check(void)
{
    int rc = CMS_ERROR_GENERIC_ERROR;
    rc = cms_id_auth_check();
    if (rc != CMS_ERROR_SUCCESS)
    {
        printf("Device is don't auth.\r\n");
    }
    else
    {
        printf("Device was authed.\r\n");
    }
}

static void atest_cms_id_auth(void)
{
    cms_id_authorize();
}

static void cms_id_all(void)
{
    ATEST_UNIT_RUN(atest_cms_id_regist_check);
    ATEST_UNIT_RUN(atest_cms_id_auth_check);

    ATEST_UNIT_RUN(atest_cms_id_regist);
    ATEST_UNIT_RUN(atest_cms_id_regist_check);

    ATEST_UNIT_RUN(atest_cms_id_auth);
    ATEST_UNIT_RUN(atest_cms_id_auth_check);

    ATEST_UNIT_RUN(atest_cms_id_get_id);
}

ATEST_TC_EXPORT(cms.id.all, cms_id_all, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.id.getid, atest_cms_id_get_id, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.id.check.auth, atest_cms_id_auth_check, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.id.auth, atest_cms_id_auth, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.id.regist, atest_cms_id_regist, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.id.check.regist, atest_cms_id_regist_check, NULL, NULL, TC_PRIORITY_LOW);
#endif

/* just use for debug code. */
//#define CMS_ID_DEBUG
#ifdef CMS_ID_DEBUG
#include "cms_id_internal.h"
static void cms_id_debug(void)
{
    uint8_t cti[16] = {0};
    memset(cti, 3, 16);
    uint8_t oid[] = "1.2.156.113753.011P9sMHSB1P000000000032";

    cms_id_init();
    cms_id_set_value(CTI_PLATFORM, cti, 16);
    cms_id_gene_cwt();

    cms_id_set_value(OID, oid, sizeof(oid));
    atest_cms_id_get_id();

    cms_id_kdf_master(cti, 16);
    memset(cti, 0, 16);
    cms_id_get_value(KEY_MASTER, cti, 16);

    hexdump("key_diff", 64, cti, 16);
}
ATEST_TC_EXPORT(cms.id.debug, cms_id_debug, NULL, NULL, TC_PRIORITY_LOW);
#endif
