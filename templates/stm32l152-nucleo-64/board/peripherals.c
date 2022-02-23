extern ADC_HandleTypeDef hadc;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc", hadc);

extern DAC_HandleTypeDef hdac;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac", hdac);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi2;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi2", hspi2);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern TIM_HandleTypeDef htim9;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim9", htim9);

extern TIM_HandleTypeDef htim10;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim10", htim10);

extern TIM_HandleTypeDef htim11;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim11", htim11);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

