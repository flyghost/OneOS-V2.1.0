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
 * @file        sdio.c
 *
 * @brief       This file provides operation functions for sdio.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_list.h>
#include <os_clock.h>
#include <os_task.h>
#include <os_util.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <device.h>
#include <dlog.h>

#include <drv_sdio.h>
#include <fsl_sdio.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sdio"
#include <drv_log.h>

typedef struct os_imxrt_sdio_device
{
    void *func;
    os_device_t device;
    sdio_card_t card;
    os_sem_t sem;
    os_mutex_t mutex;
    os_task_t *task;
}os_imxrt_sdio_device_t;

void os_imxrt_sdio_delay_ms(os_uint32_t ms)
{
    if (ms < 1000 / OS_TICK_PER_SECOND)
        os_task_msleep(1);
    else
        os_task_msleep(ms * OS_TICK_PER_SECOND / 1000);
}

void os_imxrt_sdio_irq_handler(sdio_card_t *card, os_uint32_t func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (card == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_card is null!");
    }
    sdio_device = os_container_of(card, os_imxrt_sdio_device_t, card);
    
    sdio_device->func.oal_sdio_isr[func](&sdio_device->func);
}

void os_imxrt_sdio_release_irq(sdio_func_t *func)
{   
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    SDIO_EnableIOInterrupt(&sdio_device->card, (sdio_func_num_t)func->func_num, OS_FALSE);
    SDIO_SetIOIRQHandler(&sdio_device->card, (sdio_func_num_t)func->func_num, (sdio_io_irq_handler_t)NULL);
    if (sdio_device->task)
        os_task_destroy(sdio_device->task);
}

static void os_imxrt_sdio_irq_task_entry(void *parameter)
{
    os_imxrt_sdio_device_t *sdio_device = parameter;
    while(1)
    {
        if (os_sem_wait(&sdio_device->sem, OS_WAIT_FOREVER) == OS_EOK)
        {
            SDIO_HandlePendingIOInterrupt(&sdio_device->card);
            SDMMCHOST_EnableCardInt(sdio_device->card.host, true);
        }
    }
}

os_err_t os_imxrt_sdio_require_irq(sdio_func_t *func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return OS_ERROR;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    SDIO_EnableIOInterrupt(&sdio_device->card, (sdio_func_num_t)func->func_num, OS_TRUE);
    SDIO_SetIOIRQHandler(&sdio_device->card, (sdio_func_num_t)func->func_num, os_imxrt_sdio_irq_handler);
    
    sdio_device->task = os_task_create("os_imxrt_sdio_task", os_imxrt_sdio_irq_task_entry, sdio_device, 1024, 3, 5);
    if (sdio_device->task == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "hi_sdio_rx_task creat failed!");
        return OS_ERROR;
    }

    if (os_task_startup(sdio_device->task) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "os_task_startup failed!");
        os_task_destroy(sdio_device->task);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_uint32_t os_imxrt_sdio_get_max_block_count(sdio_func_t *func)
{
    return USDHC_MAX_BLOCK_COUNT;
}

os_uint32_t os_imxrt_sdio_get_max_req_size(sdio_func_t *func)
{
    return 512;
}

os_uint32_t os_imxrt_sdio_get_max_blk_size(sdio_func_t *func)
{
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    if (func->func_num == kSDIO_FunctionNum0)
        return sdio_device->card.io0blockSize;
    else
        return sdio_device->card.ioFBR[(uint32_t)func->func_num - 1U].ioBlockSize;
}

os_uint32_t os_imxrt_sdio_get_cur_blk_size(sdio_func_t *func)
{
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    if (func->func_num == kSDIO_FunctionNum0)
        return sdio_device->card.io0blockSize;
    else
        return sdio_device->card.ioFBR[(uint32_t)func->func_num - 1U].ioBlockSize;
}

os_uint8_t os_imxrt_sdio_read_byte(sdio_func_t *func, unsigned int addr, int *err_ret)
{
    os_uint8_t data = 0;
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }

    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Read_Direct(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, &data);

    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_read_byte failed!");

    if (err_ret)
    {
        *err_ret = ret;
    }
    os_mutex_unlock(&sdio_device->mutex);
    return data;
}

void os_imxrt_sdio_write_byte(sdio_func_t *func, os_uint8_t buf, unsigned int addr, int *err_ret)
{
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return;
    }
    
    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Write_Direct(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, &buf, 0x01);

    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_write_byte failed!");
    
    if (err_ret)
    {
        *err_ret = ret;
    }
    os_mutex_unlock(&sdio_device->mutex);
}

os_uint16_t os_imxrt_sdio_readw(sdio_func_t *func, unsigned int addr, int *err_ret)
{
    os_uint16_t buf = 0;
    os_int32_t  ret;
    os_int32_t  flags = 0;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    flags |= SDIO_EXTEND_CMD_OP_CODE_MASK;
    
    ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)&buf, 2, flags);

    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_readw failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
    return buf;
}

void os_imxrt_sdio_writew(sdio_func_t *func, os_uint16_t buf, unsigned int addr, int *err_ret)
{
    os_int32_t flags = 0;
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    
    flags |= SDIO_EXTEND_CMD_OP_CODE_MASK;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)&buf, 2, flags);
    
    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_writew failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
}

os_uint32_t os_imxrt_sdio_readl(sdio_func_t *func, unsigned int addr, int *err_ret)
{
    os_uint32_t buf = 0;
    os_int32_t  ret;
    os_int32_t  flags = 0;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    flags |= SDIO_EXTEND_CMD_OP_CODE_MASK;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)&buf, 4, flags);
    
    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_readl failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
    return buf;
}

void os_imxrt_sdio_writel(sdio_func_t *func, os_uint32_t buf, unsigned int addr, int *err_ret)
{
    os_int32_t  ret;
    os_int32_t  flags = 0;
    
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    
    flags |= SDIO_EXTEND_CMD_OP_CODE_MASK;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)&buf, 4, flags);
    
    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_writel failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
}

os_uint8_t os_imxrt_sdio_f0_readb(sdio_func_t *func, unsigned int addr, int *err_ret)
{
    os_uint8_t buf = 0;
    os_int32_t ret;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return 0;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Read_Direct(&sdio_device->card, kSDIO_FunctionNum0, addr, &buf);

    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_f0_readb failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
    return buf;
}

void os_imxrt_sdio_f0_writeb(sdio_func_t *func, os_uint8_t buf, unsigned int addr, int *err_ret, bool raw)
{
    int ret;
    
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    ret = SDIO_IO_Write_Direct(&sdio_device->card, kSDIO_FunctionNum0, addr, &buf, raw);

    if (ret != kStatus_Success)
        LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_f0_writeb failed!");
    
    if (err_ret)
        *err_ret = ret;
    os_mutex_unlock(&sdio_device->mutex);
}

os_err_t os_imxrt_sdio_block_transfer(sdio_func_t *func, unsigned int addr, os_uint8_t *buf, os_uint32_t count, os_uint8_t direct, os_uint8_t incflags)
{
    os_int32_t  ret;
    os_uint32_t left_size;
    os_uint32_t max_blks, blks;
    os_uint32_t max_blk_size;
    os_int32_t  flags = 0;

    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return OS_ERROR;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    os_mutex_lock(&sdio_device->mutex, OS_WAIT_FOREVER);
    left_size = count;
    max_blk_size = os_imxrt_sdio_get_cur_blk_size(func);

    if (incflags)
        flags |= SDIO_EXTEND_CMD_OP_CODE_MASK;
        
    if ((sdio_device->card.cccrflags & kSDIO_CCCRSupportMultiBlock) && (count > max_blk_size))
    {
        max_blks = 511u;
        flags |= SDIO_EXTEND_CMD_BLOCK_MODE_MASK;
        
        while (left_size > max_blk_size)
        {
            blks = left_size / max_blk_size;
            if (blks > max_blks)
                blks = max_blks;
            count = blks * max_blk_size;

        if (direct)
        {
            ret = SDIO_IO_Write_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)buf, blks, flags);
            if (ret)
            {
                LOG_E(DRV_EXT_TAG, "SDIO_IO_Write_Extended block failed!\n");
                goto err;
            }
        }
        else
        {
            ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)buf, blks, flags);
            if (ret)
            {
                LOG_E(DRV_EXT_TAG, "SDIO_IO_Read_Extended block failed!\n");
                goto err;
            }
        }

            left_size -= count;
            buf += count;
            if (incflags)
                addr += count;
        }
    }

    while (left_size > 0)
    {
        count = MIN(left_size, max_blk_size);

        if (direct)
        {
            ret = SDIO_IO_Write_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)buf, count, flags);
            if (ret)
            {
                LOG_E(DRV_EXT_TAG, "SDIO_IO_Write_Extended block failed!\n");
                goto err;
            }
        }
        else
        {
            ret = SDIO_IO_Read_Extended(&sdio_device->card, (sdio_func_num_t)func->func_num, addr, (os_uint8_t *)buf, count, flags);
            if (ret)
            {
                LOG_E(DRV_EXT_TAG, "SDIO_IO_Read_Extended block failed!\n");
                goto err;
            }
        }

        left_size -= count;
        buf += count;
        if (incflags)
            addr += count;
    }
    os_mutex_unlock(&sdio_device->mutex);
    return OS_EOK;
err:
    os_mutex_unlock(&sdio_device->mutex);
    return OS_ERROR;
}

os_err_t os_imxrt_sdio_read_fifo_block(sdio_func_t *func, os_uint8_t *buf, unsigned int addr, os_uint32_t count)
{
    return os_imxrt_sdio_block_transfer(func, addr, buf, count, 0, 0);
}

os_err_t os_imxrt_sdio_write_fifo_block(sdio_func_t *func, unsigned int addr, os_uint8_t *buf, os_uint32_t count)
{
    return os_imxrt_sdio_block_transfer(func, addr, buf, count, 1, 0);
}

os_err_t os_imxrt_sdio_read_incr_block(sdio_func_t *func, os_uint8_t *buf, unsigned int addr, int count)
{
    return os_imxrt_sdio_block_transfer(func, addr, buf, count, 0, 1);
}

os_err_t os_imxrt_sdio_write_incr_block(sdio_func_t *func, unsigned int addr, os_uint8_t *buf, int count)
{
    return os_imxrt_sdio_block_transfer(func, addr, buf, count, 1, 1);
}

os_err_t os_imxrt_sdio_set_block_size(sdio_func_t *func, os_uint32_t blksz)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return OS_ERROR;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    return SDIO_SetBlockSize(&sdio_device->card, (sdio_func_num_t)func->func_num, blksz);
}

os_err_t os_imxrt_sdio_enable_func(sdio_func_t *func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return OS_ERROR;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    return SDIO_EnableIO(&sdio_device->card, (sdio_func_num_t)func->func_num, OS_TRUE);
}

os_err_t os_imxrt_sdio_disable_func(sdio_func_t *func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return OS_ERROR;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;
    
    return SDIO_EnableIO(&sdio_device->card, (sdio_func_num_t)func->func_num, OS_FALSE);
}

void imxrt_mmc_release_card(sdio_func_t *func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return ;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    SDIO_Deinit(&sdio_device->card);
    LOG_D(DRV_EXT_TAG, "SDIO_Deinit success!");
}

void imxrt_mmc_acquire_card(sdio_func_t *func)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return ;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    if (SDIO_Init(&sdio_device->card))
        LOG_E(DRV_EXT_TAG, "SDIO_Init failed!");
    else
        LOG_D(DRV_EXT_TAG, "SDIO_Init success!");
}

sdio_func_t *os_imxrt_sdio_get_func(void)
{
    os_kprintf("g_os_imxrt_sdio_device.func %p \r\n", g_os_imxrt_sdio_device.func);
    return &g_os_imxrt_sdio_device.func;
}

void os_imxrt_sdio_force_clkon(sdio_func_t *func, bool enable)
{
    os_imxrt_sdio_device_t *sdio_device = OS_NULL;
    if (func == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sdio_func is null!");
        return ;
    }
    sdio_device = (os_imxrt_sdio_device_t *)func;

    SDMMCHOST_ForceClockOn(sdio_device->card.host, enable);
}

static void os_imxrt_sdio_cardinterruptcb(void *userData)
{
    os_imxrt_sdio_device_t *sdio_device = (os_imxrt_sdio_device_t *)userData;
    SDMMCHOST_EnableCardInt(sdio_device->card.host, false);
    os_sem_post(&sdio_device->sem);
}

os_imxrt_sdio_device_t *imxrt_get_sdio_dev(void)
{
    return &g_os_imxrt_sdio_device;
}

static void imxrt_sdio_cardinterruptcb(void *userData)
{
    sdio_card_t *card = (sdio_card_t *)userData;
    SDMMCHOST_EnableCardInt(card.host, false);
    os_sem_post(&sdio_device->sem);
}

static os_err_t os_imxrt_sdio_init(os_imxrt_sdio_device_t *sdio_dev, os_imxrt_sdio_cfg_param_t *cfg_param, os_imxrt_sdio_sd_cd_param_t *sd_cd_param)
{
    sdio_card_t *card = &sdio_dev->card;
    sdio_card_int_t *sdioInt;
    
    if (os_imxrt_sdio_config(card, OS_SDIO_TYPE_SD, cfg_param, sd_cd_param) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "sdio_config fail");
        return OS_ERROR;
    }
    
    if (SDIO_Init(card) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "SD card init fail");
        return OS_ERROR;
    }
    os_imxrt_sdio_init_end();
    return OS_EOK;
}

static int os_imxrt_sdio_device_init(void)
{
    int ret;
    os_uint8_t sdio_index = 0;
    char sdio_device_name[] = "sdio0";

    ret = os_imxrt_sdio_index_get(OS_SDIO_TYPE_SD, &sdio_index);
    if (ret != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "cannot find sd config param!");
        return OS_ERROR;
    }

    for(;;)
    {
        os_imxrt_sdio_device_t *sdio_device = os_calloc(os_imxrt_sdio_device_t);
        if (sdio_device == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "failed!");
            return OS_ENOMEM;
        }

        os_imxrt_sdio_sd_cd_param_t *sd_cd_param = os_calloc(1, sizeof(os_imxrt_sdio_sd_cd_param_t));
        if (sd_cd_param == OS_NULL)
        {
            os_free(sdio_device);
            LOG_E(DRV_EXT_TAG, "failed!");
            return OS_ENOMEM;
        }
        
        sdio_card_int_t *sdioint = os_calloc(1, sizeof(sdio_card_int_t));
        if (sdioint == OS_NULL)
        {
            os_free(sd_cd_param);
            os_free(sdio_device);
            LOG_E(DRV_EXT_TAG, "failed!");
            return OS_ENOMEM;
        }

        os_imxrt_sdio_cfg_param_t *cfg_param = &sdio_cfg_param[sdio_index];
        cfg_param->buf = os_dma_malloc_align(cfg_param->buf_size, 4);
        
        sdioint->cardInterrupt = imxrt_sdio_cardinterruptcb;
        sdioint->userData = sdio_device;
        sdio_device->card->usrParam.sdioInt = sdioint;
        
        sd_cd_param->sd_cd = OS_NULL;
        sd_cd_param->userData = OS_NULL;
        ret = os_imxrt_sdio_init(sdio_device, cfg_param, sd_cd_param);
        if (ret != OS_EOK)
        {
            os_free(sdioint);
            os_free(sd_cd_param);
            os_free(sdio_device);
            os_dma_free_align(cfg_param->buf);
            LOG_E(DRV_EXT_TAG, "os_imxrt_sdio_init failed!");
            return OS_ERROR;
        }
        
        if (os_sem_init(&sdio_device->sem, "sdio_sem", 0) != OS_EOK)
        {
            LOG_E(DRV_EXT_TAG, "os_sem_init failed!");
            SDIO_Deinit(&sdio_device->card);
            os_free(sdioint);
            os_free(sd_cd_param);
            os_free(sdio_device);
            return OS_ERROR;
        }

        if (os_mutex_init(&sdio_device->mutex, "sdio_mutex", OS_FALSE) != OS_EOK)
        {
            LOG_E(DRV_EXT_TAG, "os_mutex_init failed!");
            os_sem_deinit(&sdio_device->sem);
            SDIO_Deinit(&sdio_device->card);
            os_free(sdioint);
            os_free(sd_cd_param);
            os_free(sdio_device);
            return OS_ERROR;
        }
        SDIO_SetBlockSize(&sdio_device->card, (sdio_func_num_t)0, cfg_param->block_size);
        SDIO_SetBlockSize(&sdio_device->card, (sdio_func_num_t)1, cfg_param->block_size);
        SDIO_SetBlockSize(&sdio_device->card, (sdio_func_num_t)2, cfg_param->block_size);
        
        ret = os_device_register(&sdio_device->device, sdio_device_name);
        if (ret != OS_EOK)
        {
            LOG_E(DRV_EXT_TAG, "os_device_register failed!");
            os_mutex_deinit(&sdio_device->mutex);
            os_sem_deinit(&sdio_device->sem);
            SDIO_Deinit(&sdio_device->card);
            os_free(sdioint);
            os_free(sd_cd_param);
            os_free(sdio_device);
            return OS_ERROR;
        }
        sdio_device_name[4]++;
        sdio_index++;
        ret = os_imxrt_sdio_index_get(OS_SDIO_TYPE_SD, &sdio_index);
        if (ret != OS_EOK)
        {
            break;
        }
    }
    
    LOG_D(DRV_EXT_TAG, "g_os_imxrt_sdio_device success!");
    return OS_EOK;
}

OS_DEVICE_INIT(os_imxrt_sdio_device_init, OS_INIT_SUBLEVEL_LOW);


