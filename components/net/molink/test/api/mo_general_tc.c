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
 * @file        mo_general_tc.c
 *
 * @brief       module link kit general api test case.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <atest.h>
#include <shell.h>
#include <serial.h>
#include <mo_api.h>
#include <os_task.h>
#include "oneos_config.h"
#include "mo_common.h"
#include <string.h>
#include <shell.h>

#define MO_LOG_TAG "molink.general.tc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

static mo_object_t *test_module = OS_NULL;

static void test_mo_at_test(void)
{
    os_err_t ret = mo_at_test(test_module);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module %s AT test OK", test_module->name);
    }
    else
    {
        ERROR("module %s AT test fail", test_module->name);
    }
}

static void test_mo_get_imei(void)
{
    char imei[16] = {0};

    os_err_t ret = mo_get_imei(test_module, imei, sizeof(imei));
    tp_assert_true(OS_EOK == ret);
    tp_assert_true(15 == strlen(imei));
    if (OS_EOK == ret)
    {
        INFO("module %s get imei OK, imei: %s, len: %u", test_module->name, imei, strlen(imei));
    }
    else
    {
        ERROR("module %s get imei fail", test_module->name);
    }

    return;
}

static void test_mo_get_imsi(void)
{
    char imsi[16] = {0};

    os_err_t ret = mo_get_imsi(test_module, imsi, sizeof(imsi));
    tp_assert_true(OS_EOK == ret);
    tp_assert_true(15 == strlen(imsi));
    if (OS_EOK == ret)
    {
        INFO("module %s get imsi OK, imsi: %s, len: %u", test_module->name, imsi, strlen(imsi));
    }
    else
    {
        ERROR("module %s get imsi fail", test_module->name);
    }

    return;
}

static void test_mo_get_iccid(void)
{
    char iccid[21] = {0};

    os_err_t ret = mo_get_iccid(test_module, iccid, sizeof(iccid));
    tp_assert_true(OS_EOK == ret);
    tp_assert_true(20 == strlen(iccid));

    if (OS_EOK == ret)
    {
        INFO("module %s get iccid OK, iccid: %s, len: %u", test_module->name, iccid, strlen(iccid));
    }
    else
    {
        ERROR("module %s get iccid fail", test_module->name);
    }

    return;
}

static void test_mo_get_cfun(os_uint8_t *cfun)
{
    OS_ASSERT(cfun != OS_NULL);

    os_err_t ret = mo_get_cfun(test_module, cfun);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module %s get cfun OK, cfun status: %u", test_module->name, *cfun);
    }
    else
    {
        ERROR("module %s test mo_get_cfun fail", test_module->name);
    }

    return;    
}

static os_err_t test_mo_set_cfun(os_uint8_t cfun)
{
    os_err_t ret = mo_set_cfun(test_module, cfun);

    if (OS_EOK == ret)
    {
        INFO("module %s set cfun to %d success", test_module->name, cfun);
    }
    else
    {
        ERROR("module %s set cfun to %d fail", test_module->name, cfun);
    }

    return ret;
}

static void test_mo_set_and_get_cfun(void)
{
    os_err_t ret = OS_ERROR;

    /* Test set minimum functionality */
    ret = test_mo_set_cfun(0);
    tp_assert_true(OS_EOK == ret);

    os_task_tsleep(10 * OS_TICK_PER_SECOND);

    /* Test set full functionality */
    ret = test_mo_set_cfun(1);
    tp_assert_true(OS_EOK == ret);

    /* Now the level of functionality should be full functionality */
    os_uint8_t cfun = 0;
    test_mo_get_cfun(&cfun);
    tp_assert_true(1 == cfun);

    return;
}

static void test_mo_get_firmware_version(void)
{
    mo_firmware_version_t version = {0};

    os_err_t ret = mo_get_firmware_version(test_module, &version);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module %s get firmware version infomation success", test_module->name);

        for (int i = 0; i < version.line_counts; i++)
        {
            os_kprintf("%s\r\n", version.ver_info[i]);
        }
    }
    else
    {
        ERROR("module %s get firmware version infomation fail", test_module->name);
    }

    mo_get_firmware_version_free(&version);
}

static void test_case(void)
{
    mo_general_ops_t *ops = (mo_general_ops_t *)test_module->ops_table[MODULE_OPS_GENERAL];

    if (ops->at_test != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_at_test);
    }
    
    if (ops->get_imei != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_get_imei);
    }

    if (ops->get_imsi != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_get_imsi);
    }

    if (ops->get_iccid != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_get_iccid);
    }

    if (ops->set_cfun != OS_NULL && ops->get_cfun != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_set_and_get_cfun);
    }

    if (ops->get_firmware_version != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_get_firmware_version);
    }
    
}

static os_err_t test_init(void)
{
    test_module = mo_get_default();

    if (OS_NULL == test_module)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t test_cleanup(void)
{
    os_task_msleep(100);

    return OS_EOK;
}

ATEST_TC_EXPORT(components.net.molink.api.general.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);

#ifdef MOLINK_USING_AT_BASIC_TC

os_err_t at_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        os_kprintf("Usage:at AT[+CMD]\r\n");
        return OS_EOK;
    }

    mo_object_t *test_module = mo_get_default();
    if (OS_NULL == test_module)
    {
        os_kprintf("No default module!\r\n");
        return OS_EOK;
    }

    at_parser_t *parser = &test_module->parser;

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, argv[1]);

    os_kprintf("%s AT exec %s %s\r\n", __func__, argv[1], result == OS_EOK ? "success" : "failed");

    return OS_EOK;
}
SH_CMD_EXPORT(at, at_sh, "AT TEST");

#endif /* MOLINK_USING_AT_BASIC_TC */
