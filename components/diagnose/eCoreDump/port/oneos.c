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
 * @file        oneos.c
 *
 * @brief       This file provides port function for cCoreDump.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include "oneos_config.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ecoredump.h"
#include "faultdump.h"
#include "fal.h"
#include "os_util.h"
#include "shell.h"
#include "dlog.h"
#include "arch_interrupt.h"
#include "os_task.h"
#include "os_errno.h"
#include "arch_exception.h"

#include "../src/utils.h"

#define ECD_STORAGE_SIZE         8192
#define XIP_FLASH_START_ADDR     0x08000000
#define XIP_FLASH_PAGE_ALGIN     32

#define collect_registers   collect_registers_armv7m

uint32_t ecd_crc32b(const uint8_t *message, int32_t megLen, uint32_t initCrc);

static void collect_registers_armv7m(uint32_t * stack_top,
                            core_regset_type * core_regset,
                            fp_regset_type * fp_regset);


#if defined(__CC_ARM) ||  defined(__GNUC__)
const uint8_t dump_storage[ECD_STORAGE_SIZE] __attribute__ ((aligned (ECD_STORAGE_SIZE))) = { _8K_0xFF };
#else
#error "Please add other compile implementation"
#endif

uint8_t page_buffer[XIP_FLASH_PAGE_ALGIN];

int32_t ecd_flash_op_init(void ** ctx)
{
    fal_flash_t *fal_flash;
    fal_flash = fal_flash_find("onchip_flash");
    *ctx = (void*)fal_flash;
    if ((dump_storage[0] == 0xFF) &&
        (dump_storage[1] == 0xFF) &&
        (ECD_STORAGE_SIZE % fal_flash->block_size == 0))
    {
        fal_flash->ops.erase_block(fal_flash, 
            ((uint32_t)&dump_storage[0] - XIP_FLASH_START_ADDR) / fal_flash->page_size,
            ECD_STORAGE_SIZE / fal_flash->page_size);
    }

    if (fal_flash->page_size > XIP_FLASH_PAGE_ALGIN)
        return ECD_ERROR;
    else
        return ECD_OK;
}

uint32_t ecd_flash_op_get_page_size(void * ctx)
{
    fal_flash_t *fal_flash = (fal_flash_t *)ctx;
    return fal_flash->page_size;
}

const uint8_t * ecd_flash_op_get_start_addr(void * ctx)
{
    return &dump_storage[0];
}

uint32_t ecd_flash_op_get_total_size(void * ctx)
{
    return sizeof(dump_storage);
}

int32_t ecd_flash_op_write_page(void * ctx, uint32_t page_addr, uint8_t *buff, uint32_t page_nr)
{
    fal_flash_t *fal_flash = (fal_flash_t *)ctx;
    uint32_t page_no = (page_addr - XIP_FLASH_START_ADDR) / fal_flash->page_size;

    fal_flash->ops.write_page(fal_flash, page_no, buff, page_nr);
    return ECD_OK;
}

os_err_t os_hw_hard_fault_exception_hook(void *context, os_size_t *msp, os_size_t *psp)
{
    collect_registers((uint32_t *)context,
                get_cur_core_regset_address(),
                get_cur_fp_regset_address());

    ecd_faultdump();

    return OS_ERROR;
}

extern void os_hw_exception_install(os_err_t (*exception_handle)(void*, os_size_t*, os_size_t*));

int faultdump_init(void)
{
    os_hw_exception_install(os_hw_hard_fault_exception_hook);
    return OS_EOK;
}
OS_CMPOENT_INIT(faultdump_init, OS_INIT_SUBLEVEL_MIDDLE);

static os_err_t sh_corefile_dump(os_int32_t argc, char **argv)
{
    int index;
    const uint8_t * corefile;
    uint32_t clen, crc32;

    if (argc == 2)
        index = atoi(argv[1]);
    else
        index = 0;

    ecd_get_fault_file(&corefile, &clen, index);

    if (clen > 0 && corefile != NULL)
    {
        os_kprintf("core file (idx : %d) {\n", index);

        for (uint32_t i = 0; i < clen; i++)
        {
            os_kprintf("%02x", corefile[i]);
        }

        os_kprintf("\n}\n");

        crc32 = 0xFFFFFFFF;
        crc32 = ecd_crc32b(corefile, clen, crc32);

        os_kprintf("crc32 : %08x\n", crc32);
    }

    return 0;
}

SH_CMD_EXPORT(corefile_dump, sh_corefile_dump, "print out corefile, give arg to specify index");

static os_err_t sh_corefile_count()
{
    int c = ecd_get_fault_file_count();
    os_kprintf("core file count : %d\n", c);
    return 0;
}

SH_CMD_EXPORT(corefile_count, sh_corefile_count, "show corefile count");

void ecd_log_err(char * format, ...)
{
#if (DLOG_ERROR <= DLOG_COMPILE_LEVEL)
    char log_buf[64];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    dlog_output(DLOG_ERROR, "ECD", OS_TRUE, "%s", log_buf);
#endif
}

uint32_t ecd_crc32b(const uint8_t *message, int32_t megLen, uint32_t initCrc)
{
   int i, j;
   uint32_t byte, crc, mask;

   i = 0;
   crc = initCrc;
   for (i=0; i<megLen; i++)
   {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--)      // Do eight times.
      {
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }

   return ~crc;
}

typedef struct {
    int32_t     thr_cnts;
    int32_t     cur_idx;
} oneos_ti_priv_t;

static int32_t is_task_object(os_task_t * task)
{
    return (task->object_inited == 0x55) &&
                (task->stack_top >= task->stack_begin) &&
                (task->stack_top <= task->stack_end);
}

static int32_t oneos_thr_cnts(struct thread_info_ops * ops)
{
    oneos_ti_priv_t * priv = (oneos_ti_priv_t *)ops->priv;
    int32_t idx = 0;
    os_task_t *iter_task;
    os_task_t * cur_task = os_task_self();

    if (-1 == priv->thr_cnts)
    {
        os_list_for_each_entry(iter_task, &cur_task->resource_node, os_task_t, resource_node)
        {
            if (is_task_object(iter_task))
            {
                idx++;
            }
        }

        priv->thr_cnts = idx + 1;
        priv->cur_idx = idx;
    }

    return priv->thr_cnts;
}

static int32_t oneos_cur_idx(struct thread_info_ops * ops)
{
    oneos_ti_priv_t * priv = (oneos_ti_priv_t *)ops->priv;

    if (-1 == priv->cur_idx)
        oneos_thr_cnts(ops);

    return priv->cur_idx;
}

static void oneos_thr_rset(struct thread_info_ops* ops, int32_t idx,
                                core_regset_type * core_regset,
                                fp_regset_type * fp_regset)
{
    int32_t idx_l = 0;
    os_task_t *iter_task;
    os_task_t * cur_task = os_task_self();

    if (idx == oneos_cur_idx(ops))
    {
        memcpy(core_regset, get_cur_core_regset_address(), sizeof(core_regset_type));
        memcpy(fp_regset, get_cur_fp_regset_address(), sizeof(fp_regset_type));
        return;
    }

    os_list_for_each_entry(iter_task, &cur_task->resource_node, os_task_t, resource_node)
    {
        if (is_task_object(iter_task))
        {
            if (idx == idx_l)
            {
                collect_registers(iter_task->stack_top, core_regset, fp_regset);
                break;
            }
            idx_l++;
        }
    }
}

static int32_t oneos_get_mem_cnts(struct thread_info_ops* ops)
{
    return oneos_thr_cnts(ops);
}

static int32_t oneos_get_memarea(struct thread_info_ops* ops, int32_t idx,
                        uint32_t* addr, uint32_t* memlen)
{
    int32_t         idx_l = 0;
    os_task_t *     cur_task = os_task_self();
    os_task_t *     iter_task;

    if (idx == oneos_cur_idx(ops))
    {
        *addr = get_cur_core_regset_address()->sp;
        *memlen = 1024;
        return 0;
    }

    os_list_for_each_entry(iter_task, &cur_task->resource_node, os_task_t, resource_node)
    {
        if (is_task_object(iter_task))
        {
            if (idx == idx_l)
            {
                *addr = (uint32_t)iter_task->stack_top;
                *memlen = (uint32_t)iter_task->stack_end - (uint32_t)iter_task->stack_top;
                return 0;
            }
            idx_l++;
        }
    }
    return 0;
}

void ecd_rtos_thread_ops(struct thread_info_ops * ops)
{
    static oneos_ti_priv_t priv;
    ops->get_threads_count = oneos_thr_cnts;
    ops->get_current_thread_idx = oneos_cur_idx;
    ops->get_thread_regset = oneos_thr_rset;
    ops->get_memarea_count = oneos_get_mem_cnts;
    ops->get_memarea = oneos_get_memarea;
    ops->priv = &priv;
    priv.cur_idx = -1;
    priv.thr_cnts = -1;
}

static void collect_registers_armv7m(uint32_t * stack_top,
                                    core_regset_type * core_regset,
                                    fp_regset_type * fp_regset)
{
    struct stack_frame_common * frame = (struct stack_frame_common *)stack_top;

    core_regset->r4 = frame->r4;
    core_regset->r5 = frame->r5;
    core_regset->r6 = frame->r6;
    core_regset->r7 = frame->r7;
    core_regset->r8 = frame->r8;
    core_regset->r9 = frame->r9;
    core_regset->r10 = frame->r10;
    core_regset->r11 = frame->r11;

    if (frame->exc_return & 0x10)
    {
        struct stack_frame_nofpu * nframe = (struct stack_frame_nofpu *)frame;
        core_regset->r0 = nframe->r0;
        core_regset->r1 = nframe->r1;
        core_regset->r2 = nframe->r2;
        core_regset->r3 = nframe->r3;
        core_regset->r12 = nframe->r12;
        core_regset->pc = nframe->pc;
        core_regset->lr = nframe->lr;
        core_regset->xpsr = nframe->psr;

        core_regset->sp = (uint32_t)(&nframe->psr) + 4;
        memset(fp_regset, 0, sizeof(fp_regset_type));
    }
    else
    {
        struct stack_frame_fpu * fframe = (struct stack_frame_fpu *)frame;

        core_regset->r0 = fframe->r0;
        core_regset->r1 = fframe->r1;
        core_regset->r2 = fframe->r2;
        core_regset->r3 = fframe->r3;
        core_regset->r12 = fframe->r12;
        core_regset->pc = fframe->pc;
        core_regset->lr = fframe->lr;
        core_regset->xpsr = fframe->psr;

        core_regset->sp = (uint32_t)(&fframe->NO_NAME) + 4;
        memcpy(&fp_regset->d0, &fframe->S0, 16 * 4);
        memcpy(&fp_regset->d8, &fframe->s16, 16 * 4);
        memcpy(&fp_regset->fpscr, &fframe->FPSCR, 4);
    }
}
