
#include <board.h>
#include <os_clock.h>
#include <timer/clockevent.h>
#include <sys/time.h>
#include <lpm.h>

os_clockevent_t *lpm_ce = NULL;
extern void SystemClock_Config(void);

os_err_t lpm_timer_start_once(os_uint32_t timeout_ms)
{
    os_uint64_t sleep_ns;

    sleep_ns = NSEC_PER_MSEC * (os_uint64_t)timeout_ms;
    
    if (lpm_ce->min_nsec > sleep_ns)
    {
        os_kprintf("time too short, timeout_ms[%d ms], time[%d ms, %d ms]\r\n", 
            timeout_ms, lpm_ce->min_nsec / NSEC_PER_MSEC, lpm_ce->max_nsec / NSEC_PER_MSEC);
        return OS_EINVAL;
    }
    
    if (lpm_ce->max_nsec < sleep_ns)
    {
        os_kprintf("time too long, timeout_ms[%d ms], time[%d ms, %d ms]\r\n", 
            timeout_ms, lpm_ce->min_nsec / NSEC_PER_MSEC, lpm_ce->max_nsec / NSEC_PER_MSEC);
        return OS_EINVAL;
    }
    
    os_clockevent_start_oneshot(lpm_ce, sleep_ns);

    return OS_EOK;
}

typedef struct
{
    os_uint32_t sys_load;
    os_uint32_t sys_ctrl;
}systick_val_sava_s;

static systick_val_sava_s sys_save;
static void cortexm_systick_tick_deinit(void)
{
    sys_save.sys_load = SysTick->LOAD;
    sys_save.sys_ctrl = SysTick->CTRL;    
    SysTick->CTRL &= ~(1 << 0);
}

static void cortexm_systick_tick_init(void)
{
    SysTick->LOAD = sys_save.sys_load;
    SysTick->VAL   = 0;
    SysTick->CTRL = sys_save.sys_ctrl;
}

void lpm_enter_sleep(void)
{
    cortexm_systick_tick_deinit();
    /* Enter STOP 2 mode  */
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

    /* Re-configure the system clock */
    SystemClock_Config();

    cortexm_systick_tick_init();
}

int lpm_init(void)
{    
    lpm_ce = (os_clockevent_t *)os_device_find("lptim1");
    OS_ASSERT(lpm_ce != NULL);
    
    /* Enable power clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    return 0;
}

OS_DEVICE_INIT(lpm_init, OS_INIT_SUBLEVEL_MIDDLE);

