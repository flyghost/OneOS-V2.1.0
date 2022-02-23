/*!
    \file  usbh_conf.h
    \brief general USB host driver configuration

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

#ifndef __USBH_CONF_H
#define __USBH_CONF_H

#define USBH_MAX_EP_NUM                       3
#define USBH_MAX_INTERFACES_NUM               5
#define USBH_MAX_ALT_SETTING                  2
#define USBH_MAX_SUPPORTED_CLASS              2

#define USBH_DATA_BUF_MAX_LEN                 0x200
#define USBH_CFGSET_MAX_LEN                   0x200

#define USBH_USE_RTOS

#define USBH_USE_FREERTOS                     1
#define USBH_USE_UCOSII                       0
#define USBH_USE_RTX                          0

#if (USBH_USE_FREERTOS == 1)
    #include "cmsis_os.h"
    #define   USBH_PROCESS_PRIO               20
    #define   USBH_PROCESS_STACK_SIZE         (8 * configMINIMAL_STACK_SIZE)
#endif

typedef enum {
    APP_IDLE = 0,
    APP_DISCONNECT,
    APP_READY,
}cdc_app_state;

#endif /* _USBH_CONF_H */
