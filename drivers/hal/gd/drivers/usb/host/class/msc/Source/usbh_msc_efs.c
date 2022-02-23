/*!
    \file  usbh_msc_efs.c
    \brief this file is the interface between file systems and host msc class

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


#include "efs.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bbb.h"
#include "usbh_msc_efs.h"

extern usbh_host usb_host;

/*!
    \brief      initialize the mass storage parameters
    \param[in]  file: file pointer
    \param[in]  opts: optional parameter, not used here
    \param[out] none
    \retval     status: 0 -> pass, -1 -> fail
*/
int8_t if_initInterface(hwInterface* file, char* opts)
{
    msc_lun info;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (pudev->host.connect_status) {
        if (USBH_OK == usbh_msc_lun_info_get(&usb_host, 0, &info)) {
            file->sectorCount = info.capacity.block_nbr;
        }
    }

    return(EFS_PASS);
}

/*!
    \brief      this function is responsible to read a sector from the disc and store it in a user supplied buffer
    \param[in]  file: file pointer
    \param[in]  address: an LBA address, relative to the beginning of the disc
    \param[in]  buf: buffer where the data will be stored after reading
    \param[out] none
    \retval     status: 0 -> pass, -1 -> fail
*/
int8_t if_readBuf(hwInterface* file, uint32_t address, uint8_t* buf)
{
    int8_t status = EFS_ERROR;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_read (&usb_host, 0, address, buf, 1);
        } while ((status == USBH_BUSY) && (pudev->host.connect_status));
    }

    return(status);
}

/*!
    \brief      this function is responsible to write a sector of data on the disc from a user supplied buffer
    \param[in]  file: file pointer
    \param[in]  address: an LBA address, relative to the beginning of the disc
    \param[in]  buf: buffer where the data will be taken to write
    \param[out] none
    \retval     status: 0 -> pass, -1 -> fail
*/
int8_t if_writeBuf(hwInterface* file, uint32_t address, uint8_t* buf)
{
    int8_t status = EFS_ERROR;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_write(&usb_host, 0, address, buf, 1);
        } while((status == USBH_BUSY ) && (pudev->host.connect_status));
    }

    return(status);
}

/*!
    \brief      this function is responsible to issue a test unit ready command.user can issue a test unit ready command by calling this function
    \param[in]  none
    \param[out] none
    \retval     status: 0 -> pass, -1 -> fail
*/
int8_t if_TestUnitReady(void)
{
    int8_t status = EFS_ERROR;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_test_unitready(&usb_host, 0U);
        } while ((status == USBH_BUSY ) && (pudev->host.connect_status));
    }

    return(status);
}

/*!
    \brief      this function is responsible to issue a requestsense command. user can issue a requestsense command by calling this function
    \param[in]  none
    \param[out] none
    \retval     status: 0 -> pass, -1 -> fail
*/
int8_t if_RequestSense(void)
{
    int8_t status = EFS_ERROR;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;
    msc_scsi_sense sense_data;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_request_sense(&usb_host, 0U, &sense_data);
        } while ((status == USBH_BUSY ) && (pudev->host.connect_status));
    }

    return(status);
}
