extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern ADC_HandleTypeDef hadc3;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc3", hadc3);

extern DMA2D_HandleTypeDef hdma2d;
OS_HAL_DEVICE_DEFINE("DMA2D_HandleTypeDef", "dma2d", hdma2d);

extern DSI_HandleTypeDef hdsi;
OS_HAL_DEVICE_DEFINE("DSI_HandleTypeDef", "dsi", hdsi);

extern ETH_HandleTypeDef heth;
OS_HAL_DEVICE_DEFINE("ETH_HandleTypeDef", "eth", heth);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x19};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern I2C_HandleTypeDef hi2c4;
struct stm32_i2c_info i2c4_info = {.instance = &hi2c4, .scl = 0x3c, .sda = 0x17};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c4", i2c4_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LTDC_HandleTypeDef hltdc;
OS_HAL_DEVICE_DEFINE("LTDC_HandleTypeDef", "ltdc", hltdc);

extern QSPI_HandleTypeDef hqspi;
OS_HAL_DEVICE_DEFINE("QSPI_HandleTypeDef", "qspi", hqspi);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SAI_HandleTypeDef hsai_BlockA1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA1", hsai_BlockA1);

extern SAI_HandleTypeDef hsai_BlockB1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockB1", hsai_BlockB1);

extern SAI_HandleTypeDef hsai_BlockA2;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA2", hsai_BlockA2);

extern SD_HandleTypeDef hsd2;
OS_HAL_DEVICE_DEFINE("SD_HandleTypeDef", "sd2", hsd2);

extern DMA_HandleTypeDef hdma_sdmmc2_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sdmmc2_rx", hdma_sdmmc2_rx);

extern DMA_HandleTypeDef hdma_sdmmc2_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sdmmc2_tx", hdma_sdmmc2_tx);

extern SPDIFRX_HandleTypeDef hspdif;
OS_HAL_DEVICE_DEFINE("SPDIFRX_HandleTypeDef", "spdif", hspdif);

extern TIM_HandleTypeDef htim3;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim3", htim3);

extern TIM_HandleTypeDef htim10;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim10", htim10);

extern TIM_HandleTypeDef htim11;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim11", htim11);

extern TIM_HandleTypeDef htim12;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim12", htim12);

extern UART_HandleTypeDef huart5;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart5", huart5);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart6;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart6", huart6);

extern DMA_HandleTypeDef hdma_uart5_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart5_rx", hdma_uart5_rx);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart6_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart6_rx", hdma_usart6_rx);

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_HS", hpcd_USB_OTG_HS);

extern WWDG_HandleTypeDef hwwdg;
OS_HAL_DEVICE_DEFINE("WWDG_HandleTypeDef", "wwdg", hwwdg);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

