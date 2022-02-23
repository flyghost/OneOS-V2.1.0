#include <sys/time.h>
#include "ams_net.h"
#include "core/ams_core.h"

#define AMS_SERVER_PORT     28765

static int g_ams_main_socket = -1;
static int g_create = 0;
int ams_close_socket(int socket)
{
#ifdef AMS_USING_PLATFORM
    ams_log("------create times: %d." , --g_create);
    return closesocket(socket);
#else
    return 0;
#endif
}

int ams_get_main_socket(void)
{
    return g_ams_main_socket;
}

void ams_set_main_socket(int sock)
{
    g_ams_main_socket = sock;
    return;
}

void ams_show_netinfo(void)
{
#ifdef AMS_USING_PLATFORM
    os_kprintf("Network information[Platform IP: %s, Port: %d, socket: %d]\r\n", 
        AMS_PLARFORM_IP_ADDR, AMS_SERVER_PORT, g_ams_main_socket);
#endif
}

int ams_send(int sock, char *msg, int msg_len)
{
#ifdef AMS_USING_PLATFORM
    return send(sock, msg, msg_len, 0);
#else
    return 0;
#endif
}

int ams_recv(int sock, char *msg, int msg_len)
{
#ifdef AMS_USING_PLATFORM
    return recv(sock, msg, msg_len, 0);
#else
    return 0;
#endif
}

int ams_connet_platform(int *conn_sock, uint32_t timeout, const char *saddr, int port)
{
#ifdef AMS_USING_PLATFORM
    struct hostent *host = NULL;
    struct sockaddr_in server_addr = {0};
    struct timeval recv_time = {
        .tv_sec = timeout, 
        .tv_usec = 0
    };
    
    *conn_sock = socket(AMS_SOCK_DOMAIN_V4, AMS_SOCK_PROT_TCP, 0);
    if (*conn_sock < 0)
    {
        ams_log("Create sock failed.");
        return AMS_ERROR;
    }
    ams_log("++++++create times: %d." , ++g_create);
    (void)setsockopt(*conn_sock, SOL_SOCKET, SO_RCVTIMEO, &recv_time, sizeof(recv_time));

    if (saddr == NULL)
    {
        host = gethostbyname(AMS_PLARFORM_IP_ADDR);
        server_addr.sin_port = htons(AMS_SERVER_PORT);
    }
    else
    {
        host = gethostbyname(saddr);
        server_addr.sin_port = htons(port);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    if (connect(*conn_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0)
    {
        closesocket(*conn_sock);
        return AMS_ERROR;
    }
#endif
    return AMS_OK;
}


