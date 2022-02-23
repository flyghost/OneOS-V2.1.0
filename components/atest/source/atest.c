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
 * @file        atest.c
 *
 * @brief       This function implements the automatic test framework. atest(And-Test).
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-15   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_stddef.h>
#include <os_task.h>
#include <os_errno.h>
#include <string.h>
#include <stdlib.h>
#include <atest.h>
#include <dlog.h>
#include <os_util.h>
#include <os_assert.h>
#include <shell.h>
#include <option_parse.h>

#ifdef OS_USING_ATEST

#if OS_LOG_BUFF_SIZE < 256
#error "OS_LOG_BUFF_SIZE is less than 256!"
#endif

#if (ATEST_TASK_PRIORITY > OS_TASK_PRIORITY_MAX - 1)
#error "Atest task priority is greater than or equal to OS_TASK_PRIORITY_MAX"
#endif

#define ATEST_NAME_MAX_LEN          128
#define ATEST_TAG                   "ATEST"

struct atest_tc_table_info
{
    atest_tc_entry_t    *tc_table;
    os_uint32_t          tc_num;
};
typedef struct atest_tc_table_info atest_tc_table_info_t;

struct atest_ctrl_info
{
    char                    tc_name[ATEST_NAME_MAX_LEN];
    os_int32_t              loop_cnt;
    os_bool_t               use_task;
    os_bool_t               fail_stop;
    os_bool_t               print_help;
    enum atest_tc_priority  run_priority;
    atest_stats_t           test_stats;
    atest_tp_stats_t        tp_stats;
};
typedef struct atest_ctrl_info atest_ctrl_info_t;

static atest_tc_table_info_t    gs_tc_table_info;
static atest_ctrl_info_t        gs_ctrl_info;

static os_bool_t                gs_is_busy = OS_FALSE;
static os_uint16_t              gs_old_log_level;

#if defined(__ICCARM__) || defined(__ICCRX__)           /* For IAR compiler */
#pragma section="AtestTcTab"
#endif

static os_err_t atest_init(void)
{
    /* Initialize the atest commands table.*/
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM C Compiler */
    extern const int AtestTcTab$$Base;
    extern const int AtestTcTab$$Limit;
    
    gs_tc_table_info.tc_table = (atest_tc_entry_t *)&AtestTcTab$$Base;
    gs_tc_table_info.tc_num   = (atest_tc_entry_t *)&AtestTcTab$$Limit - gs_tc_table_info.tc_table;
    
#elif defined(__ICCARM__) || defined(__ICCRX__)         /* For IAR Compiler */
    gs_tc_table_info.tc_table = (atest_tc_entry_t *)__section_begin("AtestTcTab");
    gs_tc_table_info.tc_num   = (atest_tc_entry_t *)__section_end("AtestTcTab") - gs_tc_table_info.tc_table;
    
#elif defined(__GNUC__)                                 /* For GCC Compiler */
    extern const int __atest_tc_table_start;
    extern const int __atest_tc_table_end;
    
    gs_tc_table_info.tc_table = (atest_tc_entry_t *)&__atest_tc_table_start;
    gs_tc_table_info.tc_num   = (atest_tc_entry_t *)&__atest_tc_table_end - gs_tc_table_info.tc_table;;
#else
#error "Use the compilier that is not supported!!!"
#endif

    LOG_I(ATEST_TAG, "Atest is initialized success!");
    
    return OS_EOK;
}
OS_PREV_INIT(atest_init, OS_INIT_SUBLEVEL_LOW);

/**
 ***********************************************************************************************************************
 * @brief           Execute test unit function.
 *
 * @param[in]       func            Test unit function.
 * @param[in]       unit_func_name  Test unit name.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void atest_unit_run(atest_unit_func_t func, const char *unit_func_name)
{
    os_uint16_t old_tp_passed_num;
    os_uint16_t old_tp_failed_num;

    old_tp_passed_num = gs_ctrl_info.tp_stats.tp_passed_num;
    old_tp_failed_num = gs_ctrl_info.tp_stats.tp_failed_num;

    LOG_I(ATEST_TAG, "[   TU   ] [ ====== ] Unit name (%s).", unit_func_name);

    if (OS_NULL != func)
    {
        func();
    }

    if (gs_ctrl_info.tp_stats.tp_failed_num == old_tp_failed_num)
    {
        gs_ctrl_info.test_stats.tu_passed_num++;
    
        LOG_I(ATEST_TAG, "[   TU   ] [ PASSED ] Unit name (%s), tp passed (%u), tp failed (%u).",
              unit_func_name,
              gs_ctrl_info.tp_stats.tp_passed_num - old_tp_passed_num,
              gs_ctrl_info.tp_stats.tp_failed_num - old_tp_failed_num);
    }
    else
    {
        gs_ctrl_info.test_stats.tu_failed_num++;
    
        LOG_E(ATEST_TAG, "[   TU   ] [ FAILED ] Unit name (%s), tp passed (%u), tp failed (%u).",
              unit_func_name,
              gs_ctrl_info.tp_stats.tp_passed_num - old_tp_passed_num,
              gs_ctrl_info.tp_stats.tp_failed_num - old_tp_failed_num);
    }

    os_kprintf("\r\n");
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Get Statistical results of test points in a testcase.
 *
 * @param           None.
 *
 * @return          Statistical results of test points.
 ***********************************************************************************************************************
 */
atest_tp_stats_t *atest_tp_stats_get(void)
{
    return &gs_ctrl_info.tp_stats;
}

static const char *atest_file_basename(const char *file)
{
    char *slash_pos;
    char *rst;

    /* Find the last slash position */
    slash_pos = strrchr(file, '\\');
    if (!slash_pos)
    {
        slash_pos = strrchr(file, '/');
    }

    /* Not found slash in filename or filename length is less than 2 */
    if ((OS_NULL == slash_pos) || (strlen(file) < 2))
    {
        rst = (char *)file;
    }
    /* Found slash in filename, and filename length is more than or equal 2 */
    else
    {
        rst = (char *)(slash_pos + 1);
    }
    
    return (const char *)rst;
}

/**
 ***********************************************************************************************************************
 * @brief           Atest assert function.
 *
 * @param[in]       condition       Assert condition.
 *                                      - 0:    Assert.
 *                                      - else: Not assert.
 * @param[in]       file            File name.
 * @param[in]       line            Flie line number.
 * @param[in]       func            Function nmae.
 * @param[in]       msg             Assert message.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void atest_assert(os_bool_t condition, const char *file, os_int32_t line, const char *func, const char *msg)
{
    if (!condition)
    {
        gs_ctrl_info.tp_stats.tp_failed_num++;
        gs_ctrl_info.test_stats.tp_failed_num++;
        
        LOG_E(ATEST_TAG, "[   TP   ] [ ASSERT ] File: (%s); func: (%s:%d); msg: (%s)",
              atest_file_basename(file),
              func,
              line,
              msg);
    }
    else
    {
        gs_ctrl_info.tp_stats.tp_passed_num++;
        gs_ctrl_info.test_stats.tp_passed_num++;
    
        LOG_I(ATEST_TAG, "[   TP   ] [ PASSED ] File: (%s); func: (%s:%d).",
              atest_file_basename(file),
              func,
              line);
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Judge whether to assert according to the string comparison result and the desired comparison 
 *                  condition.
 *
 * @param[in]       str_a           String a.
 * @param[in]       str_b           String b.
 * @param[in]       equal           The desired comparison condition.
 *                                      - OS_TRUE:  Want to be equal.
 *                                      - OS_FALSE: Want to not be equal.
 * @param[in]       file            File name.
 * @param[in]       line            File line number.
 * @param[in]       func            Function name.
 * @param[in]       msg             Assert message.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void atest_assert_string(const char     *str_a,
                         const char     *str_b,
                         os_bool_t       equal,
                         const char     *file,
                         os_int32_t      line,
                         const char     *func,
                         const char     *msg)
{
    if ((OS_NULL == str_a) || (OS_NULL == str_b))
    {
        atest_assert(OS_FALSE, file, line, func, msg);
        return;
    }

    if (equal)
    {
        if (!strcmp(str_a, str_b))
        {
            atest_assert(OS_TRUE, file, line, func, msg);
        }
        else
        {
            atest_assert(OS_FALSE, file, line, func, msg);
        }
    }
    else
    {
        if (!strcmp(str_a, str_b))
        {
            atest_assert(OS_FALSE, file, line, func, msg);
        }
        else
        {
            atest_assert(OS_TRUE, file, line, func, msg);
        }
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Judge whether to assert according to the buffer comparison result and the desired comparison 
 *                  condition.
 *
 * @param[in]       buff_a          Buffer a.
 * @param[in]       buff_b          Buffer b.
 * @param[in]       size            The size you want to compare.
 * @param[in]       equal           The desired comparison condition.
 *                                      - OS_TRUE:  Want to be equal.
 *                                      - OS_FALSE: Want to not be equal.
 * @param[in]       file            File name.
 * @param[in]       line            File line number.
 * @param[in]       func            Function name.
 * @param[in]       msg             Assert message.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void atest_assert_buf(const os_uint8_t *buff_a,
                      const os_uint8_t *buff_b,
                      os_size_t         size,
                      os_bool_t         equal,
                      const char       *file,
                      os_int32_t        line,
                      const char       *func,
                      const char       *msg)
{
    if ((OS_NULL == buff_a) || (OS_NULL == buff_b))
    {
        atest_assert(OS_FALSE, file, line, func, msg);
        return;
    }

    if (equal)
    {
        if (!memcmp(buff_a, buff_b, size))
        {
            atest_assert(OS_TRUE, file, line, func, msg);
        }
        else
        {
            atest_assert(OS_FALSE, file, line, func, msg);
        }
    }
    else
    {
        if (!memcmp(buff_a , buff_b, size))
        {
            atest_assert(OS_FALSE, file, line, func, msg);
        }
        else
        {
            atest_assert(OS_TRUE, file, line, func, msg);
        }
    }

    return;
}

static void atest_help(void)
{
    os_kprintf("\r\n");
  
    os_kprintf("Command: atest_run\r\n");
    os_kprintf("   Info: Execute testcases.\r\n");
    os_kprintf(" Format: atest_run [-n testacse name] [-l loop count] [-p priority level] [-t] [-s] [-h]\r\n");
    os_kprintf("\r\n");
  
    os_kprintf("  Usage:\r\n");
  
    os_kprintf("         -n     Specify a testcase name.\r\n");
    os_kprintf("                If don't specify this option, run all testcases.\r\n");
    os_kprintf("\r\n");
    
    os_kprintf("         -l     Specify a loop count that the testcase runs.\r\n");
    os_kprintf("                If don't specify this option, the testcase only run one times.\r\n");
    os_kprintf("\r\n");
    
    os_kprintf("         -p     Specify a priority level. Only testcases greater or equal than this level can be run.\r\n");
    os_kprintf("                H  -- High priority level.\r\n");
    os_kprintf("                M  -- Middle priority level.\r\n");
    os_kprintf("                L  -- Low priority level.\r\n");
    os_kprintf("                If don't specify this option, low priority level is be used by default.\r\n");
    os_kprintf("\r\n");
    
    os_kprintf("         -t     Create a new task to execute the testcase(s).\r\n");
    os_kprintf("                If don't specify this option, execute the testcase(s) in shell task.\r\n");
    os_kprintf("\r\n");
    
    os_kprintf("         -s     If specify this option, as long as a testcase fails, the subsequent testcases are no\r\n"
               "                longer running.\r\n");
    os_kprintf("                If don't specify this option, when a testcase fails, the subsequent testcases still run.\r\n");
    os_kprintf("\r\n");
    
    os_kprintf("         -h     Print help information of atest_run command.\r\n");
    os_kprintf("                If specify this option, other options are ignored.\r\n");
    os_kprintf("\r\n");
  
    return;
}

static os_err_t atest_ctrl_info_get(os_int32_t argc, char * const *argv, atest_ctrl_info_t *ctrl_info)
{
    opt_state_t state;
    os_int32_t  opt_ret;
    os_int32_t  ret;

    memset(ctrl_info, 0, sizeof(atest_ctrl_info_t));
    ctrl_info->loop_cnt        = 1;
    ctrl_info->use_task        = OS_FALSE;
    ctrl_info->fail_stop       = OS_FALSE;
    ctrl_info->print_help      = OS_FALSE;
    ctrl_info->run_priority    = TC_PRIORITY_LOW;

    memset(&state, 0, sizeof(state));
    opt_init(&state, 1);

    ret = OS_EOK;
    while (1)
    {
        opt_ret = opt_get(argc, argv, "n:l:p:tsh", &state);
        if (opt_ret == OPT_EOF)
        {
            break;
        }

        if ((opt_ret == OPT_BADOPT) || (opt_ret == OPT_BADARG))
        {
            ret = OS_ERROR;
            break;
        }
    
        switch (opt_ret)
        {
        case 'n':
            memset(ctrl_info->tc_name, 0, ATEST_NAME_MAX_LEN);
            strncpy(ctrl_info->tc_name, state.opt_arg, ATEST_NAME_MAX_LEN);
            break;
            
        case 'l':
            ctrl_info->loop_cnt = atoi(state.opt_arg);
            if (ctrl_info->loop_cnt <= 0)
            {
                os_kprintf("Invalid loop count(%s)\r\n", state.opt_arg);

                ret = OS_EINVAL;
                break;
            }
            
            break;
            
        case 'p':
            if (!strcmp(state.opt_arg, "H"))
            {
                ctrl_info->run_priority = TC_PRIORITY_HIGH;
            }
            else if (!strcmp(state.opt_arg, "M"))
            {
                ctrl_info->run_priority = TC_PRIORITY_MIDDLE;
            }
            else if (!strcmp(state.opt_arg, "L"))
            {
                ctrl_info->run_priority = TC_PRIORITY_LOW;
            }
            else
            {
                os_kprintf("Invalid priority(%s)\r\n", state.opt_arg);

                ret = OS_EINVAL;
                break;
            }
            
            break;
            
        case 't':
            ctrl_info->use_task = OS_TRUE;
            break;
            
        case 's':
            ctrl_info->fail_stop = OS_TRUE;
            break;
        case 'h':
            ctrl_info->print_help = OS_TRUE;
            break;
            
        default:
            os_kprintf("Invalid option: %c\r\n", (char)opt_ret);

            ret = OS_EINVAL;
            break;
        }

        if (ret != OS_EOK)
        {
            break;
        }
    }

    return ret;
}

static os_err_t atest_run_a_testcase(atest_tc_entry_t *tc_entry, atest_ctrl_info_t *ctrl_info)
{
    os_err_t init_ret;
    os_err_t cleanup_ret;
    os_err_t ret;

    ret = OS_EOK;
    ctrl_info->tp_stats.tp_passed_num = 0;
    ctrl_info->tp_stats.tp_failed_num = 0;

    do
    {
        LOG_I(ATEST_TAG, "[   TC   ] [ ====== ] Testcase (%s) begin to run.", tc_entry->name);
        os_kprintf("\r\n");
        
        if (OS_NULL != tc_entry->init)
        {
            init_ret = tc_entry->init();
            if (OS_EOK != init_ret)
            {
                LOG_E(ATEST_TAG, "[   TC   ] [ FAILED ] Testcase (%s) init failed", tc_entry->name);

                ret = OS_ERROR;
                break;
            }
        }

        if (OS_NULL != tc_entry->tc)
        {
            tc_entry->tc();
                    
            if (0 == ctrl_info->tp_stats.tp_failed_num)
            {
                LOG_I(ATEST_TAG, "[   TC   ] [ PASSED ] Testcase (%s), tp passed (%u), tp failed (%u)",
                      tc_entry->name,
                      ctrl_info->tp_stats.tp_passed_num,
                      ctrl_info->tp_stats.tp_failed_num);
            }
            else
            {
                LOG_E(ATEST_TAG, "[   TC   ] [ FAILED ] Testcase (%s), tp passed (%u), tp failed (%u)",
                      tc_entry->name,
                      ctrl_info->tp_stats.tp_passed_num,
                      ctrl_info->tp_stats.tp_failed_num);
                ret = OS_ERROR;
            }
        }
        else
        {
            LOG_E(ATEST_TAG, "[   TC   ] [ FAILED ] Testcase (%s) has no execute function.", tc_entry->name);
            ret = OS_ERROR;
        }

        if (OS_NULL != tc_entry->cleanup)
        {
            cleanup_ret = tc_entry->cleanup();
            if (OS_EOK != cleanup_ret)
            {
                LOG_E(ATEST_TAG, "[   TC   ] [ FAILED ] Testcase (%s) cleanup failed.", tc_entry->name);

                ret = OS_ERROR;
                break;
            }
        }   
    } while (0);

    return ret;
}

static atest_tc_entry_t *atest_get_next_tc_entry(const atest_ctrl_info_t     *ctrl_info,
                                                 const atest_tc_table_info_t *tc_table_info,
                                                 os_uint16_t                 *next_query_index)
{
    atest_tc_entry_t *tc_entry;
    os_size_t         cmp_len;
    os_uint16_t       index;

    if (*next_query_index >= tc_table_info->tc_num)
    {
        return OS_NULL;
    }

    cmp_len = 0;
    if (*ctrl_info->tc_name)
    {
        cmp_len = strlen(ctrl_info->tc_name);
        if (ctrl_info->tc_name[cmp_len - 1] == '*')
        {
            cmp_len -= 1;
        }
    }

    tc_entry = OS_NULL;
    for (index = *next_query_index; index < tc_table_info->tc_num; index++)
    {
        if (cmp_len > 0)
        {
            if (memcmp(tc_table_info->tc_table[index].name, ctrl_info->tc_name, cmp_len))
            {
                continue;
            }
        }

        if (tc_table_info->tc_table[index].priority > ctrl_info->run_priority)
        {
            continue;
        }

        tc_entry = &tc_table_info->tc_table[index];
        *next_query_index = index + 1;

        break;
    }

    return tc_entry;
}

static void atest_tc_do_run(void *arg)
{
    atest_ctrl_info_t *ctrl_info;
    atest_tc_entry_t  *tc_entry;
    os_bool_t          found_entry;
    os_bool_t          abort;
    os_uint16_t        loop_index;
    os_uint16_t        next_query_index;
    os_uint16_t        total;
    os_err_t           ret;

    ctrl_info = (atest_ctrl_info_t *)arg;
    ctrl_info->test_stats.tc_passed_num = 0;
    ctrl_info->test_stats.tc_failed_num = 0;
    ctrl_info->test_stats.tu_passed_num = 0;
    ctrl_info->test_stats.tu_failed_num = 0;
    ctrl_info->test_stats.tp_passed_num = 0;
    ctrl_info->test_stats.tp_failed_num = 0;

    os_kprintf("\r\n");
    
    abort = OS_FALSE;

    for (loop_index = 0; loop_index < ctrl_info->loop_cnt; loop_index++)
    {  
        os_kprintf("-----------------------------------Loop: %u-------------------------------------\r\n",
                   loop_index + 1);
        os_kprintf("\r\n");
    
        found_entry      = OS_FALSE;
        next_query_index = 0;

        while (1)
        {
            tc_entry = atest_get_next_tc_entry(ctrl_info, &gs_tc_table_info, &next_query_index);
            if (!tc_entry)
            {
                break;
            }

            if (!found_entry)
            {
                found_entry = OS_TRUE;
            }
              
            ret = atest_run_a_testcase(tc_entry, ctrl_info);
            if (OS_EOK == ret)
            {
                ctrl_info->test_stats.tc_passed_num++;    
            }
            else
            {
                ctrl_info->test_stats.tc_failed_num++;
                
                if (ctrl_info->fail_stop)
                {
                    abort = OS_TRUE;
                    break;
                } 
            }

            os_kprintf("\r\n");
            os_kprintf("-----------------------------------Loop: %u-------------------------------------\r\n",
                       loop_index + 1);
            os_kprintf("\r\n");
        }

        if (!found_entry)
        {
            LOG_W(ATEST_TAG, "[ ATEST  ] [ ====== ] Not find testcase(%s).", ctrl_info->tc_name);
            break;
        }

        if (abort)
        {
            break;
        }
    }

    os_kprintf("\r\n");
    LOG_I(ATEST_TAG, "[ ATEST  ] [ ====== ] Finished.");
    os_kprintf("\r\n");

    os_kprintf("  type        total      passed      failed      pass rate \r\n");
    os_kprintf("----------    -----      ------      ------      ---------\r\n");

    total = ctrl_info->test_stats.tc_passed_num + ctrl_info->test_stats.tc_failed_num;
    os_kprintf("test case      %3u         %3u         %3u          %2u%% \r\n",
               total,
               ctrl_info->test_stats.tc_passed_num,
               ctrl_info->test_stats.tc_failed_num,
               total > 0 ? ctrl_info->test_stats.tc_passed_num * 100 / total : 0);

    total = ctrl_info->test_stats.tu_passed_num + ctrl_info->test_stats.tu_failed_num;
    os_kprintf("test unit      %3u         %3u         %3u          %2u%% \r\n",
               total,
               ctrl_info->test_stats.tu_passed_num,
               ctrl_info->test_stats.tu_failed_num,
               total > 0 ? ctrl_info->test_stats.tu_passed_num * 100 / total : 0);

    total = ctrl_info->test_stats.tp_passed_num + ctrl_info->test_stats.tp_failed_num;
    os_kprintf("test point     %3u         %3u         %3u          %2u%% \r\n",
               total,
               ctrl_info->test_stats.tp_passed_num,
               ctrl_info->test_stats.tp_failed_num,
               total > 0 ? ctrl_info->test_stats.tp_passed_num * 100 / total : 0);

    os_kprintf("\r\n");

    gs_is_busy = OS_FALSE;
    dlog_global_lvl_set(gs_old_log_level);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Run testcase(s).
 *
 * @param[in]       argc            Argument count.
 * @param[in]       argv            Argument value.
 *
 * @return          The result of executing command.
 * @retval          OS_EOK          Success.
 * @retval          else            Failed.
 ***********************************************************************************************************************
 */
static os_err_t atest_tc_run(os_int32_t argc, char **argv)
{
    os_task_t *task;
    os_err_t   ret;

    ret = OS_EOK;

    do
    {
        if (OS_TRUE == gs_is_busy)
        {
            os_kprintf("Testcases are running, please try again later.\r\n");
            
            ret = OS_EBUSY;
            break;
        }

        gs_is_busy = OS_TRUE;

        ret = atest_ctrl_info_get(argc, argv, &gs_ctrl_info);
        if (ret != OS_EOK)
        {
            atest_help();
            gs_is_busy = OS_FALSE;
            
            break;
        }

        if (OS_TRUE == gs_ctrl_info.print_help)
        {
            atest_help();
        
            gs_is_busy = OS_FALSE;
            break;
        }

        gs_old_log_level = dlog_global_lvl_get();
        dlog_global_lvl_set(DLOG_INFO);

        if (!gs_ctrl_info.use_task)
        {
            atest_tc_do_run((void *)&gs_ctrl_info);
        }
        else
        {
            task = os_task_create("atest",
                                  atest_tc_do_run,
                                  (void *)&gs_ctrl_info,
                                  ATEST_TASK_STACK_SIZE,
                                  ATEST_TASK_PRIORITY);
            if (OS_NULL != task)
            {
                ret = os_task_startup(task);
                if (OS_EOK != ret)
                {
                    OS_ASSERT_EX(0, "Atest start task to execute testcase failed");
                }
            }
            else
            {
                os_kprintf("Create atest task failed, execute testcase(s) failed.\r\n");

                gs_is_busy = OS_FALSE;
                dlog_global_lvl_set(gs_old_log_level);
                
                ret = OS_ERROR;
            }
        }
    } while (0);
    
    return ret;
}
SH_CMD_EXPORT(atest_run, atest_tc_run, "atest_run [-n testacse name] [-l loop count] [-p priority level] [-t] [-s] [-h]");

/**
 ***********************************************************************************************************************
 * @brief           Display all atest testcases.
 *
 * @param[in]       argc            Argument count.
 * @param[in]       argv            Argument value.
 *
 * @return          The result of executing command.
 * @retval          OS_EOK          Success.
 * @retval          else            Failed.
 ***********************************************************************************************************************
 */
OS_UNUSED static os_err_t atest_tc_list(os_int32_t argc, char **argv)
{
    os_size_t  index;
    os_int32_t priority;
    char      *prior_str[TC_PRIORITY_CNT_MAX];

    prior_str[TC_PRIORITY_HIGH]    = "High";
    prior_str[TC_PRIORITY_MIDDLE]  = "Middle";
    prior_str[TC_PRIORITY_LOW]     = "Low";
    
    os_kprintf("\r\n");
    os_kprintf("%-61.s %-10.s\r\n", "Testcase name", "Priority");
    os_kprintf("---------------------------------                             --------\r\n");

    for (index = 0; index < gs_tc_table_info.tc_num; index++)
    {
        if (!memcmp(gs_tc_table_info.tc_table[index].name, "atest_occupancy", strlen("atest_occupancy")))
        {
            continue;
        }

        priority = (os_int32_t)gs_tc_table_info.tc_table[index].priority;
        
        if ((priority < TC_PRIORITY_HIGH) || (priority > TC_PRIORITY_LOW))
        {
            priority = TC_PRIORITY_CNT_MAX;   
        }
    
        os_kprintf("%-62s %s\r\n",
                   gs_tc_table_info.tc_table[index].name,
                   priority != TC_PRIORITY_CNT_MAX ? prior_str[priority] : "Invalid");
    }

    os_kprintf("\r\n");

    return OS_EOK;
}
SH_CMD_EXPORT(atest_list, atest_tc_list, "Display all atest testcases");

/**
 ***********************************************************************************************************************
 * @brief           This is a occupancy testcase.
 *
 * @attention       If there are no testcases, a warning will appear during compilation. This occupancy testcase will
 *                  resolve it.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void atest_tc_occupancy(void)
{
    /* Do nothing */
    
    return;
}
ATEST_TC_EXPORT(atest_occupancy, atest_tc_occupancy, OS_NULL, OS_NULL, TC_PRIORITY_CNT_MAX);

#endif /* OS_USING_ATEST */

