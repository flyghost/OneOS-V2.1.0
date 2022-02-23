/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/
/*
 * How to set up clock using clock driver functions:
 *
 * 1. Setup clock sources.
 *
 * 2. Set up wait states of the flash.
 *
 * 3. Set up all dividers.
 *
 * 4. Set up all selectors to provide selected clocks.
 */

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Clocks v7.0
processor: LPC55S28
package_id: LPC55S28JBD64
mcu_data: ksdk2_0
processor_version: 8.0.3
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

#include "fsl_power.h"
#include "fsl_clock.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

/*******************************************************************************
 ********************** Configuration BOARD_BootClockRUN ***********************
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!Configuration
name: BOARD_BootClockRUN
called_from_default_init: true
outputs:
- {id: ASYNCADC_clock.outFreq, value: 16 MHz}
- {id: CTIMER0_clock.outFreq, value: 96 MHz}
- {id: CTIMER1_clock.outFreq, value: 96 MHz}
- {id: FXCOM0_clock.outFreq, value: 48 MHz}
- {id: FXCOM1_clock.outFreq, value: 12 MHz, locked: true, accuracy: '0.001'}
- {id: FXCOM2_clock.outFreq, value: 48 MHz}
- {id: FXCOM3_clock.outFreq, value: 48 MHz}
- {id: FXCOM4_clock.outFreq, value: 12 MHz}
- {id: FXCOM5_clock.outFreq, value: 12 MHz}
- {id: FXCOM6_clock.outFreq, value: 12 MHz}
- {id: SYSTICK0_clock.outFreq, value: 1 MHz}
- {id: System_clock.outFreq, value: 72 MHz}
- {id: UTICK_clock.outFreq, value: 1 MHz}
- {id: WDT_clock.outFreq, value: 20 kHz}
settings:
- {id: PLL0_Mode, value: Normal}
- {id: ANALOG_CONTROL_FRO192M_CTRL_ENDI_FRO_96M_CFG, value: Enable}
- {id: ENABLE_CLKIN_ENA, value: Enabled}
- {id: ENABLE_SYSTEM_CLK_OUT, value: Enabled}
- {id: SYSCON.ADCCLKDIV.scale, value: '6', locked: true}
- {id: SYSCON.ADCCLKSEL.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.CTIMERCLKSEL0.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.CTIMERCLKSEL1.sel, value: ANACTRL.fro_hf_clk}
- {id: SYSCON.FCCLKSEL0.sel, value: SYSCON.MAINCLKSELB}
- {id: SYSCON.FCCLKSEL1.sel, value: ANACTRL.fro_12m_clk}
- {id: SYSCON.FCCLKSEL2.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FCCLKSEL3.sel, value: SYSCON.FROHFDIV}
- {id: SYSCON.FCCLKSEL4.sel, value: ANACTRL.fro_12m_clk}
- {id: SYSCON.FCCLKSEL5.sel, value: ANACTRL.fro_12m_clk}
- {id: SYSCON.FCCLKSEL6.sel, value: ANACTRL.fro_12m_clk}
- {id: SYSCON.FRGCTRL0_DIV.scale, value: '384'}
- {id: SYSCON.FROHFDIV.scale, value: '2', locked: true}
- {id: SYSCON.MAINCLKSELB.sel, value: SYSCON.PLL0_BYPASS}
- {id: SYSCON.PLL0CLKSEL.sel, value: ANACTRL.fro_12m_clk}
- {id: SYSCON.PLL0M_MULT.scale, value: '192', locked: true}
- {id: SYSCON.PLL0N_DIV.scale, value: '8', locked: true}
- {id: SYSCON.PLL0_PDEC.scale, value: '4', locked: true}
- {id: SYSCON.SYSTICKCLKSEL0.sel, value: SYSCON.fro_1m}
- {id: SYSCON.WDTCLKDIV.scale, value: '50', locked: true}
- {id: SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_CFG, value: Enabled}
- {id: UTICK_EN_CFG, value: Enable}
sources:
- {id: ANACTRL.fro_hf.outFreq, value: 96 MHz}
- {id: SYSCON.XTAL32M.outFreq, value: 16 MHz, enabled: true}
- {id: SYSCON.fro_1m.outFreq, value: 1 MHz}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Variables for BOARD_BootClockRUN configuration
 ******************************************************************************/
/*******************************************************************************
 * Code for BOARD_BootClockRUN configuration
 ******************************************************************************/
void BOARD_BootClockRUN(void)
{
#ifndef SDK_SECONDARY_CORE
    /*!< Set up the clock sources */
    /*!< Configure FRO192M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO192M);               /*!< Ensure FRO is on  */
    CLOCK_SetupFROClocking(12000000U);                   /*!< Set up FRO to the 12 MHz, just for sure */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);                /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    /*!< Configure fro_1m */
    SYSCON->CLOCK_CTRL |=  SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;                 /*!< Ensure fro_1m is on */

    CLOCK_SetupFROClocking(96000000U);                   /* Enable FRO HF(96MHz) output */

    /*!< Configure XTAL32M */
    POWER_DisablePD(kPDRUNCFG_PD_XTAL32M);                        /* Ensure XTAL32M is powered */
    POWER_DisablePD(kPDRUNCFG_PD_LDOXO32M);                       /* Ensure XTAL32M is powered */
    CLOCK_SetupExtClocking(16000000U);                            /* Enable clk_in clock */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK;       /* Enable clk_in from XTAL32M clock  */
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;    /* Enable clk_in to system  */

    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_UTICK_ENA_MASK;               /* The FRO 1 MHz clock to UTICK is enabled. */

    POWER_SetVoltageForFreq(96000000U);                  /*!< Set voltage for the one of the fastest clock outputs: CTIMER0 clock output */
    CLOCK_SetFLASHAccessCyclesForFreq(72000000U);          /*!< Set FLASH wait states for core */

    /*!< Set up PLL */
    CLOCK_AttachClk(kFRO12M_to_PLL0);                    /*!< Switch PLL0CLKSEL to FRO12M */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0);                  /* Ensure PLL is on  */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0_SSCG);
    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(41U) | SYSCON_PLL0CTRL_SELP(31U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(8U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(2U),
        .pllsscg = {0x0U,(SYSCON_PLL0SSCG1_MDIV_EXT(192U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
        .pllRate = 72000000U,
        .flags =  PLL_SETUPFLAG_WAITLOCK
    };
    CLOCK_SetPLL0Freq(&pll0Setup);                       /*!< Configure PLL0 to the desired values */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivArmTrClkDiv, 0U, true);               /*!< Reset TRACECLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivArmTrClkDiv, 1U, false);         /*!< Set TRACECLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk0, 0U, true);               /*!< Reset SYSTICKCLKDIV0 divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk0, 1U, false);         /*!< Set SYSTICKCLKDIV0 divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 0U, true);               /*!< Reset FRGCTRL0_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 33024U, false);         /*!< Set FRGCTRL0_DIV divider to value 384 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg1, 0U, true);               /*!< Reset FRGCTRL1_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg1, 256U, false);         /*!< Set FRGCTRL1_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg2, 0U, true);               /*!< Reset FRGCTRL2_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg2, 256U, false);         /*!< Set FRGCTRL2_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg3, 0U, true);               /*!< Reset FRGCTRL3_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg3, 256U, false);         /*!< Set FRGCTRL3_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg4, 0U, true);               /*!< Reset FRGCTRL4_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg4, 256U, false);         /*!< Set FRGCTRL4_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg5, 0U, true);               /*!< Reset FRGCTRL5_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg5, 256U, false);         /*!< Set FRGCTRL5_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg6, 0U, true);               /*!< Reset FRGCTRL6_DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg6, 256U, false);         /*!< Set FRGCTRL6_DIV divider to value 256 */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);         /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 0U, true);               /*!< Reset AHBCLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);         /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 0U, true);               /*!< Reset FROHFDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 2U, false);         /*!< Set FROHFDIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 0U, true);               /*!< Reset FROHFDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 2U, false);         /*!< Set FROHFDIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 0U, true);               /*!< Reset WDTCLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 50U, false);         /*!< Set WDTCLKDIV divider to value 50 */
    CLOCK_SetClkDiv(kCLOCK_DivAdcAsyncClk, 0U, true);               /*!< Reset ADCCLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivAdcAsyncClk, 6U, false);         /*!< Set ADCCLKDIV divider to value 6 */
    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 0U, true);               /*!< Reset PLL0DIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 1U, false);         /*!< Set PLL0DIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kPLL0_to_MAIN_CLK);                 /*!< Switch MAIN_CLK to PLL0 */
    CLOCK_AttachClk(kFRO_HF_to_ADC_CLK);                 /*!< Switch ADC_CLK to FRO_HF */
    CLOCK_AttachClk(kMAIN_CLK_to_FLEXCOMM0);                 /*!< Switch FLEXCOMM0 to MAIN_CLK */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);                 /*!< Switch FLEXCOMM1 to FRO12M */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM2);                 /*!< Switch FLEXCOMM2 to FRO_HF_DIV */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM3);                 /*!< Switch FLEXCOMM3 to FRO_HF_DIV */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);                 /*!< Switch FLEXCOMM4 to FRO12M */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);                 /*!< Switch FLEXCOMM5 to FRO12M */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);                 /*!< Switch FLEXCOMM6 to FRO12M */
    CLOCK_AttachClk(kFRO1M_to_SYSTICK0);                 /*!< Switch SYSTICK0 to FRO1M */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);                 /*!< Switch CTIMER0 to FRO_HF */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);                 /*!< Switch CTIMER1 to FRO_HF */

    /*< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
#endif
}

