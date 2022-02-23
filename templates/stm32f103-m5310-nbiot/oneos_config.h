#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_M5310_NBIOT
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3

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
#define OS_USING_TIMER
#define OS_TIMER_TASK_STACK_SIZE 2048
#define OS_USING_WORKQUEUE
#define OS_USING_SYSTEM_WORKQUEUE
#define OS_SYSTEM_WORKQUEUE_STACK_SIZE 2048
#define OS_SYSTEM_WORKQUEUE_PRIORITY 0

/* Inter-task communication and synchronization */

#define OS_USING_MUTEX
#define OS_USING_SPINLOCK
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

/* C standard library */

#define OS_USING_LIBC
#define OS_GCC_USING_NEWLIB
/* end of C standard library */

/* Osal */

/* POSIX compatibility layer */

/* end of POSIX compatibility layer */

/* RT-Thread compatibility layer */

/* end of RT-Thread compatibility layer */

/* CMSIS compatibility layer */

/* end of CMSIS compatibility layer */

/* FreeRTOS compatibility layer */

/* end of FreeRTOS compatibility layer */
/* end of Osal */

/* Drivers */

#define OS_USING_DEVICE

/* Audio */

/* end of Audio */

/* BLOCK */

#define OS_USING_BLOCK
/* end of BLOCK */

/* CAN */

/* end of CAN */

/* CONSOLE */

#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart1"
/* end of CONSOLE */

/* DMA */

#define OS_USING_DMA
/* end of DMA */

/* FAL */

#define OS_USING_FAL
/* end of FAL */

/* Graphic */

/* end of Graphic */

/* HAL */

#define MANUFACTOR_STM32
#define SERIES_STM32F1
#define SOC_STM32F103xE

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* HwCrypto */

/* end of HwCrypto */

/* I2C */

#define OS_USING_I2C
#define OS_USING_I2C_BITOPS
#define SOFT_I2C_BUS_DELAY_US 10
#define BSP_USING_SOFT_I2C1
#define BSP_SOFT_I2C1_SCL_PIN 22
#define BSP_SOFT_I2C1_SDA_PIN 23
#define BSP_USING_SOFT_I2C2
#define BSP_SOFT_I2C2_SCL_PIN 29
#define BSP_SOFT_I2C2_SDA_PIN 28
/* end of I2C */

/* Infrared */

/* end of Infrared */

/* Low power manager */

/* end of Low power manager */

/* MISC */

#define OS_USING_PUSH_BUTTON
#define OS_USING_LED
#define OS_USING_BUZZER
#define OS_USING_ADC
#define OS_USING_DAC
#define OS_USING_PWM
/* end of MISC */

/* NAND */

/* end of NAND */

/* PIN */

#define OS_USING_PIN
#define OS_PIN_MAX_CHIP 1
/* end of PIN */

/* RTC */

#define OS_USING_RTC
/* end of RTC */

/* RTT */

/* end of RTT */

/* SDIO */

/* end of SDIO */

/* Sensors */

#define OS_USING_SENSOR
#define OS_USING_SHT20
#define OS_SHT20_I2C_BUS_NAME "soft_i2c1"
#define OS_SHT20_I2C_ADDR 0x40
#define OS_USING_ADXL345
#define OS_ADXL345_I2C_BUS_NAME "soft_i2c1"
#define OS_ADXL345_I2C_ADDR 0x53
#define OS_USING_BH1750
#define OS_BH1750_I2C_BUS_NAME "soft_i2c2"
#define OS_BH1750_I2C_ADDR 0x23
#define OS_USING_BMP180
#define OS_BMP180_I2C_BUS_NAME "soft_i2c2"
#define OS_BMP180_I2C_ADDR 0x77
/* end of Sensors */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_RX_BUFSZ 64
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* SPI */

#define OS_USING_SPI
/* end of SPI */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_CLOCKEVENT

/* cortex-m hardware timer config */

#define OS_USING_SYSTICK_FOR_KERNEL_TICK
#define OS_USING_DWT_FOR_CLOCKSOURCE
/* end of cortex-m hardware timer config */
/* end of Timer */

/* Touch */

/* end of Touch */

/* USB */

/* end of USB */

/* WDG */

#define OS_USING_WDG
/* end of WDG */

/* WLAN */

/* end of WLAN */
/* end of Drivers */

/* Components */

/* Atest */

/* end of Atest */

/* Cloud */

/* Aliyun */

/* end of Aliyun */

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

/* Diagnose */


/* eCoreDump */

/* end of eCoreDump */
/* end of Diagnose */

/* Dlog */

#define OS_USING_DLOG
#define DLOG_PRINT_LVL_W
#define DLOG_GLOBAL_PRINT_LEVEL 4
#define DLOG_COMPILE_LVL_W
#define DLOG_COMPILE_LEVEL 4
#define DLOG_USING_ISR_LOG
#define DLOG_USING_FILTER
#define DLOG_USING_ASYNC_OUTPUT
#define DLOG_ASYNC_OUTPUT_BUF_SIZE 2048
#define DLOG_ASYNC_OUTPUT_TASK_STACK_SIZE 2048
#define DLOG_ASYNC_OUTPUT_TASK_PRIORITY 30
#define DLOG_USING_SYSLOG

/* Log format */

#define DLOG_WITH_FUNC_LINE
#define DLOG_USING_COLOR
/* end of Log format */

/* Dlog backend option */

#define DLOG_BACKEND_USING_CONSOLE
/* end of Dlog backend option */
/* end of Dlog */

/* FileSystem */

/* end of FileSystem */

/* Network */

/* TCP/IP */

/* LwIP */

/* end of LwIP */
/* end of TCP/IP */

/* Molink */

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

/* OnePos */

/* end of OnePos */

/* Ramdisk */

/* end of Ramdisk */

/* Security */

/* end of Security */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "tshell"
#define SHELL_TASK_PRIORITY 20
#define SHELL_TASK_STACK_SIZE 4096
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

/* GUI */

#define OS_GUI_DISP_DEV_NAME "lcd"
#define OS_GUI_INPUT_DEV_NAME "touch"
/* end of GUI */
/* end of Thirdparty */

/* Boot Config */

/* end of Boot Config */

/* Debug */

#define OS_DEBUG
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

