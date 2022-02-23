/**
  ******************************************************************************
  * @file    usbh_hid_parser.c
  * @author  MCD Application Team
  * @brief   This file is the header file of the usbh_hid_parser.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */


#ifndef __USBH_HID_PARSER_H
#define __USBH_HID_PARSER_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "usbh_hid_core.h"
#include "usbh_hid_usage.h"




/** @defgroup USBH_HID_PARSER_Exported_Types
  * @{
  */
typedef struct
{
    uint8_t    *data;
    uint32_t    size;
    uint8_t     shift;
    uint8_t     count;
    uint8_t     sign;
    uint32_t    logical_min;  /*min value device can return*/
    uint32_t    logical_max;  /*max value device can return*/
    uint32_t    physical_min; /*min vale read can report*/
    uint32_t    physical_max; /*max value read can report*/
    uint32_t    resolution;
} hid_report_item;


uint32_t hid_item_read (hid_report_item *ri, uint8_t ndx);

uint32_t hid_item_write (hid_report_item *ri, uint32_t value, uint8_t ndx);

#ifdef __cplusplus
    }
#endif

#endif /* __USBH_HID_PARSER_H */
