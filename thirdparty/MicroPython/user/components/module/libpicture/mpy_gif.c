#if 0

#include <st7789vw.h>
#include "mpy_picture.h"

const uint16_t _aMaskTbl[16] =
{
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff,
    0x0fff, 0x1fff, 0x3fff, 0x7fff,
};
const uint8_t _aInterlaceOffset[] = {8, 8, 4, 2};
const uint8_t _aInterlaceYPos  [] = {0, 4, 2, 1};

static uint8_t mpy_pic_gif_decoding = 0; //1: 标记GIF正在解码, 0: 非解码状态.
extern os_err_t lcd_write_half_word(const os_uint16_t da);
/**
 * @brief    检测GIF头
 *
 * @param   file    文件
 *
 * @return  uint8_t        0,是GIF89a/87a;非零,非GIF89a/87a
 */
int mpy_pic_gif_check_head(int fd)
{
    uint8_t gif_version[6] = {0};
    uint32_t check_sum = 1;
    int ret = -1;
    
    ret = read(fd, gif_version, sizeof(gif_version));
    if (ret < 0)
    {
        mp_err("Read file failed[%d], when check head.", ret);
        return -1;
    }

    check_sum = ((gif_version[0] != 'G') || (gif_version[1] != 'I') || (gif_version[2] != 'F')) ||
                ((gif_version[3] != '8') || ((gif_version[4] != '7') && (gif_version[4] != '9')) || (gif_version[5] != 'a'));
    if(check_sum)
    {
        mp_err("Check head[%c, %c, %c, %c, %c, %c] failed.", 
                    gif_version[0], gif_version[1], gif_version[2],
                    gif_version[3], gif_version[4], gif_version[5]);
        return -1;
    }

    return 0;
}
/**
 * @brief    将RGB888转为RGB565
 *
 * @param   ctb        RGB888颜色数组首地址
 *
 * @return  uint8_t        RGB565颜色
 */
uint16_t mpy_pic_gif_get_rgb565(uint8_t *ctb)
{
    uint16_t r, g, b;
    r = (ctb[0] >> 3) & 0X1F;
    g = (ctb[1] >> 2) & 0X3F;
    b = (ctb[2] >> 3) & 0X1F;
    return b + (g << 5) + (r << 11);
}
/**
 * @brief    读取颜色表
 *
 * @param   file    文件
 * @param   gif        gif信息
 * @param   num        tbl大小
 *
 * @return  uint8_t        0,OK;其他,失败
 */
int mpy_pic_gif_read_colortbl(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl, uint16_t tbl_size)
{
    uint8_t rgb[3] = {0};
    uint16_t t = 0;
    int ret = -1;
    
    for(t = 0; t < tbl_size; t++)
    {
        ret = read(fd, rgb, 3);
        if(ret < 0)
        {
            mp_err("Read gif table size failed[%d].", ret);
            return -1;
        }

        gif_ctrl->cl_tbl[t] = mpy_pic_gif_get_rgb565(rgb);
    }
    return 0;
}
/**
 * @brief    得到逻辑屏幕描述,图像尺寸等
 *
 * @param   file    文件
 * @param   gif        gif信息
 *
 * @return  uint8_t        0,OK;其他,失败
 */
int mpy_pic_gif_get_ctrlinfo(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl)
{
    int ret;
    
    ret = read(fd, (uint8_t*)&gif_ctrl->lsd, sizeof(gif_ctrl->lsd));
    if(ret < 0)
    {
        mp_err("Read gif logic sreen descriptor failed[%d].", ret);
        return -1;
    }
    
    if(gif_ctrl->lsd.flag & 0x80) //存在全局颜色表
    {
        gif_ctrl->cl_tbl_size = 2 << (gif_ctrl->lsd.flag & 0x07); //得到颜色表大小
        if (gif_ctrl->cl_tbl_size > MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE) 
        {
            mp_err("Color table size[%d] is bigger than the maximum[%d].", 
                                    gif_ctrl->cl_tbl_size, MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE);
            return -1;
        }
        ret = mpy_pic_gif_read_colortbl(fd, gif_ctrl, gif_ctrl->cl_tbl_size);
        if (ret < 0)
        {
            mp_err("Read gif color table failed.");
            return -1; 
        }
    }
    return 0;
}
/**
 * @brief    保存全局颜色表
 *
 * @param   gif        gif信息
 *
 * @return  void
 */
static void mpy_pic_gif_save_cl_tbl(mpy_pic_gif_ctrl_info_t* gif_ctrl)
{
    uint16_t i = 0;
    for(i = 0; i < MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE; i++)
    {
        gif_ctrl->bk_cl_tbl[i] = gif_ctrl->cl_tbl[i]; //保存全局颜色.
    }
}
/**
 * @brief    恢复全局颜色表
 *
 * @param   gif_ctrl        gif控制块信息
 *
 * @return  void
 */
static void mpy_pic_gif_recoverg_cl_tbl(mpy_pic_gif_ctrl_info_t *gif_ctrl)
{
    uint16_t i = 0;
    for(i = 0; i < MPY_PIC_GIF_MAX_COLOR_TABLE_SIZE; i++)
    {
        gif_ctrl->cl_tbl[i] = gif_ctrl->bk_cl_tbl[i]; //恢复全局颜色.
    }
}
/**
 * @brief    初始化LZW相关参数
 *
 * @param   gif            gif信息
 * @param   codesize    lzw码长度
 *
 * @return  void
 */
static void mpy_pic_gif_initlzw(mpy_pic_gif_ctrl_info_t *gif_ctrl, uint8_t codesize)
{
    memset((uint8_t *)gif_ctrl->lzw, 0, sizeof(mpy_pic_gif_lzw_info_t));
    gif_ctrl->lzw->SetCodeSize  = codesize;
    gif_ctrl->lzw->CodeSize     = codesize + 1;
    gif_ctrl->lzw->ClearCode    = (1 << codesize);
    gif_ctrl->lzw->EndCode      = (1 << codesize) + 1;
    gif_ctrl->lzw->MaxCode      = (1 << codesize) + 2;
    gif_ctrl->lzw->MaxCodeSize  = (1 << codesize) << 1;
    gif_ctrl->lzw->ReturnClear  = 1;
    gif_ctrl->lzw->LastByte     = 2;
    gif_ctrl->lzw->sp           = gif_ctrl->lzw->aDecompBuffer;

    return;
}
/**
 * @brief    读取一个数据块
 *
 * @param   gfile        gif文件
 * @param   buf            数据缓存区
 * @param   maxnum        最大读写数据限制
 *
 * @return  void
 */
static int mpy_pic_gif_getdatablock(int fd, uint8_t *buf, uint16_t maxnum)
{
    uint8_t cnt;
    int ret = 0;
    
    ret = read(fd, &cnt, 1); //得到LZW长度
    if (ret < 0)
    {
        mp_err("Read data block failed[%d].", ret);
        return -1;
    }
    if(cnt)
    {
        if(buf) //需要读取
        {
            if(cnt > maxnum)
            {
                lseek(fd, cnt, SEEK_SET); //跳过
                return cnt;//直接不读
            }
            ret = read(fd, buf, cnt); //得到LZW长度
            if (ret < 0)
            {
                mp_err("Read data block failed[%d].", ret);
                return -1;
            }
        }
        else     //直接跳过
        {
            lseek(fd, cnt, SEEK_SET); //跳过
        }
    }
    
    return cnt;
}

/**
 * @brief   ReadExtension
 *          Purpose:
 *          Reads an extension block. One extension block can consist of several data blocks.
 *          If an unknown extension block occures, the routine failes.
 *
 * @return  uint8_t        0,OK;其他,失败
 */
static int mpy_pic_gif_readextension(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl, 
                                    int *pTransIndex, uint8_t *pDisposal)
{
    uint8_t temp;
    uint8_t buf[4];
    int ret = -1;
    
    ret = read(fd, &temp, 1); //得到长度
    if (ret < 0)
    {
        mp_err("Read file failed.");
        return -1;
    }
    switch(temp)
    {
    case MPY_PIC_GIF_PLAINTEXT:
    case MPY_PIC_GIF_APPLICATION:
    case MPY_PIC_GIF_COMMENT:
        while (mpy_pic_gif_getdatablock(fd, 0, 256) > 0);            //获取数据块
        return 0;
    case MPY_PIC_GIF_GRAPHICCTL://图形控制扩展块
        if (mpy_pic_gif_getdatablock(fd, buf, 4) != 4)
        { 
            mp_err("Get data block failed.");
            return -1;    //图形控制扩展块的长度必须为4
        }
        gif_ctrl->delay = (buf[2] << 8) | buf[1];                    //得到延时
        *pDisposal = (buf[0] >> 2) & 0x7;                     //得到处理方法
        if ((buf[0] & 0x1) != 0)
        { 
            *pTransIndex = buf[3];            //透明色表
        }
        read(fd, &temp, 1);             //得到LZW长度
        if (temp != 0) 
        {
            mp_err("Read file failed, temp[%d].", temp);
            return -1;                            //读取数据块结束符错误.
        }
        return 0;
    }
    
    return -1;//错误的数据
}

/**
 * @brief   从LZW缓存中得到下一个LZW码,每个码包含12位
 *
 * @param   gfile,gif       gif参数
 *
 * @return  uint8_t              <0,错误 其他,正常
 */
static int mpy_pic_gif_getnextcode(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl)
{
    int i, j, End;
    long Result;
    
    if(gif_ctrl->lzw->ReturnClear)
    {
        //The first code should be a clearcode.
        gif_ctrl->lzw->ReturnClear = 0;
        return gif_ctrl->lzw->ClearCode;
    }
    End = gif_ctrl->lzw->CurBit + gif_ctrl->lzw->CodeSize;
    if(End >= gif_ctrl->lzw->LastBit)
    {
        int Count;
        if (gif_ctrl->lzw->GetDone)
        {
            mp_err("lzw GetDone: %d.", gif_ctrl->lzw->GetDone);
            return -1; //Error
        }
        
        gif_ctrl->lzw->aBuffer[0] = gif_ctrl->lzw->aBuffer[gif_ctrl->lzw->LastByte - 2];
        gif_ctrl->lzw->aBuffer[1] = gif_ctrl->lzw->aBuffer[gif_ctrl->lzw->LastByte - 1];
        if((Count = mpy_pic_gif_getdatablock(fd, &gif_ctrl->lzw->aBuffer[2], 300)) == 0)
        {
            gif_ctrl->lzw->GetDone = 1;
        }
        
        if (Count < 0)
        {
            mp_err("Count[%d] is error.", Count);
            return -1; //Error
        }
        gif_ctrl->lzw->LastByte = 2 + Count;
        gif_ctrl->lzw->CurBit = (gif_ctrl->lzw->CurBit - gif_ctrl->lzw->LastBit) + 16;
        gif_ctrl->lzw->LastBit = (2 + Count) * 8;
        End = gif_ctrl->lzw->CurBit + gif_ctrl->lzw->CodeSize;
    }
    j = End >> 3;
    i = gif_ctrl->lzw->CurBit >> 3;
    if (i == j)
    {
        Result = (long)gif_ctrl->lzw->aBuffer[i];
    }
    else if (i + 1 == j)
    {
        Result = (long)gif_ctrl->lzw->aBuffer[i] | ((long)gif_ctrl->lzw->aBuffer[i + 1] << 8);
    }
    else 
    {
        Result = (long)gif_ctrl->lzw->aBuffer[i] | ((long)gif_ctrl->lzw->aBuffer[i + 1] << 8) | ((long)gif_ctrl->lzw->aBuffer[i + 2] << 16);
    }
    Result = (Result >> (gif_ctrl->lzw->CurBit & 0x7)) & _aMaskTbl[gif_ctrl->lzw->CodeSize];
    gif_ctrl->lzw->CurBit += gif_ctrl->lzw->CodeSize;
    return (int)Result;
}

/**
 * @brief   得到LZW的下一个码
 *
 * @param   gfile,gif   gif参数
 *
 * @return  int         <0,错误(-1,不成功;-2,读到结束符了)
 *                      >=0,OK.(LZW的第一个码)
 */
static int mpy_pic_gif_getnextbyte(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl)
{
    int i, Code, Incode;
    while((Code = mpy_pic_gif_getnextcode(fd, gif_ctrl)) >= 0)
    {
        if(Code == gif_ctrl->lzw->ClearCode)
        {
            //Corrupt GIFs can make this happen
            if (gif_ctrl->lzw->ClearCode >= (1 << MAX_NUM_LWZ_BITS))
            {
                mp_err("Clear code[%d] is bigger than the LWZ size[%d].", 
                        gif_ctrl->lzw->ClearCode, (1 << MAX_NUM_LWZ_BITS));
                return -1; //Error
            }
            //Clear the tables
            memset((uint8_t*)gif_ctrl->lzw->aCode, 0, sizeof(gif_ctrl->lzw->aCode));
            for(i = 0; i < gif_ctrl->lzw->ClearCode; ++i)
            {
                gif_ctrl->lzw->aPrefix[i] = i;
            }
            //Calculate the'special codes' independence of the initial code size
            //and initialize the stack pointer
            gif_ctrl->lzw->CodeSize = gif_ctrl->lzw->SetCodeSize + 1;
            gif_ctrl->lzw->MaxCodeSize = gif_ctrl->lzw->ClearCode << 1;
            gif_ctrl->lzw->MaxCode = gif_ctrl->lzw->ClearCode + 2;
            gif_ctrl->lzw->sp = gif_ctrl->lzw->aDecompBuffer;
            //Read the first code from the stack after clear ingand initializing*/
            do {
                gif_ctrl->lzw->FirstCode = mpy_pic_gif_getnextcode(fd, gif_ctrl);
            } while(gif_ctrl->lzw->FirstCode == gif_ctrl->lzw->ClearCode);
            gif_ctrl->lzw->OldCode = gif_ctrl->lzw->FirstCode;
            return gif_ctrl->lzw->FirstCode;
        }
        if(Code == gif_ctrl->lzw->EndCode)
        {
            mp_log("Gif data read over.")
            return 2; //End code
        }
        Incode = Code;
        if(Code >= gif_ctrl->lzw->MaxCode)
        {
            *(gif_ctrl->lzw->sp)++ = gif_ctrl->lzw->FirstCode;
            Code = gif_ctrl->lzw->OldCode;
        }
        while(Code >= gif_ctrl->lzw->ClearCode)
        {
            *(gif_ctrl->lzw->sp)++ = gif_ctrl->lzw->aPrefix[Code];
            if(Code == gif_ctrl->lzw->aCode[Code])
            {
                return Code;
            }
            if((gif_ctrl->lzw->sp - gif_ctrl->lzw->aDecompBuffer) >= sizeof(gif_ctrl->lzw->aDecompBuffer))
            {
                return Code;
            }
            Code = gif_ctrl->lzw->aCode[Code];
        }
        *(gif_ctrl->lzw->sp)++ = gif_ctrl->lzw->FirstCode = gif_ctrl->lzw->aPrefix[Code];
        if((Code = gif_ctrl->lzw->MaxCode) < (1 << MAX_NUM_LWZ_BITS))
        {
            gif_ctrl->lzw->aCode[Code] = gif_ctrl->lzw->OldCode;
            gif_ctrl->lzw->aPrefix[Code] = gif_ctrl->lzw->FirstCode;
            ++gif_ctrl->lzw->MaxCode;
            if((gif_ctrl->lzw->MaxCode >= gif_ctrl->lzw->MaxCodeSize) && (gif_ctrl->lzw->MaxCodeSize < (1 << MAX_NUM_LWZ_BITS)))
            {
                gif_ctrl->lzw->MaxCodeSize <<= 1;
                ++gif_ctrl->lzw->CodeSize;
            }
        }
        gif_ctrl->lzw->OldCode = Incode;
        if(gif_ctrl->lzw->sp > gif_ctrl->lzw->aDecompBuffer)
        {
            return *--(gif_ctrl->lzw->sp);
        }
    }
    return Code;
}

/**
 *         DispGIFImage
 *            Purpose:
 *               This routine draws a GIF image from the current pointer which should point to a
 *               valid GIF data block. The size of the desired image is given in the image descriptor.
 *            Return value:
 *              0 if succeed
 *              1 if not succeed
 *            Parameters:
 *              pDescriptor  - Points to a IMAGE_DESCRIPTOR structure, which contains infos about size, colors and interlacing.
 *              x0, y0       - Obvious.
 *              Transparency - Color index which should be treated as transparent.
 *              Disposal     - Contains the disposal method of the previous image. If Disposal == 2, the transparent pixels
 *                             of the image are rendered with the background color.
 */
int mpy_pic_gif_displayimage(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl, 
                            uint16_t x0, uint16_t y0, 
                            int Transparency, uint8_t Disposal)
{
    uint8_t lzwlen;
    int Index, OldIndex, XPos, YPos, YCnt, Pass, Interlace, XEnd;
    int Width, Height, Cnt, ColorIndex;
    uint16_t bkcolor;
    uint16_t *pTrans;
    int ret = -1;

    Width = gif_ctrl->isd.wd;
    Height = gif_ctrl->isd.ht;
    XEnd = Width + x0 - 1;
    bkcolor = gif_ctrl->cl_tbl[gif_ctrl->lsd.bk_index];
    pTrans = (uint16_t*)gif_ctrl->cl_tbl;
    ret = read(fd, &lzwlen, 1); //得到LZW长度
    if (ret < 0)
    {
        mp_err("Read file failed[%d] when get LZW length.", ret);
        return ret;
    }
    
    mpy_pic_gif_initlzw(gif_ctrl, lzwlen); //Initialize the LZW stack with the LZW code size
    Interlace = gif_ctrl->isd.flag & 0x40; //是否交织编码
    for (YCnt = 0, YPos = y0, Pass = 0; YCnt < Height; YCnt++)
    {
        Cnt = 0;
        OldIndex = -1;
        for(XPos = x0; XPos <= XEnd; XPos++)
        {
            if(gif_ctrl->lzw->sp > gif_ctrl->lzw->aDecompBuffer)
            {
                Index = *(--(gif_ctrl->lzw->sp));
            }
            else 
            {
                Index = mpy_pic_gif_getnextbyte(fd, gif_ctrl);
            }
            if(Index == -2)return 0; //Endcode
            if((Index < 0) || (Index >= gif_ctrl->cl_tbl_size))
            {
                //IfIndex out of legal range stop decompressing
                mp_err("Index[%d] of color table is outof range.", Index);
                return -1;//Error
            }
            //If current index equals old index increment counter
            if ((Index == OldIndex) && (XPos <= XEnd))
            {
                Cnt++;
            }
            else
            {
                if (Cnt)
                {
                    if(OldIndex != Transparency)
                    {
                        lcd_draw_line(XPos - Cnt - 1, YPos, Cnt + 1, *(pTrans + OldIndex));
                    }
                    else if(Disposal == 2)
                    {
                        lcd_draw_line(XPos - Cnt - 1, YPos, Cnt + 1, bkcolor);
                    }
                    Cnt = 0;
                }
                else
                {
                    if(OldIndex >= 0)
                    {
                        if (OldIndex != Transparency)
                        {
                            lcd_address_set(XPos - 1, YPos, XPos - 1, YPos);
                            lcd_write_half_word(*(pTrans + OldIndex));
                        }
                        else if (Disposal == 2) 
                        {
                            lcd_address_set(XPos - 1, YPos, XPos - 1, YPos);
                            lcd_write_half_word(bkcolor);
                        }
                    }
                }
            }
            OldIndex = Index;
        }
        if((OldIndex != Transparency) || (Disposal == 2))
        {
            if(OldIndex != Transparency)ColorIndex = *(pTrans + OldIndex);
            else ColorIndex = bkcolor;
            if(Cnt)
            {
                lcd_draw_line(XPos - Cnt - 1, YPos, Cnt + 1, ColorIndex);
            }
            else {
                lcd_address_set(XEnd, YPos, XEnd, YPos);
                lcd_write_half_word(ColorIndex);
            }
        }
        //Adjust YPos if image is interlaced
        if(Interlace)//交织编码
        {
            YPos += _aInterlaceOffset[Pass];
            if((YPos - y0) >= Height)
            {
                ++Pass;
                YPos = _aInterlaceYPos[Pass] + y0;
            }
        }
        else YPos++;
    }
    return 0;
}

/**
 * @brief   恢复成背景色
 *
 * @param   x,y     坐标
 * @param   gif     gif信息
 * @param   pimge   图像描述块信息
 *
 * @return  void
 */
static void mpy_pic_gif_clear2bkcolor(uint16_t x, uint16_t y,
                            mpy_pic_gif_ctrl_info_t *gif_ctrl, 
                            mpy_pic_gif_isd_t *isd)
{
    uint16_t x0, y0, x1, y1;
    uint16_t color = gif_ctrl->cl_tbl[gif_ctrl->lsd.bk_index];

    if(isd->wd == 0 || isd->ht == 0)
    {
        return; //直接不用清除了,原来没有图像!!
    }
    
    if(gif_ctrl->isd.y_off > isd->y_off)
    {
        x0 = x + isd->x_off;
        y0 = y + isd->y_off;
        x1 = x + isd->x_off + isd->wd - 1;
        y1 = y + gif_ctrl->isd.y_off - 1;
        if(x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
        {
            lcd_fill(x0, y0, x1, y1, color); //设定xy,的范围不能太大.
        }
    }
    
    if(gif_ctrl->isd.x_off > isd->x_off)
    {
        x0 = x + isd->x_off;
        y0 = y + isd->y_off;
        x1 = x + gif_ctrl->isd.x_off - 1;
        y1 = y + isd->y_off + isd->ht - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
        {
            lcd_fill(x0, y0, x1, y1, color); //设定xy,的范围不能太大.
        }
    }
    
    if((gif_ctrl->isd.y_off + gif_ctrl->isd.ht) < (isd->y_off + isd->ht))
    {
        x0 = x + isd->x_off;
        y0 = y + gif_ctrl->isd.y_off + gif_ctrl->isd.ht - 1;
        x1 = x + isd->x_off + isd->wd - 1;
        y1 = y + isd->y_off + isd->ht - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
        {
            lcd_fill(x0, y0, x1, y1, color); //设定xy,的范围不能太大.
        }
    }
    
    if((gif_ctrl->isd.x_off + gif_ctrl->isd.wd) < (isd->x_off + isd->wd))
    {
        x0 = x + gif_ctrl->isd.x_off + gif_ctrl->isd.wd - 1;
        y0 = y + isd->y_off;
        x1 = x + isd->x_off + isd->wd - 1;
        y1 = y + isd->y_off + isd->ht - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
        {
            lcd_fill(x0, y0, x1, y1, color); //设定xy,的范围不能太大.
        }
    }

    return;
}

/**
 * @brief   画GIF图像的一帧
 *
 * @param   gfile   gif文件.
 * @param   x0,y0   开始显示的坐标
 *
 * @return  void
 */
int mpy_pic_gif_drawimage(int fd, mpy_pic_gif_ctrl_info_t *gif_ctrl, mpy_pic_base_info_t *base_info)
{
    uint8_t temp;
    uint16_t cl_tbl_size = 0;
    mpy_pic_gif_isd_t tmp_isd = {0};
    uint8_t clear_bk = 0;
    int TransIndex;
    uint8_t introducer = 0;
    TransIndex = -1;
    int ret = -1;

    do {
        ret = read(fd, &introducer, 1); //读取一个字节
        if (ret < 0)
        {
            mp_err("Read file failed when draw image.");
            return -1;
        }
        switch(introducer)
        {
        case MPY_PIC_GIF_INTRO_IMAGE://图像描述
            tmp_isd.x_off = gif_ctrl->isd.x_off;
            tmp_isd.y_off = gif_ctrl->isd.y_off;
            tmp_isd.wd = gif_ctrl->isd.wd;
            tmp_isd.ht = gif_ctrl->isd.ht;

            ret = read(fd, (uint8_t *)&gif_ctrl->isd, sizeof(gif_ctrl->isd)); //读取一个字节
            if (ret)
            {
                return -1;
            }
            if (gif_ctrl->isd.flag & 0x80) //存在局部颜色表
            {
                mpy_pic_gif_save_cl_tbl(gif_ctrl);//保存全局颜色表
                cl_tbl_size = 2 << (gif_ctrl->isd.flag & 0X07); //得到局部颜色表大小
                if (mpy_pic_gif_read_colortbl(fd, gif_ctrl, cl_tbl_size))
                {
                    return -1; //读错误
                }
            }
            if (clear_bk == 2) 
            {
                mpy_pic_gif_clear2bkcolor(base_info->x, base_info->y, gif_ctrl, &tmp_isd);
            }
            
            mpy_pic_gif_displayimage(fd, gif_ctrl, base_info->x + gif_ctrl->isd.x_off, 
                            base_info->y + gif_ctrl->isd.y_off, TransIndex, clear_bk);
            while(1)
            {
                ret = read(fd, &temp, 1); //读取一个字节
                if (temp == 0)
                {
                    break;
                }

                if (lseek(fd, temp, SEEK_SET)) //继续向后偏移
                {
                    break; 
                }
            }
            if (temp != 0) 
            {
                mp_err("Read file failed when seek(%d).", temp);
                return -1; //Error
            }
            return 0;
        case MPY_PIC_GIF_INTRO_TERMINATOR://得到结束符了
            mp_log("Get the END code, draw Gif over.");
            return 2;//代表图像解码完成了.
        case MPY_PIC_GIF_INTRO_EXTENSION:
            //Read image extension*/
            ret = mpy_pic_gif_readextension(fd, gif_ctrl, &TransIndex, &clear_bk); //读取图像扩展块消息
            if (ret < 0)
            {
                return -1;
            }
            break;
        default:
            return -1;
        }
    }while(introducer != MPY_PIC_GIF_INTRO_TERMINATOR); //读到结束符了
    return 0;
}

/**
 * @brief   解码一个gif文件,本例子不能显示尺寸大与给定尺寸的gif图片!!!
 *
 * @param   filename            带路径的gif文件名字
 * @param   x,y,width,height    显示坐标及区域大小
 *
 * @return  uint8_t                  0,成功，其他,失败
 */
int mpy_pic_gif_decode(mpy_pic_base_info_t *base_info)
{
    int fd = -1;
    int ret = -1;
    uint16_t delay = 0; //解码延时
    mpy_pic_gif_ctrl_info_t *gif_ctrl = NULL;
    
    fd = open(base_info->pic_path, O_RDONLY, 0);
    if (fd < 0)
    {
        mp_err("Open file: %d failed.", base_info->pic_path);
         return -1;
    }

    /* 1.根据gif头检查是否为89a或者79a版本的GIF文件 */
    ret = mpy_pic_gif_check_head(fd);
    if (ret < 0)
    {
        ret = -1;
        goto _error;
    }

    /* 2.获取控制块参数 */
    gif_ctrl = os_malloc(sizeof(mpy_pic_gif_ctrl_info_t));
    if (gif_ctrl == NULL)
    {
        mp_err("No memory for gif ctrl info[%d].", sizeof(mpy_pic_gif_ctrl_info_t));
        ret = -1;
        goto _error;
    }
    
    gif_ctrl->lzw = os_malloc(sizeof(mpy_pic_gif_lzw_info_t));
    if (gif_ctrl->lzw == NULL)
    {
        mp_err("No memory for lzw[%d].", sizeof(mpy_pic_gif_lzw_info_t));
        ret = -1;
        goto _error;
    }
    
    ret = mpy_pic_gif_get_ctrlinfo(fd, gif_ctrl);
    if (ret < 0)
    {
        ret = -1;
        goto _error;
    }
    
    /* 3.逐帧显示 */
    if(gif_ctrl->lsd.wd > base_info->wd || gif_ctrl->lsd.ht > base_info->ht)
    {
        ret = -2; //尺寸太大.
        base_info->wd = gif_ctrl->lsd.wd;
        base_info->ht = gif_ctrl->lsd.ht;
    }
    else
    {
        base_info->x += (base_info->wd - gif_ctrl->lsd.wd) / 2;
        base_info->y += (base_info->ht - gif_ctrl->lsd.ht) / 2;
    }

    mpy_pic_gif_decoding = 1;
    while (mpy_pic_gif_decoding && ret == 0) //解码循环
    {
        ret = mpy_pic_gif_drawimage(fd, gif_ctrl, base_info);
        if (gif_ctrl->lsd.flag & 0x80) //存在全局颜色表
        {
            mpy_pic_gif_recoverg_cl_tbl(gif_ctrl); //恢复全局颜色表
        }
        delay = gif_ctrl->delay;
        if (delay == 0) 
        {
            delay = 10; //默认时延
        }

        while(delay-- && mpy_pic_gif_decoding)
        {
            os_task_tsleep(10); //延迟
        }

        if (ret == 2)
        {
            ret = 0;
            break;
        }
    }
_error:
    if (gif_ctrl->lzw != NULL)
    {
        os_free(gif_ctrl->lzw);
        gif_ctrl->lzw = NULL;
    }

    if (gif_ctrl != NULL)
    {
        os_free(gif_ctrl);
        gif_ctrl = NULL;
    }
    
    close(fd);
    return ret;
}
#endif

