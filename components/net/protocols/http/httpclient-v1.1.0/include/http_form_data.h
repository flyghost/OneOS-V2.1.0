/**
 * @version   V1.0
 * @date      2020-08-01
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 */

#ifndef HTTP_FORM_DATA
#define HTTP_FORM_DATA

#include "http_client.h"

#define HTTP_DATA_SIZE   1500
#define FORM_DATA_MAXLEN 32
#define CLIENT_FORM_DATA_NUM  1

typedef struct formdata_node_t formdata_node_t;
struct formdata_node_t
{
    formdata_node_t *next;
    int   is_file;
    char  file_path[FORM_DATA_MAXLEN];
    char  *data;
    int   data_len;
};

typedef struct {
    int                is_used;
    formdata_node_t    *form_data;
    http_client_data_t  *client_data;
} formdata_info_t;

void httpclient_clear_form_data(http_client_data_t * client_data);
int httpclient_formdata_len(http_client_data_t *client_data);
int http_client_send_formdata(http_client_t *client, http_client_data_t *client_data);

#endif

