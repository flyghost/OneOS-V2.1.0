#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/mphal.h"
#include "usr_misc.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/netutils/netutils.h"
#include "modnetwork.h"

#if MICROPY_PY_MO_NETWORK
#include "mo_api.h"
/*
typedef struct netif_obj_t {
    mp_obj_base_t base;
} netif_obj_t;

const mp_obj_type_t netif_type;
STATIC const netif_obj_t netif_obj = {{&netif_type}};
*/

#ifndef ERR_OK
#define ERR_OK      0
#endif

STATIC mp_obj_t mo_register(void) {
    mo_object_t    *module = mo_get_default();
    eps_reg_info_t  info;

    if(ERR_OK != mo_get_reg(module, &info))
        return mp_const_false;

    return MP_OBJ_NEW_SMALL_INT(info.reg_stat);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_register_obj, mo_register);


STATIC mp_obj_t mo_active(size_t n_args, const mp_obj_t *args) {
    mo_object_t    *module = mo_get_default();
    os_uint8_t      cid; 
    os_uint8_t      act_stat;
    mp_obj_dict_t  *act_dic;

    /*get cgact stat*/
    if (0 == n_args) 
    {
        if(ERR_OK != mo_get_cgact(module, &cid, &act_stat))
            return mp_const_false;
        else
        {
            act_dic = MP_OBJ_TO_PTR(mp_obj_new_dict(MICROPY_MODULE_DICT_SIZE));            
            mp_obj_dict_store(MP_OBJ_FROM_PTR(act_dic), MP_OBJ_NEW_SMALL_INT(cid), MP_OBJ_NEW_SMALL_INT(act_stat));

            return MP_OBJ_FROM_PTR(act_dic);
        }
    }
    /*set cgact stat*/
    else if(1 == n_args)
    {
        act_stat = mp_obj_get_int(args[0]);
        cid = 1;
    }
    else
    {
        act_stat = mp_obj_get_int(args[0]);
        cid =  mp_obj_get_int(args[1]);
    }
    
    if(ERR_OK != mo_set_cgact(module, cid, act_stat))
        return mp_const_false;
    else
        return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mo_active_obj, 0, 2, mo_active);

STATIC mp_obj_t mo_cfun(size_t n_args, const mp_obj_t *args) {
    mo_object_t    *module = mo_get_default();
    os_uint8_t      fun_lvl;
    
    /*get cfun stat*/
    if (0 == n_args) 
    {
        if(ERR_OK != mo_get_cfun(module, &fun_lvl))
            return mp_const_false;
        else
            return MP_OBJ_NEW_SMALL_INT(fun_lvl);
    }
    /*set cfun stat*/
    else 
    {
        if(ERR_OK != mo_set_cfun(module, mp_obj_get_int(args[0])))
            return mp_const_false;
        else
            return mp_const_true;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mo_cfun_obj, 0, 1, mo_cfun);

STATIC mp_obj_t mo_ipaddr(void) {
    mo_object_t    *module = mo_get_default();
    char            ipaddr[32]={0};

    if(ERR_OK != mo_get_ipaddr(module, ipaddr))
        return mp_const_false;
    else
        return mp_obj_new_str_copy(&mp_type_str, (const byte*)ipaddr, strlen(ipaddr));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_ipaddr_obj, mo_ipaddr);


STATIC mp_obj_t mo_csq(void) {
    mo_object_t    *module = mo_get_default();
    os_uint8_t      rssi;
    os_uint8_t      ber;

    if(ERR_OK != mo_get_csq(module, &rssi, &ber))
        return mp_const_false;
    else
    {
        mp_obj_t tuple[2] = {MP_OBJ_NEW_SMALL_INT(rssi), MP_OBJ_NEW_SMALL_INT(ber)};
        return mp_obj_new_tuple(2, tuple);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_csq_obj, mo_csq);


STATIC mp_obj_t mo_imei(void) {
    mo_object_t    *module = mo_get_default();
    char            imei[30]={0};

    if(ERR_OK != mo_get_imei(module, imei, 30))
        return mp_const_false;
    else
        return mp_obj_new_str_copy(&mp_type_str, (const byte*)imei, strlen(imei));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_imei_obj, mo_imei);

STATIC mp_obj_t mo_imsi(void) {
    mo_object_t    *module = mo_get_default();
    char            imsi[30]={0};

    if(ERR_OK != mo_get_imsi(module, imsi, 30))
        return mp_const_false;
    else
        return mp_obj_new_str_copy(&mp_type_str, (const byte*)imsi, strlen(imsi));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_imsi_obj, mo_imsi);

STATIC mp_obj_t mo_ccid(void) {
    mo_object_t    *module = mo_get_default();
    char            ccid[30]={0};

    if(ERR_OK != mo_get_iccid(module, ccid, 30))
        return mp_const_false;
    else
        return mp_obj_new_str_copy(&mp_type_str, (const byte*)ccid, strlen(ccid));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_ccid_obj, mo_ccid);

STATIC mp_obj_t mo_simstat(void) 
{
    mp_raise_NotImplementedError("Do not support this function yet.");
    mp_log("Do not support this function yet.");
    return MP_OBJ_NEW_SMALL_INT(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mo_simstat_obj, mo_simstat);

STATIC mp_obj_t mo_apn(size_t n_args, const mp_obj_t *args) 
{
    mp_raise_NotImplementedError("Do not support this function yet.");
    mp_log("Do not support this function yet.");
    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mo_apn_obj, 1, 5, mo_apn);

STATIC const mp_rom_map_elem_t net_if_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_register),    MP_ROM_PTR(&mo_register_obj) },
    { MP_ROM_QSTR(MP_QSTR_active),      MP_ROM_PTR(&mo_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_cfun),        MP_ROM_PTR(&mo_cfun_obj) },
    { MP_ROM_QSTR(MP_QSTR_ipaddr),      MP_ROM_PTR(&mo_ipaddr_obj) },
    { MP_ROM_QSTR(MP_QSTR_csq),         MP_ROM_PTR(&mo_csq_obj) },
    { MP_ROM_QSTR(MP_QSTR_imei),        MP_ROM_PTR(&mo_imei_obj) },
    { MP_ROM_QSTR(MP_QSTR_imsi),        MP_ROM_PTR(&mo_imsi_obj) },
    { MP_ROM_QSTR(MP_QSTR_ccid),        MP_ROM_PTR(&mo_ccid_obj) },
    { MP_ROM_QSTR(MP_QSTR_apn),         MP_ROM_PTR(&mo_apn_obj) },
    { MP_ROM_QSTR(MP_QSTR_simstat),     MP_ROM_PTR(&mo_simstat_obj) },
};

STATIC MP_DEFINE_CONST_DICT(net_if_locals_dict, net_if_locals_dict_table);
const mp_obj_type_t netif_type = {
    { &mp_type_type },
    .name = MP_QSTR_netif,
    .locals_dict = (mp_obj_t)&net_if_locals_dict,
};

/*
STATIC mp_obj_t get_netif(size_t n_args, const mp_obj_t *args) 
{
#ifndef UIS8910DM_AUTO_CREATE
    module_uis8910dm_create("uis8910", 512);
#endif

    return MP_OBJ_FROM_PTR(&netif_obj);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(get_netif_obj, 0, 1, get_netif);
*/
STATIC const mp_rom_map_elem_t mp_module_mo_network_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_mo_net) },

    /*{ MP_ROM_QSTR(MP_QSTR_netif),       MP_ROM_PTR(&get_netif_obj) }, */
    { MP_ROM_QSTR(MP_QSTR_register),    MP_ROM_PTR(&mo_register_obj) },
    { MP_ROM_QSTR(MP_QSTR_active),      MP_ROM_PTR(&mo_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_cfun),        MP_ROM_PTR(&mo_cfun_obj) },
    { MP_ROM_QSTR(MP_QSTR_ipaddr),      MP_ROM_PTR(&mo_ipaddr_obj) },
    { MP_ROM_QSTR(MP_QSTR_csq),         MP_ROM_PTR(&mo_csq_obj) },
    { MP_ROM_QSTR(MP_QSTR_imei),        MP_ROM_PTR(&mo_imei_obj) },
    { MP_ROM_QSTR(MP_QSTR_imsi),        MP_ROM_PTR(&mo_imsi_obj) },
    { MP_ROM_QSTR(MP_QSTR_ccid),        MP_ROM_PTR(&mo_ccid_obj) },
    { MP_ROM_QSTR(MP_QSTR_apn),         MP_ROM_PTR(&mo_apn_obj) },
    { MP_ROM_QSTR(MP_QSTR_simstat),     MP_ROM_PTR(&mo_simstat_obj) },   
};

STATIC MP_DEFINE_CONST_DICT(mp_module_mo_network_globals, mp_module_mo_network_globals_table);

const mp_obj_module_t mp_module_mo_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_mo_network_globals,
};

#endif
