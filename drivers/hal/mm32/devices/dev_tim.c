#if BSP_USING_TIM1
static const struct mm32_timer_info tim1_info = 
{
    .htim                                                   = TIM1,
    .tim_clk                                                = RCC_APB2ENR_TIM1,
    .NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 3,
    .NVIC_InitStructure.NVIC_IRQChannel                     = TIM1_UP_IRQn,
    .NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 3,
    .NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("TIM_TypeDef", "tim1", tim1_info);
#endif

#if BSP_USING_TIM2
static const struct mm32_timer_info tim2_info = 
{
    .htim                                                   = TIM2,
    .tim_clk                                                = RCC_APB1ENR_TIM2,
    .NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 3,
    .NVIC_InitStructure.NVIC_IRQChannel                     = TIM2_IRQn,
    .NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 3,
    .NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE,
};
OS_HAL_DEVICE_DEFINE("TIM_TypeDef", "tim2", tim2_info);
#endif

