#ifndef __AMS_NET_H__
#define __AMS_NET_H__
#include"core/ams_core.h"
#ifdef AMS_USING_PLATFORM
#include <sys/socket.h>
#define AMS_SOCK_DOMAIN_V4      AF_INET
#define AMS_SOCK_DOMAIN_V6      AF_INET6
#define AMS_SOCK_DOMAIN_UNSPEC  AF_UNSPEC
#define AMS_SOCK_PROT_TCP       SOCK_STREAM
#define AMS_SOCK_PROT_UDP       SOCK_DGRAM
#define AMS_SOCK_PROT_RAW       SOCK_RAW
#endif

#define AMS_RECV_MSG_TIMEOUT_ONE_MINUTE     (60)

#define AMS_RECV_MSG_MAX_LEN                600



typedef enum AMS_SOCK_STAT {
    AMS_SOCK_STAT_NONE = 0,
    AMS_SOCK_STAT_IDLE,
    AMS_SOCK_STAT_CREATED,
    AMS_SOCK_STAT_CONNTED,
    AMS_SOCK_STAT_ERR,
}AMS_SOCK_STAT_T;


int ams_connet_platform(int *conn_sock, uint32_t timeout, const char *saddr, int port);

int ams_close_socket(int socket);

void ams_show_netinfo(void);

int ams_get_main_socket(void);

void ams_set_main_socket(int sock);

int ams_send(int sock, char *msg, int msg_len);
int ams_recv(int sock, char *msg, int msg_len);


#endif /* __AMS_NET_H__ */


