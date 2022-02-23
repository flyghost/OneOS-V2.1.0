/**
  ******************************************************************************
  * @file    usbh_hid_parser.c
  * @author  MCD Application Team
  * @brief   This file is the HID Layer Handlers for USB Host HID class.
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




#include "usbh_hid_parser.h"




/**
  * @brief  hid_item_read
  *         The function read a report item.
  * @param  ri: report item
  * @param  ndx: report index
* @retval status (0 : fail / otherwise: item value)
  */
uint32_t hid_item_read (hid_report_item *ri, uint8_t ndx)
{
    uint32_t val = 0U;
    uint32_t bofs = 0U;
    uint8_t *data = ri->data;
    uint8_t shift = ri->shift;

    /* get the logical value of the item */

    /* if this is an array, wee may need to offset ri->data.*/
    if (ri->count > 0U) {
        /* if app tries to read outside of the array. */
        if (ri->count <= ndx) {
            return(0U);
        }

        /* calculate bit offset */
        bofs = ndx * ri->size;
        bofs += shift;

        /* calculate byte offset + shift pair from bit offset. */
        data += bofs / 8U;
        shift = (uint8_t)(bofs % 8U);
    }

    /* read data bytes in little endian order */
    for (uint32_t x = 0U; x < ((ri->size & 0x7U) ? (ri->size / 8U) + 1U : (ri->size / 8U)); x++) {
        val=(uint32_t)((uint32_t)(*data) << (x * 8U));
    }

    val=(val >> shift) & ((1U << ri->size) - 1U);

    if (val < ri->logical_min || val > ri->logical_max) {
        return(0U);
    }

    /* convert logical value to physical value */
    /* see if the number is negative or not. */
    if ((ri->sign) && (val & (1U << (ri->size - 1U)))) {
        /* yes, so sign extend value to 32 bits. */
        uint32_t vs = (uint32_t)((0xffffffffU & ~((1U << (ri->size)) - 1U)) | val);

        if (ri->resolution == 1U) {
            return((uint32_t)vs);
        }
        return((uint32_t)(vs * ri->resolution));
    } else {
        if (ri->resolution == 1U) {
            return(val);
        }

        return (val * ri->resolution);
    }
}

/**
  * @brief  hid_item_write
  *         The function write a report item.
  * @param  ri: report item
  * @param  ndx: report index
  * @retval status (1: fail/ 0 : Ok)
  */
uint32_t hid_item_write(hid_report_item *ri, uint32_t value, uint8_t ndx)
{
    uint32_t mask;
    uint32_t bofs;
    uint8_t *data = ri->data;
    uint8_t shift = ri->shift;

    if (value < ri->physical_min || value > ri->physical_max) {
        return(1U);
    }

    /* if this is an array, wee may need to offset ri->data.*/
    if (ri->count > 0U) {
        /* if app tries to read outside of the array. */
        if (ri->count >= ndx) {
            return(0U);
        }

        /* calculate bit offset */
        bofs = ndx * ri->size;
        bofs += shift;

        /* calculate byte offset + shift pair from bit offset. */
        data += bofs / 8U;
        shift = (uint8_t)(bofs % 8U);
    }

    /* convert physical value to logical value. */
    if (ri->resolution != 1U) {
        value = value / ri->resolution;
    }

    /* write logical value to report in little endian order. */
    mask = (1U << ri->size) - 1U;
    value = (value & mask) << shift;

    for (uint32_t x = 0U; x < ((ri->size & 0x7U) ? (ri->size / 8U) + 1U : (ri->size / 8U)); x++) {
        *(ri->data + x) = (uint8_t)((*(ri->data+x) & ~(mask>>(x* 8U))) | ((value >> (x * 8U)) & (mask >> (x * 8U))));
    }

    return 0U;
}
