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
 * @file        main.c
 *
 * @brief       This file implements functions to dump core-file to flash.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "faultdump.h"
#include "./utils.h"

#define MY_MAGIC_NUMBER             (0x5cc5)
#define MY_MAGIC_FREE               (0xffff)

extern uint8_t page_buffer[];

typedef struct
{
    uint16_t magic;
    uint16_t page_size;
    uint32_t file_size;
    uint8_t corefile[1];
} persistent_format_t;

static void * flash_op_ctx;

static uint32_t cur_page_addr;
static uint32_t off_in_page;

static int is_available_storage(const persistent_format_t * current)
{
    return MY_MAGIC_NUMBER == current->magic;
}

static int is_free_storage(const persistent_format_t * current)
{
    return MY_MAGIC_FREE == current->magic;
}

static const persistent_format_t * first_storage()
{
    return (persistent_format_t *)ecd_flash_op_get_start_addr(flash_op_ctx);
}

static int is_enough_space(const persistent_format_t * current, uint32_t core_size)
{
    uint32_t need = (uint32_t)current + core_size;
    uint32_t last_storage = (uint32_t)ecd_flash_op_get_start_addr(flash_op_ctx) +
                ecd_flash_op_get_total_size(flash_op_ctx);
    return last_storage >= need;
}

static const persistent_format_t * find_next_storage(const persistent_format_t * current)
{
    if (!is_available_storage(current))
        return NULL;

    uint32_t current_usage = ELEM_OFFSET(persistent_format_t, corefile) + current->file_size;
    current_usage = MY_ALIGN(current_usage, current->page_size);

    persistent_format_t * next = (persistent_format_t *)((uint32_t)current + current_usage);
    return next;
}

static const persistent_format_t * find_free_storage(uint32_t core_size)
{
    const persistent_format_t * current = first_storage();
    if (is_free_storage(current))
    {
        if (is_enough_space(current, core_size))
            return current;
        else
            return NULL;
    }

    const persistent_format_t * next = current;

    while((next = find_next_storage(next)) != NULL)
    {
        if (is_free_storage(next) && is_enough_space(next, core_size))
            return next;
    }

    return NULL;
}

typedef void (*iter_func_t)(const persistent_format_t *, void * data);

static void iter_coredump_files(iter_func_t func, void * param)
{
    const persistent_format_t * first = first_storage();
    if (is_available_storage(first))
    {
        const persistent_format_t * next = first;
        func(next, param);
        while((next = find_next_storage(next)) != NULL)
        {
            if (is_available_storage(next))
                func(next, param);
        }
    }
}

static void flash_write(uint8_t *data, int len)
{
    for(int i = 0; i < len; i++)
    {
        page_buffer[off_in_page++] = data[i];

        if (off_in_page == ecd_flash_op_get_page_size(flash_op_ctx))
        {
            ecd_flash_op_write_page(flash_op_ctx, cur_page_addr, page_buffer, 1);
            cur_page_addr += ecd_flash_op_get_page_size(flash_op_ctx);
            off_in_page = 0;
        }
    }
}

static void flash_write_flush()
{
    if (off_in_page > 0)
    {
        ecd_flash_op_write_page(flash_op_ctx, cur_page_addr, page_buffer, 1);
        cur_page_addr += ecd_flash_op_get_page_size(flash_op_ctx);
        off_in_page = 0;
    }
}

static void core_file_flash_write(uint8_t *data, int len)
{
    flash_write(data, len);
}

int ecd_faultdump()
{
    int32_t err;
    struct thread_info_ops ops;
    const persistent_format_t * avail_location;

    err = ecd_flash_op_init(&flash_op_ctx);
    if (err != ECD_OK)
    {
        ecd_log_err("faultdump, init flash ops fail");
        return err;
    }

    ecd_init(1, core_file_flash_write);
    uint32_t need_size = ecd_mini_dump_size();

    avail_location = find_free_storage(need_size);

    if (NULL == avail_location)
    {
        ecd_log_err("faultdump, no space to store more core-file");
        return ECD_ERROR;
    }

    cur_page_addr = (uint32_t)avail_location;
    off_in_page = 0;

    persistent_format_t s;
    s.magic = MY_MAGIC_NUMBER;
    s.page_size = ecd_flash_op_get_page_size(flash_op_ctx);
    s.file_size = need_size;
    flash_write((uint8_t *)&s, ELEM_OFFSET(persistent_format_t, corefile));

    ecd_mini_dump_ops(&ops);
    ecd_gen_coredump(&ops);

    flash_write_flush();

    return ECD_OK;
}

struct get_corefile_info {
    const uint8_t ** corefile;
    uint32_t *len;
    int index;
    int counter;
};

static void get_corefile(const persistent_format_t * core, void * param)
{
    struct get_corefile_info *info = (struct get_corefile_info *)param;
    if (info->index == info->counter)
    {
        *info->corefile = &core->corefile[0];
        *info->len = core->file_size;
    }
    info->counter++;
}

void ecd_get_fault_file(const uint8_t ** core_data, uint32_t *len, int index)
{
    struct get_corefile_info info;
    info.counter = 0;
    info.corefile = core_data;
    info.len = len;
    info.index = index;
    *core_data = NULL;
    *len = 0;
    iter_coredump_files(get_corefile, &info);
}

static void count_corefile(const persistent_format_t * core, void * param)
{
    (*((int *)param))++;
}

int ecd_get_fault_file_count()
{
    int count = 0;
    iter_coredump_files(count_corefile, &count);
    return count;
}

__WEAK int32_t ecd_flash_op_init(void ** ctx)
{
    return -1;
}
__WEAK uint32_t ecd_flash_op_get_page_size(void * ctx)
{
    return 0;
}
__WEAK const uint8_t * ecd_flash_op_get_start_addr(void * ctx)
{
    return NULL;
}
__WEAK uint32_t ecd_flash_op_get_total_size(void * ctx)
{
    return 0;
}
__WEAK int32_t ecd_flash_op_write_page(void * ctx, uint32_t page_addr, uint8_t *buff, uint32_t page_nr)
{
    return -1;
}
__WEAK void ecd_log_err(char * format, ...)
{

}
