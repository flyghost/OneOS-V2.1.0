/*!
    \file  usbh_video.c
    \brief this file is the UVC layer handlers for USB host UVC class
    \note  This driver manages the video Class 1.1 following the "USB Device 
           Class Definition for video Devices V1.0 Mar 18, 98"

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

#include "usbh_video.h"
#include "usbh_transc.h"
#include "usbh_pipe.h"
#include "usbh_enum.h"

#include "usbh_usr.h"

//#include "lcd_log.h"
#include <string.h>

/* Target UVC mode */
usbh_video_target_format target_format = UVC_DEFAULT_CAPTURE_MODE;

/* value of "bFormatIndex" for target settings (set in USBH_VIDEO_AnalyseFormatDescriptors) */
int usbh_video_best_bformat_index = -1;

/* value of "bFrameIndex" for target settings (set in USBH_VIDEO_AnalyseFrameDescriptors) */
int usbh_video_best_bframe_index = -1;

/* width in pixels */
int usbh_video_target_width = UVC_DEFAULT_TARGET_WIDTH;

/* height in pixels */
int usbh_video_target_height = UVC_DEFAULT_TARGET_HEIGHT;

/* buffer to store received UVC data packet */
volatile uint8_t tmp_packet_framebuffer[UVC_RX_FIFO_SIZE_LIMIT];

#define UVC_HEADER_SIZE_POS             0
#define UVC_HEADER_BIT_FIELD_POS        1

#define UVC_HEADER_FID_BIT              (1 << 0)
#define UVC_HEADER_EOF_BIT              (1 << 1)

#define UVC_HEADER_SIZE                 12

uint32_t uvc_total_packet_cnt = 0;
uint32_t uvc_data_packet_cnt = 0;
uint32_t uvc_frame_cnt = 0;
uint32_t uvc_header_cnt = 0;

uint8_t uvc_prev_fid_state = 0;

uint32_t uvc_curr_frame_length = 0;

/* This value should be used by external software */
uint32_t uvc_ready_frame_length = 0;

uint8_t uvc_parsing_initialized = 0;

uint8_t uvc_parsing_new_frame_ready = 0;

/* flag - show that "Ready" framebuffer is not used by exteranal software now */
uint8_t uvc_parsing_switch_ready = 1;
uint8_t uvc_parsing_enabled = 1;
uint8_t uvc_frame_start_detected = 0;

/* Previous packet was EOF */
uint8_t uvc_prev_packet_eof = 1;

/* Pointers to a framebuffers to store captured frame */
uint8_t* uvc_framebuffer0_ptr = NULL;
uint8_t* uvc_framebuffer1_ptr = NULL;

/* Pointer to a buffer that is FILLING now */
uint8_t* uvc_curr_framebuffer_ptr = NULL;

/* Pointer to a buffer that is FILLED now */
uint8_t* uvc_ready_framebuffer_ptr = NULL;

const uint8_t camera_terminal_feature_len[] = {
    LEN_SCANNING_MODE,
    LEN_AUTO_EXPOSURE_MODE,
    LEN_AUTO_EXPOSURE_PRIO,
    LEN_EXPOSURE_TIME_ABSO,
    LEN_EXPOSURE_TIME_RELAT,
    LEN_FOCUS_ABSO,
    LEN_FOCUS_RELAT,
    LEN_IRIS_ABSO,
    LEN_IRIS_RELAT,
    LEN_ZOOM_ABSO,
    LEN_ZOOM_RELAT,
    LEN_PAN_TILT_ABSO,
    LEN_PAN_TILT_RELAT,
    LEN_ROLL_ABSO,
    LEN_ROLL_RELAT,
    0U,
    0U,
    LEN_FOCUS_AUTO,
    LEN_PRIVACY_SHUTTER
};

const uint8_t processing_unit_feature_len[] = {
    LEN_BRIGHTNESS,
    LEN_CONTRAST,
    LEN_HUE,
    LEN_SATURATION,
    LEN_SHARPNESS,
    LEN_GAMMA,
    LEN_WHITE_BALANCE_TEMPERATURE,
    LEN_WHITE_BALANCE_COMPONENT,
    LEN_BACKLIGHT_COMPENSATION,
    LEN_GAIN,
    LEN_POWER_LINE_FREQUENCY,
    LEN_HUE_AUTO,
    LEN_WHITE_BALANCE_TEMPERATURE_AUTO,
    LEN_WHITE_BALANCE_COMPONENT_AUTO,
    LEN_DIGITAL_MULTIPLIER,
    LEN_DIGITAL_MULTIPLIER_LIMIT,
    LEN_ANALOG_VIDEO_STANDARD,
    LEN_ANALOG_VIDEO_LOCK_STATUS
};

const uint8_t camera_terminal_control_selector[] = {
    CT_SCANNING_MODE_CONTROL,
    CT_AE_MODE_CONTROL,
    CT_AE_PRIORITY_CONTROL,
    CT_EXPOSURE_TIME_ABSOLUTE_CONTROL,
    CT_EXPOSURE_TIME_RELATIVE_CONTROL,
    CT_FOCUS_ABSOLUTE_CONTROL,
    CT_FOCUS_RELATIVE_CONTROL,
    CT_IRIS_ABSOLUTE_CONTROL,
    CT_IRIS_RELATIVE_CONTROL,
    CT_ZOOM_ABSOLUTE_CONTROL,
    CT_ZOOM_RELATIVE_CONTROL,
    CT_PANTILT_ABSOLUTE_CONTROL,
    CT_PANTILT_RELATIVE_CONTROL,
    CT_ROLL_ABSOLUTE_CONTROL,
    CT_ROLL_RELATIVE_CONTROL,
    0U,
    0U,
    CT_FOCUS_AUTO_CONTROL,
    CT_PRIVACY_CONTROL,
    CT_CONTROL_UNDEFINED,
};

const uint8_t processing_unit_control_selector[] = {
    PU_BRIGHTNESS_CONTROL,
    PU_CONTRAST_CONTROL,
    PU_HUE_CONTROL,
    PU_SATURATION_CONTROL,
    PU_SHARPNESS_CONTROL,
    PU_GAMMA_CONTROL,
    PU_WHITE_BALANCE_TEMPERATURE_CONTROL,
    PU_WHITE_BALANCE_COMPONENT_CONTROL,
    PU_BACKLIGHT_COMPENSATION_CONTROL,
    PU_GAIN_CONTROL,
    PU_POWER_LINE_FREQUENCY_CONTROL,
    PU_HUE_AUTO_CONTROL,
    PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL,
    PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL,
    PU_DIGITAL_MULTIPLIER_CONTROL,
    PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL,
    PU_ANALOG_VIDEO_STANDARD_CONTROL,
    PU_ANALOG_LOCK_STATUS_CONTROL,
    PU_CONTROL_UNDEFINED,
};



//const uint8_t processing_unit_get_info[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

//const uint8_t processing_unit_get_min[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0};

//const uint8_t processing_unit_get_max[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0};

//const uint8_t processing_unit_get_res[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0};

//const uint8_t processing_unit_get_def[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    
const uint8_t processing_unit_request_state[5][18] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
};

//const uint8_t processing_unit_get_cur[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

//const uint8_t processing_unit_set_cur[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};


static usbh_status usbh_video_interface_init        (usbh_host *puhost);
static void usbh_video_interface_deinit             (usbh_host *puhost);
static usbh_status usbh_video_process_dummy         (usbh_host *puhost);
static usbh_status usbh_video_sof_process           (usbh_host *puhost);
static usbh_status usbh_video_class_request         (usbh_host *puhost);
static usbh_status usbh_video_cs_request            (usbh_host *puhost, uint8_t feature, uint8_t channel);

static usbh_status vc_handle_input_terminal_request  (usbh_host *puhost, 
                                                      uint8_t control_selector, 
                                                      uint8_t index, 
                                                      uint16_t len);

static usbh_status vc_handle_processing_unit_request (usbh_host *puhost, 
                                                      uint8_t control_selector, 
                                                      uint8_t index, 
                                                      uint16_t len);

static usbh_status vc_handle_extension_unit_request  (usbh_host *puhost, 
                                                      uint8_t control_selector, 
                                                      uint8_t index, 
                                                      uint16_t len);

static usbh_status usbh_video_handle_cs_vc_itt_request  (usbh_host *puhost);
static usbh_status usbh_video_handle_cs_vc_pru_request  (usbh_host *puhost);
static usbh_status usbh_video_handle_cs_vc_exu_request  (usbh_host *puhost);

static usbh_status usbh_video_handle_cs_vs_request  (usbh_host *puhost);

static usbh_status usbh_video_input_stream          (usbh_host *puhost);

static usbh_status usbh_video_find_control          (usbh_host *puhost);

static usbh_status usbh_video_find_streaming_in     (usbh_host *puhost);
static usbh_status usbh_video_parse_cs_descriptors  (usbh_host *puhost);

static usbh_status ParseCSDescriptors               (video_desc_class_specific *class_desc, uint8_t vs_subclass, uint8_t *pdesc);

static void usbh_video_format_descriptors_analyse   (video_desc_class_specific *class_desc);

static void usbh_video_frame_descriptors_analyse    (video_desc_class_specific *class_desc);

static void video_stream_process_packet(uint16_t size);

static uint8_t video_stream_switch_buffers(void);

static void video_stream_add_packet_data(uint8_t* buf, uint16_t data_size);

static usbh_status usbh_vc_len_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index, 
                                    uint16_t len);

static usbh_status usbh_vc_info_get (usbh_host *puhost, 
                                     uint8_t sub_type, 
                                     uint8_t control_selector,
                                     uint8_t index, 
                                     uint16_t len);

static usbh_status usbh_vc_max_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len);

static usbh_status usbh_vc_min_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len);

static usbh_status usbh_vc_def_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t len);

static usbh_status usbh_vc_res_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t len);

static usbh_status usbh_vs_cur_set (usbh_host *puhost, uint16_t req_type);
static usbh_status usbh_vs_cur_get (usbh_host *puhost, uint16_t req_type);
static usbh_status usbh_vs_max_get (usbh_host *puhost, uint16_t req_type);
static usbh_status usbh_vs_min_get (usbh_host *puhost, uint16_t req_type);

usbh_class usbh_video = 
{
    CC_VIDEO,
    usbh_video_interface_init,
    usbh_video_interface_deinit,
    usbh_video_class_request,
    usbh_video_process_dummy,
    usbh_video_sof_process
};

/*!
    \brief      the function init the video stream buffers
    \param[in]  buffer0: pointer to buffer 0
    \param[in]  buffer1: pointer to buffer 1
    \param[out] none
    \retval     USB host status
*/
void video_stream_init_buffers(uint8_t* buffer0, uint8_t* buffer1)
{
    if ((buffer0 == NULL) || (buffer1 == NULL)) {
        return;
    }

    uvc_framebuffer0_ptr = buffer0;
    uvc_framebuffer1_ptr = buffer1;
    uvc_curr_framebuffer_ptr = uvc_framebuffer0_ptr;
    uvc_ready_framebuffer_ptr = uvc_framebuffer1_ptr;
    uvc_parsing_initialized = 1;
    uvc_parsing_enabled = 1;
    uvc_parsing_switch_ready = 1;
}

/*!
    \brief      update stream ready flag
    \param[in]  none
    \param[out] none
    \retval     USB host status
*/
void video_stream_ready_update(void)
{
    /* "Ready" framebuffer is not used by exteranal software now */
    uvc_parsing_switch_ready = 1;
}

/*!
    \brief      the function init the video class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_interface_init (usbh_host *puhost)
{
    uint8_t  interface, index;
    uint16_t ep_size_in = 0;

    usbh_status status = USBH_FAIL;
    usbh_status in_status;

    interface = usbh_interface_find (&puhost->dev_prop, CC_VIDEO, USB_SUBCLASS_VIDEOCONTROL, USB_PROTOCOL_UNDEFINED);

    if(interface == 0xFF) /* Not Valid Interface */{
        status = USBH_FAIL;
    } else {
        static usbh_video_handle video_handler;

        memset(&video_handler, 0U, sizeof(usbh_video_handle));

        puhost->active_class->class_data = (void *)&video_handler;

        /* 1st Step:  Find Audio Interfaces */
        in_status = usbh_video_find_streaming_in (puhost);

        if (in_status == USBH_FAIL) {
            LCD_DbgLog("can not find IN Stream.\n");

            status = USBH_FAIL;
        } else {
            /* 2nd Step: select audio streaming interfaces with largest endpoint size: default behavior */
            for (index = 0; index < VIDEO_MAX_VIDEO_STD_INTERFACE; index ++) {
                if (video_handler.stream_in[index].valid == 1) {
                    uint16_t ep_size = video_handler.stream_in[index].EpSize;

                    if ((ep_size > ep_size_in) && (ep_size <= UVC_RX_FIFO_SIZE_LIMIT)) {
                        video_handler.camera.interface = video_handler.stream_in[index].interface;
                        video_handler.camera.AltSettings = video_handler.stream_in[index].AltSettings;
                        video_handler.camera.Ep = video_handler.stream_in[index].Ep;
                        video_handler.camera.EpSize = video_handler.stream_in[index].EpSize; 
                        video_handler.camera.Poll = video_handler.stream_in[index].Poll;
                        video_handler.camera.supported = 1;

                        ep_size_in = ep_size;
                    }
                }
            }

            /* 3rd Step: Find and Parse video interfaces */ 
            usbh_video_parse_cs_descriptors (puhost);

            usbh_video_find_control(puhost);

            /* 4th Step: Find desrcroptors for target settings */
            usbh_video_format_descriptors_analyse(&video_handler.class_desc);
            if (usbh_video_best_bformat_index == -1) {
                LCD_DbgLog("video format error.\n");
                status = USBH_FAIL;
            }

            usbh_video_frame_descriptors_analyse(&video_handler.class_desc);
            if (usbh_video_best_bframe_index == -1) {
                LCD_DbgLog("video frame error.\n");
                status = USBH_FAIL;
            }

            if(video_handler.camera.supported == 1) {
                video_handler.camera.Pipe = usbh_pipe_allocate (puhost->data, video_handler.camera.Ep);

                /* Open pipe for OUT endpoint */
                usbh_pipe_create (puhost->data,
                                  &puhost->dev_prop,
                                  video_handler.camera.Pipe,
                                  USB_EPTYPE_ISOC,
                                  video_handler.camera.EpSize); 

                usbh_pipe_toggle_set (puhost->data, video_handler.camera.Pipe, 0);
            }

            video_handler.req_state = VIDEO_REQ_INIT;
            video_handler.control_state = VIDEO_CONTROL_INIT;

            status = USBH_OK;
        }
    }

    return status;
}

/*!
    \brief      the function deinit the pipes used for the audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static void usbh_video_interface_deinit (usbh_host *puhost)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (video->camera.Pipe != 0x00) {
        usb_pipe_halt (puhost->data, video->camera.Pipe);
        usbh_pipe_free (puhost->data, video->camera.Pipe);
        video->camera.Pipe = 0;     /* Reset the pipe as Free */
    }
}

/*!
    \brief      the function is responsible for handling standard requests for audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_class_request(usbh_host *puhost)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    usbh_host *pphost = puhost;

    /* Switch AUDIO REQ state machine */
    switch (video->req_state) {
        case VIDEO_REQ_INIT:
        case VIDEO_REQ_SET_DEFAULT_IN_INTERFACE:
            if (video->camera.supported == 1) {
                req_status = usbh_setinterface(puhost, video->camera.interface, 0);

                if(req_status == USBH_OK) {
                    if (video->class_desc.input_terminal_num > 0) {
                        video->req_state = VIDEO_REQ_CS_VC_ITT_REQUESTS;
                        video->cs_itt_req_state = VIDEO_ITT_IDLE;
                    } else if (video->class_desc.processing_unit_num > 0) {
                        video->req_state = VIDEO_REQ_CS_VC_PRU_REQUESTS;
                        video->cs_pru_req_state = VIDEO_PRU_IDLE;
                    } else if (video->class_desc.extension_unit_num > 0) {
                        video->req_state = VIDEO_REQ_CS_VC_EXU_REQUESTS;
                        video->cs_exu_req_state = VIDEO_EXU_IDLE;
                    }

                    video->enum_index = 0U;
                    video->enum_feature = 0U;
//                    video->req_state = VIDEO_REQ_SET_IN_INTERFACE;
                }
            } else {
                video->req_state = VIDEO_REQ_SET_DEFAULT_IN_INTERFACE;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;

        case VIDEO_REQ_CS_VC_ITT_REQUESTS:
            if (USBH_OK == usbh_video_handle_cs_vc_itt_request(puhost)) {
                video->req_state = VIDEO_REQ_CS_VC_PRU_REQUESTS;
                video->cs_pru_req_state = VIDEO_PRU_IDLE;
            }
            break;

        case VIDEO_REQ_CS_VC_PRU_REQUESTS:
            if (USBH_OK == usbh_video_handle_cs_vc_pru_request(puhost)) {
                video->req_state = VIDEO_REQ_CS_VC_EXU_REQUESTS;
                video->cs_exu_req_state = VIDEO_EXU_IDLE;
            }
            break;

        case VIDEO_REQ_CS_VC_EXU_REQUESTS:
            if (USBH_OK == usbh_video_handle_cs_vc_exu_request(puhost)) {
                video->req_state = VIDEO_REQ_CS_VS_REQUESTS;
                video->cs_req_state = VIDEO_REQ_GET_CUR;
            }
            break;

        case VIDEO_REQ_CS_VS_REQUESTS:
            if (usbh_video_handle_cs_vs_request(puhost) == USBH_OK) {
                video->req_state = VIDEO_REQ_SET_IN_INTERFACE;
            }
            break;

        case VIDEO_REQ_SET_IN_INTERFACE:
            if (video->camera.supported == 1) {
                req_status = usbh_setinterface(pphost, video->camera.interface, video->camera.AltSettings);

                if (req_status == USBH_OK) {
                    video->req_state = VIDEO_REQ_IDLE;
                    video->steam_in_state = VIDEO_STATE_START_IN;
                }
            } else {
                video->req_state = VIDEO_REQ_SET_IN_INTERFACE;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;


        case VIDEO_REQ_IDLE:
            status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_CLASS_EVENT, 0);
#endif

        default:
            break;
    }

    return status; 
}

/*!
    \brief      the function is responsible for handling VS Specific requests for a specific feature and channel for video class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  feature: CS feature
    \param[in]  channel: audio channel
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_cs_request(usbh_host *puhost, uint8_t feature, uint8_t channel)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    /* Switch VIDEO REQ state machine */
    switch (video->cs_req_state) {
        case VIDEO_REQ_GET_CUR:
            req_status = usbh_vs_cur_get(puhost, 0x0100);

            if (USBH_OK == req_status) {
                video->cs_req_state = VIDEO_REQ_GET_MAX;
            } 
            break;

        case VIDEO_REQ_GET_MAX:
            req_status = usbh_vs_max_get(puhost, 0x0100);

            if (USBH_OK == req_status) {
                video->cs_req_state = VIDEO_REQ_GET_MIN;
            }
            break;

        case VIDEO_REQ_GET_MIN:
            req_status = usbh_vs_min_get(puhost, 0x0100);

            if (USBH_OK == req_status) {
                video->cs_req_state = VIDEO_REQ_SET_CUR;
            }
            break;

        case VIDEO_REQ_SET_CUR:
            req_status = usbh_vs_cur_set(puhost, 0x0100);

            if (USBH_OK == req_status) {
                video->cs_req_state = VIDEO_REQ_SET_CUR2;
            }
            break;

        case VIDEO_REQ_SET_CUR2:
            req_status = usbh_vs_cur_set(puhost, 0x0200);

            if (USBH_OK == req_status) {
                video->cs_req_state = VIDEO_REQ_CS_IDLE;
            }
            break;

        case VIDEO_REQ_CS_IDLE:
            status = USBH_OK;

        default:
            break;
    }

    return status; 
}

static usbh_status vc_handle_input_terminal_request  (usbh_host *puhost, 
                                                      uint8_t control_selector, 
                                                      uint8_t index, 
                                                      uint16_t len)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    switch (video->cs_itt_req_state) {
        case VIDEO_ITT_IDLE:
            video->cs_itt_req_state = VIDEO_ITT_GET_INFO;
            break;

        case VIDEO_ITT_GET_INFO:
            req_status = usbh_vc_info_get (puhost,
                                           UVC_VC_INPUT_TERMINAL,
                                           control_selector,
                                           index,
                                           len);

            if (USBH_OK == req_status) {
                video->cs_itt_req_state = VIDEO_ITT_GET_MIN;
            }
            break;

        case VIDEO_ITT_GET_MIN:
            req_status = usbh_vc_min_get (puhost,
                                          UVC_VC_INPUT_TERMINAL,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_itt_req_state = VIDEO_ITT_GET_MAX;
            }
            break;

        case VIDEO_ITT_GET_MAX:
            req_status = usbh_vc_max_get (puhost,
                                          UVC_VC_INPUT_TERMINAL,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_itt_req_state = VIDEO_ITT_GET_RES;
            }
            break;

        case VIDEO_ITT_GET_RES:
            req_status = usbh_vc_res_get (puhost,
                                          UVC_VC_INPUT_TERMINAL,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_itt_req_state = VIDEO_ITT_GET_DEF;
            }
            break;

        case VIDEO_ITT_GET_DEF:
            req_status = usbh_vc_def_get (puhost,
                                          UVC_VC_INPUT_TERMINAL,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_itt_req_state = VIDEO_ITT_END;
            }
            break;

        case VIDEO_ITT_END:
            video->cs_itt_req_state = VIDEO_ITT_IDLE;
            status = USBH_OK;
            break;

        default:
            break;
    }

    return status;
}


static int vc_pru_next_state(usbh_host *puhost, int cur_state)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    int i = cur_state + 1;

    while (processing_unit_request_state[i][video->enum_feature] == 0) {
        i++;
        if (i >= VIDEO_PRU_END) {
            break;
        }
    }

    return i;
}

static usbh_status vc_handle_processing_unit_request  (usbh_host *puhost, 
                                                       uint8_t control_selector, 
                                                       uint8_t index, 
                                                       uint16_t len)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    switch (video->cs_pru_req_state) {
        case VIDEO_PRU_IDLE:
            video->cs_pru_req_state = VIDEO_PRU_GET_INFO;
            break;

        case VIDEO_PRU_GET_INFO:
            req_status = usbh_vc_info_get (puhost,
                                           UVC_VC_PROCESSING_UNIT,
                                           control_selector,
                                           index,
                                           len);

            if (USBH_OK == req_status) {
                video->cs_pru_req_state = vc_pru_next_state(puhost, VIDEO_PRU_GET_INFO);
            }
            break;

        case VIDEO_PRU_GET_MIN:
            req_status = usbh_vc_min_get (puhost,
                                          UVC_VC_PROCESSING_UNIT,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_pru_req_state = vc_pru_next_state(puhost, VIDEO_PRU_GET_MIN);
            }
            break;

        case VIDEO_PRU_GET_MAX:
            req_status = usbh_vc_max_get (puhost,
                                          UVC_VC_PROCESSING_UNIT,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_pru_req_state = vc_pru_next_state(puhost, VIDEO_PRU_GET_MAX);
            }
            break;

        case VIDEO_PRU_GET_RES:
            req_status = usbh_vc_res_get (puhost,
                                          UVC_VC_PROCESSING_UNIT,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_pru_req_state = vc_pru_next_state(puhost, VIDEO_PRU_GET_RES);
            }
            break;

        case VIDEO_PRU_GET_DEF:
            req_status = usbh_vc_def_get (puhost,
                                          UVC_VC_PROCESSING_UNIT,
                                          control_selector,
                                          index,
                                          len);

            if (USBH_OK == req_status) {
                video->cs_pru_req_state = VIDEO_PRU_END;
            }
            break;

        case VIDEO_PRU_END:
            video->cs_pru_req_state = VIDEO_PRU_IDLE;
            status = USBH_OK;
            break;

        default:
            break;
    }

    return status;
}

static usbh_status vc_handle_extension_unit_request  (usbh_host *puhost, 
                                                      uint8_t control_selector, 
                                                      uint8_t index, 
                                                      uint16_t len)
{
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    switch (video->cs_exu_req_state) {
        case VIDEO_EXU_IDLE:
            video->cs_exu_req_state = VIDEO_EXU_GET_LEN;
            break;

        case VIDEO_EXU_GET_LEN:
            req_status = usbh_vc_len_get (puhost,
                                          UVC_VC_EXTENSION_UNIT,
                                          control_selector,
                                          index,
                                          2);

            if (USBH_OK == req_status) {
                video->extern_info_len = video->mem[0];

                video->cs_exu_req_state = VIDEO_EXU_GET_INFO;
            }
            break;

        case VIDEO_EXU_GET_INFO:
            req_status = usbh_vc_info_get (puhost,
                                           UVC_VC_EXTENSION_UNIT,
                                           control_selector,
                                           index,
                                           video->extern_info_len);

            if (USBH_OK == req_status) {
                video->cs_exu_req_state = VIDEO_EXU_GET_MIN;
            }
            break;

        case VIDEO_EXU_GET_MIN:
            req_status = usbh_vc_min_get (puhost,
                                          UVC_VC_EXTENSION_UNIT,
                                          control_selector,
                                          index,
                                          video->extern_info_len);

            if (USBH_OK == req_status) {
                video->cs_exu_req_state = VIDEO_EXU_GET_MAX;
            }
            break;

        case VIDEO_EXU_GET_MAX:
            req_status = usbh_vc_max_get (puhost,
                                          UVC_VC_EXTENSION_UNIT,
                                          control_selector,
                                          index,
                                          video->extern_info_len);

            if (USBH_OK == req_status) {
                video->cs_exu_req_state = VIDEO_EXU_GET_RES;
            }
            break;

        case VIDEO_EXU_GET_RES:
            req_status = usbh_vc_res_get (puhost,
                                          UVC_VC_EXTENSION_UNIT,
                                          control_selector,
                                          index,
                                          video->extern_info_len);

            if (USBH_OK == req_status) {
                video->cs_exu_req_state = VIDEO_EXU_GET_DEF;
            }
            break;

        case VIDEO_EXU_GET_DEF:
            req_status = usbh_vc_def_get (puhost,
                                          UVC_VC_EXTENSION_UNIT,
                                          control_selector,
                                          index,
                                          video->extern_info_len);

            if (USBH_OK == req_status) {
                video->cs_exu_req_state = VIDEO_EXU_END;
            }
            break;

        case VIDEO_EXU_END:
            video->cs_exu_req_state = VIDEO_EXU_IDLE;
            status = USBH_OK;
            break;

        default:
            break;
    }

    return status;
}

static usbh_status usbh_video_handle_cs_vc_itt_request(usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_status cs_vc_status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (video->enum_index < video->class_desc.input_terminal_num) {
        if ((video->cs_itt_req_state == VIDEO_ITT_IDLE) && (0U == video->enum_feature)){
            video_desc_it *itt_desc = video->class_desc.cs_desc.input_terminal_desc[video->enum_index];

            if ((itt_desc->wTerminalType[0] | (uint16_t)(itt_desc->wTerminalType[1] << 8)) != ITT_CAMERA) {
                LCD_DbgLog("it is not camera.\n");

                return USBH_FAIL;
            }

            video->enum_control = ((uint32_t)itt_desc->bmControls[2] << 16) | \
                                         ((uint32_t)itt_desc->bmControls[1] << 8) | \
                                         ((uint32_t)itt_desc->bmControls[0]);
        }
 
        if (video->enum_feature < VC_FEATURE_MAX) {
            if (video->enum_control & (1 << video->enum_feature)) {
                cs_vc_status = vc_handle_input_terminal_request (puhost, 
                                                                 camera_terminal_control_selector[video->enum_feature],
                                                                 video->enum_index,
                                                                 camera_terminal_feature_len[video->enum_feature]);

                if (USBH_OK == cs_vc_status) {
                    video->enum_feature++;
                }
            } else {
                video->enum_feature++;
            }
        } else {
            video->enum_feature = 0U;
            video->enum_index++;
        }
    } else {
        video->enum_control = 0U;
        video->enum_index = 0U;
        status = USBH_OK;
    }

    return status;
}

static usbh_status usbh_video_handle_cs_vc_pru_request (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_status cs_vc_status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (video->enum_index < video->class_desc.processing_unit_num) {
        if ((video->cs_pru_req_state == VIDEO_PRU_IDLE) && (0U == video->enum_feature)) {
            video_desc_processing *pru_desc = video->class_desc.cs_desc.processing_unit_desc[video->enum_index];

            video->enum_control = ((uint32_t)pru_desc->bmControls[2] << 16) | \
                                         ((uint32_t)pru_desc->bmControls[1] << 8) | \
                                         ((uint32_t)pru_desc->bmControls[0]);
        }

        if (video->enum_feature < VC_FEATURE_MAX) {
            if (video->enum_control & (1 << video->enum_feature)) {
                cs_vc_status = vc_handle_processing_unit_request (puhost, 
                                                                  processing_unit_control_selector[video->enum_feature],
                                                                  video->enum_index,
                                                                  processing_unit_feature_len[video->enum_feature]);

                if (USBH_OK == cs_vc_status) {
                    video->enum_feature++;
                }
            } else {
                video->enum_feature++;
            }
        } else {
            video->enum_feature = 0U;
            video->enum_index++;
        }
    } else {
        video->enum_control = 0U;
        video->enum_index = 0U;
        status = USBH_OK;
    }

    return status;
}

static usbh_status usbh_video_handle_cs_vc_exu_request (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_status cs_vc_status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (video->enum_index < video->class_desc.extension_unit_num) {
        if (video->cs_exu_req_state == VIDEO_EXU_IDLE) {
            video_desc_extension *exu_desc = video->class_desc.cs_desc.extension_unit_desc[video->enum_index];

            video->enum_control = ((uint32_t)exu_desc->bmControls[2] << 16) | \
                                    ((uint32_t)exu_desc->bmControls[1] << 8) | \
                                      ((uint32_t)exu_desc->bmControls[0]);
        }

        if (video->enum_feature < VC_FEATURE_MAX) {
            if (video->enum_control & (1 << video->enum_feature)) {
                cs_vc_status = vc_handle_extension_unit_request (puhost, 
                                                                 video->enum_feature + 1,
                                                                 video->enum_index,
                                                                 0U);

                if (USBH_OK == cs_vc_status) {
                    video->enum_feature++;
                }
            } else {
                video->enum_feature++;
            }
        } else {
            video->enum_feature = 0U;
            video->enum_index++;
        }
    } else {
        video->enum_control = 0U;
        video->enum_index = 0U;
        status = USBH_OK;
    }

    return status;
}

/*!
    \brief      the function is responsible for handling AC Specific requests for a all features and associated channels for Audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_handle_cs_vs_request(usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_status cs_status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    cs_status = usbh_video_cs_request(puhost, video->enum_feature, video->enum_index);

//    if(cs_status != USBH_BUSY) {
//        if (video->temp_channels == 1) {
//            video->temp_feature = 0;
//            video->temp_channels = 0;
//            status = USBH_OK; 
//        } else {
//            video->temp_channels--;
//        }

//#if (USBH_USE_OS == 1)
//      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
//#endif
//    }
    if (cs_status != USBH_BUSY) {
        status = USBH_OK;
    }

    return status;
}

static usbh_status usbh_video_process_dummy (usbh_host *puhost)
{
    usbh_video_process (puhost);

    return USBH_OK;
}

/*!
    \brief      the function is for managing state machine for Audio data transfers
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_video_process (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (video->camera.supported == 1) {
        usbh_video_input_stream (puhost);
    }

    return status;
}

static usbh_status usbh_video_input_stream (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (video->steam_in_state) {
        case VIDEO_STATE_START_IN:
            usbh_data_recev(puhost->data,
                            (uint8_t*)tmp_packet_framebuffer,
                            video->camera.Pipe,
                            video->camera.EpSize);

            video->camera.timer = puhost->control.timer;
            video->steam_in_state = VIDEO_STATE_DATA_IN;
            break;

        case VIDEO_STATE_DATA_IN:
            if ((usbh_urbstate_get(puhost->data, video->camera.Pipe) == URB_DONE) && 
                ((puhost->control.timer - video->camera.timer) >= video->camera.Poll)) {
                video->camera.timer = puhost->control.timer;
                volatile uint32_t rxlen = usbh_xfercount_get(puhost->data, video->camera.Pipe);//Return the last transfered packet size.
//                video_stream_process_packet((uint16_t)rxlen);

                usbh_data_recev(puhost->data,
                                (uint8_t*)tmp_packet_framebuffer,
                                video->camera.Pipe,
                                video->camera.EpSize);
            }
            break;

        default:
            break;
    }

    return status;  
}


/*!
    \brief      the function is for managing the SOF callback
    \param[in]  pudev: USB core driver
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_sof_process (usbh_host *puhost)
{
    return USBH_OK;
}

static usbh_status usbh_video_find_control (usbh_host *puhost)
{
    uint8_t interface;
    usbh_status status = USBH_FAIL;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    /* look for video control interface */
    interface = usbh_interface_find(&puhost->dev_prop, CC_VIDEO, USB_SUBCLASS_VIDEOCONTROL, 0x00);

    if (interface != 0xFF) {
        for (interface = 0; interface < USBH_MAX_INTERFACES_NUM; interface++) {
            usb_desc_itf *itf_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].itf_desc;
            usb_desc_ep *ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[0];

            if (ep_desc->bmAttributes == USB_EP_ATTR_INT) {
                if ((ep_desc->wMaxPacketSize > 0) && (ep_desc->bEndpointAddress & 0x80) == 0x80) {
                    video->control.Ep = ep_desc->bEndpointAddress;
                    video->control.EpSize = ep_desc->wMaxPacketSize;
                    video->control.interface = itf_desc->bInterfaceNumber;
                    video->control.Poll = ep_desc->bInterval;
                    video->control.supported = 1;

                    if (video->class_desc.cs_desc.cs_endpoint_desc->bDescriptorSubtype == UVC_EP_INTERRUPT) {
                        status = USBH_OK;

                        break;
                    }
                }
            }
        }
    }

    return status;
}

/*!
    \brief      find IN Video Streaming interfaces
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_find_streaming_in(usbh_host *puhost)
{
    uint8_t interface, alt_settings = 0;
    usbh_status status = USBH_FAIL;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    usb_desc_config *cfg_desc = &puhost->dev_prop.cfg_desc_set.cfg_desc;

    /* look for AUDIOSTREAMING IN interface */
    for (interface = 0; interface < cfg_desc->bNumInterfaces; interface++, alt_settings = 0) {
        do {
            usb_desc_itf_set *itf_desc_set = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][alt_settings];
            usb_desc_itf *itf_desc = &itf_desc_set->itf_desc;

            if (itf_desc->header.bLength > 0) {
                if ((itf_desc->bInterfaceClass == CC_VIDEO) && (itf_desc->bInterfaceSubClass == USB_SUBCLASS_VIDEOSTREAMING)) {
                    if (itf_desc->bNumEndpoints > 0) {
                        usb_desc_ep *ep_desc = &itf_desc_set->ep_desc[0];

                        if ((ep_desc->bEndpointAddress & 0x80) && (ep_desc->wMaxPacketSize > 0)) {
                            video->stream_in[alt_settings].valid = 1;
                            video->stream_in[alt_settings].interface = itf_desc->bInterfaceNumber;
                            video->stream_in[alt_settings].AltSettings = itf_desc->bAlternateSetting;
                            video->stream_in[alt_settings].Ep = ep_desc->bEndpointAddress;
                            video->stream_in[alt_settings].EpSize = ep_desc->wMaxPacketSize & 0x07FFU;
                            video->stream_in[alt_settings].Poll = ep_desc->bInterval;

                            if (status != USBH_OK) {
                                status = USBH_OK;
                            }
                        }
                    }

                    alt_settings++;
                } else {
                    break;
                }
            } else {
                break;
            }
        } while (alt_settings < USBH_MAX_ALT_SETTING);
    }

    return status;
}

/*!
    \brief      parse AC and interfaces descriptors
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_video_parse_cs_descriptors(usbh_host *puhost)
{
    usb_desc_header *pdesc;
    uint16_t ptr;
    int8_t itf_index = 0;
    int8_t itf_number = 0;
    int8_t alt_setting;
    int8_t sub_desc_type = 0;

    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;
    pdesc = (usb_desc_header *)&(puhost->dev_prop.cfgdesc_rawdata);
    ptr = USB_CFG_DESC_LEN;

    video->class_desc.input_terminal_num = 0;
    video->class_desc.output_terminal_num = 0;
    video->class_desc.camera_terminal_num = 0;
    video->class_desc.selector_unit_num = 0;
    video->class_desc.processing_unit_num = 0;
    video->class_desc.extension_unit_num = 0;

    while (ptr < puhost->dev_prop .cfg_desc_set.cfg_desc.wTotalLength) {
        pdesc = usbh_nextdesc_get((uint8_t*) pdesc, &ptr);

        switch (pdesc->bDescriptorType) {
            case USB_DESCTYPE_ITF:
                itf_number = *((uint8_t *)pdesc + 2);
                alt_setting = *((uint8_t *)pdesc + 3);
                itf_index = usbh_interfaceindex_find (&puhost->dev_prop, itf_number, alt_setting);
                break;

            case USB_DESC_TYPE_CS_INTERFACE:
                if (itf_number <= puhost->dev_prop.cfg_desc_set.cfg_desc.bNumInterfaces) {
                    ParseCSDescriptors(&video->class_desc,
                                        puhost->dev_prop.cfg_desc_set.itf_desc_set[itf_index][alt_setting].itf_desc.bInterfaceSubClass, 
                                       (uint8_t *)pdesc);
                }
                break;

            case USB_DESC_TYPE_CS_ENDPOINT:
                sub_desc_type = *((uint8_t *)pdesc + 2);

                if (sub_desc_type == UVC_EP_INTERRUPT) {
                    video->class_desc.cs_desc.cs_endpoint_desc = (video_cs_endpoint *)pdesc;
                }
                break;

            default:
                break; 
        }
    }

    return USBH_OK;
}

/*!
    \brief      parse AC interfaces
    \param[in]  class_desc: device class descriptor
    \param[in]  ac_subclass: AC sub class
    \param[in]  pdesc: pointer to descriptor
    \param[out] none
    \retval     none
*/
static usbh_status ParseCSDescriptors(video_desc_class_specific *class_desc, uint8_t vs_subclass, uint8_t *pdesc)
{
    uint8_t desc_number = 0;

    if (vs_subclass == USB_SUBCLASS_VIDEOCONTROL) {
        switch (pdesc[2]) {
            case UVC_VC_HEADER: 
                class_desc->cs_desc.header_desc = (video_desc_header *)pdesc;
                break;

            case UVC_VC_INPUT_TERMINAL:
                class_desc->cs_desc.input_terminal_desc[class_desc->input_terminal_num++] = (video_desc_it*)pdesc;
                break;

            case UVC_VC_OUTPUT_TERMINAL:
                class_desc->cs_desc.output_terminal_desc[class_desc->output_terminal_num++] = (video_desc_ot*)pdesc;
                break;

            case UVC_VC_SELECTOR_UNIT:
                class_desc->cs_desc.selector_unit_desc[class_desc->selector_unit_num++] = (video_desc_selector*)pdesc; 
                break;

            case UVC_VC_PROCESSING_UNIT:
                class_desc->cs_desc.processing_unit_desc[class_desc->processing_unit_num++] = (video_desc_processing*)pdesc;
                break;

            case UVC_VC_EXTENSION_UNIT:
                class_desc->cs_desc.extension_unit_desc[class_desc->extension_unit_num++] = (video_desc_extension*)pdesc;
                break;

            default: 
                break;
        }
    } else if (vs_subclass == USB_SUBCLASS_VIDEOSTREAMING) {
        switch (pdesc[2]) {
            case UVC_VS_INPUT_HEADER:
                if (class_desc->input_header_num < VIDEO_MAX_NUM_IN_HEADER) {
                    class_desc->vs_desc.input_header[class_desc->input_header_num++] = (video_desc_in_header*)pdesc;
                }
                break;

            /* MJPEG */
            case UVC_VS_FORMAT_MJPEG:
                if (class_desc->mjpeg_format_num < VIDEO_MAX_MJPEG_FORMAT) {
                    class_desc->vs_desc.mjpeg_format[class_desc->mjpeg_format_num++] = (video_desc_mjpeg_format*)pdesc;

                    target_format = USBH_VIDEO_MJPEG;

                    LCD_DbgLog("Video formate is MJPEG\r\n");
                }
                break;

            case UVC_VS_FRAME_MJPEG:
                desc_number = class_desc->mjpeg_frame_num;
                if (desc_number < VIDEO_MAX_MJPEG_FRAME_D) {
                    class_desc->vs_desc.mjpeg_frame[desc_number] = (video_desc_mjpeg_frame*)pdesc;

                    uint16_t width = LE16(class_desc->vs_desc.mjpeg_frame[desc_number]->wWidth);
                    uint16_t height = LE16(class_desc->vs_desc.mjpeg_frame[desc_number]->wHeight);

                    usbh_video_target_width = width;
                    usbh_video_target_height = height;

                    LCD_DbgLog("MJPEG Frame detected: %d x %d \r\n", width, height);
                    class_desc->mjpeg_frame_num++;
                }
                break;

            /* UNCOMPRESSED */
            case UVC_VS_FORMAT_UNCOMPRESSED:
                if (class_desc->uncomp_format_num < VIDEO_MAX_UNCOMP_FORMAT) {
                    class_desc->vs_desc.uncomp_format[class_desc->uncomp_format_num++] = (video_desc_uncomp_format*)pdesc;

                    target_format = USBH_VIDEO_YUY2;

                    LCD_DbgLog("Video formate is uncompressed\r\n");
                }
                break;

            case UVC_VS_FRAME_UNCOMPRESSED:
                desc_number = class_desc->uncomp_frame_num;
                if (desc_number < VIDEO_MAX_UNCOMP_FRAME_D) {
                    class_desc->vs_desc.uncomp_frame[desc_number] = (video_desc_uncomp_frame*)pdesc;
                    uint16_t width = LE16(class_desc->vs_desc.uncomp_frame[desc_number]->wWidth);
                    uint16_t height = LE16(class_desc->vs_desc.uncomp_frame[desc_number]->wHeight);

                    usbh_video_target_width = width;
                    usbh_video_target_height = height;

                    LCD_DbgLog("Uncompressed Frame detected: %d x %d\r\n", width, height);
                    class_desc->uncomp_frame_num++;
                }
                break;

            default:
                break;
        }
    }

    return USBH_OK;
}

static void usbh_video_format_descriptors_analyse (video_desc_class_specific *class_desc)
{
    usbh_video_best_bformat_index = -1;

    if (target_format == USBH_VIDEO_MJPEG) {
        if (class_desc->mjpeg_format_num != 1) {
            LCD_DbgLog("Not supported MJPEG descriptors number: %d\n", class_desc->mjpeg_format_num);
        } else {
            video_desc_mjpeg_format* mjpeg_format_desc;

            mjpeg_format_desc = class_desc->vs_desc.mjpeg_format[0];
            usbh_video_best_bformat_index = mjpeg_format_desc->bFormatIndex;
        }

        return;
    } else if (target_format == USBH_VIDEO_YUY2) {
        if (class_desc->uncomp_format_num != 1) {
            LCD_DbgLog("Not supported UNCOMP descriptors number: %d\n", class_desc->uncomp_format_num);

            return;
        } else {
            /* Camera have a single Format descriptor, so we need to check if this descriptor is really YUY2 */
            video_desc_uncomp_format* uncomp_format_desc;
            uncomp_format_desc = class_desc->vs_desc.uncomp_format[0];

            if (memcmp(&uncomp_format_desc->guidFormat, "YUY2", 4) != 0) {
                LCD_DbgLog("Not supported UNCOMP descriptor type\n");

                return;
            } else {
                /* Found! */
                usbh_video_best_bformat_index = uncomp_format_desc->bFormatIndex;
            }
        }
    }
}

static void usbh_video_frame_descriptors_analyse(video_desc_class_specific *class_desc)
{
    usbh_video_best_bframe_index = -1;

//    if (target_format == USBH_VIDEO_MJPEG) {
        for (uint8_t i = 0; i < class_desc->mjpeg_frame_num; i++) {
            video_desc_mjpeg_frame* mjpeg_frame_desc;
            mjpeg_frame_desc = class_desc->vs_desc.mjpeg_frame[i];

            if ((LE16(mjpeg_frame_desc->wWidth) == usbh_video_target_width) && \
                    (LE16(mjpeg_frame_desc->wHeight) == usbh_video_target_height)) {
                /* Found! */
                usbh_video_best_bframe_index = mjpeg_frame_desc->bFrameIndex;
            }
        }
//    } else if (target_format == USBH_VIDEO_YUY2) {
        for (uint8_t i = 0; i < class_desc->uncomp_frame_num; i++) {
            video_desc_uncomp_frame* uncomp_frame_desc;
            uncomp_frame_desc = class_desc->vs_desc.uncomp_frame[i];

            if ((LE16(uncomp_frame_desc->wWidth) == usbh_video_target_width) && \
                    (LE16(uncomp_frame_desc->wHeight) == usbh_video_target_height)) {
                //Found!
                usbh_video_best_bframe_index = uncomp_frame_desc->bFrameIndex;
            }
        }
//    }
}

/*!
    \brief      handle Get Length request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  sub_type: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_len_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index, 
                                    uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    if (UVC_VC_EXTENSION_UNIT == sub_type) {
        UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
        InterfaceNum = video->control.interface;
        wIndex = (UnitID << 8) | InterfaceNum;
        wValue = (control_selector << 8);
        wLength = len;
    } else {
        return USBH_FAIL;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_LEN,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Info request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_info_get (usbh_host *puhost, 
                                     uint8_t sub_type, 
                                     uint8_t control_selector,
                                     uint8_t index, 
                                     uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (sub_type) {
        case UVC_VC_INPUT_TERMINAL:
            UnitID = video->class_desc.cs_desc.input_terminal_desc[index]->bTerminalID;
            break;

        case UVC_VC_PROCESSING_UNIT:
            UnitID = video->class_desc.cs_desc.processing_unit_desc[index]->bUnitID;
            break;

        case UVC_VC_EXTENSION_UNIT:
            UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
            break;

        default:
            return USBH_FAIL;
    }

    InterfaceNum = video->control.interface;
    wIndex = (UnitID << 8) | InterfaceNum;
    wValue = (control_selector << 8);
    wLength = len;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_INFO,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Max request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[in]  len: length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_max_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (sub_type) {
        case UVC_VC_INPUT_TERMINAL:
            UnitID = video->class_desc.cs_desc.input_terminal_desc[index]->bTerminalID;
            break;

        case UVC_VC_PROCESSING_UNIT:
            UnitID = video->class_desc.cs_desc.processing_unit_desc[index]->bUnitID;
            break;

        case UVC_VC_EXTENSION_UNIT:
            UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
            break;

        default:
            return USBH_FAIL;
    }

    InterfaceNum = video->control.interface;
    wIndex = (UnitID << 8) | InterfaceNum;
    wValue = (control_selector << 8);
    wLength = len;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_MAX,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Min request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[in]  len: length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_min_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (sub_type) {
        case UVC_VC_INPUT_TERMINAL:
            UnitID = video->class_desc.cs_desc.input_terminal_desc[index]->bTerminalID;
            break;

        case UVC_VC_PROCESSING_UNIT:
            UnitID = video->class_desc.cs_desc.processing_unit_desc[index]->bUnitID;
            break;

        case UVC_VC_EXTENSION_UNIT:
            UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
            break;

        default:
            return USBH_FAIL;
    }

    InterfaceNum = video->control.interface;
    wIndex = (UnitID << 8) | InterfaceNum;
    wValue = (control_selector << 8);
    wLength = len;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_MIN,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get def request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[in]  len: length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_def_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (sub_type) {
        case UVC_VC_INPUT_TERMINAL:
            UnitID = video->class_desc.cs_desc.input_terminal_desc[index]->bTerminalID;
            break;

        case UVC_VC_PROCESSING_UNIT:
            UnitID = video->class_desc.cs_desc.processing_unit_desc[index]->bUnitID;
            break;

        case UVC_VC_EXTENSION_UNIT:
            UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
            break;

        default:
            return USBH_FAIL;
    }

    InterfaceNum = video->control.interface;
    wIndex = (UnitID << 8) | InterfaceNum;
    wValue = (control_selector << 8);
    wLength = len;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_DEF,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Res request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  sub_type: subtype index
    \param[in]  control_selector: control code
    \param[in]  index: unit index
    \param[in]  len: length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vc_res_get (usbh_host *puhost, 
                                    uint8_t sub_type, 
                                    uint8_t control_selector,
                                    uint8_t index,
                                    uint16_t len)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t UnitID = 0, InterfaceNum = 0;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    switch (sub_type) {
        case UVC_VC_INPUT_TERMINAL:
            UnitID = video->class_desc.cs_desc.input_terminal_desc[index]->bTerminalID;
            break;

        case UVC_VC_PROCESSING_UNIT:
            UnitID = video->class_desc.cs_desc.processing_unit_desc[index]->bUnitID;
            break;

        case UVC_VC_EXTENSION_UNIT:
            UnitID = video->class_desc.cs_desc.extension_unit_desc[index]->bUnitID;
            break;

        default:
            return USBH_FAIL;
    }

    InterfaceNum = video->control.interface;
    wIndex = (UnitID << 8) | InterfaceNum;
    wValue = (control_selector << 8);
    wLength = len;

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_RES,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(video->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Set Cur request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  controlSelector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vs_cur_set(usbh_host *puhost, uint16_t req_type)
{
    uint16_t wLength = 26;

    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

//    if (req_type == (VS_PROBE_CONTROL << 8)) {
//        memset(&video->probe_params, 0, sizeof(video->probe_params));

//        /* Set needed params, at commit stage this parameters must be receied during "GET_CUR" */
//        video->probe_params.bmHint = 1;
//        video->probe_params.bFormatIndex = usbh_video_best_bformat_index;
//        video->probe_params.bFrameIndex = usbh_video_best_bframe_index;

//        /* Maximum framerate can be selected here */
//        video->probe_params.dwFrameInterval = 333333;//30 FPS
//    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_SET_CUR,
            .wValue        = req_type,
            .wIndex        = video->camera.interface,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)&video->probe_params, wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Cur request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  controlSelector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_vs_cur_get (usbh_host *puhost, uint16_t req_type)
{
    uint16_t wLength = 26;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    usbh_status status;

    memset(&video->probe_params, 0, sizeof(video->probe_params));

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_CUR,
            .wValue        = req_type,
            .wIndex        = video->camera.interface,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)&video->probe_params, wLength);
    }

    do {
        status = usbh_ctl_handler (puhost);
    } while (status == USBH_BUSY);

    if (status == USBH_OK) {
        if (video->probe_params.dwMaxVideoFrameSize > 0) {
            return USBH_OK;
        } else {
            return USBH_FAIL;
        }
    }

    return status;
}

static usbh_status usbh_vs_max_get (usbh_host *puhost, uint16_t req_type)
{
    uint16_t wLength = 26;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    usbh_status status;

    memset(&video->probe_params, 0, sizeof(video->probe_params));

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_MAX,
            .wValue        = req_type,
            .wIndex        = video->camera.interface,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)&video->probe_params, wLength);
    }

    do {
        status = usbh_ctl_handler (puhost);
    } while (status == USBH_BUSY);

    if (status == USBH_OK) {
        if (video->probe_params.dwMaxVideoFrameSize > 0) {
            return USBH_OK;
        } else {
            return USBH_FAIL;
        }
    }

    return status;
}

static usbh_status usbh_vs_min_get (usbh_host *puhost, uint16_t req_type)
{
    uint16_t wLength = 26;
    usbh_video_handle *video = (usbh_video_handle *)puhost->active_class->class_data;

    usbh_status status;

    memset(&video->probe_params, 0, sizeof(video->probe_params));

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UVC_GET_MIN,
            .wValue        = req_type,
            .wIndex        = video->camera.interface,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)&video->probe_params, wLength);
    }

    do {
        status = usbh_ctl_handler (puhost);
    } while (status == USBH_BUSY);

    if (status == USBH_OK) {
        if (video->probe_params.dwMaxVideoFrameSize > 0) {
            return USBH_OK;
        } else {
            return USBH_FAIL;
        }
    }

    return status;
}

static void video_stream_process_packet(uint16_t size)
{
    uvc_total_packet_cnt++;

    if ((size < 2) && (size > UVC_RX_FIFO_SIZE_LIMIT)) {
        return;
    }

    if ((uvc_parsing_enabled == 0) || (uvc_parsing_initialized == 0)) {
        /* try to switch buffers */
        video_stream_switch_buffers();
        uvc_prev_packet_eof = 0;

        return;
    }

    if (size <= UVC_HEADER_SIZE) {
        uvc_header_cnt++;
    } else if (size > UVC_HEADER_SIZE) {
        /* Detected packet with data */
        uvc_data_packet_cnt++;

        /* Get FID bit state */
        uint8_t masked_fid = (tmp_packet_framebuffer[UVC_HEADER_BIT_FIELD_POS] & UVC_HEADER_FID_BIT);

        if ((masked_fid != uvc_prev_fid_state) && (uvc_prev_packet_eof == 1)) {
            /* Detected FIRST packet of the frame */
            uvc_frame_cnt++;
            uvc_curr_frame_length = 0;
            uvc_frame_start_detected = 1;
        }

        uvc_prev_fid_state = masked_fid;

        uint16_t data_size = size - UVC_HEADER_SIZE;

        video_stream_add_packet_data((uint8_t*)&tmp_packet_framebuffer[UVC_HEADER_SIZE], data_size);

        if (tmp_packet_framebuffer[UVC_HEADER_BIT_FIELD_POS] & UVC_HEADER_EOF_BIT) {
            uvc_prev_packet_eof = 1;

            if (uvc_frame_start_detected == 0) {
                uvc_curr_frame_length = 0;
                return;
            }

            if (target_format == USBH_VIDEO_MJPEG) {
                uvc_parsing_enabled = 0;
                video_stream_switch_buffers();
            }
        } else {
            uvc_prev_packet_eof = 0;
        }

        if ((target_format == USBH_VIDEO_YUY2) && 
                (uvc_curr_frame_length >= UVC_UNCOMP_FRAME_SIZE)) {
            if (uvc_frame_start_detected == 0) {
                return; //Bad frame data
            }

            video_stream_switch_buffers();
        }
    }
}

static uint8_t video_stream_switch_buffers(void)
{
    /* "ready" buffer can be switched */
    if (uvc_parsing_switch_ready == 1) {
        uvc_ready_framebuffer_ptr = uvc_curr_framebuffer_ptr;

        if (uvc_curr_framebuffer_ptr == uvc_framebuffer0_ptr) {
            uvc_curr_framebuffer_ptr = uvc_framebuffer1_ptr;
        } else {
            uvc_curr_framebuffer_ptr = uvc_framebuffer0_ptr;
        }

        uvc_parsing_new_frame_ready = 1;
        uvc_parsing_switch_ready = 0;//waiting fo data to be processed by external software
        uvc_parsing_enabled = 1;
        uvc_frame_start_detected = 0;
        uvc_ready_frame_length = uvc_curr_frame_length;
        uvc_curr_frame_length = 0;

        return 1;
    } else {
        /* waiting for external software to release "Ready" buffer */
        uvc_parsing_enabled = 0;
    }

    return 0;
}

//Add data from received packet to the image framebuffer
//buf - pointer to the data source
static void video_stream_add_packet_data(uint8_t* buf, uint16_t data_size)
{
    if ((uvc_curr_frame_length + data_size) > UVC_UNCOMP_FRAME_SIZE) {
        uvc_curr_frame_length = UVC_UNCOMP_FRAME_SIZE;
        return;
    }

    /* Copy data to a current framebuffer */
    memcpy((void*)&uvc_curr_framebuffer_ptr[uvc_curr_frame_length], buf, data_size);
    uvc_curr_frame_length += data_size;
}
