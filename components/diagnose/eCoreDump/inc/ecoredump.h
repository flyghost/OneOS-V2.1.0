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
 * @file        ecoredump.h
 *
 * @brief       This file provides interface for cCoreDump.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ECOREDUMP_H__
#define __ECOREDUMP_H__

#include "ecd_registers_define.h"

enum {
    ECD_ERROR = -1,
    ECD_OK = 0,
};

typedef void (*ecd_writeout_func_t) (uint8_t *, int);

/*  Use this struct instead list to avoid dynamic memory allocation.
    Malloc is likely to fail when hardfault*/
struct thread_info_ops {
    int32_t (*get_threads_count)(struct thread_info_ops*);
    int32_t (*get_current_thread_idx)(struct thread_info_ops*);
    void    (*get_thread_regset)(struct thread_info_ops*, int32_t,
                                    core_regset_type * core_regset,
                                    fp_regset_type * fp_regset);
    int32_t (*get_memarea_count)(struct thread_info_ops*);
    int32_t (*get_memarea)(struct thread_info_ops*, int32_t,
                            uint32_t*, uint32_t*);
    void *priv;
};

/**
 ***********************************************************************************************************************
 * @brief           Init the eCoreDump.
 *
 * @param[in]       with_fp         whether dump vfp registers.
 * @param[in]       func            function pointer to write the coredump file out.
 *
 * @return          the coredump file size.
 ***********************************************************************************************************************
 */
void ecd_init(int with_fp, ecd_writeout_func_t func);

/**
 ***********************************************************************************************************************
 * @brief           Generate coredump in current call chain.
 *
 * @return          no Return.
 ***********************************************************************************************************************
 */
void ecd_mini_dump(void);

/**
 ***********************************************************************************************************************
 * @brief           Get the core file size when current call chain dump.
 *
 * @return          the core file size.
 ***********************************************************************************************************************
 */
int32_t ecd_mini_dump_size(void);

/**
 ***********************************************************************************************************************
 * @brief           Generate coredump with all threads.
 *
 * @return          no Return.
 ***********************************************************************************************************************
 */
void ecd_multi_dump(void);

/**
 ***********************************************************************************************************************
 * @brief           Get the core file size when all threads dump.
 *
 * @return          the core file size.
 ***********************************************************************************************************************
 */
int32_t ecd_multi_dump_size(void);

void ecd_gen_coredump(struct thread_info_ops * ops);
int32_t ecd_corefile_size(struct thread_info_ops * ops);
void ecd_rtos_thread_ops(struct thread_info_ops * ops);
void ecd_mini_dump_ops(struct thread_info_ops * ops);

core_regset_type * get_cur_core_regset_address(void);
fp_regset_type * get_cur_fp_regset_address(void);

#endif
