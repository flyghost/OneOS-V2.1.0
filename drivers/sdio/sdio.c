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

#include <sdio/mmcsd_core.h>
#include <sdio/sdio.h>
#include <sdio/sd.h>
#include <os_list.h>
#include <os_clock.h>
#include <os_task.h>
#include <os_util.h>
#include <os_memory.h>
#include <string.h>
#include <os_assert.h>
#include <os_errno.h>
#include <dlog.h>

#ifdef OS_SDIO_DEBUG
#define DRV_EXT_LVL DBG_EXT_DEBUG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif /* OS_SDIO_DEBUG */
#define DBG_TAG "sdio"
#include <drv_log.h>

#ifndef OS_SDIO_STACK_SIZE
#define OS_SDIO_STACK_SIZE 512
#endif
#ifndef OS_SDIO_TASK_PRIORITY
#define OS_SDIO_TASK_PRIORITY 0x40
#endif

static os_list_node_t sdio_cards   = OS_LIST_INIT(sdio_cards);
static os_list_node_t sdio_drivers = OS_LIST_INIT(sdio_drivers);

struct sdio_card
{
    struct os_mmcsd_card *card;
    os_list_node_t        list;
};

struct sdio_driver
{
    struct os_sdio_driver *drv;
    os_list_node_t         list;
};

#define MIN(a, b) (a < b ? a : b)

static const os_uint8_t speed_value[16] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};

static const os_uint32_t speed_unit[8] = {10000, 100000, 1000000, 10000000, 0, 0, 0, 0};

OS_INLINE os_int32_t sdio_match_card(struct os_mmcsd_card *card, const struct os_sdio_device_id *id);

/**
 * @brief 发送SD_IO_SEND_OP_COND命令：CMD5，用来获取OCR寄存器的值
 * 
 * 会用来检测是否是SDIO设备，只有SDIO会对该命令有反馈，其他卡是没有反馈的
 * 
 * 如果SDIO设备，会反馈电压信息，也就是卡能支持的电压是多少
 * 
 * @param host 
 * @param ocr 
 * @param cmd5_resp 
 * @return os_int32_t 
 */
os_int32_t sdio_io_send_op_cond(struct os_mmcsd_host *host, os_uint32_t ocr, os_uint32_t *cmd5_resp)
{
    struct os_mmcsd_cmd cmd;
    os_int32_t          i, err = 0;

    OS_ASSERT(host != OS_NULL);

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_IO_SEND_OP_COND;
    cmd.arg      = ocr;
    cmd.flags    = RESP_SPI_R4 | RESP_R4 | CMD_BCR;

    for (i = 100; i; i--)
    {
        err = mmcsd_send_cmd(host, &cmd, 0);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (controller_is_spi(host))
        {
            /*
             * Both R1_SPI_IDLE and MMC_CARD_BUSY indicate
             * an initialized card under SPI, but some cards
             * (Marvell's) only behave when looking at this
             * one.
             */
            if (cmd.resp[1] & CARD_BUSY)
                break;
        }
        else
        {
            if (cmd.resp[0] & CARD_BUSY)
                break;
        }

        err = OS_ETIMEOUT;

        mmcsd_delay_ms(10);
    }

    if (cmd5_resp)
        *cmd5_resp = cmd.resp[controller_is_spi(host) ? 1 : 0];

    return err;
}

os_int32_t sdio_io_rw_direct(struct os_mmcsd_card *card,
                             os_int32_t            rw,
                             os_uint32_t           fn,
                             os_uint32_t           reg_addr,
                             os_uint8_t *          pdata,
                             os_uint8_t            raw)
{
    struct os_mmcsd_cmd cmd;
    os_int32_t          err;

    OS_ASSERT(card != OS_NULL);
    OS_ASSERT(fn <= SDIO_MAX_FUNCTIONS);
    OS_ASSERT(pdata != OS_NULL);

    if (reg_addr & ~SDIO_ARG_CMD53_REG_MASK)
        return OS_ERROR;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_IO_RW_DIRECT;
    cmd.arg      = rw ? SDIO_ARG_CMD52_WRITE : SDIO_ARG_CMD52_READ;
    cmd.arg |= fn << SDIO_ARG_CMD52_FUNC_SHIFT;
    cmd.arg |= raw ? SDIO_ARG_CMD52_RAW_FLAG : 0x00000000;
    cmd.arg |= reg_addr << SDIO_ARG_CMD52_REG_SHIFT;
    cmd.arg |= *pdata;
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_AC;

    err = mmcsd_send_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    if (!controller_is_spi(card->host))
    {
        if (cmd.resp[0] & R5_ERROR)
            return OS_EIO;
        if (cmd.resp[0] & R5_FUNCTION_NUMBER)
            return OS_ERROR;
        if (cmd.resp[0] & R5_OUT_OF_RANGE)
            return OS_ERROR;
    }

    if (!rw || raw)
    {
        if (controller_is_spi(card->host))
            *pdata = (cmd.resp[0] >> 8) & 0xFF;
        else
            *pdata = cmd.resp[0] & 0xFF;
    }

    return 0;
}

os_int32_t sdio_io_rw_extended(struct os_mmcsd_card *card,
                               os_int32_t            rw,
                               os_uint32_t           fn,
                               os_uint32_t           addr,
                               os_int32_t            op_code,
                               os_uint8_t *          buf,
                               os_uint32_t           blocks,
                               os_uint32_t           blksize)
{
    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;

    OS_ASSERT(card != OS_NULL);
    OS_ASSERT(fn <= SDIO_MAX_FUNCTIONS);
    OS_ASSERT(blocks != 1 || blksize <= 512);
    OS_ASSERT(blocks != 0);
    OS_ASSERT(blksize != 0);

    if (addr & ~SDIO_ARG_CMD53_REG_MASK)
        return OS_ERROR;

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));

    req.cmd  = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_IO_RW_EXTENDED;
    cmd.arg      = rw ? SDIO_ARG_CMD53_WRITE : SDIO_ARG_CMD53_READ;
    cmd.arg |= fn << SDIO_ARG_CMD53_FUNC_SHIFT;
    cmd.arg |= op_code ? SDIO_ARG_CMD53_INCREMENT : 0x00000000;
    cmd.arg |= addr << SDIO_ARG_CMD53_REG_SHIFT;
    if (blocks == 1 && blksize <= 512)
        cmd.arg |= (blksize == 512) ? 0 : blksize; /* byte mode */
    else
        cmd.arg |= SDIO_ARG_CMD53_BLOCK_MODE | blocks; /* block mode */
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_ADTC;

    data.blksize = blksize;
    data.blks    = blocks;
    data.flags   = rw ? DATA_DIR_WRITE : DATA_DIR_READ;
    data.buf     = buf;

    mmcsd_set_data_timeout(&data, card);

    mmcsd_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    if (!controller_is_spi(card->host))
    {
        if (cmd.resp[0] & R5_ERROR)
            return OS_EIO;
        if (cmd.resp[0] & R5_FUNCTION_NUMBER)
            return OS_ERROR;
        if (cmd.resp[0] & R5_OUT_OF_RANGE)
            return OS_ERROR;
    }

    return 0;
}

OS_INLINE os_uint32_t sdio_max_block_size(struct os_sdio_function *func)
{
    os_uint32_t size = MIN(func->card->host->max_seg_size, func->card->host->max_blk_size);
    size             = MIN(size, func->max_blk_size);

    return MIN(size, 512u); /* maximum size for byte mode */
}

os_int32_t sdio_io_rw_extended_block(struct os_sdio_function *func,
                                     os_int32_t               rw,
                                     os_uint32_t              addr,
                                     os_int32_t               op_code,
                                     os_uint8_t *             buf,
                                     os_uint32_t              len)
{
    os_int32_t  ret;
    os_uint32_t left_size;
    os_uint32_t max_blks, blks;

    left_size = len;

    /* Do the bulk of the transfer using block mode (if supported). */
    if (func->card->cccr.multi_block && (len > sdio_max_block_size(func)))
    {
        max_blks = MIN(func->card->host->max_blk_count, func->card->host->max_seg_size / func->cur_blk_size);
        max_blks = MIN(max_blks, 511u);

        while (left_size > func->cur_blk_size)
        {
            blks = left_size / func->cur_blk_size;
            if (blks > max_blks)
                blks = max_blks;
            len = blks * func->cur_blk_size;

            ret = sdio_io_rw_extended(func->card, rw, func->num, addr, op_code, buf, blks, func->cur_blk_size);
            if (ret)
                return ret;

            left_size -= len;
            buf += len;
            if (op_code)
                addr += len;
        }
    }

    while (left_size > 0)
    {
        len = MIN(left_size, sdio_max_block_size(func));

        ret = sdio_io_rw_extended(func->card, rw, func->num, addr, op_code, buf, 1, len);
        if (ret)
            return ret;

        left_size -= len;
        buf += len;
        if (op_code)
            addr += len;
    }

    return 0;
}

os_uint8_t sdio_io_readb(struct os_sdio_function *func, os_uint32_t reg, os_int32_t *err)
{
    os_uint8_t data = 0;
    os_int32_t ret;

    ret = sdio_io_rw_direct(func->card, 0, func->num, reg, &data, 0);

    if (err)
    {
        *err = ret;
    }

    return data;
}

os_int32_t sdio_io_writeb(struct os_sdio_function *func, os_uint32_t reg, os_uint8_t data)
{
    return sdio_io_rw_direct(func->card, 1, func->num, reg, &data, 0);
}

os_uint16_t sdio_io_readw(struct os_sdio_function *func, os_uint32_t addr, os_int32_t *err)
{
    os_int32_t  ret;
    os_uint32_t dmabuf;

    if (err)
        *err = 0;

    ret = sdio_io_rw_extended_block(func, 0, addr, 1, (os_uint8_t *)&dmabuf, 2);
    if (ret)
    {
        if (err)
            *err = ret;
    }

    return (os_uint16_t)dmabuf;
}

os_int32_t sdio_io_writew(struct os_sdio_function *func, os_uint16_t data, os_uint32_t addr)
{
    os_uint32_t dmabuf = data;

    return sdio_io_rw_extended_block(func, 1, addr, 1, (os_uint8_t *)&dmabuf, 2);
}

os_uint32_t sdio_io_readl(struct os_sdio_function *func, os_uint32_t addr, os_int32_t *err)
{
    os_int32_t  ret;
    os_uint32_t dmabuf;

    if (err)
        *err = 0;

    ret = sdio_io_rw_extended_block(func, 0, addr, 1, (os_uint8_t *)&dmabuf, 4);
    if (ret)
    {
        if (err)
            *err = ret;
    }

    return dmabuf;
}

os_int32_t sdio_io_writel(struct os_sdio_function *func, os_uint32_t data, os_uint32_t addr)
{
    os_uint32_t dmabuf = data;

    return sdio_io_rw_extended_block(func, 1, addr, 1, (os_uint8_t *)&dmabuf, 4);
}

os_int32_t sdio_io_read_multi_fifo_b(struct os_sdio_function *func, os_uint32_t addr, os_uint8_t *buf, os_uint32_t len)
{
    return sdio_io_rw_extended_block(func, 0, addr, 0, buf, len);
}

os_int32_t sdio_io_write_multi_fifo_b(struct os_sdio_function *func, os_uint32_t addr, os_uint8_t *buf, os_uint32_t len)
{
    return sdio_io_rw_extended_block(func, 1, addr, 0, buf, len);
}

os_int32_t sdio_io_read_multi_incr_b(struct os_sdio_function *func, os_uint32_t addr, os_uint8_t *buf, os_uint32_t len)
{
    return sdio_io_rw_extended_block(func, 0, addr, 1, buf, len);
}

os_int32_t sdio_io_write_multi_incr_b(struct os_sdio_function *func, os_uint32_t addr, os_uint8_t *buf, os_uint32_t len)
{
    return sdio_io_rw_extended_block(func, 1, addr, 1, buf, len);
}

static os_int32_t sdio_read_cccr(struct os_mmcsd_card *card)
{
    os_int32_t ret;
    os_int32_t cccr_version;
    os_uint8_t data;

    memset(&card->cccr, 0, sizeof(struct os_sdio_cccr));

    data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_CCCR_REV, &ret);
    if (ret)
        goto out;

    cccr_version = data & 0x0f;

    if (cccr_version > SDIO_CCCR_REV_3_00)
    {
        LOG_E(DBG_TAG,"unrecognised CCCR structure version %d", cccr_version);

        return OS_ERROR;
    }

    card->cccr.sdio_version = (data & 0xf0) >> 4;

    data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_CARD_CAPS, &ret);
    if (ret)
        goto out;

    if (data & SDIO_CCCR_CAP_SMB)
        card->cccr.multi_block = 1;
    if (data & SDIO_CCCR_CAP_LSC)
        card->cccr.low_speed = 1;
    if (data & SDIO_CCCR_CAP_4BLS)
        card->cccr.low_speed_4 = 1;
    if (data & SDIO_CCCR_CAP_4BLS)
        card->cccr.bus_width = 1;

    if (cccr_version >= SDIO_CCCR_REV_1_10)
    {
        data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_POWER_CTRL, &ret);
        if (ret)
            goto out;

        if (data & SDIO_POWER_SMPC)
            card->cccr.power_ctrl = 1;
    }

    if (cccr_version >= SDIO_CCCR_REV_1_20)
    {
        data = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, &ret);
        if (ret)
            goto out;

        if (data & SDIO_SPEED_SHS)
            card->cccr.high_speed = 1;
    }

out:
    return ret;
}

static os_int32_t cistpl_funce_func0(struct os_mmcsd_card *card, const os_uint8_t *buf, os_uint32_t size)
{
    if (size < 0x04 || buf[0] != 0)
        return OS_ERROR;

    /* TPLFE_FN0_BLK_SIZE */
    card->cis.func0_blk_size = buf[1] | (buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    card->cis.max_tran_speed = speed_value[(buf[3] >> 3) & 15] * speed_unit[buf[3] & 7];

    return 0;
}

static os_int32_t cistpl_funce_func(struct os_sdio_function *func, const os_uint8_t *buf, os_uint32_t size)
{
    os_uint32_t version;
    os_uint32_t min_size;

    version  = func->card->cccr.sdio_version;
    min_size = (version == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size < min_size || buf[0] != 1)
        return OS_ERROR;

    /* TPLFE_MAX_BLK_SIZE */
    func->max_blk_size = buf[12] | (buf[13] << 8);

    /* TPLFE_ENABLE_TIMEOUT_VAL, present in ver 1.1 and above */
    if (version > SDIO_SDIO_REV_1_00)
        func->enable_timeout_val = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->enable_timeout_val = 1000; /* 1000ms */

    return 0;
}

static os_int32_t sdio_read_cis(struct os_sdio_function *func)
{
    os_int32_t                     ret;
    struct os_sdio_function_tuple *curr, **prev;
    os_uint32_t                    i, cisptr = 0;
    os_uint8_t                     data;
    os_uint8_t                     tpl_code, tpl_link;

    struct os_mmcsd_card *   card  = func->card;
    struct os_sdio_function *func0 = card->sdio_function[0];

    OS_ASSERT(func0 != OS_NULL);

    for (i = 0; i < 3; i++)
    {
        data = sdio_io_readb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_CIS + i, &ret);
        if (ret)
            return ret;
        cisptr |= data << (i * 8);
    }

    prev = &func->tuples;

    do
    {
        tpl_code = sdio_io_readb(func0, cisptr++, &ret);
        if (ret)
            break;
        tpl_link = sdio_io_readb(func0, cisptr++, &ret);
        if (ret)
            break;

        if ((tpl_code == CISTPL_END) || (tpl_link == 0xff))
            break;

        if (tpl_code == CISTPL_NULL)
            continue;

        curr = (struct os_sdio_function_tuple *)os_calloc(1, sizeof(struct os_sdio_function_tuple) + tpl_link);
        if (!curr)
            return OS_ENOMEM;
        curr->data = (os_uint8_t *)curr + sizeof(struct os_sdio_function_tuple);

        for (i = 0; i < tpl_link; i++)
        {
            curr->data[i] = sdio_io_readb(func0, cisptr + i, &ret);
            if (ret)
                break;
        }
        if (ret)
        {
            os_free(curr);
            break;
        }

        switch (tpl_code)
        {
        case CISTPL_MANFID:
            if (tpl_link < 4)
            {
                LOG_D(DBG_TAG, "bad CISTPL_MANFID length");
                break;
            }
            if (func->num != 0)
            {
                func->manufacturer = curr->data[0];
                func->manufacturer |= curr->data[1] << 8;
                func->product = curr->data[2];
                func->product |= curr->data[3] << 8;
            }
            else
            {
                card->cis.manufacturer = curr->data[0];
                card->cis.manufacturer |= curr->data[1] << 8;
                card->cis.product = curr->data[2];
                card->cis.product |= curr->data[3] << 8;
            }
            break;
        case CISTPL_FUNCE:
            if (func->num != 0)
                ret = cistpl_funce_func(func, curr->data, tpl_link);
            else
                ret = cistpl_funce_func0(card, curr->data, tpl_link);

            if (ret)
            {
                LOG_D(DBG_TAG, "bad CISTPL_FUNCE size %u "
                          "type %u", tpl_link, curr->data[0]);
            }

            break;
        case CISTPL_VERS_1:
            if (tpl_link < 2)
            {
                LOG_D(DBG_TAG, "CISTPL_VERS_1 too short");
            }
            break;
        default:
            /* this tuple is unknown to the core */
            curr->next = OS_NULL;
            curr->code = tpl_code;
            curr->size = tpl_link;
            *prev      = curr;
            prev       = &curr->next;
            LOG_D(DBG_TAG, "function %d, CIS tuple code %#x, length %d", func->num, tpl_code, tpl_link);
            break;
        }

        cisptr += tpl_link;
    } while (1);

    /*
     * Link in all unknown tuples found in the common CIS so that
     * drivers don't have to go digging in two places.
     */
    if (func->num != 0)
        *prev = func0->tuples;

    return ret;
}

void sdio_free_cis(struct os_sdio_function *func)
{
    struct os_sdio_function_tuple *tuple, *tmp;
    struct os_mmcsd_card *         card = func->card;

    tuple = func->tuples;

    while (tuple && ((tuple != card->sdio_function[0]->tuples) || (!func->num)))
    {
        tmp   = tuple;
        tuple = tuple->next;
        os_free(tmp);
    }

    func->tuples = OS_NULL;
}

static os_int32_t sdio_read_fbr(struct os_sdio_function *func)
{
    os_int32_t               ret;
    os_uint8_t               data;
    struct os_sdio_function *func0 = func->card->sdio_function[0];

    data = sdio_io_readb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_STD_FUNC_IF, &ret);
    if (ret)
        goto err;

    data &= 0x0f;

    if (data == 0x0f)
    {
        data = sdio_io_readb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_STD_IF_EXT, &ret);
        if (ret)
            goto err;
    }

    func->func_code = data;

err:
    return ret;
}

static os_int32_t sdio_initialize_function(struct os_mmcsd_card *card, os_uint32_t func_num)
{
    os_int32_t               ret;
    struct os_sdio_function *func;

    OS_ASSERT(func_num <= SDIO_MAX_FUNCTIONS);

    func = (struct os_sdio_function *)os_calloc(1, sizeof(struct os_sdio_function));
    if (!func)
    {
        LOG_E(DBG_TAG,"malloc os_sdio_function failed");
        ret = OS_ENOMEM;
        goto err;
    }
    memset(func, 0, sizeof(struct os_sdio_function));

    func->card = card;
    func->num  = func_num;

    ret = sdio_read_fbr(func);
    if (ret)
        goto err1;

    ret = sdio_read_cis(func);
    if (ret)
        goto err1;

    /*
     * product/manufacturer id is optional for function CIS, so
     * copy it from the card structure as needed.
     */
    if (func->product == 0)
    {
        func->manufacturer = card->cis.manufacturer;
        func->product      = card->cis.product;
    }

    card->sdio_function[func_num] = func;

    return 0;

err1:
    sdio_free_cis(func);
    os_free(func);
    card->sdio_function[func_num] = OS_NULL;
err:
    return ret;
}

static os_int32_t sdio_set_highspeed(struct os_mmcsd_card *card)
{
    os_int32_t ret;
    os_uint8_t speed;

    if (!(card->host->flags & MMCSD_SUP_HIGHSPEED))
        return 0;

    if (!card->cccr.high_speed)
        return 0;

    speed = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, &ret);
    if (ret)
        return ret;

    speed |= SDIO_SPEED_EHS;

    ret = sdio_io_writeb(card->sdio_function[0], SDIO_REG_CCCR_SPEED, speed);
    if (ret)
        return ret;

    card->flags |= CARD_FLAG_HIGHSPEED;

    return 0;
}

static os_int32_t sdio_set_bus_wide(struct os_mmcsd_card *card)
{
    os_int32_t ret;
    os_uint8_t busif;

    if (!(card->host->flags & MMCSD_BUSWIDTH_4))
        return 0;

    if (card->cccr.low_speed && !card->cccr.bus_width)
        return 0;

    busif = sdio_io_readb(card->sdio_function[0], SDIO_REG_CCCR_BUS_IF, &ret);
    if (ret)
        return ret;

    busif |= SDIO_BUS_WIDTH_4BIT;

    ret = sdio_io_writeb(card->sdio_function[0], SDIO_REG_CCCR_BUS_IF, busif);
    if (ret)
        return ret;

    mmcsd_set_bus_width(card->host, MMCSD_BUS_WIDTH_4);

    return 0;
}

static os_int32_t sdio_register_card(struct os_mmcsd_card *card)
{
    struct sdio_card   *sc;
    struct sdio_driver *sd;
    os_list_node_t     *l;

    sc = (struct sdio_card *)os_calloc(1, sizeof(struct sdio_card));
    if (sc == OS_NULL)
    {
        LOG_E(DBG_TAG,"malloc sdio card failed");
        return OS_ENOMEM;
    }

    sc->card = card;
    os_list_add_tail(&sdio_cards, &sc->list);

    if (os_list_empty(&sdio_drivers))
    {
        goto out;
    }

    for (l = (&sdio_drivers)->next; l != &sdio_drivers; l = l->next)
    {
        sd = (struct sdio_driver *)os_list_entry(l, struct sdio_driver, list);
        if (sdio_match_card(card, sd->drv->id))
        {
            sd->drv->probe(card);
        }
    }

out:
    return 0;
}

static os_int32_t sdio_init_card(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    os_int32_t            err = 0;
    os_int32_t            i, function_num;
    os_uint32_t           cmd5_resp;
    struct os_mmcsd_card *card;

    err = sdio_io_send_op_cond(host, ocr, &cmd5_resp);
    if (err)
        goto err;

    if (controller_is_spi(host))
    {
        err = mmcsd_spi_use_crc(host, host->spi_use_crc);
        if (err)
            goto err;
    }

    function_num = (cmd5_resp & 0x70000000) >> 28;

    card = (struct os_mmcsd_card *)os_calloc(1, sizeof(struct os_mmcsd_card));
    if (!card)
    {
        LOG_E(DBG_TAG,"malloc card failed");
        err = OS_ENOMEM;
        goto err;
    }
    memset(card, 0, sizeof(struct os_mmcsd_card));

    card->card_type         = CARD_TYPE_SDIO;
    card->sdio_function_num = function_num;
    card->host              = host;
    host->card              = card;

    card->sdio_function[0] = (struct os_sdio_function *)os_calloc(1, sizeof(struct os_sdio_function));
    if (!card->sdio_function[0])
    {
        LOG_E(DBG_TAG,"malloc sdio_func0 failed");
        err = OS_ENOMEM;
        goto err1;
    }
    memset(card->sdio_function[0], 0, sizeof(struct os_sdio_function));
    card->sdio_function[0]->card = card;
    card->sdio_function[0]->num  = 0;

    if (!controller_is_spi(host))
    {
        err = mmcsd_get_card_addr(host, &card->rca);
        if (err)
            goto err2;

        mmcsd_set_bus_mode(host, MMCSD_BUSMODE_PUSHPULL);
    }

    if (!controller_is_spi(host))
    {
        err = mmcsd_select_card(card);
        if (err)
            goto err2;
    }

    err = sdio_read_cccr(card);
    if (err)
        goto err2;

    err = sdio_read_cis(card->sdio_function[0]);
    if (err)
        goto err2;

    err = sdio_set_highspeed(card);
    if (err)
        goto err2;

    if (card->flags & CARD_FLAG_HIGHSPEED)
    {
        mmcsd_set_clock(host, 50000000);
    }
    else
    {
        mmcsd_set_clock(host, card->cis.max_tran_speed);
    }

    err = sdio_set_bus_wide(card);
    if (err)
        goto err2;

    for (i = 1; i < function_num + 1; i++)
    {
        err = sdio_initialize_function(card, i);
        if (err)
            goto err3;
    }

    /* register sdio card */
    err = sdio_register_card(card);
    if (err)
    {
        goto err3;
    }

    return 0;

err3:
    if (host->card)
    {
        for (i = 1; i < host->card->sdio_function_num + 1; i++)
        {
            if (host->card->sdio_function[i])
            {
                sdio_free_cis(host->card->sdio_function[i]);
                os_free(host->card->sdio_function[i]);
                host->card->sdio_function[i] = OS_NULL;
                os_free(host->card);
                host->card = OS_NULL;
                break;
            }
        }
    }
err2:
    if (host->card && host->card->sdio_function[0])
    {
        sdio_free_cis(host->card->sdio_function[0]);
        os_free(host->card->sdio_function[0]);
        host->card->sdio_function[0] = OS_NULL;
    }
err1:
    if (host->card)
    {
        os_free(host->card);
    }
err:
    LOG_E(DBG_TAG,"error %d while initialising SDIO card", err);

    return err;
}

os_int32_t init_sdio(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    os_int32_t  err;
    os_uint32_t current_ocr;

    OS_ASSERT(host != OS_NULL);

    if (ocr & 0x7F)
    {
        LOG_W(DBG_TAG,"Card ocr below the defined voltage rang.");
        ocr &= ~0x7F;
    }

    if (ocr & VDD_165_195)
    {
        LOG_W(DBG_TAG,"Can't support the low voltage SDIO card.");
        ocr &= ~VDD_165_195;
    }

    current_ocr = mmcsd_select_voltage(host, ocr);

    if (!current_ocr)
    {
        err = OS_ERROR;
        goto err;
    }

    err = sdio_init_card(host, current_ocr);
    if (err)
        goto remove_card;

    return 0;

remove_card:
    os_free(host->card);
    host->card = OS_NULL;
err:

    LOG_E(DBG_TAG,"init SDIO card failed");

    return err;
}

static void sdio_irq_thread(void *param)
{
    os_int32_t            i, ret;
    os_uint8_t            pending;
    struct os_mmcsd_card *card;
    struct os_mmcsd_host *host = (struct os_mmcsd_host *)param;
    OS_ASSERT(host != OS_NULL);
    card = host->card;
    OS_ASSERT(card != OS_NULL);

    while (1)
    {
        if (os_sem_wait(host->sdio_irq_sem, OS_WAIT_FOREVER) == OS_EOK)
        {
            mmcsd_host_lock(host);
            pending = sdio_io_readb(host->card->sdio_function[0], SDIO_REG_CCCR_INT_PEND, &ret);
            if (ret)
            {
                mmcsd_dbg("error %d reading SDIO_REG_CCCR_INT_PEND\r\n", ret);
                goto out;
            }

            for (i = 1; i <= 7; i++)
            {
                if (pending & (1 << i))
                {
                    struct os_sdio_function *func = card->sdio_function[i];
                    if (!func)
                    {
                        mmcsd_dbg("pending IRQ for "
                                  "non-existant function %d\r\n",
                                  func->num);
                        goto out;
                    }
                    else if (func->irq_handler)
                    {
                        func->irq_handler(func);
                    }
                    else
                    {
                        mmcsd_dbg("pending IRQ with no register handler\r\n");
                        goto out;
                    }
                }
            }

        out:
            mmcsd_host_unlock(host);
            if (host->flags & MMCSD_SUP_SDIO_IRQ)
                host->ops->enable_sdio_irq(host, 1);
            continue;
        }
    }
}

static os_int32_t sdio_irq_thread_create(struct os_mmcsd_card *card)
{
    struct os_mmcsd_host *host = card->host;

    /* init semaphore and create sdio irq processing thread */
    if (!host->sdio_irq_num)
    {
        host->sdio_irq_num++;
        host->sdio_irq_sem = os_sem_create("sdio_irq", 0, 1);
        OS_ASSERT(host->sdio_irq_sem != OS_NULL);

        host->sdio_irq_thread =
            os_task_create("sdio_irq", sdio_irq_thread, host, OS_SDIO_STACK_SIZE, OS_SDIO_TASK_PRIORITY);
        if (host->sdio_irq_thread != OS_NULL)
        {
            os_task_startup(host->sdio_irq_thread);
        }
    }

    return 0;
}

static os_int32_t sdio_irq_thread_delete(struct os_mmcsd_card *card)
{
    struct os_mmcsd_host *host = card->host;

    OS_ASSERT(host->sdio_irq_num > 0);

    host->sdio_irq_num--;
    if (!host->sdio_irq_num)
    {
        if (host->flags & MMCSD_SUP_SDIO_IRQ)
            host->ops->enable_sdio_irq(host, 0);
        os_sem_destroy(host->sdio_irq_sem);
        host->sdio_irq_sem = OS_NULL;
        os_task_destroy(host->sdio_irq_thread);
        host->sdio_irq_thread = OS_NULL;
    }

    return 0;
}

os_int32_t sdio_attach_irq(struct os_sdio_function *func, os_sdio_irq_handler_t *handler)
{
    os_int32_t               ret;
    os_uint8_t               reg;
    struct os_sdio_function *func0;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->card != OS_NULL);

    func0 = func->card->sdio_function[0];

    mmcsd_dbg("SDIO: enabling IRQ for function %d\r\n", func->num);

    if (func->irq_handler)
    {
        mmcsd_dbg("SDIO: IRQ for already in use.\r\n");

        return OS_EBUSY;
    }

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_INT_EN, &ret);
    if (ret)
        return ret;

    reg |= 1 << func->num;

    reg |= 1; /* Master interrupt enable */

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_INT_EN, reg);
    if (ret)
        return ret;

    func->irq_handler = handler;

    ret = sdio_irq_thread_create(func->card);
    if (ret)
        func->irq_handler = OS_NULL;

    return ret;
}

os_int32_t sdio_detach_irq(struct os_sdio_function *func)
{
    os_int32_t               ret;
    os_uint8_t               reg;
    struct os_sdio_function *func0;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->card != OS_NULL);

    func0 = func->card->sdio_function[0];

    mmcsd_dbg("SDIO: disabling IRQ for function %d\r\n", func->num);

    if (func->irq_handler)
    {
        func->irq_handler = OS_NULL;
        sdio_irq_thread_delete(func->card);
    }

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_INT_EN, &ret);
    if (ret)
        return ret;

    reg &= ~(1 << func->num);

    /* Disable master interrupt with the last function interrupt */
    if (!(reg & 0xFE))
        reg = 0;

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_INT_EN, reg);
    if (ret)
        return ret;

    return 0;
}

void sdio_irq_wakeup(struct os_mmcsd_host *host)
{
    if (host->flags & MMCSD_SUP_SDIO_IRQ)
        host->ops->enable_sdio_irq(host, 0);
    if (host->sdio_irq_sem)
        os_sem_post(host->sdio_irq_sem);
}

os_int32_t sdio_enable_func(struct os_sdio_function *func)
{
    os_int32_t               ret;
    os_uint8_t               reg;
    os_uint32_t              timeout;
    struct os_sdio_function *func0;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->card != OS_NULL);

    func0 = func->card->sdio_function[0];

    mmcsd_dbg("SDIO: enabling function %d\r\n", func->num);

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_IO_EN, &ret);
    if (ret)
        goto err;

    reg |= 1 << func->num;

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_IO_EN, reg);
    if (ret)
        goto err;

    timeout = os_tick_get() + func->enable_timeout_val * OS_TICK_PER_SECOND / 1000;

    while (1)
    {
        reg = sdio_io_readb(func0, SDIO_REG_CCCR_IO_RDY, &ret);
        if (ret)
            goto err;
        if (reg & (1 << func->num))
            break;
        ret = OS_ETIMEOUT;
        if (os_tick_get() > timeout)
            goto err;
    }

    mmcsd_dbg("SDIO: enabled function successfull\r\n");

    return 0;

err:
    mmcsd_dbg("SDIO: failed to enable function %d\r\n", func->num);
    return ret;
}

os_int32_t sdio_disable_func(struct os_sdio_function *func)
{
    os_int32_t               ret;
    os_uint8_t               reg;
    struct os_sdio_function *func0;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->card != OS_NULL);

    func0 = func->card->sdio_function[0];

    mmcsd_dbg("SDIO: disabling function %d\r\n", func->num);

    reg = sdio_io_readb(func0, SDIO_REG_CCCR_IO_EN, &ret);
    if (ret)
        goto err;

    reg &= ~(1 << func->num);

    ret = sdio_io_writeb(func0, SDIO_REG_CCCR_IO_EN, reg);
    if (ret)
        goto err;

    mmcsd_dbg("SDIO: disabled function successfull\r\n");

    return 0;

err:
    mmcsd_dbg("SDIO: failed to disable function %d\r\n", func->num);
    return OS_EIO;
}

void sdio_set_drvdata(struct os_sdio_function *func, void *data)
{
    func->priv = data;
}

void *sdio_get_drvdata(struct os_sdio_function *func)
{
    return func->priv;
}

os_int32_t sdio_set_block_size(struct os_sdio_function *func, os_uint32_t blksize)
{
    os_int32_t ret;
    struct os_sdio_function *func0 = func->card->sdio_function[0];

    if (blksize > func->card->host->max_blk_size)
        return OS_ERROR;

    if (blksize == 0)
    {
        blksize = MIN(func->max_blk_size, func->card->host->max_blk_size);
        blksize = MIN(blksize, 512u);
    }

    ret = sdio_io_writeb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_BLKSIZE, blksize & 0xff);
    if (ret)
        return ret;
    ret = sdio_io_writeb(func0, SDIO_REG_FBR_BASE(func->num) + SDIO_REG_FBR_BLKSIZE + 1, (blksize >> 8) & 0xff);
    if (ret)
        return ret;
    func->cur_blk_size = blksize;

    return 0;
}

OS_INLINE os_int32_t sdio_match_card(struct os_mmcsd_card *card, const struct os_sdio_device_id *id)
{
    os_uint8_t num = 1;

    if ((id->manufacturer != SDIO_ANY_MAN_ID) && (id->manufacturer != card->cis.manufacturer))
        return 0;

    while (num <= card->sdio_function_num)
    {
        if ((id->product != SDIO_ANY_PROD_ID) && (id->product == card->sdio_function[num]->product))
            return 1;
        num++;
    }

    return 0;
}

static struct os_mmcsd_card *sdio_match_driver(struct os_sdio_device_id *id)
{
    os_list_node_t *      l;
    struct sdio_card *    sc;
    struct os_mmcsd_card *card;

    for (l = (&sdio_cards)->next; l != &sdio_cards; l = l->next)
    {
        sc   = (struct sdio_card *)os_list_entry(l, struct sdio_card, list);
        card = sc->card;

        if (sdio_match_card(card, id))
        {
            return card;
        }
    }

    return OS_NULL;
}

os_int32_t sdio_register_driver(struct os_sdio_driver *driver)
{
    struct sdio_driver   *sd;
    struct os_mmcsd_card *card;

    sd = (struct sdio_driver *)os_calloc(1, sizeof(struct sdio_driver));
    if (sd == OS_NULL)
    {
        LOG_E(DBG_TAG,"malloc sdio driver failed");

        return OS_ENOMEM;
    }

    sd->drv = driver;
    os_list_add_tail(&sdio_drivers, &sd->list);

    if (!os_list_empty(&sdio_cards))
    {
        card = sdio_match_driver(driver->id);
        if (card != OS_NULL)
        {
            return driver->probe(card);
        }
    }

    return OS_EEMPTY;
}

os_int32_t sdio_unregister_driver(struct os_sdio_driver *driver)
{
    os_list_node_t       *l;
    struct sdio_driver   *sd = OS_NULL;
    struct os_mmcsd_card *card;

    for (l = (&sdio_drivers)->next; l != &sdio_drivers; l = l->next)
    {
        sd = (struct sdio_driver *)os_list_entry(l, struct sdio_driver, list);
        if (sd->drv != driver)
        {
            sd = OS_NULL;
        }
    }

    if (sd == OS_NULL)
    {
        LOG_E(DBG_TAG,"SDIO driver %s not register", driver->name);
        return OS_ERROR;
    }

    if (!os_list_empty(&sdio_cards))
    {
        card = sdio_match_driver(driver->id);
        if (card != OS_NULL)
        {
            driver->remove(card);
            os_list_del(&sd->list);
            os_free(sd);
        }
    }

    return 0;
}

void os_sdio_init(void)
{
}
