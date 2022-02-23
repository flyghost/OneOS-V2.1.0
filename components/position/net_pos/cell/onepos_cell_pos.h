#ifndef __ONEPOS_CELL_LOCA_H__
#define __ONEPOS_CELL_LOCA_H__

#include <cJSON.h>
#include <mo_api.h>
#include "onepos_common.h"

/**
 ***********************************************************************************************************************
 * @enum        onepos_cell_net_type_t
 *
 * @brief       onepos Supported Cell Network Type
 ***********************************************************************************************************************
 */
typedef enum
{
    ONEPOS_CELL_TYPE_GSM = 1,
    ONEPOS_CELL_TYPE_CDMA,
    ONEPOS_CELL_TYPE_WCDMA,
    ONEPOS_CELL_TYPE_TD_CDMA,
    ONEPOS_CELL_TYPE_LTE,
    /* Add others cell network type */
} onepos_cell_net_type_t;

#define ONEPOS_CELL_MSG_FORMAT  "%3u,%2u,%10u,%15u,%4d"
#define ONEPOS_CELL_INFO_LEN    38

extern os_bool_t onepos_get_cell_sta(void);
extern os_err_t  onepos_init_cell_dev(void);
extern os_err_t  cell_pos_pub_msg(cJSON* json_src);

#endif /* __ONEPOS_CELL_LOCA_H__ */
