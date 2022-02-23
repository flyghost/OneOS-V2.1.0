#include "includes.h"
#include "wpa_psk_cache.h"
#include "uart_pub.h"
#include "../wpa_supplicant/config.h"

#ifdef CONFIG_WPA_PSK_CACHE
#include "crypto/sha1.h"

void start_wpa_psk_cal_thread();

beken_thread_t wpa_pskcalc_thread_handle = NULL;

struct wpa_psk_cache *psk_cache;

void wpa_psk_cache_init()
{
	psk_cache = os_zalloc(sizeof(*psk_cache));	
	if (psk_cache) {
#ifdef CONFIG_WPA_PSK_CACHE_MULTI
		dl_list_init(&psk_cache->all);
		dl_list_init(&psk_cache->pending);
#endif
		rtos_init_semaphore(&psk_cache->sema, 1);	//TODO: error check
		rtos_set_semaphore(&psk_cache->sema);
	}
}

#ifdef CONFIG_WPA_PSK_CACHE_MULTI
/* make cache entries less than WPA_PSK_ENTRIES */
static void wpa_psk_cache_expire(struct wpa_psk_cache *cache)
{
	struct wpa_psk_cache_item *item;
	while (dl_list_len(&cache->all) > WPA_PSK_ENTRIES) {
		item = dl_list_first(&cache->all, struct wpa_psk_cache_item, node);
		dl_list_del(&item->node);	// delete first entry
		os_free(item->passphrase);
		os_free(item->ssid);
		os_printf("vanish one\n");
	}
}

static struct wpa_psk_cache_item *wpa_psk_cache_get_from_q(struct dl_list *list, u8 *ssid, size_t ssid_len, char *passphrase)
{
	struct wpa_psk_cache_item *item = NULL;

	dl_list_for_each(item, list, struct wpa_psk_cache_item, node) {

		if (ssid_len != item->ssid_len)
			continue;

		if (os_memcmp(ssid, item->ssid, item->ssid_len))
			continue;
		/* ssid the same */

		if (os_strcmp(passphrase, item->passphrase))
			continue;

		/* found */
		return item;
	}

	return NULL;
}

/*
 * caller must hold cache->sema.
 */
struct wpa_psk_cache_item *__wpa_psk_cache_get(struct wpa_psk_cache *cache,
		u8 *ssid, size_t ssid_len, char *passphrase)
{
	struct wpa_psk_cache_item *item = NULL;

	item = wpa_psk_cache_get_from_q(&cache->all, ssid, ssid_len, passphrase);
	if (item)
		return item;
	item = wpa_psk_cache_get_from_q(&cache->pending, ssid, ssid_len, passphrase);
	return item;
}


int __wpa_get_psk_from_cache(u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	struct wpa_psk_cache_item *item;
	struct wpa_psk_cache *cache = psk_cache;

	//os_printf("ssid: %s, passphase: %s\r\n", wpa_ssid_txt(ssid, ssid_len), passphrase);

	rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);

	item = __wpa_psk_cache_get(cache, ssid, ssid_len, passphrase);
	if (item && (item->flags & WPA_PSK_CACHE_FLAG_COMPLETE)) {
		/* copy psk to ssid->psk */
		os_memcpy(psk, item->psk, sizeof(item->psk) > psk_len ? psk_len : sizeof(item->psk));

		wpa_hexdump_key(MSG_MSGDUMP, "PSK (from cache)", psk, PMK_LEN);

		rtos_set_semaphore(&cache->sema);

		return 0;
	} else if (item) {
		rtos_set_semaphore(&cache->sema);
		return 1;
	}

	rtos_set_semaphore(&cache->sema);
	return -1;
}

int wpa_get_psk_from_cache(struct wpa_ssid *ssid)
{
	return __wpa_get_psk_from_cache(ssid->ssid, ssid->ssid_len,
			ssid->passphrase, ssid->psk, sizeof(ssid->psk));
}

/*
 * Add psk to cache.
 * caller must hold cache->sema.
 */
static int ___wpa_psk_cache_add(struct wpa_psk_cache *cache,
		u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	struct wpa_psk_cache_item *item;

	/* add a new one */
	item = (struct wpa_psk_cache_item *)os_zalloc(sizeof(*item));
	if (item) {
		item->ssid = dup_binstr(ssid, ssid_len);
		item->ssid_len = ssid_len;
		item->passphrase = os_strdup(passphrase);
		if (!item->ssid || !item->passphrase) {
			os_free(item->ssid);
			os_free(item->passphrase);
			os_free(item);
			goto out;
		}

		if (psk) {
			/* copy calculated psk */
			os_memcpy(item->psk, psk, psk_len);
			item->flags = WPA_PSK_CACHE_FLAG_COMPLETE;
		} else {
			item->flags = WPA_PSK_CACHE_FLAG_PENDING;
		}
		dl_list_add_tail(&cache->all, &item->node);

		return 1;
	}

out:

	return -1;
}

/*
 * Add psk to cache.
 * caller must hold cache->sema.
 * return: 1 new added, 0 already added, -1 error
 */
static int __wpa_psk_cache_add(struct wpa_psk_cache *cache,
		u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	struct wpa_psk_cache_item *item;

	item = __wpa_psk_cache_get(cache, ssid, ssid_len, passphrase);
	if (item)	// already exist
		return 0;

	return ___wpa_psk_cache_add(cache, ssid, ssid_len, passphrase, psk, psk_len);
}

int wpa_psk_request(u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	int new_added;
	struct wpa_psk_cache *cache = psk_cache;

	rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);
	new_added = __wpa_psk_cache_add(cache, ssid, ssid_len, passphrase, psk, psk_len);
	wpa_psk_cache_expire(cache);
	rtos_set_semaphore(&cache->sema);

	if (new_added > 0)
		start_wpa_psk_cal_thread();	// new added, run thread

	return 0;
}

/* traverse all pending psk caculation */
void wpa_psk_cal_thread(void *arg)
{
	struct wpa_psk_cache *cache = psk_cache;
	struct wpa_psk_cache_item *item, *n;
	int count;

	while (1) {

		count = 0;

		// move all pending node to internal pending queue.
		rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);
		dl_list_for_each_safe(item, n, &cache->all, struct wpa_psk_cache_item, node) {
			if (item->flags & WPA_PSK_CACHE_FLAG_COMPLETE)
				continue;
			dl_list_del(&item->node);
			//dl_list_add_tail(&cache->pending, &item->node);
			dl_list_add(&cache->pending, &item->node);	// add to head, LIFO
			count++;

#ifndef CONFIG_WPA_PSK_CALCU_MULTI
			break;	// always process one, comment out it to process more
#endif
		}
		rtos_set_semaphore(&cache->sema);

		/* no more psk caculation pending */
		if (!count)
			break;

		// caculate psk on pending list
		dl_list_for_each_safe(item, n, &cache->pending, struct wpa_psk_cache_item, node) {
			os_printf("PSKC: start\n");
			pbkdf2_sha1(item->passphrase, (u8 *)item->ssid, item->ssid_len, 4096, item->psk, sizeof(item->psk));
			os_printf("PSKC: end\n");

			// requeue to complete list.
			rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);
			item->flags = WPA_PSK_CACHE_FLAG_COMPLETE;
			dl_list_del(&item->node);
			dl_list_add_tail(&cache->all, &item->node);
			wpa_psk_cache_expire(cache);
			rtos_set_semaphore(&cache->sema);
		}
	}
	wpa_pskcalc_thread_handle = 0;

	rtos_delete_thread(NULL);	/* Delete thread */
}

#else	/* !CONFIG_WPA_PSK_CACHE_MULTI */

int __wpa_get_psk_from_cache(u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	struct wpa_psk_cache *cache = psk_cache;

	rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);

	if (cache->item.flags == WPA_PSK_CACHE_FLAG_COMPLETE) {
		os_memcpy(psk, cache->item.psk, psk_len);
		rtos_set_semaphore(&cache->sema);
		return 0;
	}

	rtos_set_semaphore(&cache->sema);

	// not available
	return 1;
}

int wpa_get_psk_from_cache(struct wpa_ssid *ssid)
{
	return __wpa_get_psk_from_cache(ssid->ssid, ssid->ssid_len,
			ssid->passphrase, ssid->psk, sizeof(ssid->psk));
}

/**
 * request a psk from cache, or starts a new psk calculation task
 * @ssid: ssid
 * @ssid_len: length of ssid
 * @passphrase: passphrase or hex string of psk (in case of fast connect)
 * @psk: not used 
 * @psk_len: not used
 *
 * returns: 1 if psk has already or in calculation, 
 *          0 if psk not found and request PSKC task to calculate it.
 *         -1 error.
 */
int wpa_psk_request(u8 *ssid, size_t ssid_len, char *passphrase, u8 *psk, size_t psk_len)
{
	struct wpa_psk_cache *cache = psk_cache;
	int complete = 0;
	
	rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);

	if (ssid_len == cache->item.ssid_len &&
		os_memcmp(ssid, cache->item.ssid, ssid_len) == 0 &&
		os_strcmp(passphrase, cache->item.passphrase) == 0) {

		/* same, in calcuation process or have been calcuated */
		rtos_set_semaphore(&cache->sema);
		return 1;
	}
		
	/* a new request */
	os_free(cache->item.ssid);
	os_free(cache->item.passphrase);

	if (os_strlen(passphrase) == 2 * PMK_LEN) {
		/* passphrase is hex string of psk, convert it to binary */
		if (!hexstr2bin(passphrase, cache->item.psk, PMK_LEN))
			complete = 1;
	}

	if (psk && psk_len) {
		if (psk_len == PMK_LEN) {
			/* binary format of PMK */
			os_memcpy(cache->item.psk, psk, psk_len);
			complete = 1;
		} else if (psk_len == 2 * PMK_LEN) {
			/* hex string of PMK */
			if (!hexstr2bin((char *)psk, cache->item.psk, PMK_LEN))
				complete = 1;
		}
	}

	cache->item.flags = complete ? WPA_PSK_CACHE_FLAG_COMPLETE : WPA_PSK_CACHE_FLAG_PENDING;

	cache->item.ssid = dup_binstr(ssid, ssid_len);
	cache->item.ssid_len = ssid_len;
	cache->item.passphrase = os_strdup(passphrase);

	ASSERT(cache->item.ssid && cache->item.passphrase);
	
	rtos_set_semaphore(&cache->sema);

	if (!complete)
		start_wpa_psk_cal_thread();

	return 0;
}

void wpa_psk_cal_thread(void *arg)
{
	struct wpa_psk_cache *cache = psk_cache;
	char *ssid;
	size_t ssid_len;
	char *passphrase;
	u8 psk[PMK_LEN];
	int done;
    GLOBAL_INT_DECLARATION();

	while (1) {

		done = 0;
	
		/* copy to temporay value */
		rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);
		ssid = dup_binstr(cache->item.ssid, cache->item.ssid_len);
		ssid_len = cache->item.ssid_len;
		passphrase = os_strdup(cache->item.passphrase);
		cache->item.flags = WPA_PSK_CACHE_FLAG_PENDING;
		rtos_set_semaphore(&cache->sema);

		if (!ssid || !passphrase) {
			os_free(ssid);
			os_free(passphrase);
			break;
		}

		os_printf("PSKC: ssid %s, passphrase %s\n", wpa_ssid_txt((uint8_t*)ssid, ssid_len), passphrase);
		pbkdf2_sha1(passphrase, (u8 *)ssid, ssid_len, 4096, psk, sizeof(psk));
		os_printf("PSKC: end\n");

		/* determine ssid & passphrase have been changed */
		rtos_get_semaphore(&cache->sema, BEKEN_WAIT_FOREVER);
		if (ssid_len == cache->item.ssid_len &&
			os_memcmp(ssid, cache->item.ssid, ssid_len) == 0 &&
			os_strcmp(passphrase, cache->item.passphrase) == 0) {
			os_memcpy(cache->item.psk, psk, sizeof(cache->item.psk));
			cache->item.flags = WPA_PSK_CACHE_FLAG_COMPLETE;
			done = 1;
		}
		
		rtos_set_semaphore(&cache->sema);

		os_free(ssid);
		os_free(passphrase);

		if (done)
			break;
	}

    GLOBAL_INT_DISABLE();
	wpa_pskcalc_thread_handle = 0;
    GLOBAL_INT_RESTORE();
	rtos_delete_thread(NULL);	/* Delete thread */
}
#endif

void start_wpa_psk_cal_thread()
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

	if (!wpa_pskcalc_thread_handle) {
		rtos_create_thread(&wpa_pskcalc_thread_handle, THD_WPAS_PRIORITY,	/* lower or equal than wpas thread */
				 "pskc",	/* psk calcuation task*/
				 (beken_thread_function_t)wpa_psk_cal_thread,
				 2048,
				 (beken_thread_arg_t)0);
	}

    GLOBAL_INT_RESTORE();
}

#endif //CONFIG_WPA_PSK_CACHE
