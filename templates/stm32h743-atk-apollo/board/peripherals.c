extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern ADC_HandleTypeDef hadc2;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc2", hadc2);

extern DAC_HandleTypeDef hdac1;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac1", hdac1);

extern HRTIM_HandleTypeDef hhrtim;
OS_HAL_DEVICE_DEFINE("HRTIM_HandleTypeDef", "hrtim", hhrtim);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x17};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg1;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg1", hiwdg1);

extern QSPI_HandleTypeDef hqspi;
OS_HAL_DEVICE_DEFINE("QSPI_HandleTypeDef", "qspi", hqspi);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SPI_HandleTypeDef hspi4;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi4", hspi4);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim14;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim14", htim14);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern DMA_HandleTypeDef hdma_usart1_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart1_rx", hdma_usart1_rx);

extern DMA_HandleTypeDef hdma_usart3_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_usart3_rx", hdma_usart3_rx);

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
struct stm32_pcd_info pcd_USB_OTG_FS_info = {.instance = &hpcd_USB_OTG_FS, .interface_type = "PCD_USB_OTG_FS"};
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "hard_pcd_USB_OTG_FS", pcd_USB_OTG_FS_info);

extern SDRAM_HandleTypeDef hsdram1;
OS_HAL_DEVICE_DEFINE("SDRAM_HandleTypeDef", "sdram1", hsdram1);

