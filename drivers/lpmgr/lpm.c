#include <os_errno.h>
#include <os_util.h>
#include <os_clock.h>
#include <arch_interrupt.h>
#include <os_stddef.h>


OS_WEAK os_err_t lpm_timer_start_once(os_uint32_t timeout_ms)
{
    (void)timeout_ms;

    return OS_EOK;
}

OS_WEAK void lpm_enter_sleep(void)
{
    ;
}

os_err_t lpm_start(os_uint32_t timeout_ms)
{
    os_uint32_t level;
    os_err_t ret = OS_EOK;
    
    level = os_irq_lock();

    ret = lpm_timer_start_once(timeout_ms);
    if (ret != OS_EOK)
    {
        os_irq_unlock(level);
        return ret;
    }

    lpm_enter_sleep();
    
    os_irq_unlock(level);
    
    os_kprintf("[%s]-[%d], timeout[%d ms], cur_tick[%d]\r\n", __FILE__, __LINE__, timeout_ms, os_tick_get());

    return OS_EOK;
}

