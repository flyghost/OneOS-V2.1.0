#include "intc_pub.h"
#include "drv_uart.h"
#include "uart_pub.h"
#include "bus.h"
#include "os_drivers.h"
#include "drv_wlan.h"



#ifdef BEKEN_USING_UART1
struct beken_uart_info  uart1_info = {  .port  = UART1_PORT,
                                        .irqno = IRQ_UART1
                                     };

OS_HAL_DEVICE_DEFINE("Uart_Type","uart1", uart1_info);
#endif

#ifdef BEKEN_USING_UART2
struct beken_uart_info  uart2_info = {  .port  = UART2_PORT,
                                        .irqno = IRQ_UART2
                                     };

OS_HAL_DEVICE_DEFINE("Uart_Type","uart2", uart2_info);
#endif

#ifdef BEKEN_USING_WLAN

#ifdef BEKEN_USING_WLAN_STA
struct beken_wifi_info  wlan_sta_info = {  .mac   = {0},
                                           .state = 0,
                                           .mode  = 0,
                                           .work_mode = OS_WLAN_STATION
                                     };
OS_HAL_DEVICE_DEFINE("Wlan_Type",OS_WLAN_DEVICE_STA_NAME, wlan_sta_info);
#endif /* BEKEN_USING_WLAN_STA */

#ifdef BEKEN_USING_WLAN_AP
struct beken_wifi_info  wlan_ap_info = {   .mac   = {0},
                                           .state = 0,
                                           .mode  = 0,
                                           .work_mode = OS_WLAN_AP
                                       };
OS_HAL_DEVICE_DEFINE("Wlan_Type",OS_WLAN_DEVICE_AP_NAME, wlan_ap_info);
#endif /* #ifdef BEKEN_USING_WLAN_AP */

#endif








