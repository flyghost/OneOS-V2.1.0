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
 * @file        spi_flash_nm25q.c
 *
 * @brief       this file implements spi flash nm25q related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include <oneos_config.h>
#include <os_memory.h>
#include <fal.h>
#include "spi_flash_nm25q.h"

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

static struct spi_flash_info nm25q_info;

static os_uint8_t nm25q_statusregister_read(os_uint8_t reg)
{
    os_uint8_t cmd;
    os_uint8_t sta;

    switch(reg)
    {
    case 1:
        cmd = NM25Q_ReadStatusReg1;
        break;
    case 2:
        cmd = NM25Q_ReadStatusReg2;
        break;
    case 3:
        cmd = NM25Q_ReadStatusReg3;
        break;
    default:
        cmd = NM25Q_ReadStatusReg1;
        break;
    }
    sta = os_spi_sendrecv8(nm25q_info.spi_device, cmd);

    return sta;
}

static void nm25q_write_enable(void)
{
    os_uint8_t cmd = NM25Q_WriteEnable;

    os_spi_sendrecv8(nm25q_info.spi_device, cmd);
}

static void nm25q_wait_busy(void)
{
    while((nm25q_statusregister_read(1) & 0x01) == 0x01);
}

static os_err_t nm25q_attch(const char *flash_device_name)
{
    const char *spi_device_name = BSP_NM25Q_SPI_BUS;

    struct os_spi_device *hw_spi_device;

    os_hw_spi_device_attach(spi_device_name, NM25Q_DEV_NAME, BSP_NM25Q_SPI_CS);

    hw_spi_device = (struct os_spi_device *)os_device_find(NM25Q_DEV_NAME);
    if (hw_spi_device == OS_NULL)
    {
        os_kprintf("spi device %s not found!\r\n", NM25Q_DEV_NAME);
        return OS_ENOSYS;
    }
    nm25q_info.spi_device = hw_spi_device;

    /* config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB;
        cfg.max_hz     = 66000000;
        os_spi_configure(nm25q_info.spi_device, &cfg);
    }

    /*send dummy byte*/
    os_uint8_t dummy = NM25Q_Dummy_FF;
    os_uint8_t dummy_recv;
    os_spi_send_then_recv(nm25q_info.spi_device, &dummy, 1, &dummy_recv, 1);

    /*read id*/
    os_uint8_t id_cmd[4] = {NM25Q_ManufactDeviceID, NM25Q_Dummy_00, NM25Q_Dummy_00, NM25Q_Dummy_00};
    os_uint8_t id_recv[2] = {0, 0};
    os_spi_send_then_recv(nm25q_info.spi_device, id_cmd, 4, id_recv, 2);

    if(id_recv[0] == NM25Q_MFID)
    {
        nm25q_info.chip_data.id = NM25Q_MFID;
    }
    else
    {
        return OS_ERROR;
    }

    switch(id_recv[1])
    {
    case NM25Q_TYPE08:
        nm25q_info.chip_data.capacity = 8 * 1024 * 1024 / 8;
        break;
    case NM25Q_TYPE16:
        nm25q_info.chip_data.capacity = 16 * 1024 * 1024 / 8;
        break;
    case NM25Q_TYPE32:
        nm25q_info.chip_data.capacity = 32 * 1024 * 1024 / 8;
        break;
    case NM25Q_TYPE64:
        nm25q_info.chip_data.capacity = 64 * 1024 * 1024 / 8;
        break;
    case NM25Q_TYPE128:
        nm25q_info.chip_data.capacity = 128 * 1024 * 1024 / 8;
        break;
    case NM25Q_TYPE256:
        nm25q_info.chip_data.capacity = 256 * 1024 * 1024 / 8;
        break;
    default:
        break;
    }
    nm25q_info.chip_data.page_size = 256;
    nm25q_info.chip_data.erase_size = 4 * 1024;

    nm25q_write_enable();
    nm25q_wait_busy();
    return OS_EOK;
}
static int nm25q_read(os_uint8_t *buff, os_uint32_t addr, os_uint32_t size)
{
    os_uint8_t  cmd[4];
    memset(cmd, 0, sizeof(cmd));

    cmd[0] = NM25Q_ReadData;
    cmd[1] = (os_uint8_t)(addr >> 16);
    cmd[2] = (os_uint8_t)(addr >> 8);
    cmd[3] = (os_uint8_t)(addr);

    os_spi_send_then_recv(nm25q_info.spi_device, cmd, 4, buff, size);

    return OS_EOK;
}

static int nm25q_write_page(os_uint8_t *buff, os_uint32_t addr, os_uint32_t size)
{
    os_uint8_t  cmd[4];
    memset(cmd, 0, sizeof(cmd));
    nm25q_write_enable();

    cmd[0] = NM25Q_PageProgram;
    cmd[1] = (os_uint8_t)(addr >> 16);
    cmd[2] = (os_uint8_t)(addr >> 8);
    cmd[3] = (os_uint8_t)(addr);

    os_spi_send_then_send(nm25q_info.spi_device, cmd, 4, buff, size);
    nm25q_wait_busy();

    return OS_EOK;
}

static int nm25q_write(os_uint8_t *buff, os_uint32_t addr, os_uint32_t size)
{
    int ret = OS_EOK;

    os_uint32_t size_remain = size;
    os_uint32_t page_remain;

    page_remain = nm25q_info.chip_data.page_size - addr % nm25q_info.chip_data.page_size;

    if(size_remain < page_remain)
    {
        page_remain = size_remain;
    }

    while(1)
    {
        nm25q_write_page(buff, addr, page_remain);

        if(size_remain == page_remain)
        {
            break;
        }
        else
        {
            buff += page_remain;
            addr += page_remain;
            size_remain -= page_remain;
            if(size_remain > nm25q_info.chip_data.page_size)
            {
                page_remain = nm25q_info.chip_data.page_size;
            }
            else
            {
                page_remain = size_remain;
            }
        }
    }
    return ret;
}

static int nm25q_erase(os_uint32_t addr)
{
    os_uint8_t  cmd[4];
    memset(cmd, 0, sizeof(cmd));
    addr *= nm25q_info.chip_data.erase_size;

    nm25q_write_enable();
    nm25q_wait_busy();

    cmd[0] = NM25Q_SectorErase;
    cmd[1] = (os_uint8_t)(addr >> 16);
    cmd[2] = (os_uint8_t)(addr >> 8);
    cmd[3] = (os_uint8_t)(addr);

    os_spi_send(nm25q_info.spi_device, cmd, 4);
    nm25q_wait_busy();
    return OS_EOK;
}

static int fal_nm25q_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    os_uint32_t offset = page_addr * flash->page_size;
    os_uint32_t size   = page_nr * flash->page_size;
    return nm25q_read(buff, offset, size);
}

static int fal_nm25q_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    os_uint32_t offset = page_addr * flash->page_size;
    os_uint32_t size   = page_nr * flash->page_size;

    return nm25q_write((os_uint8_t *)buff, offset, size);
}

static int fal_nm25q_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int i, ret;

    os_uint32_t block_start = page_addr / flash->block_size;
    os_uint32_t block_cnt   = page_nr / flash->block_size;
    for (i = 0; i < block_cnt; i++)
    {
        ret = nm25q_erase(block_start);
        block_start++;
        if (ret != OS_EOK)
            return OS_EIO;
    }

    return ret;
}

static int nm25q_flash_init(void)
{
    if(nm25q_attch(NM25Q_NAME) != OS_EOK)
    {
        os_kprintf("nm25q attch failed.\r\n");

        return OS_ERROR;
    }

    os_kprintf("nm25q attch success.ID:%02X,Capacity:%dBytes\r\n", nm25q_info.chip_data.id, nm25q_info.chip_data.capacity);

    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", NM25Q_FAL_NAME);
        return -1;
    }

    memcpy(fal_flash->name,
           NM25Q_FAL_NAME,
           min(FAL_DEV_NAME_MAX - 1, strlen(NM25Q_FAL_NAME)));

    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(NM25Q_FAL_NAME))] = 0;

    fal_flash->capacity   = nm25q_info.chip_data.capacity;
    fal_flash->block_size = nm25q_info.chip_data.erase_size;
    fal_flash->page_size  = 1;

    fal_flash->ops.read_page   = fal_nm25q_read_page;
    fal_flash->ops.write_page  = fal_nm25q_write_page;
    fal_flash->ops.erase_block = fal_nm25q_erase_block;

    return fal_flash_register(fal_flash);
}

OS_DEVICE_INIT(nm25q_flash_init, OS_INIT_SUBLEVEL_MIDDLE);
