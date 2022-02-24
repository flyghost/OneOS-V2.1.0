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
 * @file        cputime.c
 *
 * @brief       This file provides functions for cputime calculation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <arch_interrupt.h>
#include <os_clock.h>
#include <drv_cfg.h>
#include <string.h>
#include <timer/clocksource.h>

static os_clocksource_t *gs_best_cs = OS_NULL;  // 当前选中的时钟源

/**
 * @brief static os_list_node_t gs_clocksource_list = {&(gs_clocksource_list), &(gs_clocksource_list)}
 * 
 */
static os_list_node_t gs_clocksource_list = OS_LIST_INIT(gs_clocksource_list);

/**
 * @brief 返回当前选中的时钟源
 * 
 * @return os_clocksource_t* 
 */
os_clocksource_t *os_clocksource_best(void)
{
    return gs_best_cs;
}

/**
 * @brief 更新输入的时钟源的计数值
 * 
 * @param cs 
 */
static void os_clocksource_update_cs(os_clocksource_t *cs)
{
    os_uint64_t nsec;
    os_uint64_t count;

    count = cs->read(cs);       // 读取当前时钟的tick值

    nsec = ((count - cs->last_update_count) & cs->mask) * cs->mult >> cs->shift;    // 真实时钟源粒度转化到cs粒度（纳秒级粒度）

    cs->last_update_count = count;
    cs->last_update_nsec += nsec;
}

// 更新所有时钟源
void os_clocksource_update(void)
{
    os_base_t level;
    os_clocksource_t *cs;
    
    level = os_irq_lock();

#if 0
    for (cs = os_list_entry((&gs_clocksource_list)->next, os_clocksource_t, list);
         &cs->list != (&gs_clocksource_list);
         cs = os_list_entry(cs->list.next, os_clocksource_t, list))
    {
        os_clocksource_update_cs(cs);
    }
#endif

    // 循环时钟源链表，更新时钟源计数值
    os_list_for_each_entry(cs, &gs_clocksource_list, os_clocksource_t, list)
    {
        os_clocksource_update_cs(cs);
    }

   os_irq_unlock(level);
}

/**
 * @brief 获取输入时钟源的纳秒级的时间粒度
 * 
 * @param cs 时钟源
 * @return os_uint64_t 纳秒级时间粒度
 */
static os_uint64_t os_clocksource_gettime_cs(os_clocksource_t *cs)
{
    os_base_t level = os_irq_lock();

    os_clocksource_update_cs(cs);
    
    os_uint64_t nsec = cs->last_update_nsec;

    os_irq_unlock(level);

    return nsec;
}

/**
 * @brief 获取时钟源时间，单位：纳秒
 * 如果没有设置时钟源，则使用系统心跳转换成纳秒级粒度返回
 * 
 * @return os_uint64_t 当前时钟源的纳秒级计数值
 */
os_uint64_t os_clocksource_gettime(void)
{
    if (gs_best_cs != OS_NULL)
    {
        return os_clocksource_gettime_cs(gs_best_cs);
    }
    else
    {
        return (os_uint64_t)os_tick_get() * NSEC_PER_SEC / OS_TICK_PER_SECOND;
    }
}

/**
 * @brief 粗略的延迟
 * 
 * @param nsec 
 */
static void __os_clocksource_ndelay(os_uint64_t nsec)
{
    os_uint64_t i;
    for (i = 0; i < nsec; i++);
}

/**
 * @brief 延迟
 * 
 * @param nsec 
 */
void os_clocksource_ndelay(os_uint64_t nsec)
{
    os_uint64_t count_tmp, count_delta, count_half;

    os_clocksource_t *cs = gs_best_cs;

    if (cs == OS_NULL)
    {
        __os_clocksource_ndelay(nsec);
        return;
    }

    count_tmp   = cs->read(cs);                     // 读取时钟源计数值，表示期望等待到达的计数值
    count_delta = nsec * cs->mult_t >> cs->shift_t; // 转化时间粒度：纳秒--->硬件counter
    count_half  = (cs->mask >> 1) + 1;              // 时钟源精度的一半

    while (count_delta > 0)
    {
        if (count_delta > cs->mask)                 // 延迟的时间大于软件规定的时钟源的delta counter的值，所以取一半来计数
        {
            count_tmp   += count_half;              // 当前期望值
            count_delta -= count_half;              // 剩余期望值
        }
        else if (count_delta > count_half)          // 延迟的时间大于一半且小于最大的mask，
        {
            count_tmp   += count_delta >> 1;
            count_delta -= count_delta >> 1;
        }
        else
        {
            count_tmp   += count_delta;
            count_delta  = 0;
        }

        while (((count_tmp  - cs->read(cs)) & cs->mask) <= count_half);
    }
}

/**
 * @brief 选取时钟源
 * 
 * 由于时钟源链表是根据rating来排序的，所以表头总是优先级最高的时钟源
 * 
 */
static void os_clocksource_select(void)
{
    if (gs_best_cs != OS_NULL)
    {
        os_device_close(&gs_best_cs->parent);
    }

    gs_best_cs = os_list_first_entry_or_null(&gs_clocksource_list, os_clocksource_t, list);
    
    OS_ASSERT(gs_best_cs != OS_NULL);
        
    if (os_device_open(&gs_best_cs->parent) != OS_EOK)
    {
        os_kprintf("open clocksource %s failed.\r\n", gs_best_cs->name);
        while (1);
    }
}

/**
 * @brief 插入新的时钟源
 * 
 * 时钟源链表按照rating从高到低排列
 * 
 * @param cs 
 */
static void os_clocksource_enqueue(os_clocksource_t *cs)
{
    os_list_node_t *entry = &gs_clocksource_list;
    os_clocksource_t *tmp;

    // 循环所有的时钟源，查找rating大于等于待插入的时钟源，然后将新的时钟源插入后面
    os_list_for_each_entry(tmp, &gs_clocksource_list, os_clocksource_t, list)
    {
        if (tmp->rating < cs->rating)
            break;
        entry = &tmp->list;
    }
    os_list_add(entry, &cs->list);
}

#ifdef OS_USING_TICKLESS_LPMGR

static void os_clocksource_update_ns(os_uint64_t nsec, void *priv)
{
    os_clocksource_t *cs = priv;

    os_ubase_t level = os_irq_lock();

    cs->last_update_nsec += nsec;

    os_irq_unlock(level);
}

#include <lpmgr.h>

static int clocksource_suspend(void *priv, os_uint8_t mode)
{
    OS_ASSERT(priv != OS_NULL);

    os_clocksource_t *cs = (os_clocksource_t *)priv;

    os_ubase_t level = os_irq_lock();
    
    os_clocksource_update_cs(cs);

    os_irq_unlock(level);
    
    return OS_EOK;
}

static void clocksource_resume(void *priv, os_uint8_t mode)
{
    OS_ASSERT(priv != OS_NULL);

    os_clocksource_t *cs = (os_clocksource_t *)priv;

    os_ubase_t level = os_irq_lock();

    cs->last_update_count = cs->read(cs);

    os_irq_unlock(level);
}

static const struct os_lpmgr_device_ops clocksource_lpmgr_ops =
{
    .suspend = clocksource_suspend,
    .resume  = clocksource_resume,
};

#endif

/**
 * @brief 检测时钟源是否有效
 * 
 * 根据时钟源硬件counter是否发生改变来判断是否有效
 * 
 * @param cs 
 * @return os_bool_t 
 */
static os_bool_t os_clocksource_valid(os_clocksource_t *cs)
{
    volatile int i = 10000;
    os_uint64_t start, end;

    start = cs->read(cs);
    
    while (i-- && start == (end = cs->read(cs)));

    return (start != end);
}

/**
 * @brief 注册新的时钟源
 * 
 * @param name 设备名
 * @param cs 
 */
void os_clocksource_register(const char *name, os_clocksource_t *cs)
{
    OS_ASSERT(cs != OS_NULL);
    OS_ASSERT(cs->read != OS_NULL);

    memcpy(cs->name, name, min(strlen(name) + 1, OS_CLOCKSOURCE_NAME_LENGTH));
    cs->name[OS_CLOCKSOURCE_NAME_LENGTH - 1] = 0;
    
    enum{SEC_PER_DAY = 86400, SEC_PER_YEAR = 31536000};
    os_uint64_t sec  = cs->mask / cs->freq;                     // 时钟源可以计算的最大秒数：mask/freq = mask(最大的counter数) * Time_per_counter(每个计数值代表多长时间：秒) = 最大可以计算的秒
    os_uint64_t nsec = NSEC_PER_SEC * cs->mask / cs->freq;      // 单位换算，最大可以计算的纳秒
    if (nsec == 0)
    {
        os_kprintf("clocksource max_nsec invalid: %Lu\r\n", nsec);
        return;
    }

    if(cs->mask > 0xffffffff)                               // 硬件counter大于32位
    {
        if(sec <= SEC_PER_YEAR && sec > SEC_PER_DAY)        // 硬件counter最大的描述大于一天小于一年，则取一年
        {
            sec = SEC_PER_YEAR;
        }
        else if(sec <= SEC_PER_DAY && sec > 600)            // 大于一小时
        {
            sec = SEC_PER_DAY;  
        }
        else                                                // 大于一天
        {
            sec = 600;
        }
    }

    cs->max_nsec = nsec;
    cs->min_nsec = NSEC_PER_SEC / cs->freq;

    calc_mult_shift(&cs->mult, &cs->shift, cs->freq, NSEC_PER_SEC, sec);            // 计算硬件时钟源---->纳秒的转化值
    calc_mult_shift(&cs->mult_t, &cs->shift_t, NSEC_PER_SEC, cs->freq, sec);        // 纳秒到硬件时钟源的转换值

    os_kprintf("cs:%s, mult:%u, shift:%d, mult_t:%u, shift_t:%d\r\n",
               name, cs->mult, cs->shift, cs->mult_t, cs->shift_t);

    if (!os_clocksource_valid(cs))                                                  // 当前时钟源是否有效
    {
        os_kprintf("invalid clocksource %s.\r\n", name);
        return;
    }

    cs->parent.type = OS_DEVICE_TYPE_CLOCKSOURCE;
    os_device_register(&cs->parent, name);

#ifdef OS_USING_TICKLESS_LPMGR
    os_lpmgr_update_callback_register(os_clocksource_update_ns, cs);
    os_lpmgr_device_register_high_prio(&cs->parent, &clocksource_lpmgr_ops);
#endif

    os_clocksource_enqueue(cs);         // 插入时钟源
    os_clocksource_select();            // 选择优先级最高的时钟源
}

void os_hw_us_delay(os_uint32_t us)
{
    os_clocksource_ndelay(us * NSEC_PER_USEC);
}

#if defined(OS_USING_SHELL) && defined(OS_CLOCKSOURCE_SHOW)

#include <shell.h>

static void os_clocksource_show(void)
{
    os_base_t level;
    os_clocksource_t *cs;

    os_kprintf("clocksource:\r\n\r\n");

    level = os_irq_lock();

    os_list_for_each_entry(cs, &gs_clocksource_list, os_clocksource_t, list)
    {
        os_kprintf("name:%s\r\n"
                   "rating:%u\r\n"
                   "freq:%u\r\n"
                   "mask:%Lx    %Lu\r\n"
                   "min_nsec:%Lu max_nsec:%Lu\r\n"
                   "last_update_nsec: %Lu\r\n"
                   "last_update_count:%Lu\r\n"
                   "current nsec: %Lu\r\n\r\n",
                   cs->name,
                   cs->rating,
                   cs->freq,
                   cs->mask, cs->mask,
                   cs->min_nsec,cs->max_nsec,
                   cs->last_update_nsec,
                   cs->last_update_count,
                   os_clocksource_gettime_cs(cs));
    }

    os_kprintf("best clocksource is %s\r\n\r\n", gs_best_cs->name);

    os_irq_unlock(level);
}

SH_CMD_EXPORT(list_clocksource, os_clocksource_show, "list_clocksource");

#endif /* OS_USING_SHELL */

