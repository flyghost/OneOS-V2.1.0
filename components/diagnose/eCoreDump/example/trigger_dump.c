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
 * @file        trigger_dump.c
 *
 * @brief       Example for trigger coredump in Thread mode.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <board.h>
#include <math.h>
#include "shell.h"
#include "ecoredump.h"

#define LOCAL_ASSERT(condition)                                                                                \
do                                                                                                          \
{                                                                                                           \
    if (!(condition))                                                                                       \
    {                                                                                                       \
        os_kprintf("Assert failed. Condition(%s). [%s][%d]\r\n", #condition, __FUNCTION__, __LINE__);       \
        after_assert_fail();                                                                                \
        while(1)                                                                                            \
        {                                                                                                   \
            ;                                                                                               \
        }                                                                                                   \
    }                                                                                                       \
} while (0)

extern uint32_t ecd_crc32b(const uint8_t *message, int32_t megLen, uint32_t initCrc);

void after_assert_fail(void);

static uint32_t sCrc32;

static void corefile_log_out(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        uint8_t b = data[i];
        sCrc32 = ecd_crc32b(&b, 1, sCrc32) ^ 0xFFFFFFFF;
        os_kprintf("%02x", b);
    }
}

void after_assert_fail()
{
    ecd_init(1, corefile_log_out);

    os_base_t irq_save = os_irq_lock();

    sCrc32 = 0xFFFFFFFF;

    os_kprintf("coredump start : {\n");
    ecd_multi_dump();
    os_kprintf("\n} coredump end\n");

    sCrc32 = ~sCrc32;

    os_kprintf("crc32 : %08x\n", sCrc32);

    os_irq_unlock(irq_save);
}

static os_err_t sh_trigger_assert(os_int32_t argc, char **argv)
{
    int x, y;
    const char * sx = "84597";
    const char * sy = "35268";

    float a, b, c;
    const char * fsa = "1.1322";
    const char * fsb = "45.2547";
    const char * fsc = "7854.2";

    a = atof(&fsa[0]);
    b = atof(&fsb[0]);
    c = atof(&fsc[0]);

    x = atoi(&sx[0]);
    y = atoi(&sy[0]);

    LOCAL_ASSERT(x * a == y * b * c);

    return 0;
}

typedef void (*fault_func)(float);

static os_err_t sh_trigger_fault(os_int32_t argc, char **argv)
{
    fault_func func = (fault_func)0xFFFF0000;
    int x, y;
    const char * sx = "84597";
    const char * sy = "35268";

    float a, b, c;
    const char * fsa = "1.1322";
    const char * fsb = "45.2547";
    const char * fsc = "7854.2";

    a = atof(&fsa[0]);
    b = atof(&fsb[0]);
    c = atof(&fsc[0]);

    x = atoi(&sx[0]);
    y = atoi(&sy[0]);

    func(x * a + y * b * c);

    return 0;
}

SH_CMD_EXPORT(trigger_assert, sh_trigger_assert, "trigger assert fail coredump");
SH_CMD_EXPORT(trigger_fault, sh_trigger_fault, "trigger fault coredump");
