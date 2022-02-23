
#include <board.h>
#include <arch_interrupt.h>
#include <device.h>
#include <drv_common.h>
#include <drv_cfg.h>
#include <stdio.h>
#include <shell.h>
#include "os_stddef.h"
#include <os_event.h>
#include <os_clock.h>
#include <stdlib.h>
#include <drv_gpio.h>
#ifdef OS_USING_TICKLESS_LPMGR

#include <lpmgr/lpmgr.h>

/* 
 * example 1:
 * enter sleep mode, Calculate sleep time according to all tasks of the system
 * The user task needs to call os_lpmgr_request(SYS_SLEEP_MODE_IDLE); and os_lpmgr_release(SYS_SLEEP_MODE_IDLE);
 */
void request_sleep(int argc, char *argv[])
{
    if (argc != 2)
    {
        os_kprintf("usage: request_sleep <sleep_mode>\r\n");
        os_kprintf("example: request_sleep 2\r\n");
        return;
    }
    
    os_lpmgr_request((lpmgr_sleep_mode_e)atoi(argv[1])); /* SYS_SLEEP_MODE_LIGHT */
}


/* OS_BOARD_INIT(timer_app_init); // Called when the system is initialized, the system calculates the sleep time according to the task   */
SH_CMD_EXPORT(request_sleep, request_sleep, "request_sleep <sleep_mode>");


/* 
 * example 2:
 * Setting the timer time is also the time for regular sleep; 
 * Note: To make the timer time effective, the time of the next priority task calculated 
 * automatically by the system is greater than this timer time
 */ 
static os_timer_t *timer1;

static void requlst_entry(void *parameter)
{
    os_lpmgr_request(SYS_SLEEP_MODE_IDLE);
    os_kprintf("user task, start current tick: %d\r\n",os_tick_get());
    os_lpmgr_release(SYS_SLEEP_MODE_IDLE);
}

void requst_wakeup(int mode, int timeout)
{
    timer1 = os_timer_create("requst_wake_up",  requlst_entry, OS_NULL,
                             OS_TICK_PER_SECOND * timeout, OS_TIMER_FLAG_PERIODIC);
    if (timer1 == OS_NULL)
    {
        os_kprintf("[%s]-[%d], os_timer_create err!\r\n", __FILE__, __LINE__);
        return;
    }
    
    os_timer_start(timer1);
    os_lpmgr_request((lpmgr_sleep_mode_e)mode);

}

void requst_wake_up(int argc, char *argv[])
{
    os_uint8_t timeout;
    os_uint8_t mode;

    if (argc != 3)
    {
        os_kprintf("usage: requst_wake_up <mode> <timeout_tick>\r\n");
        os_kprintf("example: requst_wake_up 2 5\r\n");
        return;
    }

    mode = atoi(argv[1]);
    timeout = atoi(argv[2]);
    os_kprintf("[%s]-[%d], mode[%d], timeout[%d]\r\n", __FILE__, __LINE__, mode, timeout);
    requst_wakeup(mode, timeout);
}

//OS_BOARD_INIT(requst_wake_up);

SH_CMD_EXPORT(requst_wake_up, requst_wake_up, "requst_wake_up");



/* 
 * example 3:
 * Wake up sleep mode by external interrupt, continue to sleep after processing some tasks
 * 
 */ 
#define WAKEUP_EVENT_BUTTON                 (1 << 0)
#define LED_PIN                                   led_table[0].pin
#define WAKEUP_PIN                               key_table[0].pin
#define WAKEUP_APP_STACK_SIZE        512

typedef struct _tag_sleep_info_s
{
    os_sem_t  *wakeup_sem;
    os_uint8_t mode;
}sleep_info_s;

static sleep_info_s sleep_info;

static void wakeup_callback(void *args)
{
    os_kprintf("wake up[%s]-[%d], pin[%d], tick[%d]\r\n", __FILE__, __LINE__, (os_uint32_t)args, os_tick_get());
    os_sem_post(sleep_info.wakeup_sem);
}

static void wakeup_irq_init(void)
{
    os_pin_mode(WAKEUP_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(WAKEUP_PIN, PIN_IRQ_MODE_FALLING, wakeup_callback, (void *)WAKEUP_PIN);
    os_pin_irq_enable(WAKEUP_PIN, PIN_IRQ_ENABLE);
}

static void led_work(os_uint32_t cnt)
{
    os_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    
    do
    {
        os_pin_write(LED_PIN, PIN_LOW);
        os_task_msleep(500);
        os_pin_write(LED_PIN, PIN_HIGH);
        os_task_msleep(500);
    }while(cnt-- > 0);
}

static void wakeup_app_entry(void *parameter)
{
    sleep_info_s *sleep_info;
    OS_ASSERT(parameter != OS_NULL);

    sleep_info = (sleep_info_s *)parameter;
    
    wakeup_irq_init();
    os_kprintf("request sleep [%s]-[%d], mode[%d]\r\n", __FILE__, __LINE__, sleep_info->mode);
    os_lpmgr_request((lpmgr_sleep_mode_e)sleep_info->mode);


    while (1)
    {
        os_sem_wait(sleep_info->wakeup_sem,OS_WAIT_FOREVER);
        os_lpmgr_request(SYS_SLEEP_MODE_NONE);
        os_kprintf("wake up, enter user code[%s]-[%d], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
        
        led_work(6);
        
        os_kprintf("wake up, exit user code[%s]-[%d], tick[%d]\r\n", __FILE__, __LINE__, os_tick_get());
        os_lpmgr_release(SYS_SLEEP_MODE_NONE);
        
    }
}

static int wakeup_irq(int argc, char *argv[])
{
    os_task_t *tid;

    if (argc != 2)
    {
        os_kprintf("usage: wakeup_irq <mode>\r\n");
        os_kprintf("example: wakeup_irq 2\r\n");
        return -1;
    }

    sleep_info.mode = atoi(argv[1]);

    sleep_info.wakeup_sem = os_sem_create("wakeup_irq", 0, 1);
    OS_ASSERT(sleep_info.wakeup_sem != OS_NULL);

    tid = os_task_create("wakeup_app", wakeup_app_entry, (void *)&sleep_info,
                           WAKEUP_APP_STACK_SIZE, 5);
    OS_ASSERT(tid != OS_NULL);

    os_task_startup(tid);

    return 0;
}

SH_CMD_EXPORT(wakeup_irq, wakeup_irq, "wakeup_irq");

os_err_t dev_suspend(void *priv, os_uint8_t mode)
{
    os_kprintf("[%s]-[%d], resume mode[%d]\r\n", __FILE__, __LINE__, mode);
    return OS_EOK;
}

void dev_resume(void *priv, os_uint8_t mode)
{
    os_kprintf("[%s]-[%d], resume mode[%d]\r\n", __FILE__, __LINE__, mode);
}

const struct os_lpmgr_device_ops dev_ops = 
{
    .suspend = dev_suspend,
    .resume = dev_resume,
};

void lp_register_dev(void)
{
    os_err_t ret;
    struct os_device *device;

    device = os_device_find(OS_CONSOLE_DEVICE_NAME);

    ret = os_lpmgr_device_register(device, &dev_ops);
    os_kprintf("dev [%s] register ret[%d]\r\n", OS_CONSOLE_DEVICE_NAME, ret);
}

SH_CMD_EXPORT(lp_register_dev, lp_register_dev, "lp_register_dev");



/**
 用户notify回调函数注册,进入低功耗前不能用串口打印，会影响低功耗的进入
***/

void user_notify(os_lpmgr_sys_e event, lpmgr_sleep_mode_e mode, void *data)
{
    if (event == SYS_ENTER_SLEEP)
    {
        os_kprintf("[%s]-[%d], event[%d], mode[%d]\r\n", __FILE__, __LINE__, event, mode);
    }
    else
    {
        os_kprintf("[%s]-[%d], event[%d], mode[%d]\r\n", __FILE__, __LINE__, event, mode);
    }
}

void lp_register_notify(void)
{
    os_lpmgr_notify_set(user_notify, (void *)0);
}

SH_CMD_EXPORT(lp_register_notify, lp_register_notify, "lp_register_notify");



static os_err_t pin_suspend(struct os_device *device, os_uint8_t mode)
{
    os_pin_write(73,1);    //(E,9):led off
    return 0;
}

static void pin_resume(struct os_device *device, os_uint8_t mode)
{
    os_pin_write(73,0);    //(E,9):led on
}

static struct os_lpmgr_device_ops pin_lpmgr_ops =
{
    pin_suspend,
    pin_resume,
};

os_err_t pin_lpmgr_test(int argc, char *argv[])
{
    struct os_device *device = os_device_find("pin_0");
    os_pin_mode(73,PIN_MODE_OUTPUT);
    os_pin_write(73,0);    //(E,9):led on
    os_lpmgr_device_register(device, &pin_lpmgr_ops);

    return 0;
}

SH_CMD_EXPORT(pin_lpmgr_test,pin_lpmgr_test, "pin_lpmgr_test");


#endif /* OS_USING_TICKLESS_LPMGR */

#ifdef OS_USING_SIMPLE_LPM

#include <lpm.h>

static void simple_lpm_entry(void *parameter)
{
    os_uint32_t sleep_cnt = 10;
    os_uint32_t timeout_ms;

    timeout_ms = *(os_uint32_t *)parameter;
    while (sleep_cnt--)
    {
        os_task_msleep(3000);
        lpm_start(timeout_ms);
        os_kprintf("[%s]-[%d], timeout[%d ms], cur_tick[%d]\r\n", __FILE__, __LINE__, timeout_ms, os_tick_get());
    }
}

void lpm_start_test(int argc, char *argv[])
{
    os_task_t *tid;
    os_uint32_t timeout_ms;
    
    if (argc != 2)
    {
        os_kprintf("usage: lpm_start_test <time ms>\r\n");
        os_kprintf("example: lpm_start_test  5000\r\n");
        return;
    }

    timeout_ms = atoi(argv[1]);
        
    tid = os_task_create("simple lpm", simple_lpm_entry, (void *)&timeout_ms,
                       512, 5);
    OS_ASSERT(tid != OS_NULL);
    os_task_startup(tid);

}

SH_CMD_EXPORT(lpm_start_test, lpm_start_test, "lpm_start_test");

#endif

