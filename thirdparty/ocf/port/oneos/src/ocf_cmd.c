
#include <oneos_config.h>
#include <board.h>
#include <stdlib.h>
#include <string.h>

#include <shell.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_clock.h>

#include "oc_api.h"
#include "port/oc_clock.h"

extern int app_init(void);
extern void register_resources(void);
extern void factory_presets_cb(size_t device, void *data);

/*
*/

static os_sem_t block;
static void
signal_event_loop(void)
{
  os_sem_post(&block);
}


#ifdef OC_CLOUD
/**
* cloud status handler.
* handler to print out the status of the cloud connection
*/
static void
cloud_status_handler(oc_cloud_context_t *ctx, oc_cloud_status_t status,
                     void *data)
{
  (void)data;
  PRINT("\nCloud Manager Status:\n");
  if (status & OC_CLOUD_REGISTERED) {
    PRINT("\t\t-Registered\n");
  }
  if (status & OC_CLOUD_TOKEN_EXPIRY) {
    PRINT("\t\t-Token Expiry: ");
    if (ctx) {
      PRINT("%d\n", oc_cloud_get_token_expiry(ctx));
    } else {
      PRINT("\n");
    }
  }
  if (status & OC_CLOUD_FAILURE) {
    PRINT("\t\t-Failure\n");
  }
  if (status & OC_CLOUD_LOGGED_IN) {
    PRINT("\t\t-Logged In\n");
  }
  if (status & OC_CLOUD_LOGGED_OUT) {
    PRINT("\t\t-Logged Out\n");
  }
  if (status & OC_CLOUD_DEREGISTERED) {
    PRINT("\t\t-DeRegistered\n");
  }
  if (status & OC_CLOUD_REFRESHED_TOKEN) {
    PRINT("\t\t-Refreshed Token\n");
  }
}
#endif // OC_CLOUD

extern void initialize_variables(void);

  oc_clock_time_t next_tick = 0;
  oc_clock_time_t now_tick;
void ocf_server_init(void)
{
  int init;
  os_sem_init(&block, "ocf_sem", 0, 1);

  /* set the latency to 240 seconds*/
  /* if no latency is needed then remove the next line */
  //oc_core_set_latency(240);
  // oc_set_max_app_data_size(0x4000);
	// oc_set_max_app_data_size(0x1000);
  oc_set_mtu_size(2048);

/*
 The storage folder depends on the build system
 for Windows the projects simpleserver and cloud_server are overwritten, hence the folders should be the same as those targets.
 for Linux (as default) the folder is created in the makefile, with $target as name with _cred as post fix.
*/
#ifdef OC_SECURITY
  PRINT("Intialize Secure Resources\n");
#ifdef WIN32
#ifdef OC_CLOUD
  PRINT("\tstorage at './cloudserver_creds' \n");
  oc_storage_config("./cloudserver_creds");
#else
  PRINT("\tstorage at './simpleserver_creds' \n");
  oc_storage_config("./simpleserver_creds/");
#endif
#else
  PRINT("\tstorage at './device_builder_server_creds' \n");
  oc_storage_config("./device_builder_server_creds");
#endif

  /*intialize the variables */
  initialize_variables();
  
#endif /* OC_SECURITY */
	
  /* initializes the handlers structure */
  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop,
                                       .register_resources = register_resources
#ifdef OC_CLIENT
                                       ,
                                       .requests_entry = 0 
#endif
                                       };
#ifdef OC_SECURITY
#ifdef OC_SECURITY_PIN
  /* please enable OC_SECURITY_PIN
    - have display capabilities to display the PIN value
    - server require to implement RANDOM PIN (oic.sec.doxm.rdp) onboarding mechanism
  */
  oc_set_random_pin_callback(random_pin_cb, NULL);
#endif /* OC_SECURITY_PIN */
#endif /* OC_SECURITY */

  oc_set_factory_presets_cb(factory_presets_cb, NULL);
  
  /* start the stack */
  init = oc_main_init(&handler);

  if (init < 0) {
    PRINT("oc_main_init failed %d, exiting.\n", init);
    return;
  }

#ifdef OC_CLOUD
  /* get the cloud context and start the cloud */
  PRINT("Start Cloud Manager\n");
  oc_cloud_context_t *ctx = oc_cloud_get_context(0);
  if (ctx) {
    oc_cloud_manager_start(ctx, cloud_status_handler, NULL);
  }
#endif 

  PRINT("OCF server \"server_lite_19645\" running, waiting on incoming connections.\n");

  int wait_tick;
  while (true) {
    next_tick = oc_main_poll();
    if (next_tick == 0)
#ifdef ONEOS_2_0
      wait_tick = OS_WAIT_FOREVER;
#else
      wait_tick = OS_IPC_WAITING_FOREVER;
#endif
    else
    {
      now_tick = oc_clock_time();
      wait_tick = next_tick - now_tick;
      if (wait_tick < 0)
#ifdef ONEOS_2_0
      wait_tick = OS_WAIT_FOREVER;
#else
        wait_tick = OS_IPC_WAITING_FOREVER;
#endif
    } 
    os_sem_wait(&block, wait_tick);
  }
  /* shut down the stack */
#ifdef OC_CLOUD
//   PRINT("Stop Cloud Manager\n");
//   oc_cloud_manager_stop(ctx);
#endif
//   oc_main_shutdown();
}

#ifdef OC_SERVER
#ifdef NET_USING_LWIP
#include "os_clock.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "os_assert.h"
#define OCF_IPv4_MULTICAST      "224.0.1.187"
#define OCF_MCAST_PORT (5683)
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
// static void mcast_udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
// {
//     os_kprintf("recv remote addr:%08x,port:%d\n", addr->addr, port);
// }
#endif

#define OCF_TASK_STACK_SIZE		(16*1024)
//#define OCF_TASK_PRIORITY		(OS_TASK_PRIORITY_MAX / 2 + 1)
#define OCF_TASK_PRIORITY		(8)


extern void ocf_server_init(void);
extern int oc_storage_init(void);
static void ocf_server_main(void* para)
{
#if 1
		os_task_msleep(1000);
    os_pin_mode(key_table[0].pin, key_table[0].mode);
    int reset_button_val = os_pin_read(key_table[0].pin);
    os_kprintf("Pushed button %d.\n", reset_button_val);
	  if (reset_button_val == 0)
		{
		  os_kprintf(">>>>OCF clear env. reset to OTM.\n");//zhw todo don't clear other env, only clear ocf env
			oc_storage_init();
		}
    ocf_server_init();
    return;
#else
    struct udp_pcb *mcast4_pcb = OS_NULL;
    ip_addr_t ip_addr;
    err_t ret = ERR_VAL;

    os_kprintf(">>>>udp server test\n");

#if 1
    inet_aton(OCF_IPv4_MULTICAST, &ip_addr);

    /*-------------创建多播监听server-------------------- */
    /* 加入多播组 */
    ret = igmp_joingroup(IP_ADDR_ANY,(struct ip4_addr *)(&ip_addr));
#else
    ip_addr_t ipgroup_rev;
    IP4_ADDR(&ipgroup_rev, 230,1,1,11);
    ret = igmp_joingroup(IP_ADDR_ANY,(struct ip4_addr *)(&ipgroup_rev));
#endif


    os_kprintf("igmp_joingroup %s ret is %d\n", OCF_IPv4_MULTICAST, ret);

    if (NULL == mcast4_pcb)
        mcast4_pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    OS_ASSERT(mcast4_pcb);

    ret = ip_set_option(mcast4_pcb, SO_BROADCAST);
    os_kprintf("ip_set_option SO_BROADCAST ret is %d\n", ret);
#if 0
    ip_set_option(mcast4_pcb, IP_MULTICAST_TTL);
    ip_set_option(mcast4_pcb, IP_MULTICAST_IF);
#endif
    ret = udp_setflags(mcast4_pcb, udp_flags(mcast4_pcb) | UDP_FLAGS_MULTICAST_LOOP);
    os_kprintf("udp_setflags UDP_FLAGS_MULTICAST_LOOP ret is %d\n", ret);
    ret = udp_bind(mcast4_pcb, IP_ADDR_ANY, OCF_MCAST_PORT);
    if (ERR_OK != ret) {
        os_kprintf("udp_bind in port %d err: %d\n", OCF_MCAST_PORT, ret);
        return;
    }
    udp_recv(mcast4_pcb, mcast_udp_recv, NULL);
    while (1) {
        os_kprintf("test\n");
        os_task_sleep(os_tick_from_ms(2000));
    }
#endif
}

static os_err_t auto_ocf_server(void)
{
#ifdef CUSTOM_RAM
  oc_mem_init();
#endif
	os_task_t *  taskId;
#ifdef ONEOS_2_0
	taskId = os_task_create("ocf_server", ocf_server_main, NULL, OCF_TASK_STACK_SIZE, OCF_TASK_PRIORITY);
#else
	taskId = os_task_create("ocf_server", ocf_server_main, NULL, OCF_TASK_STACK_SIZE, OCF_TASK_PRIORITY, 10);
#endif
	if (OS_NULL == taskId)
	{
		os_kprintf("create ocf_main_loop task failed\n");
		return -1;
	}
	os_task_startup(taskId);
	return 0;  
}


#if 0
struct ocf_cmd_des
{
    const char *cmd;
    int (*fun)(int argc, char *argv[]);
};


static int ocf_help(int argc, char *argv[])
{
    os_kprintf("ocf\n");
    os_kprintf("ocf server\n");
	os_kprintf("ocf client\n");

    return 0;
}


static int ocf_server(int argc, char *argv[])
{
	os_task_t *  taskId;
#ifdef ONEOS_2_0
	taskId = os_task_create("ocf_server", ocf_server_main, NULL, OCF_TASK_STACK_SIZE, OCF_TASK_PRIORITY);
#else
	taskId = os_task_create("ocf_server", ocf_server_main, NULL, OCF_TASK_STACK_SIZE, OCF_TASK_PRIORITY, 10);
#endif
	if (OS_NULL == taskId)
	{
		os_kprintf("create ocf_main_loop task failed\n");
		return -1;
	}
	os_task_startup(taskId);
	return 0;
}

#ifdef OC_CLIENT
extern void ocf_client_init(void);
static int ocf_client(int argc, char *argv[])
{
    ocf_client_init();
}
#endif

/* cmd table */
static const struct ocf_cmd_des ocf_cmd_tab[] =
{
    {"help", ocf_help},
#ifdef OC_SERVER
    {"server", ocf_server},
#endif
#ifdef OC_CLIENT
    {"client", ocf_client},
#endif
};

int ocf_sh(int argc, char *argv[])
{
    int i, result = 0;
    const struct ocf_cmd_des *run_cmd = OS_NULL;

    if (argc == 1)
    {
        ocf_help(argc, argv);
        return 0;
    }

    /* find fun */
    for (i = 0; i < sizeof(ocf_cmd_tab) / sizeof(ocf_cmd_tab[0]); i++)
    {
        if (strcmp(ocf_cmd_tab[i].cmd, argv[1]) == 0)
        {
            run_cmd = &ocf_cmd_tab[i];
            break;
        }
    }

    /* not find fun, print help */
    if (run_cmd == OS_NULL)
    {
        ocf_help(argc, argv);
        return 0;
    }

    /* run fun */
    if (run_cmd->fun != OS_NULL)
    {
        result = run_cmd->fun(argc, argv);
    }

    if (result)
    {
        ocf_help(argc, argv);
    }
    return 0;
}


SH_CMD_EXPORT(ocf, ocf_sh, "ocf command.");
#endif
OS_CMPOENT_INIT(auto_ocf_server, OS_INIT_SUBLEVEL_LOW);
#endif

