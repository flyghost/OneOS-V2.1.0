////////////////////////////////////////////////////////////////////////////////
/// @file     reg_lpuart.h
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

#ifndef __REG_LPUART_H
#define __REG_LPUART_H

// Files includes

#include <stdint.h>
#include <stdbool.h>
#include "types.h"




#if defined ( __CC_ARM )
#pragma anon_unions
#endif



////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART Base Address Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_BASE                       (APB2PERIPH_BASE + 0x0800)              ///< Base Address: 0x40010800

////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART Register Structure Definition
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    __IO u32 LPUBAUD;                                                          ///< Baud rate register                                offset: 0x00
    __IO u32 MODU;                                                             ///< Baud rate modulation control register             offset: 0x04
    __IO u32 LPUIF;                                                            ///< Interrupt flag register                           offset: 0x08
    __IO u32 LPUSTA;                                                           ///< Status register                                   offset: 0x0C
    __IO u32 LPUCON;                                                           ///< Control register                                  offset: 0x10
    __IO u32 LPUEN;                                                            ///< Receive enable register                           offset: 0x14
    __IO u32 LPURXD;                                                           ///< Receive data register                             offset: 0x18
    __IO u32 LPUTXD;                                                           ///< Send data register                                offset: 0x1C
    __IO u32 COMPARE;                                                          ///< Data match register                               offset: 0x20

} LPUART_TypeDef;


////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART type pointer Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART                            ((LPUART_TypeDef*) LPUART_BASE)

////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPURXD Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPURXD_DATA_Pos             (0)
#define LPUART_LPURXD_DATA                 (0xFFU << LPUART_LPURXD_DATA_Pos)      ///< Receive data buffer
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUTXD Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPUTXD_DATA_Pos             (0)
#define LPUART_LPUTXD_DATA                 (0xFFU << LPUART_LPUTXD_DATA_Pos)      ///< send data buffer
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUSTA Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPUSTA_START_Pos             (7)
#define LPUART_LPUSTA_START                 (0x01U << LPUART_LPUSTA_START_Pos)    ///< Start bit detection flag 
#define LPUART_LPUSTA_PERR_Pos              (6)
#define LPUART_LPUSTA_PERR                  (0x01U << LPUART_LPUSTA_PERR_Pos)     ///<  Check digit error
#define LPUART_LPUSTA_TC_Pos                (5)
#define LPUART_LPUSTA_TC                    (0x01U << LPUART_LPUSTA_TC_Pos)       ///<  Send completed flag
#define LPUART_LPUSTA_TXE_Pos               (4)
#define LPUART_LPUSTA_TXE                   (0x01U << LPUART_LPUSTA_TXE_Pos)      ///<  Send buffer empty flag
#define LPUART_LPUSTA_RXF_Pos               (3)
#define LPUART_LPUSTA_RXF                   (0x01U << LPUART_LPUSTA_RXF_Pos)      ///<  Receive buffer full
#define LPUART_LPUSTA_MATCH_Pos             (2)
#define LPUART_LPUSTA_MATCH                 (0x01U << LPUART_LPUSTA_MATCH_Pos)    ///<  Data match flag
#define LPUART_LPUSTA_FERR_Pos              (1)
#define LPUART_LPUSTA_FERR                  (0x01U << LPUART_LPUSTA_FERR_Pos)     ///<  Frame format error
#define LPUART_LPUSTA_RXOV_Pos              (0)
#define LPUART_LPUSTA_RXOV                  (0x01U << LPUART_LPUSTA_RXOV_Pos)     ///<  Receive buffer overflow

////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUCON Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPUCON_TXPOL_Pos             (12)
#define LPUART_LPUCON_TXPOL                 (0x01U << LPUART_LPUCON_TXPOL_Pos)     ///< Data transmission polarity 
#define LPUART_LPUCON_RXPOL_Pos             (11)
#define LPUART_LPUCON_RXPOL                 (0x01U << LPUART_LPUCON_RXPOL_Pos)     ///<  Data receive polarity
#define LPUART_LPUCON_PAREN_Pos             (10)
#define LPUART_LPUCON_PAREN                 (0x01U << LPUART_LPUCON_PAREN_Pos)     ///<  Check bit enable
#define LPUART_LPUCON_PTYP_Pos              (9)
#define LPUART_LPUCON_PTYP                  (0x01U << LPUART_LPUCON_PTYP_Pos)      ///<  Odd parity
#define LPUART_LPUCON_SL_Pos                (8)
#define LPUART_LPUCON_SL                    (0x01U << LPUART_LPUCON_SL_Pos)        ///< Stop 2 bit length
#define LPUART_LPUCON_DL_Pos                (7)
#define LPUART_LPUCON_DL                    (0x01U << LPUART_LPUCON_DL_Pos)        ///< Data length is 7bits 
#define LPUART_LPUCON_RXEV_Pos              (5)
#define LPUART_LPUCON_RXEV_0                (0x00U << LPUART_LPUCON_RXEV_Pos)     ///<  START bit detection wake-up
#define LPUART_LPUCON_RXEV_1                (0x01U << LPUART_LPUCON_RXEV_Pos)     ///<  1byte data reception completed
#define LPUART_LPUCON_RXEV_2                (0x02U << LPUART_LPUCON_RXEV_Pos)     ///<  Received data matched successfully
#define LPUART_LPUCON_RXEV_3                (0x03U << LPUART_LPUCON_RXEV_Pos)     ///<  Falling edge detection wake up

#define LPUART_LPUCON_ERRIE_Pos             (4)
#define LPUART_LPUCON_ERRIE                 (0x01U << LPUART_LPUCON_ERRIE_Pos)     ///<  Error interrupt enable
#define LPUART_LPUCON_TCIE_Pos              (3)
#define LPUART_LPUCON_TCIE                  (0x01U << LPUART_LPUCON_TCIE_Pos)      ///<  Send complete interrupt enable
#define LPUART_LPUCON_TXIE_Pos              (2)
#define LPUART_LPUCON_TXIE                  (0x01U << LPUART_LPUCON_TXIE_Pos)      ///<  Send buffer empty interrupt enable
#define LPUART_LPUCON_NEDET_Pos             (1)
#define LPUART_LPUCON_NEDET                 (0x01U << LPUART_LPUCON_NEDET_Pos)     ///<  Falling edge sampling enable bit
#define LPUART_LPUCON_RXIE_Pos              (0)
#define LPUART_LPUCON_RXIE                  (0x01U << LPUART_LPUCON_RXIE_Pos)      ///<  Receive interrupt enable
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUIF Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPUIF_TCIF_Pos               (3)
#define LPUART_LPUIF_TCIF                   (0x01U << LPUART_LPUIF_TCIF_Pos)      ///<  Send complete interrupt complete flag
#define LPUART_LPUIF_RXNEGIF_Pos            (2)
#define LPUART_LPUIF_RXNEGIF                (0x01U << LPUART_LPUIF_RXNEGIF_Pos)   ///< RXD Falling edge interrupt flag
#define LPUART_LPUIF_TXIF_Pos               (1)
#define LPUART_LPUIF_TXIF                   (0x01U << LPUART_LPUIF_TXIF_Pos)     ///<  Send buffer empty interrupt flag
#define LPUART_LPUIF_RXIE_Pos               (0)
#define LPUART_LPUIF_RXIE                   (0x01U << LPUART_LPUCON_RXIE_Pos)    ///<  Receive interrupt enable
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUBAUD Register Bit Definition
////////////////////////////////////////////////////////////////////////////////

#define LPUART_LPUBAUD_BAUD_Pos              (5)
#define LPUART_LPUBAUD_BAUD_9600             (0x00U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 9600 bps
#define LPUART_LPUBAUD_BAUD_4800             (0x01U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 4800 bps
#define LPUART_LPUBAUD_BAUD_2400             (0x02U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 2400 bps
#define LPUART_LPUBAUD_BAUD_1200             (0x03U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 1200 bps
#define LPUART_LPUBAUD_BAUD_600              (0x04U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 600  bps
#define LPUART_LPUBAUD_BAUD_300              (0x05U << LPUART_LPUBAUD_BAUD_Pos)     ///<  Baud is 300  bps
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_LPUEN Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_LPUEN_DMAR_Pos               (3)
#define LPUART_LPUEN_DMAR                   (0x01U << LPUART_LPUEN_DMAR_Pos)      ///< DMA receive enable
#define LPUART_LPUEN_DAMT_Pos               (2)
#define LPUART_LPUEN_DAMT                   (0x01U << LPUART_LPUEN_DAMT_Pos)     ///< DMA transmission enable
#define LPUART_LPUEN_RXEN_Pos               (1)
#define LPUART_LPUEN_RXEN                   (0x01U << LPUART_LPUEN_RXEN_Pos)     ///< receive enable
#define LPUART_LPUEN_TXEN_Pos               (0)
#define LPUART_LPUEN_TXEN                   (0x01U << LPUART_LPUEN_TXEN_Pos)     ///< transmission enable
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_COMPARE Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_COMPARE_Pos                  (3)
#define LPUART_COMPARE                      (0xFFU << LPUART_COMPARE_Pos)       ///< compare data
////////////////////////////////////////////////////////////////////////////////
/// @brief LPUART_MODU Register Bit Definition
////////////////////////////////////////////////////////////////////////////////
#define LPUART_MODU_MCTL_Pos                (3)
#define LPUART_MODU_MCTL                    (0xFFFU << LPUART_MODU_MCTL_Pos)    ///< bit modulation control signal



/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////
