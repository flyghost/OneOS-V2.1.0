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
 * @file        drv_lcd.c
 *
 * @brief       This file implements lcd driver
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <lcd.h>
#include <drv_lcd.h>
#include <string.h>

#define DBG_TAG "drv.lcd"
#include <dlog.h>

#define LCD_DISP_BUF_LEN    (10)
#define LCD_DISP_BUFF_SIZE  (LCD_DISP_BUF_LEN * 4)
#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

static const struct lcd_coord_map lcd_crd[] = LCD_CRD_MAP_INFO;

struct fm33_lcd
{
    struct os_lcd_device  lcd;
    LCD_HandleTypeDef    *hlcd;

    os_uint32_t           disp_buff[LCD_DISP_BUF_LEN];
};

static const struct lcd_num_map lcd_num_map_info[] =
{
    {.val = 0, .mask = MASK_ZERO},
    {.val = 1, .mask = MASK_ONE},
    {.val = 2, .mask = MASK_TWO},
    {.val = 3, .mask = MASK_THREE},
    {.val = 4, .mask = MASK_FOURE},
    {.val = 5, .mask = MASK_FIVE},
    {.val = 6, .mask = MASK_SIX},
    {.val = 7, .mask = MASK_SEVEN},
    {.val = 8, .mask = MASK_EIGHT},
    {.val = 9, .mask = MASK_NINE},
};

static const struct lcd_num_seg_map lcd_segs[] = 
{
    {"1A","1B","1C","1D","1E","1F","1G"},
    {"2A","2B","2C","2D","2E","2F","2G"},
    {"3A","3B","3C","3D","3E","3F","3G"},
    {"4A","4B","4C","4D","4E","4F","4G"},
    {"5A","5B","5C","5D","5E","5F","5G"},
    {"6A","6B","6C","6D","6E","6F","6G"},
    {"7A","7B","7C","7D","7E","7F","7G"},
    {"8A","8B","8C","8D","8E","8F","8G"},
};

static void hal_lcd_gpio_init(void)
{
    AnalogIO( GPIOA, GPIO_Pin_0 );
    AnalogIO( GPIOA, GPIO_Pin_1 );
    AnalogIO( GPIOA, GPIO_Pin_2 );
    AnalogIO( GPIOA, GPIO_Pin_3 );
    AnalogIO( GPIOA, GPIO_Pin_4 );
    AnalogIO( GPIOA, GPIO_Pin_5 );
    AnalogIO( GPIOA, GPIO_Pin_6 );
    AnalogIO( GPIOA, GPIO_Pin_7 );
    AnalogIO( GPIOA, GPIO_Pin_8 );
    AnalogIO( GPIOA, GPIO_Pin_9 );
    AnalogIO( GPIOA, GPIO_Pin_10 );
    AnalogIO( GPIOA, GPIO_Pin_11 );
    AnalogIO( GPIOA, GPIO_Pin_12 );

    AnalogIO( GPIOB, GPIO_Pin_5 );
    AnalogIO( GPIOB, GPIO_Pin_6 );
    AnalogIO( GPIOB, GPIO_Pin_7 );
    AnalogIO( GPIOB, GPIO_Pin_8 );
    AnalogIO( GPIOB, GPIO_Pin_9 );
    AnalogIO( GPIOB, GPIO_Pin_10 );
    AnalogIO( GPIOB, GPIO_Pin_11 );
    AnalogIO( GPIOB, GPIO_Pin_12 );
    AnalogIO( GPIOB, GPIO_Pin_13 );
    AnalogIO( GPIOB, GPIO_Pin_14 );
    AnalogIO( GPIOB, GPIO_Pin_15 );
}

void hal_lcd_com_seg_init(void)
{
    os_uint32_t com_seg_mask;

    RCC_PERCLK_SetableEx(LCDCLK, ENABLE);

    com_seg_mask = (
        LCD_COM_EN_COMEN0_Msk |
        LCD_COM_EN_COMEN1_Msk |
        LCD_COM_EN_COMEN2_Msk |
        LCD_COM_EN_COMEN3_Msk
        );
    LCD_COM_EN_Write(com_seg_mask);


    com_seg_mask = (
        LCD_SEG_EN0_SEGEN0_Msk  |
        LCD_SEG_EN0_SEGEN1_Msk  |
        LCD_SEG_EN0_SEGEN2_Msk  |
        LCD_SEG_EN0_SEGEN3_Msk  |
        LCD_SEG_EN0_SEGEN4_Msk  |
        LCD_SEG_EN0_SEGEN5_Msk  |
        LCD_SEG_EN0_SEGEN7_Msk  |
        LCD_SEG_EN0_SEGEN8_Msk  |
        LCD_SEG_EN0_SEGEN9_Msk  |
        LCD_SEG_EN0_SEGEN10_Msk |
        LCD_SEG_EN0_SEGEN11_Msk |
        LCD_SEG_EN0_SEGEN12_Msk |
        LCD_SEG_EN0_SEGEN13_Msk |
        LCD_SEG_EN0_SEGEN14_Msk |
        LCD_SEG_EN0_SEGEN15_Msk |
        LCD_SEG_EN0_SEGEN16_Msk |
        LCD_SEG_EN0_SEGEN17_Msk

        );
    LCD_SEG_EN0_Write(com_seg_mask);

    com_seg_mask = (
        LCD_SEG_EN1_SEGEN40_Msk |
        LCD_SEG_EN1_SEGEN41_Msk |
        LCD_SEG_EN1_SEGEN42_Msk |
        LCD_SEG_EN1_SEGEN43_Msk
        );
    LCD_SEG_EN1_Write(com_seg_mask);
}

static os_uint32_t hal_lcd_df_get(uint32_t LMUX, uint32_t WFT, uint32_t FreqWanted)
{
    uint8_t DivWFT;
    uint32_t DFResult;
    
    if(LCD_LCDSET_WFT_ATYPE == WFT)
        DivWFT = 2;
    else
        DivWFT = 4;
    
    if((FreqWanted > 0)&&(FreqWanted <= 100))
    {
        DFResult = (uint32_t)(32768.0/(float)((LMUX*2+4)*FreqWanted*DivWFT) + 0.5);
    }
    else
    {
        DFResult = 32;
    }
    
    return DFResult;
}

static void hal_lcd_module_init(os_uint8_t bias)
{
    os_uint32_t     DispBuf[10];
    LCD_InitTypeDef LCD_InitStruct;

    RCC_PERCLK_SetableEx(LCDCLK, ENABLE);

    memset(DispBuf, 0xFF, 10*4);
    LCD_DISPDATAx_Refresh(DispBuf);

    LCD_InitStruct.LMUX         = LCD_LCDSET_LMUX_4COM;
    LCD_InitStruct.ENMODE       = LCD_LCDDRV_ENMODE_INNERRESISTER;
    LCD_InitStruct.WFT          = LCD_LCDSET_WFT_ATYPE;
    LCD_InitStruct.DF           = hal_lcd_df_get(LCD_InitStruct.LMUX, LCD_InitStruct.WFT, 64);
    LCD_InitStruct.BIASMD       = LCD_LCDSET_BIASMD_3BIAS;
    LCD_InitStruct.SCFSEL       = LCD_LCDDRV_SCFSEL_X1;
    LCD_InitStruct.SC_CTRL      = LCD_LCDDRV_SC_CTRL_ONE;
    LCD_InitStruct.IC_CTRL      = LCD_LCDDRV_IC_CTRL_L3;
    LCD_InitStruct.LCDBIAS      = 8;
    LCD_InitStruct.ANTIPOLAR    = ENABLE;

    LCD_InitStruct.TEST         = DISABLE;
    LCD_InitStruct.DISPMD       = DISABLE;
       
    LCD_InitStruct.LCCTRL       = LCD_LCDTEST_LCCTRL_0;
    LCD_InitStruct.TESTEN       = DISABLE;
       
    LCD_InitStruct.FLICK        = DISABLE;
    LCD_InitStruct.TON          = 0;
    LCD_InitStruct.TOFF         = 0;
    LCD_InitStruct.DONIE        = DISABLE;
    LCD_InitStruct.DOFFIE       = DISABLE;
       
    LCD_InitStruct.LCDEN        = DISABLE;

    LCD_Init(&LCD_InitStruct);
}

static void hal_lcd_refresh_display(struct fm33_lcd *lcd)
{
    LCD_DISPDATAx_Refresh(lcd->disp_buff);
}

static const struct lcd_coord_map *get_sym_info(const char *name)
{
    os_int32_t                  i;
    const struct lcd_coord_map *p_info;
    
    for(i = 0; i < ITEM_NUM(lcd_crd); i++)
    {
        if (0 == strncmp(lcd_crd[i].name, name, OS_NAME_MAX))
        {
            p_info =  &lcd_crd[i];
            return p_info;
        }
    }
    return OS_NULL;
}

static os_err_t fm33_lcd_confiugre(struct os_lcd_device *lcd, struct lcd_configure *cfg)
{
    hal_lcd_gpio_init();
    hal_lcd_com_seg_init();

    hal_lcd_module_init(cfg->bias);

    LCD_DISPCTRL_LCDEN_Setable(ENABLE);

    return OS_EOK;
}

static os_err_t fm33_lcd_display_clear(struct os_lcd_device *dev)
{
    struct fm33_lcd   *fm_lcd;

    fm_lcd = os_container_of(dev, struct fm33_lcd, lcd);

    OS_ASSERT(fm_lcd != OS_NULL);

    memset(fm_lcd->disp_buff, 0x00, LCD_DISP_BUFF_SIZE);

    hal_lcd_refresh_display(fm_lcd);
    
    return OS_EOK;
}

static os_err_t fm33_lcd_display_full(struct os_lcd_device *dev)
{
    struct fm33_lcd   *fm_lcd;

    fm_lcd = os_container_of(dev, struct fm33_lcd, lcd);

    OS_ASSERT(fm_lcd != OS_NULL);

    memset(fm_lcd->disp_buff, 0xFF, LCD_DISP_BUFF_SIZE);

    hal_lcd_refresh_display(fm_lcd);

    return OS_EOK;
}

/* refer to the DISPDATAx register map for 4com*/
static os_err_t fm33_lcd_display_point(struct os_lcd_device *dev, const struct coord *crd, os_int8_t flag)
{
    struct fm33_lcd     *fm_lcd;
    os_uint32_t          rcom;
    os_uint32_t          rseg;

    fm_lcd = os_container_of(dev, struct fm33_lcd, lcd);
    OS_ASSERT(fm_lcd != OS_NULL);

    if(crd->seg > SEG31)
    {
        switch(crd->com)
        {
            case COM0:
                rcom = COM4;
                rseg = crd->seg - 32;
                break;
                
            case COM1:
                rcom = COM4;
                rseg = crd->seg - 32 + 12;
                break;
            case COM2:
                if(crd->seg < SEG40)
                {
                    rcom = COM4;
                    rseg = crd->seg - 32 + 24;
                }
                else
                {
                    rcom = COM5;
                    rseg = crd->seg - 32 - 8;
                }
                break;
            case COM3:
                {
                    rcom = COM5;
                    rseg = crd->seg - 32 + 4;
                }
                break;
        }
    }
    else
    {
        rcom = crd->com;
        rseg = crd->seg;
    }
    
    LOG_D(DBG_TAG, "crdmap: com%d -> com%d, seg%d -> seg%d", crd->com, rcom,crd->seg, rseg);
    
    if(flag <= 0)
    {
        fm_lcd->disp_buff[rcom] &= ~(1 << rseg);
    }
    else
    {
        fm_lcd->disp_buff[rcom] |= (1 << rseg);
    }
     hal_lcd_refresh_display(fm_lcd);

    return OS_EOK;
}

static os_err_t fm33_lcd_display_num(struct os_lcd_device *dev, os_uint32_t offset, os_int8_t val, os_int8_t flag)
{
    os_int8_t                     i;
    os_uint8_t                    bit = 0x80;
    const struct lcd_num_map     *num_info;
    const struct lcd_num_seg_map *seg_info;
    struct lcd_num_map            empty_num = {0, 0};
    const struct lcd_coord_map   *info;

    if((val < 0) || (val > 9))
    {
        LOG_W(DBG_TAG, "val [%d] beyond range 0~9.", val);
    }

    if(flag == OS_TRUE)
    {
        num_info = &lcd_num_map_info[val];
    }
    else
    {
        num_info = &empty_num;
    }

    seg_info = &lcd_segs[offset];
    for(i = 0; i < 7; i++)
    {
        switch(i)
        {
            case 0:
                info = get_sym_info(seg_info->seg_a);
                break;
            case 1:
                info = get_sym_info(seg_info->seg_b);
                break;
            case 2:
                info = get_sym_info(seg_info->seg_c);
                break;
            case 3:
                info = get_sym_info(seg_info->seg_d);
                break;
            case 4:
                info = get_sym_info(seg_info->seg_e);
                break;
            case 5:
                info = get_sym_info(seg_info->seg_f);
                break;
            case 6:
                info = get_sym_info(seg_info->seg_g);
                break;
            default:
                break;
        }

        if(info == OS_NULL)
         {
             os_kprintf("invalid char name[%s]\r\n",seg_info->seg_a);
             return OS_ERROR;
         }
        
        if((num_info->mask & bit) == bit)
        {
             fm33_lcd_display_point(dev, &(info->crd), OS_TRUE);
        }
        else
        {
            fm33_lcd_display_point(dev, &(info->crd), OS_FALSE);
        }
        
        bit = (bit >> 1);
    }

    return OS_EOK;
}


static os_err_t fm33_lcd_display_sym(struct os_lcd_device *dev, const char *name, os_int8_t flag)
{
    const struct lcd_coord_map *info;

    info = get_sym_info(name);
    if(info == OS_NULL)
    {
        os_kprintf("invalid char name[%s]\r\n", name);
        return OS_ERROR;
    }

    fm33_lcd_display_point(dev, &(info->crd), flag);

    return OS_EOK;
}

static os_err_t fm33_lcd_deinit(struct os_lcd_device *dev)
{
    LCD_DISPCTRL_LCDEN_Setable(DISABLE);

    LCD_Deinit();

    RCC_PERCLK_SetableEx(LCDCLK, DISABLE);
    return OS_EOK;
}

static const struct os_lcd_ops fm33_lcd_ops = {
    .configure          = fm33_lcd_confiugre,
    .deinit             = fm33_lcd_deinit,

    .display_clear      = fm33_lcd_display_clear,
    .display_full       = fm33_lcd_display_full,

    .display_num        = fm33_lcd_display_num,
    .display_sym        = fm33_lcd_display_sym,
};

static int fm33_lcd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;

    struct fm33_lcd *h_lcd = OS_NULL;
    h_lcd = os_calloc(1, sizeof(struct fm33_lcd)); 
    OS_ASSERT(h_lcd);

     h_lcd->hlcd = (LCD_HandleTypeDef *)dev->info;

     memset(h_lcd->disp_buff, 0x00, LCD_DISP_BUFF_SIZE);

    struct os_lcd_device *dev_lcd = &h_lcd->lcd;
    dev_lcd->ops       = &fm33_lcd_ops;

    result = os_lcd_register(dev_lcd, dev->name, OS_DEVICE_FLAG_RDWR, NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG, "%s register fialed!", dev->name);
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO fm33_lcd_driver = {
    .name   = "LCD_HandleTypeDef",
    .probe  = fm33_lcd_probe,
};

OS_DRIVER_DEFINE(fm33_lcd_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);
