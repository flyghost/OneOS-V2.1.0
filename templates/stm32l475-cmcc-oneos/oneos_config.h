#ifndef __ONEOS_CONFIG_H__
#define __ONEOS_CONFIG_H__

#define BOARD_CMCC_ONEOS
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
#define OS_RECYCLE_TASK_STACK_SIZE 512
#define OS_USING_TIMER
#define OS_TIMER_TASK_STACK_SIZE 512
#define OS_TIMER_SORT
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
#define OS_AUDIO_RECORD_FIFO_COUNT 2
#define OS_USING_SAI
#define OS_USING_SAI_FOR_PLAYER
#define OS_AUDIO_PLAYER_VOLUME 30
#define OS_USING_SAI_FOR_RECORDER
#define OS_AUDIO_SAMPLERATE 44100
#define OS_AUDIO_CHANNEL 2
#define BSP_USING_ES8388
#define BSP_USING_ES8388_CONFIG
#define BSP_ES8388_I2C_BUS "soft_i2c3"
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

#define OS_USING_CAN
/* end of CAN */

/* CONSOLE */

#define OS_USING_CONSOLE
#define OS_CONSOLE_DEVICE_NAME "uart3"
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

#define OS_USING_GRAPHIC
#define OS_GRAPHIC_WIDTH 240
#define OS_GRAPHIC_HEIGHT 240
#define OS_USING_ST7789VW
#define OS_ST7789VW_SPI_BUS_NAME "spi1"
#define OS_ST7789VW_SPI_BUS_MODE 3
#define OS_ST7789VW_SPI_CS_PIN 52
#define OS_ST7789VW_PWR_PIN 4
#define OS_ST7789VW_PWR_PIN_ACTIVE 0
#define OS_ST7789VW_DC_PIN 59
#define OS_ST7789VW_RES_PIN 6
/* end of Graphic */

/* HAL */

#define MANUFACTOR_STM32
#define SERIES_STM32L4
#define SOC_STM32L475VG
#define BSP_BOOT_OPTION
#define BSP_TEXT_SECTION_ADDR 0x08000000
#define BSP_TEXT_SECTION_SIZE 0x00100000
#define BSP_DATA_SECTION_ADDR 0x20000000
#define BSP_DATA_SECTION_SIZE 0x00018000

/* Configure base hal in STM32CubeMX */

/* end of HAL */

/* HwCrypto */

#define OS_USING_HWCRYPTO
#define OS_HWCRYPTO_DEFAULT_NAME "hwcryto"
#define OS_HWCRYPTO_IV_MAX_SIZE 16
#define OS_HWCRYPTO_KEYBIT_MAX_SIZE 256
#define OS_HWCRYPTO_USING_RNG
#define OS_HWCRYPTO_USING_CRC
#define OS_HWCRYPTO_USING_CRC_04C11DB7
/* end of HwCrypto */

/* I2C */

#define OS_USING_I2C
#define OS_USING_I2C_BITOPS
#define SOFT_I2C_BUS_DELAY_US 10
#define BSP_USING_SOFT_I2C3
#define BSP_SOFT_I2C3_SCL_PIN 32
#define BSP_SOFT_I2C3_SDA_PIN 33
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
#define OS_USING_PWM
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
/* end of RTC */

/* SDIO */

/* end of SDIO */

/* Sensors */

#define OS_USING_SENSOR
#define OS_USING_AHT10
#define OS_AHT10_I2C_BUS_NAME "soft_i2c3"
#define OS_AHT10_I2C_ADDR 0x38
#define OS_USING_MPU6XXX
#define OS_MPU6XXX_BUS_I2C
#define OS_MPU6XXX_BUS_NAME "soft_i2c3"
#define OS_MPU6XXX_ADDR 0x68
#define OS_MPU6XXX_INT_PIN 21
#define PKG_USING_MPU6XXX
#define PKG_USING_MPU6XXX_ACCE
#define PKG_USING_MPU6XXX_GYRO
#define OS_USING_ICM20602
#define OS_USING_AP3216C
#define OS_AP3216C_I2C_BUS_NAME "soft_i2c3"
#define OS_AP3216C_I2C_ADDR 0x1e
/* end of Sensors */

/* Serial */

#define OS_USING_SERIAL
#define OS_SERIAL_DELAY_CLOSE
#define OS_SERIAL_RX_BUFSZ 1024
#define OS_SERIAL_TX_BUFSZ 64

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
#define OS_USING_SPI_MSD
#define OS_USING_SFUD
#define OS_SFUD_USING_SFDP
#define OS_SFUD_USING_FLASH_INFO_TABLE
#define OS_SFUD_USING_QSPI
#define OS_QSPI_FLASH_BUS_NAME "qspi"
#define OS_EXTERN_FLASH_PORT_CFG
#define OS_EXTERN_FLASH_DEV_NAME "W25Q64"
#define OS_EXTERN_FLASH_BUS_NAME "sfud_bus"
#define OS_EXTERN_FLASH_NAME "nor_flash"
#define OS_EXTERN_FLASH_SIZE 8388608
#define OS_EXTERN_FLASH_BLOCK_SIZE 4096
#define OS_EXTERN_FLASH_PAGE_SIZE 4096
#define BSP_USING_W25QXX
#define BSP_USING_SDCARD
#define BSP_SDCARD_SPI_DEV "spi2"
#define BSP_SDCARD_SPI_CS_PIN 28
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
/* end of Drivers */

/* Components */

/* AMS */

/* end of AMS */

/* Atest */

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
#define SHELL_TASK_PRIORITY 20
#define SHELL_TASK_STACK_SIZE 7200
#define SHELL_USING_HISTORY
#define SHELL_HISTORY_LINES 5
#define SHELL_USING_DESCRIPTION
#define SHELL_CMD_SIZE 200
#define SHELL_PROMPT_SIZE 256
#define SHELL_ARG_MAX 10
/* end of Shell */
/* end of Components */

/* Thirdparty */

/* MicroPython */

/* end of MicroPython */

/* WIFI */

/* end of WIFI */

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

/* Optparse */

/* end of Optparse */

/* telnetd */

/* end of telnetd */
/* end of Thirdparty */

/* Debug */

#define OS_DEBUG
#define LOG_BUFF_SIZE_256
#define OS_LOG_BUFF_SIZE 256
/* end of Debug */

#endif /* __ONEOS_CONFIG_H__ */

