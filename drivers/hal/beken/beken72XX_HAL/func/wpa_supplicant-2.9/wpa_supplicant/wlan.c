#include "includes.h"

#include "wpa_ctrl.h"
#include "wlan_defs.h"
#include "wlan_ui_pub.h"
#include "notifier.h"
#include "mem_pub.h"
#include "str_pub.h"

#if CFG_NEW_SUPP
#define WLAN_CHECK_ARG	0

#define WLAN_ERR bk_printf

#if WLAN_CHECK_ARG
#define WLAN_ASSERT_POINTER(p)                  \
    do {                                        \
        if (p == NULL) {                        \
            WLAN_ERR("invalid param\n");        \
            return -1;                          \
        }                                       \
    } while (0)
#else
#define WLAN_ASSERT_POINTER(p)	do { } while (0)
#endif

/**
 * @brief Configure station in a convenient way to join a specified network
 * @param[in] ssid Network name, length is [1, 32]
 * @param[in] ssid_len The length of network name
 * @param[in] psk Network password, in one of the optional formats:
 *            - NULL or an empty string, to join an OPEN network
 *            - an ASCII string, length is [8, 63]
 *            - a hex string (two characters per octet of PSK), length is 64
 * @return 0 on success, -1 on failure
 *
 * @note This way is only adapted to join an OPEN, WPA-PSK, WPA2-PSK or
 *       WPA-PSK/WPA2-PSK network.
 */
int wlan_sta_set(uint8_t *ssid, uint8_t ssid_len, uint8_t *psk)
{
	if ((ssid == NULL) || (ssid_len == 0) || (ssid_len > WLAN_SSID_MAX_LEN)) {
		WLAN_ERR("invalid ssid (%p, %u)\n", ssid, ssid_len);
		return -1;
	}

	wlan_sta_config_t config;
	os_memset(&config, 0, sizeof(config));

	/* ssid */
	config.field = WLAN_STA_FIELD_SSID;
	os_memcpy(config.u.ssid.ssid, ssid, ssid_len);
	config.u.ssid.ssid_len = ssid_len;
	if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
		return -1;

#if 0	/* let supplicant choose auth_alg automatically */
	/* auth_alg: OPEN */
	config.field = WLAN_STA_FIELD_AUTH_ALG;
	config.u.auth_alg = WPA_AUTH_ALG_OPEN;
	if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
		return -1;
#endif

	if ((psk == NULL) || (psk[0] == '\0')) {
#if 0
		/* psk */
		config.field = WLAN_STA_FIELD_PSK;
		config.u.psk[0] = '\0';
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;
#endif

		/* key_mgmt: NONE */
		config.field = WLAN_STA_FIELD_KEY_MGMT;
		config.u.key_mgmt = WPA_KEY_MGMT_NONE;
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;
	} else {
		/* psk */
		config.field = WLAN_STA_FIELD_PSK;
		os_strlcpy((char *)config.u.psk, (char *)psk, sizeof(config.u.psk));
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;

		/* key_mgmt: PSK */
		config.field = WLAN_STA_FIELD_KEY_MGMT;
		config.u.key_mgmt = WPA_KEY_MGMT_PSK;
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;

		/* proto: WPA | RSN */
		config.field = WLAN_STA_FIELD_PROTO;
		config.u.proto = WPA_PROTO_WPA | WPA_PROTO_RSN;
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;

		/* pairwise: CCMP | TKIP */
		config.field = WLAN_STA_FIELD_PAIRWISE_CIPHER;
		config.u.pairwise_cipher = WPA_CIPHER_CCMP | WPA_CIPHER_TKIP;
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;

		/* group: CCMP | TKIP | WEP40 | WEP104 */
		config.field = WLAN_STA_FIELD_GROUP_CIPHER;
		config.u.pairwise_cipher = WPA_CIPHER_CCMP | WPA_CIPHER_TKIP
					   | WPA_CIPHER_WEP40 | WPA_CIPHER_WEP104;
		if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
			return -1;
	}



	/* scan_ssid: 1 */
	config.field = WLAN_STA_FIELD_SCAN_SSID;
	config.u.scan_ssid = 1;
	if (wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, &config) != 0)
		return -1;

	return 0;
}

/**
 * @brief Set station specified field configuration
 * @param[in] config Pointer to the configuration
 * @return 0 on success, -1 on failure
 */
int wlan_sta_set_config(wlan_sta_config_t *config)
{
	WLAN_ASSERT_POINTER(config);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_SET, config);
}

/**
 * @brief Get station specified field configuration
 * @param[in] config Pointer to the configuration
 * @return 0 on success, -1 on failure
 */
int wlan_sta_get_config(wlan_sta_config_t *config)
{
	WLAN_ASSERT_POINTER(config);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_GET, config);
}

/**
 * @brief Set autoconnect after bss lost configuration
 * @param[in] config enable or disable autoconnect function
 * @return 0 on success, -1 on failure
 */
int wlan_sta_set_autoconnect(int enable)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_AUTOCONNECT, (void *)enable);
}

/**
 * @brief Get the information size of current bss
 * @param[in] config Pointer to the information size
 * @return 0 on success, -1 on failure
 */
int wlan_sta_get_bss_size(uint32_t * size)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_BSS_SIZE_GET, size);
}

/**
 * @brief Get the information of current bss
 * @param[in] config Pointer to the information
 * @return 0 on success, -1 on failure
 */
int wlan_sta_get_bss(wlan_sta_bss_info_t * bss_get)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_BSS_GET, bss_get);
}

/**
 * @brief Set the information of bss which will be used
 * @param[in] config Pointer to the information of bss
 * @return 0 on success, -1 on failure
 */
int wlan_sta_set_bss(wlan_sta_bss_info_t * bss_set)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_BSS_SET, bss_set);
}

/**
 * @brief Enable the station
 * @return 0 on success, -1 on failure
 */
int wlan_sta_enable(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_ENABLE, NULL);
}

/**
 * @brief Disable the station
 * @return 0 on success, -1 on failure
 */
int wlan_sta_disable(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_DISABLE, NULL);
}

/**
 * @brief Station scan once according to the default parameters
 * @return 0 on success, -1 on failure
 */
int wlan_sta_scan_once(void)
{
	int ret;
	while (1) {
		ret = wpa_ctrl_request(WPA_CTRL_CMD_STA_SCAN, NULL);
		if (ret == -2) {
			//bk_printf("previous scan in progress, retry...\n");
			rtos_delay_milliseconds(30);
			continue;
		} else {
			break;
		}
	}

	return 0;
}

/**
 * @brief Station scan once according to the specified parameters
 * @return 0 on success, -1 on failure
 */
int wlan_sta_scan(wlan_sta_scan_param_t *param)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_SCAN, param);
}

/**
 * @brief Get station scan results
 * @param[in] results Pointer to the scan results
 * @return 0 on success, -1 on failure
 */
int wlan_sta_scan_result(ScanResult_adv *results)
{
	WLAN_ASSERT_POINTER(results);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_SCAN_RESULTS, results);
}

/**
 * @brief Set station scan interval
 * @param[in] sec Scan interval in Seconds
 * @return 0 on success, -1 on failure
 */
int wlan_sta_scan_interval(int sec)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_SCAN_INTERVAL, (void *)sec);
}

/**
 * @brief Set maximum BSS entries to keep in memory
 * @param[in] count Maximum BSS entries to keep in memory
 * @return 0 on success, -1 on failure
 */
int wlan_sta_bss_max_count(uint8_t count)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_BSS_MAX_COUNT,
	                        (void *)(uint32_t)count);
}

/**
 * @brief Flush station old BSS entries
 * @param[in] age Maximum entry age in seconds
 * @return 0 on success, -1 on failure
 *
 * @note Remove BSS entries that have not been updated during the last @age
 * seconds.
 */
int wlan_sta_bss_flush(int age)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_BSS_FLUSH, (void *)age);
}

/**
 * @brief Request a new connection
 * @return 0 on success, -1 on failure
 */
int wlan_sta_connect(void)
{
	//return wpa_ctrl_request(WPA_CTRL_CMD_STA_CONNECT, NULL);
	int id = 0;
	return wpa_ctrl_request(WPA_CTRL_CMD_SELECT_NETWORK, (void *)id);
}

/**
 * @brief Disconnect the current connection
 * @return 0 on success, -1 on failure
 */
int wlan_sta_disconnect(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_DISCONNECT, NULL);
}

/**
 * @brief Get station connection state
 * @param[in] state Pointer to the connection state
 * @return 0 on success, -1 on failure
 */
int wlan_sta_state(wlan_sta_states_t *state)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_STATE, state);
}

/**
 * @brief Get the information of connected AP
 * @param[in] ap Pointer to the AP information
 * @return 0 on success, -1 on failure
 */
int wlan_sta_ap_info(struct ApListStruct *ap)
{
	WLAN_ASSERT_POINTER(ap);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_AP, ap);
}

/**
 * @brief Generate WPA PSK based on passphrase and SSID
 * @param[in] param Pointer to wlan_gen_psk_param_t structure
 * @return 0 on success, -1 on failure
 */
int wlan_sta_gen_psk(wlan_gen_psk_param_t *param)
{
	WLAN_ASSERT_POINTER(param);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_GEN_PSK, param);
}

/**
 * @brief Start the WPS negotiation with PBC method
 * @return 0 on success, -1 on failure
 *
 * @note WPS will be turned off automatically after two minutes.
 */
int wlan_sta_wps_pbc(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_STA_WPS_PBC, NULL);
}

/**
 * @brief Get a random valid WPS PIN
 * @param[in] wps Pointer to the WPS pin
 * @return 0 on success, -1 on failure
 */
int wlan_sta_wps_pin_get(wlan_sta_wps_pin_t *wps)
{
	WLAN_ASSERT_POINTER(wps);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_WPS_GET_PIN, wps);
}

/**
 * @brief Start the WPS negotiation with PIN method
 * @param[in] wps Pointer to the WPS pin
 * @return 0 on success, -1 on failure
 *
 * @note WPS will be turned off automatically after two minutes.
 */
int wlan_sta_wps_pin_set(wlan_sta_wps_pin_t *wps)
{
	WLAN_ASSERT_POINTER(wps);

	return wpa_ctrl_request(WPA_CTRL_CMD_STA_WPS_SET_PIN, wps);
}

/**
 * @brief Configure AP in a convenient way to build a specified network
 * @param[in] ssid Network name, length is [1, 32]
 * @param[in] ssid_len The length of network name
 * @param[in] psk Network password, in one of the optional formats:
 *            - NULL or an empty string, to build an OPEN network
 *            - an ASCII string, length is [8, 63]
 *            - a hex string (two characters per octet of PSK), length is 64
 * @return 0 on success, -1 on failure
 *
 * @note This way is only adapted to build an OPEN or WPA-PSK/WPA2-PSK network.
 */
int wlan_ap_set(uint8_t *ssid, uint8_t ssid_len, uint8_t *psk)
{
	if ((ssid == NULL) || (ssid_len == 0) || (ssid_len > WLAN_SSID_MAX_LEN)) {
		WLAN_ERR("invalid ssid (%p, %u)\n", ssid, ssid_len);
		return -1;
	}

	wlan_ap_config_t config;
	os_memset(&config, 0, sizeof(config));

	/* ssid */
	config.field = WLAN_AP_FIELD_SSID;
	os_memcpy(config.u.ssid.ssid, ssid, ssid_len);
	config.u.ssid.ssid_len = ssid_len;
	if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
		return -1;

	/* auth_alg: OPEN */
	config.field = WLAN_AP_FIELD_AUTH_ALG;
	config.u.auth_alg = WPA_AUTH_ALG_OPEN;
	if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
		return -1;

	if ((psk == NULL) || (psk[0] == '\0')) {
		/* proto: 0 */
		config.field = WLAN_AP_FIELD_PROTO;
		config.u.proto = 0;
		if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
			return -1;

		/* key_mgmt: NONE */
		config.field = WLAN_AP_FIELD_KEY_MGMT;
		config.u.key_mgmt = WPA_KEY_MGMT_NONE;
		if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
			return -1;
	} else {
		/* psk */
		config.field = WLAN_AP_FIELD_PSK;
		os_strlcpy((char *)config.u.psk, (char *)psk, sizeof(config.u.psk));
		if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
			return -1;

		/* proto: WPA | RSN */
		config.field = WLAN_AP_FIELD_PROTO;
		config.u.proto = WPA_PROTO_WPA | WPA_PROTO_RSN;
		if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
			return -1;

		/* key_mgmt: PSK */
		config.field = WLAN_AP_FIELD_KEY_MGMT;
		config.u.key_mgmt = WPA_KEY_MGMT_PSK;
		if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
			return -1;
	}

	/* wpa_cipher: TKIP */
	config.field = WLAN_AP_FIELD_WPA_CIPHER;
	config.u.wpa_cipher = WPA_CIPHER_TKIP;
	if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
		return -1;

	/* rsn_cipher: CCMP */
	config.field = WLAN_AP_FIELD_RSN_CIPHER;
	config.u.rsn_cipher = WPA_CIPHER_CCMP;
	if (wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, &config) != 0)
		return -1;

	return 0;
}

/**
 * @brief Set AP specified field configuration
 * @param[in] config Pointer to the configuration
 * @return 0 on success, -1 on failure
 */
int wlan_ap_set_config(wlan_ap_config_t *config)
{
	WLAN_ASSERT_POINTER(config);

	return wpa_ctrl_request(WPA_CTRL_CMD_AP_SET, config);
}

/**
 * @brief Get AP specified field configuration
 * @param[in] config Pointer to the configuration
 * @return 0 on success, -1 on failure
 */
int wlan_ap_get_config(wlan_ap_config_t *config)
{
	WLAN_ASSERT_POINTER(config);

	return wpa_ctrl_request(WPA_CTRL_CMD_AP_GET, config);
}

/**
 * @brief Enable the AP
 * @return 0 on success, -1 on failure
 */
int wlan_ap_enable(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_AP_ENABLE, NULL);
}

/**
 * @brief Reload AP configuration
 * @return 0 on success, -1 on failure
 */
int wlan_ap_reload(void)
{
	int ret;
	while (1) {
		ret = wpa_ctrl_request(WPA_CTRL_CMD_AP_RELOAD, NULL);
		if (ret == -2) {
			// busy, csa in progress
			rtos_delay_milliseconds(50);
		} else {
			return ret;
		}
	}
}

/**
 * @brief Disable the AP
 * @return 0 on success, -1 on failure
 */
int wlan_ap_disable(void)
{
	return wpa_ctrl_request(WPA_CTRL_CMD_AP_DISABLE, NULL);
}

/**
 * @brief Get the number of connected stations
 * @param[in] num Pointer to the number
 * @return 0 on success, -1 on failure
 */
int wlan_ap_sta_num(int *num)
{
	WLAN_ASSERT_POINTER(num);

	return wpa_ctrl_request(WPA_CTRL_CMD_AP_STA_NUM, num);
}

/**
 * @brief Get the information of connected stations
 * @param[in] stas Pointer to the stations information
 * @return 0 on success, -1 on failure
 */
int wlan_ap_sta_info(wlan_ap_stas_t *stas)
{
	WLAN_ASSERT_POINTER(stas);

	return wpa_ctrl_request(WPA_CTRL_CMD_AP_STA_INFO, stas);
}

int wlan_register_notifier(notify_func func, void *arg)
{
	struct notifier_req req;

	req.func = func;
	req.arg = arg;

	return wpa_ctrl_request(WPA_CTLR_CMD_ADD_NOTIFIER, &req);
}

int wlan_unregister_notifier(notify_func func, void *arg)
{
	struct notifier_req req;

	req.func = func;
	req.arg = arg;

	return wpa_ctrl_request(WPA_CTLR_CMD_REMOVE_NOTIFIER, &req);
}
#endif //CFG_NEW_SUPP

