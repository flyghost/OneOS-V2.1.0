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
 * @file        mo_netserv_tc.c
 *
 * @brief       module link kit netserv api test case.
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

#include <string.h>

#define MO_LOG_TAG "molink.netserv.tc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

static mo_object_t *test_module = OS_NULL;

static void test_mo_get_attach(os_uint8_t *attach_stat)
{
    os_err_t ret = OS_ERROR;

    tp_assert_true(OS_NULL != attach_stat);
    if (OS_NULL == attach_stat)
    {
        ERROR("test mo_get_attach fail, attach_stat is NULL");
        return;
    }

    ret = mo_get_attach(test_module, attach_stat);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module get attach status OK, status: %u", *attach_stat);
    }
    else
    {
        ERROR("test get attach status fail");
    }

    return;    
}

static void test_mo_set_and_get_attach(void)
{    
    os_err_t   ret         = OS_ERROR;
    os_uint8_t attach_stat = 0;

    for (int i = 0; i < 2; i++)
    {
        ret = mo_set_attach(test_module, i);
        tp_assert_true(OS_EOK == ret);

        if (OS_EOK == ret)
        {
            INFO("module set attach status to %d success", i);
        }
        else
        {
            ERROR("module set attach status to %d fail", i);
        }

        os_task_msleep(10000);

        test_mo_get_attach(&attach_stat);

        tp_assert_true(i == attach_stat);
    }

    return;
}

static void test_mo_get_reg_status(eps_reg_info_t *info)
{
    os_err_t ret = OS_ERROR;

    tp_assert_true(OS_NULL != info);
    if (OS_NULL == info)
    {
        ERROR("test mo_get_reg fail, info ptr is NULL");
        return;
    }
    ret = mo_get_reg(test_module, info);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module get network reg status OK, reg_n: %u, status: %u", info->reg_n, info->reg_stat);
    }
    else
    {
        ERROR("test get attach status fail");
    }

    return;    
}

static void test_mo_set_and_get_reg(void)
{    
    os_err_t       ret     = OS_ERROR;
    os_uint8_t     reg_bak = 0;
    eps_reg_info_t info;

    test_mo_get_reg_status(&info);
    reg_bak = info.reg_n;

    for (int i = 0; i < 6; i++)
    {
        ret = mo_set_reg(test_module, i);
        tp_assert_true(OS_EOK == ret);
        if (OS_EOK == ret)
        {
            INFO("module set network reg_n to %d success", i);
        }
        else
        {
            ERROR("module set network reg_n to %d fail", i);
        }

        os_task_msleep(3000);
        test_mo_get_reg_status(&info);
        tp_assert_true(i == info.reg_n);
        /* Network status should not be affected */
        tp_assert_true(1 == info.reg_stat);
    }

    ret = mo_set_reg(test_module, reg_bak);
    tp_assert_true(OS_EOK == ret);
    if (OS_EOK == ret)
    {
        INFO("module set network reg_n to %u success", reg_bak);
    }
    else
    {
        ERROR("module set network reg_n to %u fail", reg_bak);
    }

    os_task_msleep(3000);
    test_mo_get_reg_status(&info);
    tp_assert_true(reg_bak == info.reg_n);
    tp_assert_true(1 == info.reg_stat);

    return;
}

static void test_mo_get_cgact(os_uint8_t *cid, os_uint8_t *act_stat)
{
    os_err_t ret = OS_ERROR;

    tp_assert_true((OS_NULL != cid) && (OS_NULL != act_stat));
    if ((OS_NULL == cid) || (OS_NULL == act_stat))
    {
        ERROR("test mo_get_cgact fail, input ptr is NULL");
        return;
    }
    ret = mo_get_cgact(test_module, cid, act_stat);
    tp_assert_true(OS_EOK == ret);

    if (OS_EOK == ret)
    {
        INFO("module get cgact status OK, cid: %u, act_stat: %u", *cid, *act_stat);
    }
    else
    {
        INFO("test get cgact status fail");
    }

    return;    
}

static void test_mo_set_and_get_cgact(void)
{    
    os_err_t   ret      = OS_ERROR;
    os_uint8_t cid      = 0;
    os_uint8_t act_stat = 0;

    test_mo_get_cgact(&cid, &act_stat);
    for (int i = 0; i < 2; i++)
    {
        ret = mo_set_cgact(test_module, cid, i);
        tp_assert_true(OS_EOK == ret);
        if (OS_EOK == ret)
        {
            INFO("module set cgact status to %d success", i);
        }
        else
        {
            ERROR("module set cgact status to %d fail", i);
        }

        os_task_msleep(5000);
        test_mo_get_cgact(&cid, &act_stat);
        tp_assert_true(i == act_stat);
    }

    return;
}

static void test_mo_get_csq(void)
{
    os_err_t   ret  = OS_ERROR;
    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;
    
    ret = mo_get_csq(test_module, &rssi, &ber);
    tp_assert_true(OS_EOK == ret);
    tp_assert_true(0 < rssi);

    if (OS_EOK == ret)
    {
        INFO("Module get csq OK, rssi: %u, ber: %u", rssi, ber);
    }
    else
    {
        ERROR("Module get csq fail");
    }

    return; 
}

static void test_case(void)
{
    mo_netserv_ops_t *ops = (mo_netserv_ops_t *)test_module->ops_table[MODULE_OPS_NETSERV];

    if (ops->set_attach != OS_NULL && ops->get_attach != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_set_and_get_attach);
    }

    if (ops->set_reg != OS_NULL && ops->get_reg != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_set_and_get_reg);
    }

    if (ops->set_cgact != OS_NULL && ops->get_cgact != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_set_and_get_cgact);
    }

    if (ops->get_csq != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_get_csq);
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

ATEST_TC_EXPORT(components.net.molink.api.netserv.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);
