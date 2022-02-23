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
#include <stdbool.h>
#include <os_stddef.h>
#include <os_assert.h>

#ifndef PKG_USING_EASYFLASH
#error "please enable easy flash"
#endif

#include <easyflash.h>
#include <ef_def.h>

#include "acw_conf.h"
#include "oneos_config.h"
#include "acw_debug.h"
#include "acw_prot_common.h"

#define ACW_MQTT_SERVER_ADDR    	"acw_mqtt_addr"
#define ACW_MQTT_SERVER_PORT    	"acw_mqtt_port"
#define ACW_MQTT_SERVER_USR    		"acw_mqtt_usr"
#define ACW_MQTT_SERVER_PASSWD  	"acw_mqtt_passwd"

#define ACW_CFG_DEV_ID_ITEM			"acw_dev_id"
#define ACW_CFG_PID_ITEM			"acw_pid"
#define ACW_CFG_SSID_ITEM			"acw_ssid"
#define ACW_CFG_PASSWD_ITEM			"acw_passwd"
#define ACW_CFG_OWNER_ITEM			"acw_owner"
#define ACW_CFG_INIT_ITEM			"acw_init"
#define ACW_CFG_RAND_ITEM			"acw_rand"
#define ACW_CFG_RSSI_ITEM			"acw_rssi"

#define MQTT_SERVER_ADDR_DEF    	"172.20.214.128"
#define MQTT_SERVER_PORT_DEF    	1883
#define MQTT_SERVER_USR_DEF			"admin"
#define MQTT_SERVER_PASSWD_DEF		"public"

#define DEV_ID_DEF 					"lt000008"
#define PRODUCT_ID_DEF 				"OneOS-test"
#define CONF_SSID_DEF				"hw_hsj"
#define CONF_PASSWD_DEF				"Aa123456"

#define ACW_INFO_MAGIC				0xa5

static dev_acw_info_t gs_dev_acw;

//key-value
static int acw_conf_set_cfg(char *item, char *value)
{
#ifdef PKG_USING_EASYFLASH
    EfErrCode flag;

    /* 底层驱动代码缺陷不支持配置空字符串 */
    if (0 == strlen(value))
    {
        ACW_PRINT_E("value is null!");
        return 0;
    }

    flag = ef_set_env(item, value);
    if (flag != EF_NO_ERR)
    {
        ACW_PRINT_E("Set device information to easyflash failed! Set environment error!");
        return -1;
    }
    ef_save_env();
#endif

    return 0;
}

//key-value
static int acw_conf_delete_cfg(char *item)
{
#ifdef PKG_USING_EASYFLASH
    EfErrCode flag;

    flag = ef_set_env(item, NULL);
    if (flag != EF_ENV_NAME_ERR && flag != EF_NO_ERR)
    {
        ACW_PRINT_E("Set device information to easyflash failed! Set environment error! %d", flag);
        return -1;
    }
    ef_save_env();
    ACW_PRINT_I("%s non-existent or delete success", item);
#endif

    return 0;
}

//key-value形式从flash获取
static int acw_conf_get_cfg(char *item, char *value, int bufsize)
{
	char *env;
	int len;

	env = OS_NULL;
	len = 0;
#ifdef PKG_USING_EASYFLASH
    env = ef_get_env(item);
#endif
	if (OS_NULL != env)
	{
		snprintf(value, bufsize, "%s", env);
		len = strlen(value);
	}

    return len;
}

static void acw_set_ap_info_func(int argc, char **argv)
{
	char *null_str = "";

	if (argc == 1)
	{
		acw_conf_save_ap_info(null_str, null_str, null_str);
	}
	else if (argc == 2)
	{
		acw_conf_save_ap_info(argv[1], null_str, null_str);
	}
	else if (argc == 3)
	{
		acw_conf_save_ap_info(argv[1], argv[2], null_str);
	}
	else if (argc == 4)
	{
		acw_conf_save_ap_info(argv[1], argv[2], argv[3]);
	}
	else 
	{
		ACW_PRINT_E("Not set any ap info");
		return;
	}

	return;
}

static void acw_set_mqtt_func(int argc, char **argv)
{
	if (argc != 5)
	{
		ACW_PRINT_E("Invalid input");
		return;
	}

	acw_conf_set_cfg(ACW_MQTT_SERVER_ADDR, argv[1]);
	acw_conf_set_cfg(ACW_MQTT_SERVER_PORT, argv[2]);
	acw_conf_set_cfg(ACW_MQTT_SERVER_USR, argv[3]);
	acw_conf_set_cfg(ACW_MQTT_SERVER_PASSWD, argv[4]);

	return;
}

static void acw_dump_conf_func(void)
{
    char addr[33];
    char usr[33];
    char passwd[33]; 
    int port;
	char *str;

	if (gs_dev_acw.magic != ACW_INFO_MAGIC)
	{
		ACW_PRINT_E("why acw info magic is invalid");
		return;
	}

	acw_get_mqtt_addr(addr, sizeof(addr), &port);
    acw_get_mqtt_user_passwd(usr, sizeof(usr), passwd, sizeof(passwd));

	str = acw_conf_get_devid();
	if (str)
	{
		ACW_PRINT_I("dev_id:%s", str);
	}
	else
	{
		ACW_PRINT_I("no conf dev_id");
	}

	str = acw_conf_get_pid();
	if (str)
	{
		ACW_PRINT_I("product_id:%s", str);
	}
	else
	{
		ACW_PRINT_I("no conf product_id");
	}

	str = acw_conf_get_stored_ssid();
	if (str)
	{
		ACW_PRINT_I("ssid:%s", str);
	}
	else
	{
		ACW_PRINT_I("no conf ssid");
	}

	str = acw_conf_get_stored_passwd();
	if (str)
	{
		ACW_PRINT_I("password:%s", str);
	}
	else
	{
		ACW_PRINT_I("no conf password");
	}

    str = acw_conf_get_owner_id();
	if (str)
	{
		ACW_PRINT_I("owner_id:%s", str);
	}
	else
	{
		ACW_PRINT_I("no owner_id");
	}

	ACW_PRINT_I("mqtt cfg [%s,%d] [%s,%s]", addr, port, usr, passwd);

	return;
}

static void acw_conf_set_devid_func(int argc, char **argv)
{
	char *devid;
	int len;

	if (1 == argc)
	{
		devid = DEV_ID_DEF;
	}
	else if (argc == 2)
	{
		devid = argv[1];
	}
	else
	{
		ACW_PRINT_E("set devid cmd error");
		return;
	}

	len = strlen(devid);
	if (len > ACW_DEV_ID_MAX_LEN)
	{
		ACW_PRINT_E("set devid id to len");
		return;
	}

	memset(gs_dev_acw.dev_id, 0, sizeof(gs_dev_acw.dev_id));
	memcpy(gs_dev_acw.dev_id, devid, len);
	ACW_PRINT_I("set devid magic=%02x", gs_dev_acw.magic);

	acw_conf_set_cfg(ACW_CFG_DEV_ID_ITEM, gs_dev_acw.dev_id);	
	
	return;
}

static void acw_conf_set_pid_func(int argc, char **argv)
{
	char *pid;
	int len;

	if (1 == argc)
	{
		pid = PRODUCT_ID_DEF;
	}
	else if (argc == 2)
	{
		pid = argv[1];
	}
	else
	{
		ACW_PRINT_E("set pid cmd error");
		return;
	}

	len = strlen(pid);
	if (len > ACW_PRODUCT_ID_MAX_LEN)
	{
		ACW_PRINT_E("set pid id to len");
		return;
	}

	memset(gs_dev_acw.product_id, 0, sizeof(gs_dev_acw.product_id));
	memcpy(gs_dev_acw.product_id, pid, len);

	ACW_PRINT_I("set pid magic=%02x", gs_dev_acw.magic);
	acw_conf_set_cfg(ACW_CFG_PID_ITEM, gs_dev_acw.product_id);	
	acw_dump_conf_func();

	return;
}

char *acw_conf_get_stored_ssid(void)
{
	int len;

	if (ACW_INFO_MAGIC != gs_dev_acw.magic)
	{
		return OS_NULL;
	}

	len = strlen(gs_dev_acw.ssid);
	if (!len)
	{
		return OS_NULL;
	}

	return gs_dev_acw.ssid;
}

char *acw_conf_get_stored_passwd(void)
{
	int len;

	if (ACW_INFO_MAGIC != gs_dev_acw.magic)
	{
		return OS_NULL;
	}

	len = strlen(gs_dev_acw.password);
	if (!len)
	{
		return OS_NULL;
	}

	return gs_dev_acw.password;
}

char *acw_conf_get_devid(void)
{
	int len;

	if (ACW_INFO_MAGIC != gs_dev_acw.magic)
	{
		return OS_NULL;
	}

	len = strlen(gs_dev_acw.dev_id);
	if (!len)
	{
		return OS_NULL;
	}

	return gs_dev_acw.dev_id;
}

char *acw_conf_get_pid(void)
{
	int len;

	if (ACW_INFO_MAGIC != gs_dev_acw.magic)
	{
		return OS_NULL;
	}

	len = strlen(gs_dev_acw.product_id);
	if (!len)
	{
		return OS_NULL;
	}

	return gs_dev_acw.product_id;
}

char *acw_conf_get_owner_id(void)
{
	int len;

	if (ACW_INFO_MAGIC != gs_dev_acw.magic)
	{
		return OS_NULL;
	}

	len = strlen(gs_dev_acw.owner_id);
	if (!len)
	{
		return OS_NULL;
	}

	return gs_dev_acw.owner_id;
}

void acw_conf_save_ap_info(char *ssid, char *passwd, char *owner_id)
{
    char* ssid_now = acw_conf_get_stored_ssid();
    char* passwd_now = acw_conf_get_stored_passwd();
	char* owner_id_now = acw_conf_get_owner_id();
	int len;

	if (ssid_now && passwd_now && owner_id)
	{
		if (strcmp(ssid, ssid_now) == 0 && strcmp(passwd, passwd_now) == 0 && strcmp(owner_id, owner_id_now) == 0)
		{
			return;
		}
	}

	len = strlen(ssid);
	memset(gs_dev_acw.ssid, 0, sizeof(gs_dev_acw.ssid));
	memcpy(gs_dev_acw.ssid, ssid, len);

	len = strlen(passwd);
	memset(gs_dev_acw.password, 0, sizeof(gs_dev_acw.password));
	memcpy(gs_dev_acw.password, passwd, len);

	len = strlen(owner_id);
	memset(gs_dev_acw.owner_id, 0, sizeof(gs_dev_acw.owner_id));
	memcpy(gs_dev_acw.owner_id, owner_id, len);

	acw_conf_set_cfg(ACW_CFG_SSID_ITEM, gs_dev_acw.ssid);
	acw_conf_set_cfg(ACW_CFG_PASSWD_ITEM, gs_dev_acw.password);
	acw_conf_set_cfg(ACW_CFG_OWNER_ITEM, gs_dev_acw.owner_id);

	return;
}

static void acw_do_board_reset(void)
{
#ifdef BOARD_BK7231N
//for expection do, if not reboot, bk create ap may failed
extern void bk_reboot(void);
	bk_reboot();
#endif
}

void acw_conf_clean_ap_info(void)
{
	memset(gs_dev_acw.ssid, 0, sizeof(gs_dev_acw.ssid));
	memset(gs_dev_acw.password, 0, sizeof(gs_dev_acw.password));
	memset(gs_dev_acw.owner_id, 0, sizeof(gs_dev_acw.owner_id));

	acw_conf_delete_cfg(ACW_CFG_SSID_ITEM);
	acw_conf_delete_cfg(ACW_CFG_PASSWD_ITEM);
	acw_conf_delete_cfg(ACW_CFG_OWNER_ITEM);

	acw_dump_conf_func();
	acw_do_board_reset();
}

void acw_get_mqtt_addr(char addr[], int len, int *port)
{
	char port_str[8];
	int do_err;

	do_err = acw_conf_get_cfg(ACW_MQTT_SERVER_ADDR, addr, len);
	if (do_err <= 0)
	{
		strcpy(addr, MQTT_SERVER_ADDR_DEF);
	}

	memset(port_str, 0 , sizeof(port_str));
	do_err = acw_conf_get_cfg(ACW_MQTT_SERVER_PORT, port_str, sizeof(port_str));
	if (do_err <= 0)
	{
		*port = MQTT_SERVER_PORT_DEF;
	}
	else
	{
		*port = atoi(port_str);
	}
	
	return;
}

void acw_get_mqtt_user_passwd(char user_name[], int user_len, char passwd[], int passwd_len)
{
	int do_err;

	do_err = acw_conf_get_cfg(ACW_MQTT_SERVER_USR, user_name, user_len);
	if (do_err <= 0)
	{
		strcpy(user_name, MQTT_SERVER_USR_DEF);
	}

	do_err = acw_conf_get_cfg(ACW_MQTT_SERVER_PASSWD, passwd, passwd_len);
	if (do_err <= 0)
	{
		strcpy(passwd, MQTT_SERVER_PASSWD_DEF);
	}

	return;
}

#define ACW_DEVTYPE_WM_STR	"wm"	// washing machine
#define ACW_DEVTYPE_AC_STR	"ac"	// air conditioner
#define ACW_DEVTYPE_PP_STR	"pp"	// power plug
#define ACW_DEVTYPE_LP_STR	"lp"	// "lamp"

acw_dev_type_t acw_get_dev_type(void)
{
	acw_dev_type_t dev_type;

	dev_type = DEV_TYPE_UNKNOWN;
	if (ACW_INFO_MAGIC == gs_dev_acw.magic)
	{
		if (!strncmp(gs_dev_acw.dev_id, ACW_DEVTYPE_AC_STR, strlen(ACW_DEVTYPE_AC_STR)))
		{
			dev_type = DEV_TYPE_AC;
		}
		else if(!strncmp(gs_dev_acw.dev_id, ACW_DEVTYPE_WM_STR, strlen(ACW_DEVTYPE_WM_STR)))
		{
			dev_type = DEV_TYPE_WM;
		}
		else if(!strncmp(gs_dev_acw.dev_id, ACW_DEVTYPE_PP_STR, strlen(ACW_DEVTYPE_PP_STR)))
		{
			dev_type = DEV_TYPE_PP;
		}
		else if(!strncmp(gs_dev_acw.dev_id, ACW_DEVTYPE_LP_STR, strlen(ACW_DEVTYPE_LP_STR)))
		{
			dev_type = DEV_TYPE_LP;
		}
	}

	return dev_type;
}

os_bool_t acw_get_init_flag(void)
{
	int do_err;
	char value[8];
	memset(value, 0, sizeof(value));

	do_err = acw_conf_get_cfg(ACW_CFG_INIT_ITEM, value, sizeof(value));
	if (do_err <= 0)
	{
		return OS_FALSE;
	}

	if (!strcmp(value, "true"))
	{
		return OS_TRUE;
	}

	return OS_FALSE;
}

os_uint8_t acw_get_rand(void)
{
	os_uint8_t rand;
	char value[4];
	int do_err;

	memset(value, 0, sizeof(value));

	do_err = acw_conf_get_cfg(ACW_CFG_RAND_ITEM, value, sizeof(value));
	if (do_err <= 0)
	{
		return 0;
	}

	rand = atoi(value);

	return rand;
}

#ifdef NET_USING_ACW_CRYPTO_PRI
void acw_get_private_key(char *key)
{
	memcpy(key, "1234567887456321", 16);
}
#endif

void acw_set_init_flag(void)
{
	os_uint8_t rand;
	char value[4];

	acw_conf_set_cfg(ACW_CFG_INIT_ITEM, "true");

	rand = acw_get_rand();
	rand++;
	memset(value, 0, sizeof(value));
	sprintf(value, "%u", rand);
	acw_conf_set_cfg(ACW_CFG_RAND_ITEM, value);

	return;
}

void acw_clr_init_flag(void)
{
	acw_conf_set_cfg(ACW_CFG_INIT_ITEM, "false");
}

int acw_get_master_connect_mim_rssi(void)
{
	char min_rssi[8];
	int do_err;
	int rssi;

	rssi = -70;
	do_err = acw_conf_get_cfg(ACW_CFG_RSSI_ITEM, min_rssi, sizeof(min_rssi));
	if (do_err > 0)
	{
		rssi = atoi(min_rssi);
	}

	return rssi;
}

int acw_conf_init(void)
{
	int do_err;

	memset(&gs_dev_acw, 0, sizeof(dev_acw_info_t));
	easyflash_init();

	do_err = acw_conf_get_cfg(ACW_CFG_DEV_ID_ITEM, gs_dev_acw.dev_id, ACW_DEV_ID_MAX_LEN + 1);
	if (do_err <= 0)
	{
		strcpy(gs_dev_acw.dev_id, DEV_ID_DEF);
	}
	ACW_PRINT_I("gs_dev_acw.dev_id=%s", gs_dev_acw.dev_id);

	do_err = acw_conf_get_cfg(ACW_CFG_PID_ITEM, gs_dev_acw.product_id, ACW_PRODUCT_ID_MAX_LEN + 1);
	if (do_err <= 0)
	{
		strcpy(gs_dev_acw.product_id, PRODUCT_ID_DEF);
	}

	do_err = acw_conf_get_cfg(ACW_CFG_SSID_ITEM, gs_dev_acw.ssid, ACW_SSID_MAX_LEN + 1);
	if (do_err <= 0)
	{
		//strcpy(gs_dev_acw.ssid, CONF_SSID_DEF);
	}

	do_err = acw_conf_get_cfg(ACW_CFG_PASSWD_ITEM, gs_dev_acw.password, ACW_AP_PASSWORD_MAX_LEN + 1);
	if (do_err <= 0)
	{
		//strcpy(gs_dev_acw.password, CONF_PASSWD_DEF);
	}

	do_err = acw_conf_get_cfg(ACW_CFG_OWNER_ITEM, gs_dev_acw.owner_id, ACW_OWNER_ID_MAX_LEN + 1);
	if (do_err <= 0)
	{
		//strcpy(gs_dev_acw.password, CONF_PASSWD_DEF);
	}
	gs_dev_acw.magic = ACW_INFO_MAGIC;

	return 0;
}

#if 0
/* Returns int value of hex string character |c| */
static uint8_t hex_to_uint(uint8_t c)
{
  if ('0' <= c && c <= '9') 
  {
    return (uint8_t)(c - '0');
  }
  if ('A' <= c && c <= 'F')
  {
    return (uint8_t)(c - 'A' + 10);
  }
  if ('a' <= c && c <= 'f') 
  {
    return (uint8_t)(c - 'a' + 10);
  }
  
  return 0;
}
#endif

static void acw_do_conf_init_func(int argc, char **argv)
{
	acw_conf_init();
	return;
}

#ifdef BOARD_BK7231N

extern int wifi_set_mac_address(char *mac);
static void acw_set_mac_func(int argc, char **argv)
{
	char mac[] = {0x28, 0x2c, 0xDD, 0x62, 0x68, 0x24};
	char mac_diff;
	char* devtype = "lt";

	if (argc == 2)
	{
		mac_diff = atoi(argv[1]);
		mac[5] += mac_diff;
		wifi_set_mac_address(mac);
		acw_do_board_reset();
	}

	return;
}
#endif

#include <os_memory.h>
static void acw_free_test_func(int argc, char **argv)
{
	char *test;

	test = os_malloc(sizeof(char *));
	os_free(test);
	os_free(test);

	return;
}

#ifdef OS_USING_SHELL
#include <shell.h>

SH_CMD_EXPORT(acw_free_test, acw_free_test_func, "acw_free_test");

SH_CMD_EXPORT(acw_conf_init, acw_do_conf_init_func, "acw init");
SH_CMD_EXPORT(acw_set_ap_info, acw_set_ap_info_func, "set ap_info [ssid] [passwd]");
#ifdef BOARD_BK7231N
SH_CMD_EXPORT(acw_set_mac, acw_set_mac_func, "set mac [xx]");
#endif
SH_CMD_EXPORT(acw_dump_info, acw_dump_conf_func, "dump acw info");
SH_CMD_EXPORT(acw_set_devid, acw_conf_set_devid_func, "set devid [xxx]");
SH_CMD_EXPORT(acw_set_pid, acw_conf_set_pid_func, "set pid [xxx]");
SH_CMD_EXPORT(acw_clear, acw_conf_clean_ap_info, "clear_acw");
SH_CMD_EXPORT(acw_set_mqtt_info, acw_set_mqtt_func, "set mqtt_server [addr] [port] [usr] [passwd]");

#endif  /* end of using OS_USING_SHELL */
