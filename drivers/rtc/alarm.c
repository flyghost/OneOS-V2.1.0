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
 * @file        alarm.c
 *
 * @brief       This file provides alarm related functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>

#define OS_RTC_YEARS_MAX      137
#define OS_ALARM_DELAY        2
#define OS_ALARM_STATE_INITED 0x02
#define OS_ALARM_STATE_START  0x01
#define OS_ALARM_STATE_STOP   0x00

#if (defined(OS_USING_RTC) && defined(OS_USING_ALARM))
static struct os_alarm_container _container;

OS_INLINE os_uint32_t alarm_mkdaysec(struct tm *time)
{
    os_uint32_t sec;

    sec  = time->tm_sec;
    sec += time->tm_min * 60;
    sec += time->tm_hour * 3600;

    return (sec);
}

static os_err_t alarm_set(struct os_alarm *alarm)
{
    os_device_t          *device;
    struct os_rtc_wkalarm wkalarm;
    os_err_t              ret;

    device = os_device_find("rtc");
    if (device == OS_NULL)
    {
        return (OS_ERROR);
    }
    if (alarm->flag & OS_ALARM_STATE_START)
        wkalarm.enable = OS_TRUE;
    else
        wkalarm.enable = OS_FALSE;

    wkalarm.tm_sec  = alarm->wktime.tm_sec;
    wkalarm.tm_min  = alarm->wktime.tm_min;
    wkalarm.tm_hour = alarm->wktime.tm_hour;

    ret = os_device_control(device, OS_DEVICE_CTRL_RTC_SET_ALARM, &wkalarm);
    if ((ret == OS_EOK) && wkalarm.enable)
    {
        ret = os_device_control(device, OS_DEVICE_CTRL_RTC_GET_ALARM, &wkalarm);
        if (ret == OS_EOK)
        {
            /*
              some RTC device like RX8025,it's alarms precision is 1 minute.
              in this case,low level RTC driver should set wkalarm->tm_sec to 0.
            */
            alarm->wktime.tm_sec  = wkalarm.tm_sec;
            alarm->wktime.tm_min  = wkalarm.tm_min;
            alarm->wktime.tm_hour = wkalarm.tm_hour;
        }
    }

    return (ret);
}

static void alarm_wakeup(struct os_alarm *alarm, struct tm *now)
{
    os_uint32_t sec_alarm, sec_now;
    os_bool_t   wakeup = OS_FALSE;
    time_t      timestamp;

    sec_alarm = alarm_mkdaysec(&alarm->wktime);
    sec_now   = alarm_mkdaysec(now);

    if (alarm->flag & OS_ALARM_STATE_START)
    {
        switch (alarm->flag & 0xFF00)
        {
        case OS_ALARM_ONESHOT:
        {
            sec_alarm = mktime(&alarm->wktime);
            sec_now   = mktime(now);
            if (((sec_now - sec_alarm) <= OS_ALARM_DELAY) && (sec_now >= sec_alarm))
            {
                /* stop alarm */
                alarm->flag &= ~OS_ALARM_STATE_START;
                alarm_set(alarm);
                wakeup = OS_TRUE;
            }
        }
        break;
        case OS_ALARM_DAILY:
        {
            if (((sec_now - sec_alarm) <= OS_ALARM_DELAY) && (sec_now >= sec_alarm))
                wakeup = OS_TRUE;
        }
        break;
        case OS_ALARM_WEEKLY:
        {
            /* alarm at wday */
            sec_alarm += alarm->wktime.tm_wday * 24 * 3600;
            sec_now += now->tm_wday * 24 * 3600;

            if (((sec_now - sec_alarm) <= OS_ALARM_DELAY) && (sec_now >= sec_alarm))
                wakeup = OS_TRUE;
        }
        break;
        case OS_ALARM_MONTHLY:
        {
            /* monthly someday generate alarm signals */
            if (alarm->wktime.tm_mday == now->tm_mday)
            {
                if ((sec_now - sec_alarm) <= OS_ALARM_DELAY)
                    wakeup = OS_TRUE;
            }
        }
        break;
        case OS_ALARM_YAERLY:
        {
            if ((alarm->wktime.tm_mday == now->tm_mday) && (alarm->wktime.tm_mon == now->tm_mon))
            {
                if ((sec_now - sec_alarm) <= OS_ALARM_DELAY)
                    wakeup = OS_TRUE;
            }
        }
        break;
        }

        if ((wakeup == OS_TRUE) && (alarm->callback != OS_NULL))
        {
            timestamp = time(OS_NULL);
            alarm->callback(alarm, timestamp);
        }
    }
}

static void alarm_update(os_uint32_t event)
{
    struct os_alarm *alm_prev = OS_NULL, *alm_next = OS_NULL;
    struct os_alarm *alarm;
    os_int32_t       sec_now, sec_alarm, sec_tmp;
    os_int32_t       sec_next = 24 * 3600, sec_prev = 0;
    time_t           timestamp;
    struct tm        now;
    os_list_t *      next;

    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    if (!os_list_isempty(&_container.head))
    {
        /* get time of now */
        timestamp = time(OS_NULL);
        localtime_r(&timestamp, &now);

        for (next = _container.head.next; next != &_container.head; next = next->next)
        {
            alarm = os_list_entry(next, struct os_alarm, list);
            /* check the overtime alarm */
            alarm_wakeup(alarm, &now);
        }

        timestamp = time(OS_NULL);
        localtime_r(&timestamp, &now);
        sec_now = alarm_mkdaysec(&now);

        for (next = _container.head.next; next != &_container.head; next = next->next)
        {
            alarm = os_list_entry(next, struct os_alarm, list);
            /* calculate seconds from 00:00:00 */
            sec_alarm = alarm_mkdaysec(&alarm->wktime);

            if ((alarm->flag & OS_ALARM_STATE_START) && (alarm != _container.current))
            {
                sec_tmp = sec_alarm - sec_now;
                if (sec_tmp > 0)
                {
                    /* find alarm after now(now to 23:59:59) and the most recent */
                    if (sec_tmp < sec_next)
                    {
                        sec_next = sec_tmp;
                        alm_next = alarm;
                    }
                }
                else
                {
                    /* find alarm before now(00:00:00 to now) and furthest from now */
                    if (sec_tmp < sec_prev)
                    {
                        sec_prev = sec_tmp;
                        alm_prev = alarm;
                    }
                }
            }
        }
        /* enable the alarm after now first */
        if (sec_next < 24 * 3600)
        {
            if (alarm_set(alm_next) == OS_EOK)
                _container.current = alm_next;
        }
        else if (sec_prev < 0)
        {
            /* enable the alarm before now */
            if (alarm_set(alm_prev) == OS_EOK)
                _container.current = alm_prev;
        }
    }
    os_mutex_release(&_container.mutex);
}

static os_uint32_t days_of_year_month(int tm_year, int tm_mon)
{
    os_uint32_t ret, year;

    year = tm_year + 1900;
    if (tm_mon == 1)
    {
        ret = 28 + ((!(year % 4) && (year % 100)) || !(year % 400));
    }
    else if (((tm_mon <= 6) && (tm_mon % 2 == 0)) || ((tm_mon > 6) && (tm_mon % 2 == 1)))
    {
        ret = 31;
    }
    else
    {
        ret = 30;
    }

    return (ret);
}

static os_bool_t is_valid_date(struct tm *date)
{
    if ((date->tm_year < 0) || (date->tm_year > OS_RTC_YEARS_MAX))
    {
        return (OS_FALSE);
    }

    if ((date->tm_mon < 0) || (date->tm_mon > 11))
    {
        return (OS_FALSE);
    }

    if ((date->tm_mday < 1) || (date->tm_mday > days_of_year_month(date->tm_year, date->tm_mon)))
    {
        return (OS_FALSE);
    }

    return (OS_TRUE);
}

static os_err_t alarm_setup(os_alarm_t alarm, struct tm *wktime)
{
    os_err_t   ret = OS_ERROR;
    time_t     timestamp;
    struct tm *setup, now;

    setup     = &alarm->wktime;
    *setup    = *wktime;
    timestamp = time(OS_NULL);
    localtime_r(&timestamp, &now);

    /* if these are a "don't care" value,we set them to now*/
    if ((setup->tm_sec > 59) || (setup->tm_sec < 0))
        setup->tm_sec = now.tm_sec;
    if ((setup->tm_min > 59) || (setup->tm_min < 0))
        setup->tm_min = now.tm_min;
    if ((setup->tm_hour > 23) || (setup->tm_hour < 0))
        setup->tm_hour = now.tm_hour;

    switch (alarm->flag & 0xFF00)
    {
    case OS_ALARM_DAILY:
    {
        /* do nothing but needed */
    }
    break;
    case OS_ALARM_ONESHOT:
    {
        /* if these are "don't care" value we set them to now */
        if (setup->tm_year == OS_ALARM_TM_NOW)
            setup->tm_year = now.tm_year;
        if (setup->tm_mon == OS_ALARM_TM_NOW)
            setup->tm_mon = now.tm_mon;
        if (setup->tm_mday == OS_ALARM_TM_NOW)
            setup->tm_mday = now.tm_mday;
        /* make sure the setup is valid */
        if (!is_valid_date(setup))
            goto _exit;
    }
    break;
    case OS_ALARM_WEEKLY:
    {
        /* if tm_wday is a "don't care" value we set it to now */
        if ((setup->tm_wday < 0) || (setup->tm_wday > 6))
            setup->tm_wday = now.tm_wday;
    }
    break;
    case OS_ALARM_MONTHLY:
    {
        /* if tm_mday is a "don't care" value we set it to now */
        if ((setup->tm_mday < 1) || (setup->tm_mday > 31))
            setup->tm_mday = now.tm_mday;
    }
    break;
    case OS_ALARM_YAERLY:
    {
        /* if tm_mon is a "don't care" value we set it to now */
        if ((setup->tm_mon < 0) || (setup->tm_mon > 11))
            setup->tm_mon = now.tm_mon;

        if (setup->tm_mon == 1)
        {
            /* tm_mon is February */

            /* tm_mday should be 1~29.otherwise,it's a "don't care" value */
            if ((setup->tm_mday < 1) || (setup->tm_mday > 29))
                setup->tm_mday = now.tm_mday;
        }
        else if (((setup->tm_mon <= 6) && (setup->tm_mon % 2 == 0)) ||
                 ((setup->tm_mon > 6) && (setup->tm_mon % 2 == 1)))
        {
            /* Jan,Mar,May,Jul,Aug,Oct,Dec */

            /* tm_mday should be 1~31.otherwise,it's a "don't care" value */
            if ((setup->tm_mday < 1) || (setup->tm_mday > 31))
                setup->tm_mday = now.tm_mday;
        }
        else
        {
            /* tm_mday should be 1~30.otherwise,it's a "don't care" value */
            if ((setup->tm_mday < 1) || (setup->tm_mday > 30))
                setup->tm_mday = now.tm_mday;
        }
    }
    break;
    default:
    {
        goto _exit;
    }
    }

    if ((setup->tm_hour == 23) && (setup->tm_min == 59) && (setup->tm_sec == 59))
    {
        /*
           for insurance purposes, we will generate an alarm
           signal two seconds ahead of.
        */
        setup->tm_sec = 60 - OS_ALARM_DELAY;
    }
    /* set initialized state */
    alarm->flag |= OS_ALARM_STATE_INITED;
    ret = OS_EOK;

_exit:

    return (ret);
}

/**
 ***********************************************************************************************************************
 * @brief           send a rtc alarm event
 *
 * @details         [Details description(Optional).]
 *
 * @attention       [Attention description(Optional).]
 *
 * @param[in]       dev             Pointer to RTC device(currently unused,you can ignore it).
 * @param[in]       event           RTC event(currently unused).
 *
 * @return          none
 ***********************************************************************************************************************
 */
void os_alarm_update(os_device_t *dev, os_uint32_t event)
{
    os_event_send(&_container.event, 1);
}

/**
 ***********************************************************************************************************************
 * @brief           modify the alarm setup
 *
 * @param[in]       alarm           Alarm pointer.
 * @param[in]       cmd             Control command.
 * @param[in]       arg             Argument.
 *
 * @return          Return modify status.
 * @retval          OS_EOK          Modify success.
 * @retval          OS_ERROR        Modify failed.
 ***********************************************************************************************************************
 */
os_err_t os_alarm_control(os_alarm_t alarm, int cmd, void *arg)
{
    os_err_t ret = OS_ERROR;

    OS_ASSERT(alarm != OS_NULL);

    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    switch (cmd)
    {
    case OS_ALARM_CTRL_MODIFY:
    {
        struct os_alarm_setup *setup;

        OS_ASSERT(arg != OS_NULL);
        setup = arg;
        os_alarm_stop(alarm);
        alarm->flag   = setup->flag & 0xFF00;
        alarm->wktime = setup->wktime;
        ret           = alarm_setup(alarm, &alarm->wktime);
    }
    break;
    }

    os_mutex_release(&_container.mutex);

    return (ret);
}

/**
 ***********************************************************************************************************************
 * @brief           start an alarm
 *
 * @param[in]       alarm           Alarm pointer.
 *
 * @return          Return start status.
 * @retval          OS_EOK          Start success.
 * @retval          OS_ERROR        Start failed.
 ***********************************************************************************************************************
 */
os_err_t os_alarm_start(os_alarm_t alarm)
{
    os_int32_t sec_now, sec_old, sec_new;
    os_err_t   ret = OS_ERROR;
    time_t     timestamp;
    struct tm  now;

    if (alarm == OS_NULL)
        return (ret);
    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    if (!(alarm->flag & OS_ALARM_STATE_INITED))
    {
        if (alarm_setup(alarm, &alarm->wktime) != OS_EOK)
            goto _exit;
    }
    if ((alarm->flag & 0x01) == OS_ALARM_STATE_STOP)
    {
        timestamp = time(OS_NULL);
        localtime_r(&timestamp, &now);

        alarm->flag |= OS_ALARM_STATE_START;
        /* set alarm */
        if (_container.current == OS_NULL)
        {
            ret = alarm_set(alarm);
        }
        else
        {
            sec_now = alarm_mkdaysec(&now);
            sec_old = alarm_mkdaysec(&_container.current->wktime);
            sec_new = alarm_mkdaysec(&alarm->wktime);

            if ((sec_new < sec_old) && (sec_new > sec_now))
            {
                ret = alarm_set(alarm);
            }
            else if ((sec_new > sec_now) && (sec_old < sec_now))
            {
                ret = alarm_set(alarm);
            }
            else if ((sec_new < sec_old) && (sec_old < sec_now))
            {
                ret = alarm_set(alarm);
            }
            else
            {
                ret = OS_EOK;
                goto _exit;
            }
        }

        if (ret == OS_EOK)
        {
            _container.current = alarm;
        }
    }

_exit:
    os_mutex_release(&_container.mutex);

    return (ret);
}

/**
 ***********************************************************************************************************************
 * @brief           stop an alarm
 *
 * @param[in]       alarm           pointer to alarm
 *
 * @return          Return stop status.
 * @retval          OS_EOK          Stop success.
 * @retval          OS_ERROR        Stop failed.
 ***********************************************************************************************************************
 */
os_err_t os_alarm_stop(os_alarm_t alarm)
{
    os_err_t ret = OS_ERROR;

    if (alarm == OS_NULL)
        return (ret);
    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    if (!(alarm->flag & OS_ALARM_STATE_START))
        goto _exit;
    /* stop alarm */
    alarm->flag &= ~OS_ALARM_STATE_START;

    if (_container.current == alarm)
    {
        ret                = alarm_set(alarm);
        _container.current = OS_NULL;
    }

    if (ret == OS_EOK)
        alarm_update(0);

_exit:
    os_mutex_release(&_container.mutex);

    return (ret);
}

/**
 ***********************************************************************************************************************
 * @brief           delete an alarm
 *
 * @param[in]       alarm           alarm pointer to alarm
 *
 * @return          Return delete status.
 * @retval          OS_EOK          Delete success.
 * @retval          OS_ERROR        Delete failed.
 ***********************************************************************************************************************
 */
os_err_t os_alarm_delete(os_alarm_t alarm)
{
    os_err_t ret = OS_ERROR;

    if (alarm == OS_NULL)
        return (ret);
    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    /* stop the alarm */
    alarm->flag &= ~OS_ALARM_STATE_START;
    if (_container.current == alarm)
    {
        ret                = alarm_set(alarm);
        _container.current = OS_NULL;
        /* set new alarm if necessary */
        alarm_update(0);
    }
    os_list_remove(&alarm->list);
    os_free(alarm);

    os_mutex_release(&_container.mutex);

    return (ret);
}

/**
 ***********************************************************************************************************************
 * @brief           create an alarm
 *
 * @param[in]       callback
 * @param[in]       setup        pointer to setup infomation
 *
 * @return          Return create alarm pointer(os_alarm_t).
 ***********************************************************************************************************************
 */
os_alarm_t os_alarm_create(os_alarm_callback_t callback, struct os_alarm_setup *setup)
{
    struct os_alarm *alarm;

    if (setup == OS_NULL)
        return (OS_NULL);
    alarm = os_calloc(1, sizeof(struct os_alarm));
    if (alarm == OS_NULL)
        return (OS_NULL);
    
    os_list_init(&alarm->list);

    alarm->wktime   = setup->wktime;
    alarm->flag     = setup->flag & 0xFF00;
    alarm->callback = callback;
    os_mutex_take(&_container.mutex, OS_WAIT_FOREVER);
    os_list_insert_after(&_container.head, &alarm->list);
    os_mutex_release(&_container.mutex);

    return (alarm);
}

/**
 ***********************************************************************************************************************
 * @brief           rtc alarm service task entry
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void os_alarmsvc_task_init(void *param)
{
    os_uint32_t recv;

    _container.current = OS_NULL;

    while (1)
    {
        if (os_event_recv(&_container.event,
                          0xFFFF,
                          OS_EVENT_FLAG_OR | OS_EVENT_FLAG_CLEAR,
                          OS_WAIT_FOREVER,
                          &recv) == OS_EOK)
        {
            alarm_update(recv);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           initialize alarm service system
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
void os_alarm_system_init(void)
{
    os_task_t tid;

    os_list_init(&_container.head);
    os_event_init(&_container.event, "alarmsvc", OS_IPC_FLAG_FIFO);
    os_mutex_init(&_container.mutex, "alarmsvc", OS_IPC_FLAG_FIFO);

    tid = os_task_create("alarmsvc", os_alarmsvc_task_init, OS_NULL, 512, 8, 1);
    if (tid != OS_NULL)
        os_task_startup(tid);
}
#endif
