#include "mpy_picture.h"

/* This means 8bit R remain 5bit, 8bit G remain 6bit, 8bit B remain 5bit */
/* [rrrrrrrr gggggggg bbbbbbbb] => [rrrrrggg gggbbbbb] */
static uint16_t mpy_pic_bmp_rgb888to565(uint32_t RGB) 
{
     int R, G, B; 
     R = (RGB >> 19) & 0x1F; 
     G = (RGB >> 10) & 0x3F; 
     B = (RGB >> 3) & 0x1F; 
     return (R << 11) | (G << 5) | B; 
} 

static int mpy_pic_read_and_show_bmp(void *dev, 
                                            mpy_pic_show_func show_func, 
                                            lcd_show_image_arg_t *show_info,
                                            int fd, 
                                            mpy_pic_bmp_head_info_t *head_info)
{
    void *image_buf = os_malloc(2 * head_info->width);
    void *row_buf = NULL;
    int image_index, row_index;
    uint16_t rgb565_temp;
    uint8_t pixel_size = 0;
    int len;
    int ret = 0;

    MP_LCD_GET_BMP_PIXEL_SIZE(head_info->bit_count, pixel_size);
    row_buf = os_malloc(pixel_size * head_info->width);
    if (row_buf == NULL || image_buf == NULL)
    {
        ret = -1;
        goto _error;
    }
    
    for(int i = 0; i < head_info->heigth; i++)
    {
        image_index = 0;
        row_index = 0;

        len = read(fd, row_buf, pixel_size * head_info->width);
        if (len < 0)
        {
            ret = -1;
            goto _error;
        }

        while(row_index < (pixel_size * head_info->width))
        {
            rgb565_temp = mpy_pic_bmp_rgb888to565(*(uint32_t *)((uint8_t *)row_buf + row_index));
            *(uint8_t *)((uint8_t *)image_buf + image_index) = (rgb565_temp >> 8);
            *(uint8_t *)((uint8_t *)image_buf + image_index + 1) = rgb565_temp & 0xff;
            
            row_index   += pixel_size;
            image_index += 2;
        }

        show_info->wide = 1;
        show_info->length = head_info->width;
        show_info->p = (const uint8_t *)image_buf;
        show_func(dev, IOCTL_LCD_SHOW_IMG, show_info);
        show_info->y--;
    }
    
_error:
    os_free(image_buf);
    os_free(row_buf);
    return ret;
}


static int mpy_pic_decode_bmp_head(int fd, mpy_pic_bmp_head_info_t *head_info)
{
    void *bmp_head_info = os_malloc(MPY_PIC_BMP_HEAD_INFO_SIZE);
    int len;
    int ret = 0;

    len = read(fd, bmp_head_info, MPY_PIC_BMP_HEAD_INFO_SIZE);
    if (len < 0)
    {
        ret = -1;
    }

    head_info->width  = *(uint32_t *)((uint8_t *)bmp_head_info + 18);
    head_info->heigth = *(uint32_t *)((uint8_t *)bmp_head_info + 22);
    head_info->bit_count = *(uint16_t *)((uint8_t *)bmp_head_info + 28);

    os_free(bmp_head_info);
    return ret;
}


int mpy_pic_show_bmp(mpy_pic_base_info_t *base_info)
{
    int fd = -1;
    int ret = -1;
    mpy_pic_bmp_head_info_t head_info = {0};
    lcd_show_image_arg_t show_info = {0};
    
    fd = open(base_info->pic_path, O_RDONLY, 0);
    if (fd < 0)
    {
         return -1;
    }
    
    /* 1.解析bmp文件头 */
    ret = mpy_pic_decode_bmp_head(fd, &head_info);
    if (ret < 0)
    {
        ret = -1;
        goto _error;
    }
    
    if (head_info.bit_count != 32 && head_info.bit_count != 24)
    {
        ret = head_info.bit_count;
        goto _error;
    }

    /* 2.解析bmp文件内容并显示 */
    show_info.x = base_info->x;
    show_info.y = base_info->y;
    ret = mpy_pic_read_and_show_bmp(base_info->dev, base_info->show_func, &show_info, fd, &head_info);
    if (ret < 0)
    {
        ret = -1;
        goto _error;
    }

    ret = 0;
_error:
    close(fd);
    return ret;
}


