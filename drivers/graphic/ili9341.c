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
 * @file        ili9341.c
 *
 * @brief       This file provides ili9341 driver functions.
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

/*********************************************************************
 *
 * ILI934 Registers
 *
 *********************************************************************
 */
#define ILI9341_SLEEP_OUT     0x11 /* Sleep out register */
#define ILI9341_GAMMA         0x26 /* Gamma register */
#define ILI9341_DISPLAY_OFF   0x28 /* Display off register */
#define ILI9341_DISPLAY_ON    0x29 /* Display on register */
#define ILI9341_COLUMN_ADDR   0x2A /* Column address register */
#define ILI9341_PAGE_ADDR     0x2B /* Page address register */
#define ILI9341_GRAM          0x2C /* GRAM register */
#define ILI9341_MAC           0x36 /* Memory Access Control register */
#define ILI9341_PIXEL_FORMAT  0x3A /* Pixel Format register */
#define ILI9341_WDB           0x51 /* Write Brightness Display register */
#define ILI9341_WCD           0x53 /* Write Control Display register */
#define ILI9341_RGB_INTERFACE 0xB0 /* RGB Interface Signal Control */
#define ILI9341_FRC           0xB1 /* Frame Rate Control register */
#define ILI9341_BPC           0xB5 /* Blanking Porch Control register */
#define ILI9341_DFC           0xB6 /* Display Function Control register */
#define ILI9341_POWER1        0xC0 /* Power Control 1 register */
#define ILI9341_POWER2        0xC1 /* Power Control 2 register */
#define ILI9341_VCOM1         0xC5 /* VCOM Control 1 register */
#define ILI9341_VCOM2         0xC7 /* VCOM Control 2 register */
#define ILI9341_POWERA        0xCB /* Power control A register */
#define ILI9341_POWERB        0xCF /* Power control B register */
#define ILI9341_PGAMMA        0xE0 /* Positive Gamma Correction register */
#define ILI9341_NGAMMA        0xE1 /* Negative Gamma Correction register */
#define ILI9341_DTCA          0xE8 /* Driver timing control A */
#define ILI9341_DTCB          0xEA /* Driver timing control B */
#define ILI9341_POWER_SEQ     0xED /* Power on sequence register */
#define ILI9341_3GAMMA_EN     0xF2 /* 3 Gamma enable register */
#define ILI9341_INTERFACE     0xF6 /* Interface control register */
#define ILI9341_PRC           0xF7 /* Pump ratio control register */

static struct os_spi_device *spi_device;

static void LCD_WRX(uint32_t val)
{
    os_pin_write(BSP_ILI9341_WRX, val ? PIN_HIGH : PIN_LOW);
}

static void LCD_WriteM(unsigned char *pData, int NumBytes)
{
    os_spi_transfer(spi_device, pData, NULL, NumBytes);
}

static void LCD_X_Write0_8(unsigned char c)
{
    LCD_WRX(0);
    LCD_WriteM(&c, 1);
}

static void LCD_X_Write1_8(unsigned char c)
{
    LCD_WRX(1);
    LCD_WriteM(&c, 1);
}

#define ILI9341_WriteReg(reg)   LCD_X_Write0_8(reg)
#define ILI9341_WriteData(data) LCD_X_Write1_8(data)

static void _InitIL9341(void)
{
    const char *spi_client_name = BSP_ILI9341_SPI_BUS_NAME "ili9341";

    /* spi_cs, wrx */
    os_pin_mode(BSP_ILI9341_SPI_CS, PIN_MODE_OUTPUT);
    os_pin_mode(BSP_ILI9341_WRX, PIN_MODE_OUTPUT);

    /* spi */
    os_hw_spi_device_attach(BSP_ILI9341_SPI_BUS_NAME, spi_client_name, BSP_ILI9341_SPI_CS);
    spi_device = (struct os_spi_device *)os_device_find(spi_client_name);
    OS_ASSERT(spi_device);
    struct os_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 */
    cfg.max_hz     = 100 * 1000;                 /* SPI Interface with Clock Speeds Up to 400 KHz */
    os_spi_configure(spi_device, &cfg);

    /* Configure LCD */
    ILI9341_WriteReg(0xCA);
    ILI9341_WriteData(0xC3);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x50);
    ILI9341_WriteReg(ILI9341_POWERB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);
    ILI9341_WriteReg(ILI9341_POWER_SEQ);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);
    ILI9341_WriteReg(ILI9341_DTCA);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);
    ILI9341_WriteReg(ILI9341_POWERA);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);
    ILI9341_WriteReg(ILI9341_PRC);
    ILI9341_WriteData(0x20);
    ILI9341_WriteReg(ILI9341_DTCB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteReg(ILI9341_FRC);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x1B);
    ILI9341_WriteReg(ILI9341_DFC);
    ILI9341_WriteData(0x0A);
    ILI9341_WriteData(0xA2);
    ILI9341_WriteReg(ILI9341_POWER1);
    ILI9341_WriteData(0x10);
    ILI9341_WriteReg(ILI9341_POWER2);
    ILI9341_WriteData(0x10);
    ILI9341_WriteReg(ILI9341_VCOM1);
    ILI9341_WriteData(0x45);
    ILI9341_WriteData(0x15);
    ILI9341_WriteReg(ILI9341_VCOM2);
    ILI9341_WriteData(0x90);
    ILI9341_WriteReg(ILI9341_MAC);
    ILI9341_WriteData(0xC8);
    ILI9341_WriteReg(ILI9341_3GAMMA_EN);
    ILI9341_WriteData(0x00);
    ILI9341_WriteReg(ILI9341_RGB_INTERFACE);
    ILI9341_WriteData(0xC2);
    ILI9341_WriteReg(ILI9341_DFC);
    ILI9341_WriteData(0x0A);
    ILI9341_WriteData(0xA7);
    ILI9341_WriteData(0x27);
    ILI9341_WriteData(0x04);

    /* Column address set */
    ILI9341_WriteReg(ILI9341_COLUMN_ADDR);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xEF);

    /* Page address set */
    ILI9341_WriteReg(ILI9341_PAGE_ADDR);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x3F);
    ILI9341_WriteReg(ILI9341_INTERFACE);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x06);

    ILI9341_WriteReg(ILI9341_GRAM);
    os_task_msleep(200);

    ILI9341_WriteReg(ILI9341_GAMMA);
    ILI9341_WriteData(0x01);

    ILI9341_WriteReg(ILI9341_PGAMMA);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x29);
    ILI9341_WriteData(0x24);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0x78);
    ILI9341_WriteData(0x3C);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x13);
    ILI9341_WriteData(0x05);
    ILI9341_WriteData(0x17);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x00);
    ILI9341_WriteReg(ILI9341_NGAMMA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x16);
    ILI9341_WriteData(0x1B);
    ILI9341_WriteData(0x04);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x33);
    ILI9341_WriteData(0x42);
    ILI9341_WriteData(0x05);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0A);
    ILI9341_WriteData(0x28);
    ILI9341_WriteData(0x2F);
    ILI9341_WriteData(0x0F);

    ILI9341_WriteReg(ILI9341_SLEEP_OUT);
    os_task_msleep(200);

    ILI9341_WriteReg(ILI9341_DISPLAY_ON);
    ILI9341_WriteReg(ILI9341_GRAM);
}

static void ili9341_init_task(void *parameter)
{
    _InitIL9341();
}

/* ili9341 initial task */
static int device_ili9341_init(void)
{
    os_task_t *task;

    task = os_task_create("ili9341_init", ili9341_init_task, NULL, 2048, 4);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

/* depends on os_hw_lcd_init */
OS_DEVICE_INIT(device_ili9341_init, "1");
