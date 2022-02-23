#include "oneos_config.h"
#include "py/mpconfig.h"
/*2021-6-16:目前底层驱动的wifi扫描接口未提供，暂不适配。*/
#if MICROPY_PY_WIFISCAN
#include <stdlib.h>
#include "os_util.h"
#include "os_errno.h"
#include "py/runtime.h"

static int wifiscan_ap_compare(const void *ctx1, const void *ctx2)
{
    const wifi_ap_info_t *w1 = (const wifi_ap_info_t *)ctx1;
    const wifi_ap_info_t *w2 = (const wifi_ap_info_t *)ctx2;
    return (w2->rssival - w1->rssival);
}

static int mod_scan_result_sort(void)
{
    
}

static mp_obj_t mod_scan_channel(size_t n_args, const mp_obj_t *args)
{
    int max = 10;
    int timeout = 500;
    int channel = 1;
    os_err_t result;

    if(n_args == 3){
        max = mp_obj_get_int(args[0]);
        timeout = mp_obj_get_int(args[1]);
        channel = mp_obj_get_int(args[2]);
        if(max < 1 || timeout < 100 || (channel < 1 || channel > 13 )){
            mp_raise_ValueError("wrong params");
            return mp_const_none;
        }
    }
    else{
        mp_raise_ValueError("wrong params");
        return mp_const_none;
    }

    wifi_scan_request_t req;
    mp_obj_t ap_list = mp_obj_new_list(0, NULL);
    wifi_ap_info_t *aps = (wifi_ap_info_t *)os_malloc(max * sizeof(wifi_ap_info_t));
    char tmp_mac[18] = {0};

    req.found = 0;
    req.max = max;
    req.maxtimeout = timeout;
    req.aps = aps;

    result = uis8910_wrap_wifi_open();
    if (OS_EOK == result)
    {
        os_task_mdelay(500);
        uis8910_wrap_wifi_scan_channel(&req, channel);
        if (req.found > 0){
            qsort(&req.aps[0], req.found, sizeof(wifi_ap_info_t), wifiscan_ap_compare);
            for (int j = 0; j < req.found; j++){
                wifi_ap_info_t *nap = &(req.aps[j]);
                os_uint8_t tmp_bss[6];
                tmp_bss[0] = (nap->bssid_high >> 8) & 0xFF;
                tmp_bss[1] = nap->bssid_high & 0xFF;
                tmp_bss[2] = (nap->bssid_low >> 24) & 0xFF;
                tmp_bss[3] = (nap->bssid_low >> 16) & 0xFF;
                tmp_bss[4] = (nap->bssid_low >> 8) & 0xFF;
                tmp_bss[5] = nap->bssid_low & 0xFF;
                sprintf(tmp_mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
                        tmp_bss[0], tmp_bss[1], tmp_bss[2], tmp_bss[3], tmp_bss[4], tmp_bss[5]);
                mp_obj_t tuple[3] = {
                    mp_obj_new_str(tmp_mac, 17),
                    MP_OBJ_NEW_SMALL_INT(nap->channel),
                    MP_OBJ_NEW_SMALL_INT(nap->rssival),
                };
                mp_obj_list_append(ap_list, mp_obj_new_tuple(3, tuple));
            }
        }
        os_task_mdelay(50);
        uis8910_wrap_wifi_close();
        return ap_list;
    }
    else{
        mp_raise_ValueError("wifi open err");
        return mp_const_none;
    }

}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_scan_channel_obj, 3, 3, mod_scan_channel);

static mp_obj_t mod_scan_all(size_t n_args, const mp_obj_t *args)
{
    int max = 10;
    int timeout = 500;
    os_err_t result;

    if(n_args == 2){
        max = mp_obj_get_int(args[0]);
        timeout = mp_obj_get_int(args[1]);
        if(max < 1 || timeout < 100){
            mp_raise_ValueError("wrong params");
            return mp_const_none;
        }
    }
    else{
        mp_raise_ValueError("wrong params");
        return mp_const_none;
    }

    wifi_scan_request_t reqs[13];
    mp_obj_t ap_list = mp_obj_new_list(0, NULL);
    char tmp_mac[18] = {0};

    result = uis8910_wrap_wifi_open();
    if (OS_EOK == result)
    {
        os_task_mdelay(500);
        for (int i = 0; i < 13; i++)
        {
            wifi_ap_info_t *aps = (wifi_ap_info_t *)os_malloc(max * sizeof(wifi_ap_info_t));
            wifi_scan_request_t *req = &reqs[i];
            req->aps = aps;
            req->found = 0;
            req->max = max;
            req->maxtimeout = timeout;
            uis8910_wrap_wifi_scan_channel(req, i+1);

            if(req->found > 0)
            {
                qsort(&req->aps[0], req->found, sizeof(wifi_ap_info_t), wifiscan_ap_compare);
                for (int j = 0; j < req->found; j++){
                    wifi_ap_info_t *nap = &(req->aps[j]);
                    os_uint8_t tmp_bss[6];
                    tmp_bss[0] = (nap->bssid_high >> 8) & 0xFF;
                    tmp_bss[1] = nap->bssid_high & 0xFF;
                    tmp_bss[2] = (nap->bssid_low >> 24) & 0xFF;
                    tmp_bss[3] = (nap->bssid_low >> 16) & 0xFF;
                    tmp_bss[4] = (nap->bssid_low >> 8) & 0xFF;
                    tmp_bss[5] = nap->bssid_low & 0xFF;
                    sprintf(tmp_mac, "%02x:%02x:%02x:%02x:%02x:%02x", 
                            tmp_bss[0], tmp_bss[1], tmp_bss[2], tmp_bss[3], tmp_bss[4], tmp_bss[5]);
                    mp_obj_t tuple[3] = {
                        mp_obj_new_str(tmp_mac, 17),
                        MP_OBJ_NEW_SMALL_INT(nap->channel),
                        MP_OBJ_NEW_SMALL_INT(nap->rssival),
                    };
                    mp_obj_list_append(ap_list, mp_obj_new_tuple(3, tuple));
                }
            }
            os_task_mdelay(50);
        }
        os_task_mdelay(50);
        uis8910_wrap_wifi_close();
        return ap_list;
    }
    else{
        mp_raise_ValueError("wifi open err");
        return mp_const_none;
    }
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_scan_all_obj, 2, 2, mod_scan_all);

STATIC mp_obj_t mod_wifiscan___init__(void)
{
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_wifiscan___init___obj, mod_wifiscan___init__);

STATIC const mp_rom_map_elem_t mp_module_wifiscan_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_wifiscan) },
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&mod_wifiscan___init___obj) },
    { MP_ROM_QSTR(MP_QSTR_scan_channel), MP_ROM_PTR(&mod_scan_channel_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan_all), MP_ROM_PTR(&mod_scan_all_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_wifiscan_globals, mp_module_wifiscan_globals_table);

const mp_obj_module_t mp_module_wifiscan = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_wifiscan_globals,
};

#endif
