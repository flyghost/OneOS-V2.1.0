extern CRC_HandleTypeDef hcrc;
OS_HAL_DEVICE_DEFINE("CRC_HandleTypeDef", "crc", hcrc);

extern DMA2D_HandleTypeDef hdma2d;
OS_HAL_DEVICE_DEFINE("DMA2D_HandleTypeDef", "dma2d", hdma2d);

extern I2C_HandleTypeDef hi2c3;
struct stm32_i2c_info i2c3_info = {.instance = &hi2c3, .scl = 0x8, .sda = 0x29};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c3", i2c3_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LTDC_HandleTypeDef hltdc;
OS_HAL_DEVICE_DEFINE("LTDC_HandleTypeDef", "ltdc", hltdc);

extern RNG_HandleTypeDef hrng;
OS_HAL_DEVICE_DEFINE("RNG_HandleTypeDef", "rng", hrng);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SAI_HandleTypeDef hsai_BlockA1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA1", hsai_BlockA1);

extern DMA_HandleTypeDef hdma_sai1_a;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sai1_a", hdma_sai1_a);

extern SPI_HandleTypeDef hspi5;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi5", hspi5);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern TIM_HandleTypeDef htim3;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim3", htim3);

extern TIM_HandleTypeDef htim4;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim4", htim4);

extern TIM_HandleTypeDef htim5;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim5", htim5);

extern TIM_HandleTypeDef htim6;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim6", htim6);

extern TIM_HandleTypeDef htim7;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim7", htim7);

extern TIM_HandleTypeDef htim8;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim8", htim8);

extern TIM_HandleTypeDef htim9;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim9", htim9);

extern TIM_HandleTypeDef htim10;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim10", htim10);

extern TIM_HandleTypeDef htim11;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim11", htim11);

extern TIM_HandleTypeDef htim12;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim12", htim12);

extern TIM_HandleTypeDef htim13;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim13", htim13);

extern TIM_HandleTypeDef htim14;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim14", htim14);

extern UART_HandleTypeDef huart5;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart5", huart5);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern DMA_HandleTypeDef hdma_uart5_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart5_rx", hdma_uart5_rx);

extern DMA_HandleTypeDef hdma_uart5_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart5_tx", hdma_uart5_tx);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart1_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_tx", hdma_usart1_tx);

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_HS", hpcd_USB_OTG_HS);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

