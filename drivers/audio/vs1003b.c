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
 * @file        vs1003b.c
 *
 * @brief       This file provides vs1003b driver functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <os_workqueue.h>
#include <drv_spi.h>
#include <drv_gpio.h>
#include "vs1003b.h"

#ifndef BSP_VS1003B_SPI_BUS
#define BSP_VS1003B_SPI_BUS "spi2"
#endif
#define VS1003B_BUS_NAME "vs1003b"

#ifndef BSP_VS1003B_SPI_CS
#define BSP_VS1003B_SPI_CS 28
#endif
#define BSP_VS1003B_DREQ 7
#define BSP_VS1003B_DCS  4
#define BSP_VS1003B_RSET 5

#define SPI_SPEED_HIGH 1
#define SPI_SPEED_LOW  0

#define VS1003B_SPI_CS_SET()   os_pin_write(BSP_VS1003B_SPI_CS, PIN_HIGH)
#define VS1003B_SPI_CS_RESET() os_pin_write(BSP_VS1003B_SPI_CS, PIN_LOW)
#define VS1003B_DCS_SET()      os_pin_write(BSP_VS1003B_DCS, PIN_HIGH)
#define VS1003B_DCS_RESET()    os_pin_write(BSP_VS1003B_DCS, PIN_LOW)
#define VS1003B_RSET_SET()     os_pin_write(BSP_VS1003B_RSET, PIN_HIGH)
#define VS1003B_RSET_RESET()   os_pin_write(BSP_VS1003B_RSET, PIN_LOW)
#define VS1003B_DREQ_GET()     os_pin_read(BSP_VS1003B_DREQ)

#define VS1003B_WRITE_CMD 0x02
#define VS1003B_READ_CMD  0x03

#define VS1003B_SPI_MODE        0x00
#define VS1003B_SPI_STATUS      0x01
#define VS1003B_SPI_BASS        0x02
#define VS1003B_SPI_CLOCKF      0x03
#define VS1003B_SPI_DECODE_TIME 0x04
#define VS1003B_SPI_AUDATA      0x05
#define VS1003B_SPI_WRAM        0x06
#define VS1003B_SPI_WRAMADDR    0x07
#define VS1003B_SPI_HDAT0       0x08
#define VS1003B_SPI_HDAT1       0x09
#define VS1003B_SPI_AIADDR      0x0a
#define VS1003B_SPI_VOL         0x0b
#define VS1003B_SPI_AICTRL0     0x0c
#define VS1003B_SPI_AICTRL1     0x0d
#define VS1003B_SPI_AICTRL2     0x0e
#define VS1003B_SPI_AICTRL3     0x0f
#define VS1003B_SM_DIFF         0x01
#define VS1003B_SM_JUMP         0x02
#define VS1003B_SM_RESET        0x04
#define VS1003B_SM_OUTOFWAV     0x08
#define VS1003B_SM_PDOWN        0x10
#define VS1003B_SM_TESTS        0x20
#define VS1003B_SM_STREAM       0x40
#define VS1003B_SM_PLUSV        0x80
#define VS1003B_SM_DACT         0x100
#define VS1003B_SM_SDIORD       0x200
#define VS1003B_SM_SDISHARE     0x400
#define VS1003B_SM_SDINEW       0x800
#define VS1003B_SM_ADPCM        0x1000
#define VS1003B_SM_ADPCM_HP     0x2000

static struct os_spi_device *spi_device;

static void vs1003b_spi_setspeed(os_uint8_t speed)
{
    struct os_spi_configuration cfg;

    cfg.data_width = 8;
    cfg.mode       = OS_SPI_MODE_3 | OS_SPI_MSB; /* SPI Compatible Modes 0 */

    if (SPI_SPEED_HIGH == speed)
    {
        cfg.max_hz = 10 * 1000; /* SPI Interface with Clock Speeds Up to 400 KHz */
    }
    else if (SPI_SPEED_LOW == speed)
    {
        cfg.max_hz = 100 * 1000; /* SPI Interface with Clock Speeds Up to 400 KHz */
    }
    os_spi_configure(spi_device, &cfg);
}

static void vs1003b_write_reg(os_uint8_t reg, os_uint16_t value)
{
    os_uint8_t retry = 0;
    os_uint8_t send_databuf[4];
    /* os_uint8_t read_databuf[4]; */
    struct os_spi_message message;

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d  %04d\r\n", 0, 0);
            break;
        }
    }
    retry = 0;

    vs1003b_spi_setspeed(SPI_SPEED_LOW);
    VS1003B_DCS_SET();
    VS1003B_SPI_CS_RESET();

    send_databuf[0] = VS1003B_WRITE_CMD;
    send_databuf[1] = reg;
    send_databuf[2] = (value >> 8) & 0xFF;
    send_databuf[3] = value & 0xFF;

    message.send_buf   = send_databuf;
    message.recv_buf   = OS_NULL;
    message.length     = sizeof(send_databuf);
    message.cs_take    = 0;
    message.cs_release = 0;
    message.next       = OS_NULL;
    os_spi_transfer_message(spi_device, &message);

    VS1003B_SPI_CS_SET();
    vs1003b_spi_setspeed(SPI_SPEED_HIGH);
}

static os_uint16_t vs1003b_read_reg(os_uint8_t reg)
{
    os_uint16_t           value;
    os_uint8_t            retry = 0;
    os_uint8_t            send_databuf[4];
    os_uint8_t            read_databuf[4];
    struct os_spi_message message;

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d  %04d\r\n", 0, 1);
            break;
        }
    }
    retry = 0;

    vs1003b_spi_setspeed(SPI_SPEED_LOW);
    VS1003B_DCS_SET();
    VS1003B_SPI_CS_RESET();

    send_databuf[0] = VS1003B_READ_CMD;
    send_databuf[1] = reg;
    send_databuf[2] = 0xFF;
    send_databuf[3] = 0xFF;

    message.send_buf   = send_databuf;
    message.recv_buf   = read_databuf;
    message.length     = sizeof(send_databuf);
    message.cs_take    = 0;
    message.cs_release = 0;
    message.next       = OS_NULL;
    os_spi_transfer_message(spi_device, &message);

    VS1003B_SPI_CS_SET();
    vs1003b_spi_setspeed(SPI_SPEED_HIGH);
    value = read_databuf[2];
    value = (value << 8) | read_databuf[3];

    return value;
}

static void vs1003b_resetdecodetime(void)
{
    vs1003b_write_reg(VS1003B_SPI_DECODE_TIME, 0x0000);
    vs1003b_write_reg(VS1003B_SPI_DECODE_TIME, 0x0000);
}

#if 0
static os_uint16_t vs1003b_getdecodetime(void)
{
    os_uint16_t value;
    vs1003b_read_reg(VS1003B_SPI_DECODE_TIME);
    return value;
}
#endif

static void vs1003b_send_dummybyte(void)
{
    os_uint8_t send_databuf[4];
    /* os_uint8_t read_databuf[4]; */
    struct os_spi_message message;

    send_databuf[0] = 0XFF;
    send_databuf[1] = 0XFF;
    send_databuf[2] = 0XFF;
    send_databuf[3] = 0XFF;

    message.send_buf   = send_databuf;
    message.recv_buf   = OS_NULL;
    message.length     = sizeof(send_databuf);
    message.cs_take    = 0;
    message.cs_release = 0;
    message.next       = OS_NULL;
    os_spi_transfer_message(spi_device, &message);
}

void vs1003b_softreset(void)
{
    os_uint8_t retry = 0;
    /* os_uint8_t send_databuf[2] = {0xff,0xff}; */
    /* os_uint8_t read_databuf; */

    os_uint16_t databuf;

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d  %04d\r\n", 0, 2);
            break;
        }
    }
    retry = 0;

    vs1003b_write_reg(VS1003B_SPI_MODE, 0x0804);

    while (vs1003b_read_reg(VS1003B_SPI_MODE) != 0x0800)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_MODE);
            os_kprintf("%04d\r\n", databuf);
            break;
        }
    }
    retry = 0;

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d  %04d\r\n", 0, 3);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_CLOCKF) != 0x9800)
    {
        vs1003b_write_reg(VS1003B_SPI_CLOCKF, 0x9800);
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_CLOCKF);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_AUDATA) != 0xBB81)
    {
        vs1003b_write_reg(VS1003B_SPI_AUDATA, 0xBB81);
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_AUDATA);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_BASS) != 0x0055)
    {
        vs1003b_write_reg(VS1003B_SPI_BASS, 0x0055);
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_BASS);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_VOL) != 0x2020)
    {
        vs1003b_write_reg(VS1003B_SPI_VOL, 0x2020);
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_VOL);
            break;
        }
    }
    retry = 0;

    vs1003b_resetdecodetime();
    VS1003B_DCS_RESET();
    vs1003b_send_dummybyte();
    VS1003B_DCS_SET();

    os_task_msleep(20);
}

#if 0
static void vs1003b_reset(void)
{
    os_uint8_t retry = 0;   
    os_uint8_t send_databuf[2] = {0xff,0xff};
    /* os_uint8_t read_databuf; */
    
        
    os_spi_transfer(spi_device,send_databuf,OS_NULL,1);
    
    VS1003B_DCS_SET();

    while(VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if( retry++ > 5)
        { 
           os_kprintf("%04d  %04d\r\n",0,4);
           break; 
        }
    }
    retry = 0;   

    os_task_msleep(20);
}
#endif
#if 0
static void vs1003b_setvol(void)
{
    os_uint8_t i= 0;
    /* os_uint8_t retry = 0; */
    os_uint16_t bass = 0;
    os_uint16_t volt = 0;
    os_uint8_t vset = 0;
    uint8_t vs1003ram[5] = { 0 , 0 , 0 , 0 , 250 };

    vset = 255 - vs1003ram[4];  
    volt =vset;
    volt <<= 8;
    volt += vset;
    
    for( i = 0; i < 4; i++ )
    {
        bass <<= 4;
        bass += vs1003ram[i]; 
    }     

    vs1003b_write_reg(VS1003B_SPI_BASS,bass);  
    vs1003b_write_reg(VS1003B_SPI_VOL,volt); 

}
#endif

void vs1003b_writedata(os_uint8_t *buf)
{
    os_uint8_t count = 32;

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, buf, OS_NULL, count);
    VS1003B_DCS_SET();
}

void vs1003b_record_init(void)
{
    os_uint8_t retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_CLOCKF) != 0x9800)
    {
        vs1003b_write_reg(VS1003B_SPI_CLOCKF, 0x9800);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_CLOCKF);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_BASS) != 0x0000)
    {
        vs1003b_write_reg(VS1003B_SPI_BASS, 0x0000);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_BASS);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_AICTRL0) != 0x0012)
    {
        vs1003b_write_reg(VS1003B_SPI_AICTRL0, 0x0012);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_AICTRL0);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_AICTRL1) != 0x1000)
    {
        vs1003b_write_reg(VS1003B_SPI_AICTRL1, 0x1000);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_AICTRL1);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_MODE) != 0x1804)
    {
        vs1003b_write_reg(VS1003B_SPI_MODE, 0x1804);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_MODE);
            break;
        }
    }
    retry = 0;

    while (vs1003b_read_reg(VS1003B_SPI_CLOCKF) != 0x9800)
    {
        vs1003b_write_reg(VS1003B_SPI_CLOCKF, 0x9800);
        os_task_msleep(5);
        if (retry++ > 100)
        {
            os_kprintf("%04d\r\n", VS1003B_SPI_CLOCKF);
            break;
        }
    }
    retry = 0;
}

void vs1003b_sinetest(void)
{
    os_uint8_t retry            = 0;
    os_uint8_t send_databuf0[8] = {0x53, 0xef, 0x6e, 0x24, 0x00, 0x00, 0x00, 0x00};
    os_uint8_t send_databuf1[8] = {0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00};
    os_uint8_t send_databuf2[8] = {0x53, 0xef, 0x6e, 0x44, 0x00, 0x00, 0x00, 0x00};
    /* os_uint8_t read_databuf; */

    /* os_uint16_t databuf; */

    vs1003b_write_reg(VS1003B_SPI_VOL, 0x2020);
    vs1003b_write_reg(VS1003B_SPI_MODE, 0x0820);

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            break;
        }
    }
    retry = 0;

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, send_databuf0, OS_NULL, sizeof(send_databuf0));
    VS1003B_DCS_SET();
    os_task_msleep(500);

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, send_databuf1, OS_NULL, sizeof(send_databuf1));
    VS1003B_DCS_SET();
    os_task_msleep(100);

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, send_databuf2, OS_NULL, sizeof(send_databuf0));
    VS1003B_DCS_SET();
    os_task_msleep(500);

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, send_databuf1, OS_NULL, sizeof(send_databuf1));
    VS1003B_DCS_SET();
    os_task_msleep(100);
}

void vs1003b_ramtest(void)
{
    os_uint8_t retry           = 0;
    os_uint8_t send_databuf[8] = {0x4d, 0xea, 0x6d, 0x54, 0x00, 0x00, 0x00, 0x00};
    /* os_uint16_t read_data = 0; */

    VS1003B_RSET_RESET();

    vs1003b_write_reg(VS1003B_SPI_MODE, 0x0802);

    while (VS1003B_DREQ_GET() == 0)
    {
        os_task_msleep(5);
        if (retry++ > 5)
        {
            os_kprintf("%04d  %04d\r\n", 0, 6);
            break;
        }
    }
    retry = 0;

    VS1003B_DCS_RESET();
    os_spi_transfer(spi_device, send_databuf, OS_NULL, sizeof(send_databuf));
    VS1003B_DCS_SET();
    os_task_msleep(100);

    /* read_data = vs1003b_read_reg(VS1003B_SPI_HDAT0); */
}

void vs1003b_init(void) /* void *parameter */
{
    /* spi_cs, wrx */
    os_pin_mode(BSP_VS1003B_DCS, PIN_MODE_OUTPUT);
    os_pin_mode(BSP_VS1003B_RSET, PIN_MODE_OUTPUT);
    os_pin_mode(BSP_VS1003B_DREQ, PIN_MODE_INPUT_PULLUP);

    /* spi */
    os_hw_spi_device_attach(BSP_VS1003B_SPI_BUS, VS1003B_BUS_NAME, BSP_VS1003B_SPI_CS);
    spi_device = (struct os_spi_device *)os_device_find(VS1003B_BUS_NAME);
    OS_ASSERT(spi_device);

    vs1003b_spi_setspeed(SPI_SPEED_HIGH);

    VS1003B_RSET_RESET();
    os_task_msleep(100);
    VS1003B_RSET_SET();

    VS1003B_DCS_SET();
    VS1003B_SPI_CS_SET();

    vs1003b_softreset();
}
