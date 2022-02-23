
#include "bsp.h"
#include "board.h"

#if defined(BSP_USING_UART0)
UART_HandleTypeDef huart0 =
{
    AM_UART0_INST,
    AM_HAL_INTERRUPT_UART
};
#endif

#if defined(BSP_USING_UART1)
UART_HandleTypeDef huart1 =
{
    AM_UART1_INST,
    AM_HAL_INTERRUPT_UART1
};
#endif

