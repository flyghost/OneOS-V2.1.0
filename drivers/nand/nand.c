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
 * @file        nand.c
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-07-22    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <arch_interrupt.h>
#include <arch_misc.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <drv_cfg.h>
#include <drv_log.h>

#ifdef OS_USING_FAL

#include <fal/fal.h>

static int fal_nand_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{    
    os_nand_device_t *nand = flash->priv;
    
    return os_nand_read_page(nand, page_addr, buff);
}

static int fal_nand_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    os_nand_device_t *nand = flash->priv;
    
    return os_nand_write_page(nand, page_addr, buff);
}

static int fal_nand_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int i, ret;
    
    os_nand_device_t *nand = flash->priv;

    for (i = 0; i < page_nr; i += nand->cfg.info.block_size)
    {
        ret = os_nand_erase_block(nand, page_addr + i);
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

static int fal_nand_flash_register(os_nand_device_t *nand)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal nand mem leak %s.\r\n", device_name(&nand->parent));
        return -1;
    }

    memcpy(fal_flash->name,
           device_name(&nand->parent),
           min(FAL_DEV_NAME_MAX - 1, strlen(device_name(&nand->parent))));
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(device_name(&nand->parent)))] = 0;
    
    fal_flash->capacity = nand->cfg.capacity;
    fal_flash->block_size = nand->cfg.info.page_size * nand->cfg.info.block_size;
    fal_flash->page_size  = nand->cfg.info.page_size;
    
    fal_flash->ops.read_page   = fal_nand_read_page,
    fal_flash->ops.write_page  = fal_nand_write_page,
    fal_flash->ops.erase_block = fal_nand_erase_block,

    fal_flash->priv = nand;

    return fal_flash_register(fal_flash);
}

#endif

static os_err_t os_nand_cfg_prepare(struct os_nand_config *cfg)
{
    const struct nand_device_info *info;

    info = get_nand_info_by_id(cfg->info.id);

    if (info == OS_NULL)
    {
        return OS_ENOSYS;
    }

    cfg->info = *info;

    cfg->page_shift  = os_ffs(cfg->info.page_size) - 1;
    cfg->block_shift = cfg->page_shift + os_ffs(cfg->info.block_size) - 1;
    cfg->plane_shift = cfg->block_shift + os_ffs(cfg->info.plane_size) - 1;
    
    cfg->page_mask   = (cfg->info.block_size - 1) << cfg->page_shift;
    cfg->block_mask  = (cfg->info.plane_size - 1) << cfg->block_shift;    
    cfg->plane_mask  = (cfg->info.plane_nr - 1) << cfg->plane_shift;

    cfg->capacity = info->page_size * info->block_size * info->plane_size * info->plane_nr;

    return OS_EOK;
}

os_err_t os_nand_device_register(os_nand_device_t *nand, const char *name)
{
    os_err_t ret;

    OS_ASSERT(nand != OS_NULL);

    ret = os_nand_cfg_prepare(&nand->cfg);
    if (ret != OS_EOK)
    {
        os_kprintf("nand device config failed %s, %d\r\n", name, ret);
        return ret;
    }
    
    if (nand->ops->config_hardecc != OS_NULL)
    {
        nand->ops->config_hardecc(nand);
        nand->hardecc_flag = OS_TRUE;
    }
    else
    {
        nand->hardecc_flag = OS_FALSE;
    }
    
    nand->cfg.info.oob_len = nand->cfg.info.spare_size - nand->cfg.info.badflag_info.length - nand->cfg.info.hardecc_info.length;
    nand->page_buff = os_calloc(1, nand->cfg.info.page_size);
    OS_ASSERT(nand->page_buff);
    nand->spare_buff = os_calloc(1, nand->cfg.info.spare_size);
    OS_ASSERT(nand->spare_buff);
    
    nand->parent.ops  = OS_NULL;
    nand->parent.type = OS_DEVICE_TYPE_MTD;
    os_device_register(&nand->parent, name);

    struct nand_device_info *info = &nand->cfg.info;

    os_kprintf("nand device register success %s(%s):\r\n"
               "    nand  id   : %08X\r\n"
               "    page  size : %d\r\n"
               "    spare size : %d\r\n"
               "    block size : %d\r\n"
               "    plane size : %d\r\n"
               "    plane count: %d\r\n"
               "    total size : %d MB\r\n",
               name, info->name,
               info->id,
               info->page_size,
               info->spare_size,
               info->block_size,
               info->plane_size,
               info->plane_nr,
               nand->cfg.capacity / 0x100000);

#ifdef OS_USING_FAL
    return fal_nand_flash_register(nand);
#else
    return OS_EOK;
#endif
}

os_err_t os_nand_read_page(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff)
{
    return nand->ops->read_page(nand, page_addr, buff, 1);
}

os_err_t os_nand_write_page(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff)
{
    return nand->ops->write_page(nand, page_addr, buff, 1);
}

os_err_t os_nand_read_spare(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff)
{
    return nand->ops->read_spare(nand, page_addr, buff, 1);
}

os_err_t os_nand_write_spare(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff)
{
    return nand->ops->write_spare(nand, page_addr, buff, 1);
}

os_err_t os_nand_erase_block(os_nand_device_t *nand, os_uint32_t block_no)
{
    return nand->ops->erase_block(nand, block_no * nand->cfg.info.block_size);
}

os_uint32_t os_nand_get_status(os_nand_device_t *nand)
{
    return nand->ops->get_status(nand);
}

void nand_page2addr(os_nand_device_t *nand, os_uint32_t page_addr, os_nand_addr_t *nand_addr)
{
    os_uint32_t addr = page_addr << nand->cfg.page_shift;

    nand_addr->plane = (addr & nand->cfg.plane_mask) >> nand->cfg.plane_shift;
    nand_addr->block = (addr & nand->cfg.block_mask) >> nand->cfg.block_shift;
    nand_addr->page  = (addr & nand->cfg.page_mask) >> nand->cfg.page_shift;
}

os_uint32_t nand_addr2page(os_nand_device_t *nand, os_nand_addr_t *nand_addr)
{
    os_uint32_t page_addr;

    page_addr  = nand_addr->plane << nand->cfg.plane_shift;
    page_addr |= nand_addr->block << nand->cfg.block_shift;
    page_addr |= nand_addr->page << nand->cfg.page_shift;
        
    return page_addr;
}

os_err_t os_nand_badflag_check(os_nand_device_t *nand, os_uint32_t block_no)
{
    os_err_t ret;
    os_uint32_t badflag_buf = 0;
    os_uint32_t page_addr = block_no * nand->cfg.info.block_size + nand->cfg.info.badflag_info.page_offset;
    
    ret = nand->ops->read_spare(nand, page_addr, nand->spare_buff, 1);
    if (ret != OS_EOK)
    {
        os_kprintf("nand read spare(badflag_check) failed!\r\n");
        return OS_ERROR;
    }
        
    memcpy(&badflag_buf, nand->spare_buff + nand->cfg.info.badflag_info.addr_offset, nand->cfg.info.badflag_info.length);

    for (int i = 0;i < nand->cfg.info.badflag_info.length;i++)
    {
        if (((badflag_buf >> (i * 8)) & 0xFF) != 0xFF)
        {
            os_kprintf("nand block %d is bad!\r\n", block_no);
            return OS_ERROR;
        }
    }
    
    return OS_EOK;
}

os_err_t os_nand_badflag_mark(os_nand_device_t *nand, os_uint32_t block_no)
{
    os_err_t ret;
    os_uint32_t page_addr = block_no * nand->cfg.info.block_size + nand->cfg.info.badflag_info.page_offset;

    memset(nand->spare_buff, 0xFF, nand->cfg.info.spare_size);
    memcpy(nand->spare_buff + nand->cfg.info.badflag_info.addr_offset, &nand->cfg.info.badflag_info.data, nand->cfg.info.badflag_info.length);

    ret = nand->ops->write_spare(nand, page_addr, nand->spare_buff, 1);
    if (ret != OS_EOK)
    {
        os_kprintf("nand write spare(badflag_mark) failed!\r\n");
        return OS_ERROR;
    }
    return ret;
}

os_err_t os_nand_read(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t buff_len, os_uint8_t *oob, os_uint32_t oob_len)
{
    os_err_t ret;
    
    if ((buff_len > nand->cfg.info.page_size) || (oob_len > nand->cfg.info.oob_len))
    {
        os_kprintf("nandflash read size out of range!");
        return OS_ERROR;
    }

    if (buff != OS_NULL)
    {
        if (buff_len == nand->cfg.info.page_size)
        {
            ret = nand->ops->read_page(nand, page_addr, buff, 1);
        }
        else
        {
            ret = nand->ops->read_page(nand, page_addr, nand->page_buff, 1);
            memcpy(buff, nand->page_buff, buff_len);
        }
        if (ret != OS_EOK)
        {
            os_kprintf("nand read page failed!\r\n");
            return OS_ERROR;
        }
    }
    
    if (oob != OS_NULL)
    {
        ret = nand->ops->read_spare(nand, page_addr, nand->spare_buff, 1);
        if (ret != OS_EOK)
        {
            os_kprintf("nand read spare failed!\r\n");
            return OS_ERROR;
        }

        for (int i = 0;i < nand->cfg.info.spare_size;i++)
        {
            if ((i >= nand->cfg.info.badflag_info.addr_offset) && (i < nand->cfg.info.badflag_info.addr_offset + nand->cfg.info.badflag_info.length))
            {
            }
            else if ((i >= nand->cfg.info.hardecc_info.addr_offset) && (i < nand->cfg.info.hardecc_info.addr_offset + nand->cfg.info.hardecc_info.length))
            {  
            }
            else
            {
                *oob++ = *(nand->spare_buff + i);
            }
        }
    }

    return ret; 
}
os_err_t os_nand_write(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t buff_len, const os_uint8_t *oob, os_uint32_t oob_len)
{
    os_err_t ret;

    if ((buff_len > nand->cfg.info.page_size) || (oob_len > nand->cfg.info.oob_len))
    {
        os_kprintf("nandflash write size out of range");
        return OS_ERROR;
    }

    if (buff != OS_NULL)
    {
        if (buff_len == nand->cfg.info.page_size)
        {
            ret =nand->ops->write_page(nand, page_addr, buff, 1);
        }
        else
        {
            memset(nand->page_buff, 0xFF, nand->cfg.info.page_size);
            memcpy(nand->page_buff, buff, buff_len);
            ret =nand->ops->write_page(nand, page_addr, nand->page_buff, 1);
        }
        if (ret != OS_EOK)
        {
            os_kprintf("nand write page failed!\r\n");
            return OS_ERROR;
        }
    }
    
    if (oob != OS_NULL)
    {
        memset(nand->spare_buff, 0xFF, nand->cfg.info.spare_size);
        for (int i = 0;i < nand->cfg.info.spare_size;i++)
        {
            if ((i >= nand->cfg.info.badflag_info.addr_offset) && (i < nand->cfg.info.badflag_info.addr_offset + nand->cfg.info.badflag_info.length))
            {
                *(nand->spare_buff + i) = 0xFF;
            }
            else if ((i >= nand->cfg.info.hardecc_info.addr_offset) && (i < nand->cfg.info.hardecc_info.addr_offset + nand->cfg.info.hardecc_info.length))
            {
                *(nand->spare_buff + i) = 0xFF;
            }
            else
            {
                *(nand->spare_buff + i) = *oob++;
            }
        }

        ret = nand->ops->write_spare(nand, page_addr, nand->spare_buff, 1);
        if (ret != OS_EOK)
        {
            os_kprintf("nand write spare failed!\r\n");
            return OS_ERROR;
        }
    }

    return ret; 
}

