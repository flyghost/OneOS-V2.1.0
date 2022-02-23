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
 * @file        sdio_imx.h
 *
 * @brief       This file provides  definition and operation functions declaration for sdio.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_SDIO_H__
#define DRV_SDIO_H__

#include <os_stddef.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <fsl_sdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/***
this struct need user provide! here just give a sample!
some member must have:
***/
typedef void (*oal_sdio_isr_t)(void *userdata);
typedef struct sdio_func
{
    os_uint8_t func_num;
    oal_sdio_isr_t oal_sdio_isr;
}sdio_func_t;

os_uint32_t imxrt_sdio_get_max_block_count(struct sdio_func *func);
os_uint32_t imxrt_sdio_get_max_req_size(struct sdio_func *func);
os_uint32_t imxrt_sdio_get_max_blk_size(struct sdio_func *func);
os_uint32_t imxrt_sdio_en_timeout(struct sdio_func *func);
os_uint32_t imxrt_sdio_func_num(struct sdio_func *func);

struct sdio_func * imxrt_sdio_get_func(void);
void imxrt_sdio_release_irq(struct sdio_func *func);
os_err_t imxrt_sdio_require_irq(struct sdio_func *func);
os_uint8_t imxrt_sdio_read_byte(struct sdio_func *func, unsigned int addr, int *err_ret);
void imxrt_sdio_write_byte(struct sdio_func *func, os_uint8_t buf, unsigned int addr, int *err_ret);
os_uint16_t imxrt_sdio_readw(struct sdio_func *func, unsigned int addr, int *err_ret);
void imxrt_sdio_writew(struct sdio_func *func, os_uint16_t buf, unsigned int addr, int *err_ret);
os_uint32_t imxrt_sdio_readl(struct sdio_func *func, unsigned int addr, int *err_ret);
void imxrt_sdio_writel(struct sdio_func *func, os_uint32_t buf, unsigned int addr, int *err_ret);
os_err_t imxrt_sdio_read_fifo_block(struct sdio_func *func, os_uint8_t *buf, unsigned int addr, os_uint32_t count);
os_err_t imxrt_sdio_write_fifo_block(struct sdio_func *func, unsigned int addr, os_uint8_t *buf, os_uint32_t count);
os_uint8_t imxrt_sdio_f0_readb(struct sdio_func *func, unsigned int addr, int *err_ret);
void imxrt_sdio_f0_writeb(struct sdio_func *func, os_uint8_t buf, unsigned int addr, int *err_ret, bool raw);
os_err_t imxrt_sdio_read_incr_block(struct sdio_func *func, os_uint8_t *buf, unsigned int addr, int count);
os_err_t imxrt_sdio_write_incr_block(struct sdio_func *func, unsigned int addr, os_uint8_t *buf, int count);
os_err_t imxrt_sdio_set_block_size(struct sdio_func *func, os_uint32_t blksz);
os_err_t imxrt_sdio_enable_func(struct sdio_func *func);
os_err_t imxrt_sdio_disable_func(struct sdio_func *func);
void imxrt_mmc_release_card(struct sdio_func *func);
void imxrt_mmc_acquire_card(struct sdio_func *func);
//os_int32_t imxrt_sdio_rw_scat_extended(struct sdio_func *func, os_int32_t write, os_uint32_t fn, os_uint32_t addr, os_int32_t incr_addr, struct scatterlist *sg, os_uint32_t sg_len, os_uint32_t blocks, os_uint32_t blksz);
void imxrt_sdio_force_clkon(struct sdio_func *func, bool enable);

imxrt_sdio_device_t *imxrt_get_sdio_dev(void);
os_err_t imxrt_sdio_dev_init(void);


#ifdef __cplusplus
}
#endif

#endif
