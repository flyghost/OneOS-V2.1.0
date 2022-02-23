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
 * @file        lpmgr.h
 *
 * @brief       this file implements lpmgr related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __LPMGR_H__
#define __LPMGR_H__

#include <device.h>
#include <os_task.h>
#include <os_clock.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

typedef enum
{
    SYS_SLEEP_MODE_NONE = 0,
    SYS_SLEEP_MODE_IDLE,
    SYS_SLEEP_MODE_LIGHT,
    SYS_SLEEP_MODE_DEEP,
    SYS_SLEEP_MODE_MAX,
}lpmgr_sleep_mode_e;

#define SYS_SLEEP_MODE_NAMES    \
{                               \
    "None Mode",                \
    "Idle Mode",                \
    "LightSleep Mode",          \
    "DeepSleep Mode",           \
    "Standby Mode",             \
    "Shutdown Mode",            \
}


#define OS_LPMGR_DEVICE_NAME    "lpm"

struct os_lpmgr_register_dev
{
    /* the list of device, which has low power manager feature */
    os_uint8_t              device_number;
    struct os_lpmgr_device *lp_device;
} ;

typedef enum
{
    SYS_ENTER_SLEEP = 0,
    SYS_EXIT_SLEEP,
}os_lpmgr_sys_e;

struct lpmgr_notify
{
    void (*notify)(os_lpmgr_sys_e event, lpmgr_sleep_mode_e mode, void *data);
    void *data;
};

struct os_lpmgr_dev
{
    struct os_device parent;

    /* modes */
    lpmgr_sleep_mode_e modes[SYS_SLEEP_MODE_MAX];
    lpmgr_sleep_mode_e sleep_mode; /* current sleep mode */
    os_uint8_t mode_change_falg;

    struct lpmgr_notify gs_lpmgr_notify;
    /* if the mode has timer, the corresponding bit is 1*/
    os_tick_t  sleep_tick;

    os_err_t (*sleep)(lpmgr_sleep_mode_e mode);

    os_clockevent_t *lpce;

    os_uint32_t mult;       /* time(ns) = tick * mult >> shift */
    os_uint32_t shift;

    os_uint32_t mult_t;     /* tick = time(ns) * mult_t >> shift_t */
    os_uint32_t shift_t;
};

struct os_lpmgr_device_ops
{
    os_err_t (*suspend)(void *priv, lpmgr_sleep_mode_e mode);
    void (*resume)(void *priv, lpmgr_sleep_mode_e mode);
};

struct os_lpmgr_device
{
    const struct os_lpmgr_device_ops *ops;
    void *priv;
    os_list_node_t list;
};

void os_lpmgr_update_callback_register(void (*update_cb)(os_uint64_t nsec, void *priv), void *priv);

void os_lpmgr_request(lpmgr_sleep_mode_e sleep_mode);
void os_lpmgr_release(lpmgr_sleep_mode_e sleep_mode);

os_err_t os_lpmgr_device_register_high_prio(void *priv, const struct os_lpmgr_device_ops *ops);
os_err_t os_lpmgr_device_register(void *priv, const struct os_lpmgr_device_ops *ops);
void os_lpmgr_device_unregister(void *priv, const struct os_lpmgr_device_ops *ops);

void os_lpmgr_notify_set(void (*notify)(os_lpmgr_sys_e event, lpmgr_sleep_mode_e mode, void *data), void *data);

void os_lpmgr_init(os_err_t (*sleep)(lpmgr_sleep_mode_e mode));

os_err_t lpmgr_notickless_start(lpmgr_sleep_mode_e sleep_mode, os_uint32_t timeout_ms);
OS_WEAK os_err_t other_check(struct os_lpmgr_dev *lpm);

#endif /* __LPMGR_H__ */
