/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v8.0
processor: LPC55S26
package_id: LPC55S26JBD100
mcu_data: ksdk2_0
processor_version: 8.0.3
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: 4ee6b10f-6049-46b3-bcba-8ff7b946f43d
  called_from_default_init: true
  selectedCore: cm33_core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/

/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
component:
- type: 'system'
- type_id: 'system_54b53072540eeeb8f8e9343e71f28176'
- global_system_definitions:
  - user_definitions: ''
  - user_includes: ''
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * DMA0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'DMA0'
- type: 'lpc_dma'
- mode: 'basic'
- custom_name_enabled: 'false'
- type_id: 'lpc_dma_c13ca997a68f2ca6c666916ba13db7d7'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'DMA0'
- config_sets:
  - fsl_dma:
    - dma_table: []
    - dma_channels: []
    - init_interrupt: 'false'
    - dma_interrupt:
      - IRQn: 'DMA0_IRQn'
      - enable_interrrupt: 'enabled'
      - enable_priority: 'false'
      - priority: '0'
      - enable_custom_name: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/* Empty initialization function (commented out)
static void DMA0_init(void) {
} */

/***********************************************************************************************************************
 * DMA1 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'DMA1'
- type: 'lpc_dma'
- mode: 'basic'
- custom_name_enabled: 'false'
- type_id: 'lpc_dma_c13ca997a68f2ca6c666916ba13db7d7'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'DMA1'
- config_sets:
  - fsl_dma:
    - dma_table:
      - 0: []
    - dma_channels:
      - 0:
        - apiMode: 'trans'
        - dma_channel:
          - channel_prefix_id: 'CH0'
          - DMA_source: 'kDma1RequestHashCrypt'
          - init_channel_priority: 'false'
          - dma_priority: 'kDMA_ChannelPriority0'
          - enable_custom_name: 'false'
        - peripheral_request: 'false'
        - init_trigger_config: 'false'
        - trigger_config:
          - type: 'kDMA_NoTrigger'
          - burst: 'kDMA_SingleTransfer'
          - wrap: 'kDMA_NoWrap'
        - trans_config:
          - init_callback: 'false'
          - callback_function: ''
          - callback_user_data: ''
        - tcd_config: []
        - allocateTCD: 'noncache'
        - initTCD: 'noTCDInit'
    - dma_interrupt_trans:
      - IRQn: 'DMA1_IRQn'
      - enable_interrrupt: 'enabled'
      - enable_priority: 'false'
      - priority: '0'
    - quick_selection: 'default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

  /* Channel CH0 global variables */
dma_handle_t DMA1_CH0_Handle;

static void DMA1_init(void) {

  /* Channel CH0 initialization */
  /* Enable the DMA 0channel in the DMA */
  DMA_EnableChannel(DMA1_DMA_BASEADDR, DMA1_CH0_DMA_CHANNEL);
  /* Create the DMA DMA1_CH0_Handlehandle */
  DMA_CreateHandle(&DMA1_CH0_Handle, DMA1_DMA_BASEADDR, DMA1_CH0_DMA_CHANNEL);
}

/***********************************************************************************************************************
 * USART0 initialization code
 **********************************************************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'USART0'
- type: 'flexcomm_usart'
- mode: 'transfer'
- custom_name_enabled: 'true'
- type_id: 'flexcomm_usart_1a3fb4b775feaf430dd3789553461e5c'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FLEXCOMM0'
- config_sets:
  - transferCfg:
    - transfer:
      - init_rx_transfer: 'true'
      - rx_transfer:
        - data_size: '10'
      - init_tx_transfer: 'true'
      - tx_transfer:
        - data_size: '10'
      - init_callback: 'false'
      - callback_fcn: ''
      - user_data: ''
    - quick_selection: 'QuickSelection1'
  - usartConfig_t:
    - usartConfig:
      - clockSource: 'FXCOMFunctionClock'
      - clockSourceFreq: 'BOARD_BootClockRUN'
      - baudRate_Bps: '115200'
      - syncMode: 'kUSART_SyncModeDisabled'
      - parityMode: 'kUSART_ParityDisabled'
      - stopBitCount: 'kUSART_OneStopBit'
      - bitCountPerChar: 'kUSART_8BitsPerChar'
      - loopback: 'false'
      - txWatermark: 'kUSART_TxFifo0'
      - rxWatermark: 'kUSART_RxFifo1'
      - enableRx: 'true'
      - enableTx: 'true'
      - clockPolarity: 'kUSART_RxSampleOnFallingEdge'
      - enableContinuousSCLK: 'false'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */
const usart_config_t USART0_config = {
  .baudRate_Bps = 115200UL,
  .syncMode = kUSART_SyncModeDisabled,
  .parityMode = kUSART_ParityDisabled,
  .stopBitCount = kUSART_OneStopBit,
  .bitCountPerChar = kUSART_8BitsPerChar,
  .loopback = false,
  .txWatermark = kUSART_TxFifo0,
  .rxWatermark = kUSART_RxFifo1,
  .enableRx = true,
  .enableTx = true,
  .enableMode32k = false,
  .clockPolarity = kUSART_RxSampleOnFallingEdge,
  .enableContinuousSCLK = false
};
usart_handle_t USART0_handle;
uint8_t USART0_rxBuffer[USART0_RX_BUFFER_SIZE];
const usart_transfer_t USART0_rxTransfer = {
  .data = USART0_rxBuffer,
  .dataSize = USART0_RX_BUFFER_SIZE
};
uint8_t USART0_txBuffer[USART0_TX_BUFFER_SIZE];
const usart_transfer_t USART0_txTransfer = {
  .data = USART0_txBuffer,
  .dataSize = USART0_TX_BUFFER_SIZE
};

static void USART0_init(void) {
  /* Reset FLEXCOMM device */
  RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
  USART_Init(USART0_PERIPHERAL, &USART0_config, USART0_CLOCK_SOURCE);
  USART_TransferCreateHandle(USART0_PERIPHERAL, &USART0_handle, NULL, NULL);
}

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
  /* Global initialization */
  DMA_Init(DMA0_DMA_BASEADDR);
  DMA_Init(DMA1_DMA_BASEADDR);

  /* Initialize components */
  DMA1_init();
  USART0_init();
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
  BOARD_InitPeripherals();
}