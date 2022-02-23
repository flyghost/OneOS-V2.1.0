/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * \@file        http_application_api.c
 *
 * \@brief       http api implemnt 
 *
 * \@details     provide interface for up application  
 *
 * \@revision    v1.0
 * Date         Author          Notes
 * 2020-8-12   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "http_application_api.h"
#include "http_client.h"
#include "http_form_data.h"


HTTP_RESULT_CODE http_request_common(http_client_t *client, const char *url, int method, http_client_data_t *client_data)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    /* reset httpclient redirect flag */
    client_data->is_redirected = 0;
    
    ret = http_client_conn(client, url);

    if(!ret)
    {
        ret = http_client_send(client, url, method, client_data);
        if(!ret)
        {
            ret = (HTTP_RESULT_CODE)http_client_recv(client, client_data);
        }
    }
    
    /* Don't reset form data when got a redirected response */
    if(client_data->is_redirected == 0) 
    {
        httpclient_clear_form_data(client_data);
    }

    ret = (HTTP_RESULT_CODE)http_client_close(client);
    
    return ret;
}

HTTP_RESULT_CODE http_client_get(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_GET, client_data);

    while((0 == ret) && (1 == client_data->is_redirected)) 
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_GET, client_data);
    }
    
    if(client_data->redirect_url != NULL) 
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_post(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_POST, client_data);

    while((0 == ret) && (1 == client_data->is_redirected)) 
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_POST, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_put(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_PUT, client_data);

    while((0 == ret) && (1 == client_data->is_redirected)) 
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_PUT, client_data);
    }

    if(client_data->redirect_url != NULL) 
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_head(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_HEAD, client_data);

    while((0 == ret) && (1 == client_data->is_redirected)) 
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_HEAD, client_data);
    }

    if(client_data->redirect_url != NULL) 
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_delete(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    return http_request_common(client, url, HTTP_DELETE, client_data);
}

#ifdef CONFIG_HTTP_DOWNLOAD
HTTP_RESULT_CODE http_client_get_file(http_client_t *client, const char *url, http_client_data_t *client_data, const char *filename)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;
    int reclen = 0;
    /* reset httpclient redirect flag */
    client_data->is_redirected = 0;

    char *buf = NULL;
    int timeout_ms = CONFIG_HTTP_RECV_TIMEOUT_MS;
    
    buf = malloc(HTTP_CLIENT_CHUNK_SIZE);

    ret = http_client_conn(client, url);

    if (!ret)
    {
        ret = http_client_send(client, url, HTTP_GET, client_data);
        if (!ret)
        {
            if (client->socket < 0)
            {
                LOG_E(HTTP_TAG, "Invalid socket fd %d!", client->socket);
                return (HTTP_RESULT_CODE)ret;
            }

            if (client->is_http)
            {
                ret = (HTTP_RESULT_CODE)http_tcp_recv_file_wrapper(client, buf,  HTTP_CLIENT_CHUNK_SIZE - 1, timeout_ms, &reclen, filename);
            }
            else
            {
                LOG_E(HTTP_TAG, "need http\n");
            }

            if (ret != HTTP_SUCCESS && ret != HTTP_ECLSD)
            {
                free(buf);
                return ret;
            }

            free(buf);
            LOG_D(HTTP_TAG, "httpclient_recv_data() result:%d, client:%p, reclen:%d\n", ret, client, reclen);
        }
    }

    http_client_close(client);

    return ret;
}
#endif

