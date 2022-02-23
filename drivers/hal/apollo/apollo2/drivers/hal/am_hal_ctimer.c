//*****************************************************************************
//
//  am_hal_ctimer.c
//! @file
//!
//! @brief Functions for interfacing with the Counter/Timer module.
//!
//! @addtogroup ctimer2 Counter/Timer (CTIMER)
//! @ingroup apollo2hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2020, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision 2.5.1 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Address space distance between timer configuration registers.
//
//*****************************************************************************
#define MAX_CTIMERS         4
#define TIMER_OFFSET        (AM_REG_CTIMER_TMR1_O - AM_REG_CTIMER_TMR0_O)
#define CTIMER_CMPR_OFFSET  (AM_REG_CTIMER_CMPRB0_O - AM_REG_CTIMER_CMPRA0_O)

//*****************************************************************************
//
// Adjacency check
//
// This is related to the timer read workaround. This macro checks to see if
// the two supplied count values are within one "tick" of eachother. It should
// still pass in the event of a timer rollover.
//
//*****************************************************************************
//! Timer read workaround: Do count values differ by one tick or less.
#define adjacent(A, B)      (((A) == (B)) || (((A) + 1) == (B)) || ((B) == 0))

//*****************************************************************************
//
//! Array of function pointers for handling CTimer interrupts.
//
//*****************************************************************************
am_hal_ctimer_handler_t am_hal_ctimer_ppfnHandlers[16];

//*****************************************************************************
//
//! @brief Check to see if the given CTimer is using the HFRC
//!
//! @note Calls to this function should be from inside a critical section.
//!
//! @return None.
//
//*****************************************************************************
static bool
ctimer_source_hfrc(uint32_t ui32CtimerNum)
{
    uint32_t *pui32ConfigReg;
    uint32_t ui32TimerASrc, ui32TimerBSrc;

    //
    // Find the correct register to write.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32CtimerNum * TIMER_OFFSET));

    //
    // Determine if this timer is using HFRC as the clock source.
    // The value we are looking for is HFRC_DIV4 to HFRC_DIV4K.
    // Get the clock sources and 0-base the extracted value.
    //
    ui32TimerASrc = AM_BFX(CTIMER, CTRL0, TMRA0CLK, *pui32ConfigReg) -
                    AM_ENUMX(CTIMER, CTRL0, TMRA0CLK, HFRC_DIV4);
    ui32TimerBSrc = AM_BFX(CTIMER, CTRL0, TMRB0CLK, *pui32ConfigReg) -
                    AM_ENUMX(CTIMER, CTRL0, TMRB0CLK, HFRC_DIV4);

    //
    // If the source value is 0 to (HFRC_DIV4K - HFRC_DIV4), then it's HFRC.
    //
    if ( (ui32TimerASrc <= (AM_ENUMX(CTIMER, CTRL0, TMRA0CLK, HFRC_DIV4K) -
                            AM_ENUMX(CTIMER, CTRL0, TMRA0CLK, HFRC_DIV4)))  ||
         (ui32TimerBSrc <= (AM_ENUMX(CTIMER, CTRL0, TMRB0CLK, HFRC_DIV4K) -
                            AM_ENUMX(CTIMER, CTRL0, TMRB0CLK, HFRC_DIV4))) )
    {
        return true;
    }
    else
    {
        return false;
    }

} // ctimer_source_hfrc()

//*****************************************************************************
//
// @brief Check to see if any of the CTimers or STimer are using the HFRC.
//
//  This function should be used to check if the HFRC is being used in order
//  to correctly establish power related settings.
//
//  Note - Calls to this function should be from inside a critical section.
//
//! @return None.
//
//*****************************************************************************
static bool
timers_use_hfrc(void)
{
    uint32_t ui32TimerASrc, ui32CtimerNum;

    //
    // Check STimer to see if it is using HFRC.
    //
    ui32TimerASrc = AM_BFR(CTIMER, STCFG, CLKSEL);
    if ( (ui32TimerASrc == AM_REG_CTIMER_STCFG_CLKSEL_HFRC_DIV16)   ||
         (ui32TimerASrc == AM_REG_CTIMER_STCFG_CLKSEL_HFRC_DIV256) )
    {
        return true;
    }

    //
    // Check the CTimers to see if any are using HFRC as their clock source.
    //
    for ( ui32CtimerNum = 0; ui32CtimerNum < MAX_CTIMERS; ui32CtimerNum++ )
    {
        if ( ctimer_source_hfrc(ui32CtimerNum) )
        {
            return true;
        }
    }

    return false;

} // timers_use_hfrc()

//*****************************************************************************
//
// ctimer_clr()
//
// For the appropriate ctimer configuration register, set the CLR bit high
// in the appropriate timer segment (A, B, or both).
//
// The CLR bit is required to be set in order to completely initialize
// the timer at config time.  The timer clear occurs asynchrnously during the
// low-to-high transition of the CLR bit.
//
// This function only sets the CLR bit.  It is assumed that the actual timer
// configuration will occur following the call to this function and will clear
// the CLR bit at that time.
//
//*****************************************************************************
static void
ctimer_clr(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    //
    // Find the address of the correct control register and set the CLR bit
    // for the timer segment in that control register.
    //
    volatile uint32_t *pui32ConfigReg =
        (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                     (ui32TimerNumber * TIMER_OFFSET));

    AM_CRITICAL_BEGIN
    AM_REGVAL(pui32ConfigReg) |= (ui32TimerSegment &
                                  (AM_REG_CTIMER_CTRL0_TMRA0CLR_M |
                                   AM_REG_CTIMER_CTRL0_TMRB0CLR_M));
    AM_CRITICAL_END

} // ctimer_clr()

//*****************************************************************************
//
//! @brief Convenience function for responding to CTimer interrupts.
//!
//! @param ui32Status is the interrupt status as returned by
//! am_hal_ctimer_int_status_get()
//!
//! This function may be called from am_ctimer_isr() to read the status of
//! the CTimer interrupts, determine which source caused the most recent
//! interrupt, and call an interrupt handler function to respond. The interrupt
//! handler to be called must be first registered with the
//! am_hal_ctimer_int_register() function.
//!
//! In the event that multiple sources are active, the corresponding
//! interrupt handlers will be called in numerical order based on interrupt def.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_service(uint32_t ui32Status)
{
    uint32_t ui32Clz;

    am_hal_ctimer_handler_t pfnHandler;

    ui32Status &= 0xFFFF;

    while ( ui32Status )
    {
        //
        // Pick one of any remaining active interrupt bits
        //
#ifdef __IAR_SYSTEMS_ICC__
        ui32Clz = __CLZ(ui32Status);
#else
        ui32Clz = __builtin_clz(ui32Status);
#endif

        //
        // Turn off the bit we picked in the working copy
        //
        ui32Status &= ~(0x80000000 >> ui32Clz);

        //
        // Check the bit handler table to see if there is an interrupt handler
        // registered for this particular bit.
        //
        pfnHandler = am_hal_ctimer_ppfnHandlers[31 - ui32Clz];
        if ( pfnHandler )
        {
            //
            // If we found an interrupt handler routine, call it now.
            //
            pfnHandler();
        }
    }
} // am_hal_ctimer_int_service()

//*****************************************************************************
//
//! @brief Register an interrupt handler for CTimer.
//!
//! @param ui32Interrupt - interrupt number to assign this interrupt handler to.
//! @param pfnHandler - Function to call when this interrupt is received.
//!
//! This function allows the caller to specify a function that should be called
//! any time a Ctimer interrupt is received. Registering an
//! interrupt handler using this function adds the function pointer to an array
//! in SRAM. This interrupt handler will be called by am_hal_ctimer_int_service()
//! whenever the ui32Status parameter indicates that the corresponding interrupt.
//!
//! To remove an interrupt handler that has already been registered, the
//! pfnHandler parameter may be set to zero.
//!
//! @note This function will not have any effect unless the
//! am_hal_ctimer_int_service() function is being used.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_register(uint32_t ui32Interrupt,
                           am_hal_ctimer_handler_t pfnHandler)
{
    uint32_t intIdx = 0;

    //
    // Check to make sure the interrupt number is valid. (Debug builds only)
    //
    switch (ui32Interrupt)
    {
        case AM_REG_CTIMER_INTEN_CTMRA0C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA0C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB0C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB0C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA1C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA1C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB1C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB1C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA2C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA2C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB2C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB2C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA3C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA3C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB3C0INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB3C0INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA0C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA0C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB0C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB0C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA1C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA1C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB1C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB1C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA2C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA2C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB2C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB2C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRA3C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRA3C1INT_S;
            break;

        case AM_REG_CTIMER_INTEN_CTMRB3C1INT_M:
            intIdx = AM_REG_CTIMER_INTEN_CTMRB3C1INT_S;
            break;

        default:
            am_hal_debug_assert_msg(false, "CTimer interrupt number out of range.");
    }

    am_hal_ctimer_ppfnHandlers[intIdx] = pfnHandler;

} // am_hal_ctimer_int_register()

//*****************************************************************************
//
//! @brief Set up the counter/timer.
//!
//! @param ui32TimerNumber is the number of the Timer that should be
//! configured.
//!
//! @param psConfig is a pointer to a structure that holds important settings
//! for the timer.
//!
//! This function should be used to perform the initial set-up of the
//! counter-timer.
//!
//! @note This function is deprecated and will eventually be replaced by
//! am_hal_ctimer_config_single(), which performs the same configuration
//! without requiring a structure and without assuming both timer halves
//! are being configured.
//! Please use am_hal_ctimer_config_single() for new development.
//!
//! @return None.
//!
//!
//! @note In order to initialize the given timer into a known state, this
//! function asserts the CLR configuration bit. The CLR bit will be deasserted
//! with the write of the configuration register. The CLR bit is also
//! intentionally deasserted with a call to am_hal_ctimer_start().
//!
//
//*****************************************************************************
void
am_hal_ctimer_config(uint32_t ui32TimerNumber,
                     am_hal_ctimer_config_t *psConfig)
{
    uint32_t *pui32ConfigReg;
    uint32_t ui32ConfigVal;

    //
    // Make sure the timer is completely initialized on configuration by
    // setting the CLR bit.
    //
    ctimer_clr(ui32TimerNumber, AM_HAL_CTIMER_BOTH);

    //
    // Start preparing the configuration word for this timer. The configuration
    // values for Timer A and Timer B provided in the config structure should
    // match the register definitions already, so we will mostly just need to
    // OR them together.
    //
    ui32ConfigVal = ( (psConfig->ui32TimerAConfig)  |
                      (psConfig->ui32TimerBConfig << 16) );

    //
    // OR in the Link bit if the timers need to be linked.
    //
    ui32ConfigVal |= psConfig->ui32Link ? AM_HAL_CTIMER_LINK : 0;

    //
    // Find the correct register to write.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section while config registers are read and modified.
    //
    AM_CRITICAL_BEGIN

    //
    // Write our configuration value.
    //
    AM_REGVAL(pui32ConfigReg) = ui32ConfigVal;

    //
    // If all of the clock sources are not HRFC disable LDO when sleeping if timers are enabled.
    //
    if ( timers_use_hfrc() )
    {
        AM_BFW(PWRCTRL, MISCOPT, DIS_LDOLPMODE_TIMERS, 0);
    }
    else
    {
        AM_BFW(PWRCTRL, MISCOPT, DIS_LDOLPMODE_TIMERS, 1);
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_config()

//*****************************************************************************
//
//! @brief Set up the counter/timer.
//!
//! @param ui32TimerNumber is the number of the Timer that should be
//! configured.
//!
//! @param ui32TimerSegment specifies which segment of the timer should be
//! enabled.
//!
//! @param ui32ConfigVal specifies the configuration options for the selected
//! timer.
//!
//! This function should be used to perform the initial set-up of the
//! counter-timer. It can be used to configure either a 16-bit timer (A or B) or a
//! 32-bit timer using the BOTH option.
//!
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! The timer's clock source, mode, interrupt, and external pin behavior are
//! all controlled through the \e ui32Configval parameter. The valid options
//! for ui32ConfigVal include any ORed together combination of the following:
//!
//! Clock configuration macros:
//!
//!     AM_HAL_CTIMER_HFRC_24MHZ
//!     AM_HAL_CTIMER_LFRC_512HZ
//!     ... etc. (See am_hal_ctimer.h for the full set of options.)
//!
//! Mode selection macros:
//!
//!     AM_HAL_CTIMER_FN_ONCE
//!     AM_HAL_CTIMER_FN_REPEAT
//!     AM_HAL_CTIMER_FN_PWM_ONCE
//!     AM_HAL_CTIMER_FN_PWM_REPEAT
//!     AM_HAL_CTIMER_FN_CONTINUOUS
//!
//! Interrupt control:
//!
//!     AM_HAL_CTIMER_INT_ENABLE
//!
//! Pin control:
//!
//!     AM_HAL_CTIMER_PIN_ENABLE
//!     AM_HAL_CTIMER_PIN_INVERT
//!
//! ADC trigger (Timer 3 only):
//!
//!     AM_HAL_CTIMER_ADC_TRIG
//!
//! @return None.
//!
//!
//! @note In order to initialize the given timer into a known state, this
//! function asserts the CLR configuration bit. The CLR bit will be deasserted
//! with the write of the configuration register. The CLR bit is also
//! intentionally deasserted with a call to am_hal_ctimer_start().
//!
//
//*****************************************************************************
void
am_hal_ctimer_config_single(uint32_t ui32TimerNumber,
                            uint32_t ui32TimerSegment,
                            uint32_t ui32ConfigVal)
{
    volatile uint32_t *pui32ConfigReg;
    uint32_t ui32WriteVal;

    //
    // Make sure the timer is completely initialized on configuration by
    // setting the CLR bit.
    //
    ctimer_clr(ui32TimerNumber, ui32TimerSegment);

    //
    // Find the correct register to write based on the timer number.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section while config registers are read and modified.
    //
    AM_CRITICAL_BEGIN

    //
    // Save the value that's already in the register.
    //
    ui32WriteVal = AM_REGVAL(pui32ConfigReg);

    //
    // If we're working with TIMERB, we need to shift our configuration value
    // up by 16 bits.
    //
    if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERB )
    {
        ui32ConfigVal = ((ui32ConfigVal & 0xFFFF) << 16);
    }

    //
    // Replace part of the saved register value with the configuration value
    // from the caller.
    //
    ui32WriteVal = (ui32WriteVal & ~(ui32TimerSegment)) | ui32ConfigVal;

    //
    // If we're configuring both timers, we need to set the "link" bit.
    //
    if ( ui32TimerSegment == AM_HAL_CTIMER_BOTH )
    {
        ui32WriteVal |= AM_HAL_CTIMER_LINK;
    }

    //
    // Write our completed configuration value.
    //
    AM_REGVAL(pui32ConfigReg) = ui32WriteVal;

    //
    // If all of the clock sources are not HRFC disable LDO when sleeping if timers are enabled.
    //
    if ( timers_use_hfrc() )
    {
        AM_BFW(PWRCTRL, MISCOPT, DIS_LDOLPMODE_TIMERS, 0);
    }
    else
    {
        AM_BFW(PWRCTRL, MISCOPT, DIS_LDOLPMODE_TIMERS, 1);
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_config_single()

//*****************************************************************************
//
//! @brief Start a timer
//!
//! @param ui32TimerNumber is the number of the timer to enable
//!
//! @param ui32TimerSegment specifies which segment of the timer should be
//! enabled.  Valid values for ui32TimerSegment are:
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! This function will enable a timer to begin incrementing. The \e
//! ui32TimerNumber parameter selects the timer that should be enabled, for
//! example, a 0 would target TIMER0. The \e ui32TimerSegment parameter allows
//! the caller to individually select a segment within a timer to be enabled,
//! such as TIMER0A, TIMER0B, or both.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_start(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t *pui32ConfigReg;
    uint32_t ui32ConfigVal;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section while config registers are read and modified.
    //
    AM_CRITICAL_BEGIN

    //
    // Read the current value.
    //
    ui32ConfigVal = *pui32ConfigReg;

    //
    // Clear out the "clear" bit.
    //
    ui32ConfigVal &= ~(ui32TimerSegment & (AM_REG_CTIMER_CTRL0_TMRA0CLR_M |
                                           AM_REG_CTIMER_CTRL0_TMRB0CLR_M));

    //
    // Set the "enable bit"
    //
    ui32ConfigVal |= (ui32TimerSegment & (AM_REG_CTIMER_CTRL0_TMRA0EN_M |
                                          AM_REG_CTIMER_CTRL0_TMRB0EN_M));

    //
    // Write the value back to the register.
    //
    AM_REGVAL(pui32ConfigReg) = ui32ConfigVal;

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_start()

//*****************************************************************************
//
//! @brief Stop a timer
//!
//! @param ui32TimerNumber is the number of the timer to disable.
//!
//! @param ui32TimerSegment specifies which segment of the timer should be
//! disabled.
//!
//! This function will stop the selected timer from incrementing. The \e
//! ui32TimerNumber parameter selects the timer that should be disabled, for
//! example, a 0 would target TIMER0. The \e ui32TimerSegment parameter allows
//! the caller to individually select a segment within a timer to be disabled,
//! such as TIMER0A, TIMER0B, or both.
//!
//! This function will stop a counter/timer from counting, but does not return
//! the count value to 'zero'. If you would like to reset the counter back to
//! zero, try the am_hal_ctimer_clear() function instead.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_stop(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t *pui32ConfigReg;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Clear the "enable" bit
    //
    AM_REGVAL(pui32ConfigReg) &= ~(ui32TimerSegment &
                                   (AM_REG_CTIMER_CTRL0_TMRA0EN_M |
                                    AM_REG_CTIMER_CTRL0_TMRB0EN_M));

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_stop()

//*****************************************************************************
//
//! @brief Stops a timer and resets its value back to zero.
//!
//! @param ui32TimerNumber is the number of the timer to clear.
//!
//! @param ui32TimerSegment specifies which segment of the timer should be
//! cleared.
//!
//! This function will stop a free-running counter-timer, reset its value to
//! zero, and leave the timer disabled. When you would like to restart the
//! counter, you will need to call am_hal_ctimer_start().
//!
//! The \e ui32TimerSegment parameter allows the caller to individually select
//! a segment within, such as TIMER0A, TIMER0B, or both.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return None.
//!
//!
//! @note Setting the CLR bit is necessary for completing timer initialization
//! including after MCU resets.
//!
//
//*****************************************************************************
void
am_hal_ctimer_clear(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t *pui32ConfigReg;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Set the "clear" bit
    //
    AM_REGVAL(pui32ConfigReg) |= (ui32TimerSegment &
                                  (AM_REG_CTIMER_CTRL0_TMRA0CLR_M |
                                   AM_REG_CTIMER_CTRL0_TMRB0CLR_M));

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_clear()

//*****************************************************************************
//
//! @brief Returns the current free-running value of the selected timer.
//!
//! @param ui32TimerNumber is the number of the timer to read.
//! @param ui32TimerSegment specifies which segment of the timer should be
//! read.
//!
//! This function returns the current free-running value of the selected timer.
//!
//! @note When reading from a linked timer, be sure to use AM_HAL_CTIMER both
//! for the segment argument.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return Current timer value.
//
//*****************************************************************************
uint32_t
am_hal_ctimer_read(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t ui32Value = 0;
    uint32_t ui32Values[4] = {0, };
    uint32_t ui32TimerAddrTbl[4] =
    {
        REG_CTIMER_BASEADDR + AM_REG_CTIMER_TMR0_O,
        REG_CTIMER_BASEADDR + AM_REG_CTIMER_TMR1_O,
        REG_CTIMER_BASEADDR + AM_REG_CTIMER_TMR2_O,
        REG_CTIMER_BASEADDR + AM_REG_CTIMER_TMR3_O
    };

    //
    // Read the timer with back2back reads. This is a workaround for a clock
    // domain synchronization issue. Some timer bits may be slow to increment,
    // which means that the value in the timer register will sometimes be
    // wrong.
    //
    // The architecture guarantees that:
    //
    // 1) If the timer is running at a speed close to the core frequency, the
    // core and timer clock domains will be synchronized, and no "bad" reads
    // will happen.
    //
    // 2) Bad reads will only happen if the core reads the timer register while
    // the timer value is transitioning from one count to the next.
    //
    // 3) The timer will resolve to the correct value within one 24 MHz clock
    // cycle.
    //
    // If we read the timer three times in a row with back-to-back load
    // instructions, then we can guarantee that the timer will only have time
    // to increment once, and that only one of the three reads can be wrong.
    // This routine will perform the back-to-back reads and return all three
    // values. The rest of this fuction determines which value we should
    // actually use.
    //
    am_hal_triple_read(ui32TimerAddrTbl[ui32TimerNumber], ui32Values);

    //
    // Shift or mask the values based on the given timer segment.
    //
    if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERB )
    {
        ui32Values[0] >>= 16;
        ui32Values[1] >>= 16;
        ui32Values[2] >>= 16;
    }
    else if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERA )
    {
        ui32Values[0] &= 0xFFFF;
        ui32Values[1] &= 0xFFFF;
        ui32Values[2] &= 0xFFFF;
    }

    //
    // Now, we'll figure out which of the three values is the correct time.
    //
    if (ui32Values[0] == ui32Values[1])
    {
        //
        // If the first two values match, then neither one was a bad read.
        // We'll take this as the current time.
        //
        ui32Value = ui32Values[1];
    }
    else
    {
        //
        // If the first two values didn't match, then one of them might be bad.
        // If one of the first two values is bad, then the third one should
        // always be correct. We'll take the third value as the correct time.
        //
        ui32Value = ui32Values[2];

        //
        // If all of the statements about the architecture are true, the third
        // value should be correct, and it should always be within one count of
        // either the first or the second value.
        //
        // Just in case, we'll check against the previous two values to make
        // sure that our final answer was reasonable. If it isn't, we will
        // flag it as a "bad read", and fail this assert statement.
        //
        // This shouldn't ever happen, and it hasn't ever happened in any of
        // our tests so far.
        //
        am_hal_debug_assert_msg((adjacent(ui32Values[1], ui32Values[2]) ||
                                 adjacent(ui32Values[0], ui32Values[2])),
                                "Bad CTIMER read");
    }

    return ui32Value;
} // am_hal_ctimer_read()

//*****************************************************************************
//
//! @brief Enable output to the timer pin
//!
//! @param ui32TimerNumber is the number of the timer to configure.
//!
//! @param ui32TimerSegment specifies which segment of the timer to use.
//!
//! This function will enable the output pin for the selected timer. The \e
//! ui32TimerSegment parameter allows the caller to individually select a
//! segment within, such as TIMER0A, TIMER0B, or both.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_pin_enable(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t *pui32ConfigReg;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Set the pin enable bit
    //
    AM_REGVAL(pui32ConfigReg) |= (ui32TimerSegment &
                                  (AM_REG_CTIMER_CTRL0_TMRA0PE_M |
                                   AM_REG_CTIMER_CTRL0_TMRB0PE_M));

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_pin_enable()

//*****************************************************************************
//
//! @brief Disable the output pin.
//!
//! @param ui32TimerNumber is the number of the timer to configure.
//!
//! @param ui32TimerSegment specifies which segment of the timer to use.
//!
//! This function will disable the output pin for the selected timer. The \e
//! ui32TimerSegment parameter allows the caller to individually select a
//! segment within, such as TIMER0A, TIMER0B, or both.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_pin_disable(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment)
{
    volatile uint32_t *pui32ConfigReg;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Clear the pin enable bit
    //
    AM_REGVAL(pui32ConfigReg) &= ~(ui32TimerSegment &
                                   (AM_REG_CTIMER_CTRL0_TMRA0PE_M |
                                    AM_REG_CTIMER_CTRL0_TMRB0PE_M));

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_pin_disable()

//*****************************************************************************
//
//! @brief Set the polarity of the output pin.
//!
//! @param ui32TimerNumber is the number of the timer to configure.
//!
//! @param ui32TimerSegment specifies which segment of the timer to use.
//!
//! @param bInvertOutput determines whether the output should be inverted. If
//! true, the timer output pin for the selected timer segment will be
//! inverted.
//!
//! This function will set the polarity of the the output pin for the selected
//! timer. The \e ui32TimerSegment parameter allows the caller to individually
//! select a segment within, such as TIMER0A, TIMER0B, or both.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_pin_invert(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment,
                         bool bInvertOutput)
{
    volatile uint32_t *pui32ConfigReg;

    //
    // Find the correct control register.
    //
    pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                  (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Figure out if we're supposed to be setting or clearing the polarity bit.
    //
    if ( bInvertOutput )
    {
        //
        // Set the polarity bit to invert the output.
        //
        AM_REGVAL(pui32ConfigReg) |= (ui32TimerSegment &
                                      (AM_REG_CTIMER_CTRL0_TMRA0POL_M |
                                       AM_REG_CTIMER_CTRL0_TMRB0POL_M));
    }
    else
    {
        //
        // Clear the polarity bit.
        //
        AM_REGVAL(pui32ConfigReg) &= ~(ui32TimerSegment &
                                       (AM_REG_CTIMER_CTRL0_TMRA0POL_M |
                                        AM_REG_CTIMER_CTRL0_TMRB0POL_M));
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_pin_invert()

//*****************************************************************************
//
//! @brief Set a compare register.
//!
//! @param ui32TimerNumber is the number of the timer to configure.
//!
//! @param ui32TimerSegment specifies which segment of the timer to use.
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @param ui32CompareReg specifies which compare register should be set
//! (either 0 or 1)
//!
//! @param ui32Value is the value that should be written to the compare
//! register.
//!
//! This function allows the caller to set the values in the compare registers
//! for a timer. These registers control the period and duty cycle of the
//! timers and their associated output pins. Please see the datasheet for
//! further information on the operation of the compare registers. The \e
//! ui32TimerSegment parameter allows the caller to individually select a
//! segment within, such as TIMER0A, TIMER0B, or both.
//!
//! @note For simple manipulations of period or duty cycle for timers and PWMs,
//! you may find it easier to use the am_hal_ctimer_period_set() function.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_compare_set(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment,
                          uint32_t ui32CompareReg, uint32_t ui32Value)
{
    volatile uint32_t *pui32CmprRegA, *pui32CmprRegB;
    uint32_t ui32CmprRegA, ui32CmprRegB, ui32ValB;

    //
    // Find the correct compare register to write.
    // Assume A or BOTH.  We'll change later if B.
    //
    pui32CmprRegA = (uint32_t *)(AM_REG_CTIMERn(0) +
                                 AM_REG_CTIMER_CMPRA0_O +
                                 (ui32TimerNumber * TIMER_OFFSET));
    pui32CmprRegB = pui32CmprRegA + CTIMER_CMPR_OFFSET / 4;

    ui32ValB = ( ui32TimerSegment == AM_HAL_CTIMER_BOTH ) ?
               ui32Value >> 16 : ui32Value & 0xFFFF;

    //
    // Write the compare register with the selected value.
    // Begin critical section while CMPR registers are modified.
    //
    AM_CRITICAL_BEGIN

    ui32CmprRegA = *pui32CmprRegA;
    ui32CmprRegB = *pui32CmprRegB;

    if ( ui32CompareReg == 1 )
    {
        //
        // CMPR reg 1
        // Get the lower 16b (but may not be used if TIMERB).
        //
        ui32CmprRegA = ( (ui32CmprRegA & AM_REG_CTIMER_CMPRA0_CMPR0A0_M) |
                          AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32Value & 0xFFFF) );

        //
        // Get the upper 16b (but may not be used if TIMERA)
        //
        ui32CmprRegB = ( (ui32CmprRegB & AM_REG_CTIMER_CMPRA0_CMPR0A0_M) |
                          AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32ValB) );
    }
    else
    {
        //
        // CMPR reg 0
        // Get the lower 16b (but may not be used if TIMERB)
        //
        ui32CmprRegA = ( (ui32CmprRegA & AM_REG_CTIMER_CMPRA0_CMPR1A0_M) |
                         AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32Value & 0xFFFF) );

        //
        // Set the upper 16b (but may not be used if TIMERA)
        //
        ui32CmprRegB = ( (ui32CmprRegB & AM_REG_CTIMER_CMPRA0_CMPR1A0_M) |
                         AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32ValB) );
    }

    if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERB )
    {
        *pui32CmprRegB = ui32CmprRegB;
    }
    else
    {
        //
        // It's TIMERA or BOTH.
        //
        *pui32CmprRegA = ui32CmprRegA;

        if ( ui32TimerSegment == AM_HAL_CTIMER_BOTH )
        {
            *pui32CmprRegB = ui32CmprRegB;
        }
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_compare_set()

//*****************************************************************************
//
//! @brief Set the period and duty cycle of a timer.
//!
//! @param ui32TimerNumber is the number of the timer to configure.
//!
//! @param ui32TimerSegment specifies which segment of the timer to use.
//!
//! @param ui32Period specifies the desired period.  This parameter effectively
//! specifies the CTIMER CMPR field(s). The CMPR fields are handled in hardware
//! as (n+1) values, therefore ui32Period is actually specified as 1 less than
//! the desired period. Finally, as mentioned in the data sheet, the CMPR fields
//! cannot be 0 (a value of 1), so neither can ui32Period be 0.
//!
//! @param ui32OnTime set the number of clocks where the output signal is high.
//!
//! This function should be used for simple manipulations of the period and
//! duty cycle of a counter/timer. To set the period and/or duty cycle of a
//! linked timer pair, use AM_HAL_CTIMER_BOTH as the timer segment argument. If
//! you would like to set the period and/or duty cycle for both TIMERA and
//! TIMERB you will need to call this function twice: once for TIMERA, and once
//! for TIMERB.
//!
//! Valid values for ui32TimerSegment are:
//!
//!     AM_HAL_CTIMER_TIMERA
//!     AM_HAL_CTIMER_TIMERB
//!     AM_HAL_CTIMER_BOTH
//!
//! @note The ui32OnTime parameter will only work if the timer is currently
//! operating in one of the PWM modes.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_period_set(uint32_t ui32TimerNumber, uint32_t ui32TimerSegment,
                         uint32_t ui32Period, uint32_t ui32OnTime)
{
    volatile uint32_t *pui32ControlReg;
    volatile uint32_t *pui32CompareRegA;
    volatile uint32_t *pui32CompareRegB;
    uint32_t ui32Mode, ui32Comp0, ui32Comp1;

    //
    // Find the correct control register to pull the function select field
    // from.
    //
    pui32ControlReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                   (ui32TimerNumber * TIMER_OFFSET));

    //
    // Find the correct compare registers to write.
    //
    pui32CompareRegA = (uint32_t *)(AM_REG_CTIMERn(0) +
                                    AM_REG_CTIMER_CMPRA0_O +
                                    (ui32TimerNumber * TIMER_OFFSET));

    pui32CompareRegB = (uint32_t *)(AM_REG_CTIMERn(0) +
                                    AM_REG_CTIMER_CMPRB0_O +
                                    (ui32TimerNumber * TIMER_OFFSET));

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Extract the timer mode from the register based on the ui32TimerSegment
    // selected by the user.
    //
    ui32Mode = *pui32ControlReg;
    if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERB )
    {
        ui32Mode = ui32Mode >> 16;
    }

    //
    // Mask to get to the bits we're interested in.
    //
    ui32Mode = ui32Mode & AM_REG_CTIMER_CTRL0_TMRA0FN_M;

    //
    // If the mode is a PWM mode, we'll need to calculate the correct CMPR0 and
    // CMPR1 values here.
    //
    if (ui32Mode == AM_HAL_CTIMER_FN_PWM_ONCE   ||
        ui32Mode == AM_HAL_CTIMER_FN_PWM_REPEAT)
    {
        ui32Comp0 = ui32Period - ui32OnTime;
        ui32Comp1 = ui32Period;
    }
    else
    {
        ui32Comp0 = ui32Period;
        ui32Comp1 = 0;
    }

    //
    // Based on the timer segment argument, write the calculated Compare 0 and
    // Compare 1 values to the correct halves of the correct registers.
    //
    if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERA )
    {
        //
        // For timer A, write the values to the TIMERA compare register.
        //
        *pui32CompareRegA = (AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32Comp0) |
                             AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32Comp1));
    }
    else if ( ui32TimerSegment == AM_HAL_CTIMER_TIMERB )
    {
        //
        // For timer B, write the values to the TIMERA compare register.
        //
        *pui32CompareRegB = (AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32Comp0) |
                             AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32Comp1));
    }
    else
    {
        //
        // For the linked case, write the lower halves of the values to the
        // TIMERA compare register, and the upper halves to the TIMERB compare
        // register.
        //
        *pui32CompareRegA = (AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32Comp0) |
                             AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32Comp1));

        *pui32CompareRegB = (AM_REG_CTIMER_CMPRA0_CMPR0A0(ui32Comp0 >> 16) |
                             AM_REG_CTIMER_CMPRA0_CMPR1A0(ui32Comp1 >> 16));
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_period_set()

//*****************************************************************************
//
//! @brief Enable the TIMERA3 ADC trigger
//!
//! This function enables the ADC trigger within TIMERA3.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_adc_trigger_enable(void)
{
    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Enable the ADC trigger.
    //
    AM_REGn(CTIMER, 0, CTRL3) |= AM_REG_CTIMER_CTRL3_ADCEN_M;

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_adc_trigger_enable()

//*****************************************************************************
//
//! @brief Disable the TIMERA3 ADC trigger
//!
//! This function disables the ADC trigger within TIMERA3.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_adc_trigger_disable(void)
{
    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Disable the ADC trigger.
    //
    AM_REGn(CTIMER, 0, CTRL3) &= ~AM_REG_CTIMER_CTRL3_ADCEN_M;

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_adc_trigger_disable()

//*****************************************************************************
//
//! @brief Enables the selected timer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will enable the selected interrupts in the main CTIMER
//! interrupt enable register. In order to receive an interrupt from a timer,
//! you will need to enable the interrupt for that timer in this main register,
//! as well as in the timer control register (accessible though
//! am_hal_ctimer_config()), and in the NVIC.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_enable(uint32_t ui32Interrupt)
{
    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Enable the interrupt at the module level.
    //
    AM_REGn(CTIMER, 0, INTEN) |= ui32Interrupt;

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_int_enable()

//*****************************************************************************
//
//! @brief Return the enabled timer interrupts.
//!
//! This function will return all enabled interrupts in the main CTIMER
//! interrupt enable register.
//!
//! @return return enabled interrupts. This will be a logical or of:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return Return the enabled timer interrupts.
//
//*****************************************************************************
uint32_t
am_hal_ctimer_int_enable_get(void)
{
    //
    // Return enabled interrupts.
    //
    return AM_REGn(CTIMER, 0, INTEN);
} // am_hal_ctimer_int_enable_get()

//*****************************************************************************
//
//! @brief Disables the selected timer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will disable the selected interrupts in the main CTIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_disable(uint32_t ui32Interrupt)
{
    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Disable the interrupt at the module level.
    //
    AM_REGn(CTIMER, 0, INTEN) &= ~ui32Interrupt;

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // am_hal_ctimer_int_disable()

//*****************************************************************************
//
//! @brief Clears the selected timer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will clear the selected interrupts in the main CTIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_clear(uint32_t ui32Interrupt)
{
    //
    // Disable the interrupt at the module level.
    //
    AM_REGn(CTIMER, 0, INTCLR) = ui32Interrupt;
} // am_hal_ctimer_int_clear()

//*****************************************************************************
//
//! @brief Sets the selected timer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will set the selected interrupts in the main CTIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_ctimer_int_set(uint32_t ui32Interrupt)
{
    //
    // Set the interrupts.
    //
    AM_REGn(CTIMER, 0, INTSET) = ui32Interrupt;
} // am_hal_ctimer_int_set()

//*****************************************************************************
//
//! @brief Returns either the enabled or raw timer interrupt status.
//!
//! This function will return the timer interrupt status.
//!
//! @return bEnabledOnly if true returns the status of the enabled interrupts
//! only.
//!
//! The return value will be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_CTIMER_INT_TIMERA0C0
//!     AM_HAL_CTIMER_INT_TIMERA0C1
//!     AM_HAL_CTIMER_INT_TIMERB0C0
//!     AM_HAL_CTIMER_INT_TIMERB0C1
//!     AM_HAL_CTIMER_INT_TIMERA1C0
//!     AM_HAL_CTIMER_INT_TIMERA1C1
//!     AM_HAL_CTIMER_INT_TIMERB1C0
//!     AM_HAL_CTIMER_INT_TIMERB1C1
//!     AM_HAL_CTIMER_INT_TIMERA2C0
//!     AM_HAL_CTIMER_INT_TIMERA2C1
//!     AM_HAL_CTIMER_INT_TIMERB2C0
//!     AM_HAL_CTIMER_INT_TIMERB2C1
//!     AM_HAL_CTIMER_INT_TIMERA3C0
//!     AM_HAL_CTIMER_INT_TIMERA3C1
//!     AM_HAL_CTIMER_INT_TIMERB3C0
//!     AM_HAL_CTIMER_INT_TIMERB3C1
//!
//! @return Returns either the timer interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_ctimer_int_status_get(bool bEnabledOnly)
{
    //
    // Return the desired status.
    //
    if ( bEnabledOnly )
    {
        uint32_t u32RetVal;

        //
        // Begin critical section.
        //
        AM_CRITICAL_BEGIN

        u32RetVal  = AM_REGn(CTIMER, 0, INTSTAT);
        u32RetVal &= AM_REGn(CTIMER, 0, INTEN);

        //
        // Done with critical section.
        //
        AM_CRITICAL_END

        return u32RetVal;
    }
    else
    {
        return AM_REGn(CTIMER, 0, INTSTAT);
    }
} // am_hal_ctimer_int_status_get()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
