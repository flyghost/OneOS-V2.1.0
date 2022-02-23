/**
 * @file http_application_api.h
 * http API header file.
 *
 * @version   V1.0
 * @date      2020-08-01
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 */


#ifndef HTTP_APPLICATION_API
#define HTTP_APPLICATION_API

#include "http.h"
#include "http_client.h"


/**
 * This function executes a GET request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_get(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a POST request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be posted.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_post(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a PUT request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be put.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_put(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a HEAD request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_head(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a DELETE request on a given URL. It blocks until completion.
 * @param[in] client               client is a pointer to the #httpclient_t.
 * @param[in] url                  url is the URL to run the request.
 * @param[in, out] client_data client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_delete(http_client_t *client, const char *url, http_client_data_t *client_data);

#ifdef CONFIG_HTTP_DOWNLOAD

HTTP_RESULT_CODE http_client_get_file(http_client_t *client, const char *url, http_client_data_t *client_data, const char *filename);

int http_tcp_recv_file_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len, const char *filename);

int wav_player(char *wav_file);
#endif

#endif
