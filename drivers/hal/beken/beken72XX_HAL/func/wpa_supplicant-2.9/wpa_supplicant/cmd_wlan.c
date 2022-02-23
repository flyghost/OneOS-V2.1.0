/*
 * net mode <mode>
 * 		- net mode sta
 * 		- net mode ap
 * 		- net mode mon
 */

/*
 * net sta config <ssid> [psk]
 * 		- net sta config ssid_example
 * 		- net sta config ssid_example psk_example
 *
 * net sta set <field> <value>
 * 		- net sta set ssid ssid_example
 * 		- net sta set psk psk_example
 * 		- net sta set wep_key0 wep_key_example
 * 		- net sta set wep_key1 wep_key_example
 * 		- net sta set wep_key2 wep_key_example
 * 		- net sta set wep_key3 wep_key_example
 * 		- net sta set wep_key_index <0, 1, 2, 3>
 * 		- net sta set key_mgmt {WPA-PSK, NONE}
 * 		- net sta set pairwise {CCMP, TKIP, WEP40, WEP104, NONE}
 * 		- net sta set group {CCMP, TKIP, WEP40, WEP104, NONE}
 * 		- net sta set proto {WPA, RSN}
 * 		- net sta set auth_alg {OPEN, SHARED}
 * 		- net sta set ptk_rekey <seconds>
 * 		- net sta set scan_ssid (0, 1)
 *      - net sta set sae_groups {19, 20, 21, }
 *      - net sta set ieee80211w <0, 1, 2>
 *      - net sta set debug_level <0,1,2,3,4,5>
 *      - net sta set debug_show_keys <0,1>
 *
 * net sta get <field>
 * 		- net sta get ssid
 * 		- net sta get psk
 * 		- net sta get wep_key0
 * 		- net sta get wep_key1
 * 		- net sta get wep_key2
 * 		- net sta get wep_key3
 * 		- net sta get wep_key_index
 * 		- net sta get key_mgmt
 * 		- net sta get pairwise
 * 		- net sta get group
 * 		- net sta get proto
 * 		- net sta get auth_alg
 * 		- net sta get ptk_rekey
 * 		- net sta get scan_ssid
 *
 * net sta enable
 * net sta disable
 *
 * net sta scan once
 * net sta scan result <num>
 * net sta scan interval <sec>
 * net sta bss max count <num>
 * net sta bss flush <age>
 *
 * net sta connect
 * net sta disconnect
 * net sta state
 * net sta ap
 *
 * net sta genpsk <ssid> <passphrase>
 *
 * net sta wps pbc
 * net sta wps pin get
 * net sta wps pin set <pin>
 */

/*
 * net ap config <ssid> [psk]
 * 		- net ap config ssid_example
 * 		- net ap config ssid_example psk_example
 *
 * net ap set <field> <value>
 * 		- net ap set ssid ssid_example
 * 		- net ap set psk psk_example
 * 		- net ap set key_mgmt {WPA-PSK, NONE}
 * 		- net ap set wpa {CCMP, TKIP, NONE}
 * 		- net ap set rsn {CCMP, TKIP, NONE}
 * 		- net ap set proto <NONE, {WPA, RSN}>
 * 		- net ap set auth_alg {OPEN}
 * 		- net ap set group_rekey <seconds>
 * 		- net ap set strict_rekey <0, 1>
 * 		- net ap set gmk_rekey <seconds>
 * 		- net ap set ptk_rekey <seconds>
 * 		- net ap set hw_mode <b, g>
 * 		- net ap set 80211n <0, 1>
 * 		- net ap set channel <1 ~ 13>
 * 		- net ap set beacon_int <15 ~ 65535>
 * 		- net ap set dtim <1 ~ 255>
 * 		- net ap set max_num_sta <num>
 *
 * net ap get <field>
 * 		- net ap get ssid
 * 		- net ap get psk
 * 		- net ap get key_mgmt
 * 		- net ap get wpa
 * 		- net ap get rsn
 * 		- net ap get proto
 * 		- net ap get auth_alg
 * 		- net ap get group_rekey
 * 		- net ap get strict_rekey
 * 		- net ap get gmk_rekey
 * 		- net ap get ptk_rekey
 * 		- net ap get hw_mode
 * 		- net ap get 80211n
 * 		- net ap get channel
 * 		- net ap get beacon_int
 * 		- net ap get dtim
 * 		- net ap get max_num_sta
 *
 * net ap enable
 * net ap reload
 * net ap disable
 *
 * net ap sta num
 * net ap sta info <num>
 */

//#include "wlan.h"
#include "sys_config.h"
#include "common.h"
#include "wlan_defs.h"
#include "uart_pub.h"
#include "wlan_ui_pub.h"
#include "param_config.h"
#include "ieee802_11_demo.h"
#include "net.h"
#define CMD_WLAN_MAX_BSS_CNT	50

#if (CFG_WFA_CERT || CFG_NEW_SUPP)
/* parse all argument vectors from a command string, return argument count */
static int cmd_parse_argv(char *cmd, char *argv[], int size)
{
	int last, argc;
	char *start, *end;

	argc = 0;
	start = cmd;

	while (argc < size && *start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';
		argv[argc++] = start;
		if (last)
			break;
		start = end + 1;
	}

	if (argc > 0 && argc < size) {
		argv[argc] = NULL; /* ANSI-C requirement */
	}

	return argc;
}

/* parse all argument vectors from a command string, return argument count */
static int cmd_parse_array_int(char *cmd, int *array, int size)
{
	int last, argc;
	char *start, *end;

	argc = 0;
	start = cmd;

	while (argc < size && *start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';
		array[argc++] = atoi(start);
		if (last)
			break;
		start = end + 1;
	}

	return argc;
}

/* wpas parse */

static int cmd_wpas_parse_int(const char *value, int min, int max, int *dst)
{
	int val;
	char *end;

	val = strtol(value, &end, 0);
	if (*end) {
		bk_printf("Invalid number '%s'", value);
		return -1;
	}

	if (val < min || val > max) {
		bk_printf("out of range value %d (%s), range is [%d, %d]\n",
			 val, value, min, max);
		return -1;
	}

	*dst = val;
	return 0;
}

static int cmd_wpas_parse_key_mgmt(const char *value)
{
	int val = 0, last, errors = 0;
	char *start, *end, *buf;

	buf = os_strdup(value);
	if (buf == NULL)
		return -1;
	start = buf;

	while (*start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';
		if (os_strcmp(start, "WPA-PSK") == 0)
			val |= WPA_KEY_MGMT_PSK;
		else if (os_strcmp(start, "WPA-EAP") == 0)
			val |= WPA_KEY_MGMT_IEEE8021X;
		else if (os_strcmp(start, "IEEE8021X") == 0)
			val |= WPA_KEY_MGMT_IEEE8021X_NO_WPA;
		else if (os_strcmp(start, "NONE") == 0)
			val |= WPA_KEY_MGMT_NONE;
		else if (os_strcmp(start, "WPA-NONE") == 0)
			val |= WPA_KEY_MGMT_WPA_NONE;
		else if (os_strcmp(start, "SAE") == 0)
			val |= WPA_KEY_MGMT_SAE;
		else {
			os_printf("Invalid key_mgmt '%s'", start);
			errors++;
		}

		if (last)
			break;
		start = end + 1;
	}
	os_free(buf);

	if (val == 0) {
		os_printf("No key_mgmt values configured\n");
		errors++;
	}

	os_printf("key_mgmt: 0x%x\n", val);
	return errors ? -1 : val;
}

static int cmd_wpas_parse_cipher(const char *value)
{
	int val = 0, last;
	char *start, *end, *buf;

	buf = os_strdup(value);
	if (buf == NULL)
		return -1;
	start = buf;

	while (*start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';
		if (os_strcmp(start, "CCMP-256") == 0)
			val |= WPA_CIPHER_CCMP_256;
		else if (os_strcmp(start, "GCMP-256") == 0)
			val |= WPA_CIPHER_GCMP_256;
		else if (os_strcmp(start, "CCMP") == 0)
			val |= WPA_CIPHER_CCMP;
		else if (os_strcmp(start, "GCMP") == 0)
			val |= WPA_CIPHER_GCMP;
		else if (os_strcmp(start, "TKIP") == 0)
			val |= WPA_CIPHER_TKIP;
		else if (os_strcmp(start, "WEP104") == 0)
			val |= WPA_CIPHER_WEP104;
		else if (os_strcmp(start, "WEP40") == 0)
			val |= WPA_CIPHER_WEP40;
		else if (os_strcmp(start, "NONE") == 0)
			val |= WPA_CIPHER_NONE;
		else if (os_strcmp(start, "GTK_NOT_USED") == 0)
			val |= WPA_CIPHER_GTK_NOT_USED;
		else {
			os_free(buf);
			return -1;
		}

		if (last)
			break;
		start = end + 1;
	}
	os_free(buf);

	return val;
}

static int cmd_wpas_parse_proto(const char *value)
{
	int val = 0, last, errors = 0;
	char *start, *end, *buf;

	buf = os_strdup(value);
	if (buf == NULL)
		return -1;
	start = buf;

	while (*start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';

		/* softAP work on open mode. */
		if (os_strcmp(start, "NONE") == 0) {
			val = 0;
			break;
		}

		if (os_strcmp(start, "WPA") == 0)
			val |= WPA_PROTO_WPA;
		else if (os_strcmp(start, "RSN") == 0 ||
			 os_strcmp(start, "WPA2") == 0)
			val |= WPA_PROTO_RSN;
		else if (os_strcmp(start, "OSEN") == 0)
			val |= WPA_PROTO_OSEN;
		else {
			os_printf("Invalid proto '%s'\n", start);
			errors++;
		}

		if (last)
			break;
		start = end + 1;
	}
	os_free(buf);

/* softAP work on open mode. */
#if 0
	if (val == 0) {
		os_printf("No proto values configured\n");
		errors++;
	}
#endif

	os_printf("proto: 0x%x\n", val);
	return errors ? -1 : val;
}

static int cmd_wpas_parse_auth_alg(const char *value)
{
	int val = 0, last, errors = 0;
	char *start, *end, *buf;

	buf = os_strdup(value);
	if (buf == NULL)
		return -1;
	start = buf;

	while (*start != '\0') {
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '\0')
			break;
		end = start;
		while (*end != ' ' && *end != '\t' && *end != '\0')
			end++;
		last = *end == '\0';
		*end = '\0';
		if (os_strcmp(start, "OPEN") == 0)
			val |= WPA_AUTH_ALG_OPEN;
		else if (os_strcmp(start, "SHARED") == 0)
			val |= WPA_AUTH_ALG_SHARED;
		else if (os_strcmp(start, "LEAP") == 0)
			val |= WPA_AUTH_ALG_LEAP;
		else if (os_strcmp(start, "SAE") == 0)
			val |= WPA_AUTH_ALG_SAE;
		else {
			os_printf("Invalid auth_alg '%s'\n", start);
			errors++;
		}

		if (last)
			break;
		start = end + 1;
	}
	os_free(buf);

	if (val == 0) {
		os_printf("No auth_alg values configured\n");
		errors++;
	}

	os_printf("auth_alg: 0x%x\n", val);
	return errors ? -1 : val;
}

static void cmd_wlan_sta_print_ap(struct ApListStruct *ap)
{
	os_printf("" MACSTR " ssid=%-32.32s\n",
		MAC2STR(ap->bssid), ap->ssid);
}

static void cmd_wlan_sta_print_scan_results(ScanResult_adv *results)
{
	int i;

	for (i = 0; i < results->ApNum; ++i) {
		os_printf("\n%02d:  ", i + 1);
		cmd_wlan_sta_print_ap(&results->ApList[i]);
	}
}

/* @return
 *   -2: CMD_STATUS_INVALID_ARG
 *   -1: CMD_STATUS_FAIL
 *    0: CMD_STATUS_OK
 */
static int cmd_wlan_sta_set(char *cmd)
{
	char *value;
	wlan_sta_config_t config;

	value = os_strchr(cmd, ' ');
	if (value == NULL)
		return -2;
	*value++ = '\0';

	config.field = WLAN_STA_FIELD_NUM;

	if (os_strcmp(cmd, "ssid") == 0) {
		uint8_t ssid_len = os_strlen(value);
		if ((ssid_len >= 0) && (ssid_len <= WLAN_SSID_MAX_LEN)) {
			config.field = WLAN_STA_FIELD_SSID;
			os_memcpy(config.u.ssid.ssid, value, ssid_len);
			config.u.ssid.ssid_len = ssid_len;
			os_memcpy(g_sta_param_ptr->ssid.array, value, ssid_len);
			g_sta_param_ptr->ssid.length = ssid_len;
		}
	} else if (os_strcmp(cmd, "psk") == 0) {
		uint8_t psk_len = os_strlen(value);
		uint8_t psk_max_len = 64;
#if CFG_WFA_CERT
		psk_max_len = 63;
#endif
		if (psk_len >= 8 && psk_len <= psk_max_len) {
			config.field = WLAN_STA_FIELD_PSK;
			os_strlcpy((char *)config.u.psk, value, sizeof(config.u.psk));
			os_strlcpy((char*)(g_sta_param_ptr->key), value, sizeof(config.u.psk));
			g_sta_param_ptr->key_len = os_strlen(value);
		}
	} else if (os_strcmp(cmd, "wep_key0") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY0;
		os_strlcpy((char *)config.u.wep_key, value, sizeof(config.u.wep_key));
	} else if (os_strcmp(cmd, "wep_key1") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY1;
		os_strlcpy((char *)config.u.wep_key, value, sizeof(config.u.wep_key));
	} else if (os_strcmp(cmd, "wep_key2") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY2;
		os_strlcpy((char *)config.u.wep_key, value, sizeof(config.u.wep_key));
	} else if (os_strcmp(cmd, "wep_key3") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY3;
		os_strlcpy((char *)config.u.wep_key, value, sizeof(config.u.wep_key));
	} else if (os_strcmp(cmd, "wep_key_index") == 0) {
		int index;
		if (cmd_wpas_parse_int(value, 0, 3, &index) == 0) {
			config.field = WLAN_STA_FIELD_WEP_KEY_INDEX;
			config.u.wep_tx_keyidx = index;
		}
	} else if (os_strcmp(cmd, "key_mgmt") == 0) {
		int key_mgmt = cmd_wpas_parse_key_mgmt(value);
		if (key_mgmt > 0) {
			config.field = WLAN_STA_FIELD_KEY_MGMT;
			config.u.key_mgmt = key_mgmt;
		}
	} else if (os_strcmp(cmd, "pairwise") == 0) {
		int pairwise_cipher = cmd_wpas_parse_cipher(value);
		if (pairwise_cipher > 0) {
			config.field = WLAN_STA_FIELD_PAIRWISE_CIPHER;
			config.u.pairwise_cipher = pairwise_cipher;
		}
	} else if (os_strcmp(cmd, "group") == 0) {
		int group_cipher = cmd_wpas_parse_cipher(value);
		if (group_cipher > 0) {
			config.field = WLAN_STA_FIELD_GROUP_CIPHER;
			config.u.group_cipher = group_cipher;
		}
	} else if (os_strcmp(cmd, "proto") == 0) {
		int proto = cmd_wpas_parse_proto(value);
		if (proto >= 0) {
			config.field = WLAN_STA_FIELD_PROTO;
			config.u.proto = proto;
		}
	} else if (os_strcmp(cmd, "auth_alg") == 0) {
		int auth_alg = cmd_wpas_parse_auth_alg(value);
		if (auth_alg > 0) {
			config.field = WLAN_STA_FIELD_AUTH_ALG;
			config.u.auth_alg = auth_alg;
		}
	} else if (os_strcmp(cmd, "ptk_rekey") == 0) {
		int sec;
		if (cmd_wpas_parse_int(value, 0, INT32_MAX, &sec) == 0) {
			config.field = WLAN_STA_FIELD_WPA_PTK_REKEY;
			config.u.wpa_ptk_rekey = sec;
		}
	} else if (os_strcmp(cmd, "scan_ssid") == 0) {
		int enable;
		if (cmd_wpas_parse_int(value, 0, 1, &enable) == 0) {
			config.field = WLAN_STA_FIELD_SCAN_SSID;
			config.u.scan_ssid = enable;
		}
#if CFG_WPA3
	} else if (os_strcmp(cmd, "sae_groups") == 0) {
		int groups[16] = {0};
		int num;
		if ((num = cmd_parse_array_int(value, &groups, ARRAY_SIZE(groups))) > 0) {
			int i;
			int valid = 1;
			for (i = 0; i < num; i++) {
				if (sae_suitable_group(groups[i]) == 0) {
					os_printf("Invalid sae group %d\n", groups[i]);
					valid = 0;
				}
			}
			if (!valid)
				goto out;

			config.field = WLAN_STA_FIELD_SAE_GROUPS;
			os_memset(config.u.sae_groups, 0, sizeof(config.u.sae_groups));
			os_memcpy(config.u.sae_groups, groups, num * sizeof(int));
		}
#endif
	} else if (os_strcmp(cmd, "ieee80211w") == 0) {
		int mfp;
		if (cmd_wpas_parse_int(value, 0, 2, &mfp) == 0) {
			config.field = WLAN_STA_FIELD_MFP;
			config.u.ieee80211w = mfp;
		}
	} else if (os_strcmp(cmd, "debug_level") == 0) {
		int val;
		if (cmd_wpas_parse_int(value, 0, 5, &val) == 0) {
			config.field = WLAN_STA_FIELD_DEBUG_LEVEL;
			config.u.debug_level = val;
		}
	} else if (os_strcmp(cmd, "debug_show_keys") == 0) {
		int val;
		if (cmd_wpas_parse_int(value, 0, 1, &val) == 0) {
			config.field = WLAN_STA_FIELD_DEBUG_SHOW_KEYS;
			config.u.debug_show_keys = val;
		}
	}
	
#if CFG_WPA3
out:
#endif
	if (config.field < WLAN_STA_FIELD_NUM)
		return wlan_sta_set_config(&config);

	os_printf("invalid arg '%s %s'\n", cmd, value);
	return -2;
}

/* @return
 *   -2: CMD_STATUS_INVALID_ARG
 *   -1: CMD_STATUS_FAIL
 *    0: CMD_STATUS_OK
 */
static int cmd_wlan_sta_get(char *cmd)
{
	wlan_sta_config_t config;
	os_memset(&config, 0, sizeof(config));

	if (os_strcmp(cmd, "ssid") == 0) {
		config.field = WLAN_STA_FIELD_SSID;
	} else if (os_strcmp(cmd, "psk") == 0) {
		config.field = WLAN_STA_FIELD_PSK;
	} else if (os_strcmp(cmd, "wep_key0") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY0;
	} else if (os_strcmp(cmd, "wep_key1") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY1;
	} else if (os_strcmp(cmd, "wep_key2") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY2;
	} else if (os_strcmp(cmd, "wep_key3") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY3;
	} else if (os_strcmp(cmd, "wep_key_index") == 0) {
		config.field = WLAN_STA_FIELD_WEP_KEY_INDEX;
	} else if (os_strcmp(cmd, "key_mgmt") == 0) {
		config.field = WLAN_STA_FIELD_KEY_MGMT;
	} else if (os_strcmp(cmd, "pairwise") == 0) {
		config.field = WLAN_STA_FIELD_PAIRWISE_CIPHER;
	} else if (os_strcmp(cmd, "group") == 0) {
		config.field = WLAN_STA_FIELD_GROUP_CIPHER;
	} else if (os_strcmp(cmd, "proto") == 0) {
		config.field = WLAN_STA_FIELD_PROTO;
	} else if (os_strcmp(cmd, "auth_alg") == 0) {
		config.field = WLAN_STA_FIELD_AUTH_ALG;
	} else if (os_strcmp(cmd, "ptk_rekey") == 0) {
		config.field = WLAN_STA_FIELD_WPA_PTK_REKEY;
	} else if (os_strcmp(cmd, "scan_ssid") == 0) {
		config.field = WLAN_STA_FIELD_SCAN_SSID;
	} else {
		os_printf("invalid arg '%s'\n", cmd);
		return -2;
	}

	if (wlan_sta_get_config(&config) != 0) {
		os_printf("get config failed\n");
		return -1;
	}

	if (config.field == WLAN_STA_FIELD_SSID) {
		os_printf("ssid: %.32s\n", config.u.ssid.ssid);
	} else if (config.field == WLAN_STA_FIELD_PSK) {
		os_printf("psk: %s\n", config.u.psk);
	} else if (config.field == WLAN_STA_FIELD_WEP_KEY0) {
		os_printf("wep_key0: %s\n", config.u.wep_key);
	} else if (config.field == WLAN_STA_FIELD_WEP_KEY1) {
		os_printf("wep_key1: %s\n", config.u.wep_key);
	} else if (config.field == WLAN_STA_FIELD_WEP_KEY2) {
		os_printf("wep_key2: %s\n", config.u.wep_key);
	} else if (config.field == WLAN_STA_FIELD_WEP_KEY3) {
		os_printf("wep_key3: %s\n", config.u.wep_key);
	} else if (config.field == WLAN_STA_FIELD_WEP_KEY_INDEX) {
		os_printf("wep_key_index: %d\n", config.u.wep_tx_keyidx);
	} else if (config.field == WLAN_STA_FIELD_KEY_MGMT) {
		os_printf("key_mgmt: %#06x\n", config.u.key_mgmt);
	} else if (config.field == WLAN_STA_FIELD_PAIRWISE_CIPHER) {
		os_printf("pairwise_cipher: %#06x\n", config.u.pairwise_cipher);
	} else if (config.field == WLAN_STA_FIELD_GROUP_CIPHER) {
		os_printf("group_cipher: %#06x\n", config.u.group_cipher);
	} else if (config.field == WLAN_STA_FIELD_PROTO) {
		os_printf("proto: %#06x\n", config.u.proto);
	} else if (config.field == WLAN_STA_FIELD_AUTH_ALG) {
		os_printf("auth_alg: %#06x\n", config.u.auth_alg);
	} else if (config.field == WLAN_STA_FIELD_WPA_PTK_REKEY) {
		os_printf("ptk_rekey: %d\n", config.u.wpa_ptk_rekey);
	} else if (config.field == WLAN_STA_FIELD_SCAN_SSID) {
		os_printf("scan_ssid: %d\n", config.u.scan_ssid);
	}

	return 0;
}

int cmd_wlan_sta_exec(char *cmd)
{
	int ret = 0;

	if (os_strncmp(cmd, "config ", 7) == 0) {
		char *argv[2];
		if (cmd_parse_argv(cmd + 7, argv, ARRAY_SIZE(argv)) == 0) {
			ret = -2;
			goto out;
		}
		ret = wlan_sta_set((uint8_t *)argv[0], os_strlen(argv[0]), (uint8_t *)argv[1]);
	} else if (os_strncmp(cmd, "set ", 4) == 0) {
		ret = cmd_wlan_sta_set(cmd + 4);
	} else if (os_strncmp(cmd, "get ", 4) == 0) {
		ret = cmd_wlan_sta_get(cmd + 4);
	} else if (os_strcmp(cmd, "enable") == 0) {
		/* default value for WFA certification */

		network_InitTypeDef_st wNetConfig;
		int len;
		char *oob_ssid = "Wi-Fi";
		char *connect_key = "12345678";

		os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

		os_strcpy((char *)wNetConfig.wifi_ssid, oob_ssid);
		os_strcpy((char *)wNetConfig.wifi_key, connect_key);

		wNetConfig.wifi_mode = BK_STATION;
		wNetConfig.dhcp_mode = DHCP_CLIENT;
		wNetConfig.wifi_retry_interval = 100;

		bk_wlan_sta_init(&wNetConfig);
		ret = wlan_sta_enable();
	} else if (os_strcmp(cmd, "disable") == 0) {
		ret = wlan_sta_disable();
	} else if (os_strcmp(cmd, "scan once") == 0) {
		ret = wlan_sta_scan_once();
	} else if (os_strncmp(cmd, "scan result ", 12) == 0) {
		int size;
		if (cmd_wpas_parse_int(cmd + 12, 1, CMD_WLAN_MAX_BSS_CNT, &size) != 0) {
			ret = -2;
			goto out;
		}
		ScanResult_adv results;
		ret = wlan_sta_scan_result(&results);
		if (ret == 0)
			cmd_wlan_sta_print_scan_results(&results);
		os_free(results.ApList);
	} else if (os_strncmp(cmd, "scan interval ", 14) == 0) {
		int sec;
		if (cmd_wpas_parse_int(cmd + 14, 0, INT32_MAX, &sec) != 0) {
			ret = -2;
			goto out;
		}
		ret = wlan_sta_scan_interval(sec);
	} else if (os_strncmp(cmd, "bss max count ", 14) == 0) {
		int count;
		if (cmd_wpas_parse_int(cmd + 14, 1, CMD_WLAN_MAX_BSS_CNT, &count) != 0) {
			ret = -2;
			goto out;
		}
		ret = wlan_sta_bss_max_count((uint8_t)count);
	} else if (os_strncmp(cmd, "bss flush ", 10) == 0) {
		int age;
		if (cmd_wpas_parse_int(cmd + 10, 0, INT32_MAX, &age) != 0) {
			ret = -2;
			goto out;
		}
		ret = wlan_sta_bss_flush(age);
	} else if (os_strcmp(cmd, "connect") == 0) {

		ret = wlan_sta_connect();
		ip_address_set(BK_STATION, DHCP_CLIENT, NULL, NULL, NULL, NULL);

	} else if (os_strcmp(cmd, "disconnect") == 0) {
		sta_ip_down();	// XXX: WLAN_DISCONNECT_EVENT may handle this
		ret = wlan_sta_disconnect();

		rtos_delay_milliseconds(500);

	} else if (os_strcmp(cmd, "state") == 0) {
		wlan_sta_states_t state;
		ret = wlan_sta_state(&state);
		if (ret == 0)
			os_printf("sta state: %d\n", state);
	} else if (os_strcmp(cmd, "ap") == 0) {
		struct ApListStruct *ap = os_malloc(sizeof(*ap));
		if (ap == NULL) {
			os_printf("no mem\n");
			ret = -1;
			goto out;
		}
		ret = wlan_sta_ap_info(ap);
		if (ret == 0)
			cmd_wlan_sta_print_ap(ap);
		os_free(ap);
	} else if (os_strncmp(cmd, "genpsk ", 7) == 0) {
		uint8_t i;
		char *argv[2];
		wlan_gen_psk_param_t param;

		if (cmd_parse_argv(cmd + 7, argv, ARRAY_SIZE(argv)) != 2) {
			ret = -2;
			goto out;
		}
		param.ssid_len = os_strlen(argv[0]);
		os_memcpy(param.ssid, argv[0], param.ssid_len);
		os_strlcpy(param.passphrase, argv[1], sizeof(param.passphrase));
		ret = wlan_sta_gen_psk(&param);
		if (ret == 0) {
			os_printf("psk: ");
			for (i = 0; i < sizeof(param.psk); ++i)
				os_printf("%02x", param.psk[i]);
			os_printf("\n");
		}
	} else if (os_strcmp(cmd, "wps pbc") == 0) {
		ret = wlan_sta_wps_pbc();
	} else if ((os_strlen(cmd) == 7) || (os_strcmp(cmd, "wps pin") == 0)) {
		wlan_sta_wps_pin_t wps;
		ret = wlan_sta_wps_pin_get(&wps);
		if (ret == 0)
			os_printf("WPS pin: %s\n", wps.pin);
	} else if (os_strncmp(cmd, "wps pin ", 8) == 0) {
		if (os_strlen(cmd + 8) != 8) {
			ret = -2;
			goto out;
		}
		wlan_sta_wps_pin_t wps;
		os_memcpy(wps.pin, cmd + 8, 8);
		wps.pin[8] = '\0';
		ret = wlan_sta_wps_pin_set(&wps);
	} else {
		os_printf("unknown cmd '%s'\n", cmd);
		return -1;
	}

out:
	if (ret == 0) {
		os_printf("[OK]\n");
	} else if (ret == -2) {
		os_printf("cmd '%s' invalid arg\n", cmd);
		return -1;
	} else if (ret == -1) {
		os_printf("cmd '%s' exec failed\n", cmd);
		return -1;
	}

	return 0;
}

static void cmd_wlan_ap_print_sta_info(wlan_ap_stas_t *stas)
{
	int i;

	os_printf("sta_num: %d\n", stas->num);

	for (i = 0; i < stas->num; i++) {
		os_printf("[%02d]Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
			i + 1, stas->sta[i].addr[0], stas->sta[i].addr[1],
			stas->sta[i].addr[2], stas->sta[i].addr[3],
			stas->sta[i].addr[4], stas->sta[i].addr[5]);
	}
}

/* @return
 *   -2: CMD_STATUS_INVALID_ARG
 *   -1: CMD_STATUS_FAIL
 *    0: CMD_STATUS_OK
 */
static int cmd_wlan_ap_set(char *cmd)
{
	char *value;
	wlan_ap_config_t config;

	value = os_strchr(cmd, ' ');
	if (value == NULL)
		return -2;
	*value++ = '\0';

	config.field = WLAN_AP_FIELD_NUM;

	if (os_strcmp(cmd, "ssid") == 0) {
		uint8_t ssid_len = os_strlen(value);
		if ((ssid_len >= 1) && (ssid_len <= 32)) {
			config.field = WLAN_STA_FIELD_SSID;
			os_memcpy(config.u.ssid.ssid, value, ssid_len);
			config.u.ssid.ssid_len = ssid_len;

			os_memcpy(g_ap_param_ptr->ssid.array, value, ssid_len);
			g_ap_param_ptr->ssid.length = ssid_len;
		}
	} else if (os_strcmp(cmd, "psk") == 0) {
		int len = strlen(value);
		if (len <8 || len > 63)
			goto out;
		config.field = WLAN_AP_FIELD_PSK;
		os_strlcpy((char *)config.u.psk, value, sizeof(config.u.psk));
	} else if (os_strcmp(cmd, "key_mgmt") == 0) {
		int key_mgmt = cmd_wpas_parse_key_mgmt(value);
		if (key_mgmt > 0) {
			config.field = WLAN_AP_FIELD_KEY_MGMT;
			config.u.key_mgmt = key_mgmt;
		}
		if ((key_mgmt & (WPA_KEY_MGMT_SAE | WPA_KEY_MGMT_PSK)) == (WPA_KEY_MGMT_SAE | WPA_KEY_MGMT_PSK))
			g_ap_param_ptr->cipher_suite = BK_SECURITY_TYPE_WPA3_WPA2_MIXED;
		else if (key_mgmt & WPA_KEY_MGMT_SAE)
			g_ap_param_ptr->cipher_suite = BK_SECURITY_TYPE_WPA3_SAE;
		else if (key_mgmt & WPA_KEY_MGMT_PSK)
			g_ap_param_ptr->cipher_suite = BK_SECURITY_TYPE_WPA2_AES;
		else
			g_ap_param_ptr->cipher_suite = 0;
		//os_printf("key_mgmt: %d\n", g_ap_param_ptr->cipher_suite);
	} else if (os_strcmp(cmd, "wpa") == 0) {
		int wpa_cipher = cmd_wpas_parse_cipher(value);
		if (wpa_cipher > 0) {
			config.field = WLAN_AP_FIELD_WPA_CIPHER;
			config.u.wpa_cipher = wpa_cipher;
		}
	} else if (os_strcmp(cmd, "rsn") == 0) {
		int rsn_cipher = cmd_wpas_parse_cipher(value);
		if (rsn_cipher > 0) {
			config.field = WLAN_AP_FIELD_RSN_CIPHER;
			config.u.rsn_cipher = rsn_cipher;
		}
	} else if (os_strcmp(cmd, "proto") == 0) {
		int proto = cmd_wpas_parse_proto(value);
		if (proto >= 0) {
			config.field = WLAN_AP_FIELD_PROTO;
			config.u.proto = proto;
		}
	} else if (os_strcmp(cmd, "auth_alg") == 0) {
		int auth_alg = cmd_wpas_parse_auth_alg(value);
		if (auth_alg > 0) {
			config.field = WLAN_AP_FIELD_AUTH_ALG;
			config.u.auth_alg = auth_alg;
		}
	} else if (os_strcmp(cmd, "group_rekey") == 0) {
		int group_rekey;
		if (cmd_wpas_parse_int(value, 0, INT32_MAX, &group_rekey) == 0) {
			config.field = WLAN_AP_FIELD_GROUP_REKEY;
			config.u.group_rekey = group_rekey;
		}
	} else if (os_strcmp(cmd, "strict_rekey") == 0) {
		int strict_rekey;
		if (cmd_wpas_parse_int(value, 0, 1, &strict_rekey) == 0) {
			config.field = WLAN_AP_FIELD_STRICT_REKEY;
			config.u.strict_rekey = strict_rekey;
		}
	} else if (os_strcmp(cmd, "gmk_rekey") == 0) {
		int gmk_rekey;
		if (cmd_wpas_parse_int(value, 0, INT32_MAX, &gmk_rekey) == 0) {
			config.field = WLAN_AP_FIELD_GMK_REKEY;
			config.u.gmk_rekey = gmk_rekey;
		}
	} else if (os_strcmp(cmd, "ptk_rekey") == 0) {
		int ptk_rekey;
		if (cmd_wpas_parse_int(value, 0, INT32_MAX, &ptk_rekey) == 0) {
			config.field = WLAN_AP_FIELD_PTK_REKEY;
			config.u.ptk_rekey = ptk_rekey;
		}
	} else if (os_strcmp(cmd, "hw_mode") == 0) {
		if ((value[0] == 'b') && (value[1] == '\0')) {
			config.field = WLAN_AP_FIELD_HW_MODE;
			config.u.hw_mode = WLAN_AP_HW_MODE_IEEE80211B;
		} else if ((value[0] == 'g') && (value[1] == '\0')) {
			config.field = WLAN_AP_FIELD_HW_MODE;
			config.u.hw_mode = WLAN_AP_HW_MODE_IEEE80211G;
		}
	} else if (os_strcmp(cmd, "80211n") == 0) {
		int ieee80211n;
		if (cmd_wpas_parse_int(value, 0, 1, &ieee80211n) == 0) {
			config.field = WLAN_AP_FIELD_IEEE80211N;
			config.u.ieee80211n = ieee80211n;
		}
	} else if (os_strcmp(cmd, "channel") == 0) {
		int channel;
		if (cmd_wpas_parse_int(value, 1, 14, &channel) == 0) {
			config.field = WLAN_AP_FIELD_CHANNEL;
			config.u.channel = channel;
			g_ap_param_ptr->chann = channel;
		}
	} else if (os_strcmp(cmd, "beacon_int") == 0) {
		int beacon_int;
		if (cmd_wpas_parse_int(value, 15, 65535, &beacon_int) == 0) {
			config.field = WLAN_AP_FIELD_BEACON_INT;
			config.u.beacon_int = beacon_int;
		}
	} else if (os_strcmp(cmd, "dtim") == 0) {
		int dtim;
		if (cmd_wpas_parse_int(value, 1, 255, &dtim) == 0) {
			config.field = WLAN_AP_FIELD_DTIM;
			config.u.dtim = dtim;
		}
	} else if (os_strcmp(cmd, "max_num_sta") == 0) {
		int max_num_sta;
		if (cmd_wpas_parse_int(value, 0, INT32_MAX, &max_num_sta) == 0) {
			config.field = WLAN_AP_FIELD_MAX_NUM_STA;
			config.u.max_num_sta = max_num_sta;
		}
	}

	if (config.field < WLAN_AP_FIELD_NUM)
		return wlan_ap_set_config(&config);

out:
	os_printf("invalid arg '%s %s'\n", cmd, value);
	return -2;
}

/* @return
 *   -2: CMD_STATUS_INVALID_ARG
 *   -1: CMD_STATUS_FAIL
 *    0: CMD_STATUS_OK
 */
static int cmd_wlan_ap_get(char *cmd)
{
	wlan_ap_config_t config;
	os_memset(&config, 0, sizeof(config));

	if (os_strcmp(cmd, "ssid") == 0) {
		config.field = WLAN_AP_FIELD_SSID;
	} else if (os_strcmp(cmd, "psk") == 0) {
		config.field = WLAN_AP_FIELD_PSK;
	} else if (os_strcmp(cmd, "key_mgmt") == 0) {
		config.field = WLAN_AP_FIELD_KEY_MGMT;
	} else if (os_strcmp(cmd, "wpa") == 0) {
		config.field = WLAN_AP_FIELD_WPA_CIPHER;
	} else if (os_strcmp(cmd, "rsn") == 0) {
		config.field = WLAN_AP_FIELD_RSN_CIPHER;
	} else if (os_strcmp(cmd, "proto") == 0) {
		config.field = WLAN_AP_FIELD_PROTO;
	} else if (os_strcmp(cmd, "auth_alg") == 0) {
		config.field = WLAN_AP_FIELD_AUTH_ALG;
	} else if (os_strcmp(cmd, "group_rekey") == 0) {
		config.field = WLAN_AP_FIELD_GROUP_REKEY;
	} else if (os_strcmp(cmd, "strict_rekey") == 0) {
		config.field = WLAN_AP_FIELD_STRICT_REKEY;
	} else if (os_strcmp(cmd, "gmk_rekey") == 0) {
		config.field = WLAN_AP_FIELD_GMK_REKEY;
	} else if (os_strcmp(cmd, "ptk_rekey") == 0) {
		config.field = WLAN_AP_FIELD_PTK_REKEY;
	} else if (os_strcmp(cmd, "hw_mode") == 0) {
		config.field = WLAN_AP_FIELD_HW_MODE;
	} else if (os_strcmp(cmd, "80211n") == 0) {
		config.field = WLAN_AP_FIELD_IEEE80211N;
	} else if (os_strcmp(cmd, "channel") == 0) {
		config.field = WLAN_AP_FIELD_CHANNEL;
	} else if (os_strcmp(cmd, "beacon_int") == 0) {
		config.field = WLAN_AP_FIELD_BEACON_INT;
	} else if (os_strcmp(cmd, "dtim") == 0) {
		config.field = WLAN_AP_FIELD_DTIM;
	} else if (os_strcmp(cmd, "max_num_sta") == 0) {
		config.field = WLAN_AP_FIELD_MAX_NUM_STA;
	} else {
		os_printf("invalid arg '%s'\n", cmd);
		return -2;
	}

	if (wlan_ap_get_config(&config) != 0) {
		os_printf("get config failed\n");
		return -1;
	}

	if (config.field == WLAN_AP_FIELD_SSID) {
		os_printf("ssid: %.32s\n", config.u.ssid.ssid);
	} else if (config.field == WLAN_AP_FIELD_PSK) {
		os_printf("psk: %s\n", config.u.psk);
	} else if (config.field == WLAN_AP_FIELD_KEY_MGMT) {
		os_printf("key_mgmt: %#06x\n", config.u.key_mgmt);
	} else if (config.field == WLAN_AP_FIELD_WPA_CIPHER) {
		os_printf("wpa_cipher: %#06x\n", config.u.wpa_cipher);
	} else if (config.field == WLAN_AP_FIELD_RSN_CIPHER) {
		os_printf("rsn_cipher: %#06x\n", config.u.rsn_cipher);
	} else if (config.field == WLAN_AP_FIELD_PROTO) {
		os_printf("proto: %#06x\n", config.u.proto);
	} else if (config.field == WLAN_AP_FIELD_AUTH_ALG) {
		os_printf("auth_alg: %#06x\n", config.u.auth_alg);
	} else if (config.field == WLAN_AP_FIELD_GROUP_REKEY) {
		os_printf("group_rekey: %d\n", config.u.group_rekey);
	} else if (config.field == WLAN_AP_FIELD_STRICT_REKEY) {
		os_printf("strict_rekey: %d\n", config.u.strict_rekey);
	} else if (config.field == WLAN_AP_FIELD_GMK_REKEY) {
		os_printf("gmk_rekey: %d\n", config.u.gmk_rekey);
	} else if (config.field == WLAN_AP_FIELD_PTK_REKEY) {
		os_printf("ptk_rekey: %d\n", config.u.ptk_rekey);
	} else if (config.field == WLAN_AP_FIELD_HW_MODE) {
		if (config.u.hw_mode == WLAN_AP_HW_MODE_IEEE80211B) {
			os_printf("hw_mode: b\n");
		} else if (config.u.hw_mode == WLAN_AP_HW_MODE_IEEE80211G) {
			os_printf("hw_mode: g\n");
		} else if (config.u.hw_mode == WLAN_AP_HW_MODE_IEEE80211A) {
			os_printf("hw_mode: a\n");
		} else if (config.u.hw_mode == WLAN_AP_HW_MODE_IEEE80211AD) {
			os_printf("hw_mode: ad\n");
		} else {
			os_printf("invalid hw_mode %d\n", config.u.hw_mode);
		}
	} else if (config.field == WLAN_AP_FIELD_IEEE80211N) {
		os_printf("ieee80211n: %d\n", config.u.ieee80211n);
	} else if (config.field == WLAN_AP_FIELD_CHANNEL) {
		os_printf("channel: %d\n", config.u.channel);
	} else if (config.field == WLAN_AP_FIELD_BEACON_INT) {
		os_printf("beacon_int: %d\n", config.u.beacon_int);
	} else if (config.field == WLAN_AP_FIELD_DTIM) {
		os_printf("dtim: %d\n", config.u.dtim);
	} else if (config.field == WLAN_AP_FIELD_MAX_NUM_STA) {
		os_printf("max_num_sta: %d\n", config.u.max_num_sta);
	}

	return 0;
}

int cmd_wlan_ap_exec(char *cmd)
{
	int ret = 0;

	if (os_strncmp(cmd, "config ", 7) == 0) {
		char *argv[2];
		if (cmd_parse_argv(cmd + 7, argv, ARRAY_SIZE(argv)) == 0) {
			ret = -2;
			goto out;
		}
		ret = wlan_ap_set((uint8_t *)argv[0], os_strlen(argv[0]), (uint8_t *)argv[1]);
	} else if (os_strncmp(cmd, "set ", 4) == 0) {
		if (!g_ap_param_ptr) {
			os_printf("enable AP first\n");
			goto out;
		}
		ret = cmd_wlan_ap_set(cmd + 4);
	} else if (os_strncmp(cmd, "get ", 4) == 0) {
		ret = cmd_wlan_ap_get(cmd + 4);
	} else if (os_strcmp(cmd, "enable") == 0) {
		network_InitTypeDef_ap_st wNetConfig;

		/* stop lwip netif */
		uap_ip_down();

		/* set AP parameter, ssid, akm, etc. */
		os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));
		os_strcpy((char *)wNetConfig.wifi_ssid, "Wi-Fi");
		os_strcpy((char *)wNetConfig.wifi_key, "12345678");
		wNetConfig.channel = 6;
		wNetConfig.security = BK_SECURITY_TYPE_WPA3_SAE;
		os_strcpy((char *)wNetConfig.local_ip_addr, WLAN_DEFAULT_IP);
		os_strcpy((char *)wNetConfig.net_mask, WLAN_DEFAULT_MASK);
		os_strcpy((char *)wNetConfig.gateway_ip_addr, WLAN_DEFAULT_GW);
		os_strcpy((char *)wNetConfig.dns_server_ip_addr, WLAN_DEFAULT_GW);
		wNetConfig.dhcp_mode = DHCP_SERVER;
		wNetConfig.wifi_retry_interval = 100;
		bk_wlan_ap_init_adv(&wNetConfig);

		// enable hostapd
		ret = wlan_ap_enable();
	} else if (os_strcmp(cmd, "reload") == 0 || os_strcmp(cmd, "start") == 0) {
		ret = wlan_ap_reload();

		/* now ap has started, set ip address to this interface */
	    ip_address_set(BK_SOFT_AP,
	                   DHCP_SERVER,
	                   WLAN_DEFAULT_IP,
	                   WLAN_DEFAULT_MASK,
	                   WLAN_DEFAULT_GW,
	                   WLAN_DEFAULT_GW);

		/* restart lwip network */
	    uap_ip_start();

	} else if (os_strcmp(cmd, "disable") == 0) {
		ret = wlan_ap_disable();
	} else if (os_strcmp(cmd, "sta num") == 0) {
		int num;
		ret = wlan_ap_sta_num(&num);
		if (ret == 0)
			os_printf("sta num: %d\n", num);
	} else if (os_strncmp(cmd, "sta info ", 9) == 0) {
		int size;
		if (cmd_wpas_parse_int(cmd + 9, 1, 30, &size) != 0) {
			ret = -2;
			goto out;
		}
		wlan_ap_stas_t stas;
		stas.sta = (wlan_ap_sta_t *)os_malloc(size * sizeof(wlan_ap_sta_t));
		if (stas.sta == NULL) {
			os_printf("no mem\n");
			ret = -1;
			goto out;
		}
		stas.size = size;
		ret = wlan_ap_sta_info(&stas);
		if (ret == 0)
			cmd_wlan_ap_print_sta_info(&stas);
		os_free(stas.sta);
	} else {
		os_printf("unknown cmd '%s'\n", cmd);
		return -1;
	}

out:
	if (ret == 0) {
		os_printf("[OK]\n");
	} else if (ret == -2) {
		os_printf("cmd '%s' invalid arg\n", cmd);
		return -1;
	} else if (ret == -1) {
		os_printf("cmd '%s' exec failed\n", cmd);
		return -1;
	}

	return 0;
}

void net_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	char buf[128];
	int i, left = sizeof(buf) - 1, len = 0;

	// net sta xxx
	// net ap xxx
	if (argc <= 2) {
		os_printf("Usage: net sta/ap <param...>\n");
		return;
	}

	buf[0] = 0;
	for (i = 2; i < argc; i++) {
		len = os_strlen(buf);
		snprintf(buf + len, left - len, "%s ", argv[i]);
	}
	buf[strlen(buf)-1] = 0;
	//os_printf("CMD: |%s|\n", buf);

#if 1
	if (os_strcmp(argv[1], "sta") == 0) {
		cmd_wlan_sta_exec(buf);
	} else if (os_strcmp(argv[1], "ap") == 0) {
		cmd_wlan_ap_exec(buf);
	} else {
		os_printf("Usage: net sta/ap <param...>\n");
		return;
	}
#endif
}
#endif

