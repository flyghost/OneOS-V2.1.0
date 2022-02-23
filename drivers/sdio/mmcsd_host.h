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
 * @file        mmcsd_host.h
 *
 * @brief       This file provides struct/macro definition for mmcsd_host.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HOST_H__
#define __HOST_H__

#include <os_task.h>
#include <os_sem.h>
#include <os_mutex.h>

#ifdef __cplusplus
extern "C" {
#endif

struct os_mmcsd_io_cfg
{
    os_uint32_t clock; /* clock rate */
    os_uint16_t vdd;

    /* vdd stores the bit number of the selected voltage range from below. */

    os_uint8_t bus_mode; /* command output mode */

#define MMCSD_BUSMODE_OPENDRAIN 1
#define MMCSD_BUSMODE_PUSHPULL  2

    os_uint8_t chip_select; /* SPI chip select */

#define MMCSD_CS_IGNORE 0
#define MMCSD_CS_HIGH   1
#define MMCSD_CS_LOW    2

    os_uint8_t power_mode; /* power supply mode */

#define MMCSD_POWER_OFF 0
#define MMCSD_POWER_UP  1
#define MMCSD_POWER_ON  2

    os_uint8_t bus_width; /* data bus width */

#define MMCSD_BUS_WIDTH_1 0
#define MMCSD_BUS_WIDTH_4 2
#define MMCSD_BUS_WIDTH_8 3
};

struct os_mmcsd_host;
struct os_mmcsd_req;

/**
 ***********************************************************************************************************************
 * @struct      os_mmcsd_host_ops
 *
 * @brief       structure of operation functions set
 ***********************************************************************************************************************
 */
struct os_mmcsd_host_ops
{
    void (*request)(struct os_mmcsd_host *host, struct os_mmcsd_req *req);
    void (*set_iocfg)(struct os_mmcsd_host *host, struct os_mmcsd_io_cfg *io_cfg);
    os_int32_t (*get_card_status)(struct os_mmcsd_host *host);
    void (*enable_sdio_irq)(struct os_mmcsd_host *host, os_int32_t en);
};

/**
 ***********************************************************************************************************************
 * @struct      os_mmcsd_host
 *
 * @brief       structure of os_mmcsd_host
 ***********************************************************************************************************************
 */
struct os_mmcsd_host
{
    struct os_mmcsd_card           *card;
    const struct os_mmcsd_host_ops *ops;
    os_uint32_t                     freq_min;
    os_uint32_t                     freq_max;
    struct os_mmcsd_io_cfg          io_cfg;
    os_uint32_t                     valid_ocr; /* current valid OCR */
#define VDD_165_195 (1 << 7)                   /* VDD voltage 1.65 - 1.95 */
#define VDD_20_21   (1 << 8)                   /* VDD voltage 2.0 ~ 2.1 */
#define VDD_21_22   (1 << 9)                   /* VDD voltage 2.1 ~ 2.2 */
#define VDD_22_23   (1 << 10)                  /* VDD voltage 2.2 ~ 2.3 */
#define VDD_23_24   (1 << 11)                  /* VDD voltage 2.3 ~ 2.4 */
#define VDD_24_25   (1 << 12)                  /* VDD voltage 2.4 ~ 2.5 */
#define VDD_25_26   (1 << 13)                  /* VDD voltage 2.5 ~ 2.6 */
#define VDD_26_27   (1 << 14)                  /* VDD voltage 2.6 ~ 2.7 */
#define VDD_27_28   (1 << 15)                  /* VDD voltage 2.7 ~ 2.8 */
#define VDD_28_29   (1 << 16)                  /* VDD voltage 2.8 ~ 2.9 */
#define VDD_29_30   (1 << 17)                  /* VDD voltage 2.9 ~ 3.0 */
#define VDD_30_31   (1 << 18)                  /* VDD voltage 3.0 ~ 3.1 */
#define VDD_31_32   (1 << 19)                  /* VDD voltage 3.1 ~ 3.2 */
#define VDD_32_33   (1 << 20)                  /* VDD voltage 3.2 ~ 3.3 */
#define VDD_33_34   (1 << 21)                  /* VDD voltage 3.3 ~ 3.4 */
#define VDD_34_35   (1 << 22)                  /* VDD voltage 3.4 ~ 3.5 */
#define VDD_35_36   (1 << 23)                  /* VDD voltage 3.5 ~ 3.6 */
    os_uint32_t flags;                         /* define device capabilities */
#define MMCSD_BUSWIDTH_4        (1 << 0)
#define MMCSD_BUSWIDTH_8        (1 << 1)
#define MMCSD_MUTBLKWRITE       (1 << 2)
#define MMCSD_HOST_IS_SPI       (1 << 3)
#define controller_is_spi(host) (host->flags & MMCSD_HOST_IS_SPI)
#define MMCSD_SUP_SDIO_IRQ      (1 << 4) /* support signal pending SDIO IRQs */
#define MMCSD_SUP_HIGHSPEED     (1 << 5) /* support high speed */

    os_uint32_t max_seg_size;  /* maximum size of one dma segment */
    os_uint32_t max_dma_segs;  /* maximum number of dma segments in one request */
    os_uint32_t max_blk_size;  /* maximum block size */
    os_uint32_t max_blk_count; /* maximum block count */

    os_uint32_t         spi_use_crc;
    struct os_mutex     bus_lock;
    struct os_semaphore sem_ack;

    os_uint32_t          sdio_irq_num;
    struct os_semaphore *sdio_irq_sem;
    os_task_t           *sdio_irq_thread;

    void *private_data;
};

OS_INLINE void mmcsd_delay_ms(os_uint32_t ms)
{
    if (ms < 1000 / OS_TICK_PER_SECOND)
    {
        os_task_msleep(1);
    }
    else
    {
        os_task_msleep(ms / (1000 / OS_TICK_PER_SECOND));
    }
}

#ifdef __cplusplus
}
#endif

#endif
