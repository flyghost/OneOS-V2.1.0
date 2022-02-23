extern ADC_HandleTypeDef hadc1;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", hadc1);

extern ADC_HandleTypeDef hadc2;
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc2", hadc2);

extern DMA_HandleTypeDef hdma_adc1;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_adc1", hdma_adc1);

extern DAC_HandleTypeDef hdac;
OS_HAL_DEVICE_DEFINE("DAC_HandleTypeDef", "dac", hdac);

extern I2C_HandleTypeDef hi2c1;
struct stm32_i2c_info i2c1_info = {.instance = &hi2c1, .scl = 0x18, .sda = 0x19};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "hard_i2c1", i2c1_info);

extern IWDG_HandleTypeDef hiwdg;
OS_HAL_DEVICE_DEFINE("IWDG_HandleTypeDef", "iwdg", hiwdg);

extern RTC_HandleTypeDef hrtc;
OS_HAL_DEVICE_DEFINE("RTC_HandleTypeDef", "rtc", hrtc);

extern TIM_HandleTypeDef htim1;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim1", htim1);

extern TIM_HandleTypeDef htim2;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim2", htim2);

extern TIM_HandleTypeDef htim4;
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "tim4", htim4);

extern UART_HandleTypeDef huart4;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart4", huart4);

extern UART_HandleTypeDef huart1;
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", huart1);

extern DMA_HandleTypeDef hdma_uart4_rx;
OS_HAL_DEVICE_DEFINE("DMA_HandleTypeDef", "dma_uart4_rx", hdma_uart4_rx);

