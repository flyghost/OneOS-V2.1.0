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
 * @file        block_dev.c
 *
 * @brief       This file provides functions for mmcsd read/wrtie/probe.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_list.h>
#include <os_sem.h>
#include <string.h>
#include <sdio/mmcsd_core.h>
#include <os_memory.h>
#include <dlog.h>
#include <block/block_device.h>

#ifdef OS_SDIO_DEBUG
#define DRV_EXT_LVL DBG_EXT_DEBUG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif /* OS_SDIO_DEBUG */
#define DBG_TAG "block_dev"
#include <drv_log.h>

#define SECTOR_SIZE (512)

#define BLK_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 ***********************************************************************************************************************
 * @struct      mmcsd_blk_device
 *
 * @brief       structure of mmcsd_blk_device
 ***********************************************************************************************************************
 */
struct mmcsd_blk_device
{
    os_blk_device_t blk_dev;
    struct os_mmcsd_card   *card;
    os_off_t    offset;
    os_size_t   max_req_size;
};

#ifndef OS_MMCSD_MAX_PARTITION
#define OS_MMCSD_MAX_PARTITION 16
#endif

os_int32_t mmcsd_num_wr_blocks(struct os_mmcsd_card *card)
{
    os_int32_t  err;
    os_uint32_t blocks;

    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;
    os_uint32_t          timeout_us;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = APP_CMD;
    cmd.arg      = card->rca << 16;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_AC;

    err = mmcsd_send_cmd(card->host, &cmd, 0);
    if (err)
        return OS_ERROR;
    if (!controller_is_spi(card->host) && !(cmd.resp[0] & R1_APP_CMD))
        return OS_ERROR;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_APP_SEND_NUM_WR_BLKS;
    cmd.arg      = 0;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct os_mmcsd_data));

    data.timeout_ns   = card->tacc_ns * 100;
    data.timeout_clks = card->tacc_clks * 100;

    timeout_us = data.timeout_ns / 1000;
    timeout_us += data.timeout_clks * 1000 / (card->host->io_cfg.clock / 1000);

    if (timeout_us > 100000)
    {
        data.timeout_ns   = 100000000;
        data.timeout_clks = 0;
    }

    data.blksize = 4;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = (os_uint8_t *)&blocks;

    memset(&req, 0, sizeof(struct os_mmcsd_req));

    req.cmd  = &cmd;
    req.data = &data;

    mmcsd_send_request(card->host, &req);

    if (cmd.err || data.err)
        return OS_ERROR;

    return blocks;
}

static os_err_t
os_mmcsd_req_blk(struct os_mmcsd_card *card, os_uint32_t sector, void *buf, os_size_t blks, os_uint8_t dir)
{
    struct os_mmcsd_cmd   cmd, stop;
    struct os_mmcsd_data  data;
    struct os_mmcsd_req   req;
    struct os_mmcsd_host *host = card->host;
    os_uint32_t           r_cmd, w_cmd;

    mmcsd_host_lock(host);
    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&stop, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));
    req.cmd  = &cmd;
    req.data = &data;

    cmd.arg = sector;
    if (!(card->flags & CARD_FLAG_SDHC))
    {
        cmd.arg <<= 9;
    }
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = SECTOR_SIZE;
    data.blks    = blks;

    if (blks > 1)
    {
        if (!controller_is_spi(card->host) || !dir)
        {
            req.stop      = &stop;
            stop.cmd_code = STOP_TRANSMISSION;
            stop.arg      = 0;
            stop.flags    = RESP_SPI_R1B | RESP_R1B | CMD_AC;
        }
        r_cmd = READ_MULTIPLE_BLOCK;
        w_cmd = WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        req.stop = OS_NULL;
        r_cmd    = READ_SINGLE_BLOCK;
        w_cmd    = WRITE_BLOCK;
    }

    if (!dir)
    {
        cmd.cmd_code = r_cmd;
        data.flags |= DATA_DIR_READ;
    }
    else
    {
        cmd.cmd_code = w_cmd;
        data.flags |= DATA_DIR_WRITE;
    }

    mmcsd_set_data_timeout(&data, card);
    data.buf = buf;
    mmcsd_send_request(host, &req);

    if (!controller_is_spi(card->host) && dir != 0)
    {
        do
        {
            os_int32_t err;

            cmd.cmd_code = SEND_STATUS;
            cmd.arg      = card->rca << 16;
            cmd.flags    = RESP_R1 | CMD_AC;
            err          = mmcsd_send_cmd(card->host, &cmd, 5);
            if (err)
            {
                LOG_E(DBG_TAG,"error %d requesting status", err);
                break;
            }
            /*
             * Some cards mishandle the status bits,
             * so make sure to check both the busy
             * indication and the card state.
             */
        } while (!(cmd.resp[0] & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(cmd.resp[0]) == 7));
    }

    mmcsd_host_unlock(host);

    if (cmd.err || data.err || stop.err)
    {
        LOG_E(DBG_TAG,"mmcsd request blocks error");
        LOG_E(DBG_TAG,"%d,%d,%d, 0x%08x,0x%08x", cmd.err, data.err, stop.err, data.flags, sector);

        return OS_ERROR;
    }

    return OS_EOK;
}

static os_int32_t mmcsd_set_blksize(struct os_mmcsd_card *card)
{
    struct os_mmcsd_cmd cmd;
    int                 err;

    /* Block-addressed cards ignore MMC_SET_BLOCKLEN. */
    if (card->flags & CARD_FLAG_SDHC)
        return 0;

    mmcsd_host_lock(card->host);
    cmd.cmd_code = SET_BLOCKLEN;
    cmd.arg      = 512;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_AC;
    err          = mmcsd_send_cmd(card->host, &cmd, 5);
    mmcsd_host_unlock(card->host);

    if (err)
    {
        LOG_E(DBG_TAG,"MMCSD: unable to set block size to %d: %d", cmd.arg, err);

        return OS_ERROR;
    }

    return 0;
}

static int mmcsd_blk_read_block(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr)
{
    os_err_t    err         = 0;
    os_size_t   offset      = 0;
    os_size_t   req_size    = 0;
    os_size_t   remain_size = block_nr;
    void       *rd_ptr      = (void *)buff;

    struct mmcsd_blk_device *mmcsd_blk_dev  = (struct mmcsd_blk_device *)blk;

    if (blk == OS_NULL)
    {
        return OS_ERROR;
    }

    while (remain_size)
    {
        req_size = (remain_size > mmcsd_blk_dev->max_req_size) ? mmcsd_blk_dev->max_req_size : remain_size;
        err      = os_mmcsd_req_blk(mmcsd_blk_dev->card, mmcsd_blk_dev->offset + block_addr + offset, rd_ptr, req_size, 0);
        if (err)
            break;
        offset += req_size;
        rd_ptr = (void *)((os_uint8_t *)rd_ptr + (req_size << 9));
        remain_size -= req_size;
    }

    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        return OS_ERROR;
    }
    return OS_EOK;
}

static int mmcsd_blk_write_block(os_blk_device_t *blk, os_uint32_t block_addr, const os_uint8_t *buff, os_uint32_t block_nr)
{
    os_err_t  err         = 0;
    os_size_t offset      = 0;
    os_size_t req_size    = 0;
    os_size_t remain_size = block_nr;
    void     *wr_ptr      = (void *)buff;
    
    struct mmcsd_blk_device *mmcsd_blk_dev = (struct mmcsd_blk_device *)blk;

    if (blk == OS_NULL)
    {
        return OS_ERROR;
    }

    while (remain_size)
    {
        req_size = (remain_size > mmcsd_blk_dev->max_req_size) ? mmcsd_blk_dev->max_req_size : remain_size;
        err      = os_mmcsd_req_blk(mmcsd_blk_dev->card, mmcsd_blk_dev->offset + block_addr + offset, wr_ptr, req_size, 1);
        if (err)
            break;
        offset += req_size;
        wr_ptr = (void *)((os_uint8_t *)wr_ptr + (req_size << 9));
        remain_size -= req_size;
    }

    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        return OS_ERROR;
    }
    return OS_EOK;
}

const static struct os_blk_ops mmcsd_blk_ops = {
    .read_block   = mmcsd_blk_read_block,
    .write_block  = mmcsd_blk_write_block,
};

os_int32_t os_mmcsd_blk_probe(struct os_mmcsd_card *card)
{
    os_int32_t  err = 0;
    os_uint8_t  status;
    os_uint8_t *sector;
    
    struct mmcsd_blk_device *mmcsd_blk_dev = os_calloc(1, sizeof(struct mmcsd_blk_device));
    if (mmcsd_blk_dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"allocate mmcsd_blk_dev failed!");

        return OS_ENOMEM;
    }

    mmcsd_blk_dev->max_req_size = BLK_MIN((card->host->max_dma_segs * card->host->max_seg_size) >> 9,
                                        (card->host->max_blk_count * card->host->max_blk_size) >> 9);

    err = mmcsd_set_blksize(card);
    if (err)
    {
        return err;
    }

    LOG_D(DBG_TAG, "probe mmcsd block device!");

    /* get the first sector to read partition table */
    sector = os_calloc(1, SECTOR_SIZE);
    
    if (sector == OS_NULL)
    {
        LOG_E(DBG_TAG,"allocate partition sector buffer failed!");

        return OS_ENOMEM;
    }

    status = os_mmcsd_req_blk(card, 0, sector, 1, 0);
    if (status == OS_EOK)
    {
        /* register mmcsd device */
        mmcsd_blk_dev->card = card;

        mmcsd_blk_dev->blk_dev.priv = card;

        mmcsd_blk_dev->blk_dev.geometry.block_size = SECTOR_SIZE;
        mmcsd_blk_dev->blk_dev.geometry.capacity   = card->card_capacity * 1024;

        mmcsd_blk_dev->blk_dev.blk_ops = &mmcsd_blk_ops;
        block_device_register(&mmcsd_blk_dev->blk_dev, "sd0");
    }
    else
    {
        LOG_E(DBG_TAG,"read mmcsd first sector failed");
        err = OS_ERROR;
    }

    os_free(sector);

    return err;
}

static os_err_t _os_mmcsd_blk_remove(os_device_t *dev, void *data)
{
    os_blk_device_t *blk_dev = (os_blk_device_t *)os_container_of(dev, os_blk_device_t, blk_dev);

    if (dev->type == OS_DEVICE_TYPE_BLOCK && blk_dev->priv == data)
    {
        block_device_unregister(blk_dev);
        return 1;
    }

    return 0;
}

void os_mmcsd_blk_remove(struct os_mmcsd_card *card)
{
    os_device_for_each(_os_mmcsd_blk_remove, card);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize block device on the mmc/sd.
 *
 * @details         since 2.1.0, this function does not need to be invoked in the system initialization.
 *
 * @param[in]       none
 *
 * @return          always return 0
 ***********************************************************************************************************************
 */

int os_mmcsd_blk_init(void)
{
    /* nothing */
    return 0;
}
