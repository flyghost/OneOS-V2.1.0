#ifndef __ONEPOS_NET_POS_IF_H__
#define __ONEPOS_NET_POS_IF_H__

#include <os_mq.h>
#include <os_sem.h>
#include <os_task.h>
#include <os_timer.h>
#include <os_mutex.h>
#include <os_types.h>
#include <cJSON.h>
#include "onepos_common.h"
#include "onepos_protocol.h"

typedef enum
{
    ONEPOS_CLOSING    = 0,
    ONEPOS_RUNING     = 1,
    ONEPOS_SIG_RUNING = 2,
    ONEPOS_WILL_CLOSE = 3,
    ONEPOS_MAX_STA    = 4
} onepos_serv_sta_t;

typedef enum
{
    ONEPOS_INVAILD_TYPE = -1,
    ONEPOS_CIRC_RUN     = 0,
    ONEPOS_SIG_RUN      = 1,
    ONEPOS_MAX_TYPE     = 2
} onepos_serv_type_t;

#define ONEPOS_NET_POS_TASK_STA_SIZE 2048
#define ONEPOS_NET_POS_TASK_PRIO     25
#define ONEPOS_NET_POS_TASK_TICK     20 

#define IS_VAILD_SEV_TYPE(type) (type > ONEPOS_INVAILD_TYPE && type < ONEPOS_MAX_TYPE)
#define IS_VALID_POS_MODE(mode) (mode > ONEPOS_INVAILD_POS_MODE && mode < ONEPOS_MAX_POS_MODE)

/**
 ***********************************************************************************************************************
 * @def         NET_POS_MSG_PUB_TOPIC_SUFF
 *
 * @brief       onepos position message publish topic suffix(send)
 ***********************************************************************************************************************
 */
#define NET_POS_MSG_PUB_TOPIC_SUFF "/lbs/post"

/**
 ***********************************************************************************************************************
 * @def         NET_POS_MSG_SUB_TOPIC_SUFF
 *
 * @brief       onepos position result subscribe topic suffix(receive)
 ***********************************************************************************************************************
 */
#define NET_POS_MSG_SUB_TOPIC_SUFF "/lbs/post_reply"

/**
 ***********************************************************************************************************************
 * @def         ONEPOS_CONF_PUB_TOPIC_SUFF
 *
 * @brief       onepos config message publish topic suffix(send)
 ***********************************************************************************************************************
 */
#define NET_POS_CONF_PUB_TOPIC_SUFF "/config/set_reply"

/**
 ***********************************************************************************************************************
 * @def         NET_POS_CONF_SUB_TOPIC_SUFF
 *
 * @brief       onepos config message subscibe topic suffix(receive)
 ***********************************************************************************************************************
 */
#define NET_POS_CONF_SUB_TOPIC_SUFF "/config/set"

typedef struct
{
    onepos_pos_t  position;
    os_bool_t     err_flag;
} net_pos_msg_t;

typedef enum
{
    POS_MSG_PUB_TOPIC =   0,
    CONF_PUB_TOPIC    =   1,

    /* add other topic */
    NET_POS_PUB_TOPIC_MAX
}net_pos_pub_topic_index_t;

typedef enum
{
    POS_MSG_SUB_TOPIC =   0,
    #ifdef NET_POS_SUPP_REMOTE_CONF
    CONF_SUB_TOPIC    =   1,
    #endif
    /* add other topic */
    NET_POS_SUB_TOPIC_MAX
}net_pos_sub_topic_index_t;

typedef struct onepos_net_pos{
    onepos_pos_t       *pos_result;
    os_timer_t         *timer;
    os_sem_t           *notice;
    os_mutex_t         *lock;
    os_task_t          *task;
    onepos_prot_pub_tpc_t  *pub_topic;
    onepos_prot_t      *prot;
    os_uint32_t         pos_err;
    os_uint16_t         interval;
    onepos_serv_sta_t   status;
    onepos_serv_type_t  type;
}onepos_net_pos_t;

#define DEF_NET_POS_INIT(pos_result) {pos_result, OS_NULL, OS_NULL, OS_NULL, OS_NULL, OS_NULL,                   \
						OS_NULL, 0, NET_POS_DEF_INTERVAL, ONEPOS_CLOSING, (onepos_serv_type_t)NET_POS_DEF_SEV_TYPE}

#if defined(ONEPOS_WIFI_POS)
#include "onepos_wifi_pos.h"
#endif

#if defined(ONEPOS_CELL_POS)
#include "onepos_cell_pos.h"
#endif

extern void               onepos_stop_server(void);
extern void               onepos_start_server(void);
extern void               clean_net_pos_msg(char *msg_str);
extern void               onepos_info_print(onepos_pos_t *src_info);
extern void               onepos_rep_net_pos_sta(onepos_net_pos_t *pos);
extern os_bool_t          onepos_net_pos_get_dev_sta(void);
extern os_bool_t          net_pos_func(onepos_net_pos_t *pos);
extern os_bool_t          onepos_set_pos_err(os_uint32_t pos_err);
extern os_bool_t          onepos_set_pos_interval(os_int32_t interval);
extern os_bool_t          onepos_set_server_type(onepos_serv_type_t type);
extern os_bool_t          onepos_get_latest_position(onepos_pos_t *src_info);
extern os_uint32_t        onepos_get_sev_pos_err(void);
extern os_uint32_t        onepos_get_pos_interval(void);
extern onepos_serv_sta_t  onepos_get_server_sta(void);
extern onepos_serv_type_t onepos_get_server_type(void);

#endif /* __ONEPOS_NET_POS_IF_H__ */
