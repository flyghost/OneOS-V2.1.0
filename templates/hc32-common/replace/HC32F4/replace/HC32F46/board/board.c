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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_gpio.h>
#include "hc32f46x_clk.h"

#ifdef OS_USING_LED
const led_t led_table[] =
{
    {GET_PIN(A, 8), PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);
#endif

#ifdef OS_USING_PUSH_BUTTON
const struct push_button key_table[] =
{
    {GET_PIN(A, 7),     PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);
#endif

#ifdef OS_USING_SN
const int board_no_pin_tab[] =
{
    GET_PIN(B, 3),
    GET_PIN(A, 15),
    GET_PIN(A, 12),
    GET_PIN(A, 11),
    GET_PIN(A, 3),
    GET_PIN(A, 2),
    GET_PIN(A, 1),
    GET_PIN(A, 0),
};

const int board_no_pin_tab_size = ARRAY_SIZE(board_no_pin_tab);

const int slot_no_pin_tab[] =
{
    GET_PIN(B, 0),
    GET_PIN(B, 1),
    GET_PIN(B, 5),
    GET_PIN(B, 6),
    GET_PIN(B, 7),
};

const int slot_no_pin_tab_size = ARRAY_SIZE(slot_no_pin_tab);
#endif

static void ClkInit(void)
{
    stc_clk_xtal_cfg_t   stcXtalCfg;
    stc_clk_mpll_cfg_t   stcMpllCfg;
    en_clk_sys_source_t  enSysClkSrc;
    stc_clk_sysclk_cfg_t stcSysClkCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* MPLL config. */
    stcMpllCfg.pllmDiv = 1u; /* XTAL 8M / 1 */
    stcMpllCfg.plln = 50u;   /* 8M*50 = 400M */
    stcMpllCfg.PllpDiv = 4u; /* MLLP = 100M */
    stcMpllCfg.PllqDiv = 4u; /* MLLQ = 100M */
    stcMpllCfg.PllrDiv = 4u; /* MLLR = 100M */
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_4);
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);

    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
    }

    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}

void System_cfg(void)
{
    ClkInit();
}
