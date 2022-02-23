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
 * @file        atest.h
 *
 * @brief       Header file for atest(And-Test)
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-15   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ATEST_H__
#define __ATEST_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @enum        atest_tc_priority
 *
 * @brief       Testcase priorities.
 ***********************************************************************************************************************
 */
enum atest_tc_priority
{
    TC_PRIORITY_HIGH    = 0,        /* High priority testcase. */
    TC_PRIORITY_MIDDLE,             /* Middle priority testcase. */
    TC_PRIORITY_LOW,                /* Low priority testcase. */
    TC_PRIORITY_CNT_MAX
};

/**
 ***********************************************************************************************************************
 * @struct      atest_stats
 *
 * @brief       Statistical results of executing multiple testcases.
 ***********************************************************************************************************************
 */
struct atest_stats
{
    os_uint16_t     tc_passed_num;          /* Total number of testcases passed. */
    os_uint16_t     tc_failed_num;          /* Total number of testcases failed. */
    os_uint16_t     tu_passed_num;          /* Total number of test units passed. */
    os_uint16_t     tu_failed_num;          /* Total number of test units failed. */
    os_uint16_t     tp_passed_num;          /* Total number of test points passed. */
    os_uint16_t     tp_failed_num;          /* Total number of test points failed. */
};
typedef struct atest_stats atest_stats_t;

/**
 ***********************************************************************************************************************
 * @struct      atest_tp_stats
 *
 * @brief       Statistical results of test points in a single testcase.
 ***********************************************************************************************************************
 */
struct atest_tp_stats
{
    os_uint16_t     tp_passed_num;          /* Total number of test points passed in a single testcase. */
    os_uint16_t     tp_failed_num;          /* Total number of test points failed in a single testcase. */
};
typedef struct atest_tp_stats atest_tp_stats_t;


/**
 ***********************************************************************************************************************
 * @struct      atest_tc_entry
 *
 * @brief       Atest testcase entry data structure. Will export the data to 'AtestTcTab' section in flash.
 ***********************************************************************************************************************
 */
struct atest_tc_entry
{
    const char               *name;             /* Testcase name. */
    os_err_t                (*init)(void);      /* Initialization before executing the testcase function. */
    void                    (*tc)(void);        /* Execute the testcase function. */
    os_err_t                (*cleanup)(void);   /* Cleanup after executing the testcase function.  */
    enum atest_tc_priority    priority;         /* The priority of testcase */
};
typedef struct atest_tc_entry atest_tc_entry_t;

/* Unit test handler function pointer */
typedef void (*atest_unit_func_t)(void);

/**
 ***********************************************************************************************************************
 * @def         ATEST_TC_EXPORT
 *
 * @brief       Export testcase entry to 'AtestTcTab' section in flash. Used in application layer.
 *
 * @param       name            The testcase name.
 * @param       testcase        The execute function of the testcase.
 * @param       init            The initialization function of the testcase.
 * @param       cleanup         The cleanup function of the testcase.
 * @param       priority        The testcase priority.
 ***********************************************************************************************************************
 */
#define ATEST_TC_EXPORT(name, testcase, init, cleanup, priority)                            \
    OS_USED static const atest_tc_entry_t gs_atest##testcase OS_SECTION("AtestTcTab") =     \
    {                                                                                       \
        #name,                                                                              \
        init,                                                                               \
        testcase,                                                                           \
        cleanup,                                                                            \
        priority                                                                            \
    };

/**
 ***********************************************************************************************************************
 * @def         ATEST_UNIT_RUN
 *
 * @brief       Test unit function executor. Used in 'testcase' function in application.
 *
 * @param       test_unit_func  Unit test function.
 ***********************************************************************************************************************
 */
#define ATEST_UNIT_RUN(test_unit_func)                                                      \
    do                                                                                      \
    {                                                                                       \
        atest_unit_run(test_unit_func, #test_unit_func);                                    \
        if (atest_tp_stats_get()->tp_failed_num != 0)                                       \
        {                                                                                   \
            return;                                                                         \
        }                                                                                   \
    } while (0)

#define __atest_assert(value, msg)  atest_assert(value, __FILE__, __LINE__, __func__, msg)

/* If @value is true, not assert, means passing. */        
#define tp_assert_true(value)       __atest_assert(value, "(" #value ") is false")

/* If @value is false, not assert, means passing. */
#define tp_assert_false(value)      __atest_assert(!(value), "(" #value ") is true")

/* If @value is null, not assert, means passing. */
#define tp_assert_null(value)       __atest_assert(OS_NULL == (const char *)(value), "(" #value ") is not null")

/* If @value is not null, not assert, means passing. */
#define tp_assert_not_null(value)   __atest_assert(OS_NULL != (const char *)(value), "(" #value ") is null")

/* If @a equal to @b, not assert, means passing. Integer type test. */
#define tp_assert_integer_equal(a, b)       __atest_assert((a) == (b), "(" #a ") not equal to (" #b ")")

/* If @a not equal to @b, not assert, means passing. Integer type test. */
#define tp_assert_integer_not_equal(a, b)   __atest_assert((a) != (b), "(" #a ") equal to (" #b ")")

/* if @a equal to @b, not assert, means passing. String type test. */
#define tp_assert_str_equal(a, b)           atest_assert_string((const char*)(a),           \
                                                                (const char*)(b),           \
                                                                OS_TRUE,                    \
                                                                __FILE__,                   \
                                                                __LINE__,                   \
                                                                __func__,                   \
                                                                "string not equal")

/* If @a not equal to @b, not assert, means passing. String type test. */
#define tp_assert_str_not_equal(a, b)       atest_assert_string((const char*)(a),           \
                                                                (const char*)(b),           \
                                                                OS_FALSE,                   \
                                                                __FILE__,                   \
                                                                __LINE__,                   \
                                                                __func__,                   \
                                                                "string equal")

/* If @a equal to @b, not assert, means passing. Buffer type test. */
#define tp_assert_buf_equal(a, b, size)     atest_assert_buf((const os_uint8_t *)(a),       \
                                                             (const os_uint8_t *)(b),       \
                                                             (size),                        \
                                                             OS_TRUE,                       \
                                                             __FILE__,                      \
                                                             __LINE__,                      \
                                                             __func__,                      \
                                                              "buf not equal")

/* If @a not equal to @b, not assert, means passing. Buffer type test. */
#define tp_assert_buf_not_equal(a, b, size) atest_assert_buf((const os_uint8_t *)(a),       \
                                                             (const os_uint8_t *)(b),       \
                                                             (size),                        \
                                                             OS_FALSE,                      \
                                                             __FILE__,                      \
                                                             __LINE__,                      \
                                                             __func__,                      \
                                                             "buf equal")

/* If @value is in range of @min and @max, not assert, means passing. */
#define tp_assert_in_range(value, min, max)         __atest_assert(((value >= min) && (value <= max)),          \
                                                                   "(" #value ") not in range("#min","#max")")

/* If @value is not in range of @min and @max, not assert, means passing. */
#define tp_assert_not_in_range(value, min, max)     __atest_assert(!((value >= min) && (value <= max)),         \
                                                                   "(" #value ") in range("#min","#max")")


extern atest_tp_stats_t  *atest_tp_stats_get(void);
extern void               atest_unit_run(atest_unit_func_t func, const char *unit_func_name);

extern void               atest_assert(os_bool_t        condition,
                                       const char      *file,
                                       os_int32_t       line,
                                       const char      *func,
                                       const char      *msg);

extern void               atest_assert_string(const char     *str_a,
                                              const char     *str_b,
                                              os_bool_t       equal,
                                              const char     *file,
                                              os_int32_t      line,
                                              const char     *func,
                                              const char     *msg);

extern void               atest_assert_buf(const os_uint8_t *buff_a,
                                           const os_uint8_t *buff_b,
                                           os_size_t         size,
                                           os_bool_t         equal,
                                           const char       *file,
                                           os_int32_t        line,
                                           const char       *func,
                                           const char       *msg);
#ifdef __cplusplus
}
#endif

#endif /* __ATEST_H__ */

