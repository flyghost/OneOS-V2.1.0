/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* coap -- simple implementation of the Constrained Application Protocol (CoAP)
 *         as defined in RFC 7252
 *
 * Copyright (C) 2010--2019 Olaf Bergmann <bergmann@tzi.org> and others
 *
 * This file is part of the CoAP library libcoap. Please see README for terms
 * of use.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <os_clock.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pin.h>
//#include <os_irq.h>
#include <os_assert.h>
#include <drv_gpio.h>

#include "acw_intf.h"
#include "acw_prot_common.h"
//#include "mqtt_client.h"
#include "acw_conf.h"
#include "acw_dev_ap.h"
#include "acw_debug.h"

#define ACW_TASK_STACK_SIZE		8192
#define ACW_TASK_PRIORITY		(OS_TASK_PRIORITY_MAX / 2 + 1)

static acw_run_ctrl_t gs_acw_ctrl;

void assert_reboot_hook(const char *ex_string, const char *func, os_int32_t line)
{
#if 0    
	if (0 != os_interrupt_get_nest())
	{
		os_kprintf("dead in interupt :");
	}
	else
	{
		char * task_name = os_task_name(os_task_self());
		os_kprintf("dead in %s :", task_name);
	}
#endif	
	os_kprintf("(%s) assertion failed at function: %s, line number: %d \r\n", ex_string, func, line);
	while(1);
}

void acw_start_proc(acw_intf_t intf, acw_dev_connect_home_succ_sync cnt_succ, acw_dev_req_add_home add_home)
{
	os_task_t *  taskId;

	memset(&gs_acw_ctrl, 0, sizeof(gs_acw_ctrl));
    ACW_PRINT_I("acw_main_proc ver 1.8");
	
	acw_conf_init();
	acw_master_dev_ap_init();
	acw_slave_dev_ap_init();
	//os_assert_set_hook(assert_reboot_hook);
	acw_intf_init(intf);

	gs_acw_ctrl.cnt_home_succ_sync_func = cnt_succ;
	gs_acw_ctrl.req_add_home_func = add_home;

	taskId = os_task_create("tacw", acw_main_loop, &gs_acw_ctrl, ACW_TASK_STACK_SIZE, ACW_TASK_PRIORITY);
	if (OS_NULL == taskId)
	{
		ACW_PRINT_E("create acw_main_loop task failed");
		return;
	}
	gs_acw_ctrl.link_home_task = taskId;
	os_task_startup(taskId);

	return;
}

void acw_do_connect_home_succ_sync(char *home_id, os_bool_t clr)
{
	if (OS_NULL != gs_acw_ctrl.cnt_home_succ_sync_func)
	{
		gs_acw_ctrl.cnt_home_succ_sync_func(home_id, clr);
	}
}

void acw_do_req_add_home(char *dev_id, os_bool_t clr, os_uint8_t rand)
{
	if (OS_NULL != gs_acw_ctrl.req_add_home_func)
	{
		gs_acw_ctrl.req_add_home_func(dev_id, clr, rand);
	}
}
