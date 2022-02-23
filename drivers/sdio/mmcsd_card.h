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
 * @file        mmcsd_card.h
 *
 * @brief       This file provides mmcsd_card struct/macro definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MMCSD_CARD_H__
#define __MMCSD_CARD_H__

#include <sdio/mmcsd_host.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_SCR_BUS_WIDTH_1 (1 << 0)
#define SD_SCR_BUS_WIDTH_4 (1 << 2)

/**
 ***********************************************************************************************************************
 * @struct      os_mmcsd_cid
 *
 * @brief       structure of mmc manufacturer information
 ***********************************************************************************************************************
 */
struct os_mmcsd_cid
{
    os_uint8_t  mid;       /* ManufacturerID */
    os_uint8_t  prv;       /* Product Revision */
    os_uint16_t oid;       /* OEM/Application ID */
    os_uint32_t psn;       /* Product Serial Number */
    os_uint8_t  pnm[5];    /* Product Name */
    os_uint8_t  reserved1; /* reserved */
    os_uint16_t mdt;       /* Manufacturing Date */
    os_uint8_t  crc;       /* CID CRC */
    os_uint8_t  reserved2; /* not used, always 1 */
};

struct os_mmcsd_csd
{
    os_uint8_t  csd_structure; /* CSD register version */
    os_uint8_t  taac;
    os_uint8_t  nsac;
    os_uint8_t  tran_speed;     /* max data transfer rate */
    os_uint16_t card_cmd_class; /* card command classes */
    os_uint8_t  rd_blk_len;     /* max read data block length */
    os_uint8_t  rd_blk_part;
    os_uint8_t  wr_blk_misalign;
    os_uint8_t  rd_blk_misalign;
    os_uint8_t  dsr_imp;     /* DSR implemented */
    os_uint8_t  c_size_mult; /* CSD 1.0 , device size multiplier */
    os_uint32_t c_size;      /* device size */
    os_uint8_t  r2w_factor;
    os_uint8_t  wr_blk_len; /* max wtire data block length */
    os_uint8_t  wr_blk_partial;
    os_uint8_t  csd_crc;
};

struct os_sd_scr
{
    os_uint8_t sd_version;
    os_uint8_t sd_bus_widths;
};

struct os_sdio_cccr 
{
os_uint8_t  sdio_version;
os_uint8_t  sd_version;
os_uint8_t  direct_cmd:1,     /*  Card Supports Direct Commands during data transfer
                                               only SD mode, not used for SPI mode */
            multi_block:1,    /*  Card Supports Multi-Block */
            read_wait:1,      /*  Card Supports Read Wait only SD mode, not used for SPI mode */
            suspend_resume:1, /*  Card supports Suspend/Resume only SD mode, not used for SPI mode */
            s4mi:1,            /* generate interrupts during a 4-bit multi-block data transfer */
            e4mi:1,            /*  Enable the multi-block IRQ during 4-bit transfer for the SDIO card */
            low_speed:1,      /*  Card  is  a  Low-Speed  card */
            low_speed_4:1;    /*  4-bit support for Low-Speed cards */

os_uint8_t  bus_width:1,     /* Support SDIO bus width, 1:4bit, 0:1bit */
            cd_disable:1,    /*  Connect[0]/Disconnect[1] the 10K-90K ohm pull-up 
                                 resistor on CD/DAT[3] (pin 1) of the card */
            power_ctrl:1,    /* Support Master Power Control */
            high_speed:1;    /* Support High-Speed  */            
};

struct os_sdio_cis
{
    os_uint16_t manufacturer;
    os_uint16_t product;
    os_uint16_t func0_blk_size;
    os_uint32_t max_tran_speed;
};

/*
 * SDIO function CIS tuple (unknown to the core)
 */
struct os_sdio_function_tuple
{
    struct os_sdio_function_tuple *next;
    os_uint8_t                     code;
    os_uint8_t                     size;
    os_uint8_t *                   data;
};

struct os_sdio_function;
typedef void(os_sdio_irq_handler_t)(struct os_sdio_function *);

/*
 * SDIO function devices
 */
struct os_sdio_function
{
    struct os_mmcsd_card * card;               /* the card this device belongs to */
    os_sdio_irq_handler_t *irq_handler;        /* IRQ callback */
    os_uint8_t             num;                /* function number */
    os_uint8_t             func_code;          /*  Standard SDIO Function interface code  */
    os_uint16_t            manufacturer;       /* manufacturer id */
    os_uint16_t            product;            /* product id */
    os_uint32_t            max_blk_size;       /* maximum block size */
    os_uint32_t            cur_blk_size;       /* current block size */
    os_uint32_t            enable_timeout_val; /* max enable timeout in msec */

    struct os_sdio_function_tuple *tuples;
    void *                         priv;
};

#define SDIO_MAX_FUNCTIONS 7

struct os_mmcsd_card
{
    struct os_mmcsd_host *host;
    os_uint32_t           rca;         /* card addr */
    os_uint32_t           resp_cid[4]; /* card CID register */
    os_uint32_t           resp_csd[4]; /* card CSD register */
    os_uint32_t           resp_scr[2]; /* card SCR register */

    os_uint16_t tacc_clks;     /* data access time by ns */
    os_uint32_t tacc_ns;       /* data access time by clk cycles */
    os_uint32_t max_data_rate; /* max data transfer rate */
    os_uint32_t card_capacity; /* card capacity, unit:KB */
    os_uint32_t card_blksize;  /* card block size */
    os_uint32_t erase_size;    /* erase size in sectors */
    os_uint16_t card_type;
#define CARD_TYPE_MMC        0 /* MMC card */
#define CARD_TYPE_SD         1 /* SD card */
#define CARD_TYPE_SDIO       2 /* SDIO card */
#define CARD_TYPE_SDIO_COMBO 3 /* SD combo (IO+mem) card */

    os_uint16_t flags;
#define CARD_FLAG_HIGHSPEED (1 << 0) /* SDIO bus speed 50MHz */
#define CARD_FLAG_SDHC      (1 << 1) /* SDHC card */
#define CARD_FLAG_SDXC      (1 << 2) /* SDXC card */

    struct os_sd_scr    scr;
    struct os_mmcsd_csd csd;
    os_uint32_t         hs_max_data_rate; /* max data transfer rate in high speed mode */

    os_uint8_t               sdio_function_num;                     /* totol number of SDIO functions */
    struct os_sdio_cccr      cccr;                                  /* common card info */
    struct os_sdio_cis       cis;                                   /* common tuple info */
    struct os_sdio_function *sdio_function[SDIO_MAX_FUNCTIONS + 1]; /* SDIO functions (devices) */
};

#ifdef __cplusplus
}
#endif

#endif
