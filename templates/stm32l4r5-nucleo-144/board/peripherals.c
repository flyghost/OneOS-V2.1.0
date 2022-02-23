extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern CAN_HandleTypeDef hcan1;
OS_HAL_DEVICE_DEFINE("CAN_HandleTypeDef", "can1", hcan1);

extern DAC_HandleTypeDef hdac1;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac1", hdac1);

extern DMA_HandleTypeDef hdma_dac1_ch1;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_dac1_ch1", hdma_dac1_ch1);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LPTIM_HandleTypeDef hlptim1;
OS_HAL_DEVICE_DEFINE("LPTIM_HandleTypeDef", "lptim1", hlptim1);

extern UART_HandleTypeDef hlpuart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "lpuart1", hlpuart1);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern DMA_HandleTypeDef hdma_lpuart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_lpuart1_rx", hdma_lpuart1_rx);

extern DMA_HandleTypeDef hdma_usart2_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart2_rx", hdma_usart2_rx);

extern DMA_HandleTypeDef hdma_usart3_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart3_rx", hdma_usart3_rx);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi1;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_FS", hpcd_USB_OTG_FS);

