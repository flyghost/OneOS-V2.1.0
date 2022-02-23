#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shell.h>
#include "onepos_common.h"
struct onepos_cmd_des
{
    const char *cmd;
    os_int32_t (*fun)(os_int32_t argc, os_int8_t *argv[]);
};

#if defined(ONEPOS_USING_NET_POS)
#include "onepos_net_pos_if.h"

static os_int32_t onepos_status_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    os_int8_t sta_str[ONEPOS_MAX_STA][30] = {"closing", "runing", "sig_sev_runing", "will_close"};

    if (2 == argc)    //  read
    {
        os_kprintf("status : %s\r\n", sta_str[onepos_get_server_sta()]);
    }
    else
    {
        os_kprintf("this cmd do not support input param\r\n");
    }

    return 0;
}

static os_int32_t onepos_interval_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (2 == argc)    //  read
    {
        os_kprintf("interval : %d sec\r\n", onepos_get_pos_interval());
    }
    else if (3 == argc && _onepos_isdigit(argv[2]))
    {
        onepos_set_pos_interval(atoi((char*)(char*)argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, should be (onepos interval <secodes>)\r\n");
    }

    return 0;
}

static os_int32_t onepos_start_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (ONEPOS_CLOSING == onepos_get_server_sta())
    {
        onepos_start_server();
    }
    else if (ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\r\n");
    }
    else
    {
        os_kprintf("onepos is already runing!\r\n");
    }

    return 0;
}

static os_int32_t onepos_stop_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    if (ONEPOS_RUNING == onepos_get_server_sta())
    {
        onepos_stop_server();
    }
    else if (ONEPOS_SIG_RUNING == onepos_get_server_sta())
    {
        os_kprintf("onepos single server is runing, it will automatic stop!\r\n");
    }
    else if (ONEPOS_WILL_CLOSE == onepos_get_server_sta())
    {
        os_kprintf("onepos is will close, pls wait ...\r\n");
    }
    else
    {
        os_kprintf("onepos is already closed!\r\n");
    }

    return 0;
}

static os_int32_t onepos_get_latest_position_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    onepos_pos_t pos_info = {0};

    memset(&pos_info, 0, sizeof(onepos_pos_t));

    onepos_get_latest_position(&pos_info);
    onepos_info_print(&pos_info);

    return 0;
}

static os_int32_t onepos_server_type_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    os_int8_t server_type_str[ONEPOS_MAX_TYPE][15] = {"circulation", "single"};

    if (2 == argc)    //  read
    {
        os_kprintf("server_type : %s\r\n", server_type_str[onepos_get_server_type()]);
    }
    else if (3 == argc && IS_VAILD_SEV_TYPE(atoi((char*)argv[2])))    // set
    {
        onepos_set_server_type((onepos_serv_type_t)atoi((char*)argv[2]));
    }
    else
    {
        os_kprintf("this cmd do not support input format, shoud be (onepos sev_type <0/1>)\r\n");
    }

    return 0;
}
#endif

static os_int32_t onepos_test_cmd_func(os_int32_t argc, os_int8_t *argv[])
{
    os_kprintf("%s entry!\n", __func__);
    return 0;
}
#if defined(GNSS_USING_RCVR)
#include "rcvr_object.h"
#if defined(GNSS_NMEA_0183_PROT)
#include "nmea_0183.h"
#endif
/* ops_test test_case rcvr_name(or NULL) */
static os_int32_t onepos_gnss_rcvr_ops_test(os_int32_t argc, os_int8_t *argv[])
{
	os_err_t       result         = OS_EOK; 
    os_uint32_t    test_case      = 0;
	rcvr_object_t *rcvr           = OS_NULL;
    os_int8_t      temp_str[256]  = {0,};

    memset(temp_str, 0, sizeof(temp_str));

    if(argc >= 3)
    {
        test_case = atoi((char*)argv[2]);                
    }
	else
	{
		os_kprintf("input param is eroor.\r\n");
		return OS_EPERM;
	}

	if(3 == argc)
		rcvr = rcvr_object_get_default();
	else
		rcvr = rcvr_object_get_by_name((const char*)argv[3]);	

	if(rcvr)
	{
        os_kprintf("Using rcvr : %s\t\r\n", rcvr->name);

        switch (test_case)
        {
            #ifdef GNSS_NMEA_0183_PROT
            case 0:
            {
                nmea_t         nmea_data = {0,};
                memset(&nmea_data, 0, sizeof(nmea_t));
                if(OS_EOK == get_rcvr_data(rcvr, (void*)&nmea_data, sizeof(nmea_t), RCVR_NMEA_0183_PROT))
                {
                    os_kprintf("one_position_test get data:\r\n");
                    os_kprintf("\t date : 20%02d - %d - %d\r\n",
                            nmea_data.rmc_frame.date.year,
                            nmea_data.rmc_frame.date.month,
                            nmea_data.rmc_frame.date.day);
                    os_kprintf("\t time : %d : %d : %d : %d\r\n",
                            nmea_data.rmc_frame.time.hours,
                            nmea_data.rmc_frame.time.minutes,
                            nmea_data.rmc_frame.time.seconds,
                            nmea_data.rmc_frame.time.microseconds);
                    printf("\t latitude : %lld, %d; %f\r\n",
                            nmea_data.rmc_frame.latitude.value,
                            nmea_data.rmc_frame.latitude.dec_len,
                            rcvr_data_to_wgs84(nmea_data.rmc_frame.latitude));
                    printf("\t longitude : %lld, %d; %f\r\n",
                            nmea_data.rmc_frame.longitude.value,
                            nmea_data.rmc_frame.longitude.dec_len,
                            rcvr_data_to_wgs84(nmea_data.rmc_frame.longitude));
                    printf("\t speed : %lld, %d; %f\r\n", nmea_data.rmc_frame.speed.value, nmea_data.rmc_frame.speed.dec_len, tras_loca_float(nmea_data.rmc_frame.speed));
                    
                    result = OS_EOK;
                }
                else
                {
                    os_kprintf("get_gnss_data Error\r\n");
                    result = OS_ERROR;
                }
            }
                break;
            #endif

            #if defined(RCVR_SUPP_RESET)
            case 1:
                {
                    result =  rcvr_reset(rcvr, RCVR_COLD_START);
                    os_kprintf("reset rcvr result : %d\r\n", result);
                }
                break;
            #endif		

            #if defined(RCVR_SUPP_RESET) && defined(RCVR_SUPP_AGNSS)
            case 2:
                {
                    result = rcvr_reset(rcvr, RCVR_COLD_START);
                    if(OS_EOK == result)
                    {
                        os_kprintf("reset rcvr is succ, will agnss\r\n");
                        result = rcvr_agnss(rcvr);
                        os_kprintf("agnss rcvr result : %d\r\n", result);
                    }
                    else
                        os_kprintf("reset rcvr is failed, result is %d\n", result);
                }
                break;
            #endif
            
        default:
            os_kprintf("no support ops code");
            result = OS_EPERM;
            break;
        }		
	}

	return result;
}

static os_int32_t onepos_gnss_rcvr_close_test(os_int32_t argc, os_int8_t *argv[])
{
    rcvr_object_t *rcvr      = OS_NULL;
	
	if(2 == argc)
		rcvr = rcvr_object_get_default();
	else
		rcvr = rcvr_object_get_by_name((const char*)argv[2]);

    if(rcvr)
	{
        os_kprintf("Closing rcvr : %s\t\r\n", rcvr->name);

        rcvr_object_deinit(rcvr);
        rcvr_object_destroy(rcvr);
        rcvr = (rcvr_object_t *)OS_NULL;
    }

    return OS_EOK;
}
#ifdef GNSS_USING_ICOFCHINA
extern rcvr_object_t *icofchina_rcvr_creat(void);
#endif

#ifdef GNSS_USING_UNICORECOMM
extern rcvr_object_t *unicorecomm_rcvr_creat(void);
#endif

#ifdef GNSS_USING_TECHTOTOP
extern rcvr_object_t *techtotop_rcvr_creat(void);
#endif

#ifdef GNSS_USING_UBLOX
extern rcvr_object_t *ublox_rcvr_creat(void);
#endif

#ifdef GNSS_USING_MX
extern rcvr_object_t *mx_rcvr_creat(void);
#endif

static os_int32_t onepos_gnss_rcvr_test(os_int32_t argc, os_int8_t *argv[])
{
    #ifdef GNSS_USING_ICOFCHINA
	icofchina_rcvr_creat();
	#endif

	#ifdef GNSS_USING_UNICORECOMM
	unicorecomm_rcvr_creat();
	#endif

    #ifdef GNSS_USING_TECHTOTOP
    techtotop_rcvr_creat();
    #endif

	#ifdef GNSS_USING_UBLOX
	ublox_rcvr_creat();
	#endif

	#ifdef GNSS_USING_MX
	mx_rcvr_creat();
	#endif
	
    os_kprintf("Default gnss receiver : %s\t\r\n", rcvr_object_get_default()->name);
	 
	return OS_EOK;
}
#endif

/* cmd table */
static const struct onepos_cmd_des onepos_cmd_tab[] = {
#if defined(ONEPOS_USING_NET_POS)
                                                       {"status", onepos_status_cmd_func},
                                                       {"interval", onepos_interval_cmd_func},
                                                       {"sev_type", onepos_server_type_cmd_func},
                                                       {"start", onepos_start_cmd_func},
                                                       {"stop", onepos_stop_cmd_func},
                                                       {"pos", onepos_get_latest_position_cmd_func},
#endif                                                       
#if defined(GNSS_USING_RCVR)
                                                       {"gnss_test", onepos_gnss_rcvr_test},
													   {"ops_test", onepos_gnss_rcvr_ops_test},
                                                       {"gnss_close", onepos_gnss_rcvr_close_test},
#endif
													   {"test", onepos_test_cmd_func}};


static os_int32_t onepos_help(os_int32_t argc, os_int8_t *argv[])
{
#if defined(ONEPOS_USING_NET_POS)
    os_kprintf("onepos\r\n");
    os_kprintf("onepos help\r\n");
    os_kprintf("onepos status <0/1(0:closing;1:runing)>\r\n");
    os_kprintf("onepos interval <second(>=3)>\r\n");
    os_kprintf("onepos sev_type <0/1(0:circ;1:single)>\r\n");
    os_kprintf("onepos start\r\n");
    os_kprintf("onepos stop\r\n");
    os_kprintf("onepos pos\r\n");
#endif
#if defined(ONEPOS_USING_GNSS_POS)
    os_kprintf("onepos gnss_test\r\n");
	os_kprintf("onepos ops_test<0:get rcvr data;1:reset rcvr;2:agnss test><rcvr name>\r\n");
    os_kprintf("onepos gnss_close\r\n");
#endif
    os_kprintf("onepos test\r\n");
    return 0;
}

static os_int32_t onepos_cmd(os_int32_t argc, os_int8_t *argv[])
{
    os_int32_t i;
    os_int32_t result = 0;

    const struct onepos_cmd_des *run_cmd = OS_NULL;

    if (argc == 1)
    {
        onepos_help(argc, argv);
        return 0;
    }

    /* find fun */
    for (i = 0; i < sizeof(onepos_cmd_tab) / sizeof(onepos_cmd_tab[0]); i++)
    {
        if (strcmp((const char*)onepos_cmd_tab[i].cmd, (const char*)argv[1]) == 0)
        {
            run_cmd = &onepos_cmd_tab[i];
            break;
        }
    }

    /* not find fun, print help */
    if (run_cmd == OS_NULL)
    {
        onepos_help(argc, argv);
        return 0;
    }

    /* run fun */
    if (run_cmd->fun != OS_NULL)
    {
        result = run_cmd->fun(argc, argv);
    }

    return result;
}

SH_CMD_EXPORT(onepos, onepos_cmd, "onepos command.");
