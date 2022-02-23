#ifndef __USER_LCD_H__
#define __USER_LCD_H__

#include <sensors/sensor.h>

#define LCD_INIT_FLAG 				(1)
#define LCD_DEINIT_FLAG 			(0)


#define IOCTL_LCD_CLEAR              (0)
#define IOCTL_LCD_LIGHT              (1)
#define IOCTL_LCD_FILL               (2)
#define IOCTL_LCD_PIXEL              (3)
#define IOCTL_LCD_TEXT               (4)
#define IOCTL_LCD_COLOR              (5)
#define IOCTL_LCD_SHOW_IMG           (6)

#define LCD_W 240
#define LCD_H 240

/* POINT_COLOR */
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0XF81F
#define GRED    0XFFE0
#define GBLUE   0X07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0XBC40
#define BRRED   0XFC07
#define GRAY    0X8430
#define GRAY175 0XAD75
#define GRAY151 0X94B2
#define GRAY187 0XBDD7
#define GRAY240 0XF79E

struct lcd_fill_arg
{
   int x;
	 int y;
	 int w;
	 int h;
	 int color;
};
typedef struct lcd_fill_arg lcd_fill_arg_t;

struct lcd_text_arg
{
   int x;
	 int y;
	 int size;
   char *str;
};
typedef struct lcd_text_arg lcd_text_arg_t;

struct lcd_color_arg
{
   int fore_color;
	 int back_color;
};
typedef struct lcd_color_arg lcd_color_arg_t;

typedef struct lcd_show_image_arg {
    os_uint16_t x; 
    os_uint16_t y; 
    os_uint16_t length; 
    os_uint16_t wide; 
    const os_uint8_t *p;
} lcd_show_image_arg_t;

#endif
