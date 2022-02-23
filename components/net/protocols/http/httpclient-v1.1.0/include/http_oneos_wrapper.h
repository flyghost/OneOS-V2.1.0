/**
 * @version   V1.0
 * @date      2020-08-01
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 */


#ifndef HTTP_CLIENT_WRAPPER_H
#define HTTP_CLIENT_WRAPPER_H

#include "http_client.h"
#include "oneos_config.h"
#include <os_util.h>


#include <dlog.h>


#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif


int http_tcp_conn_wrapper(http_client_t *client, const char *host);
int http_tcp_close_wrapper(http_client_t *client);
int http_tcp_send_wrapper(http_client_t *client, const char *data, int length);
int http_tcp_recv_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len);

#ifdef CONFIG_HTTP_SECURE
int http_ssl_conn_wrapper(http_client_t *client, const char *host);
int http_ssl_close_wrapper(http_client_t *client);
int http_ssl_send_wrapper(http_client_t *client, const char *data, size_t length);
int http_ssl_recv_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len);
#endif

#endif

