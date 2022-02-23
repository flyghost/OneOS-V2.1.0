#ifndef __MPY_PICTURE_H__
#define __MPY_PICTURE_H__
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include "usr_misc.h"
#include "usr_lcd.h"

typedef int (*mpy_pic_show_func)(void *device, int cmd, void *arg);

typedef struct mpy_pic_base_info {
    uint16_t x; 
    uint16_t y;
    const char *pic_path;
    void *dev;
    mpy_pic_show_func show_func;
    uint16_t wd;        //  gif图片区域宽度
    uint16_t ht;        //  gif图片区域高度
} mpy_pic_base_info_t;

/**************bmp******************/
typedef struct mpy_pic_bmp_head_info {
    uint32_t width; 
    uint32_t heigth; 
    uint16_t bit_count;
} mpy_pic_bmp_head_info_t;

#define MP_LCD_32BIT_BMP_PIXEL_SIZE     4
#define MP_LCD_24BIT_BMP_PIXEL_SIZE     3
#define MP_LCD_GET_BMP_PIXEL_SIZE(bit_count, pixel_size)    \
    do{                                                     \
        switch(bit_count)                                   \
        {                                                   \
        case 32:                                            \
            pixel_size = MP_LCD_32BIT_BMP_PIXEL_SIZE;       \
            break;                                          \
        case 24:                                            \
            pixel_size = MP_LCD_24BIT_BMP_PIXEL_SIZE;       \
            break;                                          \
        }                                                   \
    }while(0)
#define MPY_PIC_BMP_HEAD_INFO_SIZE                          54

int mpy_pic_show_bmp(mpy_pic_base_info_t *base_info);

/**************bmp end******************/
#if 0
/**************gif******************/
#define MPY_PIC_GIF_INTRO_TERMINATOR ';'	//0X3B   GIF文件结束符
#define MPY_PIC_GIF_INTRO_EXTENSION  '!'    //0X21
#define MPY_PIC_GIF_INTRO_IMAGE      ','	//0X2C
#define MPY_PIC_GIF_COMMENT         0xFE
#define MPY_PIC_GIF_APPLICATION     0xFF
#define MPY_PIC_GIF_PLAINTEXT       0x01
#define MPY_PIC_GIF_GRAPHICCTL      0xF9

//逻辑屏幕描述块
typedef struct mpy_pic_gif_lsd
{
    uint16_t wd;      //GIF宽度
    uint16_t ht;     //GIF高度
    uint8_t flag;        //标识符  1:3:1:3=全局颜色表标志(1):颜色深度(3):分类标志(1):全局颜色表大小(3)
    uint8_t bk_index;    //背景色在全局颜色表中的索引(仅当存在全局颜色表时有效)
    uint8_t pix_ratio;    //像素宽高比
}mpy_pic_gif_lsd_t;

//图像描述块
typedef struct mpy_pic_gif_isd
{
    uint16_t x_off; //x方向偏移
    uint16_t y_off; //y方向偏移
    uint16_t wd;    //宽度
    uint16_t ht;    //高度
    uint8_t flag;   //标识符  1:1:1:2:3=局部颜色表标志(1):交织标志(1):保留(2):局部颜色表大小(3)
}mpy_pic_gif_isd_t;

#define MAX_NUM_LWZ_BITS 12
typedef struct mpy_pic_gif_lzw_info
{
    uint8_t    aBuffer[258];                     // Input buffer for data block 
    short aCode  [(1 << MAX_NUM_LWZ_BITS)]; // This array stores the LZW codes for the compressed strings 
    uint8_t    aPrefix[(1 << MAX_NUM_LWZ_BITS)]; // Prefix character of the LZW code.
    uint8_t    aDecompBuffer[3000];              // Decompression buffer. The higher the compression, the more bytes are needed in the buffer.
    uint8_t *sp;                               // Pointer into the decompression buffer 
    int   CurBit;
    int   LastBit;
    int   GetDone;
    int   LastByte;
    int   ReturnClear;
    int   CodeSize;
    int   SetCodeSize;
    int   MaxCode;
    int   MaxCodeSize;
    int   ClearCode;
    int   EndCode;
    int   FirstCode;
    int   OldCode;
}mpy_pic_gif_lzw_info_t;

#define MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE    256
typedef struct mpy_pic_gif_ctrl_info {
    mpy_pic_gif_lsd_t lsd;              //逻辑屏幕描述块
    mpy_pic_gif_isd_t isd;              //图像描述快
    uint16_t cl_tbl[MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE];                  //当前使用颜色表
    uint16_t bk_cl_tbl[MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE];               //备份颜色表.当存在局部颜色表时使用
    uint16_t cl_tbl_size;               //颜色表大小
    uint16_t delay;                     //延迟时间
    mpy_pic_gif_lzw_info_t *lzw;        //LZW信息
} mpy_pic_gif_ctrl_info_t;
/**************gif end******************/
#endif

#endif /* __MPY_PICTURE_H__ */

