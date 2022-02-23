static const struct lpc_adc_info adc0_info = {ADC0_PERIPHERAL, &ADC0_config};
OS_HAL_DEVICE_DEFINE("ADC_Type", "adc0", adc0_info);

static const struct lpc_crc_engine_info crc_engine_info = {CRC_ENGINE_PERIPHERAL, &CRC_ENGINE_config};
OS_HAL_DEVICE_DEFINE("CRC_ENGINE_Type", "crc_engine", crc_engine_info);

static const struct lpc_ctimer_info ctimer0_info = {CTIMER0_PERIPHERAL, &CTIMER0_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer0", ctimer0_info);

static const struct lpc_ctimer_info ctimer1_info = {CTIMER1_PERIPHERAL, &CTIMER1_config};
OS_HAL_DEVICE_DEFINE("CTIMER_Type", "ctimer1", ctimer1_info);

static const struct lpc_rtc_info rtc_info = {RTC_PERIPHERAL, OS_NULL};
OS_HAL_DEVICE_DEFINE("RTC_Type", "rtc", rtc_info);

static const struct lpc_usart_info usart0_info = {USART0_PERIPHERAL, &USART0_config};
OS_HAL_DEVICE_DEFINE("USART_Type", "usart0", usart0_info);

static const struct lpc_usart_info usart2_info = {USART2_PERIPHERAL, &USART2_config};
OS_HAL_DEVICE_DEFINE("USART_Type", "usart2", usart2_info);

static const struct lpc_wwdt_info wwdt_info = {WWDT_PERIPHERAL, &WWDT_config};
OS_HAL_DEVICE_DEFINE("WWDT_Type", "wwdt", wwdt_info);

