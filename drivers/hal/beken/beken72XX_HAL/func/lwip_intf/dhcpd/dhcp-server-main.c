/*
 *  Copyright (C) 2008-2015, Marvell International Ltd.
 *  All Rights Reserved.
 */

/** dhcp-server-main.c: CLI based APIs for the DHCP Server
 */
#include <string.h>
#include "rtos_pub.h"
#include "dhcp-priv.h"

static beken_thread_t dhcpd_thread;
static bool dhcpd_running;   

int dhcp_server_start(void *intrfc_handle)
{
	int ret;

	dhcp_d("DHCP server start request \r\n");

    dhcp_enable_nack_dns_server();
    
	if (dhcpd_running || dhcp_server_init(intrfc_handle))
	{
		return -1;
	}

	ret = rtos_create_thread(&dhcpd_thread, 
                 BEKEN_APPLICATION_PRIORITY, 
                 "dhcp-server", 
				(beken_thread_function_t)dhcp_server,
                #if !OSMALLOC_STATISTICAL
                1024*4, 
                #else 
				1024*2, 
                #endif
				0);
	if (ret) 
	{
		dhcp_free_allocations();
		return -1;
	}

	dhcpd_running = 1;
	return 0;
}

void dhcp_server_stop(void)
{
	dhcp_d("DHCP server stop request\r\n");
	if (dhcpd_running) 
	{
		if (dhcp_send_halt() != 0) 
		{
			dhcp_w("failed to send halt to DHCP thread\r\n");
			return;
		}

#ifndef __ONEOS_CONFIG_H__ /* OneOS系统中上一步收到halt后线程已经退出，此出不用再删除 */
		if (rtos_delete_thread(&dhcpd_thread) != 0)
			dhcp_w("failed to delete thread\r\n");
#endif
		dhcpd_running = 0;
	} 
	else 
	{
		dhcp_w("server not dhcpd_running.\r\n");
	}
}
// eof

