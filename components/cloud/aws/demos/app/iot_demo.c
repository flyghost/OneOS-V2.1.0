/*
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file iot_demo.c
 * @brief Generic demo runner.
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* SDK initialization include. */
#include "iot_init.h"

/* Error handling include. */
#include "iot_error.h"

/* Common demo includes. */
#include "iot_demo_arguments.h"
#include "iot_demo_logging.h"

/* mbed TLS network include. */
#include "iot_network_mbedtls.h"

#define IOT_DEMO_NETWORK_INTERFACE                   IOT_NETWORK_INTERFACE_MBEDTLS
#define IOT_DEMO_SERVER_INFO_INITIALIZER             IOT_NETWORK_SERVER_INFO_MBEDTLS_INITIALIZER
#define IOT_DEMO_CREDENTIALS_INITIALIZER             AWS_IOT_NETWORK_CREDENTIALS_MBEDTLS_INITIALIZER
#define IOT_DEMO_ALPN_FOR_PASSWORD_AUTHENTICATION    AWS_IOT_PASSWORD_ALPN_FOR_MBEDTLS

#define IotDemoNetwork_Init                          IotNetworkMbedtls_Init
#define IotDemoNetwork_Cleanup                       IotNetworkMbedtls_Cleanup

/* OneOS logs. */
#include <dlog.h>
#define AWS_DEMO_TAG "AwsIotDemo"

/* Compile warning */
#if SHELL_TASK_STACK_SIZE < 4096
#error "SHELL_TASK_STACK_SIZE need more than 4096 bytes if using demo in shell"
#endif

#if MBEDTLS_SSL_MAX_CONTENT_LEN < 6144
#error "mbedtls MBEDTLS_SSL_MAX_CONTENT_LEN need more than 6144"
#endif

/* demo function */
extern int RunMqttDemo( bool awsIotMqttMode,
                         const char * pIdentifier,
                         void * pNetworkServerInfo,
                         void * pNetworkCredentialInfo,
                         const IotNetworkInterface_t * pNetworkInterface );

extern int RunShadowDemo( bool awsIotMqttMode,
                           const char * pIdentifier,
                           void * pNetworkServerInfo,
                           void * pNetworkCredentialInfo,
                           const IotNetworkInterface_t * pNetworkInterface );

extern int RunJobsDemo( bool awsIotMqttMode,
                         const char * pIdentifier,
                         void * pNetworkServerInfo,
                         void * pNetworkCredentialInfo,
                         const IotNetworkInterface_t * pNetworkInterface );


extern int RunDefenderDemo( bool awsIotMqttMode,
                             const char * pIdentifier,
                             void * pNetworkServerInfo,
                             void * pNetworkCredentialInfo,
                             const IotNetworkInterface_t * pNetworkInterface );

extern int RunProvisioningWithKeysAndCertDemo( bool awsIotMqttMode,
                                                const char * pIdentifier,
                                                void * pNetworkServerInfo,
                                                void * pNetworkCredentialInfo,
                                                const IotNetworkInterface_t * pNetworkInterface );

extern int RunProvisioningWithCsrDemo( bool awsIotMqttMode,
                                        const char * pIdentifier,
                                        void * pNetworkServerInfo,
                                        void * pNetworkCredentialInfo,
                                        const IotNetworkInterface_t * pNetworkInterface );

/*-----------------------------------------------------------*/

typedef int ( * RunDemoHandler_t )( bool awsIotMqttMode,
                                         const char * pIdentifier,
                                         void * pNetworkServerInfo,
                                         void * pNetworkCredentialInfo,
                                         const IotNetworkInterface_t * pNetworkInterface );

/*-----------------------------------------------------------*/

int AwsIotEntryDemo( RunDemoHandler_t runDemoHandler )
{
    /* Return value of this function and the exit status of this program. */
    IOT_FUNCTION_ENTRY( int, EXIT_SUCCESS );

    /* Status returned from network stack initialization. */
    IotNetworkError_t networkInitStatus = IOT_NETWORK_SUCCESS;

    /* Flags for tracking which cleanup functions must be called. */
    bool sdkInitialized = false, networkInitialized = false;

    /* Arguments for this demo. */
    IotDemoArguments_t demoArguments = IOT_DEMO_ARGUMENTS_INITIALIZER;

    /* Network server info and credentials. */
    struct IotNetworkServerInfo serverInfo = IOT_DEMO_SERVER_INFO_INITIALIZER;
    struct IotNetworkCredentials credentials = IOT_DEMO_CREDENTIALS_INITIALIZER,
                               * pCredentials = NULL;

    /* set the demoArguments load from user configuration */
    if( IotDemo_LoadArguments ( &demoArguments ) == false )
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }
                              
    /* Set the members of the server info. */
    serverInfo.pHostName = demoArguments.pHostName;
    serverInfo.port = demoArguments.port;

    /* For a secured connection, set the members of the credentials. */
    if( demoArguments.securedConnection == true )
    {
      /* Set credential information. */
        credentials.pClientCert = demoArguments.pClientCert;
        credentials.pPrivateKey = demoArguments.pPrivateKey;
        credentials.pRootCa = demoArguments.pRootCaCert;
        credentials.pUserName = NULL;
        credentials.pPassword = NULL;

      /* Set the MQTT username, as long as it's not empty or NULL. */
      if( demoArguments.pUserName != NULL )
      {
          credentials.userNameSize = strlen( demoArguments.pUserName );

          if( credentials.userNameSize > 0 )
          {
              credentials.pUserName = demoArguments.pUserName;
          }
      }

      /* Set the MQTT password, as long as it's not empty or NULL. */
      if( demoArguments.pPassword != NULL )
      {
          credentials.passwordSize = strlen( demoArguments.pPassword );

          if( credentials.passwordSize > 0 )
          {
              credentials.pPassword = demoArguments.pPassword;
          }
      }

      /* By default, the credential initializer enables ALPN with AWS IoT,
       * which only works over port 443. Clear that value if another port is
       * used. */
      if( demoArguments.port != 443 )
      {
          credentials.pAlpnProtos = NULL;
      }

      /* Per IANA standard:
       * https://www.iana.org/assignments/tls-extensiontype-values/tls-extensiontype-values.xhtml. */
      if( ( credentials.pUserName != NULL ) &&
          ( demoArguments.awsIotMqttMode == true ) )
      {
          credentials.pAlpnProtos = IOT_DEMO_ALPN_FOR_PASSWORD_AUTHENTICATION;
      }

      /* Set the pointer to the credentials. */
        pCredentials = &credentials;
    }

    /* Call the SDK initialization function. */
    sdkInitialized = IotSdk_Init();

    if( sdkInitialized == false )
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }

    /* Initialize the network stack. */
    networkInitStatus = IotDemoNetwork_Init();

    if( networkInitStatus == IOT_NETWORK_SUCCESS )
    {
        networkInitialized = true;
    }
    else
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }

    /* Run the demo. */
    if (runDemoHandler == NULL) 
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }
    
    status = runDemoHandler( demoArguments.awsIotMqttMode,
                             demoArguments.pIdentifier,
                             &serverInfo,
                             pCredentials,
                             IOT_DEMO_NETWORK_INTERFACE );

    IOT_FUNCTION_CLEANUP_BEGIN();

    /* Clean up the network stack if initialized. */
    if( networkInitialized == true )
    {
        IotDemoNetwork_Cleanup();
    }

    /* Clean up the SDK if initialized. */
    if( sdkInitialized == true )
    {
        IotSdk_Cleanup();
    }

    /* Log the demo status. */
    if( status == EXIT_SUCCESS )
    {
        IotLogInfo( "Demo completed successfully." );
    }
    else
    {
        IotLogError( "Error occurred while running demo." );
    }

    IOT_FUNCTION_CLEANUP_END();
}

/*-----------------------------------------------------------*/

#ifdef AWS_IOT_USING_DEMO
static void AwsIotDemoRunUsage()
{
    LOG_I(AWS_DEMO_TAG, "AwsIotDemoRun usage:");
    LOG_I(AWS_DEMO_TAG, "AwsIotDemoRun <DemoId: 1-6>");
    LOG_I(AWS_DEMO_TAG, "DemoId 1: RunMqttDemo");
    LOG_I(AWS_DEMO_TAG, "DemoId 2: RunShadowDemo");
    LOG_I(AWS_DEMO_TAG, "DemoId 3: RunJobsDemo");
    LOG_I(AWS_DEMO_TAG, "DemoId 4: RunDefenderDemo");
    LOG_I(AWS_DEMO_TAG, "DemoId 5: RunProvisioningWithKeysAndCertDemo");
    LOG_I(AWS_DEMO_TAG, "DemoId 6: RunProvisioningWithCsrDemo");
    return;
}

static void AwsIotDemoRun(int argc, char *argv[])
{
    char opt;
    RunDemoHandler_t runDemoHandler = NULL;
    
    if (argc != 2)
    {
        LOG_E(AWS_DEMO_TAG, "Less parameter, input <AwsIotDemoRun h> for help.");
        return;
    }
    
    opt = argv[1][0];

    switch (opt)
    {
    /* help */
    case 'h':
        AwsIotDemoRunUsage();
        return;
    /* RunMqttDemo */
    case '1':
        runDemoHandler = RunMqttDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;
    /* RunShadowDemo */
    case '2':
        runDemoHandler = RunShadowDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;
    /* RunJobsDemo */
    case '3':
        runDemoHandler = RunJobsDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;
    /* RunDefenderDemo */
    case '4':
        runDemoHandler = RunDefenderDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;
    /* RunProvisioningWithKeysAndCertDemo */
    case '5':
        runDemoHandler = RunProvisioningWithKeysAndCertDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;    
    /* RunProvisioningWithCsrDemo */
    case '6':
        runDemoHandler = RunProvisioningWithCsrDemo;
        AwsIotEntryDemo(runDemoHandler);
        break;
    /* UnknowParameter */
    default:
        LOG_E(AWS_DEMO_TAG, "Wrong parameter, input <AwsIotDemoRun h> for help.");
        break;
    }
    
}
#endif

#if defined(OS_USING_SHELL) && defined(AWS_IOT_USING_DEMO)
#include <shell.h>
SH_CMD_EXPORT(AwsIotDemoRun, AwsIotDemoRun, "use <AwsIotDemoRun h> list the information of demo run interface");
#endif /* OS_USING_SHELL */
/*-----------------------------------------------------------*/
