#include "oneos_config.h"
#include "drv_uart.h"

#ifdef BSP_USING_UART
static const mm32_uart_info_t uart1_info = 
{
    .huart                                              = UART1,
    .uart_clk                                           = RCC_APB2ENR_UART1,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_9,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource9,
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_10,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource10,
    .rx_pin_port                                        = GPIOA,
    .pin_af                                             = GPIO_AF_7,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART1_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPriority             = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
    .dma_info.RX_DMA_Mode                               = DMA_Mode_Circular,
    .dma_info.RX_DMA_Priority                           = DMA_Priority_Low,
    .dma_info.TX_DMA_Priority                           = DMA_Priority_Low,
//    .dma_tx_clk                                         = RCC_AHBENR_DMA1,
//    .dma_tx_channel                                     = DMA1_Channel4,
//    .dma_tx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel4_5_IRQn,
//    .dma_tx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
//    .dma_tx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
//    .dma_tx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
    .dma_rx_clk                                         = RCC_AHBENR_DMA1,
    .dma_rx_channel                                     = DMA1_Channel5,
    .dma_rx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel4_5_IRQn,
    .dma_rx_nvic_info.NVIC_IRQChannelPriority           = 0,
    .dma_rx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", uart1_info);
#endif
