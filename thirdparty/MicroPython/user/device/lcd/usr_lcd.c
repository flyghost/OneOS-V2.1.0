#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <device.h>
#include <drv_cfg.h>
#include <os_clock.h>
#include "usr_lcd.h"
#include <st7789vw.h>



static int g_lcd_status = 0;
static int g_lcd_color = 0;


static int lcd_light(void *flag)
{
	if(NULL == flag)
	{
		return g_lcd_status;
	}
	else
	{
		if(0 == *(int *)(flag))
		{
			lcd_display_off();
			g_lcd_status = 0;
		}
		else
		{
			lcd_display_on();
			g_lcd_status = 1;
		}
	}
	return 0;
}

static int lcd_fill_full_screen(int color)
{
  lcd_fill(0,0,LCD_W,LCD_W, color);
	g_lcd_color = color;
	return 0;
}

static int lcd_clean(void)
{
	lcd_clear(g_lcd_color);
	return 0;
}

static int lcd_pxiel(os_uint16_t x_start, os_uint16_t y_start, os_uint16_t x_end, os_uint16_t y_end, os_uint16_t color)
{
	lcd_fill(x_start, y_start, x_end, y_end, color);
	return 0;
}

static int lcd_text(os_uint16_t x, os_uint16_t y, os_uint32_t size, const char *fmt)
{
	return lcd_show_string(x, y, size, fmt);
}

static int lcd_set_forecolor_backcolor(os_uint16_t back, os_uint16_t fore)
{
	lcd_set_color(back, fore);
	return 0;
}


static int lcd_open(const char *dev_name)
{
    int flag = 1;
    
    lcd_light(&flag);
    return 0;
}

static int lcd_close(const char *dev_name)
{
    int flag = 0;
    
    lcd_light(&flag);
    return 0;
}



static int lcd_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return MP_ERROR;
    }

    switch(cmd)
    {
    case IOCTL_LCD_CLEAR:
        lcd_clean();
        return 0;
    case IOCTL_LCD_LIGHT:
        return lcd_light(arg);
    case IOCTL_LCD_FILL:
        return lcd_fill_full_screen(*(int *)arg);
    case IOCTL_LCD_PIXEL:
        return lcd_pxiel(((lcd_fill_arg_t *)arg)->x, ((lcd_fill_arg_t *)arg)->y,
                            ((lcd_fill_arg_t *)arg)->w, ((lcd_fill_arg_t *)arg)->h,
                            ((lcd_fill_arg_t *)arg)->color);
    case IOCTL_LCD_TEXT:
        return lcd_text(((lcd_text_arg_t *)arg)->x, ((lcd_text_arg_t *)arg)->y, 
                            ((lcd_text_arg_t *)arg)->size, ((lcd_text_arg_t *)arg)->str);
    case IOCTL_LCD_COLOR:
        return lcd_set_forecolor_backcolor(((lcd_color_arg_t *)arg)->back_color, 
                                            ((lcd_color_arg_t *)arg)->fore_color);
    case IOCTL_LCD_SHOW_IMG:
        return lcd_show_image(((lcd_show_image_arg_t *)arg)->x,
                                ((lcd_show_image_arg_t *)arg)->y,
                                ((lcd_show_image_arg_t *)arg)->length,
                                ((lcd_show_image_arg_t *)arg)->wide,
                                ((lcd_show_image_arg_t *)arg)->p);
    default:
        mp_raise_ValueError("the cmd is wrong, please check!\n");
        return -1;
    }
}

static struct operate lcd_ops = {
    .open = lcd_open,
    .ioctl = lcd_ioctl,
    .close = lcd_close,
};


static int lcd_register(void)
{
	device_info_t * lcd = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == lcd)
	{
		os_kprintf("mpycall_lcd_register malloc mem failed!");
		return MP_ERROR;
	}
    memset(lcd, 0, sizeof(device_info_t));
	
	lcd->owner.name = "lcd";
	lcd->owner.type = DEV_PANEL;
	
	lcd->ops = &lcd_ops;
    
	mpycall_device_add(lcd);

	return MP_EOK;
}

OS_CMPOENT_INIT(lcd_register, OS_INIT_SUBLEVEL_LOW);




