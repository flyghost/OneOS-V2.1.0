/*
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* This file contains configuration settings for the demos. */

#ifndef IOT_CONFIG_H_
#define IOT_CONFIG_H_

#include <oneos_config.h>

/* Server endpoints used for the demos. May be overridden with command line
 * options at runtime. */
#define IOT_DEMO_SECURED_CONNECTION    ( true )              

#define IOT_DEMO_SERVER                AWS_IOT_DEMO_SERVER
#define IOT_DEMO_PORT                  AWS_IOT_DEMO_PORT

/* Credential paths. May be overridden with command line options at runtime. */
#ifdef IOT_DEMO_SECURED_CONNECTION
#include "aws_iot_cert.h"
#endif

#ifdef AWS_IOT_DEMO_ROOT_CA
    #define IOT_DEMO_ROOT_CA           ( amazon_root_ca_cert )
#else
    #define IOT_DEMO_ROOT_CA           ( NULL )
#endif

#ifdef AWS_IOT_DEMO_CLIENT_CERT
    #define IOT_DEMO_CLIENT_CERT       ( amazon_client_cert )
#else
    #define IOT_DEMO_CLIENT_CERT       ( NULL )
#endif

#ifdef AWS_IOT_DEMO_PRIVATE_KEY
    #define IOT_DEMO_PRIVATE_KEY       ( amazon_client_key )
#else
    #define IOT_DEMO_PRIVATE_KEY       ( NULL )
#endif

#define IOT_DEMO_USER_NAME             AWS_IOT_DEMO_USER_NAME
#define IOT_DEMO_PASSWORD              AWS_IOT_DEMO_PASSWORD

/* MQTT client identifier (MQTT demo only) or AWS IoT Thing Name. May be set at
 * runtime with the command line option -i. Identifiers are optional for the
 * MQTT demo, but required for demos requiring a Thing Name. (The MQTT demo will
 * generate a unique identifier if no identifier is given). If a specific Thing Name
 * is required please define the following line:
 */
#define IOT_DEMO_IDENTIFIER            AWS_IOT_DEMO_IDENTIFIER

/* MQTT demo configuration. The demo publishes bursts of messages. */
#define IOT_DEMO_MQTT_PUBLISH_BURST_COUNT    ( 10 )    /* Number of message bursts. */
#define IOT_DEMO_MQTT_PUBLISH_BURST_SIZE     ( 10 )    /* Number of messages published in each burst. */

/* MQTT library configuration. */
#ifndef IOT_MQTT_ENABLE_SERIALIZER_OVERRIDES
    #define IOT_MQTT_ENABLE_SERIALIZER_OVERRIDES    ( 0 )
#endif

/* If metrics are enabled, write the metrics username into the CONNECT packet.
 * Otherwise, write the username and password only when not connecting to the
 * AWS IoT MQTT server. 
 * Anonymous metrics (SDK language, SDK version) will be provided to AWS IoT.
 * Recompile with AWS_IOT_MQTT_ENABLE_METRICS set to 0 to disable.
 */ 
/* add by OneOS Team */
#define AWS_IOT_MQTT_ENABLE_METRICS  ( 0 )
/* add by OneOS Team */
#define IOT_MQTT_RESPONSE_WAIT_MS    ( 5000U )

/* Shadow demo configuration. The demo publishes periodic Shadow updates and responds
 * to changing Shadows. */
#define AWS_IOT_DEMO_SHADOW_UPDATE_COUNT                                  ( 20 )   /* Number of updates to publish. */
#define AWS_IOT_DEMO_SHADOW_UPDATE_PERIOD_MS                              ( 3000 ) /* Period of Shadow updates. */

/**
 * The Certificate-Signing Request string to use for CSR-based Provisioning demo app.
 */
#define AWS_IOT_DEMO_PROVISIONING_CSR_PEM                                 \
"-----BEGIN CERTIFICATE REQUEST-----\r\n" \
"MIIC1DCCAbwCAQAwgY4xCzAJBgNVBAYTAkFTMRIwEAYDVQQIDAl1cy1lYXN0LTIx\r\n" \
"EjAQBgNVBAcMCXVzLWVhc3QtMjEMMAoGA1UECgwDQVdTMRAwDgYDVQQLDAdJVCBE\r\n" \
"ZXB0MTcwNQYDVQQDDC5hMnZ3ZTFnc3M1eDFxcS1hdHMuaW90LnVzLWVhc3QtMi5h\r\n" \
"bWF6b25hd3MuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxOLO\r\n" \
"Yalz6j9tsL2VlHqJp7BKPIfz3AGGNjlGtn4+f2hJJgnCf4rmFT9TVMqtT0dL9D2M\r\n" \
"J5L7nshXtakCpbOwCu7wkMsiIJzqee0QFQMdesIkuk80sZf7wtc51JZHKDpmMGuE\r\n" \
"w6jsdTBA/6E+6rk4l2KmLWTnVox2FiqWYXX1EqRMXFW4LKtzzR9ehcbXoMOzfSa6\r\n" \
"nhU80YrzfWDcIOq4GahDQl/VtMZ54e48vqxC2JTYVjMOVYnZqIVPYXR6DhmGgFuW\r\n" \
"V9lj84/vri37yGB0jkely41Fq/xe4z8YUV+XoqnINZaMPwL+8urShWQhK1sG1NAT\r\n" \
"gwJJxNBD/0K/M4BKIQIDAQABoAAwDQYJKoZIhvcNAQEFBQADggEBACyGSHxAvrY0\r\n" \
"RkoJPdP9cFpgg0xFk2/EZrAZRvNIYRzMfF7d0XzruGQUhZ/bM6o6ZnX8mJ/MrQrn\r\n" \
"bXorDa2FzEIoXAAE/Gk7mEwjBtEBT5UBNvIDk01/ZARfivnKLKS2rOsX6koxc4mp\r\n" \
"BrlEDkSvAACxginRuJnvOM7U+MAYJPpbWkUu2HWPd3bXryMWZbDpQxxlEYe0mt77\r\n" \
"Vjb3FwGfBIxgv+AdEihK2jIjU3Zqo1V423VqWh0UOyx8lc84xBTXCv5T12tHfr3r\r\n" \
"UDhUJ91ZMOldd0annXa52O7mmF1BYThxh8RhxuYc6VpxdLTTdgIc10YPJII/tijI\r\n" \
"4PzynEUiuOU=\r\n" \
"-----END CERTIFICATE REQUEST-----\r\n" 

/**
 * The name for the fleet provisioning template that will be used for provisioning
 * for registering thing in the Provisioning demo applications.
 */
#define AWS_IOT_DEMO_PROVISIONING_TEMPLATE_NAME                           "CERT"

/**
 * List of parameters that will be used for provisioning in the demo application.
 * There are 2 requirements for passing parameters to the Provisioning demo:
 * 1. One parameter is a "SerialNumber". The AWS IoT console generates a provisioning template
 * that by default uses a "SerialNumber" device parameter for creating a Thing resource. Also, the
 * setup instructions in the API reference documentation of Fleet Provisioning
 * (https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/provisioning/provisioning_tests.html
 * #provisioning_system_tests_setup)
 * provide an AWS CLI command example for creating a provisioning template, in which the template
 * JSON string contains the "SerialNumber" parameter.
 * 2. A second parameter which is configurable for both the name and value. The example provisioning
 * JSON string in the demo setup instructions contains "DeviceLocation" as the default parameter name.
 * If a different parameter name is used, make SURE to update the provisioning template on the AWS IoT Console!
 */
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_NAME            "SerialNumber"
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_NAME_LENGTH     sizeof( AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_NAME ) - 1
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_VALUE           "OneOSAwsTest001"
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_VALUE_LENGTH    sizeof( AWS_IOT_DEMO_PROVISIONING_PARAMETER_SERIAL_NUMBER_VALUE ) - 1
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_NAME                        "DeviceLocation"
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_NAME_LENGTH                 sizeof( AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_NAME ) - 1
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_VALUE                       "Chengdu"
#define AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_VALUE_LENGTH                sizeof( AWS_IOT_DEMO_PROVISIONING_PARAMETER_2_VALUE ) - 1

/* Enable asserts in the libraries. */
#define IOT_CONTAINERS_ENABLE_ASSERTS                                     ( 1 )
#define IOT_MQTT_ENABLE_ASSERTS                                           ( 1 )
#define IOT_TASKPOOL_ENABLE_ASSERTS                                       ( 1 )
#define AWS_IOT_SHADOW_ENABLE_ASSERTS                                     ( 1 )
#define AWS_IOT_DEFENDER_ENABLE_ASSERTS                                   ( 1 )
#define AWS_IOT_JOBS_ENABLE_ASSERTS                                       ( 1 )
#define AWS_IOT_PROVISIONING_ENABLE_ASSERTS                               ( 1 )

/* Library logging configuration. IOT_LOG_LEVEL_GLOBAL provides a global log
 * level for all libraries; the library-specific settings override the global
 * setting. If both the library-specific and global settings are undefined,
 * no logs will be printed. */
#define IOT_LOG_LEVEL_GLOBAL                                              AWS_IOT_LOG_LEVEL_GLOBAL//IOT_LOG_INFO

#define IOT_LOG_LEVEL_DEMO                                                IOT_LOG_DEBUG//IOT_LOG_INFO
#define IOT_LOG_LEVEL_PLATFORM                                            IOT_LOG_DEBUG//IOT_LOG_INFO//IOT_LOG_NONE
#define IOT_LOG_LEVEL_NETWORK                                             IOT_LOG_DEBUG//IOT_LOG_INFO
#define IOT_LOG_LEVEL_TASKPOOL                                            IOT_LOG_DEBUG//IOT_LOG_NONE
#define IOT_LOG_LEVEL_MQTT                                                IOT_LOG_DEBUG//IOT_LOG_INFO
#define AWS_IOT_LOG_LEVEL_SHADOW                                          IOT_LOG_DEBUG//IOT_LOG_INFO
#define AWS_IOT_LOG_LEVEL_DEFENDER                                        IOT_LOG_DEBUG//IOT_LOG_INFO
#define AWS_IOT_LOG_LEVEL_JOBS                                            IOT_LOG_DEBUG//IOT_LOG_INFO
#define AWS_IOT_LOG_LEVEL_PROVISIONING                                    IOT_LOG_DEBUG//IOT_LOG_INFO

/* Default assert and memory allocation functions. */
#include <assert.h>
#include <stdlib.h>

#define Iot_DefaultAssert    OS_ASSERT
#define Iot_DefaultMalloc    malloc
#define Iot_DefaultFree      free

/* The build system will choose the appropriate system types file for the platform
 * layer based on the host operating system. */
//#include IOT_SYSTEM_TYPES_FILE
#include <os_task.h>
#include <os_assert.h>
#include <os_clock.h>

#ifdef AWS_IOT_USING_UNITY_TESTS
#define IOT_BUILD_TESTS 1
#else
#define IOT_BUILD_TESTS 0
#endif

#endif /* ifndef IOT_CONFIG_H_ */
