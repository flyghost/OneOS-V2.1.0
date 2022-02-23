/*
 * WPA Supplicant / Configuration backend: empty starting point
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file implements dummy example of a configuration backend. None of the
 * functions are actually implemented so this can be used as a simple
 * compilation test or a starting point for a new configuration backend.
 */

#include "includes.h"

#include "common.h"
#include "config.h"
#include "base64.h"
#include "param_config.h"
#include "uart_pub.h"
#include "wlan_ui_pub.h"
#include "wpa_common.h"
#include "rw_pub.h"

#if CFG_IEEE80211N
static void wpa_config_ht_cap_by_sec(int sec)
{
	if(sec == BK_SECURITY_TYPE_WEP
		|| sec == BK_SECURITY_TYPE_WPA_TKIP
		|| sec == BK_SECURITY_TYPE_WPA2_TKIP)
	{
		rw_ieee80211_set_ht_cap(0);
	}
	else
	{
		rw_ieee80211_set_ht_cap(1);
	}
}
#endif

static int wpa_config_validate_network(struct wpa_ssid *ssid, int line)
{
	int errors = 0;

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
		!(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
		!(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Line %d: removed CCMP from group cipher"
				   " list since it was not allowed for pairwise "
				   "cipher", line);
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	if (ssid->mode == WPAS_MODE_MESH &&
		(ssid->key_mgmt != WPA_KEY_MGMT_NONE &&
		 ssid->key_mgmt != WPA_KEY_MGMT_SAE)) {
		wpa_printf(MSG_ERROR,
				   "Line %d: key_mgmt for mesh network should be open or SAE",
				   line);
		errors++;
	}

	return errors;
}

static int set_wpa_psk(struct wpa_ssid *ssid)
{
	int errors = 0;

	if (g_sta_param_ptr->key_len < 8 || g_sta_param_ptr->key_len > 64) {
		os_printf("Invalid passphrase "
				  "length %lu (expected: 8..63) '%s'.",
				  (unsigned long) g_sta_param_ptr->key_len, (char *)g_sta_param_ptr->key);
		errors++;

		return 1;
	} else if (g_sta_param_ptr->key_len == 64) {
		wpa_printf(MSG_ERROR, "use PSK");
		if (hexstr2bin((char*)(g_sta_param_ptr->key), ssid->psk, 32) ||
			g_sta_param_ptr->key[64] != '\0') {
			wpa_printf(MSG_ERROR, "Invalid PSK '%s'.",
					   g_sta_param_ptr->key);
			errors++;
		}
#if !CFG_NEW_SUPP
		ssid->passphrase = NULL;
#endif
		ssid->psk_set = 1;
	} else {
#if !CFG_NEW_SUPP
		str_clear_free(ssid->passphrase);
		ssid->passphrase = dup_binstr(g_sta_param_ptr->key, g_sta_param_ptr->key_len);
		ssid->psk_set = 0;
#endif
	}
	ssid->key_mgmt |= WPA_KEY_MGMT_PSK;
	ssid->mem_only_psk = 0;

	return errors;
}

static int set_wep_key(struct wpa_ssid *ssid)
{
	int errors = 0;

	ssid->wep_tx_keyidx = 0;
	ssid->auth_alg = WPA_AUTH_ALG_OPEN | WPA_AUTH_ALG_SHARED;
	if (g_sta_param_ptr->key_len == 5 ||
		g_sta_param_ptr->key_len == 13) {
		os_memcpy(ssid->wep_key[0], (char *)g_sta_param_ptr->key, g_sta_param_ptr->key_len);
		ssid->wep_key_len[0] = g_sta_param_ptr->key_len;
	} else if (g_sta_param_ptr->key_len == 10 ||
			   g_sta_param_ptr->key_len == 26) {
		ssid->wep_key_len[0] = g_sta_param_ptr->key_len / 2;
		hexstr2bin((char*)(g_sta_param_ptr->key), ssid->wep_key[0], ssid->wep_key_len[0]);
	} else {
		errors++;
	}
	ssid->mem_only_psk = 0;
	return errors;
}

static int cipher2security(struct wpa_ie_data *ie)
{
	bk_printf("%s %d %d %d %d\n", __FUNCTION__, ie->key_mgmt, ie->proto, ie->pairwise_cipher, ie->group_cipher);
	switch (ie->key_mgmt) {
	case WPA_KEY_MGMT_PSK:
		if (ie->proto == WPA_PROTO_WPA) {
			if (ie->pairwise_cipher & WPA_CIPHER_CCMP)
				return BK_SECURITY_TYPE_WPA_AES;
			else
				return BK_SECURITY_TYPE_WPA_TKIP;
		} else if (ie->proto == (WPA_PROTO_WPA | WPA_PROTO_RSN)) {
			return BK_SECURITY_TYPE_WPA2_MIXED;
		} else {
			if (ie->pairwise_cipher == (WPA_CIPHER_CCMP | WPA_CIPHER_TKIP))
				return BK_SECURITY_TYPE_WPA2_MIXED;
			else if (ie->pairwise_cipher & WPA_CIPHER_CCMP)
				return BK_SECURITY_TYPE_WPA2_AES;
			else
				return BK_SECURITY_TYPE_WPA2_TKIP;
		}
		break;
	case WPA_KEY_MGMT_NONE:
		if (ie->pairwise_cipher == WPA_CIPHER_NONE)
			return BK_SECURITY_TYPE_NONE;
		else
			return BK_SECURITY_TYPE_WEP;
		break;
	default:
		return BK_SECURITY_TYPE_AUTO;
	}
}

static int security2cipher(struct wpa_ie_data *ie, int security)
{
	os_memset(ie, 0, sizeof(*ie));

	switch (security) {
	case BK_SECURITY_TYPE_NONE:
		ie->key_mgmt = WPA_KEY_MGMT_NONE;
		ie->pairwise_cipher = WPA_CIPHER_NONE;
		break;
	case BK_SECURITY_TYPE_WEP:
		ie->key_mgmt = WPA_KEY_MGMT_NONE;
		ie->pairwise_cipher = WPA_CIPHER_WEP40 | WPA_CIPHER_WEP104;
		break;
	case BK_SECURITY_TYPE_WPA_TKIP:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA;
		break;

	case BK_SECURITY_TYPE_WPA2_TKIP:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_RSN;
		break;
	case BK_SECURITY_TYPE_WPA_AES:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP | WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA;
		break;
	case BK_SECURITY_TYPE_WPA2_AES:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_CCMP;
		ie->proto = WPA_PROTO_RSN;
		break;
	case BK_SECURITY_TYPE_WPA2_MIXED:
		ie->key_mgmt = WPA_KEY_MGMT_PSK;
		ie->pairwise_cipher = WPA_CIPHER_TKIP | WPA_CIPHER_CCMP;
		ie->group_cipher = WPA_CIPHER_TKIP;
		ie->proto = WPA_PROTO_WPA | WPA_PROTO_RSN;
		break;
	default:
		break;
	}
	bk_printf("%s %d %d %d %d security=%d\n", __FUNCTION__, ie->key_mgmt, ie->proto, ie->pairwise_cipher, ie->group_cipher, security);

	return 0;
}

int wpa_config_set_none(struct wpa_ssid *ssid)
{
	ssid->key_mgmt = WPA_KEY_MGMT_NONE;
	ssid->mem_only_psk = 0;
	ssid->auth_alg = 0;
	g_sta_param_ptr->cipher_suite = BK_SECURITY_TYPE_NONE;

	return 0;
}

int wpa_config_set_wep(struct wpa_ssid *ssid)
{
	int ret;

	ret = set_wep_key(ssid);
	if (!ret){
		g_sta_param_ptr->cipher_suite = BK_SECURITY_TYPE_WEP;
	}else{
		mhdr_set_station_status(RW_EVT_STA_PASSWORD_WRONG);
	}

	return 0;
}

int wpa_config_set_wpa(struct wpa_ssid *ssid, struct wpa_ie_data *ie)
{
	int ret;

	ssid->group_cipher = ie->group_cipher;
	ssid->pairwise_cipher = ie->pairwise_cipher;
	ssid->key_mgmt = ie->key_mgmt;
	ssid->proto = ie->proto;
	ssid->auth_alg = 0;	// let supplicant choose automatically.

#if !CFG_NEW_SUPP
	if ((ssid->psk_set) || (ssid->passphrase != NULL))
		return 0;
#else
	if (ssid->psk_set)
		return 0;
#endif /* CFG_NEW_SUPP */

	ret = set_wpa_psk(ssid);
	if (!ret) {
		g_sta_param_ptr->cipher_suite = cipher2security(ie);
		#if CFG_IEEE80211N
		wpa_config_ht_cap_by_sec(g_sta_param_ptr->cipher_suite);
		#endif
		
		if (ssid->passphrase && (ssid->psk_set == 0)) {
			wpa_config_update_psk(ssid);
			ssid->psk_set = 1;
		}
	}else{
		mhdr_set_station_status(RW_EVT_STA_PASSWORD_WRONG);
	}

	return ret;
}

static struct wpa_ssid *wpa_config_read_network(int *line, int id)
{
	struct wpa_ssid *ssid;
	struct wpa_ie_data ie;
	int errors = 0;

	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;

	dl_list_init(&ssid->psk_list);
	ssid->id = id;

	wpa_config_set_network_defaults(ssid);

#if !CFG_NEW_SUPP
	ssid->ssid = (uint8_t*)dup_binstr(g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length);
	ssid->ssid_len = g_sta_param_ptr->ssid.length;
	ssid->key_mgmt = 0;

	if ((g_sta_param_ptr->fast_connect_set) && (g_sta_param_ptr->cipher_suite != BK_SECURITY_TYPE_AUTO)) {
		os_memcpy(ssid->bssid, g_sta_param_ptr->fast_connect.bssid, 6);
		bk_printf("bssid %02x-%02x-%02x-%02x-%02x-%02x\r\n",
				  ssid->bssid[0], ssid->bssid[1], ssid->bssid[2],
				  ssid->bssid[3], ssid->bssid[4], ssid->bssid[5]);
		ssid->bssid_set = 1;
		if (g_sta_param_ptr->cipher_suite == BK_SECURITY_TYPE_NONE) {
			/*do nothing*/
		}else if(g_sta_param_ptr->cipher_suite == BK_SECURITY_TYPE_WEP){
			errors += wpa_config_set_wep(ssid);
		}else{
			#if CFG_IEEE80211N
			wpa_config_ht_cap_by_sec(g_sta_param_ptr->cipher_suite);
			#endif
			security2cipher(&ie, g_sta_param_ptr->cipher_suite);
			wpa_config_set_wpa(ssid, &ie);
		}
	} else {
		ssid->mem_only_psk = 1;
	}

#if CFG_SUPPORT_BSSID_CONNECT
	if (0 == ssid->ssid_len) {
		os_memcpy(ssid->bssid, g_sta_param_ptr->fast_connect.bssid, sizeof(ssid->bssid));
		ssid->bssid_set = 1;
	}
#endif

	errors += wpa_config_validate_network(ssid, *line);

	if (errors) {
		wpa_config_free_ssid(ssid);
		ssid = NULL;
	}
#else /* CFG_NEW_SUPP */
	ssid->disabled = 1;		/* disable scan @ wpa_supplicant_driver_init */
#ifdef CONFIG_IEEE80211W
	ssid->ieee80211w = MGMT_FRAME_PROTECTION_OPTIONAL;
#endif

#if CFG_WFA_CERT
	ssid->ssid = (u8 *)dup_binstr("Wi-Fi", 5);
	ssid->ssid_len = 5;
	ssid->passphrase = os_strdup("12345678");
	ssid->ieee80211w = MGMT_FRAME_PROTECTION_REQUIRED;
	ssid->scan_ssid = 1;
	ssid->mem_only_psk = 1;
#endif

#endif

	return ssid;
}

struct wpa_config *wpa_config_read(const char *name, struct wpa_config *cfgp)
{
	struct wpa_config *config;
	struct wpa_ssid *ssid, *tail, *head;
	int id = 0, line = 0, errors = 0;

	if (cfgp)
		config = cfgp;
	else
		config = wpa_config_alloc_empty(NULL, NULL);
	if (config == NULL)
		return NULL;
	tail = head = config->ssid;
	while (tail && tail->next)
		tail = tail->next;

	ssid = wpa_config_read_network(&line, id++);
	if (ssid == NULL) {
		errors++;
		goto error;
	}
	if (head == NULL) {
		head = tail = ssid;
	} else {
		tail->next = ssid;
		tail = ssid;
	}
	if (wpa_config_add_prio_network(config, ssid)) {
		errors++;
		goto error;
	}

	config->ssid = head;
	wpa_config_debug_dump_networks(config);
#if CFG_NEW_SUPP
	config->auth_dur = 3;
	//config->pmf = MGMT_FRAME_PROTECTION_OPTIONAL;
#endif

error:
	if (errors) {
		wpa_config_free(config);
		config = NULL;
		head = NULL;
	}
	return config;
}


int wpa_config_write(const char *name, struct wpa_config *config)
{
	//struct wpa_ssid *ssid;
	//struct wpa_config_blob *blob;

	wpa_printf(MSG_DEBUG, "Writing configuration file '%s'", name);

	/* TODO: write global config parameters */


#if 0
	for (ssid = config->ssid; ssid; ssid = ssid->next) {
		/* TODO: write networks */
	}

	for (blob = config->blobs; blob; blob = blob->next) {
		/* TODO: write blobs */
	}
#endif

	return 0;
}
// eof


