/*!
    \file  usbh_mtp_ptp.h
    \brief header file for USB host PTP in MTP driver

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

#ifndef __USBH_MTP_PTP_H
#define __USBH_MTP_PTP_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "usbh_core.h"

#define LE16(addr)        (((uint16_t)(addr)[0]) | \
                           ((uint16_t)(((uint32_t)(addr)[1]) << 8)))

#define LE24(addr)        (((uint32_t)(addr)[0]) | \
                           (((uint32_t)(addr)[1]) << 8) | \
                           (((uint32_t)(addr)[2]) << 16))

#define LE32(addr)        (((uint32_t)(addr)[0]) | \
                           (((uint32_t)(addr)[1]) << 8) | \
                           (((uint32_t)(addr)[2]) << 16) | \
                           (((uint32_t)(addr)[3]) << 24))

#define LE64(addr)        (((uint64_t)(addr)[0]) | \
                           (((uint64_t)(addr)[1]) << 8) | \
                           (((uint64_t)(addr)[2]) << 16) | \
                           (((uint64_t)(addr)[3]) << 24) | \
                           (((uint64_t)(addr)[4]) << 32) | \
                           (((uint64_t)(addr)[5]) << 40) | \
                           (((uint64_t)(addr)[6]) << 48) | \
                           (((uint64_t)(addr)[7]) << 56))

#define LE16S(addr)       ((int16_t)(LE16((addr))))
#define LE24S(addr)       ((int32_t)(LE24((addr))))
#define LE32S(addr)       ((int32_t)(LE32((addr))))
#define LE64S(addr)       ((int64_t)(LE64((addr))))

/* Operation Codes */

/* PTP v1.0 operation codes */
#define PTP_OC_UNDEFINED                                    0x1000U
#define PTP_OC_GET_DEVICE_INFO                              0x1001U
#define PTP_OC_OPEN_SESSION                                 0x1002U
#define PTP_OC_CLOSE_SESSION                                0x1003U
#define PTP_OC_GET_STORAGE_IDS                              0x1004U
#define PTP_OC_GET_STORAGE_INFO                             0x1005U
#define PTP_OC_GET_NUM_OBJECTS                              0x1006U
#define PTP_OC_GET_OBJECT_HANDLES                           0x1007U
#define PTP_OC_GET_OBJECT_INFO                              0x1008U
#define PTP_OC_GET_OBJECT                                   0x1009U
#define PTP_OC_GET_THUMB                                    0x100AU
#define PTP_OC_DELETE_OBJECT                                0x100BU
#define PTP_OC_SEND_OBJECT_INFO                             0x100CU
#define PTP_OC_SEND_OBJECT                                  0x100DU
#define PTP_OC_INITIATE_CAPTURE                             0x100EU
#define PTP_OC_FORMAT_STORE                                 0x100FU
#define PTP_OC_RESET_DEVICE                                 0x1010U
#define PTP_OC_SELF_TEST                                    0x1011U
#define PTP_OC_SET_OBJECT_PROTECTION                        0x1012U
#define PTP_OC_POWER_DOWN                                   0x1013U
#define PTP_OC_GET_DEVICE_PROP_DESC                         0x1014U
#define PTP_OC_GET_DEVICE_PROP_VALUE                        0x1015U
#define PTP_OC_SET_DEVICE_PROP_VALUE                        0x1016U
#define PTP_OC_RESET_DEVICE_PROP_VALUE                      0x1017U
#define PTP_OC_TERMINATE_OPEN_CAPTURE                       0x1018U
#define PTP_OC_MOVE_OBJECT                                  0x1019U
#define PTP_OC_COPY_OBJECT                                  0x101AU
#define PTP_OC_GET_PARTIAL_OBJECT                           0x101BU
#define PTP_OC_INITIATE_OPEN_CAPTURE                        0x101CU

/* PTP v1.1 operation codes */
#define PTP_OC_START_ENUM_HANDLES                           0x101DU
#define PTP_OC_ENUM_HANDLES                                 0x101EU
#define PTP_OC_STOP_ENUM_HANDLES                            0x101FU
#define PTP_OC_GET_VENDOR_EXTENSION_MAPS                    0x1020U
#define PTP_OC_GET_VENDOR_DEVICE_INFO                       0x1021U
#define PTP_OC_GET_RESIZED_IMAGE_OBJECT                     0x1022U
#define PTP_OC_GET_FILE_SYSTEM_MANIFEST                     0x1023U
#define PTP_OC_GET_STREAM_INFO                              0x1024U
#define PTP_OC_GET_STREAM                                   0x1025U

/* Microsoft / MTP extension codes */
#define PTP_OC_GET_OBJECT_PROPS_SUPPORTED                   0x9801U
#define PTP_OC_GET_OBJECT_PROP_DESC                         0x9802U
#define PTP_OC_GET_OBJECT_PROP_VALUE                        0x9803U
#define PTP_OC_SET_OBJECT_PROP_VALUE                        0x9804U
#define PTP_OC_GET_OBJ_PROPLIST                             0x9805U
#define PTP_OC_SET_OBJ_PROPLIST                             0x9806U
#define PTP_OC_GET_INTERDEPENDEND_PROP_DESC                 0x9807U
#define PTP_OC_SEND_OBJECT_PROP_LIST                        0x9808U
#define PTP_OC_GET_OBJECT_REFERENCES                        0x9810U
#define PTP_OC_SET_OBJECT_REFERENCES                        0x9811U
#define PTP_OC_UPDATE_DEVICE_FIRMWARE                       0x9812U
#define PTP_OC_SKIP                                         0x9820U


/* Response Codes */

/* PTP v1.0 response codes */
#define PTP_RC_UNDEFINED                                    0x2000U
#define PTP_RC_OK                                           0x2001U
#define PTP_RC_GENERAL_ERROR                                0x2002U
#define PTP_RC_SESSION_NOT_OPEN                             0x2003U
#define PTP_RC_INVALID_TRANSACTION_ID                       0x2004U
#define PTP_RC_OPERATION_NOT_SUPPORTED                      0x2005U
#define PTP_RC_PARAMETER_NOT_SUPPORTED                      0x2006U
#define PTP_RC_INCOMPLETE_TRANSFER                          0x2007U
#define PTP_RC_INVALID_STORAGE_ID                           0x2008U
#define PTP_RC_INVALID_OBJECT_HANDLE                        0x2009U
#define PTP_RC_DEVICE_PROP_NOT_SUPPORTED                    0x200AU
#define PTP_RC_INVALID_OBJECT_FORMAT_CODE                   0x200BU
#define PTP_RC_STORE_FULL                                   0x200CU
#define PTP_RC_OBJECT_WRITE_PROTECTED                       0x200DU
#define PTP_RC_STORE_READ_ONLY                              0x200EU
#define PTP_RC_ACCESS_DENIED                                0x200FU
#define PTP_RC_NOT_HUMBNAIL_PRESENT                         0x2010U
#define PTP_RC_SELF_TEST_FAILED                             0x2011U
#define PTP_RC_PARTIAL_DELETION                             0x2012U
#define PTP_RC_STORE_NOT_AVAILABLE                          0x2013U
#define PTP_RC_SPECIFICATION_BY_FORMAT_UNSUPPORTED          0x2014U
#define PTP_RC_NO_VALID_OBJECT_INFO                         0x2015U
#define PTP_RC_INVALID_CODE_FORMAT                          0x2016U
#define PTP_RC_UNKNOWN_VENDOR_CODE                          0x2017U
#define PTP_RC_CAPTURE_ALREADY_TERMINATED                   0x2018U
#define PTP_RC_DEVICE_BUSY                                  0x2019U
#define PTP_RC_INVALID_PARENT_OBJECT                        0x201AU
#define PTP_RC_INVALID_DEVICE_PROP_FORMAT                   0x201BU
#define PTP_RC_INVALID_DEVICE_PROP_VALUE                    0x201CU
#define PTP_RC_INVALID_PARAMETER                            0x201DU
#define PTP_RC_SESSIONAL_READY_OPENED                       0x201EU
#define PTP_RC_TRANSACTION_CANCELED                         0x201FU
#define PTP_RC_SPECIFICATION_OF_DESTINATION_UNSUPPORTED     0x2020U

/* PTP v1.1 response codes */
#define PTP_RC_INVALID_ENUM_HANDLE                          0x2021U
#define PTP_RC_NO_STREAM_ENABLED                            0x2022U
#define PTP_RC_INVALID_DATA_SET                             0x2023U

/* USB container types */
#define PTP_USB_CONTAINER_UNDEFINED                         0x0000U
#define PTP_USB_CONTAINER_COMMAND                           0x0001U
#define PTP_USB_CONTAINER_DATA                              0x0002U
#define PTP_USB_CONTAINER_RESPONSE                          0x0003U
#define PTP_USB_CONTAINER_EVENT                             0x0004U

/* PTP/IP definitions */
#define PTPIP_INIT_COMMAND_REQUEST                          1U
#define PTPIP_INIT_COMMAND_ACK                              2U
#define PTPIP_INIT_EVENT_REQUEST                            3U
#define PTPIP_INIT_EVENT_ACK                                4U
#define PTPIP_INIT_FAIL                                     5U
#define PTPIP_CMD_REQUEST                                   6U
#define PTPIP_CMD_RESPONSE                                  7U
#define PTPIP_EVENT                                         8U
#define PTPIP_START_DATA_PACKET                             9U
#define PTPIP_DATA_PACKET                                   10U
#define PTPIP_CANCEL_TRANSACTION                            11U
#define PTPIP_END_DATA_PACKET                               12U
#define PTPIP_PING                                          13U
#define PTPIP_PONG                                          14U

/* Transaction data phase description */
#define PTP_DP_NODATA                                       0x0000U /* no data phase */
#define PTP_DP_SENDDATA                                     0x0001U /* sending data */
#define PTP_DP_GETDATA                                      0x0002U /* receiving data */
#define PTP_DP_DATA_MASK                                    0x00ffU /* data phase mask */

typedef enum
{
    PTP_REQ_IDLE = 0,
    PTP_REQ_SEND,
    PTP_REQ_WAIT,
    PTP_REQ_ERROR,
} ptp_request_state;

typedef enum
{
    PTP_IDLE = 0,
    PTP_OP_REQUEST_STATE,
    PTP_OP_REQUEST_WAIT_STATE,
    PTP_DATA_OUT_PHASE_STATE,
    PTP_DATA_OUT_PHASE_WAIT_STATE,
    PTP_DATA_IN_PHASE_STATE,
    PTP_DATA_IN_PHASE_WAIT_STATE,
    PTP_RESPONSE_STATE,
    PTP_RESPONSE_WAIT_STATE,
    PTP_ERROR,
} ptp_process_state;

/* PTP request/response/event general PTP container (transport independent) */
typedef struct
{
    uint16_t Code;
    uint32_t SessionID;
    uint32_t Transaction_ID;
    /* params  may be of any type of size less or equal to uint32_t */
    uint32_t Param1;
    uint32_t Param2;
    uint32_t Param3;
    /* events can only have three parameters */
    uint32_t Param4;
    uint32_t Param5;
    /* the number of meaningful parameters */
    uint8_t Nparam;
} ptp_container;

#define PTP_USB_BULK_HS_MAX_PACKET_LEN_WRITE              1024U
#define PTP_USB_BULK_HS_MAX_PACKET_LEN_READ               1024U
#define PTP_USB_BULK_HDR_LEN                              (2U * sizeof(uint32_t) + 2U * sizeof(uint16_t))
#define PTP_USB_BULK_PAYLOAD_LEN_WRITE                    (PTP_USB_BULK_HS_MAX_PACKET_LEN_WRITE - PTP_USB_BULK_HDR_LEN)
#define PTP_USB_BULK_PAYLOAD_LEN_READ                     (PTP_USB_BULK_HS_MAX_PACKET_LEN_READ - PTP_USB_BULK_HDR_LEN)
#define PTP_USB_BULK_REQ_LEN                              (PTP_USB_BULK_HDR_LEN + 5U * sizeof(uint32_t))
#define PTP_USB_BULK_REQ_RESP_MAX_LEN                     63U

typedef struct
{
    uint32_t length;
    uint16_t type;
    uint16_t code;
    uint32_t trans_id;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
    uint32_t param5;
} ptp_resp_container;

typedef struct
{
    uint32_t length;
    uint16_t type;
    uint16_t code;
    uint32_t trans_id;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
    uint32_t param5;
} ptp_op_container;

typedef struct
{
    uint32_t length;
    uint16_t type;
    uint16_t code;
    uint32_t trans_id;

    union {
        struct {
            uint32_t param1;
            uint32_t param2;
            uint32_t param3;
            uint32_t param4;
            uint32_t param5;
        } params;
        uint8_t  data[PTP_USB_BULK_PAYLOAD_LEN_READ];
    } payload;
} ptp_data_container;

/* PTP USB Asynchronous Event Interrupt Data Format */
typedef struct
{
    uint32_t length;
    uint16_t type;
    uint16_t code;
    uint32_t trans_id;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
} ptp_event_container;

/* Structure for PTP Transport process */
typedef struct
{
    ptp_process_state      state;
    ptp_request_state      req_state;
    ptp_op_container       op_container;
    ptp_data_container     data_container;
    ptp_resp_container     resp_container;


    uint32_t               transaction_id;
    uint32_t               session_id;
    uint32_t               flags;

    /* PTP transfer control */
    uint8_t               *data_ptr;
    uint32_t               data_length;
    uint32_t               data_packet;
    uint32_t               iteration;
    uint32_t               data_packet_counter;

    /* object transfer control */
    uint8_t               *object_ptr;
} ptp_handle;

/* DeviceInfo data offset */
#define PTP_DI_STANDARD_VERSION                             0U
#define PTP_DI_VENDOR_EXTENSION_ID                          2U
#define PTP_DI_VENDOR_EXTENSION_VERSION                     6U
#define PTP_DI_VENDOR_EXTENSION_DESC                        8U
#define PTP_DI_FUNCTIONAL_MODE                              8U
#define PTP_DI_OPERATIONS_SUPPORTED                         10U

/* Max info items size */
#define PTP_SUPPORTED_OPERATIONS_NBR                        100U
#define PTP_SUPPORTED_EVENTS_NBR                            100U
#define PTP_SUPPORTED_PROPRIETIES_NBR                       100U
#define PTP_CAPTURE_FORMATS_NBR                             100U
#define PTP_IMAGE_FORMATS_NBR                               100U
#define PTP_MAX_STR_SIZE                                    255U

/* PTP device info structure */
typedef struct
{
    uint16_t StandardVersion;
    uint32_t VendorExtensionID;
    uint16_t VendorExtensionVersion;
    uint8_t  VendorExtensionDesc[PTP_MAX_STR_SIZE];
    uint16_t FunctionalMode;
    uint32_t OperationsSupported_len;
    uint16_t OperationsSupported[PTP_SUPPORTED_OPERATIONS_NBR];
    uint32_t EventsSupported_len;
    uint16_t EventsSupported[PTP_SUPPORTED_EVENTS_NBR];
    uint32_t DevicePropertiesSupported_len;
    uint16_t DevicePropertiesSupported[PTP_SUPPORTED_PROPRIETIES_NBR];
    uint32_t CaptureFormats_len;
    uint16_t CaptureFormats[PTP_CAPTURE_FORMATS_NBR];
    uint32_t ImageFormats_len;
    uint16_t ImageFormats[PTP_IMAGE_FORMATS_NBR];
    uint8_t  Manufacturer[PTP_MAX_STR_SIZE];
    uint8_t  Model[PTP_MAX_STR_SIZE];
    uint8_t  DeviceVersion[PTP_MAX_STR_SIZE];
    uint8_t  SerialNumber[PTP_MAX_STR_SIZE];
}ptp_device_info;

#define PTP_MAX_STORAGE_UNITS_NBR                           5

/* PTP storageIDs structute (returned by GetStorageIDs) */
typedef struct
{
    uint32_t n;
    uint32_t Storage [PTP_MAX_STORAGE_UNITS_NBR];
} ptp_storage_ID;

/* PTP storage information structure (returned by GetStorageInfo) */
#define PTP_SI_STORAGE_TYPE                                 0U
#define PTP_SI_FILE_SYSTEM_TYPE                             2U
#define PTP_SI_ACCESS_CAPABILITY                            4U
#define PTP_SI_MAX_CAPABILITY                               6U
#define PTP_SI_FREE_SPACE_IN_BYTES                          14U
#define PTP_SI_FREE_SPACE_IN_IMAGES                         22U
#define PTP_SI_STORAGE_DESCRIPTION                          26U


/* PTP storage types */
#define PTP_ST_UNDEFINED                                    0x0000U
#define PTP_ST_FIXEDROM                                     0x0001U
#define PTP_ST_REMOVABLEROM                                 0x0002U
#define PTP_ST_FIXEDRAM                                     0x0003U
#define PTP_ST_REMOVABLERAM                                 0x0004U

/* PTP filesystem type values */
#define PTP_FST_UNDEFINED                                   0x0000U
#define PTP_FST_GENERIC_FLAT                                0x0001U
#define PTP_FST_GENERIC_HIERARCHICAL                        0x0002U
#define PTP_FST_DCF                                         0x0003U

/* PTP storage info accesscapability values */
#define PTP_AC_READ_WRITE                                   0x0000U
#define PTP_AC_READ_ONLY                                    0x0001U
#define PTP_AC_READ_ONLY_WITH_OBJECT_DELETION               0x0002U

typedef struct
{
    uint16_t StorageType;
    uint16_t FilesystemType;
    uint16_t AccessCapability;
    uint64_t MaxCapability;
    uint64_t FreeSpaceInBytes;
    uint32_t FreeSpaceInImages;
    uint8_t  StorageDescription[PTP_MAX_STR_SIZE];
    uint8_t  VolumeLabel[PTP_MAX_STR_SIZE];
}ptp_storage_info;

/* PTP object format codes */

/* ancillary formats */
#define PTP_OFC_UNDEFINED                                   0x3000U
#define PTP_OFC_DEFINED                                     0x3800U
#define PTP_OFC_ASSOCIATION                                 0x3001U
#define PTP_OFC_SCRIPT                                      0x3002U
#define PTP_OFC_EXECUTABLE                                  0x3003U
#define PTP_OFC_TEXT                                        0x3004U
#define PTP_OFC_HTML                                        0x3005U
#define PTP_OFC_DPOF                                        0x3006U
#define PTP_OFC_AIFF                                        0x3007U
#define PTP_OFC_WAV                                         0x3008U
#define PTP_OFC_MP3                                         0x3009U
#define PTP_OFC_AVI                                         0x300AU
#define PTP_OFC_MPEG                                        0x300BU
#define PTP_OFC_ASF                                         0x300CU
#define PTP_OFC_QT                                          0x300DU /* guessing */

/* image formats */
#define PTP_OFC_EXIF_JPEG                                   0x3801U
#define PTP_OFC_TIFF_EP                                     0x3802U
#define PTP_OFC_FLASHPIX                                    0x3803U
#define PTP_OFC_BMP                                         0x3804U
#define PTP_OFC_CIFF                                        0x3805U
#define PTP_OFC_UNDEFINED_0x3806                            0x3806U
#define PTP_OFC_GIF                                         0x3807U
#define PTP_OFC_JFIF                                        0x3808U
#define PTP_OFC_PCD                                         0x3809U
#define PTP_OFC_PICT                                        0x380AU
#define PTP_OFC_PNG                                         0x380BU
#define PTP_OFC_UNDEFINED_0x380C                            0x380CU
#define PTP_OFC_TIFF                                        0x380DU
#define PTP_OFC_TIFF_IT                                     0x380EU
#define PTP_OFC_JP2                                         0x380FU
#define PTP_OFC_JPX                                         0x3810U

/* ptp v1.1 has only DNG new */
#define PTP_OFC_DNG                                         0x3811U

/* MTP extensions */
#define PTP_OFC_MTP_MEDIA_CARD                              0xb211U
#define PTP_OFC_MTP_MEDIA_CARD_GROUP                        0xb212U
#define PTP_OFC_MTP_ENCOUNTER                               0xb213U
#define PTP_OFC_MTP_ENCOUNTER_BOX                           0xb214U
#define PTP_OFC_MTP_M4A                                     0xb215U
#define PTP_OFC_MTP_ZUNE_UNDEFINED                          0xb217U /* unknown file type */
#define PTP_OFC_MTP_FIRMWARE                                0xb802U
#define PTP_OFC_MTP_WINDOWS_IMAGE_FORMAT                    0xb881U
#define PTP_OFC_MTP_UNDEFINED_AUDIO                         0xb900U
#define PTP_OFC_MTP_WMA                                     0xb901U
#define PTP_OFC_MTP_OGG                                     0xb902U
#define PTP_OFC_MTP_AAC                                     0xb903U
#define PTP_OFC_MTP_AUDIBLECOD                              0xb904U
#define PTP_OFC_MTP_FLAC                                    0xb906U
#define PTP_OFC_MTP_SAMSUNG_PLAY_LIST                       0xb909U
#define PTP_OFC_MTP_UNDEFINED_VIDEO                         0xb980U
#define PTP_OFC_MTP_WMV                                     0xb981U
#define PTP_OFC_MTP_MP4                                     0xb982U
#define PTP_OFC_MTP_MP2                                     0xb983U
#define PTP_OFC_MTP_3GP                                     0xb984U
#define PTP_OFC_MTP_UNDEFINED_COLLECTION                    0xba00U
#define PTP_OFC_MTP_ABSTRACT_MULTIMEDIA_ALBUM               0xba01U
#define PTP_OFC_MTP_ABSTRACT_IMAGE_ALBUM                    0xba02U
#define PTP_OFC_MTP_ABSTRACT_AUDIO_ALBUM                    0xba03U
#define PTP_OFC_MTP_ABSTRACT_VIDEO_ALBUM                    0xba04U
#define PTP_OFC_MTP_ABSTRACT_AUDIO_VIDEO_PLAYLIST           0xba05U
#define PTP_OFC_MTP_ABSTRACT_CONTACT_GROUP                  0xba06U
#define PTP_OFC_MTP_ABSTRACT_MESSAGE_FOLDER                 0xba07U
#define PTP_OFC_MTP_ABSTRACT_CHAPTERED_PRODUCTION           0xba08U
#define PTP_OFC_MTP_ABSTRACT_AUDIO_PLAYLIST                 0xba09U
#define PTP_OFC_MTP_ABSTRACT_VIDEO_PLAYLIST                 0xba0aU
#define PTP_OFC_MTP_ABSTRACT_MEDIA_CAST                     0xba0bU
#define PTP_OFC_MTP_WPL_PLAYLIST                            0xba10U
#define PTP_OFC_MTP_M3U_PLAYLIST                            0xba11U
#define PTP_OFC_MTP_MPL_PLAYLIST                            0xba12U
#define PTP_OFC_MTP_ASX_PLAYLIST                            0xba13U
#define PTP_OFC_MTP_PLS_PLAYLIST                            0xba14U
#define PTP_OFC_MTP_UNDEFINED_DOCUMENT                      0xba80U
#define PTP_OFC_MTP_ABSTRACT_DOCUMENT                       0xba81U
#define PTP_OFC_MTP_XML_DOCUMENT                            0xba82U
#define PTP_OFC_MTP_MSWORD_DOCUMENT                         0xba83U
#define PTP_OFC_MTP_MHTCOMPILEDHTML_DOCUMENT                0xba84U
#define PTP_OFC_MTP_MSEXCEL_SPREAD_SHEET_XLS                0xba85U
#define PTP_OFC_MTP_MSPOWERPOINT_PRESENTATION_PPT           0xba86U
#define PTP_OFC_MTP_UNDEFINED_MESSAGE                       0xbb00U
#define PTP_OFC_MTP_ABSTRACT_MESSAGE                        0xbb01U
#define PTP_OFC_MTP_UNDEFINED_CONTACT                       0xBB80U
#define PTP_OFC_MTP_ABSTRACT_CONTACT                        0xBB81U
#define PTP_OFC_MTP_VCARD2                                  0xBB82U
#define PTP_OFC_MTP_VCARD3                                  0xBB83U
#define PTP_OFC_MTP_UNDEFINED_CALENDAR_ITEM                 0xBE00U
#define PTP_OFC_MTP_ABSTRACT_CALENDAR_ITEM                  0xBE01U
#define PTP_OFC_MTP_VCALENDAR1                              0xBE02U
#define PTP_OFC_MTP_VCALENDAR2                              0xBE03U
#define PTP_OFC_MTP_UNDEFINED_WINDOWS_EXECUTABLE            0xBE80U
#define PTP_OFC_MTP_MEDIA_CAST                              0xBE81U
#define PTP_OFC_MTP_SECTION                                 0xBE82U

/* MTP specific object properties */
#define PTP_OPC_STORAGE_ID                                  0xDC01U
#define PTP_OPC_OBJECT_FORMAT                               0xDC02U
#define PTP_OPC_PROTECTION_STATUS                           0xDC03U
#define PTP_OPC_OBJECT_SIZE                                 0xDC04U
#define PTP_OPC_ASSOCIATION_TYPE                            0xDC05U
#define PTP_OPC_ASSOCIATION_DESC                            0xDC06U
#define PTP_OPC_OBJECT_FILE_NAME                            0xDC07U
#define PTP_OPC_DATE_CREATED                                0xDC08U
#define PTP_OPC_DATE_MODIFIED                               0xDC09U
#define PTP_OPC_KEY_WORDS                                   0xDC0AU
#define PTP_OPC_PARENT_OBJECT                               0xDC0BU
#define PTP_OPC_ALLOWED_FOLDER_CONTENTS                     0xDC0CU
#define PTP_OPC_HIDDEN                                      0xDC0DU
#define PTP_OPC_SYSTEM_OBJECT                               0xDC0EU
#define PTP_OPC_PERSISTANT_UNIQUE_OBJECT_IDENTIFIER         0xDC41U
#define PTP_OPC_SYNC_ID                                     0xDC42U
#define PTP_OPC_PROPERTY_BAG                                0xDC43U
#define PTP_OPC_NAME                                        0xDC44U
#define PTP_OPC_CREATEDBY                                   0xDC45U
#define PTP_OPC_ARTIST                                      0xDC46U
#define PTP_OPC_DATE_AUTHORED                               0xDC47U
#define PTP_OPC_DESCRIPTION                                 0xDC48U
#define PTP_OPC_URL_REFERENCE                               0xDC49U
#define PTP_OPC_LANGUAGE_LOCALE                             0xDC4AU
#define PTP_OPC_COPYRIGHT_INFORMATION                       0xDC4BU
#define PTP_OPC_SOURCE                                      0xDC4CU
#define PTP_OPC_ORIGIN_LOCATION                             0xDC4DU
#define PTP_OPC_DATE_ADDED                                  0xDC4EU
#define PTP_OPC_NON_CONSUMABLE                              0xDC4FU
#define PTP_OPC_CORRUPT_OR_UNPLAYABLE                       0xDC50U
#define PTP_OPC_PRODUCER_SERIAL_NUMBER                      0xDC51U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_FORMAT                0xDC81U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_SIZE                  0xDC82U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_HEIGHT                0xDC83U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_WIDTH                 0xDC84U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_DURATION              0xDC85U
#define PTP_OPC_REPRESENTATIVE_SAMPLE_DATA                  0xDC86U
#define PTP_OPC_WIDTH                                       0xDC87U
#define PTP_OPC_HEIGHT                                      0xDC88U
#define PTP_OPC_DURATION                                    0xDC89U
#define PTP_OPC_RATING                                      0xDC8AU
#define PTP_OPC_TRACK                                       0xDC8BU
#define PTP_OPC_GENRE                                       0xDC8CU
#define PTP_OPC_CREDITS                                     0xDC8DU
#define PTP_OPC_LYRICS                                      0xDC8EU
#define PTP_OPC_SUBSCRIPTION_CONTENT_ID                     0xDC8FU
#define PTP_OPC_PRODUCED_BY                                 0xDC90U
#define PTP_OPC_USE_COUNT                                   0xDC91U
#define PTP_OPC_SKIP_COUNT                                  0xDC92U
#define PTP_OPC_LAST_ACCESSED                               0xDC93U
#define PTP_OPC_PARENTAL_RATING                             0xDC94U
#define PTP_OPC_METAGENRE                                   0xDC95U
#define PTP_OPC_COMPOSER                                    0xDC96U
#define PTP_OPC_EFFECTIVE_RATING                            0xDC97U
#define PTP_OPC_SUB_TITLE                                   0xDC98U
#define PTP_OPC_ORIGINAL_RELEASE_DATE                       0xDC99U
#define PTP_OPC_ALBUM_NAME                                  0xDC9AU
#define PTP_OPC_ALBUM_ARTIST                                0xDC9BU
#define PTP_OPC_MOOD                                        0xDC9CU
#define PTP_OPC_DRMSTATUS                                   0xDC9DU
#define PTP_OPC_SUB_DESCRIPTION                             0xDC9EU
#define PTP_OPC_IS_CROPPED                                  0xDCD1U
#define PTP_OPC_IS_COLOR_CORRECTED                          0xDCD2U
#define PTP_OPC_IMAGE_BIT_DEPTH                             0xDCD3U
#define PTP_OPC_FNUMBER                                     0xDCD4U
#define PTP_OPC_EXPOSURE_TIME                               0xDCD5U
#define PTP_OPC_EXPOSURE_INDEX                              0xDCD6U
#define PTP_OPC_DISPLAY_NAME                                0xDCE0U
#define PTP_OPC_BODY_TEXT                                   0xDCE1U
#define PTP_OPC_SUBJECT                                     0xDCE2U
#define PTP_OPC_PRIORITY                                    0xDCE3U
#define PTP_OPC_GIVEN_NAME                                  0xDD00U
#define PTP_OPC_MIDDLE_NAMES                                0xDD01U
#define PTP_OPC_FAMILY_NAME                                 0xDD02U
#define PTP_OPC_PREFIX                                      0xDD03U
#define PTP_OPC_SUFFIX                                      0xDD04U
#define PTP_OPC_PHONETIC_GIVEN_NAME                         0xDD05U
#define PTP_OPC_PHONETIC_FAMILY_NAME                        0xDD06U
#define PTP_OPC_EMAIL_PRIMARY                               0xDD07U
#define PTP_OPC_EMAIL_PERSONAL1                             0xDD08U
#define PTP_OPC_EMAIL_PERSONAL2                             0xDD09U
#define PTP_OPC_EMAIL_BUSINESS1                             0xDD0AU
#define PTP_OPC_EMAIL_BUSINESS2                             0xDD0BU
#define PTP_OPC_EMAIL_OTHERS                                0xDD0CU
#define PTP_OPC_PHONENUMBER_PRIMARY                         0xDD0DU
#define PTP_OPC_PHONENUMBER_PERSONAL                        0xDD0EU
#define PTP_OPC_PHONENUMBER_PERSONAL2                       0xDD0FU
#define PTP_OPC_PHONENUMBER_BUSINESS                        0xDD10U
#define PTP_OPC_PHONENUMBER_BUSINESS2                       0xDD11U
#define PTP_OPC_PHONENUMBER_MOBILE                          0xDD12U
#define PTP_OPC_PHONENUMBER_MOBILE2                         0xDD13U
#define PTP_OPC_FAXNUMBER_PRIMARY                           0xDD14U
#define PTP_OPC_FAXNUMBER_PERSONAL                          0xDD15U
#define PTP_OPC_FAXNUMBER_BUSINESS                          0xDD16U
#define PTP_OPC_PAGER_NUMBER                                0xDD17U
#define PTP_OPC_PHONENUMBER_OTHERS                          0xDD18U
#define PTP_OPC_PRIMARY_WEB_ADDRESS                         0xDD19U
#define PTP_OPC_PERSONAL_WEB_ADDRESS                        0xDD1AU
#define PTP_OPC_BUSINESS_WEB_ADDRESS                        0xDD1BU
#define PTP_OPC_INSTANT_MESSENGER_ADDRESS                   0xDD1CU
#define PTP_OPC_INSTANT_MESSENGER_ADDRESS2                  0xDD1DU
#define PTP_OPC_INSTANT_MESSENGER_ADDRESS3                  0xDD1EU
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL                0xDD1FU
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_LINE1          0xDD20U
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_LINE2          0xDD21U
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_CITY           0xDD22U
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_REGION         0xDD23U
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_POSTAL_CODE    0xDD24U
#define PTP_OPC_POSTAL_ADDRESS_PERSONAL_FULL_COUNTRY        0xDD25U
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_FULL                0xDD26U
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_LINE1               0xDD27U
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_LINE2               0xDD28U
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_CITY                0xDD29U
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_REGION              0xDD2AU
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE         0xDD2BU
#define PTP_OPC_POSTAL_ADDRESS_BUSINESS_COUNTRY             0xDD2CU
#define PTP_OPC_POSTAL_ADDRESS_OTHER_FULL                   0xDD2DU
#define PTP_OPC_POSTAL_ADDRESS_OTHER_LINE1                  0xDD2EU
#define PTP_OPC_POSTAL_ADDRESS_OTHER_LINE2                  0xDD2FU
#define PTP_OPC_POSTAL_ADDRESS_OTHER_CITY                   0xDD30U
#define PTP_OPC_POSTAL_ADDRESS_OTHER_REGION                 0xDD31U
#define PTP_OPC_POSTAL_ADDRESS_OTHER_POSTAL_CODE            0xDD32U
#define PTP_OPC_POSTAL_ADDRESS_OTHER_COUNTRY                0xDD33U
#define PTP_OPC_ORGANIZATION_NAME                           0xDD34U
#define PTP_OPC_PHONETIC_ORGANIZATION_NAME                  0xDD35U
#define PTP_OPC_ROLE                                        0xDD36U
#define PTP_OPC_BIRTH_DATE                                  0xDD37U
#define PTP_OPC_MESSAGE_TO                                  0xDD40U
#define PTP_OPC_MESSAGE_CC                                  0xDD41U
#define PTP_OPC_MESSAGE_BCC                                 0xDD42U
#define PTP_OPC_MESSAGE_READ                                0xDD43U
#define PTP_OPC_MESSAGE_RECEIVED_TIME                       0xDD44U
#define PTP_OPC_MESSAGE_SENDER                              0xDD45U
#define PTP_OPC_ACTIVITY_BEGIN_TIME                         0xDD50U
#define PTP_OPC_ACTIVITY_END_TIME                           0xDD51U
#define PTP_OPC_ACTIVITY_LOCATION                           0xDD52U
#define PTP_OPC_ACTIVITY_REQUIRED_ATTENDEES                 0xDD54U
#define PTP_OPC_ACTIVITY_OPTIONAL_ATTENDEES                 0xDD55U
#define PTP_OPC_ACTIVITY_RESOURCES                          0xDD56U
#define PTP_OPC_ACTIVITY_ACCEPTED                           0xDD57U
#define PTP_OPC_OWNER                                       0xDD5DU
#define PTP_OPC_EDITOR                                      0xDD5EU
#define PTP_OPC_WEB_MASTER                                  0xDD5FU
#define PTP_OPC_URL_SOURCE                                  0xDD60U
#define PTP_OPC_URL_DESTINATION                             0xDD61U
#define PTP_OPC_TIME_BOOK_MARK                              0xDD62U
#define PTP_OPC_OBJECT_BOOK_MARK                            0xDD63U
#define PTP_OPC_BYTE_BOOK_MARK                              0xDD64U
#define PTP_OPC_LAST_BUILD_DATE                             0xDD70U
#define PTP_OPC_TIME_TO_LIVE                                0xDD71U
#define PTP_OPC_MEDIA_GUID                                  0xDD72U
#define PTP_OPC_TOTAL_BIT_RATE                              0xDE91U
#define PTP_OPC_BIT_RATE_TYPE                               0xDE92U
#define PTP_OPC_SAMPLE_RATE                                 0xDE93U
#define PTP_OPC_NUMBER_OF_CHANNELS                          0xDE94U
#define PTP_OPC_AUDIO_BIT_DEPTH                             0xDE95U
#define PTP_OPC_SCAN_DEPTH                                  0xDE97U
#define PTP_OPC_AUDIO_WAVE_CODEC                            0xDE99U
#define PTP_OPC_AUDIO_BIT_RATE                              0xDE9AU
#define PTP_OPC_VIDEO_FOURCC_CODEC                          0xDE9BU
#define PTP_OPC_VIDEO_BIT_RATE                              0xDE9CU
#define PTP_OPC_FRAMES_PER_THOUSAND_SECONDS                 0xDE9DU
#define PTP_OPC_KEY_FRAME_DISTANCE                          0xDE9EU
#define PTP_OPC_BUFFER_SIZE                                 0xDE9FU
#define PTP_OPC_ENCODING_QUALITY                            0xDEA0U
#define PTP_OPC_ENCODING_PROFILE                            0xDEA1U
#define PTP_OPC_BUY_FLAG                                    0xD901U

/* WIFI provisioning MTP extension property codes */
#define PTP_OPC_WIRELESS_CONFIGURATION_FILE                 0xB104U

/* PTP association types */
#define PTP_AT_UNDEFINED                                    0x0000U
#define PTP_AT_GENERIC_FOLDER                               0x0001U
#define PTP_AT_ALBUM                                        0x0002U
#define PTP_AT_TIME_SEQUENCE                                0x0003U
#define PTP_AT_HORIZONTAL_PANORAMIC                         0x0004U
#define PTP_AT_VERTICAL_PANORAMIC                           0x0005U
#define PTP_AT_2D_PANORAMIC                                 0x0006U
#define PTP_AT_ANCILLARY_DATA                               0x0007U

#define PTP_MAX_HANDLER_NBR                                 0x255U

typedef struct
{
    uint32_t n;
    uint32_t Handler[PTP_MAX_HANDLER_NBR];
}ptp_object_handle;


#define PTP_OI_STORAGE_ID                                   0U
#define PTP_OI_OBJECT_FORMAT                                4U
#define PTP_OI_PROTECTION_STATUS                            6U
#define PTP_OI_OBJECT_COMPRESSED_SIZE                       8U
#define PTP_OI_THUMB_FORMAT                                 12U
#define PTP_OI_THUMB_COMPRESSED_SIZE                        14U
#define PTP_OI_THUMB_PIX_WIDTH                              18U
#define PTP_OI_THUMB_PIX_HEIGHT                             22U
#define PTP_OI_IMAGE_PIX_WIDTH                              26U
#define PTP_OI_IMAGE_PIX_HEIGHT                             30U
#define PTP_OI_IMAGE_BIT_DEPTH                              34U
#define PTP_OI_PARENT_OBJECT                                38U
#define PTP_OI_ASSOCIATION_TYPE                             42U
#define PTP_OI_ASSOCIATION_DESC                             44U
#define PTP_OI_SEQUENCE_NUMBER                              48U
#define PTP_OI_FILE_NAME_LEN                                52U
#define PTP_OI_FILE_NAME                                    53U

typedef struct
{
    uint32_t StorageID;
    uint16_t ObjectFormat;
    uint16_t ProtectionStatus;
    /* in the regular object_info this is 32bit, but we keep the general object size here that also arrives via other methods and so use 64bit */
    uint64_t ObjectCompressedSize;
    uint16_t ThumbFormat;
    uint32_t ThumbCompressedSize;
    uint32_t ThumbPixWidth;
    uint32_t ThumbPixHeight;
    uint32_t ImagePixWidth;
    uint32_t ImagePixHeight;
    uint32_t ImageBitDepth;
    uint32_t ParentObject;
    uint16_t AssociationType;
    uint32_t AssociationDesc;
    uint32_t SequenceNumber;
    uint8_t  Filename[PTP_MAX_STR_SIZE];
    uint32_t CaptureDate;
    uint32_t ModificationDate;
    uint8_t  Keywords[PTP_MAX_STR_SIZE];
}ptp_object_info;

/* object property describing dataset (device_prop_desc) */
typedef union  _ptp_property_value
{
    char  str[PTP_MAX_STR_SIZE];
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    uint64_t u64;
    int64_t i64;
    struct array {
        uint32_t count;
        union _ptp_property_value *v;
    }a;
}ptp_property_value;

typedef struct
{
    ptp_property_value minimum_value;
    ptp_property_value maximum_value;
    ptp_property_value step_size;
}ptp_prop_desc_range_form;

/* property describing dataset, enum form */
typedef struct
{
    uint16_t            number_of_values;
    ptp_property_value  supported_value[PTP_SUPPORTED_PROPRIETIES_NBR];
}ptp_prop_desc_enum_form;

/* (MTP) object property pack/unpack */
#define PTP_OPD_OBJECT_PROPERTY_CODE                        0U
#define PTP_OPD_DATA_TYPE                                   2U
#define PTP_OPD_GET_SET                                     4U
#define PTP_OPD_FACTORY_DEFAULT_VALUE                       5U

typedef struct
{
    uint16_t            object_property_code;
    uint16_t            data_type;
    uint8_t             get_set;
    ptp_property_value  factory_default_value;
    uint32_t            group_code;
    uint8_t             form_flag;
    union {
        ptp_prop_desc_enum_form  enumf;
        ptp_prop_desc_range_form range;
    } form;
} ptp_object_prop_desc;

/* metadata lists for mtp operations */
typedef struct
{
    uint16_t            property;
    uint16_t            data_type;
    uint32_t            object_handle;
    ptp_property_value  prop_val;
}mtp_properties;


/* Device Property Form Flag */

#define PTP_DPFF_NONE                                       0x00U
#define PTP_DPFF_RANGE                                      0x01U
#define PTP_DPFF_ENUMERATION                                0x02U

/* object property codes used by MTP (first 3 are same as DPFF codes) */
#define PTP_OPFF_NONE                                       0x00U
#define PTP_OPFF_RANGE                                      0x01U
#define PTP_OPFF_ENUMERATION                                0x02U
#define PTP_OPFF_DATE_TIME                                  0x03U
#define PTP_OPFF_FIXED_LENGTH_ARRAY                         0x04U
#define PTP_OPFF_REGULAR_EXPRESSION                         0x05U
#define PTP_OPFF_BYTE_ARRAY                                 0x06U
#define PTP_OPFF_LONG_STRING                                0xFFU

/* device property pack/unpack */
#define PTP_DPD_DEVICE_PROPERTY_CODE                        0U
#define PTP_DPD_DATA_TYPE                                   2U
#define PTP_DPD_GET_SET                                     4U
#define PTP_DPD_FACTORY_DEFAULT_VALUE                       5U

/* device property describing dataset (device prop desc) */
typedef struct
{
    uint16_t            device_property_code;
    uint16_t            data_type;
    uint8_t             get_set;
    ptp_property_value  factory_default_value;
    ptp_property_value  current_value;
    uint8_t             form_flag;
    union {
        ptp_prop_desc_enum_form   enumf;
        ptp_prop_desc_range_form  range;
    } form;
}ptp_dev_prop_desc;

/* DataType Codes */

#define PTP_DTC_UNDEF                                       0x0000U
#define PTP_DTC_INT8                                        0x0001U
#define PTP_DTC_UINT8                                       0x0002U
#define PTP_DTC_INT16                                       0x0003U
#define PTP_DTC_UINT16                                      0x0004U
#define PTP_DTC_INT32                                       0x0005U
#define PTP_DTC_UINT32                                      0x0006U
#define PTP_DTC_INT64                                       0x0007U
#define PTP_DTC_UINT64                                      0x0008U
#define PTP_DTC_INT128                                      0x0009U
#define PTP_DTC_UINT128                                     0x000AU

#define PTP_DTC_ARRAY_MASK                                  0x4000U

#define PTP_DTC_AINT8                                       (PTP_DTC_ARRAY_MASK | PTP_DTC_INT8)
#define PTP_DTC_AUINT8                                      (PTP_DTC_ARRAY_MASK | PTP_DTC_UINT8)
#define PTP_DTC_AINT16                                      (PTP_DTC_ARRAY_MASK | PTP_DTC_INT16)
#define PTP_DTC_AUINT16                                     (PTP_DTC_ARRAY_MASK | PTP_DTC_UINT16)
#define PTP_DTC_AINT32                                      (PTP_DTC_ARRAY_MASK | PTP_DTC_INT32)
#define PTP_DTC_AUINT32                                     (PTP_DTC_ARRAY_MASK | PTP_DTC_UINT32)
#define PTP_DTC_AINT64                                      (PTP_DTC_ARRAY_MASK | PTP_DTC_INT64)
#define PTP_DTC_AUINT64                                     (PTP_DTC_ARRAY_MASK | PTP_DTC_UINT64)
#define PTP_DTC_AINT128                                     (PTP_DTC_ARRAY_MASK | PTP_DTC_INT128)
#define PTP_DTC_AUINT128                                    (PTP_DTC_ARRAY_MASK | PTP_DTC_UINT128)

#define PTP_DTC_STR                                         0xFFFFU

/* PTP Event Codes */

#define PTP_EC_UNDEFINED                                    0x4000U
#define PTP_EC_CANCEL_TRANSACTION                           0x4001U
#define PTP_EC_OBJECT_ADDED                                 0x4002U
#define PTP_EC_OBJECT_REMOVED                               0x4003U
#define PTP_EC_STORE_ADDED                                  0x4004U
#define PTP_EC_STORE_REMOVED                                0x4005U
#define PTP_EC_DEVICE_PROP_CHANGED                          0x4006U
#define PTP_EC_OBJECT_INFO_CHANGED                          0x4007U
#define PTP_EC_DEVICE_INFO_CHANGED                          0x4008U
#define PTP_EC_REQUEST_OBJECT_TRANSFER                      0x4009U
#define PTP_EC_STORE_FULL                                   0x400AU
#define PTP_EC_DEVICE_RESET                                 0x400BU
#define PTP_EC_STORAGE_INFO_CHANGED                         0x400CU
#define PTP_EC_CAPTURE_COMPLETE                             0x400DU
#define PTP_EC_UNREPORTED_STATUS                            0x400EU

usbh_status usbh_ptp_init (usbh_host *puhost);

usbh_status usbh_ptp_process (usbh_host *puhost);

usbh_status usbh_ptp_request_send (usbh_host *puhost, ptp_container *req);

usbh_status usbh_ptp_response_get (usbh_host *puhost, ptp_container *resp);

usbh_status usbh_ptp_session_open (usbh_host *puhost, uint32_t session);

usbh_status usbh_ptp_device_info_get (usbh_host *puhost, ptp_device_info *dev_info);

usbh_status usbh_ptp_storageID_get (usbh_host *puhost, ptp_storage_ID *storage_ids);

usbh_status usbh_ptp_storage_info_get (usbh_host *puhost, uint32_t storage_id, ptp_storage_info *storage_info);

usbh_status usbh_ptp_objects_num_get (usbh_host *puhost,
                                      uint32_t storage_id,
                                      uint32_t object_format_code,
                                      uint32_t associationOH,
                                      uint32_t* numobs);

usbh_status usbh_ptp_objects_handles_get (usbh_host *puhost,
                                          uint32_t storage_id,
                                          uint32_t object_format_code,
                                          uint32_t associationOH,
                                          ptp_object_handle* object_handles);

usbh_status usbh_ptp_object_info_get (usbh_host *puhost, uint32_t handle, ptp_object_info *object_info);

usbh_status usbh_ptp_object_delete (usbh_host *puhost, uint32_t handle, uint32_t object_format_code);

usbh_status usbh_ptp_object_get (usbh_host *puhost, uint32_t handle, uint8_t *object);

usbh_status usbh_ptp_partial_object_get(usbh_host *puhost,
                                        uint32_t handle,
                                        uint32_t offset,
                                        uint32_t maxbytes, uint8_t *object,
                                        uint32_t *len);

usbh_status usbh_ptp_object_props_supported_get (usbh_host *puhost,
                                                 uint16_t ofc,
                                                 uint32_t *propnum,
                                                 uint16_t *props);

usbh_status usbh_ptp_object_prop_desc_get (usbh_host *puhost,
                                           uint16_t opc,
                                           uint16_t ofc,
                                           ptp_object_prop_desc *opd);

usbh_status usbh_ptp_object_prop_list_get (usbh_host *puhost,
                                           uint32_t handle,
                                           mtp_properties *pprops,
                                           uint32_t *nrofprops);

usbh_status usbh_ptp_object_send (usbh_host *puhost,
                                  uint32_t handle,
                                  uint8_t *object,
                                  uint32_t size);

usbh_status usbh_ptp_device_prop_desc_get (usbh_host *puhost, uint16_t propcode, ptp_dev_prop_desc *device_property_desc);



#ifdef __cplusplus
}
#endif

#endif  /* __USBH_MTP_PTP_H */

