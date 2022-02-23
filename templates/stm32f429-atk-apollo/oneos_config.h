#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define OS_USING_PIN
#define OS_USING_SERIAL
#define BOARD_ATK_APOLLO
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

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
#define OS_USING_IO_MULTIPLEXING
#define PRESET_FD_SETSIZE 64
/* end of C standard library adapter */

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

#define OS_USING_AUDIO
#define OS_AUDIO_DATA_CONFIG
#define OS_AUDIO_REPLAY_MP_BLOCK_SIZE 2048
#define OS_AUDIO_REPLAY_MP_BLOCK_COUNT 2
#define OS_AUDIO_RECORD_FIFO_SIZE 3528
#define OS_AUDIO_RECORD_FIFO_COUNT 4
#define OS_USING_SAI
#define OS_USING_SAI_FOR_PLAYER
#define BSP_AUDIO_DATA_TX_BUS "sai_BlockA1"
#define BSP_USING_WM8978
#define BSP_USING_WM8978_CONFIG
#define BSP_WM8978_I2C_BUS "soft_i2c2"
#define BSP_WM8978_I2C_ADDR 0x1a
#define BSP_USING_WM8978_DATA
#define BSP_WM8978_POWER_PIN -1
/* end of Audio */

/* BLOCK */

#define OS_USING_BLOCK
/* end of BLOCK */

/* CAN */

#define OS_USING_CAN
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
#define OS_FAL_DYNAMIC_FLASH
/* end of FAL */

/* Graphic */

/* end of Graphic */

/* HAL */

#define MANUFACTOR_STM32
#define SERIES_STM32F4
#define SOC_STM32F429xx
#define BSP_USING_ETH

/* config eth hw */

#define ETH_RESET_PIN 1007
#define ETH_RESET_PIN_ACTIVE_HIGH
#define PHY_USING_LAN8720A

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* HwCrypto */

/* end of HwCrypto */

/* I2C */

#define OS_USING_I2C
#define OS_USING_I2C_BITOPS
#define SOFT_I2C_BUS_DELAY_US 10
#define BSP_USING_SOFT_I2C2
#define BSP_SOFT_I2C2_SCL_PIN 116
#define BSP_SOFT_I2C2_SDA_PIN 117
#define BSP_USING_I2C_AT24CXX
#define BSP_AT24CXX_I2C_BUS_NAME "soft_i2c2"
#define BSP_AT24CXX_I2C_ADDR 0x50
/* end of I2C */

/* Infrared */

#define OS_USING_INFRARED
#define BSP_USING_RMT_CTL_ATK_RX
#define BSP_USING_RMT_CTL_ATK_RX_PIN 8
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

/* MTD */

#define OS_USING_MTD
/* end of MTD */

/* NAND */

#define OS_USING_NAND
#define BSP_NAND_MT29F4G08ABADA
/* end of NAND */

/* PIN */

#define OS_PIN_MAX_CHIP 2
#define BSP_USING_PIN_PCF8574
#define OS_PCF8574_I2C_BUS_NAME "soft_i2c2"
#define OS_PCF8574_I2C_BUS_ADDR 0x20
#define OS_PCF8574_INT_PIN 28
#define OS_PCF8574_PIN_BASE 1000
/* end of PIN */

/* RTC */

#define OS_USING_RTC
/* end of RTC */

/* RTT */

/* end of RTT */

/* SDIO */

#define OS_USING_SDIO
#define OS_SDIO_STACK_SIZE 512
#define OS_SDIO_TASK_PRIORITY 15
#define OS_MMCSD_STACK_SIZE 1024
#define OS_MMCSD_TASK_PREORITY 22
#define OS_MMCSD_MAX_PARTITION 16
/* end of SDIO */

/* Sensors */

#define OS_USING_SENSOR
#define OS_USING_MPU6XXX
#define OS_MPU6XXX_BUS_I2C
#define OS_MPU6XXX_BUS_NAME "soft_i2c2"
#define OS_MPU6XXX_ADDR 0x68
#define OS_MPU6XXX_INT_PIN 21
#define PKG_USING_MPU6XXX
#define PKG_USING_MPU6XXX_ACCE
#define PKG_USING_MPU6XXX_GYRO
#define PKG_USING_MPU6XXX_TEMP
#define OS_USING_MPU9250
#define OS_USING_AK8963
#define OS_AK8963_I2C_BUS_NAME "soft_i2c2"
#define OS_AK8963_I2C_ADDR 0x0c
#define OS_USING_AP3216C
#define OS_AP3216C_I2C_BUS_NAME "soft_i2c2"
#define OS_AP3216C_I2C_ADDR 0x1e
/* end of Sensors */

/* Serial */

#define OS_SERIAL_RX_BUFSZ 64
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* SN */

/* end of SN */

/* SPI */

#define OS_USING_SPI
#define OS_USING_SFUD
#define OS_SFUD_USING_SFDP
#define OS_SFUD_USING_FLASH_INFO_TABLE
#define OS_SFUD_USING_SPI
#define OS_SPI_FLASH_BUS_NAME "spi5"
#define OS_SPI_FLASH_CS_PIN 86
#define OS_EXTERN_FLASH_PORT_CFG
#define OS_EXTERN_FLASH_DEV_NAME "W25Q128"
#define OS_EXTERN_FLASH_BUS_NAME "sfud_bus"
#define OS_EXTERN_FLASH_NAME "nor_flash"
#define OS_EXTERN_FLASH_SIZE 16777216
#define OS_EXTERN_FLASH_BLOCK_SIZE 4096
#define OS_EXTERN_FLASH_PAGE_SIZE 4096
#define BSP_USING_W25QXX
/* end of SPI */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_USING_TIMEKEEPING
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

#define OS_USING_USB_DEVICE
#define USB_USING_DEVICE_HS
#define USBD_HS_VENDOR_ID 0x0FFE
#define USBD_HS_PRODUCT_ID 0x0001
#define OS_USBD_CDC_CLASS_HS
#define OS_USBD_CDC_HS_STK_SIZE 512
#define OS_USBD_TASK_STACK_SZ 4096
/* end of USB */

/* WDG */

#define OS_USING_WDG
/* end of WDG */

/* WLAN */

/* end of WLAN */
/* end of Drivers */

/* Components */

/* Atest */

#define OS_USING_ATEST
#define ATEST_TASK_STACK_SIZE 4096
#define ATEST_TASK_PRIORITY 20
/* end of Atest */

/* Cloud */

/* Aliyun */

/* end of Aliyun */

/* AWS */

/* end of AWS */

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
#define DLOG_PRINT_LVL_I
#define DLOG_GLOBAL_PRINT_LEVEL 6
#define DLOG_COMPILE_LVL_D
#define DLOG_COMPILE_LEVEL 7
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
#define DLOG_OUTPUT_TIME_INFO
/* end of Log format */

/* Dlog backend option */

#define DLOG_BACKEND_USING_CONSOLE
/* end of Dlog backend option */
/* end of Dlog */

/* FileSystem */

#define OS_USING_VFS
#define VFS_MOUNTPOINT_MAX 4
#define VFS_FILESYSTEM_TYPES_MAX 4
#define VFS_FD_MAX 16
#define OS_USING_VFS_DEVFS
#define OS_USING_VFS_FATFS

/* Elm-ChaN's FatFs, generic FAT filesystem module */

#define OS_VFS_FAT_CODE_PAGE 437
#define OS_VFS_FAT_USE_LFN_3
#define OS_VFS_FAT_USE_LFN 3
#define OS_VFS_FAT_MAX_LFN 255
#define OS_VFS_FAT_DRIVES 2
#define OS_VFS_FAT_MAX_SECTOR_SIZE 4096
#define OS_VFS_FAT_REENTRANT
/* end of Elm-ChaN's FatFs, generic FAT filesystem module */
/* end of FileSystem */

/* Network */

/* TCP/IP */

/* LwIP */

#define NET_USING_LWIP
#define NET_USING_LWIP212
#define LWIP_USING_IGMP
#define LWIP_USING_ICMP
#define LWIP_USING_DNS
#define LWIP_USING_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define LWIP_STATIC_IPADDR "192.168.1.30"
#define LWIP_STATIC_GWADDR "192.168.1.1"
#define LWIP_STATIC_MSKADDR "255.255.255.0"
/* end of Static IPv4 Address */
#define LWIP_USING_UDP
#define LWIP_USING_TCP
#define LWIP_USING_RAW
#define LWIP_MEMP_NUM_NETCONN 8
#define LWIP_PBUF_NUM 16
#define LWIP_RAW_PCB_NUM 4
#define LWIP_UDP_PCB_NUM 4
#define LWIP_TCP_PCB_NUM 4
#define LWIP_TCP_SEG_NUM 40
#define LWIP_TCP_SND_BUF 8196
#define LWIP_TCP_WND_SIZE 8196
#define LWIP_TCP_TASK_PRIORITY 10
#define LWIP_TCP_TASK_MBOX_SIZE 8
#define LWIP_TCP_TASK_STACKSIZE 1024
#define LWIP_ETH_TASK_PRIORITY 12
#define LWIP_ETH_TASK_STACKSIZE 1024
#define LWIP_ETH_TASK_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0
#define LWIP_USING_PING
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

#define NET_USING_BSD
#define BSD_USING_LWIP
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


/* OneTLS */

/* end of OneTLS */
/* end of Security */

/* Shell */

#define OS_USING_SHELL
#define SHELL_TASK_NAME "tshell"
#define SHELL_TASK_PRIORITY 20
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

