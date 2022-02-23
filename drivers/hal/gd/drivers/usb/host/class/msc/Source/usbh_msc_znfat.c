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


struct znFAT_IO_Ctl ioctl; //����������д��IO���ƣ��������������������������Ч��
extern UINT8 Dev_No; //�豸��
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

    if (buffer == znFAT_Buffer) //��������znFAT�ڲ��������Ĳ���
    {
        if (ioctl.just_dev == Dev_No  //�������Ҫ��ȡ���������ڲ���������Ӧ�������������һ�β�������������ͬһ����
            && (ioctl.just_sec == addr && 0 != ioctl.just_sec)) //���ٽ��ж�ȡ��ֱ�ӷ���
        {
            return 0;
        }
        else //���򣬾ͽ����һ�β������������Ϊ��ǰ����
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

    if (buffer == znFAT_Buffer) //������ݻ��������ڲ�����
    {
        ioctl.just_dev = Dev_No; //����Ϊ��ǰ�豸��
        ioctl.just_sec = addr; //����Ϊ��ǰ������������ַ
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

    for(i = 0; i < 512; i++) //����ڲ�����������������������0
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

    ioctl.just_dev=Dev_No; //����Ϊ��ǰ�豸��
    ioctl.just_sec=(addr+nsec-1); //����Ϊ��ǰ������������ַ	 

    return 0;  
}

