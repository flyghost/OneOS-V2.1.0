#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_ATK_PANDORA
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M7

/* Kernel */

#define OS_NAME_MAX_15
#define OS_NAME_MAX 15
#define OS_TASK_PRIORITY_32
#define OS_TASK_PRIORITY_MAX 32
#define OS_TICK_PER_SECOND 100
#define OS_SCHEDULE_TIME_SLICE 10
#define OS_USING_OVERFLOW_CHECK
#define OS_USING_ASSERT
#define OS_USING_KERNEL_LOCK_CHECK
#define OS_USING_KERNEL_DEBUG
#define KLOG_GLOBAL_LEVEL_WARNING
#define KLOG_GLOBAL_LEVEL 1
#define KLOG_USING_COLOR
#define KLOG_WITH_FUNC_LINE
#define OS_MAIN_TASK_STACK_SIZE 2048
#define OS_IDLE_TASK_STACK_SIZE 512
#define OS_RECYCLE_TASK_STACK_SIZE 512
#define OS_USING_TIMER
#define OS_TIMER_TASK_STACK_SIZE 512
#define OS_USING_WORKQUEUE
#define OS_USING_SYSTEM_WORKQUEUE
#define OS_SYSTEM_WORKQUEUE_STACK_SIZE 2048
#define OS_SYSTEM_WORKQUEUE_PRIORITY 8

/* Inter-task communication and synchronization */

#define OS_USING_MUTEX
#define OS_USING_SEMAPHORE
#define OS_USING_EVENT
#define OS_USING_MESSAGEQUEUE
#define OS_USING_MAILBOX
/* end of Inter-task communication and synchronization */

/* Memory management */

#define OS_USING_SYS_HEAP
#define OS_USING_MEM_HEAP
#define OS_USING_ALG_FIRSTFIT
#define OS_USING_MEM_POOL
/* end of Memory management */
/* end of Kernel */

/* C standard library adapter */

#define OS_USING_LIBC_ADAPTER
#define OS_USING_NEWLIB_ADAPTER
#define OS_USING_ARMCCLIB_ADAPTER
/* end of C standard library adapter */

/* Osal */

/* POSIX compatibility layer */

/* end of POSIX compatibility layer */

/* RT-Thread compatibility layer */

/* end of RT-Thread compatibility layer */

/* CMSIS compatibility layer */

#define OS_USING_CMSIS_RTOS2_API_V2_1_2
/* end of CMSIS compatibility layer */

/* FreeRTOS compatibility layer */

#define OS_USING_FREERTOS_API_V10_4_3
/* end of FreeRTOS compatibility layer */
/* end of Osal */

/* Drivers */

#define OS_USING_DEVICE
#define OS_USING_DEVICE_NOTIFY

/* Audio */

/* end of Audio */

/* BLOCK */

#define OS_USING_BLOCK
/* end of BLOCK */

/* Boot */

/* CORTEX-M Boot */

#define BSP_INCLUDE_VECTOR_TABLE
#define BSP_SYSTEM_STACK_SIZE 512
/* end of CORTEX-M Boot */
/* end of Boot */

/* CAN */

/* end of CAN */

/* CONSOLE */

#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart1"
/* end of CONSOLE */

/* DMA */

#define OS_USING_DMA
#define OS_USING_DMA_RAM
#define OS_USING_SOFT_DMA
#define OS_SOFT_DMA_SUPPORT_NORMAL_MODE
#define OS_SOFT_DMA_SUPPORT_CIRCLE_MODE
/* end of DMA */

/* FAL */

#define OS_USING_FAL
/* end of FAL */

/* Graphic */

/* end of Graphic */

/* HAL */

#define MANUFACTOR_STM32
#define SERIES_STM32H7
#define SERIES_STM32H7_M7
#define SOC_STM32H747xx_CM7

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* HwCrypto */

/* end of HwCrypto */

/* I2C */

#define OS_USING_I2C
/* end of I2C */

/* Infrared */

/* end of Infrared */

/* LPMGR */

/* end of LPMGR */

/* MISC */

#define OS_USING_PUSH_BUTTON
#define OS_USING_LED
#define OS_USING_ADC
#define OS_USING_DAC
/* end of MISC */

/* MTD */

#define OS_USING_MTD
/* end of MTD */

/* NAND */

/* end of NAND */

/* PIN */

#define OS_USING_PIN
#define OS_PIN_MAX_CHIP 1
/* end of PIN */

/* RTC */

#define OS_USING_RTC
#define OS_RTC_DEV_NAME "rtc"
/* end of RTC */

/* SDIO */

/* end of SDIO */

/* Sensors */

/* end of Sensors */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_DELAY_CLOSE
#define OS_SERIAL_RX_BUFSZ 512
#define OS_SERIAL_TX_BUFSZ 512

/* posix serial */

/* end of posix serial */

/* rtt uart */

/* end of rtt uart */
/* end of Serial */

/* SN */

/* end of SN */

/* SPI */

#define OS_USING_SPI
#define OS_USING_QSPI
#define OS_USING_SFUD
#define OS_SFUD_USING_SFDP
#define OS_SFUD_USING_FLASH_INFO_TABLE
/* end of SPI */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_CLOCKEVENT
#define OS_USING_HRTIMER
#define OS_USING_HRTIMER_FOR_KERNEL_TICK

/* cortex-m hardware timer config */

#define OS_USING_SYSTICK_FOR_CLOCKSOURCE
/* end of cortex-m hardware timer config */
/* end of Timer */

/* Touch */

/* end of Touch */

/* USB */

#define OS_USING_USB_HOST
#define USB_USING_HOST_HS
#define ENABLE_USBH_HID_CLASS
#define ENABLE_USBH_SPECIFIC_CDC_CLASS
#define OS_USBD_TASK_STACK_SZ 4096
/* end of USB */

/* WDG */

#define OS_USING_WDG
/* end of WDG */
/* end of Drivers */

/* Components */

/* AMS */

/* end of AMS */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
/* end of Atest */

/* BLE */

/* end of BLE */

/* CLI */

/* end of CLI */

/* Cloud */

/* Aliyun */

/* end of Aliyun */

/* AWS */

/* end of AWS */

/* Baidu */

/* end of Baidu */

/* CTWing */

/* MQTT */

/* end of MQTT */
/* end of CTWing */

/* Huawei */

/* end of Huawei */

/* OneNET */

/* MQTT kit */

/* end of MQTT kit */

/* NB-IoT kit */

/* end of NB-IoT kit */

/* EDP */

/* end of EDP */
/* end of OneNET */
/* end of Cloud */

/* CMS */

/* CMS Connect */

/* end of CMS Connect */

/* CMS ID */

/* end of CMS ID */
/* end of CMS */

/* Diagnose */


/* eCoreDump */

/* end of eCoreDump */
/* end of Diagnose */

/* Dlog */

#define OS_USING_DLOG
#define DLOG_PRINT_LVL_I
#define DLOG_GLOBAL_PRINT_LEVEL 6
#define DLOG_COMPILE_LVL_D
#define DLOG_COMPILE_LEVEL 7
#define DLOG_USING_ISR_LOG
#define DLOG_USING_FILTER

/* Log format */

#define DLOG_WITH_FUNC_LINE
#define DLOG_USING_COLOR
#define DLOG_OUTPUT_TIME_INFO
/* end of Log format */

/* Dlog backend option */

#define DLOG_BACKEND_USING_CONSOLE
/* end of Dlog backend option */
/* end of Dlog */

/* FileSystem */

/* end of FileSystem */

/* Industrial */

/* CANOpen */

/* end of CANOpen */
/* end of Industrial */

/* Network */

/* Acw */

/* end of Acw */

/* TCP/IP */

/* LwIP */

/* end of LwIP */
/* end of TCP/IP */

/* Molink */

#define NET_USING_MOLINK
#define MOLINK_USING_GENERAL_OPS
#define MOLINK_USING_NETSERV_OPS
#define MOLINK_USING_PING_OPS
#define MOLINK_USING_IFCONFIG_OPS
#define MOLINK_USING_NETCONN_OPS
#define MOLINK_USING_SOCKETS_OPS
#define MOLINK_USING_IP
#define MOLINK_USING_IPV4
#define MOLINK_PLATFORM_MCU

/* Modules */

/* 4G CAT1 Modules Support */

/* end of 4G CAT1 Modules Support */

/* 4G CAT4 Modules Support */

#define MOLINK_USING_EC20

/* EC20 Config */

#define EC20_AUTO_CREATE
#define EC20_DEVICE_NAME "usbh_cdc_hs"
#define EC20_DEVICE_RATE 115200
#define EC20_RECV_BUFF_LEN 512
#define EC20_USING_GENERAL_OPS
#define EC20_USING_NETSERV_OPS
#define EC20_USING_PING_OPS
#define EC20_USING_IFCONFIG_OPS
#define EC20_USING_NETCONN_OPS
#define EC20_USING_SOCKETS_OPS
/* end of EC20 Config */
/* end of 4G CAT4 Modules Support */

/* 5G Modules Support */

/* end of 5G Modules Support */

/* NB-IOT Modules Support */

/* end of NB-IOT Modules Support */

/* WiFi Modules Support */

/* end of WiFi Modules Support */
/* end of Modules */

/* Parser */

#define MOLINK_USING_PARSER
#define AT_PARSER_TASK_STACK_SIZE 2048
#define AT_PARSER_PRINT_RAW
/* end of Parser */

/* Test */

/* end of Test */

/* Tools */

#define MOLINK_USING_TOOLS
#define MOLINK_TOOLS_USING_IFCONFIG
#define MOLINK_TOOLS_USING_PING
#define MO_PING_MAX_TIMES_CONFIG 50
#define MOLINK_TOOLS_USING_SOCKETSTAT
/* end of Tools */
/* end of Molink */

/* Protocols */

/* CoAP */

/* libcoap-v4.2.1 */

/* end of libcoap-v4.2.1 */
/* end of CoAP */

/* HTTP */

/* httpclient-v1.1.0 */

/* end of httpclient-v1.1.0 */
/* end of HTTP */

/* LWM2M */

/* LWM2M-v1.0.0 */

/* end of LWM2M-v1.0.0 */
/* end of LWM2M */

/* MQTT */

/* pahomqtt-v1.1.0 */

/* end of pahomqtt-v1.1.0 */
/* end of MQTT */
/* end of Protocols */

/* Socket */

/* end of Socket */
/* end of Network */

/* OTA */

/* Fota by CMIOT */

/* end of Fota by CMIOT */
/* end of OTA */

/* Position */

/* end of Position */

/* Ramdisk */

/* end of Ramdisk */

/* Security */


/* OneTLS */

/* end of OneTLS */
/* end of Security */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "tshell"
#define SHELL_TASK_PRIORITY 15
#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_USING_HISTORY
#define SHELL_HISTORY_LINES 5
#define SHELL_USING_DESCRIPTION
#define SHELL_CMD_SIZE 80
#define SHELL_PROMPT_SIZE 256
#define SHELL_ARG_MAX 10
/* end of Shell */
/* end of Components */

/* Thirdparty */

/* MicroPython */

/* end of MicroPython */

/* cJSON */

/* end of cJSON */

/* Easyflash */

/* end of Easyflash */

/* GUI */

#define OS_GUI_DISP_DEV_NAME "lcd"
#define OS_GUI_INPUT_DEV_NAME "touch"
/* end of GUI */

/* iotjs */

/* end of iotjs */

/* jerryscript */

/* end of jerryscript */

/* Iotivity */

/* end of Iotivity */

/* Optparse */

/* end of Optparse */

/* telnetd */

/* end of telnetd */

/* WWD Wi-Fi framework */

/* end of WWD Wi-Fi framework */
/* end of Thirdparty */

/* Debug */

#define OS_DEBUG
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

