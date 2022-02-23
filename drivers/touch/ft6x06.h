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
 * @file        ft6x06.h
 *
 * @brief       ft6x06
 *
 * @details     ft6x06
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __FT6X06_H
#define __FT6X06_H

#ifdef __cplusplus
extern "C" {
#endif

/* Set Multi-touch as non supported */
#ifndef TS_MULTI_TOUCH_SUPPORTED
#define TS_MULTI_TOUCH_SUPPORTED 0
#endif

/* Set Auto-calibration as non supported */
#ifndef TS_AUTO_CALIBRATION_SUPPORTED
#define TS_AUTO_CALIBRATION_SUPPORTED 0
#endif

/* Maximum border values of the touchscreen pad */
#define FT_6206_MAX_WIDTH  ((uint16_t)800) /* Touchscreen pad max width   */
#define FT_6206_MAX_HEIGHT ((uint16_t)480) /* Touchscreen pad max height  */

/* Touchscreen pad max width and height values for FT6x36 Touch*/
#define FT_6206_MAX_WIDTH_HEIGHT ((uint16_t)240)

/* Possible values of driver functions return status */
#define FT6206_STATUS_OK     0
#define FT6206_STATUS_NOT_OK 1

/* Possible values of global variable 'TS_I2C_Initialized' */
#define FT6206_I2C_NOT_INITIALIZED 0
#define FT6206_I2C_INITIALIZED     1

/* Max detectable simultaneous touches */
#define FT6206_MAX_DETECTABLE_TOUCH 2

/**
 * @brief : Definitions for FT6206 I2C register addresses on 8 bit
 **/

/* Current mode register of the FT6206 (R/W) */
#define FT6206_DEV_MODE_REG 0x00

/* Possible values of FT6206_DEV_MODE_REG */
#define FT6206_DEV_MODE_WORKING 0x00
#define FT6206_DEV_MODE_FACTORY 0x04

#define FT6206_DEV_MODE_MASK  0x7
#define FT6206_DEV_MODE_SHIFT 4

/* Gesture ID register */
#define FT6206_GEST_ID_REG 0x01

/* Possible values of FT6206_GEST_ID_REG */
#define FT6206_GEST_ID_NO_GESTURE 0x00
#define FT6206_GEST_ID_MOVE_UP    0x10
#define FT6206_GEST_ID_MOVE_RIGHT 0x14
#define FT6206_GEST_ID_MOVE_DOWN  0x18
#define FT6206_GEST_ID_MOVE_LEFT  0x1C
#define FT6206_GEST_ID_ZOOM_IN    0x48
#define FT6206_GEST_ID_ZOOM_OUT   0x49

/* Touch Data Status register : gives number of active touch points (0..2) */
#define FT6206_TD_STAT_REG 0x02

/* Values related to FT6206_TD_STAT_REG */
#define FT6206_TD_STAT_MASK  0x0F
#define FT6206_TD_STAT_SHIFT 0x00

/* Values Pn_XH and Pn_YH related */
#define FT6206_TOUCH_EVT_FLAG_PRESS_DOWN 0x00
#define FT6206_TOUCH_EVT_FLAG_LIFT_UP    0x01
#define FT6206_TOUCH_EVT_FLAG_CONTACT    0x02
#define FT6206_TOUCH_EVT_FLAG_NO_EVENT   0x03

#define FT6206_TOUCH_EVT_FLAG_SHIFT 6
#define FT6206_TOUCH_EVT_FLAG_MASK  (3 << FT6206_TOUCH_EVT_FLAG_SHIFT)

#define FT6206_MSB_MASK  0x0F
#define FT6206_MSB_SHIFT 0

/* Values Pn_XL and Pn_YL related */
#define FT6206_LSB_MASK  0xFF
#define FT6206_LSB_SHIFT 0

#define FT6206_P1_XH_REG 0x03
#define FT6206_P1_XL_REG 0x04
#define FT6206_P1_YH_REG 0x05
#define FT6206_P1_YL_REG 0x06

/* Touch Pressure register value (R) */
#define FT6206_P1_WEIGHT_REG 0x07

/* Values Pn_WEIGHT related  */
#define FT6206_TOUCH_WEIGHT_MASK  0xFF
#define FT6206_TOUCH_WEIGHT_SHIFT 0

/* Touch area register */
#define FT6206_P1_MISC_REG 0x08

/* Values related to FT6206_Pn_MISC_REG */
#define FT6206_TOUCH_AREA_MASK  (0x04 << 4)
#define FT6206_TOUCH_AREA_SHIFT 0x04

#define FT6206_P2_XH_REG     0x09
#define FT6206_P2_XL_REG     0x0A
#define FT6206_P2_YH_REG     0x0B
#define FT6206_P2_YL_REG     0x0C
#define FT6206_P2_WEIGHT_REG 0x0D
#define FT6206_P2_MISC_REG   0x0E

/* Threshold for touch detection */
#define FT6206_TH_GROUP_REG 0x80

/* Values FT6206_TH_GROUP_REG : threshold related  */
#define FT6206_THRESHOLD_MASK  0xFF
#define FT6206_THRESHOLD_SHIFT 0

/* Filter function coefficients */
#define FT6206_TH_DIFF_REG 0x85

/* Control register */
#define FT6206_CTRL_REG 0x86

/* Values related to FT6206_CTRL_REG */

/* Will keep the Active mode when there is no touching */
#define FT6206_CTRL_KEEP_ACTIVE_MODE 0x00

/* Switching from Active mode to Monitor mode automatically when there is no touching */
#define FT6206_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE 0x01

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT6206_TIMEENTERMONITOR_REG 0x87

/* Report rate in Active mode */
#define FT6206_PERIODACTIVE_REG 0x88

/* Report rate in Monitor mode */
#define FT6206_PERIODMONITOR_REG 0x89

/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6206_RADIAN_VALUE_REG 0x91

/* Maximum offset while Moving Left and Moving Right gesture */
#define FT6206_OFFSET_LEFT_RIGHT_REG 0x92

/* Maximum offset while Moving Up and Moving Down gesture */
#define FT6206_OFFSET_UP_DOWN_REG 0x93

/* Minimum distance while Moving Left and Moving Right gesture */
#define FT6206_DISTANCE_LEFT_RIGHT_REG 0x94

/* Minimum distance while Moving Up and Moving Down gesture */
#define FT6206_DISTANCE_UP_DOWN_REG 0x95

/* Maximum distance while Zoom In and Zoom Out gesture */
#define FT6206_DISTANCE_ZOOM_REG 0x96

/* High 8-bit of LIB Version info */
#define FT6206_LIB_VER_H_REG 0xA1

/* Low 8-bit of LIB Version info */
#define FT6206_LIB_VER_L_REG 0xA2

/* Chip Selecting */
#define FT6206_CIPHER_REG 0xA3

/* Interrupt mode register (used when in interrupt mode) */
#define FT6206_GMODE_REG 0xA4

#define FT6206_G_MODE_INTERRUPT_MASK  0x03
#define FT6206_G_MODE_INTERRUPT_SHIFT 0x00

/* Possible values of FT6206_GMODE_REG */
#define FT6206_G_MODE_INTERRUPT_POLLING 0x00
#define FT6206_G_MODE_INTERRUPT_TRIGGER 0x01

/* Current power mode the FT6206 system is in (R) */
#define FT6206_PWR_MODE_REG 0xA5

/* FT6206 firmware version */
#define FT6206_FIRMID_REG 0xA6

/* FT6206 Chip identification register */
#define FT6206_CHIP_ID_REG 0xA8

/*  Possible values of FT6206_CHIP_ID_REG */
#define FT6206_ID_VALUE 0x11
#define FT6x36_ID_VALUE 0xCD

/* Release code version */
#define FT6206_RELEASE_CODE_ID_REG 0xAF

/* Current operating mode the FT6206 system is in (R) */
#define FT6206_STATE_REG 0xBC

#ifdef __cplusplus
}
#endif
#endif /* __FT6X06_H */
