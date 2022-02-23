////////////////////////////////////////////////////////////////////////////////
/// @file     reg_lptim.h
/// @author   AE TEAM
/// @brief    THIS FILE CONTAINS ALL THE FUNCTIONS PROTOTYPES FOR THE SERIES OF
///           MM32 FIRMWARE LIBRARY.
////////////////////////////////////////////////////////////////////////////////
/// @attention
///
/// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
/// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
/// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
/// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
/// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
/// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
///
/// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion

#ifndef __REG_LPTIM_H
#define __REG_LPTIM_H

// Files includes

#include <stdint.h>
#include <stdbool.h>
#include "types.h"




#if defined ( __CC_ARM )
#pragma anon_unions
#endif



////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM Base Address Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTIM_BASE                       (APB2PERIPH_BASE + 0x2800)              ///< Base Address: 0x40012800

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM Register Structure Definition
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    __IO u32  LPTCFG;                                                           ///< configuration register                                       offset: 0x00
    __IO u32  LPTIE;                                                            ///< interrupt enable register                                    offset: 0x04
    __IO u32  LPTIF;                                                            ///< interrupt flag register                                      offset: 0x08
    __IO u32  LPTCTRL;                                                          ///< control register                                             offset: 0x0C
    __IO u32  LPTCNT;                                                           ///< count register                                               offset: 0x10
    __IO u32  LPTCMP;                                                           ///< compare value register                                       offset: 0x14
    __IO u32  LPTTARGET;                                                        ///< target value register                                        offset: 0x18
} LPTIM_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM type pointer Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTIM                            ((LPTIM_TypeDef*) LPTIM_BASE)

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTCFG Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTCFG_FLTEN_Pos                 (15)
#define LPTCFG_FLTEN                     (0xFFFFU << LPTCFG_FLTEN_Pos)          ///< Input signal filtering enable
#define LPTCFG_DIVSEL_Pos                (8)
#define LPTCFG_DIVSEL_1                  (0x00U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 1  
#define LPTCFG_DIVSEL_2                  (0x01U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 2  
#define LPTCFG_DIVSEL_4                  (0x02U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 4  
#define LPTCFG_DIVSEL_8                  (0x03U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 8  
#define LPTCFG_DIVSEL_16                 (0x04U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 16 
#define LPTCFG_DIVSEL_32                 (0x05U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 32 
#define LPTCFG_DIVSEL_64                 (0x06U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 64 
#define LPTCFG_DIVSEL_128                (0x07U << LPTCFG_DIVSEL_Pos)           ///< Count clock divided by 128
#define LPTCFG_TRIGCFG_Pos               (6)
#define LPTCFG_TRIGCFG_Rise              (0x00U << LPTCFG_DIVSEL_Pos)           ///< External input signal rising edge trigger  
#define LPTCFG_TRIGCFG_Fall              (0x01U << LPTCFG_DIVSEL_Pos)           ///< External input signal falling edge trigger  
#define LPTCFG_TRIGCFG_Both              (0x02U << LPTCFG_DIVSEL_Pos)           ///< External input signal rising and falling edge trigger  
#define LPTCFG_TRIGSEL_Pos               (5)
#define LPTCFG_TRIGSEL                   (0x01U << LPTCFG_TRIGSEL_Pos)          ///< COMP output trigger
#define LPTCFG_POLARITY_Pos              (4)
#define LPTCFG_POLARITY                  (0x01U << LPTCFG_POLARITY_Pos)         ///< Compare matching waveform polarity to select negative polarity waveform
#define LPTCFG_PWM_Pos                   (3)
#define LPTCFG_PWM                       (0x01U << LPTCFG_PWM_Pos)              ///< PWM mode output
#define LPTCFG_TMODE_Pos                 (1)
#define LPTCFG_TMODE_0                   (0x00U << LPTCFG_TMODE_Pos)            ///< Normal timer mode with waveform output
#define LPTCFG_TMODE_1                   (0x01U << LPTCFG_TMODE_Pos)            ///< Trigger pulse trigger count mode
#define LPTCFG_TMODE_2                   (0x02U << LPTCFG_TMODE_Pos)            ///< External asynchronous pulse counting mode
#define LPTCFG_TMODE_3                   (0x03U << LPTCFG_TMODE_Pos)            ///< Timerout mode
#define LPTCFG_MODE_Pos                  (0)
#define LPTCFG_MODE                      (0x01U << LPTCFG_MODE_Pos)             ///< Single counting mode

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIE Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTIE_OVIE_Pos                   (0)
#define LPTIE_OVIE                       (0x01U << LPTIE_OVIE_Pos)              ///< Counter overflow interrupt enable
#define LPTIE_TRIGIE_Pos                 (1)
#define LPTIE_TRIGIE                     (0x01U << LPTIE_TRIGIE_Pos)            ///< Counter value and comparison value match interrupt enable
#define LPTIE_COMPIE_Pos                 (2)
#define LPTIE_COMPIE                     (0x01U << LPTIE_COMPIE_Pos)            ///< External trigger arrival interrupt enable

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIF Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTIF_OVIF_Pos                   (0)
#define LPTIF_OVIF                       (0x01U << LPTIF_OVIF_Pos)              ///< Counter overflow interrupt flag
#define LPTIF_TRIGIF_Pos                 (1)
#define LPTIF_TRIGIF                     (0x01U << LPTIF_TRIGIF_Pos)            ///< Counter value and comparison value match interrupt flag
#define LPTIF_COMPIF_Pos                 (2)
#define LPTIF_COMPIF                     (0x01U << LPTIF_COMPIF_Pos)            ///< External trigger arrival interrupt flag

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTCTRL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTCTRL_LPTEN_Pos                 (0)
#define LPTCTRL_LPTEN                     (0x01U << LPTCTRL_LPTEN_Pos)         ///< Enable counter count

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTCNT Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTCNT_CNT16_Pos                 (0)
#define LPTCNT_CNT16                     (0xFFFFU << LPTCNT_CNT16_Pos)         ///<  counter count value

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTCMP Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTCMP_COMPARE_val_Pos            (0)
#define LPTCMP_COMPARE_val                (0xFFFU << LPTCMP_COMPARE_val_Pos)     ///<compare value
////////////////////////////////////////////////////////////////////////////////
/// @brief LPTTARGET Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPTTARGET_Val_Pos                 (0)
#define LPTTARGET_Val                     (0xFFFU << LPTTARGET_Val_Pos)          ///<target value


/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////
