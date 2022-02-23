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
 * @file        yaffs_flashif.c
 *
 * @brief       This file is adapter for nandflash and yaffs2.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include "nand.h"

#include "yportenv.h"
#include "yaffs_trace.h"
#include "yaffs_guts.h"
#include "yaffs_ecc.h"

#define YAFFS_TAG       "YAFFS"

#define SOFTECC_ALG_DATA_LEN    256
#define SOFTECC_ALG_ECC_LEN     3

#define YAFFS_DEV_TO_NAND(yfs_dev)  ((struct nand_context *)(yfs_dev->driver_context))->nand_dev
#define YAFFS_DEV_TO_BUF(yfs_dev)   ((struct nand_context *)(yfs_dev->driver_context))->buffer
#define YAFFS_GET_PAGE_SIZE(yfs_dev)           (yfs_dev->param.total_bytes_per_chunk)
#define NAND_HARD_ECC_ENABLE_FLAG(nand_dev)    (nand_dev->hardecc_flag)

extern void yaffsfs_LockDestroy(void);
extern void yaffs_remove_device(struct yaffs_dev *dev);

struct nand_context {
    os_nand_device_t *nand_dev;
    os_uint8_t *buffer;
};

static int yaffs_nand_write_chunk(struct yaffs_dev *yfs_dev, int nand_chunk,
                   const u8 *data, int data_len,
                   const u8 *oob, int oob_len)
{
    os_nand_device_t *nand_dev;
    os_uint8_t *buffer;
    u8 *e;
    os_uint32_t i;
    os_uint32_t ecc_len;

    if ((!yfs_dev) || (!yfs_dev->driver_context) || (!data) || (!oob))
    {
        LOG_E(YAFFS_TAG, "yaffs_nand_write_chunk: invalid param");
        return YAFFS_FAIL;
    }
    nand_dev = YAFFS_DEV_TO_NAND(yfs_dev);
    if (data_len > YAFFS_GET_PAGE_SIZE(yfs_dev))
    {
        LOG_W(YAFFS_TAG, "yaffs_nand_write_chunk data_len:%d", data_len);
        return YAFFS_FAIL;
    }

    if (NAND_HARD_ECC_ENABLE_FLAG(nand_dev))
    {
        if (os_nand_write(nand_dev, nand_chunk, data, data_len, oob, oob_len) != 0)
        {
            return YAFFS_FAIL;
        }
    }
    else
    {
        buffer = YAFFS_DEV_TO_BUF(yfs_dev);
        ecc_len = (YAFFS_GET_PAGE_SIZE(yfs_dev) / SOFTECC_ALG_DATA_LEN) * SOFTECC_ALG_ECC_LEN;
        for (i = 0, e = buffer; i < YAFFS_GET_PAGE_SIZE(yfs_dev); i += SOFTECC_ALG_DATA_LEN, e += SOFTECC_ALG_ECC_LEN)
        {
            yaffs_ecc_calc(data + i, e);
        }
        memcpy(e, oob, oob_len);

        if (os_nand_write(nand_dev, nand_chunk, data, data_len, buffer, ecc_len + oob_len) != 0)
        {
            return YAFFS_FAIL;
        }
    }
    return YAFFS_OK;
}

static int yaffs_nand_read_chunk(struct yaffs_dev *yfs_dev, int nand_chunk,
                   u8 *data, int data_len,
                   u8 *oob, int oob_len,
                   enum yaffs_ecc_result *ecc_result_out)
{
    os_nand_device_t *nand_dev;
    os_uint8_t *buffer;
    int ret;
    enum yaffs_ecc_result ecc_result;
    os_uint32_t i;
    u8 *e;
    u8 read_ecc[3];
    os_uint32_t ecc_len;

    if ((!yfs_dev) || (!yfs_dev->driver_context) || ((!data) && (!oob)))
    {
        LOG_E(YAFFS_TAG, "yaffs_nand_read_chunk: invalid param");
        return YAFFS_FAIL;
    }

    nand_dev = YAFFS_DEV_TO_NAND(yfs_dev);
    if (data_len > YAFFS_GET_PAGE_SIZE(yfs_dev))
    {
        LOG_W(YAFFS_TAG, "yaffs_nand_read_chunk data_len:%d", data_len);
        return YAFFS_FAIL;
    }

    ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
    if (NAND_HARD_ECC_ENABLE_FLAG(nand_dev))
    {
        if (os_nand_read(nand_dev, nand_chunk, data, data_len, oob, oob_len) != 0)
        {
            ecc_result = YAFFS_ECC_RESULT_UNKNOWN;
            goto err;
        }
    }
    else
    {
        buffer = YAFFS_DEV_TO_BUF(yfs_dev);
        ecc_len = (YAFFS_GET_PAGE_SIZE(yfs_dev) / SOFTECC_ALG_DATA_LEN) * SOFTECC_ALG_ECC_LEN;
        if (os_nand_read(nand_dev, nand_chunk, data, data_len, buffer, ecc_len + oob_len) != 0)
        {
            ecc_result = YAFFS_ECC_RESULT_UNKNOWN;
            goto err;
        }

        if (oob)
        {
            memcpy(oob, buffer + ecc_len, oob_len);
        }

        if (data) 
        {
            for (i = 0, e = buffer; i < YAFFS_GET_PAGE_SIZE(yfs_dev); i += SOFTECC_ALG_DATA_LEN, e += SOFTECC_ALG_ECC_LEN)
            {
                yaffs_ecc_calc(data + i, read_ecc);
                ret = yaffs_ecc_correct(data + i, e, read_ecc);

                if (ret == 0)
                {
                    ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
                }
                else if (ret > 0)
                {
                    ecc_result = YAFFS_ECC_RESULT_FIXED;
                }
                else
                {
                    ecc_result = YAFFS_ECC_RESULT_UNFIXED;
                    goto err;
                }
            }
        }
    }

    if (ecc_result_out)
    {
        *ecc_result_out = ecc_result;
    }
    return YAFFS_OK;

err:
    if (ecc_result_out)
    {
        *ecc_result_out = ecc_result;
    }
    return YAFFS_FAIL;
}

static int yaffs_nand_erase_chunk(struct yaffs_dev *yfs_dev, int block_no)
{
    os_nand_device_t *nand_dev;

    if ((!yfs_dev) || (!yfs_dev->driver_context))
    {
        LOG_E(YAFFS_TAG, "yaffs_nand_erase_chunk: invalid param");
        return YAFFS_FAIL;
    }

    nand_dev = YAFFS_DEV_TO_NAND(yfs_dev);
    if(os_nand_erase_block(nand_dev, block_no) == 0)
    {
        return YAFFS_OK;
    }
    else
    {
        return YAFFS_FAIL;
    }
}

static int yaffs_nand_mark_bad(struct yaffs_dev *yfs_dev, int block_no)
{
    os_nand_device_t *nand_dev;

    if ((!yfs_dev) || (!yfs_dev->driver_context))
    {
        LOG_E(YAFFS_TAG, "yaffs_nand_mark_bad: invalid param");
        return YAFFS_FAIL;
    }

    nand_dev = YAFFS_DEV_TO_NAND(yfs_dev);
    if(os_nand_badflag_mark(nand_dev, block_no) == OS_EOK)
    {
        return YAFFS_OK;
    }
    else
    {
        return YAFFS_FAIL;
    }
}

static int yaffs_nand_check_bad(struct yaffs_dev *yfs_dev, int block_no)
{
    os_nand_device_t *nand_dev;

    if ((!yfs_dev) || (!yfs_dev->driver_context))
    {
        LOG_E(YAFFS_TAG, "yaffs_nand_mark_bad: invalid param");
        return YAFFS_FAIL;
    }

    nand_dev = YAFFS_DEV_TO_NAND(yfs_dev);
    if(os_nand_badflag_check(nand_dev, block_no) == OS_EOK)
    {
        return YAFFS_OK;
    }
    else
    {
        return YAFFS_FAIL;
    }
}

static int yaffs_nand_inital(struct yaffs_dev *yfs_dev)
{
    (void)yfs_dev;
    return YAFFS_OK;
}

static int yaffs_nand_deinital(struct yaffs_dev *yfs_dev)
{
    (void) yfs_dev;
    return YAFFS_OK;
}

static int yaffs_nand_install_drv(struct yaffs_dev *yfs_dev, os_nand_device_t *nand_dev)
{
    struct yaffs_driver *drv = &yfs_dev->drv;
    os_uint8_t *buffer;
    struct nand_context *ctxt;

    ctxt = (struct nand_context *)os_malloc(sizeof(struct nand_context));
    if(!ctxt)
    {
        return YAFFS_FAIL;
    }
    buffer = (os_uint8_t *)os_malloc(YAFFS_GET_PAGE_SIZE(yfs_dev));
    if(!buffer)
    {
        os_free(ctxt);
        return YAFFS_FAIL;
    }

    drv->drv_write_chunk_fn = yaffs_nand_write_chunk;
    drv->drv_read_chunk_fn = yaffs_nand_read_chunk;
    drv->drv_erase_fn = yaffs_nand_erase_chunk;
    drv->drv_mark_bad_fn = yaffs_nand_mark_bad;
    drv->drv_check_bad_fn = yaffs_nand_check_bad;
    drv->drv_initialise_fn = yaffs_nand_inital;
    drv->drv_deinitialise_fn = yaffs_nand_deinital;

    ctxt->nand_dev = nand_dev;
    ctxt->buffer = buffer;
    yfs_dev->driver_context = (void *) ctxt;

    return YAFFS_OK;
}

/**
 ***********************************************************************************************************************
 * @brief           Install nand to yaffs.
 * 
 * @param[in]       dev_name        The nand device name. The device name should has already been registered.
 *
 * @return          The install result.
 * @retval          Not OS_NULL     Install successful, return the yaffs_dev.
 * @retval          OS_NULL         Install fail. Not find the device name or No enough memory.
 ***********************************************************************************************************************
 */
void *yaffs_nand_install(const char *dev_name)
{
    struct yaffs_dev *yfs_dev;
    struct yaffs_param *param;
    os_device_t *os_dev;
    os_nand_device_t *nand_dev;

    os_dev = os_device_find(dev_name);
    if (!os_dev)
    {
        LOG_E(YAFFS_TAG, "not find device£º%s", dev_name);
        return OS_NULL;
    }
    nand_dev = os_container_of(os_dev, os_nand_device_t, parent);

    yfs_dev = os_malloc(sizeof(struct yaffs_dev));
    
    if (!yfs_dev)
    {
        return OS_NULL;
    }
    memset(yfs_dev, 0, sizeof(*yfs_dev));

    param = &yfs_dev->param;
    param->name = os_malloc(strlen(dev_name) + 1);
    
    if (!param->name)
    {
        os_free(yfs_dev);
        return OS_NULL;
    }
    memset((void *)param->name, 0, strlen(dev_name) + 1);
    memcpy((void *)param->name, dev_name, strlen(dev_name));

    if (nand_dev->cfg.info.page_size % 256 != 0)
    {
        LOG_W(YAFFS_TAG, "The nand page size may be invalid:%d", nand_dev->cfg.info.page_size);
    }
    param->total_bytes_per_chunk = nand_dev->cfg.info.page_size;
    param->spare_bytes_per_chunk = nand_dev->cfg.info.spare_size;
    param->chunks_per_block = nand_dev->cfg.info.block_size;
    param->n_reserved_blocks = 5;
    param->start_block = 0;
    param->end_block = (nand_dev->cfg.info.plane_size * nand_dev->cfg.info.plane_nr) - 1;
    param->is_yaffs2 = (nand_dev->cfg.info.page_size > 512) ? 1 : 0;
    param->use_nand_ecc = 1;
    param->n_caches = 10;
    /* param->refresh_period = 10; */
    /* param->hide_lost_n_found = 1; */
    /* param->stored_endian = 1; */
    
    if (yaffs_nand_install_drv(yfs_dev, nand_dev) != YAFFS_OK)
    {
        os_free((void *)param->name);
        os_free(yfs_dev);
        return OS_NULL;
    }
    
    /* The yaffs device has been configured, install it into yaffs */
    yaffs_add_device(yfs_dev);
    yaffsfs_OSInitialisation();
    
    return (void *)yfs_dev;
}

void yaffs_nand_uninstall(struct yaffs_dev *yfs_dev)
{
    struct nand_context *ctxt;
    ctxt = yfs_dev->driver_context;

    os_free((char *)yfs_dev->param.name);
    os_free(ctxt->buffer);
    os_free(yfs_dev->driver_context);
    os_free(yfs_dev);
    yaffsfs_LockDestroy();

    yaffs_remove_device(yfs_dev);
}


#ifdef OS_USING_SHELL
#include <shell.h>
#include "yaffs_verify.h"
#include "yaffs_yaffs2.h"

void yaffs_info_show(os_int32_t argc, char **argv)
{
    unsigned  yaffs_trace_mask_old;
    Y_LOFF_T freespace;
    Y_LOFF_T totalpace;
    struct yaffs_dev *yfs_dev;
    char *name;

    if (argc < 2)
    {
        LOG_W(YAFFS_TAG, "Usage: show_yaffs [dev_name]");
        return;
    }

    yfs_dev = yaffs_getdev(argv[1]);
    if (!yfs_dev)
    {
        LOG_E(YAFFS_TAG, "not find the device:%s", argv[1]);
        return;
    }
    if (!yfs_dev->is_mounted)
    {
        LOG_E(YAFFS_TAG, "the device:%s has not mounted", argv[1]);
        return;
    }
    name = argv[1];
    LOG_W(YAFFS_TAG, "name:%s dev:%p", name, yfs_dev);
    freespace = yaffs_freespace(name);
    totalpace = yaffs_totalspace(name);
    LOG_W(YAFFS_TAG, "totalspace:%lldBytes(%dMB)", (os_uint64_t)totalpace, (os_uint32_t)(totalpace>>20));
    LOG_W(YAFFS_TAG, "freespace:%lldBytes(%dMB)", (os_uint64_t)freespace, (os_uint32_t)freespace >> 20);

    LOG_W(YAFFS_TAG, "NAME_MAX:%d", NAME_MAX);
    LOG_W(YAFFS_TAG, "YAFFSFS_N_HANDLES:%d", YAFFSFS_N_HANDLES);
    LOG_W(YAFFS_TAG, "YAFFS_MAX_FILE_SIZE:%lldMB", (os_uint64_t)YAFFS_MAX_FILE_SIZE >> 20);

    yaffsfs_Lock();
    yaffs_trace_mask_old = yaffs_trace_mask;
    yaffs_trace_mask = YAFFS_TRACE_VERIFY;
    yaffs_verify_blocks(yfs_dev);
    yaffs_trace_mask = yaffs_trace_mask_old;
    LOG_W(YAFFS_TAG, "total_bytes_per_chunk:%d", yfs_dev->param.total_bytes_per_chunk);
    LOG_W(YAFFS_TAG, "chunks_per_block:%d", yfs_dev->param.chunks_per_block);
    LOG_W(YAFFS_TAG, "n_caches: %d", yfs_dev->param.n_caches);
    LOG_W(YAFFS_TAG, "always_check_erased:%d", yfs_dev->param.always_check_erased);
    LOG_W(YAFFS_TAG, "n_reserved_blocks:%d", yfs_dev->param.n_reserved_blocks);
    LOG_W(YAFFS_TAG, "n_free_chunks:%d", yfs_dev->n_free_chunks);
    LOG_W(YAFFS_TAG, "freespace_chunks:%d", freespace/yfs_dev->param.total_bytes_per_chunk);
    LOG_W(YAFFS_TAG, "chunks_summary:%d", yfs_dev->param.chunks_per_block - yfs_dev->chunks_per_summary);
    LOG_W(YAFFS_TAG, "checkpt_blocks:%d", yaffs_calc_checkpt_blocks_required(yfs_dev));
    yaffsfs_Unlock();
}
SH_CMD_EXPORT(show_yaffs, yaffs_info_show, "yaffs_info_show");
#endif

