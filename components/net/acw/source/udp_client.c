#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <os_errno.h>

#include "acw_prot_common.h"
#include "acw_debug.h"
#include "udp_client.h"

/*
 * udp_client_init:
 *      UDP client initialize
 *      @server_ip: server ip address
 *      @server_port: server port
 *
 * Return:
 *      -1			-       udp client initialize failed
 *      other		-       udp client initialize succeed
 */
int udp_client_init(ip_addr_t s_addr, os_uint16_t s_port)
{
    struct sockaddr_in servaddr;
	int socket_fd;
	int do_err;

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        ACW_PRINT_E("Socket error, errno=%d", socket_fd);
        return OS_ERROR;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = ip_2_ip4(&s_addr)->addr;//inet_addr(server_ip);
    servaddr.sin_port = htons(s_port);
 
    do_err = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (do_err < 0)
    {
        ACW_PRINT_E("connect skfd[%d] error, errno=%d", socket_fd, do_err);
        closesocket(socket_fd);
        return OS_ERROR;
    }

	return socket_fd;
}
