#ifndef __BSP_H_
#define __BSP_H_

#include "os_types.h"
#include "board.h"

typedef struct __UART_HandleTypeDef
{
    os_uint32_t uart_device;
    os_uint32_t uart_interrupt;
    os_uint8_t *buff;
}UART_HandleTypeDef;


#if defined(BSP_USING_UART0)
extern UART_HandleTypeDef huart0;
#endif

#if defined(BSP_USING_UART1)
extern UART_HandleTypeDef huart1;
#endif

#endif /* __BSP_H_ */

