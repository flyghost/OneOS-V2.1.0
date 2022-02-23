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
 * @file        spi_flash_sfud.c
 *
 * @brief       This file provides functions for spi flash sfud.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <drv_cfg.h>
#include <os_memory.h>
#include <string.h>
#include <os_clock.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"

#ifdef OS_USING_SFUD

#ifdef OS_DEBUG_SFUD
#define DEBUG_TRACE         os_kprintf("[SFUD] "); os_kprintf
#else
#define DEBUG_TRACE(...)
#endif /* OS_DEBUG_SFUD */

#ifndef OS_SFUD_DEFAULT_SPI_CFG

#ifndef OS_SFUD_SPI_MAX_HZ
#define OS_SFUD_SPI_MAX_HZ 50000000
#endif

/* read the JEDEC SFDP command must run at 50 MHz or less */
#define OS_SFUD_DEFAULT_SPI_CFG                  \
{                                                \
    .mode = OS_SPI_MODE_0 | OS_SPI_MSB,          \
    .data_width = 8,                             \
    .max_hz = OS_SFUD_SPI_MAX_HZ,                \
}
#endif

#ifdef SFUD_USING_QSPI
#define OS_SFUD_DEFAULT_QSPI_CFG                 \
{                                                \
    OS_SFUD_DEFAULT_SPI_CFG,                     \
    .medium_size = 0x800000,                     \
    .ddr_mode = 0,                               \
    .qspi_dl_width = 4,                          \
}
#endif

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

static int mtd_sfud_read_page(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(mtd->priv);
    sfud_flash *sfud_dev = (sfud_flash *)(os_dev->user_data);

    OS_ASSERT(os_dev);
    OS_ASSERT(sfud_dev);

    os_off_t  phy_pos  = page_addr * os_dev->mtd_dev.geometry.page_size;
    os_size_t phy_size = page_nr * os_dev->mtd_dev.geometry.page_size;

    if (sfud_read(sfud_dev, phy_pos, phy_size, buff) != SFUD_SUCCESS)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

static int mtd_sfud_write_page(os_mtd_device_t *mtd, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(mtd->priv);
    sfud_flash *sfud_dev = (sfud_flash *)(os_dev->user_data);

    OS_ASSERT(os_dev);
    OS_ASSERT(sfud_dev);

    os_off_t  phy_pos  = page_addr * os_dev->mtd_dev.geometry.page_size;
    os_size_t phy_size = page_nr * os_dev->mtd_dev.geometry.page_size;

    if (sfud_erase_write(sfud_dev, phy_pos, phy_size, buff) != SFUD_SUCCESS)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

static int mtd_sfud_erase_block(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint32_t page_nr)
{
    os_uint32_t phy_start_addr;
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(mtd->priv);
    sfud_flash *sfud_dev = (sfud_flash *)(os_dev->user_data);
    os_size_t   phy_size;

    if (os_dev == OS_NULL || sfud_dev == OS_NULL)
    {
        return OS_ERROR;
    }

    phy_start_addr = page_addr * os_dev->mtd_dev.geometry.page_size;
    phy_size       = page_nr * os_dev->mtd_dev.geometry.page_size;

    if (sfud_erase(sfud_dev, phy_start_addr, phy_size) != SFUD_SUCCESS)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static sfud_err spi_write_read(const sfud_spi *spi, 
                               const uint8_t  *write_buf, 
                               size_t          write_size, 
                               uint8_t        *read_buf, 
                               size_t          read_size)
{
    sfud_err    result   = SFUD_SUCCESS;
    sfud_flash *sfud_dev = (sfud_flash *)(spi->user_data);
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(sfud_dev->user_data);

    OS_ASSERT(spi);
    OS_ASSERT(sfud_dev);
    OS_ASSERT(os_dev);
#ifdef SFUD_USING_QSPI
    struct os_qspi_device *qspi_dev = OS_NULL;
#endif
    if (write_size)
    {
        OS_ASSERT(write_buf);
    }
    if (read_size)
    {
        OS_ASSERT(read_buf);
    }
#ifdef SFUD_USING_QSPI
    if (os_dev->os_spi_device->bus->mode & OS_SPI_BUS_MODE_QSPI)
    {
        qspi_dev = (struct os_qspi_device *)(os_dev->os_spi_device);
        if (write_size && read_size)
        {
            if (os_qspi_send_then_recv(qspi_dev, write_buf, write_size, read_buf, read_size) <= 0)
            {
                result = SFUD_ERR_TIMEOUT;
            }
        }
        else if (write_size)
        {
            if (os_qspi_send(qspi_dev, write_buf, write_size) <= 0)
            {
                result = SFUD_ERR_TIMEOUT;
            }
        }
    }
    else
#endif
    {
        if (write_size && read_size)
        {
            if (os_spi_send_then_recv(os_dev->os_spi_device, write_buf, write_size, read_buf, read_size) != OS_EOK)
            {
                result = SFUD_ERR_TIMEOUT;
            }
        }
        else if (write_size)
        {
            if (os_spi_send(os_dev->os_spi_device, write_buf, write_size) <= 0)
            {
                result = SFUD_ERR_TIMEOUT;
            }
        }
        else
        {
            if (os_spi_recv(os_dev->os_spi_device, read_buf, read_size) <= 0)
            {
                result = SFUD_ERR_TIMEOUT;
            }
        }
    }

    return result;
}

#ifdef SFUD_USING_QSPI

static sfud_err qspi_read(const struct __sfud_spi   *spi,
                          uint32_t                   addr,
                          sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                          uint8_t                   *read_buf,
                          size_t                     read_size)
{
    struct os_qspi_message message;
    sfud_err result = SFUD_SUCCESS;

    sfud_flash *sfud_dev = (sfud_flash *)(spi->user_data);
    struct spi_flash_device *os_dev   = (struct spi_flash_device *)(sfud_dev->user_data);
    struct os_qspi_device   *qspi_dev = (struct os_qspi_device *)(os_dev->os_spi_device);

    OS_ASSERT(spi);
    OS_ASSERT(sfud_dev);
    OS_ASSERT(os_dev);
    OS_ASSERT(qspi_dev);

    /* Set message struct */
    message.instruction.content    = qspi_read_cmd_format->instruction;
    message.instruction.qspi_lines = qspi_read_cmd_format->instruction_lines;

    message.address.content    = addr;
    message.address.size       = qspi_read_cmd_format->address_size;
    message.address.qspi_lines = qspi_read_cmd_format->address_lines;

    message.alternate_bytes.content    = 0;
    message.alternate_bytes.size       = 0;
    message.alternate_bytes.qspi_lines = 0;

    message.dummy_cycles = qspi_read_cmd_format->dummy_cycles;

    message.parent.send_buf   = OS_NULL;
    message.parent.recv_buf   = read_buf;
    message.parent.length     = read_size;
    message.parent.cs_release = 1;
    message.parent.cs_take    = 1;
    message.qspi_data_lines   = qspi_read_cmd_format->data_lines;

    if (os_qspi_transfer_message(qspi_dev, &message) != read_size)
    {
        result = SFUD_ERR_TIMEOUT;
    }

    return result;
}
#endif

static void spi_lock(const sfud_spi *spi)
{
    sfud_flash *sfud_dev = (sfud_flash *)(spi->user_data);
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(sfud_dev->user_data);

    OS_ASSERT(spi);
    OS_ASSERT(sfud_dev);
    OS_ASSERT(os_dev);

    os_mutex_lock(&(os_dev->lock), OS_WAIT_FOREVER);
}

static void spi_unlock(const sfud_spi *spi)
{
    sfud_flash *sfud_dev = (sfud_flash *)(spi->user_data);
    struct spi_flash_device *os_dev = (struct spi_flash_device *)(sfud_dev->user_data);

    OS_ASSERT(spi);
    OS_ASSERT(sfud_dev);
    OS_ASSERT(os_dev);

    os_mutex_unlock(&(os_dev->lock));
}

static void retry_delay_100us(void)
{
    /* 100 microsecond delay */
    os_task_tsleep((OS_TICK_PER_SECOND * 1 + 9999) / 10000);
}

void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* Args point to the first variable parameter */
    va_start(args, format);
    os_kprintf("[SFUD] (%s:%ld) ", file, line);
    /* Must use vprintf to print */
    os_vsnprintf(log_buf, sizeof(log_buf), format, args);
    os_kprintf("%s\r\n", log_buf);
    va_end(args);
}

void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* Args point to the first variable parameter */
    va_start(args, format);
    os_kprintf("[SFUD] ");
    /* Must use vprintf to print */
    os_vsnprintf(log_buf, sizeof(log_buf), format, args);
    os_kprintf("%s\r\n", log_buf);
    va_end(args);
}

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_SUCCESS;

    OS_ASSERT(flash);

    /* Port SPI device interface */
    flash->spi.wr = spi_write_read;
#ifdef SFUD_USING_QSPI
    flash->spi.qspi_read = qspi_read;
#endif
    flash->spi.lock      = spi_lock;
    flash->spi.unlock    = spi_unlock;
    flash->spi.user_data = flash;
    if (OS_TICK_PER_SECOND < 1000)
    {
        os_kprintf("[SFUD] Warning: The OS tick(%d) is less than 1000. So the flash write will take more time.\r\n",
                   OS_TICK_PER_SECOND);
    }
    /* 100 microsecond delay */
    flash->retry.delay = retry_delay_100us;
    /* 60 seconds timeout */
    flash->retry.times = 60 * 10000;

    return result;
}

const static struct os_mtd_ops flash_mtd_ops = 
{
    .read_page   = mtd_sfud_read_page,
    .write_page  = mtd_sfud_write_page,
    .erase_block = mtd_sfud_erase_block,
};

/**
 ***********************************************************************************************************************
 * @brief           Probe SPI flash by SFUD(Serial Flash Universal Driver) driver library and though SPI device.
 *
 * @param[in]       spi_flash_dev_name          The name which will create SPI flash device.
 * @param[in]       spi_dev_name                Using SPI device name.
 *
 * @return          The operation status.
 * @retval          probed SPI flash device     Successful.
 * @retval          OS_NULL                     Fail.
 ***********************************************************************************************************************
 */
os_spi_flash_device_t os_sfud_flash_probe(const char *spi_flash_dev_name, const char *spi_dev_name)
{
    os_spi_flash_device_t os_dev = OS_NULL;
    sfud_flash *sfud_dev = OS_NULL;
    char *spi_flash_dev_name_bak = OS_NULL, *spi_dev_name_bak = OS_NULL;
    /*
     * using default flash SPI configuration for initialize SPI Flash
     * @note you also can change the SPI to other configuration after initialized finish
     */
    struct os_spi_configuration cfg = OS_SFUD_DEFAULT_SPI_CFG;
    extern sfud_err             sfud_device_init(sfud_flash * flash);
#ifdef SFUD_USING_QSPI
    struct os_qspi_configuration qspi_cfg = OS_SFUD_DEFAULT_QSPI_CFG;
    struct os_qspi_device       *qspi_dev = OS_NULL;
#endif

    OS_ASSERT(spi_flash_dev_name);
    OS_ASSERT(spi_dev_name);

    os_dev                 = (os_spi_flash_device_t)os_calloc(1, sizeof(struct spi_flash_device));
    sfud_dev               = (sfud_flash_t)os_calloc(1, sizeof(sfud_flash));
    spi_flash_dev_name_bak = (char *)os_calloc(1, strlen(spi_flash_dev_name) + 1);
    spi_dev_name_bak       = (char *)os_calloc(1, strlen(spi_dev_name) + 1);

    if (os_dev)
    {
        memset(os_dev, 0, sizeof(struct spi_flash_device));
        /* Initialize lock */
        os_mutex_init(&(os_dev->lock), spi_flash_dev_name, OS_FALSE);
    }

    if (os_dev && sfud_dev && spi_flash_dev_name_bak && spi_dev_name_bak)
    {
        memset(sfud_dev, 0, sizeof(sfud_flash));
        strncpy(spi_flash_dev_name_bak, spi_flash_dev_name, strlen(spi_flash_dev_name));
        strncpy(spi_dev_name_bak, spi_dev_name, strlen(spi_dev_name));
        /* Make string end sign */
        spi_flash_dev_name_bak[strlen(spi_flash_dev_name)] = '\0';
        spi_dev_name_bak[strlen(spi_dev_name)]             = '\0';
        /* SPI configure */
        {
            /* SPI device initialize */
            os_dev->os_spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);
            if (os_dev->os_spi_device == OS_NULL || os_dev->os_spi_device->parent.type != OS_DEVICE_TYPE_SPIDEVICE)
            {
                os_kprintf("ERROR: SPI device %s not found!\r\n", spi_dev_name);
                goto error;
            }
            sfud_dev->spi.name = spi_dev_name_bak;

#ifdef SFUD_USING_QSPI
            /* Set the qspi line number and configure the QSPI bus */
            if (os_dev->os_spi_device->bus->mode & OS_SPI_BUS_MODE_QSPI)
            {
                qspi_dev               = (struct os_qspi_device *)os_dev->os_spi_device;
                qspi_cfg.qspi_dl_width = qspi_dev->config.qspi_dl_width;
                os_qspi_configure(qspi_dev, &qspi_cfg);
            }
            else
#endif
                os_spi_configure(os_dev->os_spi_device, &cfg);
        }
        /* SFUD flash device initialize */
        {
            sfud_dev->name = spi_flash_dev_name_bak;
            /* Accessed each other */
            os_dev->user_data                = sfud_dev;
            os_dev->os_spi_device->user_data = os_dev;
            os_dev->mtd_dev.priv             = os_dev;
            sfud_dev->user_data              = os_dev;
            /* Initialize SFUD device */
            if (sfud_device_init(sfud_dev) != SFUD_SUCCESS)
            {
                os_kprintf("ERROR: SPI flash probe failed by SPI device %s.\r\n", spi_dev_name);
                goto error;
            }
            /* When initialize success, then copy SFUD flash device's geometry to SPI flash device */
            os_dev->mtd_dev.geometry.page_size  = sfud_dev->chip.erase_gran;
            os_dev->mtd_dev.geometry.block_size = sfud_dev->chip.erase_gran;
            os_dev->mtd_dev.geometry.capacity   = sfud_dev->chip.capacity;
#ifdef SFUD_USING_QSPI
            /* Reconfigure the QSPI bus for medium size */
            if (os_dev->os_spi_device->bus->mode & OS_SPI_BUS_MODE_QSPI)
            {
                qspi_cfg.medium_size = sfud_dev->chip.capacity;
                os_qspi_configure(qspi_dev, &qspi_cfg);
                if (qspi_dev->enter_qspi_mode != OS_NULL)
                    qspi_dev->enter_qspi_mode(qspi_dev);

                /* Set data lines width */
                sfud_qspi_fast_read_enable(sfud_dev, qspi_dev->config.qspi_dl_width);
            }
#endif /* SFUD_USING_QSPI */
        }

        /* Register device */
        os_dev->mtd_dev.mtd_ops = &flash_mtd_ops;
        mtd_device_register(&os_dev->mtd_dev, spi_flash_dev_name);
        DEBUG_TRACE("Probe SPI flash %s by SPI device %s success.\r\n", spi_flash_dev_name, spi_dev_name);
        return os_dev;
    }
    else
    {
        os_kprintf("ERROR: Low memory.\r\n");
        goto error;
    }

error:

    if (os_dev)
    {
        os_mutex_deinit(&(os_dev->lock));
    }
    /* May be one of objects memory was malloc success, so need free all */
    os_free(os_dev);
    os_free(sfud_dev);
    os_free(spi_flash_dev_name_bak);
    os_free(spi_dev_name_bak);

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete SPI flash device.
 *
 * @param[in]       spi_flash_dev   SPI flash device.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_sfud_flash_delete(os_spi_flash_device_t spi_flash_dev)
{
    sfud_flash *sfud_flash_dev = (sfud_flash *)(spi_flash_dev->user_data);

    OS_ASSERT(spi_flash_dev);
    OS_ASSERT(sfud_flash_dev);

    mtd_device_unregister(&spi_flash_dev->mtd_dev);

    os_mutex_deinit(&(spi_flash_dev->lock));

    os_free(sfud_flash_dev->spi.name);
    os_free(sfud_flash_dev->name);
    os_free(sfud_flash_dev);
    os_free(spi_flash_dev);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Find sfud flash device by SPI device name.
 *
 * @param[in]       spi_dev_name          Using SPI device name.
 *
 * @return          The result.
 * @retval          sfud flash device     Successful.
 * @retval          OS_NULL               Fail.
 ***********************************************************************************************************************
 */
sfud_flash_t os_sfud_flash_find(const char *spi_dev_name)
{
    os_spi_flash_device_t os_dev        = OS_NULL;
    struct os_spi_device *os_spi_device = OS_NULL;
    sfud_flash_t          sfud_dev      = OS_NULL;

    os_spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);

    if (os_spi_device == OS_NULL || os_spi_device->parent.type != OS_DEVICE_TYPE_SPIDEVICE)
    {
        os_kprintf("ERROR: SPI device %s not found!\r\n", spi_dev_name);
        goto __error;
    }

    os_dev = (os_spi_flash_device_t)(os_spi_device->user_data);
    if (os_dev && os_dev->user_data)
    {
        sfud_dev = (sfud_flash_t)(os_dev->user_data);
        return sfud_dev;
    }
    else
    {
        os_kprintf("ERROR: SFUD flash device not found!\r\n");
        goto __error;
    }

__error:
    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Find sfud flash device by flash device name.
 *
 * @param[in]       flash_dev_name        Using flash device name.
 *
 * @return          The result.
 * @retval          sfud flash device     Successful.
 * @retval          OS_NULL               Fail.
 ***********************************************************************************************************************
 */
sfud_flash_t os_sfud_flash_find_by_dev_name(const char *flash_dev_name)
{
    os_spi_flash_device_t os_dev   = OS_NULL;
    sfud_flash_t          sfud_dev = OS_NULL;

    os_dev = (os_spi_flash_device_t)os_device_find(flash_dev_name);
    if (os_dev == OS_NULL)
    {
        os_kprintf("ERROR: Flash device %s not found!\r\n", flash_dev_name);
        goto __error;
    }

    if (os_dev->user_data)
    {
        sfud_dev = (sfud_flash_t)(os_dev->user_data);
        return sfud_dev;
    }
    else
    {
        os_kprintf("ERROR: SFUD flash device not found!\r\n");
        goto __error;
    }

__error:
    return OS_NULL;
}

#if defined(OS_USING_SHELL)

#include <shell.h>

static void sf_operate(os_uint8_t argc, char **argv)
{

#define __is_print(ch)      ((unsigned int)((ch) - ' ') < 127u - ' ')
#define HEXDUMP_WIDTH       16
#define CMD_PROBE_INDEX     0
#define CMD_READ_INDEX      1
#define CMD_WRITE_INDEX     2
#define CMD_ERASE_INDEX     3
#define CMD_RW_STATUS_INDEX 4
#define CMD_BENCH_INDEX     5

    sfud_err                     result   = SFUD_SUCCESS;
    static const sfud_flash *    sfud_dev = NULL;
    static os_spi_flash_device_t os_dev = NULL, os_dev_bak = NULL;
    size_t                       i = 0, j = 0;

    const char *sf_help_info[] = {
        [CMD_PROBE_INDEX]     = "sf probe [spi_device]           - probe and init SPI flash by given 'spi_device'",
        [CMD_READ_INDEX]      = "sf read addr size               - read 'size' bytes starting at 'addr'",
        [CMD_WRITE_INDEX]     = "sf write addr data1 ... dataN   - write some bytes 'data' to flash starting at 'addr'",
        [CMD_ERASE_INDEX]     = "sf erase addr size              - erase 'size' bytes starting at 'addr'",
        [CMD_RW_STATUS_INDEX] = "sf status [<volatile> <status>] - read or write '1:volatile|0:non-volatile' 'status'",
        [CMD_BENCH_INDEX] = "sf bench                        - full chip benchmark. DANGER: It will erase full chip!",
    };

    if (argc < 2)
    {
        os_kprintf("Usage:\r\n");
        for (i = 0; i < sizeof(sf_help_info) / sizeof(char *); i++)
        {
            os_kprintf("%s\r\n", sf_help_info[i]);
        }
        os_kprintf("\r\n");
    }
    else
    {
        const char *operator= argv[1];
        uint32_t    addr, size;

        if (!strcmp(operator, "probe"))
        {
            if (argc < 3)
            {
                os_kprintf("Usage: %s.\r\n", sf_help_info[CMD_PROBE_INDEX]);
            }
            else
            {
                char *spi_dev_name = argv[2];
                os_dev_bak         = os_dev;

                /* Delete the old SPI flash device */
                if (os_dev_bak)
                {
                    os_sfud_flash_delete(os_dev_bak);
                }

                os_dev = os_sfud_flash_probe("sf_cmd", spi_dev_name);
                if (!os_dev)
                {
                    return;
                }

                sfud_dev = (sfud_flash_t)os_dev->user_data;
                if (sfud_dev->chip.capacity < 1024 * 1024)
                {
                    os_kprintf("%d KB %s is current selected device.\r\n",
                               sfud_dev->chip.capacity / 1024,
                               sfud_dev->name);
                }
                else
                {
                    os_kprintf("%d MB %s is current selected device.\r\n",
                               sfud_dev->chip.capacity / 1024 / 1024,
                               sfud_dev->name);
                }
            }
        }
        else
        {
            if (!sfud_dev)
            {
                os_kprintf("No flash device selected. Please run 'sf probe'.\r\n");
                return;
            }
            if (!strcmp(operator, "read"))
            {
                if (argc < 4)
                {
                    os_kprintf("Usage: %s.\r\n", sf_help_info[CMD_READ_INDEX]);
                    return;
                }
                else
                {
                    addr          = strtol(argv[2], NULL, 0);
                    size          = strtol(argv[3], NULL, 0);
                    uint8_t *data = os_calloc(1, size);
                    if (data)
                    {
                        result = sfud_read(sfud_dev, addr, size, data);
                        if (result == SFUD_SUCCESS)
                        {
                            os_kprintf("Read the %s flash data success. Start from 0x%08X, size is %ld. The data is:\r\n",
                                       sfud_dev->name,
                                       addr,
                                       size);
                            os_kprintf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
                            for (i = 0; i < size; i += HEXDUMP_WIDTH)
                            {
                                os_kprintf("[%08X] ", addr + i);
                                /* Dump hex */
                                for (j = 0; j < HEXDUMP_WIDTH; j++)
                                {
                                    if (i + j < size)
                                    {
                                        os_kprintf("%02X ", data[i + j]);
                                    }
                                    else
                                    {
                                        os_kprintf("   ");
                                    }
                                }
                                /* Dump char for hex */
                                for (j = 0; j < HEXDUMP_WIDTH; j++)
                                {
                                    if (i + j < size)
                                    {
                                        os_kprintf("%c", __is_print(data[i + j]) ? data[i + j] : '.');
                                    }
                                }
                                os_kprintf("\r\n");
                            }
                            os_kprintf("\r\n");
                        }
                        os_free(data);
                    }
                    else
                    {
                        os_kprintf("Low memory!\r\n");
                    }
                }
            }
            else if (!strcmp(operator, "write"))
            {
                if (argc < 4)
                {
                    os_kprintf("Usage: %s.\r\n", sf_help_info[CMD_WRITE_INDEX]);
                    return;
                }
                else
                {
                    addr          = strtol(argv[2], NULL, 0);
                    size          = argc - 3;
                    uint8_t *data = os_calloc(1, size);
                    if (data)
                    {
                        for (i = 0; i < size; i++)
                        {
                            data[i] = strtol(argv[3 + i], NULL, 0);
                        }
                        result = sfud_write(sfud_dev, addr, size, data);
                        if (result == SFUD_SUCCESS)
                        {
                            os_kprintf("Write the %s flash data success. Start from 0x%08X, size is %ld.\r\n",
                                       sfud_dev->name,
                                       addr,
                                       size);
                            os_kprintf("Write data: ");
                            for (i = 0; i < size; i++)
                            {
                                os_kprintf("%d ", data[i]);
                            }
                            os_kprintf(".\r\n");
                        }
                        os_free(data);
                    }
                    else
                    {
                        os_kprintf("Low memory!\r\n");
                    }
                }
            }
            else if (!strcmp(operator, "erase"))
            {
                if (argc < 4)
                {
                    os_kprintf("Usage: %s.\r\n", sf_help_info[CMD_ERASE_INDEX]);
                    return;
                }
                else
                {
                    addr   = strtol(argv[2], NULL, 0);
                    size   = strtol(argv[3], NULL, 0);
                    result = sfud_erase(sfud_dev, addr, size);
                    if (result == SFUD_SUCCESS)
                    {
                        os_kprintf("Erase the %s flash data success. Start from 0x%08X, size is %ld.\r\n",
                                   sfud_dev->name,
                                   addr,
                                   size);
                    }
                }
            }
            else if (!strcmp(operator, "status"))
            {
                if (argc < 3)
                {
                    uint8_t status;
                    result = sfud_read_status(sfud_dev, &status);
                    if (result == SFUD_SUCCESS)
                    {
                        os_kprintf("The %s flash status register current value is 0x%02X.\r\n", sfud_dev->name, status);
                    }
                }
                else if (argc == 4)
                {
                    bool    is_volatile = strtol(argv[2], NULL, 0);
                    uint8_t status      = strtol(argv[3], NULL, 0);
                    result              = sfud_write_status(sfud_dev, is_volatile, status);
                    if (result == SFUD_SUCCESS)
                    {
                        os_kprintf("Write the %s flash status register to 0x%02X success.\r\n", sfud_dev->name, status);
                    }
                }
                else
                {
                    os_kprintf("Usage: %s.\r\n", sf_help_info[CMD_RW_STATUS_INDEX]);
                    return;
                }
            }
            else if (!strcmp(operator, "bench"))
            {
                if ((argc > 2 && strcmp(argv[2], "yes")) || argc < 3)
                {
                    os_kprintf("DANGER: It will erase full chip! Please run 'sf bench yes'.\r\n");
                    return;
                }
                /* Full chip benchmark test */
                addr = 0;
                size = sfud_dev->chip.capacity;
                uint32_t start_time, time_cast;
                size_t   write_size = SFUD_WRITE_MAX_PAGE_SIZE, read_size = SFUD_WRITE_MAX_PAGE_SIZE;
                uint8_t *write_data = os_calloc(1, write_size), *read_data = os_calloc(1, read_size);

                if (write_data && read_data)
                {
                    memset(write_data, 0x55, write_size);
                    /* Benchmark testing */
                    os_kprintf("Erasing the %s %ld bytes data, waiting...\r\n", sfud_dev->name, size);
                    start_time = os_tick_get();
                    result     = sfud_erase(sfud_dev, addr, size);
                    if (result == SFUD_SUCCESS)
                    {
                        time_cast = os_tick_get() - start_time;
                        os_kprintf("Erase benchmark success, total time: %d.%03dS.\r\n",
                                   time_cast / OS_TICK_PER_SECOND,
                                   time_cast % OS_TICK_PER_SECOND / ((OS_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        os_kprintf("Erase benchmark has an error. Error code: %d.\r\n", result);
                    }
                    /* Write test */
                    os_kprintf("Writing the %s %ld bytes data, waiting...\r\n", sfud_dev->name, size);
                    start_time = os_tick_get();
                    for (i = 0; i < size; i += write_size)
                    {
                        result = sfud_write(sfud_dev, addr + i, write_size, write_data);
                        if (result != SFUD_SUCCESS)
                        {
                            os_kprintf("Writing %s failed, already wr for %lu bytes, write %d each time\r\n",
                                       sfud_dev->name,
                                       i,
                                       write_size);
                            break;
                        }
                    }
                    if (result == SFUD_SUCCESS)
                    {
                        time_cast = os_tick_get() - start_time;
                        os_kprintf("Write benchmark success, total time: %d.%03dS.\r\n",
                                   time_cast / OS_TICK_PER_SECOND,
                                   time_cast % OS_TICK_PER_SECOND / ((OS_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        os_kprintf("Write benchmark has an error. Error code: %d.\r\n", result);
                    }
                    /* Read test */
                    os_kprintf("Reading the %s %ld bytes data, waiting...\r\n", sfud_dev->name, size);
                    start_time = os_tick_get();
                    for (i = 0; i < size; i += read_size)
                    {
                        if (i + read_size <= size)
                        {
                            result = sfud_read(sfud_dev, addr + i, read_size, read_data);
                        }
                        else
                        {
                            result = sfud_read(sfud_dev, addr + i, size - i, read_data);
                        }
                        /* Data check */
                        if (memcmp(write_data, read_data, read_size))
                        {
                            os_kprintf("Data check ERROR! Please check you flash by other command.\r\n");
                            result = SFUD_ERR_READ;
                        }

                        if (result != SFUD_SUCCESS)
                        {
                            os_kprintf("Read %s failed, already rd for %lu bytes, read %d each time\r\n",
                                       sfud_dev->name,
                                       i,
                                       read_size);
                            break;
                        }
                    }
                    if (result == SFUD_SUCCESS)
                    {
                        time_cast = os_tick_get() - start_time;
                        os_kprintf("Read benchmark success, total time: %d.%03dS.\r\n",
                                   time_cast / OS_TICK_PER_SECOND,
                                   time_cast % OS_TICK_PER_SECOND / ((OS_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        os_kprintf("Read benchmark has an error. Error code: %d.\r\n", result);
                    }
                }
                else
                {
                    os_kprintf("Low memory!\r\n");
                }
                os_free(write_data);
                os_free(read_data);
            }
            else
            {
                os_kprintf("Usage:\r\n");
                for (i = 0; i < sizeof(sf_help_info) / sizeof(char *); i++)
                {
                    os_kprintf("%s\r\n", sf_help_info[i]);
                }
                os_kprintf("\r\n");
                return;
            }
            if (result != SFUD_SUCCESS)
            {
                os_kprintf("This flash operate has an error. Error code: %d.\r\n", result);
            }
        }
    }
}
SH_CMD_EXPORT(sf, sf_operate, "SPI Flash operate.");
#endif /* defined(OS_USING_SHELL) */

#endif /* OS_USING_SFUD */
