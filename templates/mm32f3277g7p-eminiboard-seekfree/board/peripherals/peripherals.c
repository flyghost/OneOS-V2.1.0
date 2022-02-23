#include "oneos_config.h"
#include "drv_uart.h"
#include "drv_hwtimer.h"
#include "drv_adc.h"

#ifdef BSP_USING_UART
static const mm32_uart_info_t uart1_info = 
{
    .huart                                              = UART1,
    .uart_clk                                           = RCC_APB2Periph_UART1,
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
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 2,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
    .dma_info.RX_DMA_Mode                               = DMA_Mode_Circular,
    .dma_info.RX_DMA_Priority                           = DMA_Priority_Low,
    .dma_info.TX_DMA_Priority                           = DMA_Priority_Low,
//    .dma_tx_clk                                         = RCC_AHBENR_DMA1,
//    .dma_tx_channel                                     = DMA1_Channel4,
//    .dma_tx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel4_IRQn,
//    .dma_tx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
//    .dma_tx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
//    .dma_tx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
    .dma_rx_clk                                         = RCC_AHBENR_DMA1,
    .dma_rx_channel                                     = DMA1_Channel5,
    .dma_rx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel5_IRQn,
    .dma_rx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
    .dma_rx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
    .dma_rx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", uart1_info);

static const mm32_uart_info_t uart2_info = 
{
    .huart                                              = UART2,
    .uart_clk                                           = RCC_APB1Periph_UART2,
    .tx_pin_info.GPIO_Pin                               = GPIO_Pin_2,
    .tx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .tx_pin_info.GPIO_Mode                              = GPIO_Mode_AF_PP,
    .tx_pin_source                                      = GPIO_PinSource2,
    .tx_pin_port                                        = GPIOA,
    .rx_pin_info.GPIO_Pin                               = GPIO_Pin_3,
    .rx_pin_info.GPIO_Speed                             = GPIO_Speed_50MHz,
    .rx_pin_info.GPIO_Mode                              = GPIO_Mode_IPU,
    .rx_pin_source                                      = GPIO_PinSource3,
    .rx_pin_port                                        = GPIOA,
    .pin_af                                             = GPIO_AF_7,
    .uart_info.UART_BaudRate                            = 115200U,
    .uart_info.UART_WordLength                          = UART_WordLength_8b,
    .uart_info.UART_StopBits                            = UART_StopBits_1,
    .uart_info.UART_Parity                              = UART_Parity_No,
    .uart_info.UART_Mode                                = UART_Mode_Rx | UART_Mode_Tx,
    .uart_info.UART_HardwareFlowControl                 = UART_HardwareFlowControl_None,
    .uart_nvic_info.NVIC_IRQChannel                     = UART2_IRQn,
    .uart_nvic_info.NVIC_IRQChannelPreemptionPriority   = 0,
    .uart_nvic_info.NVIC_IRQChannelSubPriority          = 0,
    .uart_nvic_info.NVIC_IRQChannelCmd                  = ENABLE,
    .dma_info.RX_DMA_Mode                               = DMA_Mode_Circular,
    .dma_info.RX_DMA_Priority                           = DMA_Priority_Low,
    .dma_info.TX_DMA_Priority                           = DMA_Priority_Low,
//    .dma_tx_clk                                         = RCC_AHBENR_DMA1,
//    .dma_tx_channel                                     = DMA1_Channel7,
//    .dma_tx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel7_IRQn,
//    .dma_tx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
//    .dma_tx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
//    .dma_tx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
    .dma_rx_clk                                         = RCC_AHBENR_DMA1,
    .dma_rx_channel                                     = DMA1_Channel6,
    .dma_rx_nvic_info.NVIC_IRQChannel                   = DMA1_Channel6_IRQn,
    .dma_rx_nvic_info.NVIC_IRQChannelPreemptionPriority = 2,
    .dma_rx_nvic_info.NVIC_IRQChannelSubPriority        = 0,
    .dma_rx_nvic_info.NVIC_IRQChannelCmd                = ENABLE,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", uart2_info);
#endif

#ifdef BSP_USING_TIM
static const struct mm32_timer_info tim1_info = 
{
    .htim                                                   = TIM1,
    .tim_clk                                                = RCC_APB2ENR_TIM1,
    .NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 3,
    .NVIC_InitStructure.NVIC_IRQChannel                     = TIM1_UP_IRQn,
    .NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 3,
    .NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("TIM_TypeDef", "tim1", tim1_info);

static const struct mm32_timer_info tim2_info = 
{
    .htim                                                   = TIM2,
    .tim_clk                                                = RCC_APB1ENR_TIM2,
    .NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 3,
    .NVIC_InitStructure.NVIC_IRQChannel                     = TIM2_IRQn,
    .NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 3,
    .NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("TIM_TypeDef", "tim2", tim2_info);
#endif

#ifdef BSP_USING_ADC
static const struct mm32_adc_pin adc1_pin[] = 
{
    ADC_PIN_SET(A, 6, ADC_Samctl_55_5),
    ADC_PIN_SET(A, 7, ADC_Samctl_55_5),
};
static const struct mm32_adc_info adc1_info = 
{
    .hadc                                   = ADC1,
    .init_struct.ADC_Resolution             = ADC_Resolution_12b,        
    .init_struct.ADC_PRESCARE               = ADC_PCLK2_PRESCARE_16,
    .init_struct.ADC_Mode                   = ADC_CR_IMM,
    .init_struct.ADC_ContinuousConvMode     = DISABLE,
    .init_struct.ADC_ExternalTrigConv       = ADC_ExternalTrigConv_T1_CC1,
    .init_struct.ADC_DataAlign              = ADC_DataAlign_Right,
    .adc_rcc_clk                            = RCC_APB2ENR_ADC1,
    .adc_rcc_clkcmd                         = RCC_APB2PeriphClockCmd,
    .gpio_rcc_clkcmd                        = RCC_AHBPeriphClockCmd,
    .ref_low                                = 0,
    .ref_high                               = 3300,     /* ref 0 - 3.3v */
    .pin                                    = &adc1_pin[0],
    .pin_num                                = sizeof(adc1_pin) / sizeof(adc1_pin[0]),
};
OS_HAL_DEVICE_DEFINE("ADC_TypeDef", "adc1", adc1_info);
#endif




