#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define OS_USING_PIN
#define OS_USING_SERIAL
#define BOARD_ATK_PANDORA
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
#define OS_SYSTEM_WORKQUEUE_PRIORITY 0

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
#define OS_AUDIO_REPLAY_MP_BLOCK_COUNT 5
#define OS_AUDIO_RECORD_FIFO_SIZE 3528
#define OS_AUDIO_RECORD_FIFO_COUNT 4
#define OS_USING_SAI
#define OS_USING_SAI_FOR_PLAYER
#define BSP_USING_ES8388
#define BSP_USING_ES8388_CONFIG
#define BSP_ES8388_I2C_BUS "soft_i2c1"
#define BSP_ES8388_I2C_ADDR 0x10
#define BSP_USING_ES8388_DATA
#define BSP_AUDIO_DATA_TX_BUS "sai_BlockA1"
#define BSP_AUDIO_DATA_RX_BUS "sai_BlockB1"
#define BSP_ES8388_POWER_PIN 63
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

/* end of DMA */

/* FAL */

#define OS_USING_FAL
/* end of FAL */

/* Graphic */

#define OS_USING_GRAPHIC
#define OS_GRAPHIC_WIDTH 240
#define OS_GRAPHIC_HEIGHT 240
#define OS_USING_ST7789VW
#define OS_ST7789VW_SPI_BUS_NAME "spi3"
#define OS_ST7789VW_SPI_BUS_MODE 0
#define OS_ST7789VW_SPI_CS_PIN 55
#define OS_ST7789VW_PWR_PIN 23
#define OS_ST7789VW_PWR_PIN_ACTIVE 1
#define OS_ST7789VW_DC_PIN 20
#define OS_ST7789VW_RES_PIN 22
/* end of Graphic */

/* HAL */

#define MANUFACTOR_STM32
#define SERIES_STM32L4
#define SOC_STM32L475VE

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* HwCrypto */

/* end of HwCrypto */

/* I2C */

#define OS_USING_I2C
#define OS_USING_I2C_BITOPS
#define SOFT_I2C_BUS_DELAY_US 10
#define BSP_USING_SOFT_I2C1
#define BSP_SOFT_I2C1_SCL_PIN 32
#define BSP_SOFT_I2C1_SDA_PIN 33
#define BSP_USING_SOFT_I2C2
#define BSP_SOFT_I2C2_SCL_PIN 54
#define BSP_SOFT_I2C2_SDA_PIN 33
/* end of I2C */

/* Infrared */

#define OS_USING_INFRARED
#define BSP_USING_RMT_CTL_ATK_TX
#define BSP_USING_RMT_CTL_ATK_TX_PIN 16
#define BSP_USING_RMT_CTL_ATK_RX
#define BSP_USING_RMT_CTL_ATK_RX_PIN 17
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

/* end of NAND */

/* PIN */

#define OS_PIN_MAX_CHIP 1
/* end of PIN */

/* RTC */

/* end of RTC */

/* RTT */

/* end of RTT */

/* SDIO */

/* end of SDIO */

/* Sensors */

#define OS_USING_SENSOR
#define OS_USING_MPU6XXX
#define OS_MPU6XXX_BUS_I2C
#define OS_MPU6XXX_BUS_NAME "soft_i2c1"
#define OS_MPU6XXX_ADDR 0x68
#define OS_MPU6XXX_INT_PIN 21
#define PKG_USING_MPU6XXX
#define PKG_USING_MPU6XXX_ACCE
#define PKG_USING_MPU6XXX_GYRO
#define OS_USING_ICM20608
#define OS_USING_AP3216C
#define OS_AP3216C_I2C_BUS_NAME "soft_i2c1"
#define OS_AP3216C_I2C_ADDR 0x1e
/* end of Sensors */

/* Serial */

#define OS_SERIAL_DELAY_CLOSE
#define OS_SERIAL_RX_BUFSZ 64
#define OS_SERIAL_TX_BUFSZ 64
/* end of Serial */

/* SN */

/* end of SN */

/* SPI */

#define OS_USING_SPI
#define OS_USING_QSPI
#define OS_USING_SPI_MSD
#define OS_USING_SFUD
#define OS_SFUD_USING_SFDP
#define OS_SFUD_USING_FLASH_INFO_TABLE
#define OS_SFUD_USING_QSPI
#define OS_QSPI_FLASH_BUS_NAME "qspi"
#define OS_EXTERN_FLASH_PORT_CFG
#define OS_EXTERN_FLASH_DEV_NAME "W25Q128"
#define OS_EXTERN_FLASH_BUS_NAME "sfud_bus"
#define OS_EXTERN_FLASH_NAME "nor_flash"
#define OS_EXTERN_FLASH_SIZE 16777216
#define OS_EXTERN_FLASH_BLOCK_SIZE 4096
#define OS_EXTERN_FLASH_PAGE_SIZE 4096
#define BSP_USING_W25QXX
#define BSP_USING_ENC28J60
#define BSP_ENC28J60_SPI_BUS "spi2"
#define BSP_ENC28J60_SPI_CS 53
#define BSP_ENC28J60_SPI_DEV "spi21"
#define BSP_ENC28J60_IRQ 51
#define BSP_ENC28J60_RST 52
#define BSP_USING_SDCARD
#define BSP_SDCARD_SPI_DEV "spi1"
#define BSP_SDCARD_SPI_CS_PIN 35
/* end of SPI */

/* Timer */

#define OS_USING_TIMER_DRIVER
#define OS_USING_CLOCKSOURCE
#define OS_CLOCKSOURCE_SHOW
#define OS_USING_CLOCKEVENT
#define OS_CLOCKEVENT_SHOW
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
#define USB_USING_DEVICE_FS
#define USBD_FS_VENDOR_ID 0x0FFE
#define USBD_FS_PRODUCT_ID 0x0001
#define OS_USBD_CDC_CLASS_FS
#define OS_USBD_CDC_FS_STK_SIZE 512
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

/* end of Atest */

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

/* LWM2M */

/* LWM2M */

/* end of LWM2M */
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

/* telnetd */

/* end of telnetd */
/* end of Thirdparty */

/* Boot Config */

/* end of Boot Config */

/* Debug */

#define OS_DEBUG
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

