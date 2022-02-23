/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#include "oc_config.h"
#ifdef OC_STORAGE
#include "port/oc_storage.h"
#include "port/oc_log.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <os_util.h>
#include "oc_buffer_settings.h"

#ifdef PKG_USING_EASYFLASH
#include <easyflash.h>

int oc_storage_init(void)
{
    int ret = easyflash_init();
    if (ret)
    {
        os_kprintf("easyflash_init error %d.\n", ret);
        return -1;
    }
    ret = ef_env_set_default();
    if (ret)
    {
        os_kprintf("ef_env_set_default error %d.\n", ret);
        return -1;
    }
    return 0;
}

#define STORE_PATH_SIZE 64

#if (PKG_EASYFLASH_ERASE_GRAN == 4096)
#define MAX_SAVE_LEN    (PKG_EASYFLASH_ERASE_GRAN - 58) /* SECTOR_SIZE - ENV_HDR_DATA_SIZE - EF_ENV_NAME_MAX */
#define MAX_USE_SECTOR_NUM ((oc_get_max_app_data_size() + MAX_SAVE_LEN - 1) / MAX_SAVE_LEN)

int
oc_storage_config(const char *store)
{
  easyflash_init();
  return 0;
}

long oc_storage_read(const char *item, uint8_t *value, size_t bufsize)
{
    if ((NULL == item) || (NULL == value))
    {
        return -1;
    }

#ifdef PKG_USING_EASYFLASH

    size_t get_size = 0;
    char temp_item[EF_ENV_NAME_MAX * 2];
    char *temp_value;
    size_t current_size = 0;
    int i = 0;

    temp_value = (char *)malloc(MAX_SAVE_LEN);
    for (i = 0; i < MAX_USE_SECTOR_NUM; i++) {
        memset(temp_item, 0, EF_ENV_NAME_MAX * 2);
        memset(temp_value, 0, MAX_SAVE_LEN);
        snprintf(temp_item, EF_ENV_NAME_MAX * 2, "%s%d", item, i);
        if ((get_size = ef_get_env_blob(temp_item, temp_value, MAX_SAVE_LEN, NULL)) > 0) {
            OC_DBG("%s %s len %d, ret len %d \n", __func__, temp_item, MAX_SAVE_LEN, get_size);
            if (bufsize >= get_size + current_size) {
                memcpy(value + current_size, temp_value, get_size);
                current_size += get_size;
            } else {
                OC_ERR("item %s len is too short, max len is %d, value len is %d\n", item, bufsize, get_size + current_size);
                current_size = -1;
            }
        } else {
            if (current_size == 0) {
                OC_ERR("%s %s = NULL\n", __func__, item);
                current_size = -1;
            }
            break;
        }
    }
    free(temp_value);
    return current_size;
#else
    return -1;
#endif

}

long oc_storage_write(const char *item, uint8_t *value, size_t size)
{

    if ((NULL == item) || (NULL == value))
    {
        return -1;
    }
    // if (strlen(value) != size) {
    //     OC_ERR("device_set_cfg err:%s len not %d!\n", value, size);
    //     return -1;
    // }
    OC_DBG("%s %s size %d\n", __func__, item, size);
#ifdef PKG_USING_EASYFLASH
    EfErrCode flag = 0;
    size_t remain_size = 0;
    size_t write_size = 0;
    int i = 0;
    char temp_item[EF_ENV_NAME_MAX * 2];
    int fragment_count = ((size + MAX_SAVE_LEN - 1) / MAX_SAVE_LEN);

    /* 删除之前可能存在的数据 */
    for (i = 0; i < MAX_USE_SECTOR_NUM; i++) {
        memset(temp_item, 0, EF_ENV_NAME_MAX * 2);
        snprintf(temp_item, EF_ENV_NAME_MAX * 2, "%s%d", item, i);
        if (EF_NO_ERR != ef_del_env(temp_item))
            break;
    }

    remain_size = size;
    for (i = 0; i < fragment_count; i++) {
        if (remain_size > MAX_SAVE_LEN) {
            remain_size -= MAX_SAVE_LEN;
            size = MAX_SAVE_LEN;
        } else {
            size = remain_size;
        }
        memset(temp_item, 0, EF_ENV_NAME_MAX * 2);
        snprintf(temp_item, EF_ENV_NAME_MAX * 2, "%s%d", item, i);
        flag = ef_set_env_blob(temp_item, value + write_size, size);
        if (flag != EF_NO_ERR)
        {
            OC_ERR("Set device information to easyflash failed! Set environment error!\n");
            return -1;
        }
        write_size += size;
    }
    ef_save_env();
#endif

    return 0;
}

#else /* (PKG_EASYFLASH_ERASE_GRAN == 4096) */

int
oc_storage_config(const char *store)
{
  easyflash_init();
  return 0;
}

long oc_storage_read(const char *item, uint8_t *value, size_t bufsize)
{
    if ((NULL == item) || (NULL == value))
    {
        return -1;
    }

#ifdef PKG_USING_EASYFLASH

    size_t get_size;

    if ((get_size = ef_get_env_blob(item, value, bufsize, NULL)) > 0) {
        OC_DBG("device_get_cfg %s len $d, ret len %d \n", item, bufsize, get_size);
        return get_size;        
    }
    else
    {
        OC_DBG("device_get_cfg %s = NULL\n", item);
        return -1;        
    }
#else
    return -1;
#endif

}

long oc_storage_write(const char *item, uint8_t *value, size_t size)
{

    if ((NULL == item) || (NULL == value))
    {
        return -1;
    }
    // if (strlen(value) != size) {
    //     OC_ERR("device_set_cfg err:%s len not %d!\n", value, size);
    //     return -1;
    // }
    OC_DBG("device_set_cfg %s = %s\n", item, value);
#ifdef PKG_USING_EASYFLASH
    EfErrCode flag = 0;

    flag = ef_set_env_blob(item, value, size);
    if (flag != EF_NO_ERR)
    {
        OC_ERR("Set device information to easyflash failed! Set environment error!\n");
        return -1;
    }
    ef_save_env();
#endif

    return 0;
}


#endif /* (PKG_EASYFLASH_ERASE_GRAN == 4096) */

#else /*PKG_USING_EASYFLASH*/
#include <os_task.h>
#include <os_assert.h>
#include <fal_part.h>
#include <sdram_port.h>
#include "oc_mem.h"

#define OCF_PAGE_SIZE   8192
#define OCF_ITEM_SIZE   50
#define OCF_DATA_SIZE   OCF_PAGE_SIZE - OCF_ITEM_SIZE - 4
#define OCF_PART_SIZE   128 * 1024
#define OCF_MAX_ITEM_COUNT  20
#define OCF_BACKEND_SIZE    OCF_MAX_ITEM_COUNT * OCF_PAGE_SIZE
typedef struct ocf_data
{
    char item[OCF_ITEM_SIZE];
    char len[10];
    char data[OCF_DATA_SIZE];
}__attribute__ ((__packed__)) OCF_DATA_S ;

static OCF_DATA_S *ocf_data_backend = NULL;
static fal_part_t *ocf_data_part = NULL;
static int ocf_item_num = 0;
static int flush_flash = 0;

static void storage_task(void *para)
{
    while(1)
    {
        if(flush_flash > 0)
        {
            fal_part_erase_all(ocf_data_part);
            fal_part_write(ocf_data_part, 0, (uint8_t*)ocf_data_backend, OCF_BACKEND_SIZE);
            flush_flash = 0;
            OC_DBG("OCF flush flash success\n");
        }
        os_task_msleep(30000);
    }
}

int oc_storage_init(void)
{
    ocf_data_part = fal_part_find("ocf_data");
    OS_ASSERT(NULL != ocf_data_part);
    fal_part_erase_all(ocf_data_part);
    return 0;
}

int oc_storage_config(const char *store)
{
    ocf_data_part = fal_part_find("ocf_data");
    OS_ASSERT(NULL != ocf_data_part);
    ocf_data_backend = oc_calloc(OCF_MAX_ITEM_COUNT, OCF_PAGE_SIZE);
    OS_ASSERT(NULL != ocf_data_backend);
    memset(ocf_data_backend, 0, OCF_BACKEND_SIZE);
    fal_part_read(ocf_data_part, 0, (uint8_t*)ocf_data_backend, OCF_BACKEND_SIZE);

    //Read all ocf data from flash firstly
    for(int i = 0;i < OCF_MAX_ITEM_COUNT;i ++)
    {
        if(NULL != strstr(ocf_data_backend[i].item, "ocf_data"))
        {
            OC_DBG("OCF find item %s[%s bytes][%d].\n", ocf_data_backend[i].item, ocf_data_backend[i].len, i);
            ocf_item_num ++;
        }
    }

    OC_DBG("OCF find %d ocf items in flash.\n", ocf_item_num);

    os_task_t *task;
    task = os_task_create("ocf_storage", storage_task, NULL, 2048, 10);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

long oc_storage_read(const char *item, uint8_t *value, size_t bufsize)
{
    OS_ASSERT(NULL != ocf_data_part);
    OS_ASSERT(NULL != item);
    OS_ASSERT(NULL != value);

    int i = 0;
    int len = 0;
    char name[OCF_ITEM_SIZE];
    memset(name, 0, sizeof(name));
    snprintf(name, sizeof(name), "%s%s", "ocf_data_", item);

    for(i = 0;i < OCF_MAX_ITEM_COUNT;i ++)
    {
        if(!strncmp(ocf_data_backend[i].item, name, strlen(name)))
        {
            len = (bufsize >= atoi(ocf_data_backend[i].len))?atoi(ocf_data_backend[i].len):bufsize;
            memcpy(value, ocf_data_backend[i].data, len);
            OC_DBG("OCF read %s[%s bytes][%d] from flash.\n", ocf_data_backend[i].item, ocf_data_backend[i].len, i);
            break;
        }
    }

    return (long)len;
}

long oc_storage_write(const char *item, uint8_t *value, size_t size)
{
    OS_ASSERT(NULL != ocf_data_part);
    OS_ASSERT(NULL != item);
    OS_ASSERT(NULL != value);
    OS_ASSERT(ocf_item_num < OCF_MAX_ITEM_COUNT);
    OS_ASSERT(size <= OCF_DATA_SIZE);

    int i = 0;
    int len = 0;
    char name[OCF_ITEM_SIZE];
    memset(name, 0, sizeof(name));
    snprintf(name, sizeof(name), "%s%s", "ocf_data_", item);
    int val_len = (size <= sizeof(ocf_data_backend[i].data))?size:sizeof(ocf_data_backend[i].data);

    for(i = 0;i < OCF_MAX_ITEM_COUNT;i ++)
    {
        if(!strncmp(ocf_data_backend[i].item, name, strlen(name)))
        {
            memset(ocf_data_backend[i].data, 0, sizeof(ocf_data_backend[i].data));
            memcpy(ocf_data_backend[i].data, value, val_len);
            memset(ocf_data_backend[i].len, 0, sizeof(ocf_data_backend[i].len));
            snprintf(ocf_data_backend[i].len, sizeof(ocf_data_backend[i].len), "%d", val_len);
            OC_DBG("OCF modify %s[%s bytes][%d] to flash.\n", ocf_data_backend[i].item, ocf_data_backend[i].len, i);
            len = val_len;
            flush_flash ++;
            break;
        }
    }

    if(0 == len)
    {
        for(i = 0;i < OCF_MAX_ITEM_COUNT;i ++)
        {
            if(NULL == strstr(ocf_data_backend[i].item, "ocf_data"))
            {
                memset(ocf_data_backend + i, 0, OCF_PAGE_SIZE);
                strncpy(ocf_data_backend[i].item, name, sizeof(ocf_data_backend[i].item));
                memcpy(ocf_data_backend[i].data, value, size);
                snprintf(ocf_data_backend[i].len, sizeof(ocf_data_backend[i].len), "%d", size);
                len = val_len;
                ocf_item_num ++;
                flush_flash ++;
                OC_DBG("OCF write %s[%s bytes][%d] to flash.\n", ocf_data_backend[i].item, ocf_data_backend[i].len, i);
                break;
            }
        }
    }

    // if(len > 0)
    // {
    //     fal_part_erase_all(ocf_data_part);
    //     fal_part_write(ocf_data_part, 0, (uint8_t*)ocf_data_backend, OCF_BACKEND_SIZE);
    // }

    return len;
}

#endif/*PKG_USING_EASYFLASH*/

#endif /* OC_STORAGE */
