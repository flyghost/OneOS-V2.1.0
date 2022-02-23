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
 * @brief       This file implements sdio driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_hal.h"
#include <os_event.h>
#include <os_memory.h>
#include <bus/bus.h>

#include <board.h>
#include <block/block_device.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sdio"
#include <drv_log.h>

#include "drv_uart.h"
#include "sdio_sdcard.h"

#define DATA_BLOCK_COUNT (62333952U)

typedef struct mm32_sd_device
{
    os_blk_device_t blk_dev;
}mm32_sd_device_t;

static int mm32_sdio_read_block(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);
    
    if (0 != SD_ReadDisk(buff, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "read addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

static int mm32_sdio_write_block(os_blk_device_t *blk, os_uint32_t block_addr,const os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);
    
    if (0 != SD_WriteDisk(buff, block_addr << 9, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "write addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

const static struct os_blk_ops mm32_sdio_blk_ops = {
    .read_block   = mm32_sdio_read_block,
    .write_block  = mm32_sdio_write_block,
};

void SDIO_PIN_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

//    PC8     SDCard_DAT0
//    PC9     SDCard_DAT1
//    PC10    SDCard_DAT2
//    PC11    SDCard_DAT3
//    PC12    SDCard_CLK
//    PD2     SDCard_CMD

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC | RCC_AHBENR_GPIOD, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);                         //enable sys_cfg clk

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_12);                       //PD2   SDIO_CMD
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_12);                       //PC8   SDIO_D0
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_12);                       //PC9   SDIO_D1
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_12);                      //PC10  SDIO_D2
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_12);                      //PC11  SDIO_D3
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_12);                      //PC12  SDIO_CLK

    //set PD2 AF SDCard_CMD
    GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //PC8 AF SDCard_DAT0, PC9 AF SDCard_DAT1, PC10 AF SDCard_DAT2,
    //PC11 AF SDCard_DAT3, PC12 AF SDCard_CLK
    GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_8  | \
                                    GPIO_Pin_9  | \
                                    GPIO_Pin_10 | \
                                    GPIO_Pin_11 | \
                                    GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

extern u32 SystemCoreClock;
void SDIO_ConfigInit(void)
{
    u8 clk = 0;
    SDIO_InitTypeDef SDIO_InitStruct = {0};

    SDIO_PIN_GPIO_Config();

    RCC_AHBPeriphClockCmd(RCC_AHBENR_SDIO, ENABLE);

    clk = (SystemCoreClock / 2000000 - 1);
    SDIO_ClockSet(clk);

    SDIO_StructInit(&SDIO_InitStruct);
    SDIO_InitStruct.SDIO_SelPTSM = SDIO_MMC_CTRL_SelPTSM;
    SDIO_InitStruct.SDIO_SelSM = SDIO_MMC_CTRL_SelSM;
    SDIO_InitStruct.SDIO_OPMSel = SDIO_MMC_CTRL_OPMSel;
    SDIO_InitStruct.SDIO_DATWT = SDIO_MMC_CTRL_DATWT;
    SDIO_Init(&SDIO_InitStruct);

    SDIO_CRCConfig(SDIO_MMC_CRCCTL_CMD_CRCEN | SDIO_MMC_CRCCTL_DAT_CRCEN, ENABLE);
}

void show_sdcard_info(void)
{
    switch(SDCardInfo.CardType) {
        case SDIO_STD_CAPACITY_SD_CARD_V1_1:
            os_kprintf("Card Type:SDSC V1.1\r\n");
            break;
        case SDIO_STD_CAPACITY_SD_CARD_V2_0:
            os_kprintf("Card Type:SDSC V2.0\r\n");
            break;
        case SDIO_HIGH_CAPACITY_SD_CARD:
            os_kprintf("Card Type:SDHC V2.0\r\n");
            break;
        case SDIO_MULTIMEDIA_CARD:
            os_kprintf("Card Type:MMC Card\r\n");
            break;
    }
    os_kprintf("Card ManufacturerID:%d\r\n", SDCardInfo.SD_cid.ManufacturerID); //The manufacturer ID
    os_kprintf("Card RCA:%d\r\n", SDCardInfo.RCA);                              //Card relative address
    os_kprintf("Card Capacity:%d MB\r\n", (u32)(SDCardInfo.CardCapacity >> 20));
    os_kprintf("Card BlockSize:%d\r\n\r\n", SDCardInfo.CardBlockSize);
}



int mm32_sd_hardware_init(void)
{
    SD_Error result;
    uint16_t retry_cnt = 0;
    SDIO_ConfigInit();
    os_kprintf("SDCARD Initialing\r\n");
    
    while(1) 
    {
        retry_cnt++;
        result = SD_Init();

        if(result == SD_OK) 
        {
            break;
        }
        if (retry_cnt > 10)
        {
            LOG_E(DRV_EXT_TAG, "SD Card Error!\r\n");
            return -1;
        }
    }
    show_sdcard_info();
    return 0;
}


static int mm32_sd_device_init(void)
{
    char sd_device_name[] = "sd0";
    struct mm32_sd_device * sd_dev;
    int ret;
    
    ret = mm32_sd_hardware_init();
    
    if (ret != 0)
    {
        return OS_EOK;
    }
    
    sd_dev = os_calloc(1, sizeof(struct mm32_sd_device));
    if (sd_dev == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "failed!");
        return OS_ENOMEM;
    }
    
    

    sd_dev->blk_dev.geometry.block_size = SDCardInfo.CardBlockSize;
    sd_dev->blk_dev.geometry.capacity   = SDCardInfo.CardCapacity;
    sd_dev->blk_dev.blk_ops = &mm32_sdio_blk_ops;

    block_device_register(&sd_dev->blk_dev, sd_device_name);
    
    LOG_I(DRV_EXT_TAG, "sd init success!");

    return OS_EOK;
}

OS_DEVICE_INIT(mm32_sd_device_init, OS_INIT_SUBLEVEL_LOW);

