/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        onenet_device_sample.h
 * 
 * @brief       This file contain the work information of the device of the OneNET-MQTTS process.  
 * 
 * @details     USER_PRODUCT_ID           Product ID, get in OneNET web, when User has registered with MQTT.
 *              USER_ACCESS_KEY           Access_key, get in OneNET web.
 *              USER_DEVICE_NAME          Device name, defined by user, need a unique identification.
 *              USER_KEEPALIVE_INTERVAL   HeartBeat period.
 *              USER_PUBLISH_INTERVAL     cycle publish period.
 *              USER_DEVICE_ID            deivce ID, if open "ONENET_MQTTS_USING_AUTO_REGISTER", dont care; \
 *                                        else get in OneNET web.
 *              USER_KEY                  device key, if open "ONENET_MQTTS_USING_AUTO_REGISTER", dont care;\
 *                                        else get in OneNET web.
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _ONENET_DEVICE_SAMPLE_H_
#define _ONENET_DEVICE_SAMPLE_H_

#define USER_PRODUCT_ID  "398523"
#define USER_ACCESS_KEY  "TtqIgt8Q4S8u6iCOqEI1EmNWQr1x9eo+LpYMfRrz30s="
#define USER_DEVICE_NAME "pandora-mqtt-test" /*use characters, numbers or symbols like '_' or '-',\
                                             no longer than 64, can use device serial num*/
#define USER_KEEPALIVE_INTERVAL 240 /*onenent heart interval 10~1800s*/
#define USER_PUBLISH_INTERVAL   10  /*user onenet data upload interval*/

#ifndef ONENET_MQTTS_USING_AUTO_REGISTER
#define USER_DEVICE_ID "594722122"
#define USER_KEY       "4EjlgCv+PW47/WK0ImryIOVwk9nXOYF2c6FJxRRRvfo="
#endif

extern const char *base_dp_upload_str;

#endif /* _ONENET_DEVICE_SAMPLE_H_ */
