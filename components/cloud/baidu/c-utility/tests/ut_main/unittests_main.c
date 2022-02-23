#include <oneos_config.h>
#include <os_task.h>
#include <os_assert.h>

#include "testrunnerswitcher.h"

#define DBG_EXT_TAG "baidu"
#define DBG_EXT_LVL DBG_EXT_INFO

#include <dlog.h>

static void unit_test_run(void *para)
{
    ut_main();
    return;
}
os_task_t  *baidu_unit_test_thread = NULL;
static void baidu_unit_test(void)
{
    baidu_unit_test_thread =
        os_task_create("generate_baidu_unit_test", unit_test_run, OS_NULL, 8192, OS_TASK_PRIORITY_MAX / 2);
    if (NULL == baidu_unit_test_thread)
    {
        LOG_E(DBG_EXT_TAG, "create thread failed");
        OS_ASSERT(OS_NULL != baidu_unit_test_thread);
    }
    os_task_startup(baidu_unit_test_thread);
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(baidu_unit_test, baidu_unit_test, "baidu_uint_tests_run");
#endif