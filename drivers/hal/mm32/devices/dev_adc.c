#if BSP_USING_ADC1
static const struct mm32_adc_pin adc1_pin[] = 
{
    ADC_PIN_SET(A, 6, ADC_Samctl_55_5),
    ADC_PIN_SET(A, 7, ADC_Samctl_55_5),
};
static const struct mm32_adc_info adc1_info = 
{
    .hadc                                   = ADC1,
    .init_struct.ADC_Resolution             = ADC_Resolution_12b,        
    .init_struct.ADC_PRESCARE               = ADC_PCLK2_PRESCARE_16,
    .init_struct.ADC_Mode                   = ADC_CR_IMM,
    .init_struct.ADC_ContinuousConvMode     = DISABLE,
    .init_struct.ADC_ExternalTrigConv       = ADC_ExternalTrigConv_T1_CC1,
    .init_struct.ADC_DataAlign              = ADC_DataAlign_Right,
    .adc_rcc_clk                            = RCC_APB2ENR_ADC1,
    .adc_rcc_clkcmd                         = RCC_APB2PeriphClockCmd,
    .gpio_rcc_clkcmd                        = RCC_AHBPeriphClockCmd,
    .ref_low                                = 0,
    .ref_high                               = 3300,     /* ref 0 - 3.3v */
    .pin                                    = &adc1_pin[0],
    .pin_num                                = sizeof(adc1_pin) / sizeof(adc1_pin[0]),
};
OS_HAL_DEVICE_DEFINE("ADC_TypeDef", "adc1", adc1_info);
#endif

