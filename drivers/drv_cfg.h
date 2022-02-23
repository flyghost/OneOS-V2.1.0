/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_cfg.h
 *
 * @brief       This file provides the ability of header addition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_CFG_H__
#define __DRV_CFG_H__

#include <os_task.h>
#include <os_errno.h>
#include <os_assert.h>
#include <device.h>
#include <arch_interrupt.h>
#include <bus/bus.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <driver.h>

#ifdef OS_USING_DMA
#include "dma/dma.h"
#endif

#ifdef OS_USING_RTC
#include "rtc/rtc.h"
#ifdef OS_USING_ALARM
#include "alarm/alarm.h"
#endif
#endif /* OS_USING_RTC */

#ifdef OS_USING_SPI
#include "spi/spi.h"
#endif /* OS_USING_SPI */

#ifdef OS_USING_FAL
#include "fal/fal.h"
#endif

#ifdef OS_USING_USB_DEVICE
#ifndef SERIES_NXP
#include "usbd_conf.h"
#endif
#endif /* OS_USING_USB_DEVICE */

#ifdef OS_USING_USB_HOST
#include "usb_host.h"
#include "usbh_conf.h"

#ifdef ENABLE_USBH_SPECIFIC_CDC_CLASS
#include "usb/usb_host/usbh_cdc_dev.h"
#endif

#endif /* OS_USING_USB_HOST */

#ifdef OS_USING_SERIAL
#ifdef OS_SERIAL_SIMPLE
#include "serial/serial_simple.h"
#else
#include "serial/serial.h"
#endif
#endif /* OS_USING_SERIAL */

#ifdef OS_USING_I2C
#include "i2c/i2c.h"

#ifdef OS_USING_I2C_BITOPS
#include "i2c/soft_i2c_bus.h"
#endif /* OS_USING_I2C_BITOPS */
#endif /* OS_USING_I2C */

#ifdef OS_USING_SDIO
#include "sdio/mmcsd_core.h"
#include "sdio/sd.h"
#include "sdio/sdio.h"
#endif

#ifdef OS_USING_WDG
#include "watchdog/watchdog.h"
#endif

#ifdef OS_USING_PIN
#include "pin/pin.h"
#endif

#ifdef OS_USING_PUSH_BUTTON
#include "misc/push_button.h"
#endif

#ifdef OS_USING_LED
#include "misc/led.h"
#endif

#ifdef OS_USING_BUZZER
#include "misc/buzzer.h"
#endif

#ifdef OS_USING_CAN
#include "can/can.h"
#endif

#ifdef OS_USING_TIMER_DRIVER
#include "timer/timer.h"
#endif

#ifdef OS_USING_AUDIO
#include "audio/audio.h"
#endif

#ifdef OS_USING_ADC
#include "misc/adc.h"
#endif

#ifdef OS_USING_DAC
#include "misc/dac.h"
#endif

#ifdef OS_USING_PWM
#include "misc/pwm.h"
#endif

#ifdef OS_USING_TICKLESS_LPMGR
#include "lpmgr/lpmgr.h"
#endif

#ifdef OS_USING_WIFI
#include "wlan/wlan.h"
#endif

#ifdef OS_USING_HWCRYPTO
#include "hwcrypto/crypto.h"
#endif

#ifdef OS_USING_PULSE_ENCODER
#include "misc/pulse_encoder.h"
#endif

#ifdef OS_USING_INPUT_CAPTURE
#include "misc/inputcapture.h"
#endif

#ifdef OS_USING_NAND
#include "nand/nand.h"
#endif

#ifdef OS_USING_CONSOLE
#include "console.h"
#endif

#ifdef OS_USING_HRTIMER
#include <timer/hrtimer.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DRV_CFG_H__ */
