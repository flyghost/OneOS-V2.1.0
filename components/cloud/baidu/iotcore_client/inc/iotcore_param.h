/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef IOTCORE_PARAM_H
#define IOTCORE_PARAM_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _IOTCORE_PARAM
{
    char *iot_core_id;
    char *device_key;
    char *device_secret;
    char *broker_addr;
} IOTCORE_INFO;

#ifdef __cplusplus
}
#endif

#endif // IOTCORE_PARAM_H
