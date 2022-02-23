extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x16, .sda = 0x6d};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern LPTIM_HandleTypeDef hlptim1;
OS_HAL_DEVICE_DEFINE("LPTIM_HandleTypeDef", "lptim1", hlptim1);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern SAI_HandleTypeDef hsai_BlockA1;
OS_HAL_DEVICE_DEFINE("SAI_HandleTypeDef", "sai_BlockA1", hsai_BlockA1);

extern SD_HandleTypeDef hsd1;
OS_HAL_DEVICE_DEFINE("SD_HandleTypeDef", "sd1", hsd1);

extern SPI_HandleTypeDef hspi2;
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi2", hspi2);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern UART_HandleTypeDef huart2;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", huart2);

extern UART_HandleTypeDef huart3;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", huart3);

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
OS_HAL_DEVICE_DEFINE("PCD_HandleTypeDef", "pcd_USB_OTG_FS", hpcd_USB_OTG_FS);

extern SRAM_HandleTypeDef hsram1;
OS_HAL_DEVICE_DEFINE("SRAM_HandleTypeDef", "sram1", hsram1);

