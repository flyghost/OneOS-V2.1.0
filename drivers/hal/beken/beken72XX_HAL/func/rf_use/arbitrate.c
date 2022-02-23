#include "include.h"
#include "mm.h"
#include "bk_timer_pub.h"
#include "arch.h"
#include "rtos_pub.h"
#include "error.h"
#include "sys_ctrl_pub.h"
#include "arbitrate.h"
#include "uart_pub.h"
#include "rw_pub.h"
#include "arm_arch.h"
#include "mem_pub.h"

beken_queue_t rf_msg_que = NULL;
beken_thread_t rf_arbitrate_handle = NULL;
static void rf_thread_main(void *arg);

#if (RF_USE_POLICY == WIFI_DEFAULT_BLE_REQUEST)

#elif (RF_USE_POLICY == BLE_DEFAULT_WIFI_REQUEST)
struct co_list wifi_pre_send;

int32_t rf_time_wifi = RF_WIFI_TIME_MIN;
int32_t rf_time_ble = RF_BLE_TIME_MAX;

uint8_t rf_user = RF_BLE_USE;

static void rf_schedule_start(void);

#elif (RF_USE_POLICY == BLE_WIFI_CO_REQUEST)
rf_info_t rf_info;

static void rf_use_end();
static uint32_t rf_get_time();

#endif

static OSStatus rf_timer_initialize_us(uint8_t timer_id, uint32_t time_us, void *callback)
{
    UINT32 ret;
    timer_param_t param;
    
    param.channel = timer_id;
    param.div = 1;              //timer0 timer1 timer2 26M // timer4 timer5 32K (n+1) division
    param.period = time_us;
    param.t_Int_Handler= callback;

    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM_US, &param);
    ASSERT(BK_TIMER_SUCCESS == ret);

    UINT32 timer_channel;
    timer_channel = param.channel;
    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_ENABLE, &timer_channel);
    ASSERT(BK_TIMER_SUCCESS == ret);

    return kNoErr;
}

static OSStatus rf_timer_stop(uint8_t timer_id)
{
    UINT32 ret;
    UINT32 timer_channel;
    
    timer_channel = timer_id;
    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_DISABLE, &timer_channel);
    ASSERT(BK_TIMER_SUCCESS == ret);
    return kNoErr;
}

static void rf_switch_to_wifi(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_CLR, NULL);

    rwnx_cal_ble_recover_rfconfig();

    rwnx_cal_recover_txpwr_for_wifi();
}

static void rf_switch_to_ble(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_SET, NULL);

    rwnx_cal_ble_set_rfconfig();

    rwnx_cal_set_txpwr_for_ble_boardcast();
}

void rf_thread_init(void)
{
    OSStatus ret;
    
    if (!rf_msg_que)
    {
        ret = rtos_init_queue(&rf_msg_que, 
                              "rf_arbitrate_que",
                              sizeof(rf_msg_t),
                              RF_MSG_QUEUE_COUNT);

        if (kNoErr != ret)
        {
            os_printf("Create rf queue failed\r\n");
            goto fail;
        }
    }

    if(!rf_arbitrate_handle)
    {
        ret = rtos_create_thread(&rf_arbitrate_handle, 
    			THD_RF_PRIORITY,
    			"rf_arbitrate", 
    			(beken_thread_function_t)rf_thread_main, 
    			RF_STACK_SIZE, 
    			(beken_thread_arg_t)0);

        if (kNoErr != ret)
        {
            os_printf("Create rf thread failed\r\n");
            goto fail;
        }        
    }

#if (RF_USE_POLICY == BLE_DEFAULT_WIFI_REQUEST)
    rf_schedule_start();
#endif

    bk_printf("rf_thread_init ok\r\n");

    return ;

fail:
    rf_thread_uninit();

    return;
}

void rf_thread_uninit(void)
{
    if(rf_arbitrate_handle)
    {
        rtos_delete_thread(&rf_arbitrate_handle);
        rf_arbitrate_handle = NULL;
    }

    if(rf_msg_que)
    {
        rtos_deinit_queue(&rf_msg_que);
        rf_msg_que = NULL;
    }
}

#if (RF_USE_POLICY == WIFI_DEFAULT_BLE_REQUEST)
static void rf_thread_main(void *arg)
{
    rtos_delete_thread( NULL );
}
#elif (RF_USE_POLICY == BLE_DEFAULT_WIFI_REQUEST)
wifi_sta_state_t wifi_sta_st = 
{
    .bcn_t1 = WIFI_WAKE_FORWARD_TIME * 625,
    .bcn_timer_run = 0,
    .g_time_us = 102400,
};

void wifi_ps_cal_bcn_int(uint16_t bcn_int)
{
    wifi_sta_st.bcn_int = bcn_int;
    wifi_sta_st.interval = 1;
}

void wifi_ps_cal_bcn_dtim_period(uint16_t dtim_period, uint8_t dtim_count)
{
    wifi_sta_st.dtim_period = dtim_period;
    wifi_sta_st.dtim_count = dtim_count;
}

void wifi_send_data_reset(void)
{
    struct pre_send_node *wifi_node;

    do
    {
        wifi_node = (struct pre_send_node *)co_list_pop_front(&wifi_pre_send);

        if(wifi_node)
        {
            os_free(wifi_node);
            wifi_node = NULL;
        }
        else
        {
            break;
        }
    }while(1);
}

void wifi_send_data(struct tx_hd *first_thd,
                          struct tx_hd *last_thd,
                          uint8_t access_category)
{
    rf_msg_t msg;
    struct pre_send_node *wifi_node;

    wifi_node = os_malloc(sizeof(struct pre_send_node));
    wifi_node->first_thd = first_thd;
    wifi_node->last_thd = last_thd;
    wifi_node->access_category = access_category;

    msg.msg_id = WIFI_SEND_DATA;
    msg.msg_param = (void *)wifi_node;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
}

void wifi_send_data_done(void)
{
    rf_msg_t msg;

    msg.msg_id = WIFI_SEND_OK;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
}

static void rf_wifi_ready(void)
{
    rf_msg_t msg;

    msg.msg_id = WIFI_READY_TO_SEND;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
    rf_timer_stop(RF_TIMER_ID);
}

static void rf_wifi_release(void)
{
    rf_msg_t msg;

    msg.msg_id = BLE_REQUEST_RF;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
    rf_timer_stop(RF_TIMER_ID);
}

static void rf_schedule_start(void)
{
    rf_msg_t msg;

    msg.msg_id = WIFI_REQUEST_RF;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
}

static void rf_ble_release(void)
{
    rf_msg_t msg;

    msg.msg_id = WIFI_REQUEST_RF;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
    rf_timer_stop(RF_TIMER_ID);
}

static void rf_wifi_use_end(void)
{
    rf_msg_t msg;

    msg.msg_id = WIFI_RELEASE_RF;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
    rf_timer_stop(RF_TIMER_ID);
}

static void rf_ble_use_end(void)
{
    rf_msg_t msg;

    msg.msg_id = BLE_RELEASE_RF;
    msg.msg_param = NULL;

    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
    rf_timer_stop(RF_TIMER_ID);
}

#if WIFI_STA_HD_BCN_TIMER
static OSStatus wifi_sta_bcn_timer_initialize_us(uint8_t timer_id, uint32_t time_us, void *callback)
{
    UINT32 ret;
    timer_param_t param;

    param.channel = timer_id;
    param.div = WSTA_BCN_TIMER_DIV;              //timer0 timer1 timer2 26M // timer4 timer5 32K (n+1) division
    param.period = time_us;
    param.t_Int_Handler= callback;

    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM_US, &param);
    ASSERT(BK_TIMER_SUCCESS == ret);

    UINT32 timer_channel;
    timer_channel = param.channel;
    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_ENABLE, &timer_channel);
    ASSERT(BK_TIMER_SUCCESS == ret);

    wifi_sta_st.bcn_timer_run = 1;

    return kNoErr;
}

static OSStatus wifi_sta_bcn_timer_stop(uint8_t timer_id)
{
    UINT32 ret;
    UINT32 timer_channel;

    wifi_sta_st.bcn_timer_run = 0;
    timer_channel = timer_id;
    ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_UNIT_DISABLE, &timer_channel);
    ASSERT(BK_TIMER_SUCCESS == ret);

    return kNoErr;
}


static void wifi_sta_bcn_timer_handler(void)
{
    GLOBAL_INT_DECLARATION();

    GLOBAL_INT_DISABLE();

    wifi_sta_st.bcn_timer_run = 0;
    rf_timer_stop(WSTA_BCN_TIMER_ID);

    GLOBAL_INT_RESTORE();
}

static uint32_t wifi_sta_bcn_timer_cur_tick(void)
{
    timer_param_t param;
    int ret;

    if(wifi_sta_st.bcn_timer_run)
    {
        param.channel = WSTA_BCN_TIMER_ID;
        ret = sddev_control(TIMER_DEV_NAME, CMD_TIMER_READ_CNT, &param);
        ASSERT(BK_TIMER_SUCCESS == ret);

        return param.period;
    }

    return 0;
}

static uint32_t wifi_sta_bcn_timer_cur_us(void)
{
    uint32_t tick_cnt = wifi_sta_bcn_timer_cur_tick();

    return (tick_cnt * WSTA_BCN_TIMER_DIV) / 26;
}

static uint32_t wifi_sta_bcn_timer_cur_residue_us(void)
{
    return wifi_sta_st.g_time_us - wifi_sta_bcn_timer_cur_us();
}

void calc_next_tbtt_timer(void)
{
    GLOBAL_INT_DECLARATION();

    GLOBAL_INT_DISABLE();
    wifi_sta_bcn_timer_stop(WSTA_BCN_TIMER_ID);

    wifi_sta_st.g_time_us = wifi_sta_st.interval * wifi_sta_st.bcn_int;
    wifi_sta_bcn_timer_initialize_us(WSTA_BCN_TIMER_ID, wifi_sta_st.g_time_us, wifi_sta_bcn_timer_handler);
    GLOBAL_INT_RESTORE();
}
#endif

uint32_t get_next_bcn_tbtt_time(void)
{
    int32_t time_us;
    GLOBAL_INT_DECLARATION();

    GLOBAL_INT_DISABLE();

    time_us = wifi_sta_bcn_timer_cur_residue_us();

    if(time_us < 0)
    {
        time_us = 0;
    }

    GLOBAL_INT_RESTORE();

    return time_us;
}

int wifi_beacon_info_sender(int status,int param)
{
    rf_msg_t msg;

    struct beacon_infor *bcn_info = os_malloc(sizeof(struct beacon_infor));

    msg.msg_id = WIFI_PS_BEACON;
    msg.msg_param = bcn_info;

    if(bcn_info)
    {
        if(status == BEACON_IND_MISS)
        {
            bcn_info->tatol_miss_bcn_cnt = (param >= 0xFFFFU) ? 0xFFFFU : param;
        }
        else
        {
            bcn_info->tatol_miss_bcn_cnt = 0;
        }
        bcn_info->status = status & 0xFFU;
    }
    rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);

    return 0;
}

int wifi_beacon_info_handler(rf_msg_t *msg)
{
    struct beacon_infor *bcn_info;

    if(msg != NULL)
    {
        bcn_info = msg->msg_param;
        if(bcn_info != NULL)
        {
            switch(bcn_info->status)
            {
                case BEACON_IND_MISS:
                {
                    if(bcn_info->tatol_miss_bcn_cnt > 1)
                    {
                        if(rf_time_wifi < RF_WIFI_TIME_MAX)
                        {
                            rf_time_wifi += 2;
                        }
                    }
                }
                break;
                case BEACON_IND_NO_MOREDATA:
                {
                    if(rf_time_wifi > RF_WIFI_TIME_MIN)
                    {
                        rf_time_wifi -= 1;
                    }
                }
                break;
                case BEACON_IND_MOREDATA:
                {
                    ps_fake_send_null(0);
                    if(rf_time_wifi < RF_WIFI_TIME_MAX)
                    {
                        rf_time_wifi += 2;
                    }
                }
                break;
                default:
                {

                }
                break;
            }
        }
        else
        {

        }
    }
    else
    {

    }

    return 0;
}

void wifi_station_status_event_notice(void *env,int status)
{
    switch(status)
    {
        case RW_EVT_STA_BEACON_LOSE:
        {

        }
        break;

        case RW_EVT_STA_CONNECTING:
        {

        }
        break;

        case RW_EVT_STA_IDLE:
        {

        }
        break;

        case RW_EVT_STA_GOT_IP:
        {

        }
        break;

        case RW_EVT_STA_CONNECT_FAILED:
        {

        }
        break;

        case RW_EVT_STA_CONNECTED:
        {

        }
        break;

        case RW_EVT_STA_DISCONNECTED:
        {

        }
        break;

        default:
        {

        }
        break;
    }
}

static void rf_thread_main(void *arg)
{
    OSStatus ret;
    rf_msg_t msg;
    struct pre_send_node *wifi_node;
    int32_t next_tbtt_us;

    co_list_init(&wifi_pre_send);

    while(1)
    {
        ret = rtos_pop_from_queue(&rf_msg_que, &msg, BEKEN_WAIT_FOREVER);

        if(kNoErr == ret)
        {
            switch(msg.msg_id)
            {
                case WIFI_REQUEST_RF:
                {
                    rf_timer_initialize_us(RF_TIMER_ID, wifi_sta_st.bcn_t1, rf_wifi_ready);
                    rf_switch_to_wifi();
                    //REG_WRITE((0x0802800 + (21 * 4)), 0x00);
                    rf_user = RF_WIFI_USE;
                }
                break;

                case BLE_REQUEST_RF:
                {
                    if(mhdr_get_station_status() == RW_EVT_STA_GOT_IP)
                    {
                        next_tbtt_us = get_next_bcn_tbtt_time();
                        rf_time_ble = next_tbtt_us / 625 - BLE_GUARD_TIME;
                        if(rf_time_ble < RF_BLE_TIME_MIN)
                        {
                            rf_time_ble = RF_BLE_TIME_MIN;
                        }
                    }
                    else if((mhdr_get_station_status() != RW_EVT_STA_IDLE) 
                        || get_appoint_type_vif(VIF_AP))
                    {
                        rf_time_ble = RF_BLE_TIME_MIN;
                    }
                    else
                    {
                        rf_time_ble = RF_BLE_TIME_MAX;
                    }
                    rf_switch_to_ble();
                    //REG_WRITE((0x0802800 + (21 * 4)), 0x02);
                    rf_user = RF_BLE_USE;
                    rf_timer_initialize_us(RF_TIMER_ID, (rf_time_ble * 625), rf_ble_use_end);
                }
                break;

                case WIFI_RELEASE_RF:
                {
                    if((mhdr_get_station_status() == RW_EVT_STA_GOT_IP) && !(wifi_pre_send.first))
                    {
                        ps_fake_send_null(1);
                    }

                    rf_user = RF_IN_GUARD;
                    rf_timer_initialize_us(RF_TIMER_ID, (WIFI_GUARD_TIME * 625), rf_wifi_release);
                }
                break;

                case BLE_RELEASE_RF:
                {
                    rf_user = RF_IN_GUARD;
                    //hal_assert_rec();
                    rf_timer_initialize_us(RF_TIMER_ID, (BLE_GUARD_TIME * 625), rf_ble_release);
                }
                break;

                case WIFI_SEND_DATA:
                {
                    wifi_node = os_malloc(sizeof(struct pre_send_node));
                    wifi_node->first_thd = ((struct pre_send_node *)(msg.msg_param))->first_thd;
                    wifi_node->last_thd = ((struct pre_send_node *)(msg.msg_param))->last_thd;
                    wifi_node->access_category = ((struct pre_send_node *)(msg.msg_param))->access_category;
                    co_list_push_back(&wifi_pre_send, &(wifi_node->list_hdr));
                }
                break;

                case WIFI_SEND_OK:
                {
                    if(rf_user == RF_WIFI_USE)
                    {
                        wifi_node = (struct pre_send_node *)co_list_pop_front(&wifi_pre_send);
                        if(wifi_node)
                        {
                            txl_frame_exchange_chain(wifi_node->first_thd, wifi_node->last_thd, wifi_node->access_category);
                            os_free(wifi_node);
                            wifi_node = NULL;
                        }
                    }
                }
                break;

                case WIFI_PS_BEACON:
                {
                    wifi_beacon_info_handler(&msg);
                }
                break;

                case WIFI_READY_TO_SEND:
                {
                    if(mhdr_get_station_status() == RW_EVT_STA_GOT_IP)
                    {
                        wifi_node = (struct pre_send_node *)co_list_pop_front(&wifi_pre_send);
                        if(wifi_node)
                        {
                            if(rf_time_wifi < RF_WIFI_TIME_MAX)
                            {
                                rf_time_wifi += 2;
                            }

                            txl_frame_exchange_chain(wifi_node->first_thd, wifi_node->last_thd, wifi_node->access_category);

                            os_free(wifi_node);
                            wifi_node = NULL;
                        }
                        else
                        {
                            if(rf_time_wifi > RF_WIFI_TIME_MIN)
                            {
                                rf_time_wifi -= 1;
                            }
                        }
                    }
                    else if((mhdr_get_station_status() != RW_EVT_STA_IDLE) 
                        || get_appoint_type_vif(VIF_AP))
                    {
                        wifi_node = (struct pre_send_node *)co_list_pop_front(&wifi_pre_send);
                        if(wifi_node)
                        {
                            txl_frame_exchange_chain(wifi_node->first_thd, wifi_node->last_thd, wifi_node->access_category);

                            os_free(wifi_node);
                            wifi_node = NULL;
                        }
                        rf_time_wifi = RF_WIFI_TIME_MAX;
                    }
                    else
                    {
                        wifi_node = (struct pre_send_node *)co_list_pop_front(&wifi_pre_send);
                        if(wifi_node)
                        {
                            txl_frame_exchange_chain(wifi_node->first_thd, wifi_node->last_thd, wifi_node->access_category);

                            os_free(wifi_node);
                            wifi_node = NULL;
                        }

                        rf_time_wifi = RF_WIFI_TIME_MIN;
                    }

                    rf_timer_initialize_us(RF_TIMER_ID, (rf_time_wifi * 625), rf_wifi_use_end);
                }
                break;

                default:
                {
                    os_printf("rf receive unknow msg\r\n");
                }
                break;
            }

            if (msg.msg_param)
            {
                os_free(msg.msg_param);
                msg.msg_param = NULL;
            }
        }
    }
}

uint8_t get_current_user(void)
{
    return rf_user;
}

#elif (RF_USE_POLICY == BLE_WIFI_CO_REQUEST)

OSStatus request_rf(rf_req_t *rf_request, rf_user_t *rf_user)
{
    OSStatus ret;
    rf_msg_t msg;
    rf_break_msg_t *msg_param;
    OSStatus status = kNoErr;

    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

    rf_info.rf_current_user.used_time = rf_get_time() - rf_info.rf_current_user.start_time;
    
    if (rf_info.status == RF_IDLE)
    {
        if(((rf_request->event_flag & RF_REQUESTER_MASK) >> RF_REQUESTER_POS) == RF_WIFI_REQUEST)
        {
            rf_switch_to_wifi();
            rf_info.status = RF_WIFI_USE;
        }
        else
        {
            rf_switch_to_ble();
            rf_info.status = RF_BLE_USE;
        }
        rf_info.rf_current_user.event_type = rf_request->event_type;
        rf_info.rf_current_user.event_priority = rf_request->event_priority;
        rf_info.rf_current_user.event_flag = rf_request->event_flag;
        rf_info.rf_current_user.use_time = rf_request->use_time;
        rf_info.rf_current_user.start_time = rf_get_time();
        rf_info.rf_current_user.used_time = 0;

        rf_timer_initialize_us(RF_TIMER_ID, rf_request->use_time, rf_use_end);
    }
    else if(((rf_info.rf_current_user.event_flag & RF_BREAK_EN_MASK) >> RF_BREAK_EN_POS)
            && (rf_info.rf_current_user.event_priority < rf_request->event_priority))
    {
        msg_param = os_malloc(sizeof(rf_break_msg_t));
        os_memcpy(msg_param->current_user, &(rf_info.rf_current_user), sizeof(rf_user_t));
        os_memcpy(msg_param->request_user, rf_request, sizeof(rf_req_t));
        msg.msg_param = msg_param;


        if(((rf_request->event_flag & RF_REQUESTER_MASK) >> RF_REQUESTER_POS) == RF_WIFI_REQUEST)
        {
            if(rf_info.status == RF_WIFI_USE)
            {
                msg.msg_id = WIFI_BREAK_WIFI;
            }
            else
            {
                msg.msg_id = WIFI_BREAK_BLE;
            }
        }
        else
        {
            if(rf_info.status == RF_WIFI_USE)
            {
                msg.msg_id = BLE_BREAK_WIFI;
            }
            else
            {
                msg.msg_id = BLE_BREAK_BLE;
            }
        }

        ret = rtos_push_to_queue(&rf_msg_que, &msg, BEKEN_NO_WAIT);
        ASSERT_ERR(kNoErr == ret);

        if(((rf_request->event_flag & RF_REQUESTER_MASK) >> RF_REQUESTER_POS) == RF_WIFI_REQUEST)
        {
            rf_switch_to_wifi();
            rf_info.status = RF_WIFI_USE;
        }
        else
        {
            rf_switch_to_ble();
            rf_info.status = RF_BLE_USE;
        }
        
        rf_info.rf_current_user.event_type = rf_request->event_type;
        rf_info.rf_current_user.event_priority = rf_request->event_priority;
        rf_info.rf_current_user.event_flag = rf_request->event_flag;
        rf_info.rf_current_user.use_time = rf_request->use_time;
        rf_info.rf_current_user.start_time = rf_get_time();
        rf_info.rf_current_user.used_time = 0;

        rf_timer_stop(RF_TIMER_ID);
        rf_timer_initialize_us(RF_TIMER_ID, rf_request->use_time, rf_use_end);
    }
    else
    {
        status = kGeneralErr;
    }

    rf_user = &(rf_info.rf_current_user);
    GLOBAL_INT_RESTORE();

    return status;
}

static void rf_use_end(void)
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

    if ((rf_get_time() - rf_info.rf_current_user.start_time) >= rf_info.rf_current_user.use_time)
    {
        rf_info.status = RF_IDLE;
    }

    rf_timer_stop(RF_TIMER_ID);

    GLOBAL_INT_RESTORE();
}

static uint32_t rf_get_time(void)
{
    ///TODO:how to get real time
    return 0;
}

static void rf_thread_main(void *arg)
{
    OSStatus ret;
    rf_msg_t msg;

    while(1)
    {
        ret = rtos_pop_from_queue(&rf_msg_que, &msg, BEKEN_WAIT_FOREVER);

        if(kNoErr == ret)
        {
            switch(msg.msg_id)
            {
                case WIFI_BREAK_WIFI:
                {
                    //wifi_break_wifi_handle((rf_break_msg_t *)msg->msg_param);
                }
                break;
                
                case WIFI_BREAK_BLE:
                {
                    //wifi_break_ble_handle((rf_break_msg_t *)msg->msg_param);
                }
                break;
                
                case BLE_BREAK_WIFI:
                {
                    //ble_break_wifi_handle((rf_break_msg_t *)msg->msg_param);
                }
                break;
                
                case BLE_BREAK_BLE:
                {
                    //ble_break_ble_handle((rf_break_msg_t *)msg->msg_param);
                }
                break;

                default:
                {
                    os_printf("rf receive unknow msg\r\n");
                }
                break;
            }

            if (msg.msg_param)
            {
                os_free(msg.msg_param);
                msg.msg_param = NULL;
            }
        }
    }
}

#endif

