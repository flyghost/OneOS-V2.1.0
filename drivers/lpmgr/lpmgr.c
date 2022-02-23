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
 * @file        lpmgr.c
 *
 * @brief       this file implements Low power management related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <arch_interrupt.h>
#include <os_timer.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <lpmgr.h>
#include <dlog.h>

#define DBG_TAG "lpmgr"

struct lpmgr_update_callback
{
    void (*update_cb)(os_uint64_t nsec, void *priv);
    void  *priv;
    os_list_node_t list;
};

#define LP_SLEEP_STATUS_NONE    (0)
#define LP_SLEEP_STATUS_ACTIVE  (1)
#define LP_SLEEP_STATUS_UPDATED (2)

static struct os_lpmgr_dev *gs_lpmgr = NULL;

static int gs_lpce_irq = -1;

static int gs_lp_sleep_state = LP_SLEEP_STATUS_NONE;

static os_uint64_t gs_lp_sleep_nsec = 0;

static os_list_node_t gs_register_dev_list = OS_LIST_INIT(gs_register_dev_list);

static os_list_node_t gs_lpmgr_update_callback_list = OS_LIST_INIT(gs_lpmgr_update_callback_list);

static os_err_t __os_lpmgr_device_register(void *priv, const struct os_lpmgr_device_ops *ops, os_bool_t head)
{
    os_ubase_t level;

    struct os_lpmgr_device *device_lpm;

    os_list_for_each_entry(device_lpm, &gs_register_dev_list, struct os_lpmgr_device, list)
    {
        if ((device_lpm->priv == priv) && (device_lpm->ops == ops))
        {
            LOG_D(DBG_TAG, "dev[%p], ops[%p] alread register!\n", priv, ops);
            return OS_EOK;
        }
    }

    device_lpm = os_calloc(1, sizeof(struct os_lpmgr_device));

    OS_ASSERT(device_lpm != OS_NULL);

    device_lpm->priv = priv;
    device_lpm->ops  = ops;

    level = os_irq_lock();
    
    if (head)
    {
        os_list_add(&gs_register_dev_list, &device_lpm->list);
    }
    else
    {
        os_list_add_tail(&gs_register_dev_list, &device_lpm->list);
    }
    
    os_irq_unlock(level);

    LOG_D(DBG_TAG, "register dev[%p], ops[%p]!\n", priv, ops);

    return OS_EOK;
}


os_err_t os_lpmgr_device_register_high_prio(void *priv, const struct os_lpmgr_device_ops *ops)
{
    return __os_lpmgr_device_register(priv, ops, OS_TRUE);
}

os_err_t os_lpmgr_device_register(void *priv, const struct os_lpmgr_device_ops *ops)
{
    return __os_lpmgr_device_register(priv, ops, OS_FALSE);
}

void os_lpmgr_device_unregister(void *priv, const struct os_lpmgr_device_ops *ops)
{
    struct os_lpmgr_device *device_lpm;

    os_list_for_each_entry(device_lpm, &gs_register_dev_list, struct os_lpmgr_device, list)
    {
        if ((device_lpm->priv == priv) && (device_lpm->ops == ops))
        {
            os_list_del(&device_lpm->list);
            os_free(device_lpm);
            return;
        }
    }
}

static int lpmgr_device_suspend(lpmgr_sleep_mode_e mode)
{
    int ret = OS_EOK;
    
    struct os_lpmgr_device *device_lpm;

    os_list_for_each_entry(device_lpm, &gs_register_dev_list, struct os_lpmgr_device, list)
    {
        ret = device_lpm->ops->suspend(device_lpm->priv, mode);
        if (ret != OS_EOK)
            break;
    }

    return ret;
}

static void lpmgr_device_resume(lpmgr_sleep_mode_e mode)
{
    struct os_lpmgr_device *device_lpm;

    os_list_for_each_entry(device_lpm, &gs_register_dev_list, struct os_lpmgr_device, list)
    {
        device_lpm->ops->resume(device_lpm->priv, mode);
    }
}

void os_lpmgr_notify_set(void (*notify)(os_lpmgr_sys_e event, lpmgr_sleep_mode_e mode, void *data), void *data)
{
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    lpmgr->gs_lpmgr_notify.notify = notify;
    lpmgr->gs_lpmgr_notify.data   = data;
    LOG_D(DBG_TAG, "register lpmgr_notify notify[%p]!\n", notify);
}

static int lpmgr_enter_sleep_call(lpmgr_sleep_mode_e mode)
{
    int ret = OS_EOK;
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;
    
    /* Notify app will enter sleep mode */
    if (lpmgr->gs_lpmgr_notify.notify)
        lpmgr->gs_lpmgr_notify.notify(SYS_ENTER_SLEEP, mode, lpmgr->gs_lpmgr_notify.data);

    /* Suspend all peripheral device */
    ret = lpmgr_device_suspend(mode);
    if (ret != OS_EOK)
    {
        lpmgr_device_resume(mode);
        if (lpmgr->gs_lpmgr_notify.notify)
            lpmgr->gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, lpmgr->gs_lpmgr_notify.data);

        return OS_ERROR;
    }

    return OS_EOK;
}

static int lpmgr_exit_sleep_call(lpmgr_sleep_mode_e mode)
{
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    /* resume all device */
    lpmgr_device_resume(mode);

    if (lpmgr->gs_lpmgr_notify.notify)
        lpmgr->gs_lpmgr_notify.notify(SYS_EXIT_SLEEP, mode, lpmgr->gs_lpmgr_notify.data);

    return OS_EOK;
}

void os_lpmgr_update_callback_register(void (*update_cb)(os_uint64_t nsec, void *priv), void *priv)
{
    os_base_t level;
    struct lpmgr_update_callback *cb;

    cb = os_calloc(1, sizeof(struct lpmgr_update_callback));

    OS_ASSERT(cb != OS_NULL);

    cb->update_cb = update_cb;
    cb->priv = priv;

    level = os_irq_lock();
    os_list_add_tail(&gs_lpmgr_update_callback_list, &cb->list);
    os_irq_unlock(level);
}

static void os_lpmgr_update(os_uint64_t nsec)
{
    struct lpmgr_update_callback *cb;

    if (nsec == 0)
    {
        return;
    }

    os_list_for_each_entry(cb, &gs_lpmgr_update_callback_list, struct lpmgr_update_callback, list)
    {
        cb->update_cb(nsec, cb->priv);
    }
}

static void os_lpmgr_irq_entry(int irq)
{
    os_base_t level;

    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    level = os_irq_lock();
    
    if (gs_lp_sleep_state == LP_SLEEP_STATUS_ACTIVE)
    {
        if (gs_lp_sleep_nsec != 0)
        {
            if (irq != gs_lpce_irq)
            {
                gs_lp_sleep_nsec = os_clockevent_read(lpmgr->lpce);
                os_clockevent_stop(lpmgr->lpce);
            }
            
            os_lpmgr_update(gs_lp_sleep_nsec);
        }

        gs_lp_sleep_state = LP_SLEEP_STATUS_UPDATED;
    }
    
    if (gs_lp_sleep_state != LP_SLEEP_STATUS_NONE)
    {
        /* period trig 1ms, avoid sleep forever */
        os_clockevent_start_oneshot(lpmgr->lpce, max(1000000ULL, lpmgr->lpce->min_nsec));
    }

    os_irq_unlock(level);
}

OS_IRQ_HOOK(os_lpmgr_irq_entry);

static os_err_t lpmgr_start(struct os_lpmgr_dev *lpmgr)
{
    os_err_t     ret = OS_EOK;
    os_base_t    level;
    os_uint64_t  nsec = 0;
    
    level = os_irq_lock();
    
    ret = lpmgr_enter_sleep_call(SYS_SLEEP_MODE_LIGHT);
    if (ret != OS_EOK)
    {
        lpmgr_exit_sleep_call(SYS_SLEEP_MODE_LIGHT);
        os_irq_unlock(level);
        return OS_EBUSY;
    }
    
    os_tick_t timeout_tick = os_tickless_get_sleep_ticks();

    if (timeout_tick != OS_TICK_MAX)
    {
        nsec = ((os_uint64_t)(timeout_tick) * lpmgr->mult >> lpmgr->shift);

        if (nsec < lpmgr->lpce->min_nsec || nsec < BSP_USING_MINSLEEP_MS * 1000000)
        {
            lpmgr_exit_sleep_call(SYS_SLEEP_MODE_LIGHT);
            os_irq_unlock(level);
            return OS_EBUSY;
        }
    
        if (nsec > lpmgr->lpce->max_nsec)
            nsec = lpmgr->lpce->max_nsec;
        
        gs_lp_sleep_nsec  = nsec;
        os_clockevent_start_oneshot(lpmgr->lpce, nsec);
    }
    else
    {
        gs_lp_sleep_nsec  = 0;
        os_clockevent_stop(lpmgr->lpce);
    }    

    ret = lpmgr_enter_sleep_call(lpmgr->sleep_mode);
    if (ret != OS_EOK)
    {
        lpmgr_exit_sleep_call(lpmgr->sleep_mode);
        os_irq_unlock(level);
        return OS_EBUSY;
    }

    gs_lp_sleep_state = LP_SLEEP_STATUS_ACTIVE;

    os_irq_unlock(level);

    lpmgr->sleep(lpmgr->sleep_mode);

    lpmgr_exit_sleep_call(lpmgr->sleep_mode);

    OS_ASSERT(gs_lp_sleep_state != LP_SLEEP_STATUS_ACTIVE);
    gs_lp_sleep_state = LP_SLEEP_STATUS_NONE;

    return OS_EOK;
}

void  os_low_power_manager(void)
{
    OS_ASSERT(gs_lpmgr != NULL);
    
    os_schedule_lock();
    
    if (gs_lpmgr->sleep_mode > SYS_SLEEP_MODE_IDLE)
        lpmgr_start(gs_lpmgr);

    os_schedule_unlock();
}

static void lpmgr_check_sleep_mode(struct os_lpmgr_dev *lpmgr)
{
    lpmgr_sleep_mode_e index;

    for (index = SYS_SLEEP_MODE_NONE; index < SYS_SLEEP_MODE_MAX; index++)
    {
        if (lpmgr->modes[index])
        {
            lpmgr->sleep_mode = index;
            break;
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver requests the system stall in corresponding power mode
 *
 * @param[in]       sleep_mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_request(lpmgr_sleep_mode_e sleep_mode)
{
    os_base_t     level;
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    if (sleep_mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_irq_lock();
    if (lpmgr->modes[sleep_mode] < 255)
        lpmgr->modes[sleep_mode]++;

    lpmgr_check_sleep_mode(lpmgr);

    os_irq_unlock(level);
}

/**
 ***********************************************************************************************************************
 * @brief           Upper application or device driver releases the system stall in corresponding power mode
 *
 * @param[in]       sleep_mode                the parameter of run mode or sleep mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_lpmgr_release(lpmgr_sleep_mode_e sleep_mode)
{
    os_ubase_t    level;
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    if (sleep_mode > (SYS_SLEEP_MODE_MAX - 1))
        return;

    level = os_irq_lock();
    if (lpmgr->modes[sleep_mode] > 0)
        lpmgr->modes[sleep_mode]--;

    lpmgr_check_sleep_mode(lpmgr);

    os_irq_unlock(level);
}

static void os_kernel_update_ns(os_uint64_t nsec, void *priv)
{
    os_tick_t tick = ((os_uint64_t)(nsec) * gs_lpmgr->mult_t >> gs_lpmgr->shift_t);

    os_tickless_update(tick);
}

static void lpce_event_handler(os_clockevent_t * ce)
{
    gs_lpce_irq = os_irq_num();
}

void os_lpmgr_init(os_err_t (*sleep)(lpmgr_sleep_mode_e mode))
{
    struct os_lpmgr_dev *lpmgr;
    os_uint32_t max_sec;

    lpmgr = (struct os_lpmgr_dev *)os_calloc(1, sizeof(struct os_lpmgr_dev));
    OS_ASSERT(lpmgr != NULL);

    lpmgr->lpce = (os_clockevent_t *)os_device_find(LPMGR_TIMER_DEVICE_NAME);
    OS_ASSERT(lpmgr->lpce != NULL);
    OS_ASSERT(lpmgr->lpce->prescaler_mask == 1);

    os_device_open(&lpmgr->lpce->parent);

    /* find lpce irq number */
    os_clockevent_register_isr(lpmgr->lpce, lpce_event_handler);

    os_clockevent_start_oneshot(lpmgr->lpce, 10);

    volatile int i = 1000000;

    while (gs_lpce_irq == -1 && i--);

    os_clockevent_stop(lpmgr->lpce);

    OS_ASSERT(gs_lpce_irq != -1);

    os_kprintf("lpce:%s, irq:%d\r\n", LPMGR_TIMER_DEVICE_NAME, gs_lpce_irq);

    os_clockevent_register_isr(lpmgr->lpce, OS_NULL);
    
    lpmgr->sleep = sleep;
    
    lpmgr->sleep_mode = SYS_SLEEP_MODE_IDLE;

    max_sec  = lpmgr->lpce->mask / lpmgr->lpce->freq;
    calc_mult_shift(&lpmgr->mult, &lpmgr->shift, OS_TICK_PER_SECOND, NSEC_PER_SEC, max_sec);
    calc_mult_shift(&lpmgr->mult_t, &lpmgr->shift_t, NSEC_PER_SEC,OS_TICK_PER_SECOND, max_sec);

    lpmgr->parent.type = OS_DEVICE_TYPE_PM;
    lpmgr->parent.ops  = OS_NULL;
    
    os_device_register(&lpmgr->parent, OS_LPMGR_DEVICE_NAME);

    gs_lpmgr = lpmgr;

    os_device_open(&lpmgr->parent);

    os_lpmgr_update_callback_register(os_kernel_update_ns, OS_NULL);
}

#ifdef OS_USING_SHELL
#include <shell.h>

static const char *gs_lpmgr_sleep_str[] = SYS_SLEEP_MODE_NAMES;

static void lpmgr_release_mode(int argc, char **argv)
{
    lpmgr_sleep_mode_e mode = SYS_SLEEP_MODE_NONE;
    if (argc >= 2)
    {
        mode = (lpmgr_sleep_mode_e)atoi(argv[1]);
    }

    os_lpmgr_release(mode);
}
SH_CMD_EXPORT(power_release, lpmgr_release_mode, "release power management mode");

static void lpmgr_request_mode(int argc, char **argv)
{
    lpmgr_sleep_mode_e mode = SYS_SLEEP_MODE_NONE;
    if (argc >= 2)
    {
        mode = (lpmgr_sleep_mode_e)atoi(argv[1]);
    }

    os_lpmgr_request(mode);
}
SH_CMD_EXPORT(power_request, lpmgr_request_mode, "request power management mode");


static void lpmgr_dump_status(void)
{
    os_uint32_t index;
    struct os_lpmgr_dev *lpmgr = gs_lpmgr;

    /* dump power status */
    os_kprintf("| Power Management Mode | Counter | Timer |\r\n");    
    os_kprintf("+-----------------------+---------+-------+\r\n");

    os_kprintf("lpmgr current sleep mode: %s\r\n", gs_lpmgr_sleep_str[lpmgr->sleep_mode]);

    /* dump lpmgr devices */
    index = 0;
    struct os_lpmgr_device *device_lpm;

    os_kprintf("| no    |    priv    | ops        |\r\n");
    os_kprintf("+-------+------------+------------+\r\n");

    os_list_for_each_entry(device_lpm, &gs_register_dev_list, struct os_lpmgr_device, list)
    {
        os_kprintf("| %-5d | 0x%p | 0x%p |\r\n", index++, device_lpm->priv, device_lpm->ops);
    }

    os_kprintf("total register num: %d\r\n", index);
}
SH_CMD_EXPORT(power_status, lpmgr_dump_status, "dump power management status");

#endif
