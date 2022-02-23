/*!
    \file  usbh_msc_znfat.c
    \brief USB MSC host znFAT related functions

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

#include "usb_conf.h"
#include "deviceio.h"
#include "usbh_msc_core.h"

extern usbh_host usb_host;


struct znFAT_IO_Ctl ioctl; //用于扇区读写的IO控制，尽量减少物理扇区操作，提高效率
extern UINT8 Dev_No; //设备号
extern UINT8 *znFAT_Buffer;

/*!
    \brief      initialize the disk drive
    \param[in]  none
    \param[out] none
    \retval     operation status
*/
UINT8 znFAT_Device_Init(void) 
{
    UINT8 res = 0, err = 0;

    ioctl.just_dev=0;
    ioctl.just_sec=0;

    if (res) {
        err |= 0x01;
    }

    return err;
}

/*!
    \brief      read single sector
    \param[in]  addr: start sector number (LBA)
    \param[in]  buffer: pointer to the data buffer to store read data
    \param[out] none
    \retval     operation status
*/
UINT8 znFAT_Device_Read_Sector(UINT32 addr, UINT8 *buffer)
{
    uint8_t status = USBH_OK;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (buffer == znFAT_Buffer) //如果是针对znFAT内部缓冲区的操作
    {
        if (ioctl.just_dev == Dev_No  //如果现在要读取的扇区与内部缓冲所对应的扇区（即最近一次操作的扇区）是同一扇区
            && (ioctl.just_sec == addr && 0 != ioctl.just_sec)) //则不再进行读取，直接返回
        {
            return 0;
        }
        else //否则，就将最近一次操作的扇区标记为当前扇区
        {
            ioctl.just_dev = Dev_No; 
            ioctl.just_sec = addr; 
        }
    }

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_read (&usb_host, 0, addr, buffer, 1);

            if (!pudev->host.connect_status) {
                return 1;
            }
        } while(status == USBH_BUSY);
    }

    if (status == USBH_OK) {
        return 0;
    }

    return 1;
}

/*!
    \brief      read multiple sector
    \param[in]  nsec: number of sector
    \param[in]  addr: start sector number (LBA)
    \param[in]  buffer: pointer to the data buffer to store read data
    \param[out] none
    \retval     operation status
*/
UINT8 znFAT_Device_Read_nSector(UINT32 nsec, UINT32 addr, UINT8 *buffer)
{
    uint8_t status = USBH_OK;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if(0 == nsec) return 0;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_read (&usb_host, 0, addr, buffer, nsec);

            if (!pudev->host.connect_status) {
                return 1;
            }
        } while(status == USBH_BUSY);
    }

    if (status == USBH_OK) {
        return 0;
    }

    return 1;
}

/*!
    \brief      write single sector
    \param[in]  addr: start sector number (LBA)
    \param[in]  buffer: pointer to the data buffer to store read data
    \param[out] none
    \retval     operation status
*/
UINT8 znFAT_Device_Write_Sector(UINT32 addr, UINT8 *buffer) 
{
    uint8_t status = USBH_OK;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (buffer == znFAT_Buffer) //如果数据缓冲区是内部缓冲
    {
        ioctl.just_dev = Dev_No; //更新为当前设备号
        ioctl.just_sec = addr; //更新为当前操作的扇区地址
    }

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_write (&usb_host, 0, addr, buffer, 1);

            if (!pudev->host.connect_status) {
                return 1;
            }
        } while(status == USBH_BUSY);
    }

    if (status == USBH_OK) {
        return 0;
    }

    return 1;
}

UINT8 znFAT_Device_Write_nSector(UINT32 nsec, UINT32 addr, UINT8 *buffer)
{
    uint8_t status = USBH_OK;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    if (0 == nsec) return 0;

    if (pudev->host.connect_status) {
        do {
            status = usbh_msc_write (&usb_host, 0U, addr, buffer, nsec);

            if (!pudev->host.connect_status) {
                return 1;
            }
        } while(status == USBH_BUSY);
    }

    if (status == USBH_OK) {
        return 0;
    }

    return 1;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

/*!
    \brief      I/O control function
    \param[in]  drv: physical drive number (0)
    \param[in]  ctrl: control code
    \param[in]  buff: pointer to the data buffer to store read data
    \param[out] none
    \retval     operation status
*/
UINT8 znFAT_Device_Clear_nSector(UINT32 nsec, UINT32 addr)
{
    UINT32 i = 0;
    uint8_t status = USBH_OK;
    usb_core_driver *pudev = (usb_core_driver *)usb_host.data;

    for(i = 0; i < 512; i++) //清空内部缓冲区，用于连续扇区清0
    {
        znFAT_Buffer[i]=0;
    }

    if (pudev->host.connect_status) {
        for (i = 0; i < nsec; i++) {
            do {
                status = usbh_msc_write (&usb_host, 0, addr + i, znFAT_Buffer, 1);

                if (!pudev->host.connect_status) {
                    return 1;
                }
            } while(status == USBH_BUSY);
        }
    }

    if (status == USBH_OK) {
        return 0;
    }

    ioctl.just_dev=Dev_No; //更新为当前设备号
    ioctl.just_sec=(addr+nsec-1); //更新为当前操作的扇区地址	 

    return 0;  
}

