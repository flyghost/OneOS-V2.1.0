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
 * @file        hwtimer.c
 *
 * @brief       this file implements hwtimer related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <drv_cfg.h>
#include <string.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

static os_clockevent_t *gs_best_ce = OS_NULL;

static os_list_node_t gs_clockevent_list = OS_LIST_INIT(gs_clockevent_list);

/**
 * @brief 选取最佳的时钟事件
 * 
 * @return os_clockevent_t* 
 */
os_clockevent_t *os_clockevent_best(void)
{
    return gs_best_ce;
}

/**
 * @brief 插入事件链表
 * 
 * 该链表会根据event的rating进行排序
 * 
 * @param ce 
 */
static void os_clockevent_enqueue(os_clockevent_t *ce)
{
    os_list_node_t *entry = &gs_clockevent_list;
    os_clockevent_t *tmp;

    os_list_for_each_entry(tmp, &gs_clockevent_list, os_clockevent_t, list)
    {
        if (tmp->rating < ce->rating)
            break;
        entry = &tmp->list;
    }
    os_list_add(entry, &ce->list);
}

static os_bool_t event_flag;

static void os_clockevent_valid_handler(os_clockevent_t *ce)
{
    event_flag = OS_TRUE;
}

static os_bool_t os_clockevent_valid(os_clockevent_t *ce)
{
    volatile int i = 1000000;

    event_flag = OS_FALSE;

    ce->event_handler = os_clockevent_valid_handler;

    ce->ops->start(ce, 1 & ce->prescaler_mask, 1);

    while (!event_flag && i--);

    ce->ops->stop(ce);

    ce->event_handler = OS_NULL;

    return event_flag;
}

/**
 * @brief 查找rating最高且有效的clockevent
 * 
 */
void os_clockevent_select_best(void)
{
    os_clockevent_t *ce;

    if (gs_best_ce != OS_NULL)
    {
        os_device_close(&gs_best_ce->parent);
    }

    os_list_for_each_entry(ce, &gs_clockevent_list, os_clockevent_t, list)
    {
        if (os_clockevent_valid(ce))
        {
            gs_best_ce = ce;
            break;
        }
        else
        {
            os_kprintf("invalid clockevent %s.\r\n", ce->name);
        }
    }

    OS_ASSERT(gs_best_ce != NULL);
        
    if (os_device_open(&gs_best_ce->parent) != OS_EOK)
    {
        os_kprintf("open clockevent %s failed.\r\n", gs_best_ce->name);
        while (1);
    }
}

// 注册clockevent回调
void os_clockevent_register_isr(os_clockevent_t *ce, void (*event_handler)(os_clockevent_t *ce))
{
    if (ce == gs_best_ce && ce->event_handler != OS_NULL)
    {
        os_kprintf("best ce handler replace from %p to %p\r\n", ce->event_handler, event_handler);
    }

    ce->event_handler = event_handler;
}

/**
 * @brief 读取clockevent产生中断的目标纳秒值
 * 
 * 当时间到达这个纳秒的时，会产生中断
 * 
 * @param ce 
 * @return os_uint64_t 
 */
os_uint64_t os_clockevent_read(os_clockevent_t *ce)
{
    os_uint64_t count = ce->ops->read(ce);      // 读取目标产生clockevent的硬件counter值

    return ce->prescaler * count * ce->mult_t >> ce->shift_t;   // count转位ns
}

/**
 * @brief 判断是否是周期性的clockevent
 * 
 * @param ce 
 * @return os_bool_t 
 */
static os_bool_t os_clockevent_auto_period(os_clockevent_t *ce)
{
    return ((ce->feature == OS_CLOCKEVENT_FEATURE_PERIOD)       // 周期性clockevent
            && ce->period_count != 0                            // 周期不为0
            && (ce->period_count & ~ce->count_mask) == 0);      // 周期不能超过count mask
}

/**
 * @brief 
 * 
 * @param ce 
 * @return int 
 */
static int os_clockevent_calc_param(os_clockevent_t *ce)
{
    os_uint64_t nsec;
    os_uint64_t evt;
    os_uint64_t count;
    os_uint32_t prescaler;
    os_int32_t  trig_isr = 0;

    nsec = os_clocksource_gettime();        // 获取时钟源当前时间，单位：纳秒

    /* reserve 5000 nsec */
    if (ce->next_nsec <= (nsec + 5000))     // 如果下一次时钟事件和当前事件的间隔小于5000纳秒
    {
        if (ce->period_nsec == 0)           // 非周期性时钟事件
        {
            ce->count = 0;                  // 计数值设为0
            return 1;
        }
        else
        {
            ce->next_nsec = period_calc_next_nsec(ce->next_nsec, nsec, ce->period_nsec);    // 计算目标时间
            trig_isr++;
        }
    }

    /**
     * @brief 计算下一个count值
     * 
     */
    if (os_clockevent_auto_period(ce))      // 是否是周期性时钟事件
    {
        prescaler = 1 & ce->prescaler_mask; // 是否预分频
        count     = ce->period_count;       // 周期count值
    }
    else
    {
        nsec = ce->next_nsec - nsec;        // 周期时间（纳秒）
        nsec = max(nsec, ce->min_nsec);     // 不能小于最小值
        nsec = min(nsec, ce->max_nsec);     // 不能大于最大值
        
        evt  = nsec * ce->mult >> ce->shift;    // 纳秒--->硬件counter值
        evt  = min(evt, ce->mask);              // 不能超过硬件counter计数最大值
    
        if ((evt & ~ce->count_mask) == 0)
        {
            prescaler = 1 & ce->prescaler_mask;
            count     = evt;
        }
        else
        {
            if (((evt & ce->count_mask) > (ce->count_mask / 2)) || ((evt >> ce->count_bits) > 1))
            {
                prescaler = (evt >> ce->count_bits) & ce->prescaler_mask;
                count     = ce->count_mask;
            }
            else
            {
                prescaler = 1 & ce->prescaler_mask;
                count     = ce->count_mask / 2;
            }
        }
    }

    if (count == 0)
    {
        count = 1;
    }
    

    ce->prescaler = prescaler;
    ce->count     = count;

    OS_ASSERT(count != 0);

    return trig_isr;
}

static int os_clockevent_next(os_clockevent_t *ce, os_bool_t force_trig)
{
    os_int32_t trig_isr = os_clockevent_calc_param(ce);

    if (ce->count != 0)
    {
        ce->ops->start(ce, ce->prescaler, ce->count);
    }
    else if (force_trig)
    {
        ce->prescaler = 1 & ce->prescaler_mask;
        ce->count     = 1;
        ce->ops->start(ce, ce->prescaler, ce->count);
    }
    
    return trig_isr;
}

// 启动单次clock事件
void os_clockevent_start_oneshot(os_clockevent_t *ce, os_uint64_t nsec)
{
    os_base_t level;
    
    OS_ASSERT(ce != NULL);

    os_clockevent_stop(ce);     // 停止

    nsec = max(nsec, ce->min_nsec);

    ce->next_nsec    = os_clocksource_gettime() + nsec;
    ce->period_nsec  = 0;
    ce->period_count = 0;

    level = os_irq_lock();
    os_clockevent_next(ce, OS_TRUE);
    os_irq_unlock(level);
}

void os_clockevent_start_period(os_clockevent_t *ce, os_uint64_t nsec)
{
    os_base_t level;
    
    OS_ASSERT(ce != NULL);

    os_clockevent_stop(ce);

    nsec = max(nsec, ce->min_nsec);

    ce->next_nsec    = os_clocksource_gettime() + nsec;
    ce->period_nsec  = nsec;
    ce->period_count = nsec * ce->mult >> ce->shift;

    if (ce->period_count == 0)
    {
        ce->period_count = 1;
    }

    level = os_irq_lock();
    os_clockevent_next(ce, OS_TRUE);
    os_irq_unlock(level);
}

// 停止clock事件
void os_clockevent_stop(os_clockevent_t *ce)
{
    os_base_t level;
    
    OS_ASSERT(ce != NULL);

    level = os_irq_lock();

    ce->next_nsec   = 0;
    ce->period_nsec = 0;

    ce->ops->stop(ce);

    os_irq_unlock(level);
}

void os_clockevent_isr(os_clockevent_t *ce)
{
    os_clocksource_update();
  
    if (os_clockevent_auto_period(ce))
    {
        if (ce->event_handler)
            ce->event_handler(ce);
    }
    else
    {
        ce->ops->stop(ce);
        if (os_clockevent_next(ce, OS_FALSE) != 0)
        {
            if (ce->event_handler)
                ce->event_handler(ce);
        }
    }
}

os_err_t os_clockevent_deinit(os_device_t *dev)
{
    OS_ASSERT(dev != OS_NULL);

    os_clockevent_t *ce;

    ce = (os_clockevent_t *)dev;

    os_clockevent_stop(ce);
    os_clockevent_register_isr(ce, OS_NULL);
    return OS_EOK;
}

const static struct os_device_ops _clockevent_ops =
{
    .deinit = os_clockevent_deinit,
};

#ifdef OS_USING_TICKLESS_LPMGR

#include <lpmgr.h>

static int clockevent_suspend(void *priv, os_uint8_t mode)
{
    OS_ASSERT(priv != OS_NULL);

    os_clockevent_t *ce = (os_clockevent_t *)priv;

    if (!strcmp(LPMGR_TIMER_DEVICE_NAME, ce->parent.name))
        return OS_EOK;
    
    ce->ops->stop(ce);
    
    return OS_EOK;
}

static void clockevent_resume(void *priv, os_uint8_t mode)
{
    OS_ASSERT(priv != OS_NULL);

    os_clockevent_t *ce = (os_clockevent_t *)priv;

    if (!strcmp(LPMGR_TIMER_DEVICE_NAME, ce->parent.name))
        return;

    os_ubase_t level = os_irq_lock();
    
    if (ce->next_nsec != 0)
        os_clockevent_next(ce, OS_TRUE);
    
    os_irq_unlock(level);
}

static const struct os_lpmgr_device_ops clockevent_lpmgr_ops =
{
    .suspend = clockevent_suspend,
    .resume  = clockevent_resume,
};

#endif

void os_clockevent_register(const char *name, os_clockevent_t *ce)
{
    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(ce->ops != OS_NULL);
    OS_ASSERT(ce->ops->start != OS_NULL);
    OS_ASSERT(ce->ops->stop != OS_NULL);
    OS_ASSERT(ce->feature == OS_CLOCKEVENT_FEATURE_ONESHOT || ce->feature == OS_CLOCKEVENT_FEATURE_PERIOD);
    
    memcpy(ce->name, name, min(strlen(name) + 1, OS_CLOCKEVENT_NAME_LENGTH));
    ce->name[OS_CLOCKEVENT_NAME_LENGTH - 1] = 0;
    
    enum{SEC_PER_DAY = 86400, SEC_PER_YEAR = 31536000};
    os_uint64_t sec  = ce->mask / ce->freq;
    os_uint64_t nsec = NSEC_PER_SEC * ce->mask / ce->freq;
    if (nsec == 0)
    {
        os_kprintf("clockevent max_nsec invalid: %Lu\r\n", nsec);
        return;
    }
    
    if(ce->mask > 0xffffffff)
    {
        if(sec <= SEC_PER_YEAR && sec > SEC_PER_DAY)
        {
            sec = SEC_PER_YEAR;
        }
        else if(sec <= SEC_PER_DAY && sec > 600)
        {
            sec = SEC_PER_DAY;  
        }
        else
        {
            sec = 600;
        }
    }

    ce->max_nsec = nsec;

    calc_mult_shift(&ce->mult, &ce->shift, NSEC_PER_SEC, ce->freq, sec);
    calc_mult_shift(&ce->mult_t, &ce->shift_t, ce->freq, NSEC_PER_SEC, sec);

    os_kprintf("ce:%s, mult:%u, shift:%d, mult_t:%u, shift_t:%d\r\n",
               name, ce->mult, ce->shift, ce->mult_t, ce->shift_t);

    ce->parent.ops   = &_clockevent_ops;
    ce->parent.type  = OS_DEVICE_TYPE_CLOCKEVENT;
    os_device_register(&ce->parent, name);

    os_clockevent_enqueue(ce);

#ifdef OS_USING_TICKLESS_LPMGR
    os_lpmgr_device_register(&ce->parent, &clockevent_lpmgr_ops);
#endif
}

#if defined(OS_USING_SHELL) && defined(OS_CLOCKEVENT_SHOW)

#include <shell.h>

void os_clockevent_show(void)
{
    os_base_t level;
    os_clockevent_t *ce;

    os_kprintf("clockevent:\r\n\r\n");

    level = os_irq_lock();

    os_list_for_each_entry(ce, &gs_clockevent_list, os_clockevent_t, list)
    {
        os_kprintf("name:%s\r\n"
                   "rating:%u\r\n"
                   "freq:%u\r\n"
                   "mask:%Lx    %Lu\r\n"
                   "min_nsec:%Lu max_nsec:%Lu\r\n"
                   "next_nsec:%Lu period_nsec:%Lu\r\n"
                   "event_handler:%p\r\n\r\n",
                   ce->name,
                   ce->rating,
                   ce->freq,
                   ce->mask, ce->mask,
                   ce->min_nsec,ce->max_nsec,
                   ce->next_nsec, ce->period_nsec,
                   ce->event_handler);
    }

    os_kprintf("best clockevent is %s\r\n\r\n", gs_best_ce->name);

    os_irq_unlock(level);
}

SH_CMD_EXPORT(list_clockevent, os_clockevent_show, "list_clockevent");

#endif /* OS_USING_SHELL */

