extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern DAC_HandleTypeDef hdac1;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac1", hdac1);

extern I2C_HandleTypeDef hi2c5;
struct stm32_i2c_info i2c5_info = {.instance = &hi2c5, .scl = 0xb, .sda = 0xc};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c5", i2c5_info);

extern SPI_HandleTypeDef hspi1;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern TIM_HandleTypeDef htim6;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim6", htim6);

extern TIM_HandleTypeDef htim7;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim7", htim7);

extern TIM_HandleTypeDef htim8;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim8", htim8);

extern UART_HandleTypeDef huart5;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart5", huart5);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern DMA_HandleTypeDef hdma_uart5_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart5_rx", hdma_uart5_rx);

extern DMA_HandleTypeDef hdma_usart3_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart3_rx", hdma_usart3_rx);

extern WWDG_HandleTypeDef hwwdg1;
OS_HAL_DEVICE_DEFINE("WWDG_HandleTypeDef", "wwdg1", hwwdg1);

