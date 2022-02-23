#include "usr_misc.h"
#include "os_util.h"
#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/objlist.h"
#include "onepos_interface.h"
#include "rcvr_object.h"
#include "py/obj.h"

typedef struct position_gnss_rcvr_obj
{
    mp_obj_base_t base;
    rcvr_object_t *rcvr;
} position_gnss_rcvr_obj_t;

typedef struct position_gnss_rcvr_data_obj
{
    mp_obj_base_t    base;
    rcvr_object_t   *rcvr;
    void            *buff;
    os_size_t        size;
    rcvr_prot_type_t type;
} position_gnss_rcvr_data_obj_t;

STATIC mp_obj_t position_onepos_stop_server()
{
    onepos_stop_server();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_stop_server_obj, position_onepos_stop_server);

STATIC mp_obj_t position_onepos_start_server()
{
    onepos_start_server();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_start_server_obj, position_onepos_start_server);

STATIC mp_obj_t position_onepos_set_pos_interval(mp_obj_t self_in)
{
    os_bool_t ret;
    ret = onepos_set_pos_interval(mp_obj_get_int(self_in));
    mp_obj_t self = mp_obj_new_bool(ret);
    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(position_onepos_set_pos_interval_obj, position_onepos_set_pos_interval);

STATIC mp_obj_t position_onepos_set_server_type(mp_obj_t self_in)
{
    os_bool_t ret;
    ret = onepos_set_server_type(mp_obj_get_int(self_in));
    mp_obj_t self = mp_obj_new_bool(ret);
    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(position_onepos_set_server_type_obj, position_onepos_set_server_type);

STATIC mp_obj_t position_onepos_get_latest_position()
{
    char date[16] = {0,};
    char time[16] = {0,};

    struct tm TM = {0};
    onepos_pos_t pos = {0,};

    if(onepos_get_latest_position(&pos))
    {
        mp_obj_module_t *netpos_dict =  mp_obj_new_dict(2);
        gmtime_r((time_t *)&pos.time, &TM);

        memset(date, 0, sizeof(date));
        memset(time, 0, sizeof(time));

        os_snprintf(date, sizeof(date), "%04d/%02d/%02d", TM.tm_year + 1900, TM.tm_mon + 1, TM.tm_mday);
        os_snprintf(time, sizeof(time), "%02d:%02d:%02d:%03d", TM.tm_hour + 8, TM.tm_min, TM.tm_sec, 0);

        mp_obj_dict_store(netpos_dict, MP_OBJ_NEW_QSTR(MP_QSTR_date), mp_obj_new_str(date, strlen(date)));
        mp_obj_dict_store(netpos_dict, MP_OBJ_NEW_QSTR(MP_QSTR_time), mp_obj_new_str(time, strlen(time)));
        mp_obj_dict_store(netpos_dict, MP_OBJ_NEW_QSTR(MP_QSTR_lat), mp_obj_new_float(pos.lat_coordinate));
        mp_obj_dict_store(netpos_dict, MP_OBJ_NEW_QSTR(MP_QSTR_lon), mp_obj_new_float(pos.lon_coordinate));
        
        return MP_OBJ_FROM_PTR(netpos_dict);
    }
    else
        return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_get_latest_position_obj, position_onepos_get_latest_position);

STATIC mp_obj_t position_onepos_get_pos_interval()
{
    os_uint32_t ret;
    ret = onepos_get_pos_interval();
    mp_obj_t self = mp_obj_new_int_from_uint(ret);
    return self;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_get_pos_interval_obj, position_onepos_get_pos_interval);

STATIC mp_obj_t position_onepos_get_server_sta()
{
    os_int32_t ret = -1;

    ret = (onepos_serv_sta_t)onepos_get_server_sta();
    
    mp_obj_t self = mp_obj_new_int(ret);
    return self;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_get_server_sta_obj, position_onepos_get_server_sta);

STATIC mp_obj_t position_onepos_get_server_type()
{
    os_int32_t ret = -1;

    ret = (os_int32_t)onepos_get_server_type();

    mp_obj_t self = mp_obj_new_int(ret);
    return self;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_onepos_get_server_type_obj, position_onepos_get_server_type);

rcvr_object_t * g_rcvr = (rcvr_object_t*)OS_NULL;
#ifdef GNSS_USING_TECHTOTOP
extern rcvr_object_t *techtotop_rcvr_creat(void);
#endif
STATIC mp_obj_t position_gnss_rcvr_start()
{   
    os_bool_t ret = OS_TRUE;

    #ifdef GNSS_USING_TECHTOTOP
    g_rcvr = techtotop_rcvr_creat();
    #endif

    if(OS_NULL == g_rcvr)
        ret = OS_FALSE;
    else
        ret = OS_TRUE;

    mp_obj_t self = mp_obj_new_bool(ret);

    return self;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_gnss_rcvr_start_obj, position_gnss_rcvr_start);

STATIC mp_obj_t position_gnss_rcvr_stop()
{
    os_bool_t ret = OS_TRUE;

    if(OS_NULL == g_rcvr)
        ret = OS_FALSE;
    else
    {
        rcvr_object_deinit(g_rcvr);
        rcvr_object_destroy(g_rcvr);
        g_rcvr = (rcvr_object_t*)OS_NULL;
        ret = OS_TRUE;
    }
    mp_obj_t self = mp_obj_new_bool(ret);

    return self;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_gnss_rcvr_stop_obj, position_gnss_rcvr_stop);

STATIC mp_obj_t position_gnss_get_rcvr_data()
{
    nmea_t data = {0,};
    double lat, lon, altitude,speed = 0.0f;
    os_uint32_t sat_num = 0;

    if(OS_NULL == g_rcvr)
        return mp_const_none;
    else
    {
        memset(&data, 0, sizeof(data));
        if(OS_EOK != get_rcvr_data(g_rcvr, (void*)&data, sizeof(data), RCVR_NMEA_0183_PROT))
            return mp_const_none;
        else
            {
                mp_obj_module_t *nmea_dict =  mp_obj_new_dict(2);

                char date[16] = {0,};
                char time[16] = {0,};

                memset(date, 0, sizeof(date));
                memset(time, 0, sizeof(time));

                os_snprintf(date, sizeof(date), "20%02d/%02d/%02d", data.rmc_frame.date.year, data.rmc_frame.date.month, data.rmc_frame.date.day);
                os_snprintf(time, sizeof(time), "%02d:%02d:%02d:%03d", data.rmc_frame.time.hours, data.rmc_frame.time.minutes, data.rmc_frame.time.seconds, data.rmc_frame.time.microseconds);
                
                lat      = tras_loca_float(data.rmc_frame.latitude.value, (data.rmc_frame.latitude.dec_len + 2));
                lon      = tras_loca_float(data.rmc_frame.longitude.value, (data.rmc_frame.longitude.dec_len + 2));
                speed    = tras_loca_float(data.rmc_frame.speed.value, data.rmc_frame.speed.dec_len);
                altitude = tras_loca_float(data.gga_frame.altitude.value, data.gga_frame.altitude.dec_len);
                sat_num  = data.gga_frame.satellites_used;

                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_date), mp_obj_new_str(date, strlen(date)));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_time), mp_obj_new_str(time, strlen(time)));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_lat), mp_obj_new_float(lat));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_lon), mp_obj_new_float(lon));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_speed), mp_obj_new_float(speed));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_alt), mp_obj_new_float(altitude));
                mp_obj_dict_store(nmea_dict, MP_OBJ_NEW_QSTR(MP_QSTR_sat), mp_obj_new_int(sat_num));
                return MP_OBJ_FROM_PTR(nmea_dict);
            }
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_gnss_get_rcvr_data_obj, position_gnss_get_rcvr_data);

#ifdef RCVR_SUPP_SET_MODE
STATIC mp_obj_t position_gnss_rcvr_set_mode(mp_obj_t self_in)
{
    os_err_t  result = OS_EOK;;
    os_bool_t ret    = OS_FALSE;

    if(OS_NULL == g_rcvr)
        return mp_const_none;

    result = rcvr_set_mode(g_rcvr, mp_obj_get_int(self_in));
    if(OS_EOK == result)
    {
        ret = OS_TRUE;
    }
    else
    {
        ret = OS_FALSE;
    }

    mp_obj_t self = mp_obj_new_bool(ret);

    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(position_gnss_rcvr_set_mode_obj, position_gnss_rcvr_set_mode);
#endif

#ifdef RCVR_SUPP_RESET
STATIC mp_obj_t position_gnss_rcvr_reset(mp_obj_t self_in)
{
    os_err_t  result = OS_EOK;;
    os_bool_t ret    = OS_FALSE;

    if(OS_NULL == g_rcvr)
    {
        ret = OS_FALSE;
    }
    else
    {
        result = rcvr_reset(g_rcvr, mp_obj_get_int(self_in));
        if(OS_EOK == result)
        {
            ret = OS_TRUE;
        }
        else
        {
            ret = OS_FALSE;
        }
    }
    mp_obj_t self = mp_obj_new_bool(ret);

    return self;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(position_gnss_rcvr_reset_obj, position_gnss_rcvr_reset);
#endif

STATIC mp_obj_t position_interface_explain()
{
    os_kprintf("onepos_stop_server : stop onepos position server\r\n");
    os_kprintf("onepos_start_server : start onepos position server with \"server type\"\r\n");
    os_kprintf("onepos_set_pos_interval : set position server period interval <second(>=3)>\r\n");
    os_kprintf("onepos_set_server_type : set position server type <0/1(0:circ;1:single)>\r\n");
    os_kprintf("onepos_get_latest_position : get position location\r\n");
    os_kprintf("onepos_get_pos_interval : get position server period interval\r\n");
    os_kprintf("onepos_get_server_sta : get position server status <0~3(0:closing;1:circ_runing;2:sig_runing;3:will_close)>\r\n");
    os_kprintf("onepos_get_server_type : set position server type <0/1(0:circ;1:single)>\r\n");
    os_kprintf("gnss_rcvr_start : start gnss receiver\r\n");
    os_kprintf("gnss_rcvr_stop : stop gnss receiver\r\n");
    os_kprintf("gnss_get_rcvr_data : get data of gnss receiver\r\n");
    os_kprintf("gnss_rcvr_set_mode : set runing mode of gnss receiver<1~6(1:BDS;2:GPS;3:BDS+GPS;4:GLONASS;5:BDS+GLONASS;6:GPS+GLONASS)>\r\n");
    os_kprintf("gnss_rcvr_reset : reset gnss receiver<1~3(1:cold start;2:warm start;3:hot start)>\r\n");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(position_interface_explain_obj, position_interface_explain);

STATIC const mp_rom_map_elem_t mp_module_position_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_position)},
    {MP_ROM_QSTR(MP_QSTR_onepos_stop_server), MP_ROM_PTR(&position_onepos_stop_server_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_start_server), MP_ROM_PTR(&position_onepos_start_server_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_set_pos_interval), MP_ROM_PTR(&position_onepos_set_pos_interval_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_set_server_type), MP_ROM_PTR(&position_onepos_set_server_type_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_get_latest_position), MP_ROM_PTR(&position_onepos_get_latest_position_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_get_pos_interval), MP_ROM_PTR(&position_onepos_get_pos_interval_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_get_server_sta), MP_ROM_PTR(&position_onepos_get_server_sta_obj)},
    {MP_ROM_QSTR(MP_QSTR_onepos_get_server_type), MP_ROM_PTR(&position_onepos_get_server_type_obj)},
    {MP_ROM_QSTR(MP_QSTR_gnss_rcvr_start), MP_ROM_PTR(&position_gnss_rcvr_start_obj)},
    {MP_ROM_QSTR(MP_QSTR_gnss_rcvr_stop), MP_ROM_PTR(&position_gnss_rcvr_stop_obj)},
    {MP_ROM_QSTR(MP_QSTR_gnss_get_rcvr_data), MP_ROM_PTR(&position_gnss_get_rcvr_data_obj)},
#ifdef RCVR_SUPP_SET_MODE
    {MP_ROM_QSTR(MP_QSTR_gnss_rcvr_set_mode), MP_ROM_PTR(&position_gnss_rcvr_set_mode_obj)},
#endif
#ifdef RCVR_SUPP_RESET
    {MP_ROM_QSTR(MP_QSTR_gnss_rcvr_reset), MP_ROM_PTR(&position_gnss_rcvr_reset_obj)},
#endif
    {MP_ROM_QSTR(MP_QSTR_position_interface_explain), MP_ROM_PTR(&position_interface_explain_obj)},
};

STATIC MP_DEFINE_CONST_DICT(mp_module_position_globals, mp_module_position_globals_table);

const mp_obj_module_t mp_module_position = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_position_globals,
};
