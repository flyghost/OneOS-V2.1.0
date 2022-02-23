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
 * @file        drv_common.h
 *
 * @brief       This file provides _Error_Handler() declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <device.h>

#include "peripherals.h"

#include <fsl_common.h>
#include <fsl_gpio.h>
#include <fsl_anactrl.h>
#include <fsl_lpadc.h>
#include <fsl_i2c.h>
#include "fsl_i2c_dma.h"
#include <fsl_i2s.h>
#include "fsl_i2s_dma.h"
#include <fsl_spi.h>
#include "fsl_spi_dma.h"
#include <fsl_usart.h>
#include "fsl_usart_dma.h"
#include <fsl_wwdt.h>
#include <fsl_rtc.h>
#include <fsl_crc.h>
#include <fsl_utick.h>
#include <fsl_ctimer.h>

#include <drv_gpio.h>
#include <drv_adc.h>
#include <drv_i2c.h>
#include <drv_i2s.h>
#include <drv_spi.h>
#include <drv_usart.h>
#include <drv_wwdt.h>
#include <drv_rtc.h>
#include <drv_crypto.h>
#include <drv_hwtimer.h>

#if defined(OS_USING_USB_DEVICE)
#include <drv_usbd.h>
#endif

#endif
