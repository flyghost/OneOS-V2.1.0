extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern CAN_HandleTypeDef hcan1;
OS_HAL_DEVICE_DEFINE("CAN_HandleTypeDef", "can1", hcan1);

extern CRC_HandleTypeDef hcrc;
OS_HAL_DEVICE_DEFINE("CRC_HandleTypeDef", "crc", hcrc);

extern DAC_HandleTypeDef hdac;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac", hdac);

extern ETH_HandleTypeDef heth;
OS_HAL_DEVICE_DEFINE("ETH_HandleTypeDef", "eth", heth);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x19};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RNG_HandleTypeDef hrng;
OS_HAL_DEVICE_DEFINE("RNG_HandleTypeDef", "rng", hrng);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SAI_HandleTypeDef hsai_BlockA1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA1", hsai_BlockA1);

extern DMA_HandleTypeDef hdma_sai1_a;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sai1_a", hdma_sai1_a);

extern SD_HandleTypeDef hsd;
OS_HAL_DEVICE_DEFINE("SD_HandleTypeDef", "sd", hsd);

extern DMA_HandleTypeDef hdma_sdio;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_sdio", hdma_sdio);

extern SPI_HandleTypeDef hspi5;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi5", hspi5);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart6;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart6", huart6);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart6_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart6_rx", hdma_usart6_rx);

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_HS", hpcd_USB_OTG_HS);

extern NAND_HandleTypeDef hnand1;
OS_HAL_DEVICE_DEFINE("NAND_HandleTypeDef", "nand1", hnand1);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

