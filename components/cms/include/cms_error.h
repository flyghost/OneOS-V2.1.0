/*
 * Copyright (c, 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/**
 * \file cms_error.h
 * \brief Standard error codes for the SPM and RoT Services
 */

#ifndef __CMS_ERROR_H__
#define __CMS_ERROR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        CMS_ERROR_SUCCESS = 0,
        CMS_ERROR_PROGRAMMER_ERROR = -29,
        CMS_ERROR_CONNECTION_REFUSED = -30,
        CMS_ERROR_CONNECTION_BUSY = -31,
        CMS_ERROR_GENERIC_ERROR = -32,
        CMS_ERROR_NOT_PERMITTED = -33,
        CMS_ERROR_NOT_SUPPORTED = -34,
        CMS_ERROR_INVALID_ARGUMENT = -35,
        CMS_ERROR_INVALID_HANDLE = -36,
        CMS_ERROR_BAD_STATE = -37,
        CMS_ERROR_BUFFER_TOO_SMALL = -38,
        CMS_ERROR_ALREADY_EXISTS = -39,
        CMS_ERROR_DOES_NOT_EXIST = -40,
        CMS_ERROR_INSUFFICIENT_MEMORY = -41,
        CMS_ERROR_INSUFFICIENT_STORAGE = -42,
        CMS_ERROR_INSUFFICIENT_DATA = -43,
        CMS_ERROR_SERVICE_FAILURE = -44,
        CMS_ERROR_COMMUNICATION_FAILURE = -45,
        CMS_ERROR_STORAGE_FAILURE = -46,
        CMS_ERROR_HARDWARE_FAILURE = -47,  /* 存储错误 */
        CMS_ERROR_CTX_INIT_FAILURE = -48,  /* 状态初始化错误 */
        CMS_ERROR_INVALID_SIGNATURE = -49, /* 签名验证错误 */
        // CMS Connect Component Error Code
        CMS_ERROR_INPUT_PARAM_ERROR = -100,      //输入参数错误
        CMS_ERROR_OUT_OF_MEMERY = -101,          //内存不足
        CMS_ERROR_AUTH_FAILED = -102,            //认证失败
        CMS_ERROR_BUFF_INSUFFICIENT_BUFF = -103, //预配置的缓存空间不足
        CMS_ERROR_NOT_CONNECTED = -104,          //未连接到中台
        CMS_ERROR_PACKAGE = -105,                //数据包错误
        CMS_ERROR_SERVICE_DUP_REGIST = -106,     //服务重复注册
        CMS_ERROR_PLATFORM = -107,               //平台返回错误
        CMS_ERROR_CONNECT_FAILED = -108,         //连接失败
        CMS_ERROR_SEND_FAILED            = -109,    //发送失败
        CMS_ERROR_DEVICE_OFFLINE         = -110,    //设备离线
    };

#ifdef __cplusplus
}
#endif

#endif /* __PSA_ERROR_H__ */
