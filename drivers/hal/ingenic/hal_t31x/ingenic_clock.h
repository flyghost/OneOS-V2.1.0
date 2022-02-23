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
 * @file        ingenic_clock.h
 *
 * @brief       This file provides clock functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#ifndef INGENIC_CLOCK_H_
#define INGENIC_CLOCK_H_

#include <stdint.h>
#include <t31.h>
/*
 * Clock setting
 */
#define BOARD_EXTAL_CLK     24000000
#define BOARD_RTC_CLK       32768
#define BOARD_CPU_CLK       (1008 * 1000 * 1000UL)

#define BOARD_APLL_FREQ     1008000000  /*If APLL not use mast be set 0*/
#define BOARD_MPLL_FREQ     600000000   /*If MPLL not use mast be set 0*/


#define CLK_NAME_EXT0       "ext0"
#define CLK_NAME_EXT1       "ext1"
#define CLK_NAME_OTGPHY     "otg_phy"
#define CLK_NAME_APLL       "apll"
#define CLK_NAME_MPLL       "mpll"
#define CLK_NAME_SCLKA      "sclka"
#define CLK_NAME_CCLK       "cclk"
#define CLK_NAME_L2CLK      "l2clk"
#define CLK_NAME_H0CLK      "h0clk"
#define CLK_NAME_H2CLK      "h2clk"
#define CLK_NAME_PCLK       "pclk"
#define CLK_NAME_MSC        "msc"
#define CLK_NAME_CGU_PCM1   "cgu_pcm1"
#define CLK_NAME_CGU_PCM    "cgu_pcm"
#define CLK_NAME_CGU_CIM    "cgu_cim"
#define CLK_NAME_CGU_SFC    "cgu_ssi"
#define CLK_NAME_CGU_MSC_MUX    "cgu_msc_mux"
#define CLK_NAME_CGU_USB    "cgu_usb"
#define CLK_NAME_CGU_MSC1   "cgu_msc1"
#define CLK_NAME_CGU_MSC0   "cgu_msc0"
#define CLK_NAME_CGU_LCD    "cgu_lcd"
#define CLK_NAME_CGU_I2S1   "cgu_i2s1"
#define CLK_NAME_CGU_I2S    "cgu_i2s"
#define CLK_NAME_CGU_MACPHY "cgu_macphy"
#define CLK_NAME_CGU_DDR    "cgu_ddr"
#define CLK_NAME_DDR        "ddr"
#define CLK_NAME_CPU        "cpu"
#define CLK_NAME_AHB0       "ahb0"
#define CLK_NAME_APB0       "apb0"
#define CLK_NAME_RTC        "rtc"
#define CLK_NAME_PCM        "pcm"
#define CLK_NAME_MAC        "mac"
#define CLK_NAME_AES        "aes"
#define CLK_NAME_LCD        "lcd"
#define CLK_NAME_CIM        "cim"
#define CLK_NAME_PDMA       "pdma"
#define CLK_NAME_SYS_OST    "sys_ost"
#define CLK_NAME_SSI        "ssi0"
#define CLK_NAME_TCU        "tcu"
#define CLK_NAME_DMIC       "dmic"
#define CLK_NAME_UART2      "uart2"
#define CLK_NAME_UART1      "uart1"
#define CLK_NAME_UART0      "uart0"
#define CLK_NAME_SADC       "sadc"
#define CLK_NAME_VPU        "vpu"
#define CLK_NAME_AIC        "aic"
#define CLK_NAME_I2C3       "i2c3"
#define CLK_NAME_I2C2       "i2c2"
#define CLK_NAME_I2C1       "i2c1"
#define CLK_NAME_I2C0       "i2c0"
#define CLK_NAME_SCC        "scc"
#define CLK_NAME_MSC1       "msc1"
#define CLK_NAME_MSC0       "msc0"
#define CLK_NAME_OTG        "otg1"
#define CLK_NAME_SFC        "sfc"
#define CLK_NAME_EFUSE      "efuse"
#define CLK_NAME_NEMC       "nemc"


enum {
    CLK_ID_EXT     = 0,
    CLK_ID_EXT0,
    CLK_ID_EXT1,
    CLK_ID_OTGPHY,
    CLK_ID_PLL,
    CLK_ID_APLL,
    CLK_ID_MPLL,
    CLK_ID_SCLKA,
    CLK_ID_CPPCR,
    CLK_ID_CCLK,
    CLK_ID_L2CLK,
    CLK_ID_H0CLK,
    CLK_ID_H2CLK,
    CLK_ID_PCLK,
    CLK_ID_MSC,
    CLK_ID_CGU,
    CLK_ID_CGU_PCM1,
    CLK_ID_CGU_PCM,
    CLK_ID_CGU_CIM,
    CLK_ID_CGU_SFC,
    CLK_ID_CGU_MSC_MUX,
    CLK_ID_CGU_USB,
    CLK_ID_CGU_MSC1,
    CLK_ID_CGU_MSC0,
    CLK_ID_CGU_LCD,
    CLK_ID_CGU_I2S1,
    CLK_ID_CGU_I2S,
    CLK_ID_CGU_MACPHY,
    CLK_ID_CGU_DDR,
    CLK_ID_DEVICES,
    CLK_ID_DDR,
    CLK_ID_CPU,
    CLK_ID_AHB0,
    CLK_ID_APB0,
    CLK_ID_RTC,
    CLK_ID_PCM,
    CLK_ID_MAC,
    CLK_ID_AES,
    CLK_ID_LCD,
    CLK_ID_CIM,
    CLK_ID_PDMA,
    CLK_ID_SYS_OST,
    CLK_ID_SSI,
    CLK_ID_TCU,
    CLK_ID_DMIC,
    CLK_ID_UART2,
    CLK_ID_UART1,
    CLK_ID_UART0,
    CLK_ID_SADC,
    CLK_ID_VPU,
    CLK_ID_AIC,
    CLK_ID_I2C3,
    CLK_ID_I2C2,
    CLK_ID_I2C1,
    CLK_ID_I2C0,
    CLK_ID_SCC,
    CLK_ID_MSC1,
    CLK_ID_MSC0,
    CLK_ID_OTG,
    CLK_ID_SFC,
    CLK_ID_EFUSE,
    CLK_ID_NEMC,
    CLK_ID_STOP,
    CLK_ID_INVALID,
};

enum {
    CGU_PCM1,
    CGU_CIM,
    CGU_SFC,
    CGU_USB,
    CGU_MSC1,
    CGU_MSC0,
    CGU_LCD,
    CGU_MACPHY,
    CGU_DDR,
    CGU_MSC_MUX
};

enum {
    CDIV = 0,
    L2CDIV,
    H0DIV,
    H2DIV,
    PDIV,
    SCLKA
};

enum {
    CGU_AUDIO_I2S,
    CGU_AUDIO_I2S1,
    CGU_AUDIO_PCM,
    CGU_AUDIO_PCM1
};

#define GATE(x)  (((x)<<24) | CLK_FLG_GATE)
#define CPCCR(x) (((x)<<24) | CLK_FLG_CPCCR)
#define CGU(no)  (((no)<<24) | CLK_FLG_CGU)
#define CGU_AUDIO(no)  (((no)<<24) | CLK_FLG_CGU_AUDIO)
#define PLL(no)  (((no)<<24) | CLK_FLG_PLL)
#define PARENT(P)  (((CLK_ID_##P)<<16) | CLK_FLG_PARENT)
#define RELATIVE(P)  (((CLK_ID_##P)<<16) | CLK_FLG_RELATIVE)
#define DEF_CLK(N,FLAG)                     \
    [CLK_ID_##N] = { .name = CLK_NAME_##N, .flags = FLAG, }

#define CLK_FLG_NOALLOC     BIT(0)
#define CLK_FLG_ENABLE      BIT(1)
#define CLK_GATE_BIT(flg)   ((flg) >> 24)
#define CLK_FLG_GATE        BIT(2)
#define CLK_CPCCR_NO(flg)   (((flg) >> 24) & 0xff)
#define CLK_FLG_CPCCR       BIT(3)
#define CLK_CGU_NO(flg)     (((flg) >> 24) & 0xff)
#define CLK_FLG_CGU         BIT(4)
#define CLK_PLL_NO(flg)     (((flg) >> 24) & 0xff)
#define CLK_FLG_PLL         BIT(5)
#define CLK_CGU_AUDIO_NO(flg)   (((flg) >> 24) & 0xff)
#define CLK_FLG_CGU_AUDIO   BIT(6)
#define CLK_PARENT(flg)     (((flg) >> 16) & 0xff)
#define CLK_RELATIVE(flg)   (((flg) >> 16) & 0xff)
#define CLK_FLG_PARENT      BIT(7)
#define CLK_FLG_RELATIVE    BIT(8)


#define I2S_PRI_DIV 0xb0020030
#define PCM_PRI_DIV 0xb0030014

struct clk {
    const char *name;
    uint32_t rate;
    struct clk *parent;
    uint32_t flags;
    struct clk_ops *ops;
    int count;
    int init_state;
    struct clk *source;
    struct clk *child;
    unsigned int CLK_ID;
};

struct clk_ops {
    int             (*enable)           (struct clk *,int);
    struct clk*     (*get_parent)       (struct clk *);
    int             (*set_parent)       (struct clk *,struct clk *);
    uint32_t        (*get_rate)         (struct clk *);
    int             (*set_rate)         (struct clk *,uint32_t);
    int             (*set_round_rate)   (struct clk *,uint32_t);
};


struct clk *clk_get(const char *id);
int         clk_enable(struct clk *clk);
int         clk_is_enabled(struct clk *clk);
void        clk_disable(struct clk *clk);
uint32_t    clk_get_rate(struct clk *clk);
void        clk_put(struct clk *clk);
int         clk_set_rate(struct clk *clk, uint32_t rate);
struct clk *get_clk_from_id(int clk_id);
int 		init_all_clk(void);
int 		get_clk_sources_size(void);


#endif
