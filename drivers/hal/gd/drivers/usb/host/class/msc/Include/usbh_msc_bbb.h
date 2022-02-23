/*!
    \file  usbh_msc_bbb.h
    \brief header file for usbh_msc_bbb.c

    \version 2018-10-08, V1.0.0, firmware for GD32 USBFS&USBHS
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/ 

#ifndef __USBH_MSC_BBB_H
#define __USBH_MSC_BBB_H

#include "usbh_enum.h"
#include "msc_bbb.h"

typedef union {
    msc_bbb_cbw field;

    uint8_t CBWArray[31];
}usbh_cbw_pkt;

typedef union {
    msc_bbb_csw field;

    uint8_t CSWArray[13];
}usbh_csw_pkt;

enum usbh_msc_state {
    USBH_MSC_BOT_INIT_STATE = 0,
    USBH_MSC_BOT_RESET,
    USBH_MSC_GET_MAX_LUN,
    USBH_MSC_TEST_UNIT_READY,
    USBH_MSC_READ_CAPACITY10,
    USBH_MSC_MODE_SENSE6,
    USBH_MSC_REQUEST_SENSE,
    USBH_MSC_BOT_USB_TRANSFERS,
    USBH_MSC_DEFAULT_APPLI_STATE,
    USBH_MSC_CTRL_ERROR_STATE,
    USBH_MSC_UNRECOVERED_STATE
};

typedef enum
{
    BOT_OK = 0,
    BOT_FAIL,
    BOT_PHASE_ERROR,
    BOT_BUSY
} bot_status;

typedef enum
{
    BOT_CMD_IDLE = 0,
    BOT_CMD_SEND,
    BOT_CMD_WAIT,
} bot_cmd_state;

/* csw status definitions */
typedef enum
{
    BOT_CSW_CMD_PASSED = 0x00,
    BOT_CSW_CMD_FAILED,
    BOT_CSW_PHASE_ERROR,
} bot_csw_status;

typedef enum
{
    BOT_SEND_CBW = 1U,
    BOT_SEND_CBW_WAIT,
    BOT_DATA_IN,
    BOT_DATA_IN_WAIT,
    BOT_DATA_OUT,
    BOT_DATA_OUT_WAIT,
    BOT_RECEIVE_CSW,
    BOT_RECEIVE_CSW_WAIT,
    BOT_ERROR_IN,
    BOT_ERROR_OUT,
    BOT_UNRECOVERED_ERROR
} bot_state;

typedef struct
{
    uint8_t                *pbuf;
    uint32_t                data[16];
    bot_state               state;
    bot_state               prev_state;
    bot_cmd_state           cmd_state;
    usbh_cbw_pkt            cbw;
    usbh_csw_pkt            csw;
} bot_handle;


#define USBH_MSC_BOT_CBW_TAG                0x20304050

#define USBH_MSC_CSW_MAX_LENGTH             63

#define USBH_MSC_SEND_CSW_DISABLE           0
#define USBH_MSC_SEND_CSW_ENABLE            1

#define USBH_MSC_DIR_IN                     0
#define USBH_MSC_DIR_OUT                    1
#define USBH_MSC_BOTH_DIR                   2

#define USBH_MSC_PAGE_LENGTH                512

#define CBW_CB_LENGTH                       16
#define CBW_LENGTH                          10
#define CBW_LENGTH_TEST_UNIT_READY          0

#define MAX_BULK_STALL_COUNT_LIMIT          0x04   /* If STALL is seen on Bulk 
                                                      Endpoint continously, this means 
                                                      that device and Host has phase error
                                                      Hence a Reset is needed */

void usbh_msc_bot_init (usbh_host *puhost);

usbh_status usbh_msc_bot_process (usbh_host *puhost, uint8_t lun);

bot_csw_status usbh_msc_csw_decode (usbh_host *puhost);

usbh_status usbh_msc_bot_reset (usbh_host *puhost);

usbh_status usbh_msc_bot_abort (usbh_host *puhost, uint8_t direction);

#endif  /* __USBH_MSC_BOT_H */
