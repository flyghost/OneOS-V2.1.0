static const struct mm32_rtc_info rtc_info = 
{
    .hrtc                   = rtc,
    .rtc_clk                = RCC_APB1ENR_PWR | RCC_APB1ENR_BKP,
    .rcc_init_func          = RCC_APB1PeriphClockCmd,
};
OS_HAL_DEVICE_DEFINE("RTC_TypeDef", "rtc", rtc_info);
