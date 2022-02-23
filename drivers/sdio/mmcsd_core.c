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
 * @file        mmcsd_core.c
 *
 * @brief       This file provides mmcsd_core functions for operating mmcsd.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_task.h>
#include <sdio/mmcsd_core.h>
#include <sdio/sd.h>
#include <sdio/mmc.h>
#include <sdio/sdio.h>
#include <os_mb.h>
#include <os_memory.h>
#include <string.h>
#include <os_errno.h>
#include <os_assert.h>
#include <arch_interrupt.h>
#include <arch_misc.h>
#include <dlog.h>
#include <driver.h>

#ifdef OS_SDIO_DEBUG
#define DRV_EXT_LVL DBG_EXT_DEBUG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif /* OS_SDIO_DEBUG */
#define DBG_TAG "mmcsd_core"
#include <drv_log.h>

#ifndef OS_MMCSD_STACK_SIZE
#define OS_MMCSD_STACK_SIZE 1024
#endif
#ifndef OS_MMCSD_TASK_PREORITY
#if (OS_TASK_PRIORITY_MAX == 32)
#define OS_MMCSD_TASK_PREORITY 0x16
#else
#define OS_MMCSD_TASK_PREORITY 0x40
#endif
#endif

// static struct os_semaphore mmcsd_sem;
static struct os_task    mmcsd_detect_thread;
static os_uint8_t        mmcsd_stack[OS_MMCSD_STACK_SIZE];
static os_mb_t mmcsd_detect_mb;
static os_uint32_t       mmcsd_detect_mb_pool[4];
static os_mb_t mmcsd_hotpluge_mb;
static os_uint32_t       mmcsd_hotpluge_mb_pool[4];

void mmcsd_host_lock(struct os_mmcsd_host *host)
{
    os_mutex_lock(&host->bus_lock, OS_WAIT_FOREVER);
}

void mmcsd_host_unlock(struct os_mmcsd_host *host)
{
    os_mutex_unlock(&host->bus_lock);
}

void mmcsd_req_complete(struct os_mmcsd_host *host)
{
    os_sem_post(&host->sem_ack);
}

void mmcsd_send_request(struct os_mmcsd_host *host, struct os_mmcsd_req *req)
{
    do
    {
        req->cmd->retries--;
        req->cmd->err = 0;
        req->cmd->mrq = req;
        if (req->data)
        {
            req->cmd->data = req->data;
            req->data->err = 0;
            req->data->mrq = req;
            if (req->stop)
            {
                req->data->stop = req->stop;
                req->stop->err  = 0;
                req->stop->mrq  = req;
            }
        }
        host->ops->request(host, req);

        os_sem_wait(&host->sem_ack, OS_WAIT_FOREVER);

    } while (req->cmd->err && (req->cmd->retries > 0));
}

os_int32_t mmcsd_send_cmd(struct os_mmcsd_host *host, struct os_mmcsd_cmd *cmd, int retries)
{
    struct os_mmcsd_req req;

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    req.cmd   = cmd;
    cmd->data = OS_NULL;

    mmcsd_send_request(host, &req);

    return cmd->err;
}

os_int32_t mmcsd_go_idle(struct os_mmcsd_host *host)
{
    os_int32_t          err;
    struct os_mmcsd_cmd cmd;

    if (!controller_is_spi(host))
    {
        mmcsd_set_chip_select(host, MMCSD_CS_HIGH);
        mmcsd_delay_ms(1);
    }

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = GO_IDLE_STATE;
    cmd.arg      = 0;
    cmd.flags    = RESP_SPI_R1 | RESP_NONE | CMD_BC;

    err = mmcsd_send_cmd(host, &cmd, 0);

    mmcsd_delay_ms(1);

    if (!controller_is_spi(host))
    {
        mmcsd_set_chip_select(host, MMCSD_CS_IGNORE);
        mmcsd_delay_ms(1);
    }

    return err;
}

os_int32_t mmcsd_spi_read_ocr(struct os_mmcsd_host *host, os_int32_t high_capacity, os_uint32_t *ocr)
{
    struct os_mmcsd_cmd cmd;
    os_int32_t          err;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SPI_READ_OCR;
    cmd.arg      = high_capacity ? (1 << 30) : 0;
    cmd.flags    = RESP_SPI_R3;

    err = mmcsd_send_cmd(host, &cmd, 0);

    *ocr = cmd.resp[1];

    return err;
}

os_int32_t mmcsd_all_get_cid(struct os_mmcsd_host *host, os_uint32_t *cid)
{
    os_int32_t          err;
    struct os_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = ALL_SEND_CID;
    cmd.arg      = 0;
    cmd.flags    = RESP_R2 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    memcpy(cid, cmd.resp, sizeof(os_uint32_t) * 4);

    return 0;
}

os_int32_t mmcsd_get_cid(struct os_mmcsd_host *host, os_uint32_t *cid)
{
    os_int32_t           err, i;
    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;
    os_uint8_t          *buf = OS_NULL;

    if (!controller_is_spi(host))
    {
        if (!host->card)
            return OS_ERROR;
        memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

        cmd.cmd_code = SEND_CID;
        cmd.arg      = host->card->rca << 16;
        cmd.flags    = RESP_R2 | CMD_AC;
        err          = mmcsd_send_cmd(host, &cmd, 3);
        if (err)
            return err;

        memcpy(cid, cmd.resp, sizeof(os_uint32_t) * 4);

        return 0;
    }

    buf = (os_uint8_t *)os_calloc(1, 16);
    if (!buf)
    {
        LOG_E(DBG_TAG,"allocate memory failed!");

        return OS_ENOMEM;
    }

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));

    req.cmd  = &cmd;
    req.data = &data;

    cmd.cmd_code = SEND_CID;
    cmd.arg      = 0;

    /* NOTE HACK:  the RESP_SPI_R1 is always correct here, but we
     * rely on callers to never use this with "native" calls for reading
     * CSD or CID.  Native versions of those commands use the R2 type,
     * not R1 plus a data block.
     */
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 16;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = buf;
    /*
     * The spec states that CSR and CID accesses have a timeout
     * of 64 clock cycles.
     */
    data.timeout_ns   = 0;
    data.timeout_clks = 64;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        os_free(buf);

        return OS_ERROR;
    }

    for (i = 0; i < 4; i++)
        cid[i] = buf[i];
    os_free(buf);

    return 0;
}

os_int32_t mmcsd_get_csd(struct os_mmcsd_card *card, os_uint32_t *csd)
{
    os_int32_t           err, i;
    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;
    os_uint8_t          *buf = OS_NULL;

    if (!controller_is_spi(card->host))
    {
        memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

        cmd.cmd_code = SEND_CSD;
        cmd.arg      = card->rca << 16;
        cmd.flags    = RESP_R2 | CMD_AC;
        err          = mmcsd_send_cmd(card->host, &cmd, 3);
        if (err)
            return err;

        memcpy(csd, cmd.resp, sizeof(os_uint32_t) * 4);

        return 0;
    }

    buf = (os_uint8_t *)os_calloc(1, 16);
    if (!buf)
    {
        LOG_E(DBG_TAG,"allocate memory failed!");

        return OS_ENOMEM;
    }

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));

    req.cmd  = &cmd;
    req.data = &data;

    cmd.cmd_code = SEND_CSD;
    cmd.arg      = 0;

    /* NOTE HACK:  the RESP_SPI_R1 is always correct here, but we
     * rely on callers to never use this with "native" calls for reading
     * CSD or CID.  Native versions of those commands use the R2 type,
     * not R1 plus a data block.
     */
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 16;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = buf;

    /*
     * The spec states that CSR and CID accesses have a timeout
     * of 64 clock cycles.
     */
    data.timeout_ns   = 0;
    data.timeout_clks = 64;

    mmcsd_send_request(card->host, &req);

    if (cmd.err || data.err)
    {
        os_free(buf);

        return OS_ERROR;
    }

    for (i = 0; i < 4; i++)
        csd[i] = buf[i];
    os_free(buf);

    return 0;
}

static os_int32_t _mmcsd_select_card(struct os_mmcsd_host *host, struct os_mmcsd_card *card)
{
    os_int32_t          err;
    struct os_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SELECT_CARD;

    if (card)
    {
        cmd.arg   = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg   = 0;
        cmd.flags = RESP_NONE | CMD_AC;
    }

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    return 0;
}

os_int32_t mmcsd_select_card(struct os_mmcsd_card *card)
{
    return _mmcsd_select_card(card->host, card);
}

os_int32_t mmcsd_deselect_cards(struct os_mmcsd_card *card)
{
    return _mmcsd_select_card(card->host, OS_NULL);
}

os_int32_t mmcsd_spi_use_crc(struct os_mmcsd_host *host, os_int32_t use_crc)
{
    struct os_mmcsd_cmd cmd;
    os_int32_t          err;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SPI_CRC_ON_OFF;
    cmd.flags    = RESP_SPI_R1;
    cmd.arg      = use_crc;

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (!err)
        host->spi_use_crc = use_crc;

    return err;
}

OS_INLINE void mmcsd_set_iocfg(struct os_mmcsd_host *host)
{
    struct os_mmcsd_io_cfg *io_cfg = &host->io_cfg;

    mmcsd_dbg("clock %uHz busmode %u powermode %u cs %u Vdd %u "
              "width %u \r\n",
              io_cfg->clock,
              io_cfg->bus_mode,
              io_cfg->power_mode,
              io_cfg->chip_select,
              io_cfg->vdd,
              io_cfg->bus_width);

    host->ops->set_iocfg(host, io_cfg);
}

/**
 ***********************************************************************************************************************
 * @brief           mmcsd_set_chip_select:Control chip select pin on a host.
 *
 * @param[in]       host            pointer to os_mmcsd_host
 * @param[in]       mode            select mode.
 * @param[out]      [param_n]       [The param_n description.]
 *
 * @return          none
 ***********************************************************************************************************************
 */
void mmcsd_set_chip_select(struct os_mmcsd_host *host, os_int32_t mode)
{
    host->io_cfg.chip_select = mode;
    mmcsd_set_iocfg(host);
}

/*
 * Sets the host clock to the highest possible frequency that
 * is below "hz".
 */
void mmcsd_set_clock(struct os_mmcsd_host *host, os_uint32_t clk)
{
    if (clk < host->freq_min)
    {
        LOG_W(DBG_TAG,"clock too low!");
    }

    host->io_cfg.clock = clk;
    mmcsd_set_iocfg(host);
}

/*
 * Change the bus mode (open drain/push-pull) of a host.
 */
void mmcsd_set_bus_mode(struct os_mmcsd_host *host, os_uint32_t mode)
{
    host->io_cfg.bus_mode = mode;
    mmcsd_set_iocfg(host);
}

/*
 * Change data bus width of a host.
 */
void mmcsd_set_bus_width(struct os_mmcsd_host *host, os_uint32_t width)
{
    host->io_cfg.bus_width = width;
    mmcsd_set_iocfg(host);
}

void mmcsd_set_data_timeout(struct os_mmcsd_data *data, const struct os_mmcsd_card *card)
{
    os_uint32_t mult;

    if (card->card_type == CARD_TYPE_SDIO)
    {
        data->timeout_ns   = 1000000000; /* SDIO card 1s */
        data->timeout_clks = 0;

        return;
    }

    /*
     * SD cards use a 100 multiplier rather than 10
     */
    mult = (card->card_type == CARD_TYPE_SD) ? 100 : 10;

    /*
     * Scale up the multiplier (and therefore the timeout) by
     * the r2w factor for writes.
     */
    if (data->flags & DATA_DIR_WRITE)
        mult <<= card->csd.r2w_factor;

    data->timeout_ns   = card->tacc_ns * mult;
    data->timeout_clks = card->tacc_clks * mult;

    /*
     * SD cards also have an upper limit on the timeout.
     */
    if (card->card_type == CARD_TYPE_SD)
    {
        os_uint32_t timeout_us, limit_us;

        timeout_us = data->timeout_ns / 1000;
        timeout_us += data->timeout_clks * 1000 / (card->host->io_cfg.clock / 1000);

        if (data->flags & DATA_DIR_WRITE)
            /*
             * The limit is really 250 ms, but that is
             * insufficient for some crappy cards.
             */
            limit_us = 300000;
        else
            limit_us = 100000;

        /*
         * SDHC cards always use these fixed values.
         */
        if (timeout_us > limit_us || card->flags & CARD_FLAG_SDHC)
        {
            data->timeout_ns   = limit_us * 1000; /* SDHC card fixed 250ms */
            data->timeout_clks = 0;
        }
    }

    if (controller_is_spi(card->host))
    {
        if (data->flags & DATA_DIR_WRITE)
        {
            if (data->timeout_ns < 1000000000)
                data->timeout_ns = 1000000000; /* 1s */
        }
        else
        {
            if (data->timeout_ns < 100000000)
                data->timeout_ns = 100000000; /* 100ms */
        }
    }
}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
os_uint32_t mmcsd_select_voltage(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    int bit;

    ocr &= host->valid_ocr;

    bit = os_ffs(ocr);
    if (bit)
    {
        bit -= 1;

        ocr &= 3 << bit;

        host->io_cfg.vdd = bit;
        mmcsd_set_iocfg(host);
    }
    else
    {
        LOG_W(DBG_TAG,"host doesn't support card's voltages!");
        ocr = 0;
    }

    return ocr;
}

static void mmcsd_power_up(struct os_mmcsd_host *host)
{
    int bit = os_fls(host->valid_ocr) - 1;

    host->io_cfg.vdd = bit;
    if (controller_is_spi(host))
    {
        host->io_cfg.chip_select = MMCSD_CS_HIGH;
        host->io_cfg.bus_mode    = MMCSD_BUSMODE_PUSHPULL;
    }
    else
    {
        host->io_cfg.chip_select = MMCSD_CS_IGNORE;
        host->io_cfg.bus_mode    = MMCSD_BUSMODE_OPENDRAIN;
    }
    host->io_cfg.power_mode = MMCSD_POWER_UP;
    host->io_cfg.bus_width  = MMCSD_BUS_WIDTH_1;
    mmcsd_set_iocfg(host);

    /*
     * This delay should be sufficient to allow the power supply
     * to reach the minimum voltage.
     */
    mmcsd_delay_ms(10);

    host->io_cfg.clock      = host->freq_min;
    host->io_cfg.power_mode = MMCSD_POWER_ON;
    mmcsd_set_iocfg(host);

    /*
     * This delay must be at least 74 clock sizes, or 1 ms, or the
     * time required to reach a stable voltage.
     */
    mmcsd_delay_ms(10);
}

static void mmcsd_power_off(struct os_mmcsd_host *host)
{
    host->io_cfg.clock = 0;
    host->io_cfg.vdd   = 0;
    if (!controller_is_spi(host))
    {
        host->io_cfg.bus_mode    = MMCSD_BUSMODE_OPENDRAIN;
        host->io_cfg.chip_select = MMCSD_CS_IGNORE;
    }
    host->io_cfg.power_mode = MMCSD_POWER_OFF;
    host->io_cfg.bus_width  = MMCSD_BUS_WIDTH_1;
    mmcsd_set_iocfg(host);
}

int mmcsd_wait_cd_changed(os_int32_t timeout)
{
    struct os_mmcsd_host *host;
    if (os_mb_recv(&mmcsd_hotpluge_mb, (os_ubase_t *)&host, timeout) == OS_EOK)
    {
        if (host->card == OS_NULL)
        {
            return MMCSD_HOST_UNPLUGED;
        }
        else
        {
            return MMCSD_HOST_PLUGED;
        }
    }
    return OS_ETIMEOUT;
}

void mmcsd_change(struct os_mmcsd_host *host)
{
    os_mb_send(&mmcsd_detect_mb, (os_uint32_t)host, 1000);
}

void mmcsd_detect(void *param)
{
    struct os_mmcsd_host *host;
    os_uint32_t           ocr;
    os_int32_t            err;

    while (1)
    {
        if (os_mb_recv(&mmcsd_detect_mb, (os_ubase_t *)&host, OS_WAIT_FOREVER) == OS_EOK)
        {
            if (host->card == OS_NULL)
            {
                mmcsd_host_lock(host);
                mmcsd_power_up(host);
                mmcsd_go_idle(host);

                mmcsd_send_if_cond(host, host->valid_ocr);

                err = sdio_io_send_op_cond(host, 0, &ocr);
                if (!err)
                {
                    if (init_sdio(host, ocr))
                        mmcsd_power_off(host);
                    mmcsd_host_unlock(host);
                    continue;
                }

                /*
                 * detect SD card
                 */
                err = mmcsd_send_app_op_cond(host, 0, &ocr);
                if (!err)
                {
                    if (init_sd(host, ocr))
                        mmcsd_power_off(host);
                    mmcsd_host_unlock(host);
                    os_mb_send(&mmcsd_hotpluge_mb, (os_uint32_t)host, 1000);
                    continue;
                }

                /*
                 * detect mmc card
                 */
                err = mmc_send_op_cond(host, 0, &ocr);
                if (!err)
                {
                    if (init_mmc(host, ocr))
                        mmcsd_power_off(host);
                    mmcsd_host_unlock(host);
                    os_mb_send(&mmcsd_hotpluge_mb, (os_uint32_t)host, 1000);
                    continue;
                }
                mmcsd_host_unlock(host);
            }
            else
            {
                /* card removed */
                mmcsd_host_lock(host);
                if (host->card->sdio_function_num != 0)
                {
                    LOG_W(DBG_TAG,"unsupport sdio card plug out!");
                }
                else
                {
                    os_mmcsd_blk_remove(host->card);
                    os_free(host->card);

                    host->card = OS_NULL;
                }
                mmcsd_host_unlock(host);
                os_mb_send(&mmcsd_hotpluge_mb, (os_uint32_t)host, 1000);
            }
        }
    }
}

struct os_mmcsd_host *mmcsd_alloc_host(void)
{
    struct os_mmcsd_host *host;

    host = (struct os_mmcsd_host *)os_calloc(1, sizeof(struct os_mmcsd_host));
    if (!host)
    {
        LOG_E(DBG_TAG,"alloc host failed");

        return OS_NULL;
    }

    memset(host, 0, sizeof(struct os_mmcsd_host));

    host->max_seg_size  = 65535;
    host->max_dma_segs  = 1;
    host->max_blk_size  = 512;
    host->max_blk_count = 4096;

    os_mutex_init(&host->bus_lock, "sd_bus_lock", OS_FALSE);
    os_sem_init(&host->sem_ack, "sd_ack", 0, 1);

    return host;
}

void mmcsd_free_host(struct os_mmcsd_host *host)
{
    os_mutex_deinit(&host->bus_lock);
    os_sem_deinit(&host->sem_ack);
    os_free(host);
}

/**
 ***********************************************************************************************************************
 * @brief           os_mmcsd_core_init:init mmcsd,create detect SD card task.
 *
 * @param[in]       none
 *
 * @return          Return init status.
 * @retval          OS_EOK          init success.
 * @retval          Others          init failed.
 ***********************************************************************************************************************
 */
int os_mmcsd_core_init(void)
{
    os_err_t ret;

    /* initialize detect SD cart task */
    /* initialize mailbox and create detect SD card task */
    ret = os_mb_init(&mmcsd_detect_mb,
                     "mmcsdmb",
                     &mmcsd_detect_mb_pool[0],
                     sizeof(mmcsd_detect_mb_pool) / sizeof(mmcsd_detect_mb_pool[0]));
    OS_ASSERT(ret == OS_EOK);

    ret = os_mb_init(&mmcsd_hotpluge_mb,
                     "mmcsdhotplugmb",
                     &mmcsd_hotpluge_mb_pool[0],
                     sizeof(mmcsd_hotpluge_mb_pool) / sizeof(mmcsd_hotpluge_mb_pool[0]));
    OS_ASSERT(ret == OS_EOK);
    ret = os_task_init(&mmcsd_detect_thread,
                       "mmcsd_detect",
                       mmcsd_detect,
                       OS_NULL,
                       &mmcsd_stack[0],
                       OS_MMCSD_STACK_SIZE,
                       OS_MMCSD_TASK_PREORITY);
    if (ret == OS_EOK)
    {
        os_task_startup(&mmcsd_detect_thread);
    }

    os_sdio_init();

    return 0;
}
OS_PREV_INIT(os_mmcsd_core_init, OS_INIT_SUBLEVEL_HIGH);
