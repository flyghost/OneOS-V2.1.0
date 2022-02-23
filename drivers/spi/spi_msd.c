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
 * @file        spi_msd.c
 *
 * @brief       This file provides functions for spi msd.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "spi_msd.h"
#include <device.h>
#include <os_memory.h>
#include <os_mutex.h>
#include <os_clock.h>

#ifdef MSD_TRACE
#define MSD_DEBUG(...)         os_kprintf("[MSD] %d ", os_tick_get()); os_kprintf(__VA_ARGS__);
#else
#define MSD_DEBUG(...)
#endif /* #ifdef MSD_TRACE */

#define DUMMY 0xFF

#define CARD_NCR_MAX 9

#define CARD_NRC 1
#define CARD_NCR 1

static struct msd_device _msd_device;

/* Function define */
static os_bool_t os_tick_timeout(os_tick_t tick_start, os_tick_t tick_long);

static os_err_t MSD_take_owner(struct os_spi_device *spi_device);

static os_err_t  _wait_token(struct os_spi_device *device, uint8_t token);
static os_err_t  _wait_ready(struct os_spi_device *device);

static os_err_t MSD_take_owner(struct os_spi_device *spi_device)
{
    os_err_t result;

    result = os_mutex_lock(&(spi_device->bus->lock), OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        if (spi_device->bus->owner != spi_device)
        {
            /* Not the same owner as current, re-configure SPI bus */
            result = spi_device->bus->ops->configure(spi_device, &spi_device->config);
            if (result == OS_EOK)
            {
                /* Set SPI bus owner */
                spi_device->bus->owner = spi_device;
            }
        }
        os_mutex_unlock(&(spi_device->bus->lock));
    }

    return result;
}

static os_bool_t os_tick_timeout(os_tick_t tick_start, os_tick_t tick_long)
{
    os_tick_t tick_end = tick_start + tick_long;
    os_tick_t tick_now = os_tick_get();
    os_bool_t result   = OS_FALSE;

    if (tick_end >= tick_start)
    {
        if (tick_now >= tick_end)
        {
            result = OS_TRUE;
        }
        else
        {
            result = OS_FALSE;
        }
    }
    else
    {
        if ((tick_now < tick_start) && (tick_now >= tick_end))
        {
            result = OS_TRUE;
        }
        else
        {
            result = OS_FALSE;
        }
    }

    return result;
}

static uint8_t crc7(const uint8_t *buf, int len)
{
    unsigned char i, j, crc, ch, ch2, ch3;

    crc = 0;

    for (i = 0; i < len; i++)
    {
        ch = buf[i];

        for (j = 0; j < 8; j++, ch <<= 1)
        {
            ch2 = (crc & 0x40) ? 1 : 0;
            ch3 = (ch & 0x80) ? 1 : 0;

            if (ch2 ^ ch3)
            {
                crc ^= 0x04;
                crc <<= 1;
                crc |= 0x01;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static os_err_t _send_cmd(
    struct os_spi_device *device,
    uint8_t cmd,
    uint32_t arg,
    uint8_t crc,
    response_type type,
    uint8_t *response
)
{
    struct os_spi_message message;
    uint8_t  cmd_buffer[8];
    uint8_t  recv_buffer[sizeof(cmd_buffer)];
    uint32_t i;

    cmd_buffer[0] = DUMMY;
    cmd_buffer[1] = (cmd | 0x40);
    cmd_buffer[2] = (uint8_t)(arg >> 24);
    cmd_buffer[3] = (uint8_t)(arg >> 16);
    cmd_buffer[4] = (uint8_t)(arg >> 8);
    cmd_buffer[5] = (uint8_t)(arg);

    if (crc == 0x00)
    {
        crc = crc7(&cmd_buffer[1], 5);
        crc = (crc << 1) | 0x01;
    }
    cmd_buffer[6] = (crc);

    cmd_buffer[7] = DUMMY;

    /* Initial message */
    message.send_buf = cmd_buffer;
    message.recv_buf = recv_buffer;
    message.length   = sizeof(cmd_buffer);
    message.cs_take = message.cs_release = 0;

    _wait_ready(device);

    /* Transfer message */
    device->bus->ops->xfer(device, &message);

    for (i = CARD_NCR; i < (CARD_NCR_MAX + 1); i++)
    {
        uint8_t send = DUMMY;

        /* Initial message */
        message.send_buf = &send;
        message.recv_buf = response;
        message.length   = 1;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);

        if (0 == (response[0] & 0x80))
        {
            break;
        }
    } /* Wait response */

    if ((CARD_NCR_MAX + 1) == i)
    {
        return OS_ERROR;
    }

    /* recieve other byte */
    if (type == response_r1)
    {
        return OS_EOK;
    }
    else if (type == response_r1b)
    {
        os_tick_t tick_start = os_tick_get();
        uint8_t   recv;

        while (1)
        {
            /* Initial message */
            message.send_buf = OS_NULL;
            message.recv_buf = &recv;
            message.length   = 1;
            message.cs_take = message.cs_release = 0;

            /* Transfer message */
            device->bus->ops->xfer(device, &message);

            if (recv == DUMMY)
            {
                return OS_EOK;
            }

            if (os_tick_timeout(tick_start, os_tick_from_ms(2000)))
            {
                return OS_ETIMEOUT;
            }
        }
    }
    else if (type == response_r2)
    {
        /* Initial message */
        message.send_buf = OS_NULL;
        message.recv_buf = response + 1;
        message.length   = 1;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    }
    else if ((type == response_r3) || (type == response_r7))
    {
        /* Initial message */
        message.send_buf = OS_NULL;
        message.recv_buf = response + 1;
        message.length   = 4;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    }
    else
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t _wait_token(struct os_spi_device *device, uint8_t token)
{
    struct os_spi_message message;
    os_tick_t tick_start;
    uint8_t   send, recv;

    tick_start = os_tick_get();

    /* Initial message */
    send             = DUMMY;
    message.send_buf = &send;
    message.recv_buf = &recv;
    message.length   = 1;
    message.cs_take = message.cs_release = 0;

    while (1)
    {
        /* Transfer message */
        device->bus->ops->xfer(device, &message);

        if (recv == token)
        {
            return OS_EOK;
        }

        if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_WAIT_TOKEN_TIMES)))
        {
            MSD_DEBUG("[err] wait data start token timeout!\r\n");
            return OS_ETIMEOUT;
        }
    } /* Wati token */
}

static os_err_t _wait_ready(struct os_spi_device *device)
{
    struct os_spi_message message;
    os_tick_t tick_start;
    uint8_t   send, recv;

    tick_start = os_tick_get();

    send = DUMMY;
    /* Initial message */
    message.send_buf = &send;
    message.recv_buf = &recv;
    message.length   = 1;
    message.cs_take = message.cs_release = 0;

    while (1)
    {
        /* Transfer message */
        device->bus->ops->xfer(device, &message);

        if (recv == DUMMY)
        {
            return OS_EOK;
        }

        if (os_tick_timeout(tick_start, os_tick_from_ms(1000)))
        {
            MSD_DEBUG("[err] wait ready timeout!\r\n");
            return OS_ETIMEOUT;
        }
    }
}

static os_err_t _read_block(struct os_spi_device *device, void *buffer, uint32_t block_size)
{
    struct os_spi_message message;
    os_err_t result;

    /* Wati token */
    result = _wait_token(device, MSD_TOKEN_READ_START);
    if (result != OS_EOK)
    {
        return result;
    }

    /* Read data */
    {
        /* Initial message */
        message.send_buf = OS_NULL;
        message.recv_buf = buffer;
        message.length   = block_size;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    } /* Read data */

    /* Get crc */
    {
        uint8_t recv_buffer[2];

        /* Initial message */
        message.send_buf = OS_NULL;
        message.recv_buf = recv_buffer;
        message.length   = 2;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    } /* Get crc */

    return OS_EOK;
}

static os_err_t _write_block(struct os_spi_device *device, const void *buffer, uint32_t block_size, uint8_t token)
{
    struct os_spi_message message;
    uint8_t send_buffer[16];

    memset(send_buffer, DUMMY, sizeof(send_buffer));
    send_buffer[sizeof(send_buffer) - 1] = token;

    /* Send start block token */
    {
        /* Initial message */
        message.send_buf = send_buffer;
        message.recv_buf = OS_NULL;
        message.length   = sizeof(send_buffer);
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    }

    /* Send data */
    {
        /* Initial message */
        message.send_buf = buffer;
        message.recv_buf = OS_NULL;
        message.length   = block_size;
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);
    }

    /* Put crc and get data response */
    {
        uint8_t recv_buffer[3];
        uint8_t response;

        /* Initial message */
        message.send_buf = send_buffer;
        message.recv_buf = recv_buffer;
        message.length   = sizeof(recv_buffer);
        message.cs_take = message.cs_release = 0;

        /* Transfer message */
        device->bus->ops->xfer(device, &message);

#if 0
        response = 0x0E & recv_buffer[2];
#endif
        response = MSD_GET_DATA_RESPONSE(recv_buffer[2]);
        if (response != MSD_DATA_OK)
        {
            MSD_DEBUG("[err] write block fail! data response : 0x%02X\r\n", response);
            return OS_ERROR;
        }
    }

    /* wati ready */
    return _wait_ready(device);
}

static os_err_t msd_blk_init(os_blk_device_t *blk);

static int msd_blk_read_block(os_blk_device_t *blk, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = OS_ERROR;
    struct msd_device *msd = (struct msd_device *)blk;
    uint8_t            response[MSD_RESPONSE_MAX_LEN];
    os_err_t           result = OS_EOK;

    result = MSD_take_owner(msd->spi_device);

    if (result != OS_EOK)
    {
        goto _exit;
    }

    if (page_nr == 1)
    {
        os_spi_take(msd->spi_device);

        result = _send_cmd(msd->spi_device,
                           READ_SINGLE_BLOCK,
                           page_addr * msd->blk_dev.geometry.block_size,
                           0x00,
                           response_r1,
                           response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] read SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
            goto _exit;
        }

        result = _read_block(msd->spi_device, buff, msd->blk_dev.geometry.block_size);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] read SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
        }
    }
    else if (page_nr > 1)
    {
        uint32_t i;

        os_spi_take(msd->spi_device);

        result = _send_cmd(msd->spi_device,
                           READ_MULTIPLE_BLOCK,
                           page_addr * msd->blk_dev.geometry.block_size,
                           0x00,
                           response_r1,
                           response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
            goto _exit;
        }

        for (i = 0; i < page_nr; i++)
        {
            result = _read_block(msd->spi_device,
                                 (uint8_t *)buff + msd->blk_dev.geometry.block_size * i,
                                 msd->blk_dev.geometry.block_size);
            if (result != OS_EOK)
            {
                MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK #%d fail!\r\n", page_addr);
                page_nr = i;
                break;
            }
        }

        /* Send CMD12 stop transfer */
        result = _send_cmd(msd->spi_device, STOP_TRANSMISSION, 0x00, 0x00, response_r1b, response);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK, send stop token fail!\r\n");
        }
    } /* READ_MULTIPLE_BLOCK */

    ret = OS_EOK;

_exit:
    /* Release and exit */
    os_spi_release(msd->spi_device);
    os_mutex_unlock(&(msd->spi_device->bus->lock));

    return ret;
}

static int msd_blk_write_block(os_blk_device_t *blk, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = OS_ERROR;
    struct msd_device *msd = (struct msd_device *)blk;
    uint8_t            response[MSD_RESPONSE_MAX_LEN];
    os_err_t           result;

    result = MSD_take_owner(msd->spi_device);

    if (result != OS_EOK)
    {
        MSD_DEBUG("[err] get SPI owner fail!\r\n");
        goto _exit;
    }

    if (page_nr == 1)
    {
        os_spi_take(msd->spi_device);
        result =
            _send_cmd(msd->spi_device, WRITE_BLOCK, page_addr * msd->blk_dev.geometry.block_size, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD WRITE_BLOCK fail!\r\n");
            page_nr = 0;
            goto _exit;
        }

        result = _write_block(msd->spi_device, buff, msd->blk_dev.geometry.block_size, MSD_TOKEN_WRITE_SINGLE_START);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] write SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
        }
    }
    else if (page_nr > 1)
    {
        struct os_spi_message message;
        uint32_t i;

        os_spi_take(msd->spi_device);

#ifdef MSD_USE_PRE_ERASED
        if (msd->card_type != MSD_CARD_TYPE_MMC)
        {
            /* CMD55 APP_CMD */
            result = _send_cmd(msd->spi_device, APP_CMD, 0x00, 0x00, response_r1, response);
            if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
            {
                MSD_DEBUG("[err] CMD55 APP_CMD fail!\r\n");
                page_nr = 0;
                goto _exit;
            }

            /* ACMD23 Pre-erased */
            result = _send_cmd(msd->spi_device, SET_WR_BLK_ERASE_COUNT, page_nr, 0x00, response_r1, response);
            if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
            {
                MSD_DEBUG("[err] ACMD23 SET_BLOCK_COUNT fail!\r\n");
                page_nr = 0;
                goto _exit;
            }
        }
#endif

        result = _send_cmd(msd->spi_device,
                           WRITE_MULTIPLE_BLOCK,
                           page_addr * msd->blk_dev.geometry.block_size,
                           0x00,
                           response_r1,
                           response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD WRITE_MULTIPLE_BLOCK fail!\r\n");
            page_nr = 0;
            goto _exit;
        }

        /* Write all block */
        for (i = 0; i < page_nr; i++)
        {
            result = _write_block(msd->spi_device,
                                  (const uint8_t *)buff + msd->blk_dev.geometry.block_size * i,
                                  msd->blk_dev.geometry.block_size,
                                  MSD_TOKEN_WRITE_MULTIPLE_START);
            if (result != OS_EOK)
            {
                MSD_DEBUG("[err] write SINGLE_BLOCK #%d fail!\r\n", page_addr);
                page_nr = i;
                break;
            }
        } /* Write all block */

        /* Send stop token */
        {
            uint8_t send_buffer[18];

            memset(send_buffer, DUMMY, sizeof(send_buffer));
            send_buffer[sizeof(send_buffer) - 1] = MSD_TOKEN_WRITE_MULTIPLE_STOP;

            /* Initial message */
            message.send_buf = send_buffer;
            message.recv_buf = OS_NULL;
            message.length   = sizeof(send_buffer);
            message.cs_take = message.cs_release = 0;

            /* Transfer message */
            msd->spi_device->bus->ops->xfer(msd->spi_device, &message);
        }

        /* Wait ready */
        result = _wait_ready(msd->spi_device);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[warning] wait WRITE_MULTIPLE_BLOCK stop token ready timeout!\r\n");
        }
    } /* size > 1 */

    ret = OS_EOK;

_exit:
    /* Release and exit */
    os_spi_release(msd->spi_device);

    return ret;
}

const static struct os_blk_ops msd_blk_ops =
{
    .read_block   = msd_blk_read_block,
    .write_block  = msd_blk_write_block,
};

static int msd_sdhc_blk_read_block(os_blk_device_t *blk, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = OS_ERROR;
    struct msd_device *msd = (struct msd_device *)blk;
    uint8_t            response[MSD_RESPONSE_MAX_LEN];
    os_err_t           result = OS_EOK;

    result = MSD_take_owner(msd->spi_device);

    if (result != OS_EOK)
    {
        goto _exit;
    }

    if (page_nr == 1)
    {
        os_spi_take(msd->spi_device);

        result = _send_cmd(msd->spi_device, READ_SINGLE_BLOCK, page_addr, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] read SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
            goto _exit;
        }

        result = _read_block(msd->spi_device, buff, msd->blk_dev.geometry.block_size);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] read SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
        }
    }
    else if (page_nr > 1)
    {
        uint32_t i;

        os_spi_take(msd->spi_device);

        result = _send_cmd(msd->spi_device, READ_MULTIPLE_BLOCK, page_addr, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
            goto _exit;
        }

        for (i = 0; i < page_nr; i++)
        {
            result = _read_block(msd->spi_device,
                                 (uint8_t *)buff + msd->blk_dev.geometry.block_size * i,
                                 msd->blk_dev.geometry.block_size);
            if (result != OS_EOK)
            {
                MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK #%d fail!\r\n", page_addr);
                page_nr = i;
                break;
            }
        }

        /* Send CMD12 stop transfer */
        result = _send_cmd(msd->spi_device, STOP_TRANSMISSION, 0x00, 0x00, response_r1b, response);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] read READ_MULTIPLE_BLOCK, send stop token fail!\r\n");
        }
    } /* READ_MULTIPLE_BLOCK */

    ret = OS_EOK;
    
_exit:
    /* Release and exit */
    os_spi_release(msd->spi_device);

    return ret;
}

static int msd_sdhc_blk_write_block(os_blk_device_t *blk, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int ret = OS_ERROR;
    struct msd_device *msd = (struct msd_device *)blk;
    uint8_t            response[MSD_RESPONSE_MAX_LEN];
    os_err_t           result;

    result = MSD_take_owner(msd->spi_device);

    if (result != OS_EOK)
    {
        goto _exit;
    }

    /* SINGLE_BLOCK? */
    if (page_nr == 1)
    {
        os_spi_take(msd->spi_device);
        result = _send_cmd(msd->spi_device, WRITE_BLOCK, page_addr, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD WRITE_BLOCK fail!\r\n");
            page_nr = 0;
            goto _exit;
        }

        result = _write_block(msd->spi_device, buff, msd->blk_dev.geometry.block_size, MSD_TOKEN_WRITE_SINGLE_START);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] write SINGLE_BLOCK #%d fail!\r\n", page_addr);
            page_nr = 0;
        }
    }
    else if (page_nr > 1)
    {
        struct os_spi_message message;
        uint32_t              i;

        os_spi_take(msd->spi_device);

#ifdef MSD_USE_PRE_ERASED
        /* CMD55 APP_CMD */
        result = _send_cmd(msd->spi_device, APP_CMD, 0x00, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD55 APP_CMD fail!\r\n");
            page_nr = 0;
            goto _exit;
        }

        /* ACMD23 Pre-erased */
        result = _send_cmd(msd->spi_device, SET_WR_BLK_ERASE_COUNT, page_nr, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] ACMD23 SET_BLOCK_COUNT fail!\r\n");
            page_nr = 0;
            goto _exit;
        }
#endif

        result = _send_cmd(msd->spi_device, WRITE_MULTIPLE_BLOCK, page_addr, 0x00, response_r1, response);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD WRITE_MULTIPLE_BLOCK fail!\r\n");
            page_nr = 0;
            goto _exit;
        }

        /* Write all block */
        for (i = 0; i < page_nr; i++)
        {
            result = _write_block(msd->spi_device,
                                  (const uint8_t *)buff + msd->blk_dev.geometry.block_size * i,
                                  msd->blk_dev.geometry.block_size,
                                  MSD_TOKEN_WRITE_MULTIPLE_START);
            if (result != OS_EOK)
            {
                MSD_DEBUG("[err] write MULTIPLE_BLOCK #%d fail!\r\n", page_addr);
                page_nr = i;
                break;
            }
        } /* Write all block */

        /* Send stop token */
        {
            uint8_t send_buffer[18];

            memset(send_buffer, DUMMY, sizeof(send_buffer));
            send_buffer[sizeof(send_buffer) - 1] = MSD_TOKEN_WRITE_MULTIPLE_STOP;

            /* Initial message */
            message.send_buf = send_buffer;
            message.recv_buf = OS_NULL;
            message.length   = sizeof(send_buffer);
            message.cs_take = message.cs_release = 0;

            /* Transfer message */
            msd->spi_device->bus->ops->xfer(msd->spi_device, &message);
        }

        result = _wait_ready(msd->spi_device);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[warning] wait WRITE_MULTIPLE_BLOCK stop token ready timeout!\r\n");
        }
    } /* size > 1 */

    ret = OS_EOK;

_exit:
    /* Release and exit */
    os_spi_release(msd->spi_device);

    return ret;
}

const static struct os_blk_ops msd_sdhc_blk_ops = 
{
    .read_block   = msd_sdhc_blk_read_block,
    .write_block  = msd_sdhc_blk_write_block,
};

static os_err_t msd_blk_init(os_blk_device_t *blk)
{
    struct msd_device *msd = (struct msd_device *)blk;
    uint8_t            response[MSD_RESPONSE_MAX_LEN];
    os_err_t           result = OS_EOK;
    os_tick_t          tick_start;
    uint32_t           OCR;

    if (msd->spi_device == OS_NULL)
    {
        MSD_DEBUG("[err] the SPI SD device has no SPI!\r\n");
        return OS_EIO;
    }

    /* Config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 */
        cfg.max_hz     = 1000 * 400;                 /* 400kbit/s */
        os_spi_configure(msd->spi_device, &cfg);
    } /* Config spi */

    /* Init SD card */
    {
        struct os_spi_message message;

        result = MSD_take_owner(msd->spi_device);

        if (result != OS_EOK)
        {
            goto _exit;
        }

        os_spi_release(msd->spi_device);

        /*
         * The host shall supply power to the card so that the voltage is reached to Vdd_min within 250ms and
         * start to supply at least 74 SD clocks to the SD card with keeping CMD line to high.
         * In case of SPI mode, CS shall be held to high during 74 clock cycles.
         */
        {
            uint8_t send_buffer[100]; /* 100byte > 74 clock */

            /* Initial message */
            memset(send_buffer, DUMMY, sizeof(send_buffer));
            message.send_buf = send_buffer;
            message.recv_buf = OS_NULL;
            message.length   = sizeof(send_buffer);
            message.cs_take = message.cs_release = 0;

            /* Transfer message */
            msd->spi_device->bus->ops->xfer(msd->spi_device, &message);
        } /* send 74 clock */

        /* Send CMD0 (GO_IDLE_STATE) to put MSD in SPI mode */
        {
            tick_start = os_tick_get();

            while (1)
            {
                os_spi_take(msd->spi_device);
                result = _send_cmd(msd->spi_device, GO_IDLE_STATE, 0x00, 0x95, response_r1, response);
                os_spi_release(msd->spi_device);

                if ((result == OS_EOK) && (response[0] == MSD_IN_IDLE_STATE))
                {
                    break;
                }

                if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_TRY_TIMES)))
                {
                    MSD_DEBUG("[err] SD card goto IDLE mode timeout!\r\n");
                    result = OS_ETIMEOUT;
                    goto _exit;
                }
            }

            MSD_DEBUG("[info] SD card goto IDLE mode OK!\r\n");
        } /* Send CMD0 (GO_IDLE_STATE) to put MSD in SPI mode */

        /* CMD8 */
        {
            tick_start = os_tick_get();

            do
            {
                os_spi_take(msd->spi_device);
                result = _send_cmd(msd->spi_device, SEND_IF_COND, 0x01AA, 0x87, response_r7, response);
                os_spi_release(msd->spi_device);

                if (result == OS_EOK)
                {
                    MSD_DEBUG("[info] CMD8 response : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
                              response[0],
                              response[1],
                              response[2],
                              response[3],
                              response[4]);

                    if (response[0] & (1 << 2))
                    {
                        /* Illegal command, SD V1.x or MMC card */
                        MSD_DEBUG("[info] CMD8 is illegal command.\r\n");
                        MSD_DEBUG("[info] maybe Ver1.X SD Memory Card or MMC card!\r\n");
                        msd->card_type = MSD_CARD_TYPE_SD_V1_X;
                        break;
                    }
                    else
                    {
                        /* SD V2.0 or later or SDHC or SDXC memory card! */
                        MSD_DEBUG("[info] Ver2.00 or later or SDHC or SDXC memory card!\r\n");
                        msd->card_type = MSD_CARD_TYPE_SD_V2_X;
                    }

                    if ((0xAA == response[4]) && (0x00 == response[3]))
                    {
                        /* SD2.0 not support current voltage */
                        MSD_DEBUG("[err] VCA = 0, SD2.0 not surpport current operation voltage range\r\n");
                        result = OS_ERROR;
                        goto _exit;
                    }
                }
                else
                {
                    if (os_tick_timeout(tick_start, os_tick_from_ms(200)))
                    {
                        MSD_DEBUG("[err] CMD8 SEND_IF_COND timeout!\r\n");
                        result = OS_ETIMEOUT;
                        goto _exit;
                    }
                }
            } while (0xAA != response[4]);
        } /* CMD8 */

        /* Ver1.X SD Memory Card or MMC card */
        if (msd->card_type == MSD_CARD_TYPE_SD_V1_X)
        {
            os_bool_t is_sd_v1_x = OS_FALSE;
            os_tick_t tick_start;

            /* Try SD Ver1.x */
            while (1)
            {
                os_spi_take(msd->spi_device);

                result = _send_cmd(msd->spi_device, READ_OCR, 0x00, 0x00, response_r3, response);
                if (result != OS_EOK)
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[info] It maybe SD1.x or MMC But it is Not response to CMD58!\r\n");
                    goto _exit;
                }

                if (0 != (response[0] & 0xFE))
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[info] It look CMD58 as illegal command so it is not SD card!\r\n");
                    break;
                }
                os_spi_release(msd->spi_device);

                OCR = response[1];
                OCR = (OCR << 8) + response[2];
                OCR = (OCR << 8) + response[3];
                OCR = (OCR << 8) + response[4];
                MSD_DEBUG("[info] OCR is 0x%08X\r\n", OCR);

                if (0 == (OCR & (0x1 << 15)))
                {
                    MSD_DEBUG(("[err] SD 1.x But not surpport current voltage\r\n"));
                    result = OS_ERROR;
                    goto _exit;
                }

                /* --Send ACMD41 to make card ready */
                tick_start = os_tick_get();

                /* Try CMD55 + ACMD41 */
                while (1)
                {
                    if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_TRY_TIMES_ACMD41)))
                    {
                        os_spi_release(msd->spi_device);
                        MSD_DEBUG("[info] try CMD55 + ACMD41 timeout! mabey MMC card!\r\n");
                        break;
                    }

                    os_spi_take(msd->spi_device);

                    /* CMD55 APP_CMD */
                    result = _send_cmd(msd->spi_device, APP_CMD, 0x00, 0x00, response_r1, response);
                    if (result != OS_EOK)
                    {
                        os_spi_release(msd->spi_device);
                        continue;
                    }

                    if (0 != (response[0] & 0xFE))
                    {
                        os_spi_release(msd->spi_device);
                        MSD_DEBUG("[info] Not SD card2 , may be MMC\r\n");
                        break;
                    }

                    /* ACMD41 SD_SEND_OP_COND */
                    result = _send_cmd(msd->spi_device, SD_SEND_OP_COND, 0x00, 0x00, response_r1, response);
                    if (result != OS_EOK)
                    {
                        os_spi_release(msd->spi_device);
                        continue;
                    }

                    if (0 != (response[0] & 0xFE))
                    {
                        os_spi_release(msd->spi_device);
                        MSD_DEBUG("[info] Not SD card4 , may be MMC\r\n");
                        break;
                    }

                    if (0 == (response[0] & 0xFF))
                    {
                        os_spi_release(msd->spi_device);
                        is_sd_v1_x = OS_TRUE;
                        MSD_DEBUG("[info] It is Ver1.X SD Memory Card!!!\r\n");
                        break;
                    }
                } /* try CMD55 + ACMD41 */

                break;
            } /* try SD Ver1.x */

            /* Try MMC */
            if (is_sd_v1_x != OS_TRUE)
            {
                uint32_t i;

                MSD_DEBUG("[info] try MMC card!\r\n");
                os_spi_release(msd->spi_device);

                /* Send dummy clock */
                {
                    uint8_t send_buffer[100];

                    /* Initial message */
                    memset(send_buffer, DUMMY, sizeof(send_buffer));
                    message.send_buf = send_buffer;
                    message.recv_buf = OS_NULL;
                    message.length   = sizeof(send_buffer);
                    message.cs_take = message.cs_release = 0;

                    for (i = 0; i < 10; i++)
                    {
                        /* Transfer message */
                        msd->spi_device->bus->ops->xfer(msd->spi_device, &message);
                    }
                } /* Send dummy clock */

                /* Send CMD0 goto IDLE state */
                tick_start = os_tick_get();
                while (1)
                {
                    os_spi_take(msd->spi_device);
                    result = _send_cmd(msd->spi_device, GO_IDLE_STATE, 0x00, 0x95, response_r1, response);
                    os_spi_release(msd->spi_device);

                    if ((result == OS_EOK) && (response[0] == MSD_IN_IDLE_STATE))
                    {
                        break;
                    }

                    if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_TRY_TIMES)))
                    {
                        MSD_DEBUG("[err] SD card goto IDLE mode timeout!\r\n");
                        result = OS_ETIMEOUT;
                        goto _exit;
                    }
                } /* Send CMD0 goto IDLE stat */

                /* Send CMD1 */
                tick_start = os_tick_get();
                while (1)
                {
                    os_spi_take(msd->spi_device);
                    result = _send_cmd(msd->spi_device, SEND_OP_COND, 0x00, 0x00, response_r1, response);
                    os_spi_release(msd->spi_device);

                    if ((result == OS_EOK) && (response[0] == MSD_RESPONSE_NO_ERROR))
                    {
                        MSD_DEBUG("[info] It is MMC card!!!\r\n");
                        msd->card_type = MSD_CARD_TYPE_MMC;
                        break;
                    }

                    if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_TRY_TIMES)))
                    {
                        MSD_DEBUG("[err] SD card goto IDLE mode timeout!\r\n");
                        result = OS_ETIMEOUT;
                        goto _exit;
                    }
                } /* Send CMD1 */
            }     /* Try MMC */
        }
        else if (msd->card_type == MSD_CARD_TYPE_SD_V2_X)
        {
            os_spi_take(msd->spi_device);

            result = _send_cmd(msd->spi_device, READ_OCR, 0x00, 0x00, response_r3, response);
            if (result != OS_EOK)
            {
                os_spi_release(msd->spi_device);
                MSD_DEBUG("[err] It maybe SD2.0 But it is Not response to CMD58!\r\n");
                goto _exit;
            }

            if ((response[0] & 0xFE) != 0)
            {
                os_spi_release(msd->spi_device);
                MSD_DEBUG("[err] It look CMD58 as illegal command so it is not SD card!\r\n");
                result = OS_ERROR;
                goto _exit;
            }

            os_spi_release(msd->spi_device);

            OCR = response[1];
            OCR = (OCR << 8) + response[2];
            OCR = (OCR << 8) + response[3];
            OCR = (OCR << 8) + response[4];
            MSD_DEBUG("[info] OCR is 0x%08X\r\n", OCR);

            if (0 == (OCR & (0x1 << 15)))
            {
                MSD_DEBUG(("[err] SD 1.x But not surpport current voltage\r\n"));
                result = OS_ERROR;
                goto _exit;
            }

            /* --Send ACMD41 to make card ready */
            tick_start = os_tick_get();

            /* Try CMD55 + ACMD41 */
            do
            {
                os_spi_take(msd->spi_device);
                if (os_tick_timeout(tick_start, os_tick_from_ms(CARD_TRY_TIMES_ACMD41)))
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[err] SD Ver2.x or later try CMD55 + ACMD41 timeout!\r\n");
                    result = OS_ERROR;
                    goto _exit;
                }

                /* CMD55 APP_CMD */
                result = _send_cmd(msd->spi_device, APP_CMD, 0x00, 0x65, response_r1, response);
                if (result != OS_EOK)
                {
                    os_spi_release(msd->spi_device);
                    continue;
                }

                if ((response[0] & 0xFE) != 0)
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[err] Not SD ready!\r\n");
                    result = OS_ERROR;
                    goto _exit;
                }

                /* ACMD41 SD_SEND_OP_COND */
                result = _send_cmd(msd->spi_device, SD_SEND_OP_COND, 0x40000000, 0x77, response_r1, response);
                if (result != OS_EOK)
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[err] ACMD41 fail!\r\n");
                    result = OS_ERROR;
                    goto _exit;
                }

                if ((response[0] & 0xFE) != 0)
                {
                    os_spi_release(msd->spi_device);
                    MSD_DEBUG("[info] Not SD card4 , response : 0x%02X\r\n", response[0]);
                    //                    break;
                }
            } while (response[0] != MSD_RESPONSE_NO_ERROR);
            os_spi_release(msd->spi_device);
            /* Try CMD55 + ACMD41 */

            /* Read OCR again */
            os_spi_take(msd->spi_device);
            result = _send_cmd(msd->spi_device, READ_OCR, 0x00, 0x00, response_r3, response);
            if (result != OS_EOK)
            {
                os_spi_release(msd->spi_device);
                MSD_DEBUG("[err] It maybe SD2.0 But it is Not response to 2nd CMD58!\r\n");
                goto _exit;
            }

            if ((response[0] & 0xFE) != 0)
            {
                os_spi_release(msd->spi_device);
                MSD_DEBUG("[err] It look 2nd CMD58 as illegal command so it is not SD card!\r\n");
                result = OS_ERROR;
                goto _exit;
            }
            os_spi_release(msd->spi_device);

            OCR = response[1];
            OCR = (OCR << 8) + response[2];
            OCR = (OCR << 8) + response[3];
            OCR = (OCR << 8) + response[4];
            MSD_DEBUG("[info] OCR 2nd read is 0x%08X\r\n", OCR);

            if ((OCR & 0x40000000) != 0)
            {
                MSD_DEBUG("[info] It is SD2.0 SDHC Card!!!\r\n");
                msd->card_type = MSD_CARD_TYPE_SD_SDHC;
            }
            else
            {
                MSD_DEBUG("[info] It is SD2.0 standard capacity Card!!!\r\n");
            }
        } /* MSD_CARD_TYPE_SD_V2_X */
        else
        {
            MSD_DEBUG("[err] SD card type unkonw!\r\n");
            result = OS_ERROR;
            goto _exit;
        }
    } /* Init SD card */

    if (msd->card_type == MSD_CARD_TYPE_SD_SDHC)
    {
        blk->blk_ops = &msd_sdhc_blk_ops;
    }
    else
    {
        blk->blk_ops = &msd_blk_ops;
    }

    /* Set CRC */
    {
        os_spi_release(msd->spi_device);
        os_spi_take(msd->spi_device);
#ifdef MSD_USE_CRC
        result = _send_cmd(msd->spi_device, CRC_ON_OFF, 0x01, 0x83, response_r1, response);
#else
        result     = _send_cmd(msd->spi_device, CRC_ON_OFF, 0x00, 0x91, response_r1, response);
#endif
        os_spi_release(msd->spi_device);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD59 CRC_ON_OFF fail! response : 0x%02X\r\n", response[0]);
            result = OS_ERROR;
            goto _exit;
        }
    } /* Set CRC */

    /* CMD16 SET_BLOCKLEN */
    {
        os_spi_release(msd->spi_device);
        os_spi_take(msd->spi_device);
        result = _send_cmd(msd->spi_device, SET_BLOCKLEN, SECTOR_SIZE, 0x00, response_r1, response);
        os_spi_release(msd->spi_device);
        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            MSD_DEBUG("[err] CMD16 SET_BLOCKLEN fail! response : 0x%02X\r\n", response[0]);
            result = OS_ERROR;
            goto _exit;
        }
        msd->blk_dev.geometry.block_size = SECTOR_SIZE;
    }

    /* Read CSD */
    {
        uint8_t CSD_buffer[MSD_CSD_LEN];

        os_spi_take(msd->spi_device);
        result = _send_cmd(msd->spi_device, SEND_CSD, 0x00, 0x00, response_r1, response);

        if (result != OS_EOK)
        {
            os_spi_release(msd->spi_device);
            MSD_DEBUG("[err] CMD9 SEND_CSD timeout!\r\n");
            goto _exit;
        }

        if ((result != OS_EOK) || (response[0] != MSD_RESPONSE_NO_ERROR))
        {
            os_spi_release(msd->spi_device);
            MSD_DEBUG("[err] CMD9 SEND_CSD fail! response : 0x%02X\r\n", response[0]);
            result = OS_ERROR;
            goto _exit;
        }

        result = _read_block(msd->spi_device, CSD_buffer, MSD_CSD_LEN);
        os_spi_release(msd->spi_device);
        if (result != OS_EOK)
        {
            MSD_DEBUG("[err] read CSD fail!\r\n");
            goto _exit;
        }

        /* Analyze CSD */
        {
            uint8_t  CSD_STRUCTURE;
            uint32_t C_SIZE;
            uint32_t card_capacity;

            uint8_t  tmp8;
            uint16_t tmp16;
            uint32_t tmp32;

            /* Get CSD_STRUCTURE */
            tmp8          = CSD_buffer[0] & 0xC0; /* 0b11000000 */
            CSD_STRUCTURE = tmp8 >> 6;

            /* MMC CSD Analyze. */
            if (msd->card_type == MSD_CARD_TYPE_MMC)
            {
                uint8_t C_SIZE_MULT;
                uint8_t READ_BL_LEN;

                if (CSD_STRUCTURE > 2)
                {
                    MSD_DEBUG("[err] bad CSD Version : %d\r\n", CSD_STRUCTURE);
                    result = OS_ERROR;
                    goto _exit;
                }

                if (CSD_STRUCTURE == 0)
                {
                    MSD_DEBUG("[info] CSD version No. 1.0\r\n");
                }
                else if (CSD_STRUCTURE == 1)
                {
                    MSD_DEBUG("[info] CSD version No. 1.1\r\n");
                }
                else if (CSD_STRUCTURE == 2)
                {
                    MSD_DEBUG("[info] CSD version No. 1.2\r\n");
                }

                /* Get TRAN_SPEED 8bit [103:96] */
                tmp8 = CSD_buffer[3];
                tmp8 &= 0x03; /* [2:0] transfer rate unit.*/
                if (tmp8 == 0)
                {
                    msd->max_clock = 100 * 1000; /* 0=100kbit/s. */
                }
                else if (tmp8 == 1)
                {
                    msd->max_clock = 1 * 1000 * 1000; /* 1=1Mbit/s. */
                }
                else if (tmp8 == 2)
                {
                    msd->max_clock = 10 * 1000 * 1000; /* 2=10Mbit/s. */
                }
                else if (tmp8 == 3)
                {
                    msd->max_clock = 100 * 1000 * 1000; /* 3=100Mbit/s. */
                }
                if (tmp8 == 0)
                {
                    MSD_DEBUG("[info] TRAN_SPEED: 0x%02X, %dkbit/s.\r\n", tmp8, msd->max_clock / 1000);
                }
                else
                {
                    MSD_DEBUG("[info] TRAN_SPEED: 0x%02X, %dMbit/s.\r\n", tmp8, msd->max_clock / 1000 / 1000);
                }

                /* Get READ_BL_LEN 4bit [83:80] */
                tmp8        = CSD_buffer[5] & 0x0F; /* 0b00001111; */
                READ_BL_LEN = tmp8;                 /* 4 bit */
                MSD_DEBUG("[info] CSD : READ_BL_LEN : %d %dbyte\r\n", READ_BL_LEN, (1 << READ_BL_LEN));

                /* Get C_SIZE 12bit [73:62] */
                tmp16 = CSD_buffer[6] & 0x03; /* Get [73:72] 0b00000011 */
                tmp16 = tmp16 << 8;
                tmp16 += CSD_buffer[7]; /* Get [71:64] */
                tmp16  = tmp16 << 2;
                tmp8   = CSD_buffer[8] & 0xC0; /* Get [63:62] 0b11000000 */
                tmp8   = tmp8 >> 6;
                tmp16  = tmp16 + tmp8;
                C_SIZE = tmp16;    // 12 bit
                MSD_DEBUG("[info] CSD : C_SIZE : %d\r\n", C_SIZE);

                /* Get C_SIZE_MULT 3bit [49:47] */
                tmp8        = CSD_buffer[9] & 0x03;    // 0b00000011;
                tmp8        = tmp8 << 1;
                tmp8        = tmp8 + ((CSD_buffer[10] & 0x80 /*0b10000000*/) >> 7);
                C_SIZE_MULT = tmp8;    // 3 bit
                MSD_DEBUG("[info] CSD : C_SIZE_MULT : %d\r\n", C_SIZE_MULT);

                card_capacity = (1 << READ_BL_LEN) * ((C_SIZE + 1) * (1 << (C_SIZE_MULT + 2)));
                msd->blk_dev.geometry.capacity = card_capacity;
                MSD_DEBUG("[info] card capacity : %d Mbyte\r\n", card_capacity / (1024 * 1024));
            }
            else /* SD CSD Analyze. */
            {
                if (CSD_STRUCTURE == 0)
                {
                    uint8_t C_SIZE_MULT;
                    uint8_t READ_BL_LEN;

                    MSD_DEBUG("[info] CSD Version 1.0\r\n");

                    /* Get TRAN_SPEED 8bit [103:96] */
                    tmp8 = CSD_buffer[3];
                    if (tmp8 == 0x32)
                    {
                        msd->max_clock = 1000 * 1000 * 10; /* 10Mbit/s. */
                    }
                    else if (tmp8 == 0x5A)
                    {
                        msd->max_clock = 1000 * 1000 * 50; /* 50Mbit/s. */
                    }
                    else
                    {
                        msd->max_clock = 1000 * 1000 * 1; /* 1Mbit/s default. */
                    }
                    MSD_DEBUG("[info] TRAN_SPEED: 0x%02X, %dMbit/s.\r\n", tmp8, msd->max_clock / 1000 / 1000);

                    /* Get READ_BL_LEN 4bit [83:80] */
                    tmp8        = CSD_buffer[5] & 0x0F; /* 0b00001111; */
                    READ_BL_LEN = tmp8;                 /* 4 bit */
                    MSD_DEBUG("[info] CSD : READ_BL_LEN : %d %dbyte\r\n", READ_BL_LEN, (1 << READ_BL_LEN));

                    /* Get C_SIZE 12bit [73:62] */
                    tmp16 = CSD_buffer[6] & 0x03; /* Get [73:72] 0b00000011 */
                    tmp16 = tmp16 << 8;
                    tmp16 += CSD_buffer[7]; /* Get [71:64] */
                    tmp16  = tmp16 << 2;
                    tmp8   = CSD_buffer[8] & 0xC0; /* Get [63:62] 0b11000000 */
                    tmp8   = tmp8 >> 6;
                    tmp16  = tmp16 + tmp8;
                    C_SIZE = tmp16;    // 12 bit
                    MSD_DEBUG("[info] CSD : C_SIZE : %d\r\n", C_SIZE);

                    /* Get C_SIZE_MULT 3bit [49:47] */
                    tmp8        = CSD_buffer[9] & 0x03;    // 0b00000011;
                    tmp8        = tmp8 << 1;
                    tmp8        = tmp8 + ((CSD_buffer[10] & 0x80 /*0b10000000*/) >> 7);
                    C_SIZE_MULT = tmp8;    // 3 bit
                    MSD_DEBUG("[info] CSD : C_SIZE_MULT : %d\r\n", C_SIZE_MULT);

                    card_capacity = (1 << READ_BL_LEN) * ((C_SIZE + 1) * (1 << (C_SIZE_MULT + 2)));
                    msd->blk_dev.geometry.capacity = card_capacity;
                    MSD_DEBUG("[info] card capacity : %d Mbyte\r\n", card_capacity / (1024 * 1024));
                }
                else if (CSD_STRUCTURE == 1)
                {
                    MSD_DEBUG("[info] CSD Version 2.0\r\n");

                    /* Get TRAN_SPEED 8bit [103:96] */
                    tmp8 = CSD_buffer[3];
                    if (tmp8 == 0x32)
                    {
                        msd->max_clock = 1000 * 1000 * 10; /* 10Mbit/s. */
                    }
                    else if (tmp8 == 0x5A)
                    {
                        msd->max_clock = 1000 * 1000 * 50; /* 50Mbit/s. */
                    }
                    else if (tmp8 == 0x0B)
                    {
                        msd->max_clock = 1000 * 1000 * 100; /* 100Mbit/s. */
                        /* UHS50 Card sets TRAN_SPEED to 0Bh (100Mbit/sec), */
                        /* For both SDR50 and DDR50 modes. */
                    }
                    else if (tmp8 == 0x2B)
                    {
                        msd->max_clock = 1000 * 1000 * 200; /* 200Mbit/s. */
                        /* UHS104 Card sets TRAN_SPEED to 2Bh (200Mbit/sec). */
                    }
                    else
                    {
                        msd->max_clock = 1000 * 1000 * 1; /* 1Mbit/s default. */
                    }
                    MSD_DEBUG("[info] TRAN_SPEED: 0x%02X, %dMbit/s.\r\n", tmp8, msd->max_clock / 1000 / 1000);

                    /* Get C_SIZE 22bit [69:48] */
                    tmp32 = CSD_buffer[7] & 0x3F; /* 0b00111111 */
                    tmp32 = tmp32 << 8;
                    tmp32 += CSD_buffer[8];
                    tmp32 = tmp32 << 8;
                    tmp32 += CSD_buffer[9];
                    C_SIZE = tmp32;
                    MSD_DEBUG("[info] CSD : C_SIZE : %d\r\n", C_SIZE);

                    /* Memory capacity = (C_SIZE+1) * 512K byte */
                    card_capacity = (C_SIZE + 1) / 2;    /* Unit : Mbyte */
                    msd->blk_dev.geometry.capacity = card_capacity; /* 512KB = 1024sector */
                    MSD_DEBUG("[info] card capacity : %d.%d Gbyte\r\n",
                              card_capacity / 1024,
                              (card_capacity % 1024) * 100 / 1024);
                    MSD_DEBUG("[info] sector_count : %d\r\n", 
                              msd->blk_dev.geometry.capacity / msd->blk_dev.geometry.block_size);
                }
                else
                {
                    MSD_DEBUG("[err] bad CSD Version : %d\r\n", CSD_STRUCTURE);
                    result = OS_ERROR;
                    goto _exit;
                }
            } /* SD CSD Analyze. */
        }     /* Analyze CSD */

    } /* Read CSD */

    /* Config spi to high speed */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 */
        cfg.max_hz     = msd->max_clock;
        os_spi_configure(msd->spi_device, &cfg);
    } /* Config spi */

_exit:
    os_spi_release(msd->spi_device);
    return result;
}

os_err_t msd_init(const char *sd_device_name, const char *spi_device_name)
{
    os_err_t              result = OS_EOK;
    struct os_spi_device *spi_device;

    spi_device = (struct os_spi_device *)os_device_find(spi_device_name);
    if (spi_device == OS_NULL)
    {
        MSD_DEBUG("spi device %s not found!\r\n", spi_device_name);
        return OS_ENOSYS;
    }
    memset(&_msd_device, 0, sizeof(_msd_device));
    _msd_device.spi_device = spi_device;

    _msd_device.blk_dev.geometry.block_size = 0;
    _msd_device.blk_dev.geometry.capacity   = 0;
    
    _msd_device.blk_dev.blk_ops = &msd_blk_ops;

    result = msd_blk_init(&_msd_device.blk_dev);

    if (result == OS_EOK)
    {
        result = block_device_register(&_msd_device.blk_dev, sd_device_name);
    }

    return result;
}
