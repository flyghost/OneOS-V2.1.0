extern DAC_HandleTypeDef hdac1;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac1", hdac1);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern DMA_HandleTypeDef hdma_usart2_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart2_rx", hdma_usart2_rx);

extern DMA_HandleTypeDef hdma_usart3_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart3_rx", hdma_usart3_rx);

