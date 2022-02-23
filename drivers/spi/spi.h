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
 * @file        spi.h
 *
 * @brief       This file provides struct definition and spi functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <stdlib.h>
#include <os_task.h>
#include <os_assert.h>
#include <device.h>
#include <os_mutex.h>
#include <os_errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_SPI_CPHA (1 << 0) /* Bit[0]:CPHA, clock phase */
#define OS_SPI_CPOL (1 << 1) /* Bit[1]:CPOL, clock polarity */
/*
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low->high transition)
 *    and data are propagated on a falling edge (high->low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define OS_SPI_LSB (0 << 2) /* Bit[2]: 0-LSB */
#define OS_SPI_MSB (1 << 2) /* Bit[2]: 1-MSB */

#define OS_SPI_MASTER (0 << 3) /* SPI master device */
#define OS_SPI_SLAVE  (1 << 3) /* SPI slave device */

#define OS_SPI_MODE_0 (0 | 0)                     /* CPOL = 0, CPHA = 0 */
#define OS_SPI_MODE_1 (0 | OS_SPI_CPHA)           /* CPOL = 0, CPHA = 1 */
#define OS_SPI_MODE_2 (OS_SPI_CPOL | 0)           /* CPOL = 1, CPHA = 0 */
#define OS_SPI_MODE_3 (OS_SPI_CPOL | OS_SPI_CPHA) /* CPOL = 1, CPHA = 1 */

#define OS_SPI_MODE_MASK (OS_SPI_CPHA | OS_SPI_CPOL | OS_SPI_MSB)

#define OS_SPI_BUS_MODE_SPI  (1 << 0)
#define OS_SPI_BUS_MODE_QSPI (1 << 1)

#define OS_SPI_CS_HIGH (1 << 4) /* Chipselect active high */
#define OS_SPI_NO_CS   (1 << 5) /* No chipselect */
#define OS_SPI_3WIRE   (1 << 6) /* SI/SO pin shared */
#define OS_SPI_READY   (1 << 7) /* Slave pulls low to pause */

#define OS_SPI_DEVICE_CTRL_RW IOC_SPIDEVICE(0x00)

/**
 ***********************************************************************************************************************
 * @struct      os_spi_message
 *
 * @brief       SPI message structure
 ***********************************************************************************************************************
 */
struct os_spi_message
{
    const void *send_buf;
    void       *recv_buf;
    os_size_t   length;

    struct os_spi_message *next;

    unsigned cs_take : 1;
    unsigned cs_release : 1;
};

/**
 ***********************************************************************************************************************
 * @struct      os_spi_configuration
 *
 * @brief       SPI configuration structure
 ***********************************************************************************************************************
 */
struct os_spi_configuration
{
    os_uint8_t  mode;
    os_uint8_t  data_width;
    os_uint16_t reserved;

    os_uint32_t max_hz;
};

struct os_spi_ops;
struct os_spi_bus
{
    struct os_device         parent;
    os_uint8_t               mode;
    const struct os_spi_ops *ops;

    struct os_mutex       lock;
    struct os_spi_device *owner;
};

/**
 ***********************************************************************************************************************
 * @struct      os_spi_ops
 *
 * @brief       SPI operators
 ***********************************************************************************************************************
 */
struct os_spi_ops
{
    os_err_t (*configure)(struct os_spi_device *device, struct os_spi_configuration *configuration);
    os_uint32_t (*xfer)(struct os_spi_device *device, struct os_spi_message *message);
};

/**
 ***********************************************************************************************************************
 * @struct      os_spi_device
 *
 * @brief       SPI Virtual BUS, one device must connected to a virtual BUS
 ***********************************************************************************************************************
 */
struct os_spi_device
{
    struct os_device   parent;
    struct os_spi_bus *bus;

    struct os_spi_configuration config;

    os_base_t cs_pin;
    
    void *user_data;
};

struct os_qspi_message
{
    struct os_spi_message parent;

    /* Instruction stage */
    struct
    {
        os_uint8_t content;
        os_uint8_t qspi_lines;
    } instruction;

    /* Address and alternate_bytes stage */
    struct
    {
        os_uint32_t content;
        os_uint8_t  size;
        os_uint8_t  qspi_lines;
    } address, alternate_bytes;

    /* Dummy_cycles stage */
    os_uint32_t dummy_cycles;

    /* Number of lines in qspi data stage, the other configuration items are in parent */
    os_uint8_t qspi_data_lines;
};

struct os_qspi_configuration
{
    struct os_spi_configuration parent;
    /* The size of medium */
    os_uint32_t medium_size;
    /* Double data rate mode */
    os_uint8_t ddr_mode;
    /* The data lines max width which QSPI bus supported, such as 1, 2, 4 */
    os_uint8_t qspi_dl_width;
};

struct os_qspi_device
{
    struct os_spi_device parent;

    struct os_qspi_configuration config;

    void (*enter_qspi_mode)(struct os_qspi_device *device);

    void (*exit_qspi_mode)(struct os_qspi_device *device);
};

#define SPI_DEVICE(dev) ((struct os_spi_device *)(dev))

os_err_t os_spi_bus_register(struct os_spi_bus       *bus,
                             const char              *name,
                             const struct os_spi_ops *ops);

os_err_t os_spi_bus_attach_device(struct os_spi_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  os_base_t            cs_pin);

os_err_t os_spi_take_bus(struct os_spi_device *device);

os_err_t os_spi_release_bus(struct os_spi_device *device);

os_err_t os_spi_take(struct os_spi_device *device);

os_err_t os_spi_release(struct os_spi_device *device);

os_err_t os_spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg);

os_err_t os_spi_send_then_recv(struct os_spi_device *device,
                               const void           *send_buf,
                               os_size_t             send_length,
                               void                 *recv_buf,
                               os_size_t             recv_length);

os_err_t os_spi_send_then_send(struct os_spi_device *device,
                               const void           *send_buf1,
                               os_size_t             send_length1,
                               const void           *send_buf2,
                               os_size_t             send_length2);

os_size_t os_spi_transfer(struct os_spi_device *device, const void *send_buf, void *recv_buf, os_size_t length);

struct os_spi_message *os_spi_transfer_message(struct os_spi_device *device, struct os_spi_message *message);

OS_INLINE os_size_t os_spi_recv(struct os_spi_device *device, void *recv_buf, os_size_t length)
{
    return os_spi_transfer(device, OS_NULL, recv_buf, length);
}

OS_INLINE os_size_t os_spi_send(struct os_spi_device *device, const void *send_buf, os_size_t length)
{
    return os_spi_transfer(device, send_buf, OS_NULL, length);
}

OS_INLINE os_uint8_t os_spi_sendrecv8(struct os_spi_device *device, os_uint8_t data)
{
    os_uint8_t value;

    os_spi_send_then_recv(device, &data, 1, &value, 1);

    return value;
}

OS_INLINE os_uint16_t os_spi_sendrecv16(struct os_spi_device *device, os_uint16_t data)
{
    os_uint16_t value;

    os_spi_send_then_recv(device, &data, 2, &value, 2);

    return value;
}

OS_INLINE void os_spi_message_append(struct os_spi_message *list, struct os_spi_message *message)
{
    OS_ASSERT(list != OS_NULL);
    if (message == OS_NULL)
        return; /* not append */

    while (list->next != OS_NULL)
    {
        list = list->next;
    }

    list->next    = message;
    message->next = OS_NULL;
}

os_err_t os_qspi_configure(struct os_qspi_device *device, struct os_qspi_configuration *cfg);

os_err_t os_qspi_bus_register(struct os_spi_bus *bus, const char *name, const struct os_spi_ops *ops);

os_size_t os_qspi_transfer_message(struct os_qspi_device *device, struct os_qspi_message *message);

os_err_t os_qspi_send_then_recv(struct os_qspi_device *device,
                                const void            *send_buf,
                                os_size_t              send_length,
                                void                  *recv_buf,
                                os_size_t              recv_length);

os_err_t os_qspi_send(struct os_qspi_device *device, const void *send_buf, os_size_t length);

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t cs_pin);

#ifdef __cplusplus
}
#endif

#endif
