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
 * @file        nand.h
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-07-22    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _OS_NAND_H_
#define _OS_NAND_H_

#include <os_task.h>
#include <device.h>
#include <driver.h>

typedef struct os_nand_device os_nand_device_t;

typedef struct
{
    os_uint32_t page;       /*!< NAND memory Page address  */
    os_uint32_t plane;      /*!< NAND memory Zone address  */
    os_uint32_t block;      /*!< NAND memory Block address */
}os_nand_addr_t;

typedef struct
{
    os_uint8_t length;
    os_uint16_t data;
    os_uint32_t addr_offset;
    os_uint32_t page_offset;
}nand_device_badflag_info_t;

typedef struct
{
    os_uint8_t length;
    os_uint32_t addr_offset;
}nand_device_hardecc_info_t;

struct nand_device_info {
    const char *name;
    os_uint32_t id;
    os_uint32_t page_size;      /* NAND memory page (without spare area) size measured in bytes */
    os_uint32_t spare_size;     /* NAND memory spare area size measured in bytes */
    os_uint32_t block_size;     /* NAND memory block size measured in number of pages */
    os_uint32_t plane_size;     /* NAND memory zone size measured in number of blocks */
    os_uint32_t plane_nr;       /* NAND memory number of planes */
    os_uint32_t oob_len;       /* NAND memory number of planes */
    nand_device_badflag_info_t badflag_info;
    nand_device_hardecc_info_t hardecc_info;
};

typedef struct os_nand_config {
    struct nand_device_info info;
    
    os_uint32_t plane_mask;
    os_uint32_t plane_shift;
    os_uint32_t block_mask;
    os_uint32_t block_shift;
    os_uint32_t page_mask;
    os_uint32_t page_shift;

    os_uint32_t capacity;
}os_nand_config_t;

struct os_nand_ops
{
    os_err_t (*read_page)(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr);
    os_err_t (*write_page)(os_nand_device_t *nand, os_uint32_t page_addr,  const os_uint8_t *buff, os_uint32_t page_nr);
    os_err_t (*read_spare)(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t spare_nr);
    os_err_t (*write_spare)(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t spare_nr);
    os_err_t (*erase_block)(os_nand_device_t *nand, os_uint32_t page_addr);
    os_err_t (*config_hardecc)(os_nand_device_t *nand);
    os_uint32_t (*get_status)(os_nand_device_t *nand);
};

struct os_nand_device
{
    struct os_device parent;
    
    struct os_nand_config cfg;
    
    const struct os_nand_ops *ops;
    
    os_uint8_t hardecc_flag;
    
    os_uint8_t *page_buff;
    os_uint8_t *spare_buff;
};

os_err_t os_nand_device_register(os_nand_device_t *nand, const char *name);
os_err_t os_nand_read_page(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff);
os_err_t os_nand_write_page(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff);
os_err_t os_nand_read_spare(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff);
os_err_t os_nand_write_spare(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff);
os_err_t os_nand_erase_block(os_nand_device_t *nand, os_uint32_t block_no);

os_uint32_t os_nand_get_status(os_nand_device_t *nand);

void nand_page2addr(os_nand_device_t *nand, os_uint32_t page_addr, os_nand_addr_t *nand_addr);
os_uint32_t nand_addr2page(os_nand_device_t *nand, os_nand_addr_t *nand_addr);

os_err_t os_nand_badflag_check(os_nand_device_t *nand, os_uint32_t block_no);
os_err_t os_nand_badflag_mark(os_nand_device_t *nand, os_uint32_t block_no);
os_err_t os_nand_read(os_nand_device_t *nand, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t buff_len, os_uint8_t *oob, os_uint32_t oob_len); 
os_err_t os_nand_write(os_nand_device_t *nand, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t buff_len, const os_uint8_t *oob, os_uint32_t oob_len);





const struct nand_device_info *get_nand_info_by_id(os_uint32_t id);
const struct nand_device_info *get_nand_info_by_name(const char *name);

#endif /* _OS_NAND_H_ */

