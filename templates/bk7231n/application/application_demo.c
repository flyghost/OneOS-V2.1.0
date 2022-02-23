/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * @file        application_demo.c
 *
 * @brief       This file provides a app demo 
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-5   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include "shell.h"
#include "rtos_pub.h"
#include "drv_flash.h"
#include "wlan_dev.h"
#include "wlan_mgnt.h"
#include "error.h"
#include <sys/socket.h>
#include "os_mq.h"
#include "os_types.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "shell.h"
#include "pin.h"

#include <string.h>
#include <shell.h>
#include <sys/time.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>


#include <easyflash.h>

#define CONFIG_INFO_ADDR      (0x1FE000)
#define LISTEN_SERVER_PORT    (28000)

extern void beken_hw_cpu_reset(void);
extern os_tick_t os_tick_from_ms(os_uint32_t ms);
extern os_err_t os_task_delay(os_tick_t tick);
extern int wifi_set_mac_address(char *mac);



struct config_info_manager {
	os_wlan_mode_t mode;
	char ssid[32];
	char password[32];
};


static struct config_info_manager g_cfg_info;
static beken_thread_t thread_recv_deal;
static beken_thread_t thread_server;
static os_mq_t        cfg_mq;
static int            g_msg[32];
static char           result[64];


/* receive data deal task */
void data_deal_task(beken_thread_arg_t arg)
{
    OSStatus  err       = kNoErr;
    int       fd        = -1;
    int       len       = 0;
    os_size_t recv_size = 0; 
	
    char *buf = NULL;
	char *ptr = NULL;
	int   count = 0;
	bool finish = false;

    buf = (char*) os_malloc(1024);
    ASSERT(buf);
    
    while (1)
    {
		/* get msg from mq */
		if (os_mq_recv(&cfg_mq, &fd, sizeof(int),OS_IPC_WAITING_FOREVER,&recv_size) == OS_EOK)
		{
			os_kprintf("recv msg from message queue, the content:%d\n",fd);
		}

		finish = false;
		while(!finish)
		{
			len = lwip_recv(fd, buf, 1024, 0);

			os_kprintf("recv buf= %s, len: %d\n",buf,len);

			if (len <= 0)
			{
			    os_kprintf( "TCP Client is disconnected, fd: %d\n", fd );
			    goto exit;
			}

			ptr = strtok(buf,"|"); // eg: "sta|beken|12345678|end";
			
			if(0 == strcmp(ptr,"sta")) {
				finish = true;
				g_cfg_info.mode = OS_WLAN_STATION;
				}
			else if(0 == strcmp(ptr,"ap")){
				finish = true;
				g_cfg_info.mode = OS_WLAN_AP;
				}
			else {
				memset(buf,0,1024);
				strcpy(result,"parse failed,info err!\n");
				len = lwip_send(fd, result,sizeof(result), 0);
				continue;
			}
	   }

		count = 0;
		while (ptr)
		{ /* parse info */
			count++;
			ptr = strtok(NULL,"|");

			if(ptr) {
				
				switch(count) {
					case 1:
							memset(g_cfg_info.ssid,'\0',32);
							strcpy(g_cfg_info.ssid,ptr);
							os_kprintf("ssid:%s\n",g_cfg_info.ssid);
					break;

					case 2:
							memset(g_cfg_info.password,'\0',32);
							strcpy(g_cfg_info.password,ptr);
							os_kprintf("password:%s\n",g_cfg_info.password);
					break;

					default:
					break;
				}
			}
		}
		
		beken_flash_write(CONFIG_INFO_ADDR,&g_cfg_info,sizeof(struct config_info_manager));
		os_task_delay(os_tick_from_ms(200));
		
		strcpy(result,"parse success,get info! reboot later!\n");
		len = lwip_send(fd, result,sizeof(result), 0);
		
		os_kprintf("parse success,get info! send len=%d, reboot later!\n",len);
	    os_task_delay(os_tick_from_ms(5000));
		beken_hw_cpu_reset();
    }

exit:
    if (err != kNoErr) 
		os_kprintf("TCP client thread exit with err: %d\n", err);
	
    if (buf != NULL) 
		os_free(buf);
	
    lwip_close(fd);
    rtos_delete_thread(&thread_recv_deal);
}


/* tcp server listener task */
void tcp_server_task(  void)
{
    OSStatus err = kNoErr;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sockaddr_t_size = sizeof(client_addr);
    char client_ip_str[16];
    int tcp_listen_fd = -1, client_fd = -1;
    fd_set readfds;

    tcp_listen_fd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;/* Accept conenction request on all network interface */
    server_addr.sin_port = htons(LISTEN_SERVER_PORT);/* Server listen on port: 28000 */
    err = lwip_bind(tcp_listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    
    err = lwip_listen(tcp_listen_fd, 0);
    
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(tcp_listen_fd, &readfds);

        lwip_select( tcp_listen_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(tcp_listen_fd, &readfds))
        {
            client_fd = lwip_accept(tcp_listen_fd, (struct sockaddr *) &client_addr, &sockaddr_t_size);
            if (client_fd >= 0)
            {
                os_strcpy(client_ip_str, inet_ntoa(client_addr.sin_addr));

				/*send msg to mq*/
				if (OS_EOK != os_mq_send(&cfg_mq, &client_fd, sizeof(int),OS_IPC_WAITING_NO))
				{
					os_kprintf("send message failed!\n");
                    lwip_close(client_fd);
					client_fd = -1;
				}
				
				os_kprintf("TCP Client %s:%d connected, fd: %d\n", client_ip_str, client_addr.sin_port, client_fd);
            }
        }
    }
	
    if (err != kNoErr) 
		os_kprintf("Server listerner thread exit with err: %d \n", err);
	
    lwip_close(tcp_listen_fd);
    rtos_delete_thread(&thread_server);
}

void demo_app_test(void)
{
    OSStatus err = kNoErr;

    /* init a message queue */
    os_mq_init(&cfg_mq, "mqt",&g_msg[0],sizeof(g_msg),sizeof(int),OS_IPC_FLAG_FIFO);

	
    err = rtos_create_thread(&thread_server, BEKEN_APPLICATION_PRIORITY, 
									"server_demo", 
									(beken_thread_function_t)tcp_server_task,
									0x800,
									(beken_thread_arg_t)0 );
	if(err != kNoErr)
    {
       os_kprintf("create \"server_demo\" thread failed!\r\n");
    }


    err = rtos_create_thread(&thread_recv_deal, BEKEN_APPLICATION_PRIORITY, 
				                     "recv_deal_demo",
                                     (beken_thread_function_t)data_deal_task,
                                     0x800, 
                                     (beken_thread_arg_t)0 );
	if(err != kNoErr)
    {
       os_kprintf("create \"recv_deal_demo\" thread failed!\r\n");
    }
	
}
SH_CMD_EXPORT(demo_app_test,demo_app_test,"start tcp server demo test.");


void demo_flash_read_test(void)
{
	struct config_info_manager info;
	
	beken_flash_read(CONFIG_INFO_ADDR,&info,sizeof(struct config_info_manager));
	os_kprintf("mode=%d, ssid=%s,password=%s \n",info.mode,info.ssid,info.password);
}
SH_CMD_EXPORT(demo_flash_read_test,demo_flash_read_test,"beken flash read test.");


void demo_flash_write_test(void)
{
	struct config_info_manager info;
	char ptr1[]="beken";
	char ptr2[]="12345678";

	info.mode = OS_WLAN_STATION;
	strcpy(info.ssid,ptr1);
	strcpy(info.password,ptr2);
	
	beken_flash_write(CONFIG_INFO_ADDR,&info,sizeof(struct config_info_manager));
	os_kprintf("mode=%d, ssid=%s, password=%s \n",info.mode,info.ssid,info.password);
}
SH_CMD_EXPORT(demo_flash_write_test,demo_flash_write_test,"beken flash write test.");


void demo_station_connect_test(void)
{
	struct config_info_manager info;
	
	beken_flash_read(CONFIG_INFO_ADDR,&info,sizeof(struct config_info_manager));

    os_wlan_connect(info.ssid,info.password);
}
SH_CMD_EXPORT(demo_station_connect_test,demo_station_connect_test,"station connect wifi of information in flash test.");

void demo_ap_start_test(void)
{
	os_wlan_staos_ap("bk","12345678");
}
SH_CMD_EXPORT(demo_ap_start_test,demo_ap_start_test,"ap startup of ssid:beken pass:12345678 test.");

/* easyflash test demo */
void easyflash_test(void)
{
    uint32_t i_boot_times = 0;//NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* init */
    easyflash_init();

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    OS_ASSERT(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);

    /* boot count +1 */
    i_boot_times ++;
    os_kprintf("The system now boot %d times\n\r", i_boot_times);

    /* interger to string */
    sprintf(c_new_boot_times,"%ld", i_boot_times);

    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
}
SH_CMD_EXPORT(easyflash_test,easyflash_test,"easyflash test.");

void demo_set_mac(void)
{
	char test_mac[]={0x28, 0xC2, 0xDD, 0x61, 0x68, 0x62};
	wifi_set_mac_address(test_mac);
}
SH_CMD_EXPORT(demo_set_mac,demo_set_mac,"test_set_mac.");

#define SW_P20 20
#define SW_P21 21
static void demo_set_pin(void)
{
	static unsigned int status = 1;

    os_pin_mode(SW_P20, PIN_MODE_OUTPUT);
    os_pin_write(SW_P20, status);

	os_pin_mode(SW_P21, PIN_MODE_OUTPUT);
    os_pin_write(SW_P21, status);

	status = 0x01&(~status);
}
SH_CMD_EXPORT(demo_set_pin,demo_set_pin,"demo set gpio pin.");

#ifdef OS_USING_RTC
static void demo_get_time(void)
{
	time_t t;
	int ti = 0;
	
	ti = (int)time(&t);

	os_kprintf("time:%d\n",ti);
}
SH_CMD_EXPORT(demo_get_time,demo_get_time,"demo get time.");
#endif


//udp test 
#define BUFSZ 1024
//#define SERVER_HOSTNAME      "192.168.10.255"
#define SERVER_HOSTNAME      "255.255.255.255"

#define SERVER_TCP_PORT          "6588"
#define SERVER_UDP_PORT          "6589"
static const char send_data[] = "This is LwIP Client from OneOS.";


static void test_udpsocket_send(void)
{
    int                ret;
    int                bytes_received;
    int                sock = -1;
    struct hostent*    host = OS_NULL;
	os_uint32_t        port;
    struct sockaddr_in server_addr;
		char *             recv_data = OS_NULL;

    /* 分配用于存放接收数据的缓冲 */
    recv_data = malloc(BUFSZ);
    if (recv_data == OS_NULL)
    {
        os_kprintf("No memory");
        return;
    }

    /* 创建一个socket，SOCK_DGRAM，udp类型 */
    if ((sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        /* 创建socket失败 */
        os_kprintf("Create socket error");
		return;
    }

	const int on = 1;

	lwip_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置套接字选项

    /* 初始化预连接的服务端地址 */
	host = lwip_gethostbyname(SERVER_HOSTNAME);
	port = strtol(SERVER_UDP_PORT, 0, 10);
		
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    //server_addr.sin_addr   = *((struct in_addr*)host->h_addr);
	server_addr.sin_addr.s_addr = INADDR_BROADCAST;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/*UDP socket send and recv data*/
	os_kprintf("UDP socket [%d] send data to server\n", sock);
	ret = lwip_sendto(sock, send_data, strlen(send_data), 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));

	os_kprintf("send ret:%d\n",ret);
	
	os_task_mdelay(300);

	os_free(recv_data);
	lwip_close(sock);
}
SH_CMD_EXPORT(test_udpsocket_send,test_udpsocket_send,"udp socket send data.");

static void test_udpsocket_recv(void *parameter)
{
    int                ret;
    int                bytes_received;
    int                sock = -1;
    struct hostent*    host = OS_NULL;
	os_uint32_t        port;
    struct sockaddr_in server_addr;
		char *             recv_data = OS_NULL;

    /* 分配用于存放接收数据的缓冲 */
    recv_data = malloc(BUFSZ);
    if (recv_data == OS_NULL)
    {
        os_kprintf("No memory");
        return;
    }

    /* 创建一个socket，SOCK_DGRAM，udp类型 */
    if ((sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        /* 创建socket失败 */
        os_kprintf("Create socket error");
		return;
    }

	const int on = 1;
	lwip_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); 

    /* init */
	host = lwip_gethostbyname(SERVER_HOSTNAME);
	port = strtol(SERVER_UDP_PORT, 0, 10);
		
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    //server_addr.sin_addr   = *((struct in_addr*)host->h_addr);
	server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	
	/* bind socket to the server address */
	if (lwip_bind(sock, (struct sockaddr *)&server_addr,
			 sizeof(struct sockaddr)) == -1)
	{
		/* bind failed. */
		os_kprintf("bind server address failed, errno=%d\n", errno);
		lwip_close(sock);
		free(recv_data);
		return;
	}

	struct sockaddr_in recv_addr;
	socklen_t len = sizeof(struct sockaddr);
	static const char back_data[] = "get data."; 
	
	while(1)
	{
		bytes_received = lwip_recvfrom(sock, recv_data, BUFSZ, 0,(struct sockaddr*)&recv_addr,&len);
		os_kprintf("bytes_received:%d,lwip_recv:%s\n",bytes_received,recv_data);

		ret = lwip_sendto(sock, back_data, strlen(back_data), 0, (struct sockaddr*)&recv_addr, sizeof(struct sockaddr));
		os_kprintf("test_udpsocket_recv send ret:%d sin_family=%d,sin_port=%d,sin_addr.s_addr=%d \n",
													ret,recv_addr.sin_family,recv_addr.sin_port,recv_addr.sin_addr.s_addr);
	}
	
	os_free(recv_data);
	lwip_close(sock);
}

void udp_recv_task(void)
{
    os_task_t *task;

    task = os_task_create("udp server", test_udpsocket_recv, OS_NULL,4096, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);
}
SH_CMD_EXPORT(udp_recv_task,udp_recv_task,"udp task recv data.");

void get_rssi(void)
{
	extern int os_wlan_get_rssi(void);
	int level = 0;

	level = os_wlan_get_rssi();

    os_kprintf(" get rssi:%d \n",level);
}
SH_CMD_EXPORT(get_rssi,get_rssi,"get rssi.");

