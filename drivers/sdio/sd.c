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
 * @file        sd.c
 *
 * @brief       This file provides functions for operating sd card.
 *
 * @details     sd
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sdio/mmcsd_core.h>
#include <sdio/sd.h>
#include <os_util.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>
#include <dlog.h>

#ifdef OS_SDIO_DEBUG
#define DRV_EXT_LVL DBG_EXT_DEBUG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif /* OS_SDIO_DEBUG */
#define DBG_TAG "sd"
#include <drv_log.h>

static const os_uint32_t tran_unit[] =
{
    10000, 100000, 1000000, 10000000,
    0,     0,      0,       0
};

static const os_uint8_t tran_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const os_uint32_t tacc_uint[] =
{
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const os_uint8_t tacc_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

OS_INLINE os_uint32_t GET_BITS(os_uint32_t *resp, os_uint32_t start, os_uint32_t size)
{
    const os_int32_t  __size = size;
    const os_uint32_t __mask = (__size < 32 ? 1 << __size : 0) - 1;
    const os_int32_t  __off  = 3 - ((start) / 32);
    const os_int32_t  __shft = (start)&31;
    os_uint32_t       __res;

    __res = resp[__off] >> __shft;
    if (__size + __shft > 32)
        __res |= resp[__off - 1] << ((32 - __shft) % 32);

    return __res & __mask;
}

/**
 * @brief 解析CSD的值
 * 
 * @param card 
 * @return os_int32_t 
 */
static os_int32_t mmcsd_parse_csd(struct os_mmcsd_card *card)
{
    struct os_mmcsd_csd *csd  = &card->csd;
    os_uint32_t         *resp = card->resp_csd;

    csd->csd_structure = GET_BITS(resp, 126, 2);

    switch (csd->csd_structure)
    {
    case 0:
        csd->taac            = GET_BITS(resp, 112, 8);
        csd->nsac            = GET_BITS(resp, 104, 8);
        csd->tran_speed      = GET_BITS(resp, 96, 8);
        csd->card_cmd_class  = GET_BITS(resp, 84, 12);
        csd->rd_blk_len      = GET_BITS(resp, 80, 4);
        csd->rd_blk_part     = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp         = GET_BITS(resp, 76, 1);
        csd->c_size          = GET_BITS(resp, 62, 12);
        csd->c_size_mult     = GET_BITS(resp, 47, 3);
        csd->r2w_factor      = GET_BITS(resp, 26, 3);
        csd->wr_blk_len      = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial  = GET_BITS(resp, 21, 1);
        csd->csd_crc         = GET_BITS(resp, 1, 7);

        card->card_blksize  = 1 << csd->rd_blk_len;
        card->card_capacity = (csd->c_size + 1) << (csd->c_size_mult + 2);
        card->card_capacity *= card->card_blksize;
        card->card_capacity >>= 10; /* unit:KB */
        card->tacc_clks     = csd->nsac * 100;
        card->tacc_ns       = (tacc_uint[csd->taac & 0x07] * tacc_value[(csd->taac & 0x78) >> 3] + 9) / 10;
        card->max_data_rate = tran_unit[csd->tran_speed & 0x07] * tran_value[(csd->tran_speed & 0x78) >> 3];

#if 0
        val = GET_BITS(resp, 115, 4);
        unit = GET_BITS(resp, 112, 3);
        csd->tacc_ns     = (tacc_uint[unit] * tacc_value[val] + 9) / 10;
        csd->tacc_clks   = GET_BITS(resp, 104, 8) * 100;

        val = GET_BITS(resp, 99, 4);
        unit = GET_BITS(resp, 96, 3);
        csd->max_data_rate    = tran_unit[unit] * tran_value[val];
        csd->ccc      = GET_BITS(resp, 84, 12);

        unit = GET_BITS(resp, 47, 3);
        val = GET_BITS(resp, 62, 12);
        csd->device_size      = (1 + val) << (unit + 2);

        csd->read_bl_len = GET_BITS(resp, 80, 4);
        csd->write_bl_len = GET_BITS(resp, 22, 4);
        csd->r2w_factor = GET_BITS(resp, 26, 3);
#endif
        break;
    case 1:
        card->flags |= CARD_FLAG_SDHC;

        /*This field is fixed to 0Eh, which indicates 1 ms.
          The host should not use TAAC, NSAC, and R2W_FACTOR
          to calculate timeout and should uses fixed timeout
          values for read and write operations*/
        csd->taac            = GET_BITS(resp, 112, 8);
        csd->nsac            = GET_BITS(resp, 104, 8);
        csd->tran_speed      = GET_BITS(resp, 96, 8);
        csd->card_cmd_class  = GET_BITS(resp, 84, 12);
        csd->rd_blk_len      = GET_BITS(resp, 80, 4);
        csd->rd_blk_part     = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp         = GET_BITS(resp, 76, 1);
        csd->c_size          = GET_BITS(resp, 48, 22);

        csd->r2w_factor     = GET_BITS(resp, 26, 3);
        csd->wr_blk_len     = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial = GET_BITS(resp, 21, 1);
        csd->csd_crc        = GET_BITS(resp, 1, 7);

        card->card_blksize  = 512;
        card->card_capacity = (csd->c_size + 1) * 512; /* unit:KB */
        card->tacc_clks     = 0;
        card->tacc_ns       = 0;
        card->max_data_rate = tran_unit[csd->tran_speed & 0x07] * tran_value[(csd->tran_speed & 0x78) >> 3];

#if 0
        csd->tacc_ns     = 0;
        csd->tacc_clks   = 0;

        val = GET_BITS(resp, 99, 4);
        unit = GET_BITS(resp, 96, 3);
        csd->max_data_rate    = tran_unit[unit] * tran_value[val];
        csd->ccc      = GET_BITS(resp, 84, 12);

        val = GET_BITS(resp, 48, 22);
        csd->device_size     = (1 + val) << 10;

        csd->read_bl_len = 9;
        csd->write_bl_len = 9;
        /* host should not use this factor and should use 250ms for write timeout */
        csd->r2w_factor = 2;
#endif
        break;
    default:
        LOG_E(DBG_TAG,"unrecognised CSD structure version %d!", csd->csd_structure);

        return OS_ERROR;
    }
    LOG_I(DBG_TAG,"SD card capacity %d KB.", card->card_capacity);
    return 0;
}

/**
 * @brief 解析SCR的值
 * 
 * @param card 
 * @return os_int32_t 
 */
static os_int32_t mmcsd_parse_scr(struct os_mmcsd_card *card)
{
    struct os_sd_scr *scr = &card->scr;
    os_uint32_t       resp[4];

    resp[3]            = card->resp_scr[1];
    resp[2]            = card->resp_scr[0];
    scr->sd_version    = GET_BITS(resp, 56, 4);
    scr->sd_bus_widths = GET_BITS(resp, 48, 4);

    return 0;
}

/**
 * @brief 发送CMD6命令，将卡切换到高速，只适用于SD2.0的卡
 * 
 * SD_SWITCH：SD专属命令
 * 
 * @param card 
 * @return os_int32_t 
 */
static os_int32_t mmcsd_switch(struct os_mmcsd_card *card)
{
    os_int32_t            err;
    struct os_mmcsd_host *host = card->host;
    struct os_mmcsd_req   req;
    struct os_mmcsd_cmd   cmd;
    struct os_mmcsd_data  data;
    os_uint8_t           *buf;

    buf = (os_uint8_t *)os_calloc(1, 64);
    if (!buf)
    {
        LOG_E(DBG_TAG,"alloc memory failed!");

        return OS_ENOMEM;
    }

    if (card->card_type != CARD_TYPE_SD)
        goto err;
    if (card->scr.sd_version < SCR_SPEC_VER_1)
        goto err;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg      = 0x00FFFFF1;
    cmd.flags    = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct os_mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = buf;

    memset(&req, 0, sizeof(struct os_mmcsd_req));

    req.cmd  = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        goto err1;
    }

    if (buf[13] & 0x02)
        card->hs_max_data_rate = 50000000;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg      = 0x80FFFFF1;
    cmd.flags    = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct os_mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = buf;

    memset(&req, 0, sizeof(struct os_mmcsd_req));

    req.cmd  = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        goto err1;
    }

    if ((buf[16] & 0xF) != 1)
    {
        LOG_I(DBG_TAG,"switching card to high speed failed!");
        goto err;
    }

    card->flags |= CARD_FLAG_HIGHSPEED;

err:
    os_free(buf);
    return 0;

err1:
    if (cmd.err)
        err = cmd.err;
    if (data.err)
        err = data.err;

    return err;
}

/**
 * @brief 发送CMD55命令，通知CARD，接下来要发送的命令是特定应用命令，不是标准命令
 * 
 * APP_CMD，SD卡专属命令
 * 
 * @param host 
 * @param card 
 * @return os_err_t 
 */
static os_err_t mmcsd_app_cmd(struct os_mmcsd_host *host, struct os_mmcsd_card *card)
{
    os_err_t err;
    struct os_mmcsd_cmd cmd = {0};

    cmd.cmd_code = APP_CMD;

    if (card)
    {
        cmd.arg   = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg   = 0;
        cmd.flags = RESP_R1 | CMD_BCR;
    }

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    /* Check that card supported application commands */
    if (!controller_is_spi(host) && !(cmd.resp[0] & R1_APP_CMD))
        return OS_ERROR;

    return OS_EOK;
}

/**
 * @brief 发送特定应用命令
 * 
 * 先发送CMD55来通知CARD设备，下一条命令是特定应用命令，然后再发送特定应用命令
 * 
 * @param host      host实例
 * @param card      卡设备，可以为NULL
 * @param cmd       命令
 * @param retry     重试次数
 * @return os_err_t 成功，返回0  错误，小于0
 */
os_err_t mmcsd_send_app_cmd(struct os_mmcsd_host *host, struct os_mmcsd_card *card, struct os_mmcsd_cmd *cmd, int retry)
{
    struct os_mmcsd_req req;

    os_uint32_t i;
    os_err_t    err;

    err = OS_ERROR;

    /*
     * We have to resend MMC_APP_CMD for each attempt so
     * we cannot use the retries field in mmc_command.
     */
    for (i = 0; i <= retry; i++)
    {
        memset(&req, 0, sizeof(struct os_mmcsd_req));

        err = mmcsd_app_cmd(host, card);
        if (err)
        {
            /* no point in retrying; no APP commands allowed */
            if (controller_is_spi(host))
            {
                if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                    break;
            }
            continue;
        }

        memset(&req, 0, sizeof(struct os_mmcsd_req));

        memset(cmd->resp, 0, sizeof(cmd->resp));

        req.cmd = cmd;
        // cmd->data = NULL;

        mmcsd_send_request(host, &req);

        err = cmd->err;
        if (!cmd->err)
            break;

        /* no point in retrying illegal APP commands */
        if (controller_is_spi(host))
        {
            if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                break;
        }
    }

    return err;
}

/**
 * @brief 发送CMD6，设置总线宽度
 * 
 * SD_APP_SET_BUS_WIDTH
 * 
 * @param card 
 * @param width 总线宽度
 * @return os_err_t 
 */
os_err_t mmcsd_app_set_bus_width(struct os_mmcsd_card *card, os_int32_t width)
{
    os_err_t            err;
    struct os_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_APP_SET_BUS_WIDTH;
    cmd.flags    = RESP_R1 | CMD_AC;

    switch (width)
    {
    case MMCSD_BUS_WIDTH_1:
        cmd.arg = MMCSD_BUS_WIDTH_1;
        break;
    case MMCSD_BUS_WIDTH_4:
        cmd.arg = MMCSD_BUS_WIDTH_4;
        break;
    default:
        return OS_ERROR;
    }

    err = mmcsd_send_app_cmd(card->host, card, &cmd, 3);
    if (err)
        return err;

    return OS_EOK;
}

/**
 * @brief 发送CMD41，检测是否为SD设备，并返回OCR信息
 * 
 * SD_APP_OP_COND
 * 
 * 该函数内实现了首先发送一条CMD55命令，再发送CMD41命令
 * 
 * CMD55：用来通知CARD下一条命令是特定应用命令，而不是标准命令
 * 
 * @param host 
 * @param ocr 
 * @param rocr 
 * @return os_err_t 
 */
os_err_t mmcsd_send_app_op_cond(struct os_mmcsd_host *host, os_uint32_t ocr, os_uint32_t *rocr)
{
    struct os_mmcsd_cmd cmd;
    os_uint32_t i;
    os_err_t    err = OS_EOK;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_APP_OP_COND;
    if (controller_is_spi(host))
        cmd.arg = ocr & (1 << 30); /* SPI only defines one bit */
    else
        cmd.arg = ocr;
    cmd.flags = RESP_SPI_R1 | RESP_R3 | CMD_BCR;

    for (i = 100; i; i--)
    {
        err = mmcsd_send_app_cmd(host, OS_NULL, &cmd, 3);       // 发送特定应用命令，card为空
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (controller_is_spi(host))
        {
            if (!(cmd.resp[0] & R1_SPI_IDLE))
                break;
        }
        else
        {
            if (cmd.resp[0] & CARD_BUSY)
                break;
        }

        err = OS_ETIMEOUT;

        mmcsd_delay_ms(10);    // delay 10ms
    }

    if (rocr && !controller_is_spi(host))
        *rocr = cmd.resp[0];

    return err;
}

/**
 * @brief To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
 * before SD_APP_OP_COND. This command will harmlessly fail for
 * SD 1.0 cards.
 * 
 * 为了支持SD2.0协议的卡，必须发送CMD8：发送SD卡接口条件，包含host支持的电压信息，并询问卡是否支持
 * 
 * 该命令对1.0的卡没有效果
 * 
 * @param host 
 * @param ocr HOST支持的电压信息
 * @return os_err_t 
 */
os_err_t mmcsd_send_if_cond(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    struct os_mmcsd_cmd cmd;
    os_err_t            err;
    os_uint8_t          pattern;

    cmd.cmd_code = SD_SEND_IF_COND;
    cmd.arg      = ((ocr & 0xFF8000) != 0) << 8 | 0xAA;
    cmd.flags    = RESP_SPI_R7 | RESP_R7 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    if (controller_is_spi(host))
        pattern = cmd.resp[1] & 0xFF;
    else
        pattern = cmd.resp[0] & 0xFF;

    if (pattern != 0xAA)
        return OS_ERROR;

    return OS_EOK;
}

/**
 * @brief 获取卡的相对地址RCA
 * 
 * @param host 
 * @param rca 返回的卡的相对地址，RCA
 * @return os_err_t 
 */
os_err_t mmcsd_get_card_addr(struct os_mmcsd_host *host, os_uint32_t *rca)
{
    os_err_t            err;
    struct os_mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_SEND_RELATIVE_ADDR;
    cmd.arg      = 0;
    cmd.flags    = RESP_R6 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    *rca = cmd.resp[0] >> 16;

    return OS_EOK;
}

#define be32_to_cpu(x)                                                                                                 \
    ((os_uint32_t)((((os_uint32_t)(x) & (os_uint32_t)0x000000ffUL) << 24) |                                            \
                   (((os_uint32_t)(x) & (os_uint32_t)0x0000ff00UL) << 8) |                                             \
                   (((os_uint32_t)(x) & (os_uint32_t)0x00ff0000UL) >> 8) |                                             \
                   (((os_uint32_t)(x) & (os_uint32_t)0xff000000UL) >> 24)))

/**
 * @brief 发送CMD51，获取卡的SCR
 * 
 * @param card 
 * @param scr 
 * @return os_int32_t 
 */
os_int32_t mmcsd_get_scr(struct os_mmcsd_card *card, os_uint32_t *scr)
{
    os_int32_t           err;
    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;

    err = mmcsd_app_cmd(card->host, card);
    if (err)
        return err;

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));

    req.cmd  = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_APP_SEND_SCR;
    cmd.arg      = 0;
    cmd.flags    = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 8;
    data.blks    = 1;
    data.flags   = DATA_DIR_READ;
    data.buf     = (os_uint8_t *)scr;

    mmcsd_set_data_timeout(&data, card);

    mmcsd_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    scr[0] = be32_to_cpu(scr[0]);
    scr[1] = be32_to_cpu(scr[1]);

    return 0;
}

/**
 * @brief 初始化SD卡
 * 
 * @param host 
 * @param ocr   OCR的值
 * @return os_int32_t 
 */
static os_int32_t mmcsd_sd_init_card(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    struct os_mmcsd_card *card;
    os_int32_t            err;
    os_uint32_t           resp[4];
    os_uint32_t           max_data_rate;

    mmcsd_go_idle(host);                // 发送CMD0，card进入IDLE状态

    /*
     * If SD_SEND_IF_COND indicates an SD 2.0
     * compliant card and we should set bit 30
     * of the ocr to indicate that we can handle
     * block-addressed SDHC cards.
     */
    err = mmcsd_send_if_cond(host, ocr);    // 发送HOST支持的电压信息，只适用于SD2.0的卡
    if (!err)
        ocr |= 1 << 30;

    err = mmcsd_send_app_op_cond(host, ocr, OS_NULL);
    if (err)
        goto err;

    if (controller_is_spi(host))
        err = mmcsd_get_cid(host, resp);        // CMD10，选定的卡的CID
    else
        err = mmcsd_all_get_cid(host, resp);    // CMD2，所有卡的CID
    if (err)
        goto err;

    card = (struct os_mmcsd_card *)os_calloc(1, sizeof(struct os_mmcsd_card));
    if (!card)
    {
        LOG_E(DBG_TAG,"malloc card failed!");
        err = OS_ENOMEM;
        goto err;
    }
    memset(card, 0, sizeof(struct os_mmcsd_card));

    card->card_type = CARD_TYPE_SD;
    card->host      = host;
    memcpy(card->resp_cid, resp, sizeof(card->resp_cid));

    /*
     * For native busses:  get card RCA and quit open drain mode.
     */
    if (!controller_is_spi(host))
    {
        err = mmcsd_get_card_addr(host, &card->rca);  // 发送CMD3，获取RCA，卡的相对地址
        if (err)
            goto err1;

        mmcsd_set_bus_mode(host, MMCSD_BUSMODE_PUSHPULL);
    }

    err = mmcsd_get_csd(card, card->resp_csd);      // 发送CMD7，获取卡的CSD
    if (err)
        goto err1;

    err = mmcsd_parse_csd(card);                    // 解析CSD
    if (err)
        goto err1;

    if (!controller_is_spi(host))
    {
        err = mmcsd_select_card(card);
        if (err)
            goto err1;
    }

    err = mmcsd_get_scr(card, card->resp_scr);      // 获取SCR
    if (err)
        goto err1;

    mmcsd_parse_scr(card);                          // 解析SCR

    if (controller_is_spi(host))
    {
        err = mmcsd_spi_use_crc(host, 1);           // 发送CMD59，是否使用CRC
        if (err)
            goto err1;
    }

    /*
     * change SD card to high-speed, only SD2.0 spec
     */
    err = mmcsd_switch(card);
    if (err)
        goto err1;

    /* set bus speed */
    max_data_rate = (unsigned int)-1;

    if (card->flags & CARD_FLAG_HIGHSPEED)
    {
        if (max_data_rate > card->hs_max_data_rate)
            max_data_rate = card->hs_max_data_rate;
    }
    else if (max_data_rate > card->max_data_rate)
    {
        max_data_rate = card->max_data_rate;
    }

    mmcsd_set_clock(host, max_data_rate);

    /*switch bus width*/
    if ((host->flags & MMCSD_BUSWIDTH_4) && (card->scr.sd_bus_widths & SD_SCR_BUS_WIDTH_4)) // 需要HOST和CARD同时支持4bit宽度
    {
        err = mmcsd_app_set_bus_width(card, MMCSD_BUS_WIDTH_4); // 设置总线宽度
        if (err)
            goto err1;

        mmcsd_set_bus_width(host, MMCSD_BUS_WIDTH_4);
    }

    host->card = card;

    return 0;

err1:
    os_free(card);
err:

    return err;
}

/**
 * @brief init_sd:Starting point for SD card init.
 * 
 * @param host 
 * @param ocr       OCR的值
 * @return os_int32_t 
 */
os_int32_t init_sd(struct os_mmcsd_host *host, os_uint32_t ocr)
{
    os_int32_t  err;
    os_uint32_t current_ocr;

    // 如果使用SPI模式，需要使用另一种方法来获取OCR
    if (controller_is_spi(host))
    {
        mmcsd_go_idle(host);                        // 发送CMD0，card进入IDLE状态

        err = mmcsd_spi_read_ocr(host, 0, &ocr);    // 发送CMD58，读取OCR
        if (err)
            goto err;
    }

    if (ocr & VDD_165_195)                          // 低压不支持？？？
    {
        LOG_I(DBG_TAG," SD card claims to support the "
                  "incompletely defined 'low voltage range'. This "
                  "will be ignored.");
        ocr &= ~VDD_165_195;
    }

    current_ocr = mmcsd_select_voltage(host, ocr);      // 选择电压，设置IO电压属性

    /*
     * Can we support the voltage(s) of the card(s)?
     */
    if (!current_ocr)
    {
        err = OS_ERROR;
        goto err;
    }

    /*
     * Detect and init the card.
     */
    err = mmcsd_sd_init_card(host, current_ocr);        // 初始化CARD
    if (err)
        goto err;

    mmcsd_host_unlock(host);                            // 解锁HOST

    err = os_mmcsd_blk_probe(host->card);
    if (err)
        goto remove_card;
    mmcsd_host_lock(host);

    return 0;

remove_card:
    mmcsd_host_lock(host);
    os_mmcsd_blk_remove(host->card);
    os_free(host->card);
    host->card = OS_NULL;
err:

    LOG_D(DBG_TAG, "init SD card failed!");

    return err;
}
