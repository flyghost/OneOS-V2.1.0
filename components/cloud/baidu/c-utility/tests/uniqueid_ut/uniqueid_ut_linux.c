// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/uniqueid.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

TEST_DEFINE_ENUM_TYPE(UNIQUEID_RESULT, UNIQUEID_RESULT_VALUES);

#define BUFFER_SIZE 37

static char *uidBuffer  = NULL;
static char *uidBuffer2 = NULL;

BEGIN_TEST_SUITE(uniqueid_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest) != 0)
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    if (uidBuffer != NULL)
    {
        free(uidBuffer);
        uidBuffer = NULL;
    }

    if (uidBuffer2 != NULL)
    {
        free(uidBuffer2);
        uidBuffer2 = NULL;
    }

    TEST_MUTEX_RELEASE(g_testByTest);
}

UNIQUEID_RESULT UniqueId_Generate(char *uid, size_t len)
{
    UNIQUEID_RESULT result;

    /* Codes_SRS_UNIQUEID_07_002: [If uid is NULL then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    /* Codes_SRS_UNIQUEID_07_003: [If len is less then 37 then UniqueId_Generate shall return UNIQUEID_INVALID_ARG] */
    if (uid == NULL || len < 37)
    {
        result = UNIQUEID_INVALID_ARG;
        LogError("Buffer Size is Null. (result = %" PRI_MU_ENUM ")", MU_ENUM_VALUE(UNIQUEID_RESULT, result));
    }
    else
    {
        uuid_t uuidVal;
        uuid_generate(uuidVal);

        /* Codes_SRS_UNIQUEID_07_001: [UniqueId_Generate shall create a unique Id 36 character long string.] */
        memset(uid, 0, len);
        uuid_unparse(uuidVal, uid);
        result = UNIQUEID_OK;
    }
    return result;
}

/* UniqueId_Generate */
TEST_FUNCTION(UniqueId_Generate_UID_NULL_Fail)
{
    // Arrange

    // Act
    UNIQUEID_RESULT result = UniqueId_Generate(NULL, BUFFER_SIZE);

    // Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

TEST_FUNCTION(UniqueId_Generate_Len_too_small_Fail)
{
    // Arrange
    char uid[BUFFER_SIZE];

    // Act
    UNIQUEID_RESULT result = UniqueId_Generate(uid, BUFFER_SIZE / 2);

    // Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_INVALID_ARG, result);
}

TEST_FUNCTION(UniqueId_Generate_Succeed)
{
    // Arrange
    char uid[BUFFER_SIZE];

    // Act
    UNIQUEID_RESULT result = UniqueId_Generate(uid, BUFFER_SIZE);

    // Assert
    ASSERT_ARE_EQUAL(UNIQUEID_RESULT, UNIQUEID_OK, result);
    ASSERT_ARE_EQUAL(size_t, 36, strlen(uid));
}

END_TEST_SUITE(uniqueid_unittests)
