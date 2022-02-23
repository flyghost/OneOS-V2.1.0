/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License) you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        edp_sample.c
 *
 * @brief       support connect, push data, save data, cmd response, heartbeat, disconnect
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <shell.h>
#include <oneos_config.h>

#include "edp_kit.h"
#ifdef OS_USING_ONENET_EDP_CRYPT
#include "mbedtls/aes.h"
#include "mbedtls/bignum.h"
#include "rsa.h"
#include "edp_enc.h"
#endif

#include <netdb.h>
#include <sys/socket.h>
#include <sys/errno.h>

#define DBG_EXT_TAG "edp.sample"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <dlog.h>

/*------------------------------error code-----------------------------------*/
#define ERR_CREATE_SOCKET -1
#define ERR_HOSTBYNAME    -2
#define ERR_CONNECT       -3
#define ERR_SEND          -4
#define ERR_TIMEOUT       -5
#define ERR_RECV          -6
/*-------------------unify linux and windows socket api-----------------------*/
#ifndef htonll
#ifdef _BIG_ENDIAN
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ((((uint64)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64)ntohl(x)) << 32) + ntohl(x >> 32))
#endif
#endif

#ifdef OS_USING_ONENET_EDP_CRYPT
static os_sem_t           *edp_connect_sem = NULL;
extern mbedtls_rsa_context g_rsa;
#endif

#ifdef OS_USING_ONENET_EDP_CRYPT
#if SHELL_TASK_STACK_SIZE < 4096
#error "SHELL_TASK_STACK_SIZE need more than 4096 bytes if sample use encrypt in shell"
#endif
#endif

//extern time_t ntp_sync_to_rtc(const char *host_name);

static void hexdump(const unsigned char *buf, uint32 num)
{
    uint32 i = 0;
    for (; i < num; i++)
    {
        os_kprintf("%02X ", buf[i]);
        if ((i + 1) % 8 == 0)
            os_kprintf("\r\n");
    }
    os_kprintf("\r\n");
}

/**
 ***********************************************************************************************************************
 * @brief       This function establish tcp connection with the server
 *
 * @param[in]   addr        server address
 * @param[in]   portno      server port
 *
 * @return      Return sockfd
 * @retval      > 0         sockfd.
 * @retval      <=0         failed.
 ***********************************************************************************************************************
 */
int32 Open(const char *addr, int16 portno)
{
    int32              sockfd;
    struct sockaddr_in serv_addr;
    struct hostent    *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG_E(DBG_EXT_TAG,"ERROR opening socket");
        return ERR_CREATE_SOCKET;
    }
    server = (struct hostent *)gethostbyname(addr);
    if (server == NULL)
    {
        LOG_E(DBG_EXT_TAG,"ERROR, no such host");
        return ERR_HOSTBYNAME;
    }
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        LOG_E(DBG_EXT_TAG,"ERROR connecting");
        return ERR_CONNECT;
    }
#ifdef _DEBUG
    LOG_EXT_D("[%s] connect to server %s:%d succ!...", __func__, addr, portno);
#endif
    return sockfd;
}

/**
 ***********************************************************************************************************************
 * @brief       This function send data by socket interface
 *
 * @param[in]   sockfd        socket descriptor
 * @param[in]   buffer        the pointer of send buffer
 * @param[in]   len           send data length
 *
 * @return      Return send length
 * @retval      > 0           send data length.
 * @retval      <=0           send data failed.
 ***********************************************************************************************************************
 */
int32 DoSend(int32 sockfd, const char *buffer, uint32 len)
{
    int32 total = 0;
    int32 n     = 0;
    while (len != total)
    {
        /* try to send len - total byte data */
        n = send(sockfd, buffer + total, len - total, 0);
        if (n <= 0)
        {
            fprintf(stderr, "ERROR writing to socket\n");
            return n;
        }
        /* successfully send n byte data */
        total += n;
    }
    /* print send data */
    hexdump((const unsigned char *)buffer, len);
    return total;
}

/**
 ***********************************************************************************************************************
 * @brief       This thread recv data from server and parse the packet
 *
 * @param[in]   arg             the pointer of sockfd
 *
 * @return      void
 ***********************************************************************************************************************
 */
int  g_tostop = 0;
void recv_thread_func(void *arg)
{
    int          sockfd = *(int *)arg;
    int          n; 
    int          rtn;
    uint8        mtype;
    SaveDataType jsonorbin;
    char         buffer[1024];
    RecvBuffer  *recv_buf = NewBuffer();
    EdpPacket   *pkg      = NULL;

    char  *src_devid;
    char  *push_data;
    uint32 push_datalen;

    cJSON         *desc_json;
    char          *desc_json_str;
    char          *save_bin;
    uint32         save_binlen;
    unsigned short msg_id;
    unsigned char  save_date_ret;

    char      *cmdid;
    uint16     cmdid_len;
    char      *cmd_req;
    uint32     cmd_req_len;
    EdpPacket *send_pkg;
    char      *ds_id;
    double     dValue = 0;

    char    *simple_str   = NULL;
    char     cmd_resp[]   = "ok";
    unsigned cmd_resp_len = 0;

    DataTime stTime = {0};

    FloatDPS *float_data        = NULL;
    char      float2str_buf[16] = {0};
    int       count             = 0;
    int       i                 = 0;

    struct UpdateInfoList *up_info = NULL;
    struct timeval         tv;

    tv.tv_sec  = 0;
    tv.tv_usec = 500 * 1000;

#ifdef _DEBUG
    LOG_EXT_D("[%s] recv thread start ...", __func__);
#endif

    while (g_tostop == 0)
    {
        /* try to recv 1024 bytes data */
        memset(buffer, 0x00, sizeof(buffer));
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
        n = recv(sockfd, buffer, 1024, 0);
        if (-1 == n)
        {
            continue;
        }
        else if (0 == n)
        {
            break;
        }
        LOG_I(DBG_EXT_TAG,"recv from server, bytes: %d", n);
        /* print recv data */
        hexdump((const unsigned char *)buffer, n);
        /* successfully recv n byte data */
        WriteBytes(recv_buf, buffer, n);
        while (1)
        {
            /* get a completed EDP packet */
            if ((pkg = GetEdpPacket(recv_buf)) == 0)
            {
                LOG_I(DBG_EXT_TAG,"need more bytes...");
                break;
            }
            /* get the EDP packet message type */
            mtype = EdpPacketType(pkg);
#ifdef OS_USING_ONENET_EDP_CRYPT
            if (mtype != ENCRYPTRESP)
            {
                rtn = SymmDecrypt(pkg);
                if (-1 == rtn)
                {
                    LOG_E(DBG_EXT_TAG,"Decrypt error...");
                    break;
                }
            }
#endif
            /* parse the EDP packet according to message type */
            switch (mtype)
            {
#ifdef OS_USING_ONENET_EDP_CRYPT
            case ENCRYPTRESP:
                rtn = UnpackEncryptResp(pkg);
                LOG_I(DBG_EXT_TAG,"recv encrypt resp, rtn: %d", rtn);

                os_sem_post(edp_connect_sem);
                break;
#endif
            case CONNRESP:
                rtn = UnpackConnectResp(pkg);
                LOG_I(DBG_EXT_TAG,"recv connect resp, rtn: %d", rtn);
                break;
            case CLOSECON:
                rtn = UnpackConnectClose(pkg);
                LOG_I(DBG_EXT_TAG,"recv connect close, rtn: %d", rtn);
                g_tostop = 1;
                break;
            case PUSHDATA:
                UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                LOG_I(DBG_EXT_TAG,"recv push data, src_devid: %s, push_data: %.*s, len: %d",
                          src_devid,
                          push_datalen,
                          push_data,
                          push_datalen);
                free(src_devid);
                free(push_data);
                break;
            case UPDATERESP:
                UnpackUpdateResp(pkg, &up_info);
                while (up_info)
                {
                    LOG_I(DBG_EXT_TAG,"name = %s", up_info->name);
                    LOG_I(DBG_EXT_TAG,"version = %s", up_info->version);
                    LOG_I(DBG_EXT_TAG,"url = %s\nmd5 = ", up_info->url);
                    for (i = 0; i < 32; ++i)
                    {
                        LOG_I(DBG_EXT_TAG,"%c", (char)up_info->md5[i]);
                    }
                    LOG_I(DBG_EXT_TAG,"\n");
                    up_info = up_info->next;
                }
                FreeUpdateInfolist(up_info);
                break;

            case SAVEDATA:
                if (UnpackSavedata(pkg, &src_devid, (uint8*)&jsonorbin) == 0)
                {
                    if (jsonorbin == kTypeFullJson || jsonorbin == kTypeSimpleJsonWithoutTime ||
                        jsonorbin == kTypeSimpleJsonWithTime)
                    {
                        LOG_I(DBG_EXT_TAG,"json type is %d", jsonorbin);
                        /* UnpackSavedataJson(pkg, &save_json); */
                        /* save_json_str=cJSON_Print(save_json); */
                        /* LOG_I(DBG_EXT_TAG,"recv save data json, src_devid: %s, json: %s", */
                        /*     src_devid, save_json_str); */
                        /* cm_free(save_json_str); */
                        /* cJSON_Delete(save_json); */

                        /* UnpackSavedataInt(jsonorbin, pkg, &ds_id, &iValue); */
                        /* LOG_I(DBG_EXT_TAG,"ds_id = %s\nvalue= %d", ds_id, iValue); */

                        UnpackSavedataDouble(jsonorbin, pkg, &ds_id, &dValue);
                        sprintf(float2str_buf, "%f", dValue);
                        LOG_I(DBG_EXT_TAG,"ds_id = %s\r\nvalue = %s", ds_id, float2str_buf);

                        /* UnpackSavedataString(jsonorbin, pkg, &ds_id, &cValue); */
                        /* LOG_I(DBG_EXT_TAG,"ds_id = %s\nvalue = %s", ds_id, cValue); */
                        /* cm_free(cValue); */

                        free(ds_id);
                    }
                    else if (jsonorbin == kTypeBin)
                    {
                        UnpackSavedataBin(pkg, &desc_json, (uint8 **)&save_bin, &save_binlen);
                        desc_json_str = cJSON_Print(desc_json);
                        LOG_I(DBG_EXT_TAG,"recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d",
                                  src_devid,
                                  desc_json_str,
                                  save_bin,
                                  save_binlen);
                        free(desc_json_str);
                        cJSON_Delete(desc_json);
                        free(save_bin);
                    }
                    else if (jsonorbin == kTypeString)
                    {
                        UnpackSavedataSimpleString(pkg, &simple_str);

                        LOG_I(DBG_EXT_TAG,"%s", simple_str);
                        free(simple_str);
                    }
                    else if (jsonorbin == kTypeStringWithTime)
                    {
                        UnpackSavedataSimpleStringWithTime(pkg, &simple_str, &stTime);

                        LOG_I(DBG_EXT_TAG,"time:%u-%02d-%02d %02d-%02d-%02d\nstr val:%s",
                                  stTime.year,
                                  stTime.month,
                                  stTime.day,
                                  stTime.hour,
                                  stTime.minute,
                                  stTime.second,
                                  simple_str);
                        free(simple_str);
                    }
                    else if (jsonorbin == kTypeFloatWithTime)
                    {
                        if (UnpackSavedataFloatWithTime(pkg, &float_data, &count, &stTime))
                        {
                            LOG_I(DBG_EXT_TAG,"UnpackSavedataFloatWithTime failed!");
                        }

                        LOG_I(DBG_EXT_TAG,"read time:%u-%02d-%02d %02d-%02d-%02d",
                                  stTime.year,
                                  stTime.month,
                                  stTime.day,
                                  stTime.hour,
                                  stTime.minute,
                                  stTime.second);
                        LOG_I(DBG_EXT_TAG,"read float data count:%d, ptr:[%p]", count, float_data);

                        for (i = 0; i < count; ++i)
                        {
                            sprintf(float2str_buf, "%f", float_data[i].f_data);
                            LOG_I(DBG_EXT_TAG,"ds_id=%u,value=%s", float_data[i].ds_id, float2str_buf);
                        }

                        free(float_data);
                        float_data = NULL;
                    }
                    free(src_devid);
                }
                else
                {
                    LOG_I(DBG_EXT_TAG,"error");
                }
                break;
            case SAVEACK:
                UnpackSavedataAck(pkg, &msg_id, &save_date_ret);
                LOG_I(DBG_EXT_TAG,"save ack, msg_id = %d, ret = %d", msg_id, save_date_ret);
                break;
            case CMDREQ:
                if (UnpackCmdReq(pkg, &cmdid, &cmdid_len, &cmd_req, &cmd_req_len) == 0)
                {
                    LOG_I(DBG_EXT_TAG,"recv cmd request");
                    /*
                     * user processe and return according to own requirements. The response
                     * message body can be empty. It is assumed that two characters "OK" are
                     * returned here.
                     */
                    cmd_resp_len = strlen(cmd_resp);
                    send_pkg     = PacketCmdResp(cmdid, cmdid_len, cmd_resp, cmd_resp_len);
#ifdef OS_USING_ONENET_EDP_CRYPT
                    SymmEncrypt(send_pkg);
#endif
                    DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
                    DeleteBuffer(&send_pkg);

                    free(cmdid);
                    free(cmd_req);
                }
                break;
            case PINGRESP:
                UnpackPingResp(pkg);
                LOG_I(DBG_EXT_TAG,"recv ping resp");
                break;

            default:
                /* unsupported msg type */
                g_tostop = 1;
                LOG_I(DBG_EXT_TAG,"recv failed...");
                break;
            }

            if (pkg != NULL)
                DeleteBuffer(&pkg);
        }
    }

    if (recv_buf != NULL)
    {
        DeleteBuffer(&recv_buf);
    }

#ifdef _DEBUG
    LOG_EXT_D("[%s] recv thread end ...", __func__);
#endif
}

void usage()
{
    LOG_I(DBG_EXT_TAG,"edp usage:");
    LOG_I(DBG_EXT_TAG,"edp 0 ,send ping to server");
    LOG_I(DBG_EXT_TAG,"edp 1 <des_dev>,send puch data to server");
    LOG_I(DBG_EXT_TAG,"edp 2 <des_dev>,send savedata full json to server");
    LOG_I(DBG_EXT_TAG,"edp 3 <des_dev>,send savedata bin to server");
    LOG_I(DBG_EXT_TAG,"edp 4 <des_dev>,send savedata simple json without time to server");
    LOG_I(DBG_EXT_TAG,"edp 5 <des_dev>,send savedata simple json with time to server");
    LOG_I(DBG_EXT_TAG,"edp 6 <des_dev>,send string split by simicolon");
    LOG_I(DBG_EXT_TAG,"edp 7 <des_dev>,send string with time to server");
    LOG_I(DBG_EXT_TAG,"edp 8 <des_dev>,send float with time to server");
    return;
}

os_task_t *edp_recv_thread = NULL;
void edp_start(int argc, char *argv[])
{
    char       opt;
    static int sockfd      = -1;
    EdpPacket *send_pkg    = NULL;
    char       push_data[] = {'a', 'b', 'c'};
    char       text2[]     = "{\"ds_id\": \"temperature\"}";
    cJSON     *desc_json;
    char       save_bin[]  = {'c', 'b', 'a'};

    char                  *ip           = NULL;
    char                  *port         = NULL;
    char                  *src_dev      = NULL;
    char                  *dst_dev      = NULL;
    char                  *src_api_key  = NULL;
    DataTime               save_time    = {0};
    char                   send_str[]   = ",;temperature,2015-03-22 22:31:12,22.5;humidity,35%;pm2.5,89;1001";
    FloatDPS               send_float[] = {{1, 0.5}, {2, 0.8}, {3, -0.5}};
    SaveDataType           data_type;
#ifdef OS_USING_ONENET_EDP_CRYPT
    int stack_size = 8192;
#else
    int stack_size = 4096;
#endif

//    ntp_sync_to_rtc(OS_NULL);
    time_t now_time     = time(OS_NULL);
    struct tm *now      = localtime(&now_time);
    save_time.day       = now->tm_mday;
    save_time.month     = now->tm_mon + 1;
    save_time.year      = now->tm_year + 1900;
    save_time.second    = now->tm_sec;
    save_time.minute    = now->tm_min;
    save_time.hour      = now->tm_hour;

    opt = argv[1][0];

    switch (opt)
    {
    case 'c':
    {
        ip          = argv[2];
        port        = argv[3];
        src_dev     = argv[4];
        src_api_key = argv[5];

        if (!ip || !port || !src_dev || !src_api_key)
        {
            usage();
            return;
        }

        /* create a socket and connect to server */
        sockfd = Open((const char *)ip, atoi(port));
        if (sockfd < 0)
        {
            return;
        }
        /* create a recv thread */
        g_tostop = 0;
        edp_recv_thread = os_task_create("edp_recv", recv_thread_func, &sockfd, stack_size, OS_TASK_PRIORITY_MAX / 2);
        os_task_startup(edp_recv_thread);
#ifdef OS_USING_ONENET_EDP_CRYPT
        edp_connect_sem = os_sem_create("edp_conn", 1, 1);
        os_sem_wait(edp_connect_sem, OS_WAIT_FOREVER);
        send_pkg = PacketEncryptReq(kTypeAes);
        /* send encryption request to onenet cloud */
        LOG_I(DBG_EXT_TAG,"send encrypt to server, bytes: %d", send_pkg->_write_pos);
        DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
        DeleteBuffer(&send_pkg);

        /* wait encryption response */
        if (0 != os_sem_wait(edp_connect_sem, 20 * OS_TICK_PER_SECOND))
        {
            os_sem_destroy(edp_connect_sem);
            LOG_I(DBG_EXT_TAG,"wait encrypt response timeout");
            return;
        }
        /* delete sem */
        os_sem_destroy(edp_connect_sem);
#endif
        /* connect to onenet cloud */
        send_pkg = PacketConnect1(src_dev, src_api_key);
        LOG_I(DBG_EXT_TAG,"send connect to server, bytes: %d", send_pkg->_write_pos);

        break;
    }

    case 'e':
    {
        g_tostop = 1;
        os_task_msleep(1500);
        if (sockfd > 0)
        {
            closesocket(sockfd);
        }    
#ifdef OS_USING_ONENET_EDP_CRYPT
        if (g_rsa.len != 0)
        {
            mbedtls_rsa_free(&g_rsa);
        }
#endif
        return;
    }

    case '0':
        send_pkg = PacketPing();
        break;

    case '1':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }
        send_pkg = PacketPushdata(dst_dev, push_data, sizeof(push_data));
        break;

    case '2':
    case '4':
    case '5':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }
        
        if (opt == '2')
        {
            data_type = kTypeFullJson;
        }
        if (opt == '4')
        {
            data_type = kTypeSimpleJsonWithoutTime;
        }
        if (opt == '5')
        {
            data_type = kTypeSimpleJsonWithTime;
        }

        send_pkg = PacketSavedataInt(data_type, dst_dev, "a", 1234, 0, 0);
        /* send_pkg = PacketSavedataDouble(data_type, dst_dev, "b", 28.6547, 0, 0); */
        /* send_pkg = PacketSavedataString(data_type, dst_dev, "c", "test12345678", 0, 1); */
        break;

    case '3':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }

        desc_json = cJSON_Parse(text2);
        send_pkg  = PacketSavedataBin(dst_dev, desc_json, (const uint8 *)save_bin, sizeof(save_bin), 0);
        break;

    case '6':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }

        send_pkg = PacketSavedataSimpleString(dst_dev, send_str, 0);
        break;

    case '7':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }

        if (save_time.year > 0)
        {
            send_pkg = PacketSavedataSimpleStringWithTime(dst_dev, send_str, &save_time, 0);
        }
        else
        {
            send_pkg = PacketSavedataSimpleStringWithTime(dst_dev, send_str, NULL, 0);
        }
        break;

    case '8':
        if (3 == argc)
        {
            dst_dev = argv[2];
        }

        if (save_time.year > 0)
        {
            send_pkg = PackSavedataFloatWithTime(dst_dev, send_float, 3, &save_time, 0);
        }
        else
        {
            send_pkg = PackSavedataFloatWithTime(dst_dev, send_float, 3, NULL, 0);
        }
        break;

    case 'h':
        usage();
        return;
    default:
        LOG_I(DBG_EXT_TAG,"input error, please try again");
        return;
    }
#ifdef OS_USING_ONENET_EDP_CRYPT
    SymmEncrypt(send_pkg);
#endif
    if (OS_NULL != send_pkg)
    {
        DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
        DeleteBuffer(&send_pkg);
    }

    return;
}

SH_CMD_EXPORT(edp, edp_start, "use <edp h> list the information of all network interfaces");
