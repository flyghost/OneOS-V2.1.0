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
 * @file        mmcsd_cmd.h
 *
 * @brief       This file provides various command words(macro definition) of the card.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMD_H__
#define __CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************基础命令********************************************************/

/* class 1 */
#define GO_IDLE_STATE       0  /* bc                          */    // 复位所有卡到IDLE状态
#define SEND_OP_COND        1  /* bcr  [31:0] OCR         R3  */
#define ALL_SEND_CID        2  /* bcr                     R2  */    // 所有的卡在CMD线上回复CID号
#define SET_RELATIVE_ADDR   3  /* ac   [31:16] RCA        R1  */    // 卡发布一个新的相对地址
#define SET_DSR             4  /* bc   [31:16] RCA            */    // 编程所有卡的DSR
#define SWITCH              6  /* ac   [31:0] See below   R1b */
#define SELECT_CARD         7  /* ac   [31:16] RCA        R1  */    // 选择/不选择卡
#define SEND_EXT_CSD        8  /* adtc                    R1  */    // 发送卡接口条件：支持的电压，并询问卡是否支持
#define SEND_CSD            9  /* ac   [31:16] RCA        R2  */    // 选定的卡在CMD线上回复卡细节数据CSD（card specific data）
#define SEND_CID            10 /* ac   [31:16] RCA        R2  */    // 选定的卡在CMD线上回复卡标志CID信息
#define READ_DAT_UNTIL_STOP 11 /* adtc [31:0] dadr        R1  */    // 
#define STOP_TRANSMISSION   12 /* ac                      R1b */    // 强制卡停止传输
#define SEND_STATUS         13 /* ac   [31:16] RCA        R1  */    // 选定的卡在CMD线上回复状态寄存器信息
#define GO_INACTIVE_STATE   15 /* ac   [31:16] RCA            */    // 选定的卡进入inactive状态
#define SPI_READ_OCR        58 /* spi                  spi_R3 */
#define SPI_CRC_ON_OFF      59 /* spi  [0:0] flag      spi_R1 */

/*******************************************************面向块的读操作********************************************************/
/* class 2 */
#define SET_BLOCKLEN        16 /* ac   [31:0] block len   R1  */    // 设置所有块长度（高容量SD卡无意义，因为块大小默认为512Byte）
#define READ_SINGLE_BLOCK   17 /* adtc [31:0] data addr   R1  */    // 读取一个块的数据(长度为CMD16设置的长度)
#define READ_MULTIPLE_BLOCK 18 /* adtc [31:0] data addr   R1  */    // 读取连续的数据块，直到CMD12命令停止，块长度和CMD17一样

/* class 3 */
#define WRITE_DAT_UNTIL_STOP 20 /* adtc [31:0] data addr   R1  */

/*******************************************************面向块的写操作********************************************************/
/* class 4 */
#define SET_BLOCK_COUNT      23 /* adtc [31:0] data addr   R1  */   // 为CMD18和CMD25（连续读写）指定的块长度
#define WRITE_BLOCK          24 /* adtc [31:0] data addr   R1  */   // 写入一个块的数据
#define WRITE_MULTIPLE_BLOCK 25 /* adtc                    R1  */   // 写入连续的数据块，直到CMD12命令停止
#define PROGRAM_CID          26 /* adtc                    R1  */   // 为厂商保留，可编程的CID
#define PROGRAM_CSD          27 /* adtc                    R1  */   // 为厂商保留，可编程的CSD

/*******************************************************面向块的写保护命令********************************************************/
/* class 6 */
#define SET_WRITE_PROT  28 /* ac   [31:0] data addr   R1b */    // 如果卡支持写保护功能，这个命令设置地址组中的写保护位
#define CLR_WRITE_PROT  29 /* ac   [31:0] data addr   R1b */    // 如果卡支持写保护功能，这个命令清除寻址组的写保护位
#define SEND_WRITE_PROT 30 /* adtc [31:0] wpdata addr R1  */

/*******************************************************擦除命令********************************************************/
/* class 5 */
#define ERASE_GROUP_START 35 /* ac   [31:0] data addr   R1  */  // 设置要擦除的第一个块的地址
#define ERASE_GROUP_END   36 /* ac   [31:0] data addr   R1  */  // 设置要擦除的最后一个块的地址
#define ERASE             38 /* ac                      R1b */  // 擦除所有预先选定的写块

/* class 9 */
#define FAST_IO      39 /* ac   <Complex>          R4  */
#define GO_IRQ_STATE 40 /* bcr                     R5  */

/*******************************************************加锁命令********************************************************/
/* class 7 */
#define LOCK_UNLOCK 42 /* adtc                    R1b */        // 用来设置/复位密码，或者加锁/解锁卡

/*******************************************************特定应用命令********************************************************/
/* class 8 */
#define APP_CMD 55 /* ac   [31:16] RCA        R1  */            // 告诉卡，下个命令是特定应用命令，而不是标准命令
#define GEN_CMD 56 /* adtc [0] RD/WR          R1  */            // 通用命令，或者是特定应用命令中，用于传输一个数据块到卡、或者卡获取一个数据块。bit0=1为读，bit0=0为写

/*******************************************************SD卡特殊命令：需要再之前先发送CMD55********************************************************/

/* SD commands                           type  argument     response */
/* class 0 */
/* This is basically the same command as for MMC with some quirks. */
#define SD_SEND_RELATIVE_ADDR 3 /* bcr                     R6  */
#define SD_SEND_IF_COND       8 /* bcr  [11:0] See below   R7  */

/* class 10 */
#define SD_SWITCH 6 /* adtc [31:0] See below   R1  */

/* Application commands */
#define SD_APP_SET_BUS_WIDTH    6  /* ac   [1:0] bus width    R1  */
#define SD_APP_SEND_NUM_WR_BLKS 22 /* adtc                    R1  */
#define SD_APP_OP_COND          41 /* bcr  [31:0] OCR         R3  */
#define SD_APP_SEND_SCR         51 /* adtc                    R1  */

#define SCR_SPEC_VER_0 0 /* Implements system specification 1.0 - 1.01 */
#define SCR_SPEC_VER_1 1 /* Implements system specification 1.10 */
#define SCR_SPEC_VER_2 2 /* Implements system specification 2.00 */

/*******************************************************SDIO卡特殊命令********************************************************/

/* SDIO commands                                          type  argument     response */
#define SD_IO_SEND_OP_COND 5  /* bcr  [23:0] OCR         R4  */
#define SD_IO_RW_DIRECT    52 /* ac   [31:0] See below   R5  */
#define SD_IO_RW_EXTENDED  53 /* adtc [31:0] See below   R5  */

/* CMD52 arguments */
#define SDIO_ARG_CMD52_READ       (0 << 31)
#define SDIO_ARG_CMD52_WRITE      (1u << 31)
#define SDIO_ARG_CMD52_FUNC_SHIFT 28
#define SDIO_ARG_CMD52_FUNC_MASK  0x7
#define SDIO_ARG_CMD52_RAW_FLAG   (1u << 27)
#define SDIO_ARG_CMD52_REG_SHIFT  9
#define SDIO_ARG_CMD52_REG_MASK   0x1ffff
#define SDIO_ARG_CMD52_DATA_SHIFT 0
#define SDIO_ARG_CMD52_DATA_MASK  0xff
#define SDIO_R5_DATA(resp)        ((resp)[0] & 0xff)

/* CMD53 arguments */
#define SDIO_ARG_CMD53_READ         (0 << 31)
#define SDIO_ARG_CMD53_WRITE        (1u << 31)
#define SDIO_ARG_CMD53_FUNC_SHIFT   28
#define SDIO_ARG_CMD53_FUNC_MASK    0x7
#define SDIO_ARG_CMD53_BLOCK_MODE   (1u << 27)
#define SDIO_ARG_CMD53_INCREMENT    (1u << 26)
#define SDIO_ARG_CMD53_REG_SHIFT    9
#define SDIO_ARG_CMD53_REG_MASK     0x1ffff
#define SDIO_ARG_CMD53_LENGTH_SHIFT 0
#define SDIO_ARG_CMD53_LENGTH_MASK  0x1ff
#define SDIO_ARG_CMD53_LENGTH_MAX   511

#ifdef __cplusplus
}
#endif

#endif
