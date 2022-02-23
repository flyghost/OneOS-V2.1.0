extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern CAN_HandleTypeDef hcan1;
OS_HAL_DEVICE_DEFINE("CAN_HandleTypeDef", "can1", hcan1);

extern DAC_HandleTypeDef hdac;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac", hdac);

extern ETH_HandleTypeDef heth;
OS_HAL_DEVICE_DEFINE("ETH_HandleTypeDef", "eth", heth);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x19};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LPTIM_HandleTypeDef hlptim1;
OS_HAL_DEVICE_DEFINE("LPTIM_HandleTypeDef", "lptim1", hlptim1);

extern QSPI_HandleTypeDef hqspi;
OS_HAL_DEVICE_DEFINE("QSPI_HandleTypeDef", "qspi", hqspi);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SAI_HandleTypeDef hsai_BlockA1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA1", hsai_BlockA1);

extern DMA_HandleTypeDef hdma_sai1_a;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sai1_a", hdma_sai1_a);

extern SD_HandleTypeDef hsd1;
OS_HAL_DEVICE_DEFINE("SD_HandleTypeDef", "sd1", hsd1);

extern DMA_HandleTypeDef hdma_sdmmc1;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sdmmc1", hdma_sdmmc1);

extern SPI_HandleTypeDef hspi3;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi3", hspi3);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern TIM_HandleTypeDef htim5;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim5", htim5);

extern TIM_HandleTypeDef htim13;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim13", htim13);

extern TIM_HandleTypeDef htim14;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim14", htim14);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart6;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart6", huart6);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart1_tx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_tx", hdma_usart1_tx);

extern DMA_HandleTypeDef hdma_usart6_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart6_rx", hdma_usart6_rx);

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_HS", hpcd_USB_OTG_HS);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

