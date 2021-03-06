/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_dma.h"
#include "fsl_common.h"
#include "fsl_lpadc.h"
#include "fsl_crc.h"
#include "fsl_ctimer.h"
#include "fsl_clock.h"
#include "fsl_rtc.h"
#include "fsl_reset.h"
#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_wwdt.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Used DMA device. */
#define DMA0_DMA_BASEADDR DMA0
/* Used DMA device. */
#define DMA1_DMA_BASEADDR DMA1
/* DMA1 interrupt vector ID (number). */
#define DMA1_IRQN DMA1_IRQn

  /* Channel CH0 definitions */
/* Selected DMA channel number. */
#define DMA1_CH0_DMA_CHANNEL 0
/* Alias for ADC0 peripheral */
#define ADC0_PERIPHERAL ADC0
/* ADC0 OFSTRIM_A value for the offset calibration */
#define ADC0_OFSTRIM_A 0U
/* ADC0 OFSTRIM_B value for the offset calibration */
#define ADC0_OFSTRIM_B 0U
/* Definition of peripheral ID */
#define CRC_ENGINE_PERIPHERAL ((CRC_Type *)CRC_ENGINE)
/* Definition of peripheral ID */
#define CTIMER0_PERIPHERAL CTIMER0
/* Timer tick frequency in Hz (input frequency of the timer) */
#define CTIMER0_TICK_FREQ 96000000UL
/* Timer tick period in ns (input period of the timer) */
#define CTIMER0_TICK_PERIOD 10UL
/* Definition of peripheral ID */
#define CTIMER1_PERIPHERAL CTIMER1
/* Timer tick frequency in Hz (input frequency of the timer) */
#define CTIMER1_TICK_FREQ 96000000UL
/* Timer tick period in ns (input period of the timer) */
#define CTIMER1_TICK_PERIOD 10UL
/* Definition of peripheral ID */
#define RTC_PERIPHERAL RTC
/* Definition of peripheral ID */
#define USART0_PERIPHERAL ((USART_Type *)FLEXCOMM0)
/* Definition of the clock source frequency */
#define USART0_CLOCK_SOURCE 48000000UL
/* Rx transfer buffer size. */
#define USART0_RX_BUFFER_SIZE 10
/* Rx transfer buffer size. */
#define USART0_TX_BUFFER_SIZE 10
/* Definition of peripheral ID */
#define USART2_PERIPHERAL ((USART_Type *)FLEXCOMM2)
/* Definition of the clock source frequency */
#define USART2_CLOCK_SOURCE 48000000UL
/* Selected DMA channel number. */
#define USART2_RX_DMA_CHANNEL 10
/* Used DMA device. */
#define USART2_RX_DMA_BASEADDR DMA0
/* Selected DMA channel number. */
#define USART2_TX_DMA_CHANNEL 11
/* Used DMA device. */
#define USART2_TX_DMA_BASEADDR DMA0
/* BOARD_InitPeripherals defines for WWDT */
/* Definition of peripheral ID */
#define WWDT_PERIPHERAL ((WWDT_Type *) WWDT_BASE)
/* Definition of the Watchdog Timer Window value */
#define WWDT_WINDOW 16777215UL
/* Definition of the Watchdog Timer Constant value */
#define WWDT_TIMEOUT 20479UL
/* Definition of the Watchdog Timer Warning Interrupt value */
#define WWDT_WARNING 1000UL
/* WWDT interrupt vector ID (number). */
#define WWDT_IRQN WDT_BOD_IRQn
/* WWDT interrupt handler identifier. */
#define WWDT_IRQHANDLER WDT_BOD_IRQHandler

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern dma_handle_t DMA1_CH0_Handle;
extern const lpadc_config_t ADC0_config;
extern lpadc_conv_command_config_t ADC0_commandsConfig[2];
extern lpadc_conv_trigger_config_t ADC0_triggersConfig[2];
extern const crc_config_t CRC_ENGINE_config;
extern const ctimer_config_t CTIMER0_config;
extern const ctimer_config_t CTIMER1_config;
extern const usart_config_t USART0_config;
extern usart_handle_t USART0_handle;
extern uint8_t USART0_rxBuffer[USART0_RX_BUFFER_SIZE];
extern const usart_transfer_t USART0_rxTransfer;
extern uint8_t USART0_txBuffer[USART0_TX_BUFFER_SIZE];
extern const usart_transfer_t USART0_txTransfer;
extern const usart_config_t USART2_config;
extern dma_handle_t USART2_RX_Handle;
extern dma_handle_t USART2_TX_Handle;
extern usart_dma_handle_t USART2_USART_DMA_Handle;
extern const wwdt_config_t WWDT_config;

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
