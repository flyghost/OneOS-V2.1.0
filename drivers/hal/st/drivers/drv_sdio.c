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
 * @file        drv_sdio.c
 *
 * @brief       This file implements SDIO driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <os_event.h>
#include <os_clock.h>
#include <device.h>
#include <os_memory.h>
#include <sdio/mmcsd_core.h>
#include <sdio/sdio.h>
#include <string.h>
#include <drv_common.h>

#define DBG_TAG "drv.sdio"
#include <dlog.h>

#define SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS (100000)

#define OSHW_SDIO_LOCK(_sdio)   os_mutex_recursive_lock(&_sdio->mutex, OS_WAIT_FOREVER)
#define OSHW_SDIO_UNLOCK(_sdio) os_mutex_recursive_unlock(&_sdio->mutex);

#if defined(SDIO) || defined(SDIO1) || defined(SDIO2)
#define SDCARD_INSTANCE_BUS_WIDE_1B SDIO_BUS_WIDE_1B
#define SDCARD_INSTANCE_BUS_WIDE_4B SDIO_BUS_WIDE_4B
#elif defined(SDMMC) || defined(SDMMC1) || defined(SDMMC2)
#define SDCARD_INSTANCE_BUS_WIDE_1B SDMMC_BUS_WIDE_1B
#define SDCARD_INSTANCE_BUS_WIDE_4B SDMMC_BUS_WIDE_4B
#else
#error: no sdio and sdmmc
#endif

#ifndef SDIO_BUFF_SIZE
#define SDIO_BUFF_SIZE (4096)
#endif

#ifndef SDIO_ALIGN_LEN
#define SDIO_ALIGN_LEN (32)
#endif

#ifndef SDIO_MAX_FREQ
#define SDIO_MAX_FREQ (24 * 1000 * 1000)
#endif

#define HW_SDIO_IT_CCRCFAIL (0x01U << 0)
#define HW_SDIO_IT_DCRCFAIL (0x01U << 1)
#define HW_SDIO_IT_CTIMEOUT (0x01U << 2)
#define HW_SDIO_IT_DTIMEOUT (0x01U << 3)
#define HW_SDIO_IT_TXUNDERR (0x01U << 4)
#define HW_SDIO_IT_RXOVERR  (0x01U << 5)
#define HW_SDIO_IT_CMDREND  (0x01U << 6)
#define HW_SDIO_IT_CMDSENT  (0x01U << 7)
#define HW_SDIO_IT_DATAEND  (0x01U << 8)
#define HW_SDIO_IT_STBITERR (0x01U << 9)
#define HW_SDIO_IT_DBCKEND  (0x01U << 10)
#define HW_SDIO_IT_CMDACT   (0x01U << 11)
#define HW_SDIO_IT_TXACT    (0x01U << 12)
#define HW_SDIO_IT_RXACT    (0x01U << 13)
#define HW_SDIO_IT_TXFIFOHE (0x01U << 14)
#define HW_SDIO_IT_RXFIFOHF (0x01U << 15)
#define HW_SDIO_IT_TXFIFOF  (0x01U << 16)
#define HW_SDIO_IT_RXFIFOF  (0x01U << 17)
#define HW_SDIO_IT_TXFIFOE  (0x01U << 18)
#define HW_SDIO_IT_RXFIFOE  (0x01U << 19)
#define HW_SDIO_IT_TXDAVL   (0x01U << 20)
#define HW_SDIO_IT_RXDAVL   (0x01U << 21)
#define HW_SDIO_IT_SDIOIT   (0x01U << 22)

#define HW_SDIO_ERRORS \
    (HW_SDIO_IT_CCRCFAIL | HW_SDIO_IT_CTIMEOUT | \
     HW_SDIO_IT_DCRCFAIL | HW_SDIO_IT_DTIMEOUT | \
     HW_SDIO_IT_RXOVERR  | HW_SDIO_IT_TXUNDERR)

#define HW_SDIO_POWER_OFF (0x00U)
#define HW_SDIO_POWER_UP  (0x02U)
#define HW_SDIO_POWER_ON  (0x03U)

#define HW_SDIO_FLOW_ENABLE   (0x01U << 14)
#define HW_SDIO_BUSWIDE_1B    (0x00U << 11)
#define HW_SDIO_BUSWIDE_4B    (0x01U << 11)
#define HW_SDIO_BUSWIDE_8B    (0x02U << 11)
#define HW_SDIO_BYPASS_ENABLE (0x01U << 10)
#define HW_SDIO_IDLE_ENABLE   (0x01U << 9)
#define HW_SDIO_CLK_ENABLE    (0x01U << 8)

#define HW_SDIO_SUSPEND_CMD    (0x01U << 11)
#define HW_SDIO_CPSM_ENABLE    (0x01U << 10)
#define HW_SDIO_WAIT_END       (0x01U << 9)
#define HW_SDIO_WAIT_INT       (0x01U << 8)
#define HW_SDIO_RESPONSE_NO    (0x00U << 6)
#define HW_SDIO_RESPONSE_SHORT (0x01U << 6)
#define HW_SDIO_RESPONSE_LONG  (0x03U << 6)

#define HW_SDIO_DATA_LEN_MASK (0x01FFFFFFU)

#define HW_SDIO_IO_ENABLE        (0x01U << 11)
#define HW_SDIO_RWMOD_CK         (0x01U << 10)
#define HW_SDIO_RWSTOP_ENABLE    (0x01U << 9)
#define HW_SDIO_RWSTART_ENABLE   (0x01U << 8)
#define HW_SDIO_DBLOCKSIZE_1     (0x00U << 4)
#define HW_SDIO_DBLOCKSIZE_2     (0x01U << 4)
#define HW_SDIO_DBLOCKSIZE_4     (0x02U << 4)
#define HW_SDIO_DBLOCKSIZE_8     (0x03U << 4)
#define HW_SDIO_DBLOCKSIZE_16    (0x04U << 4)
#define HW_SDIO_DBLOCKSIZE_32    (0x05U << 4)
#define HW_SDIO_DBLOCKSIZE_64    (0x06U << 4)
#define HW_SDIO_DBLOCKSIZE_128   (0x07U << 4)
#define HW_SDIO_DBLOCKSIZE_256   (0x08U << 4)
#define HW_SDIO_DBLOCKSIZE_512   (0x09U << 4)
#define HW_SDIO_DBLOCKSIZE_1024  (0x0AU << 4)
#define HW_SDIO_DBLOCKSIZE_2048  (0x0BU << 4)
#define HW_SDIO_DBLOCKSIZE_4096  (0x0CU << 4)
#define HW_SDIO_DBLOCKSIZE_8192  (0x0DU << 4)
#define HW_SDIO_DBLOCKSIZE_16384 (0x0EU << 4)
#define HW_SDIO_DMA_ENABLE       (0x01U << 3)
#define HW_SDIO_STREAM_ENABLE    (0x01U << 2)
#define HW_SDIO_TO_HOST          (0x01U << 1)
#define HW_SDIO_DPSM_ENABLE      (0x01U << 0)

#define HW_SDIO_DATATIMEOUT (0xF0000000U)

struct sdio_pkg
{
    struct os_mmcsd_cmd *cmd;
    void                *buff;
    os_uint32_t          flag;
};

struct stm32_sdio
{
    struct os_mmcsd_host *host;
    struct os_event       event;
    struct os_mutex       mutex;
    struct sdio_pkg      *pkg;

    SD_HandleTypeDef     *hsd;
};

static struct os_mmcsd_host *stm32_sdmmc_host1;
static struct os_mmcsd_host *stm32_sdmmc_host2;

OS_ALIGN(SDIO_ALIGN_LEN)
static os_uint8_t cache_buf[SDIO_BUFF_SIZE];

/**
 ***********************************************************************************************************************
 * @brief           get_order:This function get order from sdio.
 *
 * @param[in]       data
 *
 * @return          Return sdio order.
 ***********************************************************************************************************************
 */
static int get_order(os_uint32_t data)
{
    int order = 0;

    switch (data)
    {
    case 1:
        order = 0;
        break;
    case 2:
        order = 1;
        break;
    case 4:
        order = 2;
        break;
    case 8:
        order = 3;
        break;
    case 16:
        order = 4;
        break;
    case 32:
        order = 5;
        break;
    case 64:
        order = 6;
        break;
    case 128:
        order = 7;
        break;
    case 256:
        order = 8;
        break;
    case 512:
        order = 9;
        break;
    case 1024:
        order = 10;
        break;
    case 2048:
        order = 11;
        break;
    case 4096:
        order = 12;
        break;
    case 8192:
        order = 13;
        break;
    case 16384:
        order = 14;
        break;
    default:
        order = 0;
        break;
    }

    return order;
}

/**
 ***********************************************************************************************************************
 * @brief           This function wait sdio completed.
 *
 * @param[in]       sdio            stm32_sdio
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void stm32_sdio_wait_completed(struct stm32_sdio *st_sdio)
{
    os_uint32_t           status;
    struct os_mmcsd_cmd  *cmd     = st_sdio->pkg->cmd;
    struct os_mmcsd_data *data    = cmd->data;
    SD_TypeDef           *hw_sdio = st_sdio->hsd->Instance;

    if (os_event_recv(&st_sdio->event,
                      0xffffffff,
                      OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                      os_tick_from_ms(5000),
                      &status) != OS_EOK)
    {
        LOG_E(DBG_TAG,"wait completed timeout");
        cmd->err = OS_ETIMEOUT;
        return;
    }

    if (st_sdio->pkg == OS_NULL)
    {
        return;
    }

    cmd->resp[0] = hw_sdio->RESP1;
    cmd->resp[1] = hw_sdio->RESP2;
    cmd->resp[2] = hw_sdio->RESP3;
    cmd->resp[3] = hw_sdio->RESP4;

    if (status & HW_SDIO_ERRORS)
    {
        if ((status & HW_SDIO_IT_CCRCFAIL) && (resp_type(cmd) & (RESP_R3 | RESP_R4)))
        {
            cmd->err = OS_EOK;
        }
        else
        {
            cmd->err = OS_ERROR;
        }

        if (status & HW_SDIO_IT_CTIMEOUT)
        {
            cmd->err = OS_ETIMEOUT;
        }

        if (status & HW_SDIO_IT_DCRCFAIL)
        {
            data->err = OS_ERROR;
        }

        if (status & HW_SDIO_IT_DTIMEOUT)
        {
            data->err = OS_ETIMEOUT;
        }

        if (cmd->err == OS_EOK)
        {
            LOG_D(DBG_TAG, "sta:0x%08X [%08X %08X %08X %08X]",
                      status,
                      cmd->resp[0],
                      cmd->resp[1],
                      cmd->resp[2],
                      cmd->resp[3]);
        }
        else
        {
            LOG_D(DBG_TAG, "err:0x%08x, %s%s%s%s%s%s%s cmd:%d arg:0x%08x rw:%c len:%d blksize:%d",
                      status,
                      status & HW_SDIO_IT_CCRCFAIL ? "CCRCFAIL " : "",
                      status & HW_SDIO_IT_DCRCFAIL ? "DCRCFAIL " : "",
                      status & HW_SDIO_IT_CTIMEOUT ? "CTIMEOUT " : "",
                      status & HW_SDIO_IT_DTIMEOUT ? "DTIMEOUT " : "",
                      status & HW_SDIO_IT_TXUNDERR ? "TXUNDERR " : "",
                      status & HW_SDIO_IT_RXOVERR ? "RXOVERR " : "",
                      status == 0 ? "NULL" : "",
                      cmd->cmd_code,
                      cmd->arg,
                      data ? (data->flags & DATA_DIR_WRITE ? 'w' : 'r') : '-',
                      data ? data->blks * data->blksize : 0,
                      data ? data->blksize : 0);
        }
    }
    else
    {
        cmd->err = OS_EOK;
        LOG_D(DBG_TAG, "sta:0x%08X [%08X %08X %08X %08X]", status, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    }
}

static os_uint32_t stm32_sdio_clock(SD_HandleTypeDef *hsd)
{
#if defined(__STM32F1XX_H) || defined(__STM32L1XX_H)
    return HAL_RCC_GetHCLKFreq() / 2;
#elif defined(__HAL_RCC_GET_SDIO_SOURCE)
    os_uint32_t clock_source = __HAL_RCC_GET_SDIO_SOURCE();

#ifdef RCC_SDIOCLKSOURCE_CLK48
    if (clock_source == RCC_SDIOCLKSOURCE_CLK48)
    {
        return 48000000;
    }
    else
#endif
    {
        return HAL_RCC_GetSysClockFreq();
    }
#elif defined(RCC_CLOCKTYPE_PCLK2)
    return HAL_RCC_GetPCLK2Freq();
#else
    return HAL_RCC_GetPCLK1Freq();
#endif
}

static os_err_t stm32_sdio_dma_send(SD_HandleTypeDef *hsd, os_uint32_t *src, os_uint32_t *dst, int Size)
{
    OS_ASSERT(hsd->hdmatx);

    hsd->hdmatx->Init.Direction = DMA_MEMORY_TO_PERIPH;
    HAL_DMA_DeInit(hsd->hdmatx);
    if (HAL_DMA_Init(hsd->hdmatx) != HAL_OK)
    {
        LOG_E(DBG_TAG,"tx dma init failed.");
        return OS_ERROR;
    }

    if (HAL_DMA_Start(hsd->hdmatx, (uint32_t)src, (uint32_t)dst, Size / 4) != HAL_OK)
    {
        LOG_E(DBG_TAG,"tx dma start failed.");
        return OS_ERROR;
    }
    
    return OS_EOK;
}

static os_err_t stm32_sdio_dma_recv(SD_HandleTypeDef *hsd, os_uint32_t *src, os_uint32_t *dst, int Size)
{
    OS_ASSERT(hsd->hdmarx);    

    hsd->hdmarx->Init.Direction = DMA_PERIPH_TO_MEMORY;
    HAL_DMA_DeInit(hsd->hdmarx);
    if (HAL_DMA_Init(hsd->hdmarx) != HAL_OK)
    {
        LOG_E(DBG_TAG,"rx dma init failed.");
        return OS_ERROR;
    }

    if (HAL_DMA_Start(hsd->hdmarx, (uint32_t)src, (uint32_t)dst, Size / 4) != HAL_OK)
    {
        LOG_E(DBG_TAG,"rx dma start failed.");
        return OS_ERROR;
    }
    
    return OS_EOK;
}


static void stm32_sdio_transfer_by_dma(struct stm32_sdio *st_sdio, struct sdio_pkg *pkg)
{
    struct os_mmcsd_data *data;

    int   size;
    void *buff;
    SD_TypeDef *hw_sdio;

    if ((OS_NULL == pkg) || (OS_NULL == st_sdio))
    {
        LOG_E(DBG_TAG,"stm32_sdio_transfer_by_dma invalid args");
        return;
    }

    data = pkg->cmd->data;
    if (OS_NULL == data)
    {
        LOG_E(DBG_TAG,"stm32_sdio_transfer_by_dma invalid args");
        return;
    }

    buff = pkg->buff;
    if (OS_NULL == buff)
    {
        LOG_E(DBG_TAG,"stm32_sdio_transfer_by_dma invalid args");
        return;
    }
    hw_sdio = st_sdio->hsd->Instance;
    size    = data->blks * data->blksize;

    if (data->flags & DATA_DIR_WRITE)
    {
        stm32_sdio_dma_send(st_sdio->hsd, (os_uint32_t *)buff, (os_uint32_t *)&hw_sdio->FIFO, size);
        hw_sdio->DCTRL |= HW_SDIO_DMA_ENABLE;
    }
    else if (data->flags & DATA_DIR_READ)
    {
        stm32_sdio_dma_recv(st_sdio->hsd, (os_uint32_t *)&hw_sdio->FIFO, (os_uint32_t *)buff, size);
        hw_sdio->DCTRL |= HW_SDIO_DMA_ENABLE | HW_SDIO_DPSM_ENABLE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function send command.
 *
 * @param[in]       sdio            stm32_sdio
 * @param[in]       pkg             sdio package
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void stm32_sdio_send_command(struct stm32_sdio *st_sdio, struct sdio_pkg *pkg)
{
    struct os_mmcsd_cmd  *cmd     = pkg->cmd;
    struct os_mmcsd_data *data    = cmd->data;
    SD_TypeDef           *hw_sdio = st_sdio->hsd->Instance;
    os_uint32_t           reg_cmd;

    /* save pkg */
    st_sdio->pkg = pkg;

    LOG_D(DBG_TAG, "CMD:%d ARG:0x%08x RES:%s%s%s%s%s%s%s%s%s rw:%c len:%d blksize:%d",
              cmd->cmd_code,
              cmd->arg,
              resp_type(cmd) == RESP_NONE ? "NONE" : "",
              resp_type(cmd) == RESP_R1 ? "R1" : "",
              resp_type(cmd) == RESP_R1B ? "R1B" : "",
              resp_type(cmd) == RESP_R2 ? "R2" : "",
              resp_type(cmd) == RESP_R3 ? "R3" : "",
              resp_type(cmd) == RESP_R4 ? "R4" : "",
              resp_type(cmd) == RESP_R5 ? "R5" : "",
              resp_type(cmd) == RESP_R6 ? "R6" : "",
              resp_type(cmd) == RESP_R7 ? "R7" : "",
              data ? (data->flags & DATA_DIR_WRITE ? 'w' : 'r') : '-',
              data ? data->blks * data->blksize : 0,
              data ? data->blksize : 0);

    /* config cmd reg */
    reg_cmd = cmd->cmd_code | HW_SDIO_CPSM_ENABLE;
    if (resp_type(cmd) == RESP_NONE)
        reg_cmd |= HW_SDIO_RESPONSE_NO;
    else if (resp_type(cmd) == RESP_R2)
        reg_cmd |= HW_SDIO_RESPONSE_LONG;
    else
        reg_cmd |= HW_SDIO_RESPONSE_SHORT;

    /* config data reg */
    if (data != OS_NULL)
    {
        os_uint32_t dir  = 0;
        os_uint32_t size = data->blks * data->blksize;
        int         order;

        hw_sdio->DCTRL  = 0;
        hw_sdio->DTIMER = HW_SDIO_DATATIMEOUT;
        hw_sdio->DLEN   = size;
        order           = get_order(data->blksize);
        dir             = (data->flags & DATA_DIR_READ) ? HW_SDIO_TO_HOST : 0;
        hw_sdio->DCTRL  = HW_SDIO_IO_ENABLE | (order << 4) | dir;
    }

    /* transfer config */
    if (data != OS_NULL)
    {
        stm32_sdio_transfer_by_dma(st_sdio, pkg);
    }

    /* open irq */
    hw_sdio->MASK |= HW_SDIO_IT_CMDSENT | HW_SDIO_IT_CMDREND | HW_SDIO_ERRORS;
    if (data != OS_NULL)
    {
        hw_sdio->MASK |= HW_SDIO_IT_DATAEND;
    }

    /* send cmd */
    hw_sdio->ARG = cmd->arg;
    hw_sdio->CMD = reg_cmd;

    /* wait completed */
    stm32_sdio_wait_completed(st_sdio);

    /* Waiting for data to be sent to completion */
    if (data != OS_NULL)
    {
        volatile os_uint32_t count = SDIO_TX_RX_COMPLETE_TIMEOUT_LOOPS;

        while (count && (hw_sdio->STA & (HW_SDIO_IT_TXACT | HW_SDIO_IT_RXACT)))
        {
            count--;
        }

        if ((count == 0) || (hw_sdio->STA & HW_SDIO_ERRORS))
        {
            cmd->err = OS_ERROR;
        }
    }

    /* close irq, keep sdio irq */
    hw_sdio->MASK = hw_sdio->MASK & HW_SDIO_IT_SDIOIT ? HW_SDIO_IT_SDIOIT : 0x00;

    /* clear pkg */
    st_sdio->pkg = OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           This function send sdio request.
 *
 * @param[in]       sdio            stm32_sdio
 * @param[in]       req             request
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void stm32_sdio_request(struct os_mmcsd_host *host, struct os_mmcsd_req *req)
{
    struct sdio_pkg       pkg;
    struct stm32_sdio    *st_sdio = host->private_data;
    struct os_mmcsd_data *data;

    OSHW_SDIO_LOCK(st_sdio);
#ifdef OS_USING_TICKLESS_LPMGR
    os_lpmgr_request(SYS_SLEEP_MODE_NONE);
#endif
    if (req->cmd != OS_NULL)
    {
        memset(&pkg, 0, sizeof(pkg));
        data    = req->cmd->data;
        pkg.cmd = req->cmd;

        if (data != OS_NULL)
        {
            os_uint32_t size = data->blks * data->blksize;

            OS_ASSERT(size <= SDIO_BUFF_SIZE);

            pkg.buff = data->buf;
            if ((os_uint32_t)data->buf & (SDIO_ALIGN_LEN - 1))
            {
                pkg.buff = cache_buf;
                if (data->flags & DATA_DIR_WRITE)
                {
                    memcpy(cache_buf, data->buf, size);
                }
            }
        }

        stm32_sdio_send_command(st_sdio, &pkg);

        if ((data != OS_NULL) && (data->flags & DATA_DIR_READ) && ((os_uint32_t)data->buf & (SDIO_ALIGN_LEN - 1)))
        {
            memcpy(data->buf, cache_buf, data->blksize * data->blks);
        }
    }

    if (req->stop != OS_NULL)
    {
        memset(&pkg, 0, sizeof(pkg));
        pkg.cmd = req->stop;
        stm32_sdio_send_command(st_sdio, &pkg);
    }
#ifdef OS_USING_TICKLESS_LPMGR
    os_lpmgr_release(SYS_SLEEP_MODE_NONE);
#endif
    OSHW_SDIO_UNLOCK(st_sdio);

    mmcsd_req_complete(st_sdio->host);
}

/**
 ***********************************************************************************************************************
 * @brief           This function config sdio.
 *
 * @param[in]       host            os_mmcsd_host
 * @param[in]       io_cfg          os_mmcsd_io_cfg
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void stm32_sdio_iocfg(struct os_mmcsd_host *host, struct os_mmcsd_io_cfg *io_cfg)
{
    os_uint32_t        clkcr, div, clk_src;
    os_uint32_t        clk     = io_cfg->clock;
    struct stm32_sdio *st_sdio = host->private_data;
    SD_TypeDef        *hw_sdio = st_sdio->hsd->Instance;

    clk_src = stm32_sdio_clock(st_sdio->hsd);
    if (clk_src < 400 * 1000)
    {
        LOG_E(DBG_TAG,"The clock rate is too low! rata:%d", clk_src);
        return;
    }

    if (clk > host->freq_max)
        clk = host->freq_max;

    if (clk > clk_src)
    {
        LOG_W(DBG_TAG,"Setting rate is greater than clock source rate.");
        clk = clk_src;
    }

    LOG_D(DBG_TAG, "clk:%d width:%s%s%s power:%s%s%s",
              clk,
              io_cfg->bus_width == MMCSD_BUS_WIDTH_8 ? "8" : "",
              io_cfg->bus_width == MMCSD_BUS_WIDTH_4 ? "4" : "",
              io_cfg->bus_width == MMCSD_BUS_WIDTH_1 ? "1" : "",
              io_cfg->power_mode == MMCSD_POWER_OFF ? "OFF" : "",
              io_cfg->power_mode == MMCSD_POWER_UP ? "UP" : "",
              io_cfg->power_mode == MMCSD_POWER_ON ? "ON" : "");

    OSHW_SDIO_LOCK(st_sdio);

    if (clk != 0)
    {
        div = clk_src / clk;
    }
    
    if ((clk == 0) || (div == 0))
    {
        clkcr = 0;
    }
    else
    {
        if (div < 2)
        {
            div = 2;
        }
        else if (div > 0xFF)
        {
            div = 0xFF;
        }
        div -= 2;
        clkcr = div | HW_SDIO_CLK_ENABLE;
    }

    if (io_cfg->bus_width == MMCSD_BUS_WIDTH_8)
    {
        clkcr |= HW_SDIO_BUSWIDE_8B;
    }
    else if (io_cfg->bus_width == MMCSD_BUS_WIDTH_4)
    {
        clkcr |= HW_SDIO_BUSWIDE_4B;
    }
    else
    {
        clkcr |= HW_SDIO_BUSWIDE_1B;
    }

    hw_sdio->CLKCR = clkcr | HW_SDIO_IDLE_ENABLE;

    switch (io_cfg->power_mode)
    {
    case MMCSD_POWER_OFF:
        hw_sdio->POWER = HW_SDIO_POWER_OFF;
        break;
    case MMCSD_POWER_UP:
        hw_sdio->POWER = HW_SDIO_POWER_UP;
        break;
    case MMCSD_POWER_ON:
        hw_sdio->POWER = HW_SDIO_POWER_ON;
        break;
    default:
        LOG_W(DBG_TAG,"unknown power_mode %d", io_cfg->power_mode);
        break;
    }

    OSHW_SDIO_UNLOCK(st_sdio);
}

/**
 ***********************************************************************************************************************
 * @brief           This function update sdio interrupt.
 *
 * @param[in]       host            os_mmcsd_host
 * @param[in]       enable
 *
 * @return          none
 ***********************************************************************************************************************
 */
void stm32_sdio_enable_irq(struct os_mmcsd_host *host, os_int32_t enable)
{
    struct stm32_sdio *st_sdio = host->private_data;

    if (enable)
    {
        LOG_D(DBG_TAG, "enable sdio irq");
        st_sdio->hsd->Instance->MASK |= HW_SDIO_IT_SDIOIT;
    }
    else
    {
        LOG_D(DBG_TAG, "disable sdio irq");
        st_sdio->hsd->Instance->MASK &= ~HW_SDIO_IT_SDIOIT;
    }
}

static os_int32_t stm32_sdio_detect(struct os_mmcsd_host *host)
{
    LOG_D(DBG_TAG, "try to detect device");
    return 0x01;
}

static void stm32_sdio_irq(struct os_mmcsd_host *host)
{
    int                complete  = 0;
    struct stm32_sdio *st_sdio      = host->private_data;
    SD_TypeDef        *hw_sdio = st_sdio->hsd->Instance;
    os_uint32_t        intstatus = hw_sdio->STA;

    if (intstatus & HW_SDIO_ERRORS)
    {
        hw_sdio->ICR = HW_SDIO_ERRORS;
        complete     = 1;
    }
    else
    {
        if (intstatus & HW_SDIO_IT_CMDREND)
        {
            hw_sdio->ICR = HW_SDIO_IT_CMDREND;

            if (st_sdio->pkg != OS_NULL)
            {
                if (!st_sdio->pkg->cmd->data)
                {
                    complete = 1;
                }
                else if ((st_sdio->pkg->cmd->data->flags & DATA_DIR_WRITE))
                {
                    hw_sdio->DCTRL |= HW_SDIO_DPSM_ENABLE;
                }
            }
        }

        if (intstatus & HW_SDIO_IT_CMDSENT)
        {
            hw_sdio->ICR = HW_SDIO_IT_CMDSENT;

            if (st_sdio->pkg != OS_NULL && resp_type(st_sdio->pkg->cmd) == RESP_NONE)
            {
                complete = 1;
            }
        }

        if (intstatus & HW_SDIO_IT_DATAEND)
        {
            hw_sdio->ICR = HW_SDIO_IT_DATAEND;
            complete     = 1;
        }
    }

    if ((intstatus & HW_SDIO_IT_SDIOIT) && (hw_sdio->MASK & HW_SDIO_IT_SDIOIT))
    {
        hw_sdio->ICR = HW_SDIO_IT_SDIOIT;
        sdio_irq_wakeup(host);
    }

    if (complete)
    {
        hw_sdio->MASK &= ~HW_SDIO_ERRORS;
        os_event_send(&st_sdio->event, intstatus);
    }
}

void SDMMC1_IRQHandler(void)
{

    stm32_sdio_irq(stm32_sdmmc_host1);

}

void SDMMC2_IRQHandler(void)
{

    stm32_sdio_irq(stm32_sdmmc_host2);

}

static const struct os_mmcsd_host_ops ops = {
    stm32_sdio_request,
    stm32_sdio_iocfg,
    stm32_sdio_detect,
    stm32_sdio_enable_irq,
};

/**
 ***********************************************************************************************************************
 * @brief           This function create mmcsd host.
 *
 * @param[in]       sdio_des        stm32_sdio_des
 *
 * @return          Return the pointer of os_mmcsd_host.
 ***********************************************************************************************************************
 */
static struct os_mmcsd_host *stm32_sdio_host_create(struct stm32_sdio *st_sdio)
{
    struct os_mmcsd_host *host;

    OS_ASSERT(st_sdio);
    OS_ASSERT(st_sdio->hsd);

    host = mmcsd_alloc_host();
    if (host == OS_NULL)
    {
        LOG_E(DBG_TAG,"L:%d F:%s mmcsd alloc host fail");
        return OS_NULL;
    }

    os_event_init(&st_sdio->event, "sdio");
    os_mutex_init(&st_sdio->mutex, "sdio", OS_TRUE);

    /* set host defautl attributes */
    host->ops       = &ops;
    host->freq_min  = 400 * 1000;
    host->freq_max  = SDIO_MAX_FREQ;
    host->valid_ocr = 0x00FFFF80 | VDD_32_33 | VDD_33_34;

    if (st_sdio->hsd->Init.BusWide == SDCARD_INSTANCE_BUS_WIDE_4B)
    {
        host->flags = MMCSD_MUTBLKWRITE | MMCSD_SUP_SDIO_IRQ | MMCSD_BUSWIDTH_4;
    }
    else
    {
        host->flags = MMCSD_MUTBLKWRITE | MMCSD_SUP_SDIO_IRQ;
    }

    host->max_seg_size  = SDIO_BUFF_SIZE;
    host->max_dma_segs  = 1;
    host->max_blk_size  = 512;
    host->max_blk_count = 512;

    /* link up host and sdio */
    st_sdio->host      = host;
    host->private_data = st_sdio;

    stm32_sdio_enable_irq(host, 1);

    /* ready to change */
    mmcsd_change(host);

    return host;
}

void stm32_mmcsd1_change(void)
{
    mmcsd_change(stm32_sdmmc_host1);
}

void stm32_mmcsd2_change(void)
{
    mmcsd_change(stm32_sdmmc_host2);
}

static int stm32_sdio_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_sdio    *st_sdio;
    SD_HandleTypeDef     *hsd;
    struct os_mmcsd_host *host;

    hsd = (SD_HandleTypeDef *)dev->info;

    OS_ASSERT(hsd->hdmatx != OS_NULL || hsd->hdmarx != OS_NULL);

    /* I think this is stm32 sdio dma bug: only support single dma channel */
    if (hsd->hdmatx == OS_NULL)
        hsd->hdmatx = hsd->hdmarx;
    else if (hsd->hdmarx == OS_NULL)
        hsd->hdmarx = hsd->hdmatx;
    else if (hsd->hdmatx->Instance < hsd->hdmarx->Instance)
        hsd->hdmatx = hsd->hdmarx;
    else if (hsd->hdmatx->Instance > hsd->hdmarx->Instance)
        hsd->hdmarx = hsd->hdmatx;

    st_sdio = os_calloc(1, sizeof(struct stm32_sdio));
    OS_ASSERT(st_sdio);

    st_sdio->hsd = hsd;

    host = stm32_sdio_host_create(st_sdio);
    OS_ASSERT(host);

#ifdef SDIO
    if (hsd->Instance == SDIO)
    {
        stm32_sdmmc_host1 = host;
        LOG_I(DBG_TAG,"sdmmc host1.");
        return OS_EOK;
    }
#endif

#ifdef SDMMC1
    if (hsd->Instance == SDMMC1)
    {
        stm32_sdmmc_host1 = host;
        LOG_I(DBG_TAG,"sdmmc host1.");
        return OS_EOK;
    }
#endif
    
#ifdef SDMMC2
    else if (hsd->Instance == SDMMC2)
    {
        stm32_sdmmc_host2 = host;
        LOG_I(DBG_TAG,"sdmmc host2.");
        return OS_EOK;
    }
#endif

    LOG_E(DBG_TAG,"invalid sdmmc host.");
    return OS_ERROR;
}

OS_DRIVER_INFO stm32_sdio_driver = {
    .name   = "SD_HandleTypeDef",
    .probe  = stm32_sdio_probe,
};

OS_DRIVER_DEFINE(stm32_sdio_driver,DEVICE,OS_INIT_SUBLEVEL_MIDDLE);

