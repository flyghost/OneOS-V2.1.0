/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        http_sample_test.c
 *
 * \@brief       http get/post  interface test
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-8-12   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_assert.h>
#include <oneos_config.h>
#include <errno.h>

#ifdef HTTPCLIENT_USING_SAMPLE

#include "http_oneos_wrapper.h"
#include "http_application_api.h"
#include "os_errno.h"

#ifdef CONFIG_HTTP_SECURE
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
#endif



os_task_t *http_get_sample_task = OS_NULL;
os_task_t *http_post_sample_task = OS_NULL;

#ifdef CONFIG_HTTP_SECURE
/* @brief https get sample test */
char *geturl = "https://open.iot.10086.cn/";

/* @brief https post sample test */
char *posturl = "https://10.15.17.39:28443/server/echo";

/* @brief https put sample test */
char *puturl = "https://open.iot.10086.cn/";

/* @brief https delete sample test */
char *deleteurl = "https://open.iot.10086.cn/";
#else
/* @brief http get sample test */
char *geturl = "http://iot.10086.cn/";

/* @brief http post sample test */
char *posturl = "http://121.89.166.244/server/echo";

/* @brief http put sample test */
char *puturl = "http://iot.10086.cn/";

/* @brief http delete sample test */
char *deleteurl = "http://iot.10086.cn/";
#endif


/* @brief http request buffer */
#define BUF_SIZE 4096

#ifdef CONFIG_HTTP_SECURE
static const char *ca_cert = \
{
        \
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n" \
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n" \
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n" \
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n" \
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n" \
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n" \
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n" \
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n" \
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n" \
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n" \
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n" \
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n" \
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n" \
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n" \
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n" \
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n" \
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n" \
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n" \
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n" \
    "-----END CERTIFICATE-----"
};
#endif

static char *str_dup(char *s)
{
    char *url = NULL;
    int len = strlen(s) + 1;
    url = malloc(len);
    
    if(url == NULL)
    {
        return OS_NULL;
    }
    
    memcpy(url, s, len);

    return url;
}

static int http_get_sample_task_func(int argc, char **argv)
{
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    char *buf = NULL;
    char *URI = NULL;
    int ret = 0;
    
    if (argc == 1)
    {
        URI = str_dup(geturl);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else if (argc == 2)
    {
        URI = str_dup(argv[1]);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else
    {
        LOG_E(HTTP_TAG, "http_client_get_test [URI]  http client GET request test.\n");
        return OS_ERROR;
    }
    
    buf = malloc(BUF_SIZE);
    if (buf == NULL) 
    {
        LOG_E(HTTP_TAG, "Malloc failed.");
        return OS_ERROR;
    }

#ifdef CONFIG_HTTP_SECURE
        client.client_cert = ca_cert;
        client.client_cert_len = strlen(ca_cert);
#endif
    memset(buf, 0, BUF_SIZE);
    /* Sets a buffer to store the result */
    client_data.response_buf = buf;
    /* Sets the buffer size */
    client_data.response_buf_len = BUF_SIZE;
    //ret = http_client_get(&client, (const char *)URI, &client_data);
    
    ret = http_client_get(&client, (const char *)URI, &client_data);
    //OS_ASSERT(0 <= ret);

    if( 0 <= ret ) 
    {
        //LOG_I(HTTP_TAG, "http_get_sample: Data received:\r\n %s\r\n",  buf);
        os_kprintf("http_get_sample: Data received: %s\r\n",  buf);
    }
    else 
    {
        LOG_E(HTTP_TAG, "http client get test failed and ret = %d errno=%d \n", ret, errno);
    }

    if(buf != NULL)
    {
        free(buf);
        client_data.response_buf = NULL;
    }
    
    if (URI != NULL)
    {
        free(URI);
        URI = NULL;
    }
    
    return OS_EOK;
}


static int http_post_sample_task_func(int argc, char **argv)
{
    char *content_type = "text/csv";
    char *post_data = "OneOS is committed to serve the Internet of things industry!";
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    char *buf = NULL;
    int ret;
    char *URI = NULL;

    if (argc == 1)
    {
        URI = str_dup(posturl);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else if (argc == 2)
    {
        URI = str_dup(argv[1]);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else
    {
        LOG_E(HTTP_TAG, "http_client_post_test [URI]  - http client GET request test.\n");
        return OS_ERROR;
    }
    
    buf = malloc(BUF_SIZE);
    if (buf == NULL) 
    {
        LOG_E(HTTP_TAG, "Malloc failed.");
        return OS_ERROR;
    }
    memset(buf, 0, BUF_SIZE);
    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.post_buf = post_data;
    client_data.post_buf_len = strlen(post_data);
    client_data.post_content_type = content_type;
    ret = http_client_post(&client, (const char *)URI, &client_data);
    //OS_ASSERT(0 <= ret);

    if( 0 <= ret )
    {
        volatile int code;

        //LOG_I(HTTP_TAG, "Data received: %s \r\n", client_data.response_buf);
        os_kprintf("Data received: %s \r\n", client_data.response_buf);
        code = http_client_get_response_code(&client);
        LOG_D(HTTP_TAG, "Response code: %d \r\n", code);
    }
    else
    {
        LOG_E(HTTP_TAG, "http client post test failed and ret=%d errno=%d \n", ret, errno);
    }

    if(buf != NULL)
    {
        free(buf);
        client_data.response_buf = NULL;
    }

    if(URI != NULL)
    {
        free(URI);
        URI = NULL;
    }
    return OS_EOK;
}

static int http_put_sample_task_func(int argc, char **argv)
{
    char *content_type = "text/csv";
    char *put_data = "1,,I am string!";
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    char *buf = NULL;
    int ret = 0;
    char *URI = NULL;
    
    if (argc == 1)
    {
        URI = str_dup(puturl);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else if (argc == 2)
    {
        URI = str_dup(argv[1]);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else
    {
        LOG_E(HTTP_TAG, "http_client_put_test [URI]  - http client PUT request test.\n");
        return OS_ERROR;
    }

    buf = malloc(BUF_SIZE);
    if (buf == NULL) 
    {
        LOG_E(HTTP_TAG, "Malloc failed.");
        return -1;
    }
    memset(buf, 0, BUF_SIZE);
    /* Sets a buffer to store the result */
    client_data.response_buf = buf;
    /* Sets the buffer size */
    client_data.response_buf_len = BUF_SIZE;
    /* Sets the user data to be put */
    client_data.post_buf = put_data;
    /* Sets the put data length */
    client_data.post_buf_len = strlen(put_data);
    /* Sets the content type */
    client_data.post_content_type = content_type;
    ret = http_client_put(&client, URI, &client_data);
    //OS_ASSERT( 0 <= ret );
    if( 0 <= ret ) 
    {
        volatile int code;

        //LOG_I(HTTP_TAG, "Data received: %s \r\n", client_data.response_buf);
        os_kprintf("Data received: %s \r\n", client_data.response_buf);
        code = http_client_get_response_code(&client);
        LOG_D(HTTP_TAG, "Response code: %d \r\n", code);
    }
    else 
    {
        LOG_E(HTTP_TAG, "http client put test failed and ret=%d errno=%d \n", ret, errno);
    }
    
    free(buf);
    return ret;
}

static int http_delete_sample_task_func(int argc, char **argv)
{
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    char *buf = NULL;
    int ret;

    char *URI = NULL;
    
    if (argc == 1)
    {
        URI = str_dup(deleteurl);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else if (argc == 2)
    {
        URI = str_dup(argv[1]);
        if(URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
    }
    else
    {
        LOG_E(HTTP_TAG, "http_client_delete_test [URI]  - http client DELETE request test.\n");
        return OS_ERROR;
    }

    buf = malloc(BUF_SIZE);
    if (buf == NULL)
    {
        LOG_E(HTTP_TAG, "Malloc failed.");
        return -1;
    }
    memset(buf, 0, BUF_SIZE);
    /* Sets a buffer to store the result */
    client_data.response_buf = buf;
    /* Sets the buffer size */
    client_data.response_buf_len = BUF_SIZE;
    ret = http_client_delete(&client, URI, &client_data);
    //OS_ASSERT( 0 <= ret );
    if( 0 <= ret ) 
    {
        //LOG_I(HTTP_TAG, "Data received: %s \r\n", client_data.response_buf);
        os_kprintf("Data received: %s \r\n", client_data.response_buf);
    }
    else 
    {
        LOG_E(HTTP_TAG, "http client delete test failed and ret = %d errno=%d \r\n", ret, errno);
    }
    
    free(buf);
    return ret;
}
#ifdef CONFIG_HTTP_DOWNLOAD
static int http_get_file_task_func(int argc, char **argv)
{
    http_client_t client = {0};
    http_client_data_t client_data = {0};
    char *FILE_NAME = NULL;
    char *URI = NULL;
    volatile int ret = 0;

    if (argc < 3)
    {
        LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
        return OS_ERROR;
    }
    else if (argc == 3)
    {
        URI = str_dup(argv[1]);
        if (URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create URI buffer.\n");
            return OS_ERROR;
        }
        
        FILE_NAME = str_dup(argv[2]);
        if (URI == NULL)
        {
            LOG_E(HTTP_TAG, "no memory for create FILE_NAME buffer.\n");
            return OS_ERROR;
        }
    }

    ret = http_client_get_file(&client, (const char *)URI, &client_data, (const char *)FILE_NAME);

    if (FILE_NAME != NULL)
    {
        free(FILE_NAME);
    }

    if (URI != NULL)
    {
        free(URI);
        URI = NULL;
    }

    return OS_EOK;
}
#endif

#ifdef OS_USING_SHELL
#include <shell.h>


SH_CMD_EXPORT(http_get_sample, http_get_sample_task_func, "start http get sample");
SH_CMD_EXPORT(http_post_sample, http_post_sample_task_func, "start http post sample");
SH_CMD_EXPORT(http_put_sample, http_put_sample_task_func, "start http put sample");
SH_CMD_EXPORT(http_delete_sample, http_delete_sample_task_func, "start http delete sample");
#ifdef CONFIG_HTTP_DOWNLOAD
SH_CMD_EXPORT(http_get_file_sample, http_get_file_task_func, "start http get tts audio file");
#endif
#endif

#endif

