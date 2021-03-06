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

/**
 * @file iot_demo_arguments.h
 * @brief Declares the function and structure used for processing command line
 * arguments
 */

#ifndef IOT_DEMO_ARGUMENTS_H_
#define IOT_DEMO_ARGUMENTS_H_

/* Standard includes. */
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Holds the arguments for a single demo.
 *
 * Each demo will use one of these structs to hold its arguments.
 *
 * The default values of this struct may be set using compile-time constants,
 * either through the config file or a compiler option like `-D`.
 *
 * The default values may be overridden using command line arguments. If a default
 * value was not set, then a valid value must be set using a command line argument.
 *
 * @initializer{IotDemoArguments_t,IOT_DEMO_ARGUMENTS_INITIALIZER}
 */
 #if 0
typedef struct IotDemoArguments
{
    bool awsIotMqttMode;    /**< @brief Whether the demo is using the AWS IoT MQTT server. */
    bool securedConnection; /**< @brief Whether to secure the network connection with TLS. */
    const char * pHostName; /**< @brief The remote host name that the demo will connect to. */
    uint16_t port;          /**< @brief The remote host port that the demo will connect to. */

    /* These credentials are only used if securedConnection is true. */
    const char * pRootCaPath;     /**< @brief The path to the server root certificate to use for the connection. */
    const char * pClientCertPath; /**< @brief The path to the client certificate to use for the connection. */
    const char * pPrivateKeyPath; /**< @brief The path to the private key that matches the client certificate. */
    const char * pUserName;       /**< @brief The username for authenticating to the MQTT broker. */
    const char * pPassword;       /**< @brief The password for authenticating to the MQTT broker. */

    const char * pIdentifier;     /**< @brief The client identifier or Thing Name to use for demo. */
} IotDemoArguments_t;
#endif
typedef struct IotDemoArguments
{
    bool awsIotMqttMode;    /**< @brief Whether the demo is using the AWS IoT MQTT server. */
    bool securedConnection; /**< @brief Whether to secure the network connection with TLS. */
    const char * pHostName; /**< @brief The remote host name that the demo will connect to. */
    uint16_t port;          /**< @brief The remote host port that the demo will connect to. */

    /* These credentials are only used if securedConnection is true. */
    const char * pRootCaCert;     /**< @brief Point of the server root certificate to use for the connection. */
    const char * pClientCert;     /**< @brief Point of the client certificate to use for the connection. */
    const char * pPrivateKey;     /**< @brief Point of the private key that matches the client certificate. */
    const char * pUserName;       /**< @brief The username for authenticating to the MQTT broker. */
    const char * pPassword;       /**< @brief The password for authenticating to the MQTT broker. */

    const char * pIdentifier;     /**< @brief The client identifier or Thing Name to use for demo. */
} IotDemoArguments_t;

/**
 * @brief Provides default values for an #IotDemoArguments_t.
 *
 * All instances of #IotDemoArguments_t should be initialized with this
 * constant.
 *
 * @code{c}
 * IotDemoArguments_t demoArguments = IOT_DEMO_ARGUMENTS_INITIALIZER;
 * @endcode
 *
 * @warning Failing to initialize an #IotDemoArguments_t with this initializer
 * may result in undefined behavior!
 * @note This initializer may change at any time in future versions, but its
 * names will remain the same.
 */
#define IOT_DEMO_ARGUMENTS_INITIALIZER    { 0 }

#if 0
/**
 * @brief Parses command line arguments.
 *
 * The functions for parsing command line arguments differ depending on the
 * operating system. Therefore, this function is re-implemented for different
 * platforms.
 *
 * @param[in] argc The argument count originally passed to main().
 * @param[in] argv The argument vector originally passed to main().
 * @param[out] pArguments Set to the arguments parsed from the command line.
 *
 * @return `true` if all arguments are valid; `false` otherwise.
 */
bool IotDemo_ParseArguments( int argc,
                             char ** argv,
                             IotDemoArguments_t * pArguments );
#endif

/**
 * @brief Load Demo arguments.
 *
 * The functions for loading demo arguments
 *
 * @param[out] pArguments set to the arguments load from user configuration.
 *
 * @return `true` if all arguments are valid; `false` otherwise.
 */
bool IotDemo_LoadArguments( IotDemoArguments_t * pArguments );

#endif /* ifndef IOT_DEMO_ARGUMENTS_H_ */
