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
 * @file        ecoredump_tc.c
 *
 * @brief       ecoredump test case.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <atest.h>
#include <shell.h>
#include <serial.h>

#include <string.h>

#define DBG_EXT_TAG "molink.gerenal.tc"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#include "ecoredump.h"

static uint32_t init_ret_length;
static uint32_t log_out_length;

static void ecd_log_out(uint8_t *data, int len)
{
    log_out_length += len;
}

static void test_ecd_length_with_fp(void)
{
    struct thread_info_ops ops;
    log_out_length = 0;
    ecd_init(1, ecd_log_out);
    ecd_rtos_thread_ops(&ops);
    init_ret_length = ecd_corefile_size(&ops);
    ecd_gen_coredump(&ops);
    tp_assert_integer_equal(init_ret_length, log_out_length);
}

static void test_ecd_length_no_fp(void)
{
    struct thread_info_ops ops;
    log_out_length = 0;
    ecd_init(0, ecd_log_out);
    ecd_rtos_thread_ops(&ops);
    init_ret_length = ecd_corefile_size(&ops);
    ecd_gen_coredump(&ops);
    tp_assert_integer_equal(init_ret_length, log_out_length);
}

static void test_case(void)
{
    ATEST_UNIT_RUN(test_ecd_length_with_fp);

    ATEST_UNIT_RUN(test_ecd_length_no_fp);
}

static os_err_t test_init(void)
{
    return OS_EOK;
}

static os_err_t test_cleanup(void)
{
    return OS_EOK;
}

ATEST_TC_EXPORT(ecoredump, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);
