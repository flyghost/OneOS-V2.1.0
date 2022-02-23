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
 * @file        drv_nand.c
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>

typedef struct stm32_hardecc
{
    os_uint8_t bit_length;
    os_uint8_t byte_length;
    os_uint32_t addr_offset;
}stm32_hardecc_t;

struct stm32_nand
{
    os_nand_device_t    nand;

    NAND_HandleTypeDef *hnand;
    
    stm32_hardecc_t    hardecc;
    
    os_list_node_t      list;
};

static os_list_node_t stm32_nand_list = OS_LIST_INIT(stm32_nand_list);

#define STM32_NAND_ADDR_DEFINE()\
    os_nand_addr_t addr;\
    nand_page2addr(nand, page_addr, &addr);\
    NAND_AddressTypeDef stm32_nand_addr;\
    stm32_nand_addr.Plane = addr.plane;\
    stm32_nand_addr.Block = addr.block;\
    stm32_nand_addr.Page  = addr.page;

static os_err_t stm32_nand_read_page(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;

    STM32_NAND_ADDR_DEFINE();
    
    HAL_NAND_Read_Page_8b(st_nand->hnand, &stm32_nand_addr, buff, page_nr);

    return 0;
}

static os_err_t stm32_nand_write_page(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;

    STM32_NAND_ADDR_DEFINE();
    
    HAL_NAND_Write_Page_8b(st_nand->hnand, &stm32_nand_addr, (os_uint8_t *)buff, page_nr);

    return 0;
}

static os_err_t stm32_nand_read_spare(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t spare_nr)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;

    STM32_NAND_ADDR_DEFINE();
    
    HAL_NAND_Read_SpareArea_8b(st_nand->hnand, &stm32_nand_addr, buff, spare_nr);

    return 0;
}

static os_err_t stm32_nand_write_spare(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t spare_nr)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;

    STM32_NAND_ADDR_DEFINE();
    
    HAL_NAND_Write_SpareArea_8b(st_nand->hnand, &stm32_nand_addr, (os_uint8_t *)buff, spare_nr);

    return 0;
}

static os_err_t stm32_nand_erase_block(os_nand_device_t *nand, os_uint32_t page_addr)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;

    STM32_NAND_ADDR_DEFINE();
    
    HAL_NAND_Erase_Block(st_nand->hnand, &stm32_nand_addr);

    return 0;
}

#if 0 
static os_err_t stm32_nand_enable_ecc(os_nand_device_t *nand)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;
    
    HAL_NAND_ECC_Enable(st_nand->hnand);
    
    return 0;
}

static os_err_t stm32_nand_disable_ecc(os_nand_device_t *nand)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;
    
    HAL_NAND_ECC_Disable(st_nand->hnand);
    
    return 0;
}

static os_err_t stm32_nand_get_ecc(os_nand_device_t *nand, os_uint32_t *ecc_value)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;
    
    HAL_NAND_GetECC(st_nand->hnand, ecc_value, HAL_MAX_DELAY);

    return 0;
}

static os_err_t stm32_nand_config_hardecc(os_nand_device_t *nand)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;
    
    st_nand->hardecc.byte_length = 0;
    nand->cfg.info.hardecc_info.length = st_nand->hardecc.byte_length;
    st_nand->hardecc.addr_offset = nand->cfg.info.hardecc_info.addr_offset;
    
    for (int i = 0; i < 32;i++)
    {
        if (nand->cfg.info.page_size >> i)
        {
            st_nand->hardecc.bit_length = 6 + i * 2;
        }
    }
    
    return 0;
}
#endif

static os_uint32_t stm32_nand_get_status(os_nand_device_t *nand)
{
    struct stm32_nand *st_nand = (struct stm32_nand *)nand;
    
    HAL_NAND_Read_Status(st_nand->hnand);
    
    return 0;
}

static const struct os_nand_ops stm32_nand_ops =
{
    .read_page      = stm32_nand_read_page,
    .write_page     = stm32_nand_write_page,
    .read_spare     = stm32_nand_read_spare,
    .write_spare    = stm32_nand_write_spare,
    .erase_block    = stm32_nand_erase_block,
    .config_hardecc = OS_NULL,
    .get_status     = stm32_nand_get_status,
};

static int stm32_nand_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct stm32_nand *st_nand = os_calloc(1, sizeof(struct stm32_nand));

    OS_ASSERT(st_nand);

    st_nand->hnand = (NAND_HandleTypeDef *)dev->info;

    st_nand->nand.ops = &stm32_nand_ops;

    HAL_NAND_Reset(st_nand->hnand);

    NAND_IDTypeDef id;
    
    HAL_NAND_Read_ID(st_nand->hnand, &id);

    st_nand->nand.cfg.info.id = (id.Maker_Id << 24) | (id.Device_Id << 16) | (id.Third_Id << 8) | id.Fourth_Id;

    os_kprintf("nand id: 0x%08x\r\n", st_nand->nand.cfg.info.id);
    
    result = os_nand_device_register(&st_nand->nand, dev->name);
    
    OS_ASSERT(result == OS_EOK);

    st_nand->hnand->Config.PageSize           = st_nand->nand.cfg.info.page_size;
    st_nand->hnand->Config.SpareAreaSize      = st_nand->nand.cfg.info.spare_size;
    st_nand->hnand->Config.BlockSize          = st_nand->nand.cfg.info.block_size;
    st_nand->hnand->Config.BlockNbr           = st_nand->nand.cfg.info.plane_size * st_nand->nand.cfg.info.plane_nr;
    st_nand->hnand->Config.PlaneNbr           = st_nand->nand.cfg.info.plane_nr;
    st_nand->hnand->Config.PlaneSize          = st_nand->nand.cfg.info.plane_size;
    st_nand->hnand->Config.ExtraCommandEnable = ENABLE;

    level = os_irq_lock();
    os_list_add_tail(&stm32_nand_list, &st_nand->list);
    os_irq_unlock(level);
    
    return result;
}

OS_DRIVER_INFO stm32_nand_driver = {
    .name   = "NAND_HandleTypeDef",
    .probe  = stm32_nand_probe,
};

OS_DRIVER_DEFINE(stm32_nand_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);

