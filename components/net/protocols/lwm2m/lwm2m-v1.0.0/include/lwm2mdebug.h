/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Simon Bernard - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/ 
#ifndef LWM2M_DEBUG_H_
#define LWM2M_DEBUG_H_

#include "clog.h"


#define TAG "lwm2mlog"

#define lwm2m_log(level, format, ...) do { \
        if( level == COMP_LOG_ERROR) { \
           COMP_LOGE(TAG, format, ##__VA_ARGS__); \
        } \
        else if( level == COMP_LOG_WARN) { \
           COMP_LOGW(TAG, format, ##__VA_ARGS__); \
        } \
        else if( level == COMP_LOG_INFO) { \
           COMP_LOGI(TAG, format, ##__VA_ARGS__); \
        } \
        else if( level == COMP_LOG_DEBUG) { \
           COMP_LOGD(TAG, format, ##__VA_ARGS__); \
        } \
        } while(0)
#endif /* LWM2MDEBUG_H_ */
