/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        mo_api.h
 *
 * @brief       module link kit api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_API_H__
#define __MO_API_H__

#include "mo_common.h"
#include "mo_object.h"
#include "mo_factory.h"

#ifdef MOLINK_USING_GENERAL_OPS
#include "mo_general.h"
#endif

#ifdef MOLINK_USING_NETSERV_OPS
#include "mo_netserv.h"
#endif

#ifdef MOLINK_USING_PING_OPS
#include "mo_ping.h"
#endif

#ifdef MOLINK_USING_IFCONFIG_OPS
#include "mo_ifconfig.h"
#endif

#ifdef MOLINK_USING_NETCONN_OPS
#include "mo_netconn.h"
#endif

#ifdef MOLINK_USING_SOCKETS_OPS
#include "mo_socket.h"
#endif

#ifdef MOLINK_USING_ONENET_NB_OPS
#include "mo_onenet_nb.h"
#endif

#ifdef MOLINK_USING_WIFI_OPS
#include "mo_wifi.h"
#endif

#ifdef MOLINK_USING_PPP_OPS
#include "mo_ppp.h"
#endif

#ifdef MOLINK_USING_MQTTC_OPS
#include "mo_mqttc.h"
#endif

#ifdef MOLINK_USING_CTM2M_OPS
#include "mo_ctm2m.h"
#endif

/**
 ***********************************************************************************************************************
 * @note These macros call the methods of the default molink module instance,
 *       making sure that the default instance on the system has been created.
 ***********************************************************************************************************************
 */
#ifdef MOLINK_USING_GENERAL_OPS
#define at_test()                                  mo_at_test(mo_get_default())
#define get_imei(value, len)                       mo_get_imei(mo_get_default(), value, len)
#define get_imsi(value, len)                       mo_get_imsi(mo_get_default(), value, len)
#define get_iccid(value, len)                      mo_get_iccid(mo_get_default(), value, len)
#define get_cfun(fun_lvl)                          mo_get_cfun(mo_get_default(), fun_lvl)
#define set_cfun(fun_lvl)                          mo_set_cfun(mo_get_default(), fun_lvl)
#define get_firmware_version(value, len)           mo_get_firmware_version(mo_get_default(), value, len)
#define sleep_mode_set(fun_lvl)                    mo_sleep_mode_set(mo_get_default(), fun_lvl)
#define get_eid(eid, len)                          mo_get_eid(mo_get_default(), eid, len)
#define get_local_time(l_tm)                       mo_get_local_time(mo_get_default(), l_tm)
#endif /* MOLINK_USING_GENERAL_OPS */

#ifdef MOLINK_USING_NETSERV_OPS
#define set_attach(attach_stat)                    mo_set_attach(mo_get_default(), attach_stat)
#define get_attach(attach_stat)                    mo_get_attach(mo_get_default(), attach_stat)
#define set_reg(reg_n)                             mo_set_reg(mo_get_default(), reg_n)
#define get_reg(info)                              mo_get_reg(mo_get_default(), info)
#define get_5g_reg(info)                           mo_get_5g_reg(mo_get_default(), info)
#define set_cgact(cid, act_stat)                   mo_set_cgact(mo_get_default(), cid, act_stat)
#define get_cgact(cid, act_stat)                   mo_get_cgact(mo_get_default(), cid, act_stat)
#define get_csq(rssi, ber)                         mo_get_csq(mo_get_default(), rssi, ber)
#define get_radio(radio_info)                      mo_get_radio(mo_get_default(), radio_info)
#define get_cell_info(onepos_cell_info)            mo_get_cell_info(mo_get_default(), onepos_cell_info)
#define get_psm(info)                              mo_get_psm(mo_get_default(), info)
#define set_psm(info)                              mo_set_psm(mo_get_default(), info)
#define set_edrx_cfg(cfg)                          mo_set_edrx_cfg(mo_get_default(), cfg)
#define get_edrx_cfg(edrx_local)                   mo_get_edrx_cfg(mo_get_default(), edrx_local)
#define get_edrx_dynamic(edrx_dynamic)             mo_get_edrx_dynamic(mo_get_default(), edrx_dynamic)
#define set_band(band_list, num)                   mo_set_band(mo_get_default(), band_list, num)
#define set_earfcn(earfcn)                         mo_set_earfcn(mo_get_default(), earfcn)
#define get_earfcn(earfcn)                         mo_get_earfcn(mo_get_default(), earfcn)
#define clear_stored_earfcn()                      mo_clear_stored_earfcn(mo_get_default())
#define clear_plmn()                               mo_clear_plmn(mo_get_default())
#endif /* MOLINK_USING_NETSERV_OPS */

#ifdef MOLINK_USING_PING_OPS
#define ping(host, len, timeout, resp)             mo_ping(mo_get_default(), host, len, timeout, resp)
#endif /* MOLINK_USING_NETSERV_OPS */

#ifdef MOLINK_USING_IFCONFIG_OPS
#define ifconfig()                                 mo_ifconfig(mo_get_default())
#define get_ipaddr(ip)                             mo_get_ipaddr(mo_get_default(), ip)
#define set_dnsserver(dns)                         mo_set_dnsserver(mo_get_default(), dns)
#define get_dnsserver(dns)                         mo_get_dnsserver(mo_get_default(), dns)
#endif /* MOLINK_USING_IFCONFIG_OPS */

#ifdef MOLINK_USING_NETCONN_OPS
#define netconn_create(type)                       mo_netconn_create(mo_get_default(), type)
#define netconn_destroy(netconn)                   mo_netconn_destroy(mo_get_default(), netconn)
#define netconn_bind(netconn, addr, port)          mo_netconn_bind(mo_get_default(), netconn, addr, port)
#define netconn_connect(netconn, addr, port)       mo_netconn_connect(mo_get_default(), netconn, addr, port)
#define netconn_send(netconn, data, size)          mo_netconn_send(mo_get_default(), netconn, data, size)
#define netconn_recv(netconn, data, size, timeout) mo_netconn_recvfrom(mo_get_default(), netconn, data, size, OS_NULL, OS_NULL, timeout)
#define netconn_gethostbyname(domain_name, addr)   mo_netconn_gethostbyname(mo_get_default(), domain_name, addr)
#endif /* MOLINK_USING_NETCONN_OPS */

#ifdef MOLINK_USING_WIFI_OPS
#define wifi_set_mode(mode)                        mo_wifi_set_mode(mo_get_default(), mode)
#define wifi_get_mode()                            mo_wifi_get_mode(mo_get_default())
#define wifi_get_stat()                            mo_wifi_get_stat(mo_get_default())
#define wifi_scan_info(ssid, scan_result)          mo_wifi_scan_info(mo_get_default(), ssid, scan_result)
#define wifi_scan_info_free(scan_result)           mo_wifi_scan_info_free(scan_result)
#define wifi_connect_ap(ssid, password)            mo_wifi_connect_ap(mo_get_default(), ssid, password)
#define wifi_disconnect_ap()                       mo_wifi_disconnect_ap(mo_get_default())
#endif /* MOLINK_USING_WIFI_OPS */

#ifdef MOLINK_USING_PPP_OPS
#define ppp_init()                                 mo_ppp_init(mo_get_default())
#define ppp_dial()                                 mo_ppp_dial(mo_get_default())
#define ppp_exit()                                 mo_ppp_exit(mo_get_default())
#endif /* MOLINK_USING_PPP_OPS */

#ifdef MOLINK_USING_ONENET_NB_OPS
#define onenetnb_get_config(timeout, resp, format, ...)                                                                \
    mo_onenetnb_get_config(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_set_config(timeout, resp, format, ...)                                                                \
    mo_onenetnb_set_config(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_discoverrsp(timeout, resp, format, ...)                                                               \
    mo_onenetnb_discoverrsp(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_executersp(timeout, resp, format, ...)                                                                \
    mo_onenetnb_executersp(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_parameterrsp(timeout, resp, format, ...)                                                              \
    mo_onenetnb_parameterrsp(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_get_write(timeout, resp, format, ...)                                                                 \
    mo_onenetnb_get_write(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_create(timeout, resp, format, ...)   mo_onenetnb_create(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_createex(timeout, resp, format, ...) mo_onenetnb_createex(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_delete(timeout, resp, format, ...)   mo_onenetnb_delete(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_addobj(timeout, resp, format, ...)   mo_onenetnb_addobj(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_delobj(timeout, resp, format, ...)   mo_onenetnb_delobj(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_nmi(timeout, resp, format, ...)      mo_onenetnb_nmi(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_open(timeout, resp, format, ...)     mo_onenetnb_open(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_close(timeout, resp, format, ...)    mo_onenetnb_close(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_readrsp(timeout, resp, format, ...)  mo_onenetnb_readrsp(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_writersp(timeout, resp, format, ...) mo_onenetnb_writersp(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_notify(timeout, resp, format, ...)   mo_onenetnb_notify(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_update(timeout, resp, format, ...)   mo_onenetnb_update(mo_get_default(), timeout, resp, format, __VA_ARGS__)
#define onenetnb_cb_register(user_callbacks)          mo_onenetnb_cb_register(mo_get_default(), user_callbacks)
#endif /* MOLINK_USING_ONENET_NB_OPS */

#endif /* __MO_API_H__ */
