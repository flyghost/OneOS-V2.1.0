/*
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file iot_tests.c
 * @brief Common test runner.
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <string.h>

/* Error handling include. */
#include "iot_error.h"

/* Platform threads include. */
#include "platform/iot_threads.h"

/* Test framework includes. */
#include "unity_fixture.h"

#include "iot_network.h"

/* OneOS logs. */
#include <dlog.h>
#define AWS_TEST_TAG "AwsIotTest"

/* Compile warning */
#if SHELL_TASK_STACK_SIZE < 8192
#error "SHELL_TASK_STACK_SIZE need more than or equal to 8192 bytes if using unity tests in shell"
#endif

#if AWS_IOT_LOG_LEVEL_GLOBAL > 2
#error "Log level can not be IOT_LOG_INFO or IOT_LOG_DEBUG, it may cause unity test judged FAIL."
#endif

/*-----------------------------------------------------------*/

/**
 * @brief Used to provide unity_malloc with critical sections.
 */
static IotMutex_t _unityMallocMutex;

/*-----------------------------------------------------------*/

/**
 * @brief Enter a critical section for unity_malloc.
 */
static void IotTest_EnterCritical( void )
{
    IotMutex_Lock( &_unityMallocMutex );
}

/*-----------------------------------------------------------*/

/**
 * @brief Exit a critical section for unity_malloc.
 */
static void IotTest_ExitCritical( void )
{
    IotMutex_Unlock( &_unityMallocMutex );
}

/*-----------------------------------------------------------*/

/* This file can also used to test the demos. */
#if IOT_TEST_DEMO == 1
/* All add by OneOS Team 2020.9.9 */

#define UnityRunDemoHandler_t RunDemoHandler_t

typedef int ( * RunDemoHandler_t )( bool awsIotMqttMode,
                                    const char * pIdentifier,
                                    void * pNetworkServerInfo,
                                    void * pNetworkCredentialInfo,
                                    const IotNetworkInterface_t * pNetworkInterface );

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

extern int AwsIotEntryDemo( RunDemoHandler_t runDemoHandler );


static int AwsIotUnityTests_RunDemos( UnityRunDemoHandler_t unityRunDemoHandler )
{   
    IOT_FUNCTION_ENTRY( int, EXIT_SUCCESS );
    bool mallocMutexCreated = false;
    
    /* Create the mutex that guards the Unity malloc overrides. */
    mallocMutexCreated = IotMutex_Create( &_unityMallocMutex, false );

    if( mallocMutexCreated == false )
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }

    /* Provide Unity with functions for critical sections. */
    unity_provide_critical_section( IotTest_EnterCritical, IotTest_ExitCritical );

    /* This file is also used to test the demos. When testing demos, the demo main
     * function is called. */
    if( unityRunDemoHandler != NULL )
    {
        status = AwsIotEntryDemo( unityRunDemoHandler );
    }

    IOT_FUNCTION_CLEANUP_BEGIN();

    if( mallocMutexCreated == true )
    {
        IotMutex_Destroy( &_unityMallocMutex );
        
        /* Must do this, otherwise unity test using shell is non-reentrant. */
        unity_clear_critical_section();
    }

    IOT_FUNCTION_CLEANUP_END();
}

static void UnityTestsRunDemosUsage(void)
{
    LOG_I(AWS_TEST_TAG, "AwsIotUnityRunDemos usage:");
    LOG_I(AWS_TEST_TAG, "AwsIotUnityRunDemos <TestDemoId: 1-6>");
    LOG_I(AWS_TEST_TAG, "TestDemoId 1: RunMqttDemo");
    LOG_I(AWS_TEST_TAG, "TestDemoId 2: RunShadowDemo");
    LOG_I(AWS_TEST_TAG, "TestDemoId 3: RunJobsDemo");
    LOG_I(AWS_TEST_TAG, "TestDemoId 4: RunDefenderDemo");
    LOG_I(AWS_TEST_TAG, "TestDemoId 5: RunProvisioningWithKeysAndCertDemo");
    LOG_I(AWS_TEST_TAG, "TestDemoId 6: RunProvisioningWithCsrDemo");
    
    return;
}

static void UnityTestsRunDemos(int argc, char *argv[])
{
    char opt;
    UnityRunDemoHandler_t unityRunDemoHandler = NULL;
    
    if (argc != 2)
    {
        LOG_E(AWS_TEST_TAG, "Less parameter, input <AwsIotUnityRunDemos h> for help.");
        return;
    }
    
    opt = argv[1][0];

    switch (opt)
    {
    /* unity test help */
    case 'h':
        UnityTestsRunDemosUsage();
        return;
    /* unity test RunMqttDemo */
    case '1':
        unityRunDemoHandler = RunMqttDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* unity test RunShadowDemo */
    case '2':
        unityRunDemoHandler = RunShadowDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* unity test RunJobsDemo */
    case '3':
        unityRunDemoHandler = RunJobsDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* unity test RunDefenderDemo */
    case '4':
        unityRunDemoHandler = RunDefenderDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* unity test RunProvisioningWithKeysAndCertDemo */
    case '5':
        unityRunDemoHandler = RunProvisioningWithKeysAndCertDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* unity test RunProvisioningWithCsrDemo */
    case '6':
        unityRunDemoHandler = RunProvisioningWithCsrDemo;
        AwsIotUnityTests_RunDemos(unityRunDemoHandler);
        break;
    /* UnknowParameter*/
    default:
        LOG_E(AWS_TEST_TAG, "Wrong parameter, input <AwsIotUnityRunDemos h> for help.");
        break;
    }
}
#endif

extern void RunAwsIotCommonTests( bool disableNetworkTests, bool disableLongTests );
extern void RunCommonTests( bool disableNetworkTests, bool disableLongTests );
extern void RunSerializerTests( bool disableNetworkTests, bool disableLongTests );
extern void RunMqttTests( bool disableNetworkTests, bool disableLongTests );
extern void RunShadowTests( bool disableNetworkTests, bool disableLongTests );
extern void RunJobsTests( bool disableNetworkTests, bool disableLongTests );
extern void RunDefenderTests( bool disableNetworkTests, bool disableLongTests );
extern void RunProvisioningTests( bool disableNetworkTests, bool disableLongTests );

typedef void ( * UnityRunTestHandler_t )( bool disableNetworkTests, bool disableLongTests );

typedef struct
{
    bool disableNetworkTests;
    bool disableLongTests;
    UnityRunTestHandler_t unityRunTestHandler;
}AwsIotUnityTests_RunTestsParm_t;

static int AwsIotUnityTests_RunTests(AwsIotUnityTests_RunTestsParm_t unityTests_RunTestsParm)
{
    IOT_FUNCTION_ENTRY( int, EXIT_SUCCESS );
    bool mallocMutexCreated;
    
    /* Create the mutex that guards the Unity malloc overrides. */
    mallocMutexCreated = IotMutex_Create( &_unityMallocMutex, false );

    if( mallocMutexCreated == false )
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }

    /* Provide Unity with functions for critical sections. */
    unity_provide_critical_section( IotTest_EnterCritical, IotTest_ExitCritical );

    /* Unity setup. */
    UnityFixture.Verbose = 1;
    UnityFixture.RepeatCount = 1;
    UnityFixture.NameFilter = NULL;
    UnityFixture.GroupFilter = NULL;
    UNITY_BEGIN();
    
    /* Call the test runner function. */
    unityTests_RunTestsParm.unityRunTestHandler(unityTests_RunTestsParm.disableNetworkTests, unityTests_RunTestsParm.disableLongTests);
    
    /* Return failure if any tests failed. */
    if( UNITY_END() != 0 )
    {
        IOT_SET_AND_GOTO_CLEANUP( EXIT_FAILURE );
    }

    IOT_FUNCTION_CLEANUP_BEGIN();

    if( mallocMutexCreated == true )
    {
        IotMutex_Destroy( &_unityMallocMutex );
        
        /* Must do this, otherwise unity test using shell is non-reentrant. */
        unity_clear_critical_section();
    }

    IOT_FUNCTION_CLEANUP_END();
}

static void UnityTestsRunTestsUsage(void)
{
    LOG_I(AWS_TEST_TAG, "AwsIotUnityRunTests usage:");
    LOG_I(AWS_TEST_TAG, "AwsIotUnityRunTests <TestUnitId: 1-8> <disableNetworkTests: 0-1> <disableLongTests: 0-1>");
    LOG_I(AWS_TEST_TAG, "TestUnitId 1: RunAwsIotCommonTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 2: RunCommonTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 3: RunSerializerTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 4: RunMqttTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 5: RunShadowTests ");
    LOG_I(AWS_TEST_TAG, "TestUnitId 6: RunJobsTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 7: RunDefenderTests");
    LOG_I(AWS_TEST_TAG, "TestUnitId 8: RunProvisioningTests");
    
    return;
}

static void UnityTestsRunTests(int argc, char *argv[])
{   
    char opt = '\0';
    char disableNetworkTests_Parm = '\0';
    char disableLongTests_Parm = '\0';
    AwsIotUnityTests_RunTestsParm_t awsIotUnityTests_RunTestsParm = {0};
    
    opt = argv[1][0];

    if (opt == 'h')
    {
        UnityTestsRunTestsUsage();
        return;
    }
    else
    {
        if (argc != 4)
        {
            LOG_E(AWS_TEST_TAG, "Less parameter, input <AwsIotUnityRunTests h> for help.");
            return;
        }
        
        disableNetworkTests_Parm = argv[2][0];
        switch (disableNetworkTests_Parm)
        {
        case '0':
            awsIotUnityTests_RunTestsParm.disableNetworkTests = 0;
            break;
        case '1':
            awsIotUnityTests_RunTestsParm.disableNetworkTests = 1;
            break;
        default:
            LOG_E(AWS_TEST_TAG, "Wrong parameter, input <AwsIotUnityRunTests h> for help.");
            return;
        }

        disableLongTests_Parm = argv[3][0];
        switch (disableLongTests_Parm)
        {
        case '0':
            awsIotUnityTests_RunTestsParm.disableLongTests = 0;
            break;
        case '1':
            awsIotUnityTests_RunTestsParm.disableLongTests = 1;
            break;
        default:
            LOG_E(AWS_TEST_TAG, "Wrong parameter, input <AwsIotUnityRunTests h> for help.");
            return;
        } 
        
        switch (opt)
        {
        /* RunAwsIotCommonTests */
        case '1':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunAwsIotCommonTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunCommonTests */
        case '2':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunCommonTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunSerializerTests */
        case '3':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunSerializerTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunMqttTests */
        case '4':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunMqttTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;  
        /* RunShadowTests */
        case '5':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunShadowTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunJobsTests */
        case '6':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunJobsTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunDefenderTests */
        case '7':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunDefenderTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* RunProvisioningTests */
        case '8':
            awsIotUnityTests_RunTestsParm.unityRunTestHandler = RunProvisioningTests;
            AwsIotUnityTests_RunTests(awsIotUnityTests_RunTestsParm);
            break;
        /* UnknowParameter*/
        default:
            LOG_E(AWS_TEST_TAG, "Wrong parameter, input <AwsIotUnityRunTests h> for help.");
            break;
        }
    }
}

#ifdef OS_USING_SHELL
#include <shell.h>
#if IOT_TEST_DEMO == 1
SH_CMD_EXPORT(AwsIotUnityRunDemos, UnityTestsRunDemos, "use <AwsIotUnityRunDemos h> list the information of all unity run demos interface");
#endif
SH_CMD_EXPORT(AwsIotUnityRunTests, UnityTestsRunTests, "use <AwsIotUnityRunTests h> list the information of all unity run tests interfaces");
#endif /* OS_USING_SHELL */
