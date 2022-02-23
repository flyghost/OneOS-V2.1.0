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
 * @file        st7789vw.c
 *
 * @brief       This file provides st7789vw driver functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <device.h>
#include <os_memory.h>
#include <graphic/graphic.h>
#include <lcd_port.h>
#include "st7789vw.h"
#include "st7789vw_font.h"
#include "drv_gpio.h"
#include "drv_spi.h"

#define DBG_TAG "LCD"

#include <dlog.h>

#define LCD_PWR_PIN OS_ST7789VW_PWR_PIN
#define LCD_DC_PIN  OS_ST7789VW_DC_PIN
#define LCD_RES_PIN OS_ST7789VW_RES_PIN

#define LCD_CLEAR_SEND_NUMBER 5760

os_uint16_t BACK_COLOR = WHITE, FORE_COLOR = BLACK;

struct st7789_lcd {
    os_device_graphic_t graphic;
};

static struct os_spi_device *spi_dev_lcd;

static os_err_t lcd_write_cmd(const os_uint8_t cmd)
{
    os_size_t len;

    os_pin_write(LCD_DC_PIN, PIN_LOW);

    len = os_spi_send(spi_dev_lcd, &cmd, 1);

    if (len != 1)
    {
        LOG_I(DBG_TAG,"lcd_write_cmd error. %d", len);
        return OS_ERROR;
    }
    else
    {
        return OS_EOK;
    }
}

static os_err_t lcd_write_data(const os_uint8_t data)
{
    os_size_t len;

    os_pin_write(LCD_DC_PIN, PIN_HIGH);

    len = os_spi_send(spi_dev_lcd, &data, 1);

    if (len != 1)
    {
        LOG_I(DBG_TAG,"lcd_write_data error. %d", len);
        return OS_ERROR;
    }
    else
    {
        return OS_EOK;
    }
}

static os_err_t lcd_write_half_word(const os_uint16_t da)
{
    os_size_t len;
    char      data[2] = {0};

    data[0] = da >> 8;
    data[1] = da;

    os_pin_write(LCD_DC_PIN, PIN_HIGH);
    len = os_spi_send(spi_dev_lcd, data, 2);
    if (len != 2)
    {
        LOG_I(DBG_TAG,"lcd_write_half_word error. %d", len);
        return OS_ERROR;
    }
    else
    {
        return OS_EOK;
    }
}

static void lcd_gpio_init(void)
{
    os_pin_mode(LCD_DC_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(LCD_RES_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(LCD_PWR_PIN, PIN_MODE_OUTPUT);

    lcd_display_off();

    os_pin_write(LCD_RES_PIN, PIN_HIGH);
    os_task_msleep(50);
    os_pin_write(LCD_RES_PIN, PIN_LOW);
    /* wait at least 100ms for reset */
    os_task_msleep(100);
    os_pin_write(LCD_RES_PIN, PIN_HIGH);
    os_task_msleep(150);
}

static int os_hw_lcd_config(void)
{
    spi_dev_lcd = (struct os_spi_device *)os_device_find(OS_ST7789VW_SPI_BUS_NAME "_lcd");

    struct os_spi_configuration cfg;
    cfg.data_width = 8;

    if (OS_ST7789VW_SPI_BUS_MODE == 0)
    {
        cfg.mode = OS_SPI_MASTER | OS_SPI_MODE_0 | OS_SPI_MSB | OS_SPI_3WIRE;
    }
    else
    {
        cfg.mode = OS_SPI_MASTER | OS_SPI_MODE_3 | OS_SPI_MSB | OS_SPI_3WIRE;
    }

    cfg.max_hz = 42 * 1000 * 1000; /* 42M,SPI max 42MHz,lcd 4-wire spi */

    os_spi_configure(spi_dev_lcd, &cfg);

    return OS_EOK;
}

static int st7789_init(void)
{
    os_hw_spi_device_attach(OS_ST7789VW_SPI_BUS_NAME, OS_ST7789VW_SPI_BUS_NAME "_lcd", OS_ST7789VW_SPI_CS_PIN);
    os_hw_lcd_config();
    lcd_gpio_init();
    
    /* Memory Data Access Control */
    lcd_write_cmd(0x36);
    lcd_write_data(0x00);
    /* RGB 5-6-5-bit  */
    lcd_write_cmd(0x3A);
    lcd_write_data(0x65);
    /* Porch Setting */
    lcd_write_cmd(0xB2);
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);
    /*  Gate Control */
    lcd_write_cmd(0xB7);
    lcd_write_data(0x35);
    /* VCOM Setting */
    lcd_write_cmd(0xBB);
    lcd_write_data(0x19);
    /* LCM Control */
    lcd_write_cmd(0xC0);
    lcd_write_data(0x2C);
    /* VDV and VRH Command Enable */
    lcd_write_cmd(0xC2);
    lcd_write_data(0x01);
    /* VRH Set */
    lcd_write_cmd(0xC3);
    lcd_write_data(0x12);
    /* VDV Set */
    lcd_write_cmd(0xC4);
    lcd_write_data(0x20);
    /* Frame Rate Control in Normal Mode */
    lcd_write_cmd(0xC6);
    lcd_write_data(0x0F);
    /* Power Control 1 */
    lcd_write_cmd(0xD0);
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);
    /* Positive Voltage Gamma Control */
    lcd_write_cmd(0xE0);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2B);
    lcd_write_data(0x3F);
    lcd_write_data(0x54);
    lcd_write_data(0x4C);
    lcd_write_data(0x18);
    lcd_write_data(0x0D);
    lcd_write_data(0x0B);
    lcd_write_data(0x1F);
    lcd_write_data(0x23);
    /* Negative Voltage Gamma Control */
    lcd_write_cmd(0xE1);
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0C);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2C);
    lcd_write_data(0x3F);
    lcd_write_data(0x44);
    lcd_write_data(0x51);
    lcd_write_data(0x2F);
    lcd_write_data(0x1F);
    lcd_write_data(0x1F);
    lcd_write_data(0x20);
    lcd_write_data(0x23);
    /* Display Inversion On */
    lcd_write_cmd(0x21);
    /* Sleep Out */
    lcd_write_cmd(0x11);

    /* wait for power stability */
    os_task_msleep(100);

    lcd_clear(WHITE);

    /* display on */
    lcd_display_on();

    lcd_write_cmd(0x29);

    return OS_EOK;
}

static void st7789_fill(struct os_device *dev, struct os_device_rect_info *rect)
{
    os_uint16_t i = 0, j = 0;
    os_uint32_t size = 0, size_remain = 0;
    os_uint8_t *fill_buf = OS_NULL;

    os_int32_t x_start = rect->x;
    os_int32_t y_start = rect->y;
    os_int32_t x_end   = rect->x + rect->width - 1;
    os_int32_t y_end   = rect->y + rect->height - 1;
    const char *color  = rect->color;

    size = rect->width * rect->height * 2;

    if (size > LCD_CLEAR_SEND_NUMBER)
    {
        /* the number of remaining to be filled */
        size_remain = size - LCD_CLEAR_SEND_NUMBER;
        size        = LCD_CLEAR_SEND_NUMBER;
    }

    lcd_address_set(x_start, y_start, x_end, y_end);

    fill_buf = (os_uint8_t *)os_calloc(1, size);
    if (fill_buf)
    {
        /* fast fill */
        while (1)
        {
            for (i = 0; i < size; i += 2)
            {
                fill_buf[i + 1] = *color++;
                fill_buf[i]     = *color++;
            }
            
            os_pin_write(LCD_DC_PIN, PIN_HIGH);
            os_spi_send(spi_dev_lcd, fill_buf, size);

            /* Fill completed */
            if (size_remain == 0)
                break;

            /* calculate the number of fill next time */
            if (size_remain > LCD_CLEAR_SEND_NUMBER)
            {
                size_remain = size_remain - LCD_CLEAR_SEND_NUMBER;
            }
            else
            {
                size        = size_remain;
                size_remain = 0;
            }
        }
        os_free(fill_buf);
    }
    else
    {
        for (i = y_start; i <= y_end; i++)
        {
            for (j = x_start; j <= x_end; j++, color += 2)
                lcd_write_half_word((*color << 8) | *(color + 1));
        }
    }

}

const static struct os_device_graphic_ops ops =
{
    .set_pixel  = OS_NULL,
    .get_pixel  = OS_NULL,

    .draw_hline = OS_NULL,
    .draw_vline = OS_NULL,

    .blit_line  = OS_NULL,

    .display_on = OS_NULL,
    .update     = OS_NULL,

    .fill       = st7789_fill,
};

static int os_hw_lcd_init(void)
{    
    struct st7789_lcd *lcd;

    lcd = os_calloc(1, sizeof(struct st7789_lcd));

    OS_ASSERT(lcd);

    st7789_init();

    lcd->graphic.info.width            = LCD_WIDTH;
    lcd->graphic.info.height           = LCD_HEIGHT;
    lcd->graphic.info.pixel_format     = LCD_PIXEL_FORMAT;
    lcd->graphic.info.bits_per_pixel   = LCD_BITS_PER_PIXEL;

#if 0
    lcd->graphic.info.framebuffer_size = LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8;
    lcd->graphic.info.framebuffer      = (void *)os_malloc_align(LCD_WIDTH * LCD_HEIGHT * (LCD_BITS_PER_PIXEL / 8), 32);
    OS_ASSERT(lcd->graphic.info.framebuffer);
    memset(lcd->graphic.info.framebuffer, 0, LCD_WIDTH * LCD_HEIGHT * (LCD_BITS_PER_PIXEL / 8));
#endif

    lcd->graphic.ops = &ops;

    os_graphic_register("lcd", &lcd->graphic);

    os_kprintf("st7789 lcd found.\r\n");

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_lcd_init,"1");

/**
 ***********************************************************************************************************************
 * @brief           Set background color and foreground color
 *
 * @param[in]       back       background color
 * @param[in]       fore       fore color
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_set_color(os_uint16_t back, os_uint16_t fore)
{
    BACK_COLOR = back;
    FORE_COLOR = fore;
}

void lcd_display_on(void)
{
#if OS_ST7789VW_PWR_PIN_ACTIVE == 1
    os_pin_write(LCD_PWR_PIN, PIN_HIGH);
#else
    os_pin_write(LCD_PWR_PIN, PIN_LOW);
#endif
}

void lcd_display_off(void)
{
#if OS_ST7789VW_PWR_PIN_ACTIVE == 1
    os_pin_write(LCD_PWR_PIN, PIN_LOW);
#else
    os_pin_write(LCD_PWR_PIN, PIN_HIGH);
#endif
}

/* lcd enter the minimum power consumption mode and backlight off. */
void lcd_enter_sleep(void)
{
    lcd_display_off();
    os_task_msleep(5);
    lcd_write_cmd(0x10);
}
/* lcd turn off sleep mode and backlight on. */
void lcd_exit_sleep(void)
{
    lcd_display_on();
    os_task_msleep(5);
    lcd_write_cmd(0x11);
    os_task_msleep(120);
}

/**
 ***********************************************************************************************************************
 * @brief           Set drawing area
 *
 * @param[in]       x1       start of x position
 * @param[in]       y1       start of y position
 * @param[in]       x2       end of x position
 * @param[in]       y2       end of y position
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_address_set(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2)
{
    lcd_write_cmd(0x2a);
    lcd_write_data(x1 >> 8);
    lcd_write_data(x1);
    lcd_write_data(x2 >> 8);
    lcd_write_data(x2);

    lcd_write_cmd(0x2b);
    lcd_write_data(y1 >> 8);
    lcd_write_data(y1);
    lcd_write_data(y2 >> 8);
    lcd_write_data(y2);

    lcd_write_cmd(0x2C);
}

/**
 ***********************************************************************************************************************
 * @brief           clear the lcd
 *
 * @param[in]       color       Fill color
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_clear(os_uint16_t color)
{
    os_uint16_t i, j;
    os_uint8_t  data[2] = {0};
    os_uint8_t *buf     = OS_NULL;

    data[0] = color >> 8;
    data[1] = color;
    lcd_address_set(0, 0, LCD_W - 1, LCD_H - 1);

    /* 5760 = 240*240/20 */
    buf = os_calloc(1, LCD_CLEAR_SEND_NUMBER);
    if (buf)
    {
        /* 2880 = 5760/2 color is 16 bit */
        for (j = 0; j < LCD_CLEAR_SEND_NUMBER / 2; j++)
        {
            buf[j * 2]     = data[0];
            buf[j * 2 + 1] = data[1];
        }

        os_pin_write(LCD_DC_PIN, PIN_HIGH);
        for (i = 0; i < 20; i++)
        {
            os_spi_send(spi_dev_lcd, buf, LCD_CLEAR_SEND_NUMBER);
        }
        os_free(buf);
    }
    else
    {
        os_pin_write(LCD_DC_PIN, PIN_HIGH);
        for (i = 0; i < LCD_W; i++)
        {
            for (j = 0; j < LCD_H; j++)
            {
                os_spi_send(spi_dev_lcd, data, 2);
            }
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           display a point on the lcd.
 *
 * @param[in]       x       x position
 * @param[in]       y       y position
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_draw_point(os_uint16_t x, os_uint16_t y)
{
    lcd_address_set(x, y, x, y);
    lcd_write_half_word(FORE_COLOR);
}

/**
 ***********************************************************************************************************************
 * @brief           full color on the lcd.
 *
 * @param[in]       x_start       start of x position
 * @param[in]       y_start       start of y position
 * @param[in]       x_end         end of x position
 * @param[in]       y_end         end of y position
 * @param[in]       color         Fill color
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_fill(os_uint16_t x_start, os_uint16_t y_start, os_uint16_t x_end, os_uint16_t y_end, os_uint16_t color)
{
    os_uint16_t i = 0, j = 0;
    os_uint32_t size = 0, size_remain = 0;
    os_uint8_t *fill_buf = OS_NULL;

    size = (x_end - x_start) * (y_end - y_start) * 2;

    if (size > LCD_CLEAR_SEND_NUMBER)
    {
        /* the number of remaining to be filled */
        size_remain = size - LCD_CLEAR_SEND_NUMBER;
        size        = LCD_CLEAR_SEND_NUMBER;
    }

    lcd_address_set(x_start, y_start, x_end, y_end);

    fill_buf = (os_uint8_t *)os_calloc(1, size);
    if (fill_buf)
    {
        /* fast fill */
        while (1)
        {
            for (i = 0; i < size / 2; i++)
            {
                fill_buf[2 * i]     = color >> 8;
                fill_buf[2 * i + 1] = color;
            }
            os_pin_write(LCD_DC_PIN, PIN_HIGH);
            os_spi_send(spi_dev_lcd, fill_buf, size);

            /* Fill completed */
            if (size_remain == 0)
                break;

            /* calculate the number of fill next time */
            if (size_remain > LCD_CLEAR_SEND_NUMBER)
            {
                size_remain = size_remain - LCD_CLEAR_SEND_NUMBER;
            }
            else
            {
                size        = size_remain;
                size_remain = 0;
            }
        }
        os_free(fill_buf);
    }
    else
    {
        for (i = y_start; i <= y_end; i++)
        {
            for (j = x_start; j <= x_end; j++)
                lcd_write_half_word(color);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           display a line on the lcd.
 *
 * @param[in]       x1       x1 position
 * @param[in]       y1       y1 position
 * @param[in]       x2       x2 position
 * @param[in]       y2       y2 position
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_draw_line(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2)
{
    os_uint16_t t;
    os_uint32_t i    = 0;
    int         xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int         incx, incy, row, col;

    if (y1 == y2)
    {
        /* fast draw transverse line */
        lcd_address_set(x1, y1, x2, y2);

        os_uint8_t line_buf[480] = {0};

        for (i = 0; i < x2 - x1; i++)
        {
            line_buf[2 * i]     = FORE_COLOR >> 8;
            line_buf[2 * i + 1] = FORE_COLOR;
        }

        os_pin_write(LCD_DC_PIN, PIN_HIGH);
        os_spi_send(spi_dev_lcd, line_buf, (x2 - x1) * 2);

        return;
    }

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row     = x1;
    col     = y1;
    if (delta_x > 0)
        incx = 1;
    else if (delta_x == 0)
        incx = 0;
    else
    {
        incx    = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;
    else
    {
        incy    = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;
    for (t = 0; t <= distance + 1; t++)
    {
        lcd_draw_point(row, col);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           display a rectangle on the lcd.
 *
 * @param[in]       x1       x1 position
 * @param[in]       y1       y1 position
 * @param[in]       x2       x2 position
 * @param[in]       y2       y2 position

 * @return          void
 ***********************************************************************************************************************
 */
void lcd_draw_rectangle(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2)
{
    lcd_draw_line(x1, y1, x2, y1);
    lcd_draw_line(x1, y1, x1, y2);
    lcd_draw_line(x1, y2, x2, y2);
    lcd_draw_line(x2, y1, x2, y2);
}

/**
 ***********************************************************************************************************************
 * @brief           display a circle on the lcd.
 *
 * @param[in]       x       [The param_1 description.]
 * @param[in]       y       [The param_2 description.]
 * @param[out]      r       [The param_n description.]
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_draw_circle(os_uint16_t x0, os_uint16_t y0, os_uint8_t r)
{
    int a, b;
    int di;
    a  = 0;
    b  = r;
    di = 3 - (r << 1);
    while (a <= b)
    {
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 + b, y0 - a);
        lcd_draw_point(x0 - a, y0 + b);
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 - a, y0 - b);
        lcd_draw_point(x0 + b, y0 + a);
        lcd_draw_point(x0 + a, y0 - b);
        lcd_draw_point(x0 + a, y0 + b);
        lcd_draw_point(x0 - b, y0 + a);
        a++;
        /* Bresenham */
        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
        lcd_draw_point(x0 + a, y0 + b);
    }
}

static void lcd_show_char(os_uint16_t x, os_uint16_t y, os_uint8_t data, os_uint32_t size)
{
    os_uint8_t temp;
    os_uint8_t num = 0;
    os_uint8_t  pos, t;
    os_uint16_t colortemp = FORE_COLOR;
    os_uint8_t *font_buf  = OS_NULL;

    if (x > LCD_W - size / 2 || y > LCD_H - size)
        return;

    data = data - ' ';
#ifdef ASC2_1608
    if (size == 16)
    {
        lcd_address_set(x, y, x + size / 2 - 1, y + size - 1); /* (x,y,x+8-1,y+16-1) */

        font_buf = (os_uint8_t *)os_calloc(1, size * size);
        if (!font_buf)
        {
            /* fast show char */
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(os_uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    lcd_write_half_word(colortemp);
                    temp <<= 1;
                }
            }
        }
        else
        {
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(os_uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    font_buf[2 * (8 * pos + t)]     = colortemp >> 8;
                    font_buf[2 * (8 * pos + t) + 1] = colortemp;
                    temp <<= 1;
                }
            }
            os_pin_write(LCD_DC_PIN, PIN_HIGH);
            os_spi_send(spi_dev_lcd, font_buf, size * size);
            os_free(font_buf);
        }
    }
    else
#endif

#ifdef ASC2_2412
        if (size == 24)
    {
        lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

        font_buf = (os_uint8_t *)os_calloc(1, size * size);
        if (!font_buf)
        {
            /* fast show char */
            for (pos = 0; pos < (size * 16) / 8; pos++)
            {
                temp = asc2_2412[(os_uint16_t)data * (size * 16) / 8 + pos];
                if (pos % 2 == 0)
                {
                    num = 8;
                }
                else
                {
                    num = 4;
                }

                for (t = 0; t < num; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    lcd_write_half_word(colortemp);
                    temp <<= 1;
                }
            }
        }
        else
        {
            for (pos = 0; pos < (size * 16) / 8; pos++)
            {
                temp = asc2_2412[(os_uint16_t)data * (size * 16) / 8 + pos];
                if (pos % 2 == 0)
                {
                    num = 8;
                }
                else
                {
                    num = 4;
                }

                for (t = 0; t < num; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    if (num == 8)
                    {
                        font_buf[2 * (12 * (pos / 2) + t)]     = colortemp >> 8;
                        font_buf[2 * (12 * (pos / 2) + t) + 1] = colortemp;
                    }
                    else
                    {
                        font_buf[2 * (8 + 12 * (pos / 2) + t)]     = colortemp >> 8;
                        font_buf[2 * (8 + 12 * (pos / 2) + t) + 1] = colortemp;
                    }
                    temp <<= 1;
                }
            }
            os_pin_write(LCD_DC_PIN, PIN_HIGH);
            os_spi_send(spi_dev_lcd, font_buf, size * size);
            os_free(font_buf);
        }
    }
    else
#endif

#ifdef ASC2_3216
        if (size == 32)
    {
        lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

        font_buf = (os_uint8_t *)os_calloc(1, size * size);
        if (!font_buf)
        {
            /* fast show char */
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_3216[(os_uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    lcd_write_half_word(colortemp);
                    temp <<= 1;
                }
            }
        }
        else
        {
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_3216[(os_uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)
                        colortemp = FORE_COLOR;
                    else
                        colortemp = BACK_COLOR;
                    font_buf[2 * (8 * pos + t)]     = colortemp >> 8;
                    font_buf[2 * (8 * pos + t) + 1] = colortemp;
                    temp <<= 1;
                }
            }
            os_pin_write(LCD_DC_PIN, PIN_HIGH);
            os_spi_send(spi_dev_lcd, font_buf, size * size);
            os_free(font_buf);
        }
    }
    else
#endif
    {
        LOG_E(DBG_TAG,"There is no any define ASC2_1208 && ASC2_2412 && ASC2_2416 && ASC2_3216 !");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           display the number on the lcd.
 *
 * @param[in]       x         x position
 * @param[in]       y         y position
 * @param[in]       num       number
 * @param[in]       len       length of number
 * @param[in]       size      size of font
 *
 * @return          void
 ***********************************************************************************************************************
 */
void lcd_show_num(os_uint16_t x, os_uint16_t y, os_uint32_t num, os_uint8_t len, os_uint32_t size)
{
    lcd_show_string(x, y, size, "%d", num);
}

/**
 ***********************************************************************************************************************
 * @brief           display the string on the lcd.
 *
 * @param[in]       x       x position
 * @param[in]       y       y position
 * @param[in]       size    size of font
 * @param[in]       p       the string to be display
 *
 * @return          void
 ***********************************************************************************************************************
 */
os_err_t lcd_show_string(os_uint16_t x, os_uint16_t y, os_uint32_t size, const char *fmt, ...)
{
#define LCD_STRING_BUF_LEN 128

    va_list     args;
    os_uint8_t  buf[LCD_STRING_BUF_LEN] = {0};
    os_uint8_t *p                       = OS_NULL;

    if (size != 16 && size != 24 && size != 32)
    {
        LOG_E(DBG_TAG,"font size(%d) is not support!", size);
        return OS_ERROR;
    }

    va_start(args, fmt);
    os_vsnprintf((char *)buf, 100, (const char *)fmt, args);
    va_end(args);

    p = buf;
    while (*p != '\0')
    {
        if (x > LCD_W - size / 2)
        {
            x = 0;
            y += size;
        }
        if (y > LCD_H - size)
        {
            y = x = 0;
            lcd_clear(RED);
        }
        lcd_show_char(x, y, *p, size);
        x += size / 2;
        p++;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           display the image on the lcd.
 *
 * @param[in]       x            x position
 * @param[in]       y            y position
 * @param[in]       length       length of image
 * @param[in]       wide         wide of image
 * @param[in]       p            image
 *
 * @return          os_err_t
 * @retval          OS_EOK       run successfully
 * @retval          OS_ERROR     failed
 ***********************************************************************************************************************
 */
os_err_t lcd_show_image(os_uint16_t x, os_uint16_t y, os_uint16_t length, os_uint16_t wide, const os_uint8_t *p)
{
    OS_ASSERT(p);

    if (x + length > LCD_W || y + wide > LCD_H)
    {
        return OS_ERROR;
    }

    lcd_address_set(x, y, x + length - 1, y + wide - 1);

    os_pin_write(LCD_DC_PIN, PIN_HIGH);
    os_spi_send(spi_dev_lcd, p, length * wide * 2);

    return OS_EOK;
}

