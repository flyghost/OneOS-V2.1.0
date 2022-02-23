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
 * @file        ingenic_clock.c
 *
 * @brief       This file provides clock functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <ingenic_clock.h>


struct clk clk_srcs[] = {
    DEF_CLK(EXT0,       CLK_FLG_NOALLOC),
    DEF_CLK(EXT1,       CLK_FLG_NOALLOC),
    DEF_CLK(OTGPHY,         CLK_FLG_NOALLOC),
    DEF_CLK(APLL,       PLL(CPM_CPAPCR)),
    DEF_CLK(MPLL,       PLL(CPM_CPMPCR)),
    DEF_CLK(SCLKA,      CPCCR(SCLKA)),
    DEF_CLK(CCLK,       CPCCR(CDIV)),
    DEF_CLK(L2CLK,      CPCCR(L2CDIV)),
    DEF_CLK(H0CLK,      CPCCR(H0DIV)),
    DEF_CLK(H2CLK,      CPCCR(H2DIV)),
    DEF_CLK(PCLK,       CPCCR(PDIV)),
    DEF_CLK(NEMC,       GATE(0) | PARENT(H2CLK)),
    DEF_CLK(EFUSE,      GATE(1) | PARENT(H2CLK)),
    DEF_CLK(SFC,        GATE(2) | PARENT(CGU_SFC)),
    DEF_CLK(OTG,        GATE(3)),
    DEF_CLK(MSC0,       GATE(4) | PARENT(PCLK)),
    DEF_CLK(MSC1,       GATE(5) | PARENT(PCLK)),
    DEF_CLK(SCC,        GATE(6) | PARENT(PCLK)),
    DEF_CLK(I2C0,       GATE(7) | PARENT(PCLK)),
    DEF_CLK(I2C1,       GATE(8) | PARENT(PCLK)),
    DEF_CLK(I2C2,       GATE(9) | PARENT(PCLK)),
    DEF_CLK(I2C3,       GATE(10) | PARENT(PCLK)),
    DEF_CLK(AIC,        GATE(11)),
    DEF_CLK(VPU,        GATE(12) | PARENT(LCD)),
    DEF_CLK(SADC,       GATE(13)),
    DEF_CLK(UART0,      GATE(14) | PARENT(EXT1)),
    DEF_CLK(UART1,      GATE(15) | PARENT(EXT1)),
    DEF_CLK(UART2,      GATE(16) | PARENT(EXT1)),
    DEF_CLK(DMIC,       GATE(17)),
    DEF_CLK(TCU,        GATE(18)),
    DEF_CLK(SSI,        GATE(19)),
    DEF_CLK(SYS_OST,    GATE(20)),
    DEF_CLK(PDMA,       GATE(21)),
    DEF_CLK(CIM,        GATE(22) | PARENT(LCD)),
    DEF_CLK(LCD,        GATE(23)),
    DEF_CLK(AES,        GATE(24)),
    DEF_CLK(MAC,        GATE(25)),
    DEF_CLK(PCM,        GATE(26)),
    DEF_CLK(RTC,        GATE(27)),
    DEF_CLK(APB0,       GATE(28)),
    DEF_CLK(AHB0,       GATE(29)),
    DEF_CLK(CPU,        GATE(30)),
    DEF_CLK(DDR,        GATE(31)),
    DEF_CLK(CGU_MSC_MUX,    CGU(CGU_MSC_MUX)),
    DEF_CLK(CGU_PCM,    CGU_AUDIO(CGU_AUDIO_PCM)),
    DEF_CLK(CGU_CIM,    CGU(CGU_CIM)),
    DEF_CLK(CGU_SFC,    CGU(CGU_SFC)),
    DEF_CLK(CGU_USB,    CGU(CGU_USB)),
    DEF_CLK(CGU_MSC1,   CGU(CGU_MSC1)| PARENT(CGU_MSC_MUX)),
    DEF_CLK(CGU_MSC0,   CGU(CGU_MSC0)| PARENT(CGU_MSC_MUX)),
    DEF_CLK(CGU_LCD,    CGU(CGU_LCD)),
    DEF_CLK(CGU_I2S,    CGU_AUDIO(CGU_AUDIO_I2S)),
    DEF_CLK(CGU_MACPHY, CGU(CGU_MACPHY)),
    DEF_CLK(CGU_DDR,    CGU(CGU_DDR)),
#undef GATE
#undef CPCCR
#undef CGU
#undef CGU_AUDIO
#undef PARENT
#undef DEF_CLK
#undef RELATIVE
};

int get_clk_sources_size(void)
{
    return sizeof(clk_srcs)/sizeof(clk_srcs[0]);
}

struct clk *get_clk_from_id(int clk_id)
{
    return &clk_srcs[clk_id];
}

int get_clk_id(struct clk *clk)
{
    return (clk - &clk_srcs[0]);
}

static uint32_t pll_get_rate(struct clk *clk) 
{
    uint32_t offset;
    uint32_t cpxpcr;
    uint32_t m,n,od;
    uint32_t rate;


    if (clk->CLK_ID == CLK_ID_APLL)
        offset = 8;
    else if (clk->CLK_ID == CLK_ID_MPLL)
        offset = 7;
    else
        offset = 0;

    cpxpcr = cpm_inl(CLK_PLL_NO(clk->flags));
    if(cpxpcr >> offset & 1)
    {
        clk->flags |= CLK_FLG_ENABLE;
        m = ((cpxpcr >> 24) & 0x7f) + 1;
        n = ((cpxpcr >> 18) & 0x1f) + 1;
        od = ((cpxpcr >> 16) & 0x3);
        od = 1 << od;
        rate = clk->parent->rate * m / n / od;
    }
    else
    {
        clk->flags &= ~(CLK_FLG_ENABLE);
        rate = 0;
    }
    return rate;
}

static struct clk_ops clk_pll_ops = {
    .get_rate = pll_get_rate,
    .set_rate = NULL,
};

void init_ext_pll(struct clk *clk)
{
    switch (get_clk_id(clk))
    {
        case CLK_ID_EXT0:
            clk->rate = BOARD_RTC_CLK;
            clk->flags |= CLK_FLG_ENABLE;
            break;
        case CLK_ID_EXT1:
            clk->rate = BOARD_EXTAL_CLK;
            clk->flags |= CLK_FLG_ENABLE;
            break;
        case CLK_ID_OTGPHY:
            clk->rate = 48 * 1000 * 1000;
            clk->flags |= CLK_FLG_ENABLE;
            break;
        default:
            clk->parent = get_clk_from_id(CLK_ID_EXT1);
            clk->rate = pll_get_rate(clk);
            clk->ops = &clk_pll_ops;
            break;
    }
}

struct cpccr_clk
{
    uint16_t off,sel,ce;
};
static struct cpccr_clk cpccr_clks[] =
{
#define CPCCR_CLK(N,O,D,E)          \
    [N] = { .off = O, .sel = D, .ce = E}
    CPCCR_CLK(CDIV, 0, 28,22),
    CPCCR_CLK(L2CDIV, 4, 28,22),
    CPCCR_CLK(H0DIV, 8, 26,21),
    CPCCR_CLK(H2DIV, 12, 24,20),
    CPCCR_CLK(PDIV, 16, 24,20),
    CPCCR_CLK(SCLKA,-1, -1,30),
#undef CPCCR_CLK
};


static uint32_t cpccr_selector[4] = {0,CLK_ID_SCLKA,CLK_ID_MPLL,0};

static uint32_t cpccr_get_rate(struct clk *clk)
{
    int sel;
    uint32_t cpccr = cpm_inl(CPM_CPCCR);
    uint32_t rate;
    int v;
    if (CLK_CPCCR_NO(clk->flags) == SCLKA)
    {
        int clka_sel[4] =
        {
            0, CLK_ID_EXT1, CLK_ID_APLL, 0
        };
        sel = cpm_inl(CPM_CPCCR) >> 30;
        if (clka_sel[sel] == 0)
        {
            rate = 0;
            clk->flags &= ~CLK_FLG_ENABLE;
        }
        else
        {
            clk->parent = get_clk_from_id(clka_sel[sel]);
            rate = clk->parent->rate;
            clk->flags |= CLK_FLG_ENABLE;
        }
    }
    else
    {
        v = (cpccr >> cpccr_clks[CLK_CPCCR_NO(clk->flags)].off) & 0xf;
        sel = (cpccr >> (cpccr_clks[CLK_CPCCR_NO(clk->flags)].sel)) & 0x3;
        rate = get_clk_from_id(cpccr_selector[sel])->rate;
        rate = rate / (v + 1);
    }
    return rate;
}
static struct clk_ops clk_cpccr_ops =
{
    .get_rate = cpccr_get_rate,
    .set_rate = NULL,
};

void init_cpccr_clk(struct clk *clk)
{
    int sel;                
    uint32_t cpccr = cpm_inl(CPM_CPCCR);
    if (CLK_CPCCR_NO(clk->flags) != SCLKA)
    {
        sel = (cpccr >> cpccr_clks[CLK_CPCCR_NO(clk->flags)].sel) & 0x3;
        if (cpccr_selector[sel] != 0)
        {
            clk->parent = get_clk_from_id(cpccr_selector[sel]);
            clk->flags |= CLK_FLG_ENABLE;
        }
        else
        {
            clk->parent = NULL;
            clk->flags &= ~CLK_FLG_ENABLE;
        }
    }
    clk->rate = cpccr_get_rate(clk);
    clk->ops = &clk_cpccr_ops;
}

struct clk_selectors
{
    uint16_t route[4];
};

enum {
    SELECTOR_A = 0,
    SELECTOR_2,
    SELECTOR_C,
    SELECTOR_3,
    SELECTOR_MSC_MUX,
    SELECTOR_F,
    SELECTOR_G,
};

const struct clk_selectors selector[] = {
#define CLK(X)  CLK_ID_##X
    [SELECTOR_A].route = {CLK(STOP),CLK(SCLKA),CLK(MPLL),CLK(INVALID)},
    [SELECTOR_2].route  = {CLK(SCLKA),CLK(SCLKA),CLK(MPLL),CLK(MPLL)},
    [SELECTOR_C].route = {CLK(EXT1) ,CLK(EXT1),CLK(SCLKA),CLK(MPLL)},
    [SELECTOR_3].route = {CLK(SCLKA),CLK(MPLL),CLK(EXT1),CLK(INVALID)},
    [SELECTOR_MSC_MUX].route = {CLK(SCLKA),CLK(SCLKA),CLK(MPLL),CLK(MPLL)},
    [SELECTOR_F].route = {CLK(SCLKA),CLK(MPLL),CLK(OTGPHY),CLK(INVALID)},
    [SELECTOR_G].route = {CLK(SCLKA),CLK(EXT1),CLK(MPLL),CLK(INVALID)},
#undef CLK
};


struct cgu_clk
{
    int off;
    int ce_busy_stop;
    int coe;
    int div;
    int sel;
    int cache;
};
static struct cgu_clk cgu_clks[] = {
    [CGU_DDR] =     { CPM_DDRCDR,   27, 1, 4, SELECTOR_A},
    [CGU_MACPHY] =  { CPM_MACCDR,   27, 1, 8, SELECTOR_2},
    [CGU_LCD] =     { CPM_LPCDR,    26, 1, 8, SELECTOR_2},
    [CGU_MSC_MUX]=  { CPM_MSC0CDR,  27, 2, 0, SELECTOR_MSC_MUX},
    [CGU_MSC0] =    { CPM_MSC0CDR,  27, 2, 8, SELECTOR_MSC_MUX},
    [CGU_MSC1] =    { CPM_MSC1CDR,  27, 2, 8, SELECTOR_MSC_MUX},
    [CGU_USB] =     { CPM_USBCDR,   27, 1, 8, SELECTOR_C},
    [CGU_SFC] =     { CPM_SFCCDR,   27, 1, 8, SELECTOR_G},
    [CGU_CIM] =     { CPM_CIMCDR,   27, 1, 8, SELECTOR_2},
};


static uint32_t cgu_get_rate(struct clk *clk)
{
    uint32_t x;

    int no = CLK_CGU_NO(clk->flags);

    if (clk->parent == get_clk_from_id(CLK_ID_EXT1))
        return clk->parent->rate;

    if (no == CGU_MSC_MUX)
        return clk->parent->rate;

    if (cgu_clks[no].div == 0)
        return clk_get_rate(clk->parent);

    x = cpm_inl(cgu_clks[no].off);
    x &= (1 << cgu_clks[no].div) - 1;
    x = (x + 1) * cgu_clks[no].coe;

    return clk->parent->rate / x;
}

static int cgu_enable(struct clk *clk,int on)
{
    int no = CLK_CGU_NO(clk->flags);
    int reg_val;
    int ce, stop, busy;
    int prev_on;

    uint32_t mask;


    if (no == CGU_MSC_MUX)
        return 0;

    reg_val = cpm_inl(cgu_clks[no].off);
    stop    = cgu_clks[no].ce_busy_stop;
    busy    = stop + 1;
    ce      = stop + 2;
    prev_on = !(reg_val & (1 << stop));
    mask    = (1 << cgu_clks[no].div) - 1;

    if (prev_on && on)
        goto cgu_enable_finish;

    if ((!prev_on) && (!on))
        goto cgu_enable_finish;

    if (no == CGU_USB)
    {
        if (on)
            reg_val &= ~(1 << 26);
        else
            reg_val |= (1 << 26);
    }

    if (on)
    {
        if (cgu_clks[no].cache && ((cgu_clks[no].cache & mask) != (reg_val & mask)))
        {
            int x = cgu_clks[no].cache;
            x = (x & ~(0x1 << stop)) | (0x1 << ce);

            cpm_outl(x, cgu_clks[no].off);
            while (cpm_test_bit(busy, cgu_clks[no].off));

            cpm_clear_bit(ce, cgu_clks[no].off);
            x &= (1 << cgu_clks[no].div) - 1;
            x = (x + 1) * cgu_clks[no].coe;
            clk->rate = clk->parent->rate / x;
            cgu_clks[no].cache = 0;
        }
        else
        {
            reg_val |= (1 << ce);
            reg_val &= ~(1 << stop);
            cpm_outl(reg_val, cgu_clks[no].off);
            cpm_clear_bit(ce, cgu_clks[no].off);
        }
    }
    else
    {
        reg_val |= (1 << ce);
        reg_val |= (1 << stop);
        cpm_outl(reg_val, cgu_clks[no].off);
        cpm_clear_bit(ce, cgu_clks[no].off);
    }

cgu_enable_finish:

    return 0;
}

static int cgu_set_rate(struct clk *clk, uint32_t rate)
{
    uint32_t x,tmp;
    int i,no = CLK_CGU_NO(clk->flags);
    int ce,stop,busy;
    uint32_t reg_val,mask;

    if(no == CGU_MSC_MUX)
        return -1;

    mask = (1 << cgu_clks[no].div) - 1;
    tmp  = clk->parent->rate / cgu_clks[no].coe;

    for (i = 1; i <= mask + 1; i++)
    {
        if ((tmp / i) <= rate)
            break;
    }
    i--;
    if (i > mask)
        i = mask;
    reg_val = cpm_inl(cgu_clks[no].off);
    x = reg_val & ~mask;
    x |= i;
    stop = cgu_clks[no].ce_busy_stop;
    busy = stop + 1;
    ce = stop + 2;
    if (x & (1 << stop))
    {
        cgu_clks[no].cache = x;
        clk->rate = tmp / (i + 1);
    }
    else if ((mask & reg_val) != i)
    {

        x = (x & ~(0x1 << stop)) | (0x1 << ce);
        cpm_outl(x, cgu_clks[no].off);
        while (cpm_test_bit(busy, cgu_clks[no].off));
        x &= ~(1 << ce);
        cpm_outl(x, cgu_clks[no].off);
        cgu_clks[no].cache = 0;
        clk->rate = tmp / (i + 1);
    }

    return 0;
}

static struct clk* cgu_get_parent(struct clk *clk)
{
    uint32_t no,cgu,idx,pidx;

    no = CLK_CGU_NO(clk->flags);
    cgu = cpm_inl(cgu_clks[no].off);
    idx = cgu >> 30;
    pidx = selector[cgu_clks[no].sel].route[idx];
    if (pidx == CLK_ID_STOP || pidx == CLK_ID_INVALID)
        return NULL;

    return get_clk_from_id(pidx);
}

static int cgu_set_parent(struct clk *clk, struct clk *parent)
{
    int i,tmp;
    int no = CLK_CGU_NO(clk->flags);
    int ce,stop,busy;

    uint32_t reg_val,cgu,mask;

    stop = cgu_clks[no].ce_busy_stop;
    busy = stop + 1;
    ce = stop + 2;
    mask = (1 << cgu_clks[no].div) - 1;
    for(i = 0;i < 4;i++) {
        if(selector[cgu_clks[no].sel].route[i] == get_clk_id(parent)){
            break;
        }
    }
    if(i >= 4)
        return -1;
    cgu = cpm_inl(cgu_clks[no].off);
    reg_val = cgu;
    if (cgu_clks[no].sel == SELECTOR_2)
    {
        if (i == 0)
            cgu &= ~(1 << 31);
        else
            cgu |= (1 << 31);
    }
    else
    {
        cgu &= ~(3 << 30);
        cgu |= ~(i << 30);
    }

    tmp = parent->rate / cgu_clks[no].coe;
    for (i = 1; i <= mask + 1; i++)
    {
        if ((tmp / i) <= clk->rate)
            break;
    }
    i--;
    mask = (1 << cgu_clks[no].div) - 1;
    cgu = (cgu & ~(0x1 << stop)) | (0x1 << ce);
    cgu = cgu & ~mask;
    cgu |= i;

    if (reg_val & (1 << stop))
        cgu_clks[no].cache = cgu;
    else if ((mask & reg_val) != i)
    {
        cpm_outl(cgu, cgu_clks[no].off);
        while (cpm_test_bit(busy, cgu_clks[no].off));
        cgu &= ~(1 << ce);
        cpm_outl(cgu, cgu_clks[no].off);
        cgu_clks[no].cache = 0;
    }
    return 0;
}

static int cgu_is_enabled(struct clk *clk)
{
    int no = CLK_CGU_NO(clk->flags);
    int stop;
    stop = cgu_clks[no].ce_busy_stop;
    return !(cpm_inl(cgu_clks[no].off) & (1 << stop));
}

static struct clk_ops clk_cgu_ops =
{
    .enable = cgu_enable,
    .get_rate = cgu_get_rate,
    .set_rate = cgu_set_rate,
    .get_parent = cgu_get_parent,
    .set_parent = cgu_set_parent,
};

void init_cgu_clk(struct clk *clk)
{
    int no;
    int id;

    if (clk->flags & CLK_FLG_PARENT)
    {
        id = CLK_PARENT(clk->flags);
        clk->parent = get_clk_from_id(id);
    }
    else
    {
        clk->parent = cgu_get_parent(clk);
    }
    no = CLK_CGU_NO(clk->flags);
    cgu_clks[no].cache = 0;
    clk->rate = cgu_get_rate(clk);
    if (cgu_is_enabled(clk))
    {
        clk->flags |= CLK_FLG_ENABLE;
    }
    if (no == CGU_MSC_MUX)
        clk->ops = NULL;
    else if(no == CGU_DDR)
    {
    }
    else
        clk->ops = &clk_cgu_ops;
}

/*********************************************************************************************************
 **   CGU_AUDIO
 *********************************************************************************************************/
enum
{
    SELECTOR_AUDIO = 0,
};

const struct clk_selectors audio_selector[] =
{
#define CLK(X)  CLK_ID_##X
    [SELECTOR_AUDIO].route = {CLK(EXT1),CLK(SCLKA),CLK(EXT1),CLK(MPLL)},
#undef CLK
};
static int audio_div_apll[64] =
{
    8000 , 1 , 126000 ,
    11025 , 2 , 182857 ,
    12000 , 1 , 84000 ,
    16000 , 1 , 63000 ,
    22050 , 4 , 182857 ,
    24000 , 1 , 42000 ,
    32000 , 1 , 31500 ,
    44100 , 7 , 160000 ,
    48000 , 1 , 21000 ,
    88200 , 21 , 240000 ,
    96000 , 1 , 10500 ,
    176400 , 42 , 240000 ,
    192000 , 1 , 5250 ,

    0
};
static int audio_div_mpll[64] =
{
    8000 , 1 , 75000 ,
    11025 , 4 , 217687 ,
    12000 , 1 , 50000 ,
    16000 , 1 , 37500 ,
    22050 , 8 , 217687 ,
    24000 , 1 , 25000 ,
    32000 , 1 , 18750 ,
    44100 , 16 , 217687 ,
    48000 , 1 , 12500 ,
    88200 , 25 , 170068 ,
    96000 , 1 , 6250 ,
    176400 , 75 , 255102 ,
    192000 , 1 , 3125 ,

    0
};

struct cgu_audio_clk
{
    int off,en,maskm,bitm,maskn,bitn,maskd,bitd,sel,cache;
};
static struct cgu_audio_clk cgu_audio_clks[] =
{
    [CGU_AUDIO_I2S] =   { CPM_I2SCDR, 1<<29, 0x1f << 13, 13, 0x1fff, 0, SELECTOR_AUDIO},
    [CGU_AUDIO_I2S1] =  { CPM_I2SCDR1, -1, -1, -1, -1, -1, -1},
    [CGU_AUDIO_PCM] =   { CPM_PCMCDR, 1<<29, 0x1f << 13, 13, 0x1fff, 0, SELECTOR_AUDIO},
    [CGU_AUDIO_PCM1] =  { CPM_PCMCDR1, -1, -1, -1, -1, -1, -1},
};


static uint32_t cgu_audio_get_rate(struct clk *clk)
{
    uint32_t m, n, d;

    int no = CLK_CGU_AUDIO_NO(clk->flags);

    if (clk->parent == get_clk_from_id(CLK_ID_EXT1))
        return clk->parent->rate;

    m = cpm_inl(cgu_audio_clks[no].off);
    n = m & cgu_audio_clks[no].maskn;
    m &= cgu_audio_clks[no].maskm;

    if (no == CGU_AUDIO_I2S)
    {
        d = readl(I2S_PRI_DIV);
        return (clk->parent->rate * m) / (n * ((d & 0x3f) + 1) * (64));
    }
    else if (no == CGU_AUDIO_PCM)
    {
        d = readl(PCM_PRI_DIV);
        return (clk->parent->rate * m) / (n * (((d & 0x1f << 6) >> 6) + 1) * 8);
    }
    return 0;
}
static int cgu_audio_enable(struct clk *clk, int on)
{
    int no = CLK_CGU_AUDIO_NO(clk->flags);
    int reg_val;


    if (on)
    {
        reg_val = cpm_inl(cgu_audio_clks[no].off);
        if (reg_val & (cgu_audio_clks[no].en))
            goto cgu_enable_finish;

        cpm_outl(cgu_audio_clks[no].cache, cgu_audio_clks[no].off);
        cpm_outl(cgu_audio_clks[no].cache | cgu_audio_clks[no].en, cgu_audio_clks[no].off);
        cgu_audio_clks[no].cache = 0;
    }
    else
    {
        reg_val = cpm_inl(cgu_audio_clks[no].off);
        reg_val &= ~cgu_audio_clks[no].en;
        cpm_outl(reg_val, cgu_audio_clks[no].off);
    }
cgu_enable_finish:
    return 0;
}

static int get_div_val(int max1,int max2,int machval, int* res1, int* res2)
{
    int tmp1 = 0, tmp2 = 0;
    for (tmp1 = 1; tmp1 < max1; tmp1++)
        for (tmp2 = 1; tmp2 < max2; tmp2++)
            if (tmp1 * tmp2 == machval)
                break;
    if (tmp1 * tmp2 != machval)
    {
        return -1;
    }
    *res1 = tmp1;
    *res2 = tmp2;
    return 0;
}

static int cgu_audio_calculate_set_rate(struct clk* clk, uint32_t rate, uint32_t pid)
{
    int i,m,n,d,sync,tmp_val,d_max,sync_max;
    int no = CLK_CGU_AUDIO_NO(clk->flags);
    int n_max = cgu_audio_clks[no].maskn >> cgu_audio_clks[no].bitn;
    int *audio_div;

    if(pid == CLK_ID_MPLL)
    {
        audio_div = (int*)audio_div_mpll;
    }
    else if(pid == CLK_ID_SCLKA)
        audio_div = (int*)audio_div_apll;
    else
        return 0;

    for (i = 0; i < 50; i += 3)
    {
        if (audio_div[i] == rate)
            break;
    }
    if(i >= 50)
    {
        return -1;
    }
    else
    {
        m = audio_div[i+1];
        if(no == CGU_AUDIO_I2S)
        {
#ifdef CONFIG_SND_ASOC_JZ_AIC_SPDIF_V13
            m*=2;
#endif
            d_max = 0x1ff;
            tmp_val = audio_div[i + 2] / 64;
            if (tmp_val > n_max)
            {
                if (get_div_val(n_max, d_max, tmp_val, &n, &d))
                    goto calculate_err;
            }
            else
            {
                n = tmp_val / 4;
                d = 4;
            }
            tmp_val = cpm_inl(cgu_audio_clks[no].off)&(~(cgu_audio_clks[no].maskm|cgu_audio_clks[no].maskn));
            tmp_val |= (m<<cgu_audio_clks[no].bitm)|(n<<cgu_audio_clks[no].bitn);
            if (tmp_val & cgu_audio_clks[no].en)
            {
                cpm_outl(tmp_val, cgu_audio_clks[no].off);
            }
            else
            {
                cgu_audio_clks[no].cache = tmp_val;
            }

            cpm_outl(0,CPM_I2SCDR1);
            writel(d - 1,I2S_PRI_DIV);
        }
        else if (no == CGU_AUDIO_PCM)
        {
            tmp_val = audio_div[i+2]/(8);
            d_max = 0x7f;
            if (tmp_val > n_max)
            {
                if (get_div_val(n_max, d_max, tmp_val, &n, &d))
                    goto calculate_err;
                if (d > 0x3f)
                {
                    tmp_val = d;
                    d_max = 0x3f, sync_max = 0x1f;
                    if (get_div_val(d_max, sync_max, tmp_val, &d, &sync))
                        goto calculate_err;
                }
                else
                {
                    sync = 1;
                }
            }
            else
            {
                n = tmp_val;
                d = 1;
                sync = 1;
            }
            tmp_val = cpm_inl(cgu_audio_clks[no].off)&(~(cgu_audio_clks[no].maskm|cgu_audio_clks[no].maskn));
            tmp_val |= (m<<cgu_audio_clks[no].bitm)|(n<<cgu_audio_clks[no].bitn);
            if (tmp_val & cgu_audio_clks[no].en)
            {
                cpm_outl(tmp_val, cgu_audio_clks[no].off);
            }
            else
            {
                cgu_audio_clks[no].cache = tmp_val;
            }
            writel(((d-1)|(sync-1)<<6),PCM_PRI_DIV);
        }
    }
    clk->rate = rate;
    return 0;
calculate_err:
    return -1;
}

static struct clk* cgu_audio_get_parent(struct clk *clk)
{
    uint32_t no,cgu,idx,pidx;

    struct clk* pclk;

    no = CLK_CGU_AUDIO_NO(clk->flags);
    cgu = cpm_inl(cgu_audio_clks[no].off);
    idx = cgu >> 30;
    pidx = audio_selector[cgu_audio_clks[no].sel].route[idx];
    if (pidx == CLK_ID_STOP || pidx == CLK_ID_INVALID)
    {
        return NULL;
    }
    pclk = get_clk_from_id(pidx);

    return pclk;
}

static int cgu_audio_set_parent(struct clk *clk, struct clk *parent)
{
    int tmp_val,i;
    int no = CLK_CGU_AUDIO_NO(clk->flags);

    for(i = 0;i < 4;i++) {
        if(audio_selector[cgu_audio_clks[no].sel].route[i] == get_clk_id(parent)){
            break;
        }
    }

    if(i >= 4)
        return -1;

    if (get_clk_id(parent) != CLK_ID_EXT1)
    {
        tmp_val = cpm_inl(cgu_audio_clks[no].off) & (~(3 << 30));
        tmp_val |= i << 30;
        cpm_outl(tmp_val, cgu_audio_clks[no].off);
    }
    else
    {
        tmp_val = cpm_inl(cgu_audio_clks[no].off) & (~(3 << 30 | 0x3fffff));
        tmp_val |= i << 30 | 1 << 13 | 1;
        cpm_outl(tmp_val, cgu_audio_clks[no].off);
    }

    return 0;
}

static int cgu_audio_set_rate(struct clk *clk, uint32_t rate)
{
    int tmp_val;

    int no = CLK_CGU_AUDIO_NO(clk->flags);
    if (rate == 24000000)
    {
        cgu_audio_set_parent(clk, get_clk_from_id(CLK_ID_EXT1));
        clk->parent = get_clk_from_id(CLK_ID_EXT1);
        clk->rate = rate;
        tmp_val = cpm_inl(cgu_audio_clks[no].off);
        tmp_val &= ~0x3fffff;
        tmp_val |= 1<<13|1;
        if(tmp_val&cgu_audio_clks[no].en)
            cpm_outl(tmp_val,cgu_audio_clks[no].off);
        else
            cgu_audio_clks[no].cache = tmp_val;
        return 0;
    }
    else
    {
        if(get_clk_id(clk->parent) == CLK_ID_EXT1)
            cgu_audio_set_parent(clk,get_clk_from_id(CLK_ID_SCLKA));

        cgu_audio_calculate_set_rate(clk,rate,CLK_ID_SCLKA);

        clk->parent = get_clk_from_id(CLK_ID_SCLKA);
    }
    return 0;
}


static int cgu_audio_is_enabled(struct clk *clk) {
    int no,state;

    no = CLK_CGU_AUDIO_NO(clk->flags);
    state = (cpm_inl(cgu_audio_clks[no].off) & cgu_audio_clks[no].en);
    return state;
}

static struct clk_ops clk_cgu_audio_ops =
{
    .enable     = cgu_audio_enable,
    .get_rate   = cgu_audio_get_rate,
    .set_rate   = cgu_audio_set_rate,
    .get_parent = cgu_audio_get_parent,
    .set_parent = cgu_audio_set_parent,
};

void init_cgu_audio_clk(struct clk *clk)
{
    int no,id,tmp_val;

    if (clk->flags & CLK_FLG_PARENT)
    {
        id = CLK_PARENT(clk->flags);
        clk->parent = get_clk_from_id(id);
    }
    else
    {
        clk->parent = cgu_audio_get_parent(clk);
    }
    no = CLK_CGU_AUDIO_NO(clk->flags);
    cgu_audio_clks[no].cache = 0;
    if (cgu_audio_is_enabled(clk))
    {
        clk->flags |= CLK_FLG_ENABLE;
    }
    clk->rate = cgu_audio_get_rate(clk);
    tmp_val = cpm_inl(cgu_audio_clks[no].off);
    tmp_val &= ~0x3fffff;
    tmp_val |= 1<<13|1;
    if((tmp_val&cgu_audio_clks[no].en)&&(clk->rate == 24000000))
        cpm_outl(tmp_val,cgu_audio_clks[no].off);
    else
        cgu_audio_clks[no].cache = tmp_val;

    clk->ops = &clk_cgu_audio_ops;
}

static int cpm_gate_enable(struct clk *clk,int on)
{
    int bit = CLK_GATE_BIT(clk->flags);
    uint32_t clkgr[2] = {CPM_CLKGR};

    if (on)
    {
        cpm_clear_bit(bit % 32, clkgr[bit / 32]);
    }
    else
    {
        cpm_set_bit(bit % 32, clkgr[bit / 32]);
    }

    return 0;
}
static struct clk_ops clk_gate_ops =
{
    .enable = cpm_gate_enable,
};

void init_gate_clk(struct clk *clk)
{
    int id = 0;
    static  uint32_t clkgr[2]={0};
    static  int     clkgr_init = 0;
    int bit = CLK_GATE_BIT(clk->flags);

    if (clkgr_init == 0)
    {
        clkgr[0] = cpm_inl(CPM_CLKGR);
        clkgr_init = 1;
    }
    if (clk->flags & CLK_FLG_PARENT)
    {
        id = CLK_PARENT(clk->flags);
        clk->parent = get_clk_from_id(id);
    }
    else
        clk->parent = get_clk_from_id(CLK_ID_EXT1);

    clk->rate = clk_get_rate(clk->parent);
    if (clkgr[bit / 32] & (1 << (bit % 32)))
    {
        clk->flags &= ~(CLK_FLG_ENABLE);
    }
    else
    {
        clk->flags |= CLK_FLG_ENABLE;
    }
    clk->ops = &clk_gate_ops;
}

static void init_clk_parent(struct clk *p)
{
    int init = 0;
    if (!p)
        return;
    if (p->init_state)
    {
        p->count = 1;
        p->init_state = 0;
        init = 1;
    }
    if (p->count == 0)
    {
        p->count = 1;
    }
    if (!init)
        p->count ++;
}

int init_all_clk(void)
{
    int i;
    struct clk *clk_srcs = get_clk_from_id(0);
    int clk_srcs_size = get_clk_sources_size();

    for (i = 0; i < clk_srcs_size; i++)
    {
        clk_srcs[i].CLK_ID = i;

        if (clk_srcs[i].flags & CLK_FLG_CPCCR)
        {
            init_cpccr_clk(&clk_srcs[i]);
        }
        if (clk_srcs[i].flags & CLK_FLG_CGU)
        {
            init_cgu_clk(&clk_srcs[i]);
        }

        if (clk_srcs[i].flags & CLK_FLG_CGU_AUDIO)
        {
            init_cgu_audio_clk(&clk_srcs[i]);
        }

        if (clk_srcs[i].flags & CLK_FLG_PLL)
        {
            init_ext_pll(&clk_srcs[i]);
        }
        if (clk_srcs[i].flags & CLK_FLG_NOALLOC)
        {
            init_ext_pll(&clk_srcs[i]);
        }
        if (clk_srcs[i].flags & CLK_FLG_GATE)
        {
            init_gate_clk(&clk_srcs[i]);
        }
        if (clk_srcs[i].flags & CLK_FLG_ENABLE)
            clk_srcs[i].init_state = 1;
    }

    for (i = 0; i < clk_srcs_size; i++)
    {
        if (clk_srcs[i].parent && clk_srcs[i].init_state)
            init_clk_parent(clk_srcs[i].parent);
    }

    return 0;
}
