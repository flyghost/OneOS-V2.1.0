////////////////////////////////////////////////////////////////////////////////
/// @file     reg_rtc.h
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

#ifndef __REG_RTC_H
#define __REG_RTC_H

// Files includes

#include <stdint.h>
#include <stdbool.h>
#include "types.h"




#if defined ( __CC_ARM )
#pragma anon_unions
#endif








////////////////////////////////////////////////////////////////////////////////
/// @brief RTC Base Address Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_BASE                    (0x40000000UL + 0x2800)              ///< Base Address: 0x40002800





////////////////////////////////////////////////////////////////////////////////
/// @brief RTC Registers Structure Definition
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    union {
        __IO u32 CR;                                                                ///< Control Register,                      offset: 0x00
        __IO u32 CRH;
    };
    union {
        __IO u32 CSR;                                                               ///< Control & Status Register,             offset: 0x04
        __IO u32 CRL;
    };
    __IO u32 PRLH;                                                              ///< Prescaler Reload Value High,           offset: 0x08
    __IO u32 PRLL;                                                              ///< Prescaler Reload Value Low,            offset: 0x0C
    __IO u32 DIVH;                                                              ///< Clock Divider High,                    offset: 0x10
    __IO u32 DIVL;                                                              ///< Clock Divider Low,                     offset: 0x14
    __IO u32 CNTH;                                                              ///< Counter High,                          offset: 0x18
    __IO u32 CNTL;                                                              ///< Counter Low,                           offset: 0x1C
    __IO u32 ALRH;                                                              ///< Alarm High,                            offset: 0x20
    __IO u32 ALRL;                                                              ///< Alarm Low,                             offset: 0x24
    __IO u32 MSRH;                                                              ///< Millisecond alarm high register        offset: 0x28
    __IO u32 MSRL;                                                              ///< Millisecond alarm low register         offset: 0x2C
} RTC_TypeDef;


////////////////////////////////////////////////////////////////////////////////
/// @brief RTC type pointer Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC                         ((RTC_TypeDef*)RTC_BASE)



////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_CR Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_CR_SECIE_Pos            (0)
#define RTC_CR_SECIE                (0x01U << RTC_CR_SECIE_Pos)             ///< Second Interrupt Enable
#define RTC_CR_ALRIE_Pos            (1)
#define RTC_CR_ALRIE                (0x01U << RTC_CR_ALRIE_Pos)             ///< Alarm Interrupt Enable
#define RTC_CR_OWIE_Pos             (2)
#define RTC_CR_OWIE                 (0x01U << RTC_CR_OWIE_Pos)              ///< OverfloW Interrupt Enable

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_CSR Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_CSR_SECF_Pos            (0)
#define RTC_CSR_SECF                (0x01 << RTC_CSR_SECF_Pos)              ///< Second Flag
#define RTC_CSR_ALRF_Pos            (1)
#define RTC_CSR_ALRF                (0x01 << RTC_CSR_ALRF_Pos)              ///< Alarm Flag
#define RTC_CSR_OWF_Pos             (2)
#define RTC_CSR_OWF                 (0x01 << RTC_CSR_OWF_Pos)               ///< OverfloW Flag
#define RTC_CSR_RSF_Pos             (3)
#define RTC_CSR_RSF                 (0x01 << RTC_CSR_RSF_Pos)               ///< Registers Synchronized Flag
#define RTC_CSR_CNF_Pos             (4)
#define RTC_CSR_CNF                 (0x01 << RTC_CSR_CNF_Pos)               ///< Configuration Flag
#define RTC_CSR_RTOFF_Pos           (5)
#define RTC_CSR_RTOFF               (0x01 << RTC_CSR_RTOFF_Pos)             ///< RTC operation OFF
#define RTC_CSR_ALPEN_Pos           (6)
#define RTC_CSR_ALPEN               (0x01 << RTC_CSR_ALPEN_Pos)             ///< RTC Alarm Loop Enable
////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_PRLH Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_PRLH_PRL_Pos            (0)
#define RTC_PRLH_PRL                (0x0F << RTC_PRLH_PRL_Pos)              ///< RTC Prescaler Reload Value High

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_PRLL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_PRLL_PRL_Pos            (0)
#define RTC_PRLL_PRL                (0xFFFFU << RTC_PRLL_PRL_Pos)           ///< RTC Prescaler Reload Value Low

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_DIVH Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_DIVH_DIV_Pos            (0)
#define RTC_DIVH_DIV                (0x0F << RTC_DIVH_DIV_Pos)              ///< RTC Clock Divider High

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_DIVL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_DIVL_DIV_Pos            (0)
#define RTC_DIVL_DIV                (0xFFFFU << RTC_DIVL_DIV_Pos)           ///< RTC Clock Divider Low

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_CNTH Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_CNTH_CNT_Pos            (0)
#define RTC_CNTH_CNT                (0xFFFFU << RTC_CNTH_CNT_Pos)           ///< RTC Counter High

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_CNTL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_CNTL_CNT_Pos            (0)
#define RTC_CNTL_CNT                (0xFFFFU << RTC_CNTL_CNT_Pos)           ///< RTC Counter Low

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_ALRH Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_ALRH_ALR_Pos            (0)
#define RTC_ALRH_ALR                (0xFFFFU << RTC_ALRH_ALR_Pos)           ///< RTC Alarm High

////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_ALRL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_ALRL_ALR_Pos            (0)
#define RTC_ALRL_ALR                (0xFFFFU << RTC_ALRL_ALR_Pos)           ///< RTC Alarm Low


////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_MSRH Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_MSRH_MSR_Pos            (0)
#define RTC_MSRH_MSR                (0xFFFFU << RTC_MSRH_MSR_Pos)          ///< RTC MS Alarm Register High


////////////////////////////////////////////////////////////////////////////////
/// @brief RTC_MSRL Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define RTC_MSRL_MSR_Pos            (0)
#define RTC_MSRL_MSR                (0xFFFFU << RTC_MSRL_MSR_Pos)           ///< RTC MS Alarm Register Low




/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////
