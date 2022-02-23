extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x17};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern I2S_HandleTypeDef hi2s2;
OS_HAL_DEVICE_DEFINE("I2S_HandleTypeDef", "i2s2", hi2s2);

extern DMA_HandleTypeDef hdma_spi2_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_spi2_tx", hdma_spi2_tx);

extern DMA_HandleTypeDef hdma_i2s2_ext_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_i2s2_ext_rx", hdma_i2s2_ext_rx);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi1;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", hspi1);

extern SPI_HandleTypeDef hspi3;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi3", hspi3);

extern SPI_HandleTypeDef hspi4;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi4", hspi4);

extern TIM_HandleTypeDef htim10;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim10", htim10);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern UART_HandleTypeDef huart6;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart6", huart6);

extern DMA_HandleTypeDef hdma_usart6_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart6_rx", hdma_usart6_rx);

extern DMA_HandleTypeDef hdma_usart6_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart6_tx", hdma_usart6_tx);

