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
 * \@file        lwm2m_sample_test.c
 *
 * \@brief       lwm2m  interface test
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-10-12   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_assert.h>
#include <oneos_config.h>
#include <sys/errno.h>
#include "lwm2m.h"
#include "os_errno.h"
#include "lwm2mclient.h"
#include "lwm2mdebug.h"
#include "liblwm2m.h"
#ifdef CONFIG_LWM2M_SECURE
#include "mbedconnection.h"
#else
#include "connection.h"
#endif
#include <sys/socket.h>
#include <signal.h>
#include <sys/select.h>


#include "dlog.h"
#define LWM2M_TAG "LWM2M_TAG"


#ifdef LWM2M_USING_SAMPLE

#define MAX_PACKET_SIZE 1024
//#define LWM2M_SERVER_ADDR_STR "192.168.1.102"

#ifndef MEMP_NUM_NETCONN
#define MEMP_NUM_NETCONN 8
#endif

static int g_quit = 0;

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    int sock;
#ifdef CONFIG_LWM2M_SECURE
		dtls_connection_t * connList;
		lwm2m_context_t * lwm2mH;
#else
		connection_t * connList;
#endif

    int addressFamily;
} client_data_t;

extern lwm2m_object_t * get_security_object(int serverId,
                                     const char* serverUri,
                                     char * bsPskId,
                                     char * psk,
                                     uint16_t pskLen,
                                     bool isBootstrap);
extern lwm2m_object_t * get_server_object(int serverId,
                                   const char* binding,
                                   int lifetime,
                                   bool storing);
extern void clean_security_object(lwm2m_object_t * objectP);



static void lwm2m_data_sample_test_func(void)
{
    int ret_i = -1;
    int size_i = 1;
    lwm2m_data_t * lwm2m_data = NULL;
    int64_t test_integer = 10;
    int64_t decode_integer = -1;
    char *test_uri_str = "/6/2/6";
    int test_uri_str_len = strlen(test_uri_str);
    lwm2m_uri_t uri;

    lwm2m_data = lwm2m_data_new(size_i);
    
    //OS_ASSERT( NULL != lwm2m_data );
    if (!lwm2m_data)
    {
        lwm2m_log(COMP_LOG_ERROR, "ERROR:lwm2m_data:NULL");
        return;
    }
	
    lwm2m_data_encode(LWM2M_CODEC_TYPE_INTEGER, &test_integer, 0, lwm2m_data);
    ret_i = lwm2m_data_decode(LWM2M_CODEC_TYPE_INTEGER, lwm2m_data, &decode_integer);
    //OS_ASSERT( 1 == ret_i && test_integer == decode_integer);
    if ((1 != ret_i) || (test_integer != decode_integer))
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR : DECODE FAILED! ret_i = %d, decode_integer = %lld test_integer = %lld",
				ret_i,decode_integer,test_integer);
		return;
	}
	
    ret_i = lwm2m_string_to_uri(test_uri_str, test_uri_str_len, &uri);
    //OS_ASSERT( test_uri_str_len == ret_i &&
   //               uri.objectId == 6 && 
    //              uri.instanceId == 2 && 
    //              uri.resourceId == 6 );
    if (test_uri_str_len != ret_i ||
					   6 != uri.objectId	||
					   2 != uri.instanceId  ||
					   6 != uri.resourceId)
    {
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M CHANGE STRING to URI FAILED!\n");
		return;
	}
					   
	lwm2m_log(COMP_LOG_INFO, "DEBUG:Lwm2m data sample test is success!");

    lwm2m_data_free(size_i, lwm2m_data);
    lwm2m_data = NULL;
}

#define OBJ_COUNT 3

static void lwm2m_sample_proc_test_func(void)
{
    int ret_i;
    lwm2m_context_t *ctx = NULL;
    lwm2m_object_t *objectArray[OBJ_COUNT] = { NULL };
    time_t timeout_sec = 5;
    lwm2m_object_t *testObject = NULL;
	
    char * endpointName = "testlwm2mclient";

    ctx = lwm2m_init(NULL);
   // OS_ASSERT( NULL != ctx );
	
    if (NULL == ctx) 
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M Context Init Failed!\n");
        return;
    }

    objectArray[0] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    //OS_ASSERT( NULL != objectArray[0] );
    if (NULL == objectArray[0])
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M Malloc objectArray[0] Memory Failed\n");
        goto exit;
    }
    memset(objectArray[0], 0, sizeof(lwm2m_object_t));
    objectArray[0]->objID = LWM2M_SECURITY_OBJECT_ID;

    objectArray[1] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
   // OS_ASSERT( NULL != objectArray[1] );
	
    if (NULL == objectArray[1])
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M Malloc objectArray[1] Memory Failed\n");
		goto exit;
    }
    memset(objectArray[1], 0, sizeof(lwm2m_object_t));
    objectArray[1]->objID = LWM2M_SERVER_OBJECT_ID;

    objectArray[2] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    //OS_ASSERT( NULL != objectArray[2] );
	
    if (NULL == objectArray[1]) 
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M Malloc objectArray[2] Memory Failed\n");
        goto exit;
    }
    memset(objectArray[2], 0, sizeof(lwm2m_object_t));
    objectArray[2]->objID = LWM2M_DEVICE_OBJECT_ID;

	#ifdef LWM2M_CLIENT_MODE
	
    ret_i = lwm2m_configure(ctx, endpointName, NULL, NULL, OBJ_COUNT, objectArray);
   // OS_ASSERT( ret_i == 0);
    if (0 != ret_i) 
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M lwm2m_configure Failed!\n");
        goto exit;
    }
	#endif
	
    testObject = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
    //OS_ASSERT( NULL != testObject );
	
    if (NULL == testObject) 
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M testObject Malloc Memory Failed!\n");
        goto exit;
    }

    memset(testObject, 0, sizeof(lwm2m_object_t));
    testObject->objID = TEST_OBJECT_ID;

	#ifdef LWM2M_CLIENT_MODE
    ret_i = lwm2m_add_object(ctx, testObject);
   // OS_ASSERT( ret_i == 0 );
	
    if (0 != ret_i) 
	{
        goto exit;
    }

    ret_i = lwm2m_remove_object(ctx, TEST_OBJECT_ID);
    //OS_ASSERT( ret_i == 0 );
	
    if (0 != ret_i) 
	{
		lwm2m_log(COMP_LOG_ERROR, "ERROR:LWM2M lwm2m_remove_object Ret Failed!\n");
        goto exit;
    }
	#endif
	
    ret_i = lwm2m_step(ctx, &timeout_sec);
    // OS_ASSERT(COAP_503_SERVICE_UNAVAILABLE == ret_i);

	#ifdef LWM2M_CLIENT_MODE
   	if (COAP_503_SERVICE_UNAVAILABLE != ret_i)
   	{
   		lwm2m_log(COMP_LOG_ERROR, "ERROR:lwm2m_step Ret Failed!");
		goto exit;
  	}
	
	lwm2m_log(COMP_LOG_INFO, "DEBUG:Lwm2m proc test sample run success!");
	#endif
	
exit:
    lwm2m_free(objectArray[0]);
    lwm2m_free(objectArray[1]);
    lwm2m_free(objectArray[2]);
	lwm2m_free(testObject);

    lwm2m_deinit(ctx);
 }

#define OBJ_COUNT_NUM 4
void lwm2m_sample_connect_server_test_func(void *param)
{
    client_data_t data;
    lwm2m_context_t * lwm2mH = NULL;
    lwm2m_object_t * objArray[OBJ_COUNT_NUM];

    const char * localPort = "56830";
    char * name = "testlwm2mclient";
	
    const char * server = LWM2M_SERVER_ADDR_STR;
    const char * serverPort = LWM2M_SERVER_PORT_STR;

    int result;

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = AF_INET;

    /*
     *This call an internal function that create an IPv6 socket on the port 5683.
     */
	lwm2m_log(COMP_LOG_DEBUG, "Trying to bind LWM2M Client to port %s\r\n", localPort);
    data.sock = create_socket(localPort, data.addressFamily);
    if (data.sock < 0)
    {
        lwm2m_log(COMP_LOG_ERROR, "Failed to open socket: %d %s\r\n", errno, strerror(errno));
        return;
    }

    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
    int serverId = 123;
    char serverUri[50];
#ifdef CONFIG_LWM2M_SECURE
    sprintf (serverUri, "coaps://%s:%s", server, serverPort);
#else
    sprintf (serverUri, "coap://%s:%s", server, serverPort);
#endif
    bool isBootstrap = false;
    objArray[0] = get_security_object(serverId,
                                     serverUri,
                                     NULL,
                                     NULL,
                                     0,
                                     isBootstrap);
	
    if (NULL == objArray[0])
    {
        lwm2m_log(COMP_LOG_ERROR, "Failed to create security object\r\n");
        return;
    }
    data.securityObjP = objArray[0];
	
	const char* binding = "U";
	int lifetime = 300;
	bool storing = false;
    objArray[1] = get_server_object(serverId,
                                   binding,
                                   lifetime,
                                   storing);
    if (NULL == objArray[1])
    {
        lwm2m_log(COMP_LOG_ERROR, "Failed to create server object\r\n");
        return;
    }

    objArray[2] = get_object_device();
    if (NULL == objArray[2])
    {
        lwm2m_log(COMP_LOG_ERROR, "Failed to create Device object\r\n");
        return;
    }

    objArray[3] = get_test_object();
    if (NULL == objArray[3])
    {
        lwm2m_log(COMP_LOG_ERROR, "Failed to create Test object\r\n");
        return;
    }

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        lwm2m_log(COMP_LOG_ERROR, "lwm2m_init() failed\r\n");
        return;
    }

#ifdef CONFIG_LWM2M_SECURE
		data.lwm2mH = lwm2mH;
#endif

    /*
     * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
     * the number of objects we will be passing through and the objects array
     */
    result = lwm2m_configure(lwm2mH, name, NULL, NULL, OBJ_COUNT_NUM, objArray);
    if (result != 0)
    {
        lwm2m_log(COMP_LOG_ERROR, "lwm2m_configure() failed: 0x%X\r\n", result);
        return;
    }

    /*
     * We catch Ctrl-C signal for a clean exit
     */
   // signal(SIGINT, SigIntHandler);

    //lwm2m_log(COMP_LOG_DEBUG, "LWM2M Client \"%s\" started on port %s.\r\nUse Ctrl-C to exit.\r\n\n", name, localPort);

    /*
     * We now enter in a while loop that will handle the communications from the server
     */
    while (0 == g_quit)
    {
        struct timeval tv;
        fd_set readfds;

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(data.sock, &readfds);

        /*
         * This function does two things:
         *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
         *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
         *    (eg. retransmission) and the time before the next operation
         */
        result = lwm2m_step(lwm2mH, (time_t*)&(tv.tv_sec));
        if (result == 0)
        {
            os_kprintf("Register The Server Succeed!\r\n");
        }
        else
        {
            lwm2m_log(COMP_LOG_ERROR, "lwm2m_step() failed: 0x%X\r\n", result);
            break;
        }

        /*
         * This part wait for an event on the socket until "tv" timed out (set
         * with the precedent function)
         */
        result = select(MEMP_NUM_NETCONN, &readfds, NULL, NULL, &tv);
        if (result < 0)
        {
            if (errno != EINTR)
            {
            	lwm2m_log(COMP_LOG_ERROR, "Error in select(): %d %s\r\n", errno, strerror(errno));
            }
        }
        else if (result > 0)
        {
            uint8_t buffer[MAX_PACKET_SIZE];
            int numBytes;

            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(data.sock, &readfds))
            {
                struct sockaddr_storage addr;
                socklen_t addrLen;

                addrLen = sizeof(addr);

                /*
                 * We retrieve the data received
                 */
                numBytes = recvfrom(data.sock, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);

                if (0 > numBytes)
                {
                    lwm2m_log(COMP_LOG_ERROR, "Error in recvfrom(): %d %s\r\n", errno, strerror(errno));
                }
                else if (0 < numBytes)
                {
#ifdef CONFIG_LWM2M_SECURE
					dtls_connection_t * connP;
#else
					connection_t * connP;
#endif
                    connP = connection_find(data.connList, &addr, addrLen);
                    if (connP != NULL)
                    {
                        /*
                         * Let liblwm2m respond to the query depending on the context
                         */
#ifdef CONFIG_LWM2M_SECURE
						int result = connection_handle_packet(connP, buffer, numBytes);
						if (0 != result)
						{
							 lwm2m_log(COMP_LOG_ERROR, "error handling message %d\n",result);
						}
#else
						lwm2m_handle_packet(lwm2mH, buffer, numBytes, connP);
#endif
                    }
                    else
                    {
                        /*
                         * This packet comes from an unknown peer
                         */
                        lwm2m_log(COMP_LOG_ERROR, "received bytes ignored!\r\n");
                    }
                }
            }
        }
    }

    /*
     * Finally when the loop is left, we unregister our client from it
     */
    lwm2m_deinit(lwm2mH);
    closesocket(data.sock);
    connection_free(data.connList);

    clean_security_object(objArray[0]);
    clean_server_object(objArray[1]);
    free_object_device(objArray[2]);
    free_test_object(objArray[3]);
    os_kprintf("Unregister The Server Succeed!\r\n");

    return;
}

static int lwm2m_connect_server_example_start(int argc, char *argv[])
{
    os_task_t *lwm2m_connect_test = os_task_create("lwm2m_connect_server_test", 
                                             lwm2m_sample_connect_server_test_func, 
                                             NULL, 
                                             4096 * 2, 
                                             OS_TASK_PRIORITY_MAX / 2);

    if(NULL == lwm2m_connect_test)
    {
        LOG_E(LWM2M_TAG, "create task error");  
        return -1;
    }
    else
    {    
        g_quit = 0;
        os_task_startup(lwm2m_connect_test);
        return 0;
    }
}

void lwm2m_sample_disconnect_server_test_func(void)
{
	if( 0 == g_quit)
	{
		//lwm2m_log(COMP_LOG_DEBUG, "Ready to disconnect the server about 60 seconds later!\r\n");
        os_kprintf("Ready to disconnect the server about 60 seconds later!\r\n");
		g_quit = 1;
	}

	return;
}



#ifdef OS_USING_SHELL
#include <shell.h>


SH_CMD_EXPORT(lwm2m_data_sample, lwm2m_data_sample_test_func, "start lwm2m data sample");
SH_CMD_EXPORT(lwm2m_proc_sample, lwm2m_sample_proc_test_func, "start lwm2m proc sample");
SH_CMD_EXPORT(lwm2m_connect_server_sample, lwm2m_connect_server_example_start, "start lwm2m connect server sample");
SH_CMD_EXPORT(lwm2m_disconnect_server_sample, lwm2m_sample_disconnect_server_test_func, "start lwm2m disconnect server sample");




#endif
#endif

