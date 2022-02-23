#include "os_sem.h"
#include <os_task.h>

#include "sdk_common.h"
#include "nrf_sdh.h"
#include "sdk_config.h"
#include "app_util_platform.h"
#include "nrf_sdh_oneos.h"

#define NRF_LOG_MODULE_NAME nrf_sdh_oneos
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static os_sem_t sem_static;
static nrf_sdh_oneos_task_hook_t m_task_hook;

void SD_EVT_IRQHandler(void)
{
    os_sem_post(&sem_static);
}

/* This function gets events from the SoftDevice and processes them. */
static void softdevice_task(void *pvParameter)
{
    NRF_LOG_DEBUG("Enter softdevice_task.");

    if (m_task_hook != OS_NULL)
    {
        m_task_hook(pvParameter);
    }

    while (OS_TRUE)
    {
        nrf_sdh_evts_poll(); /* let the handlers run first, incase the EVENT occured before creating this task */

        os_sem_wait(&sem_static, OS_WAIT_FOREVER);
    }
}

void nrf_sdh_oneos_init(nrf_sdh_oneos_task_hook_t hook_fn, void *p_context)
{
    NRF_LOG_DEBUG("Creating a SoftDevice task.");

    m_task_hook = hook_fn;

    os_sem_init(&sem_static, "softdevice_sem", 0, OS_SEM_MAX_VALUE);

    static os_task_t *task = OS_NULL;
    task = os_task_create("softdevice",
                          softdevice_task,
                          p_context,
                          8192,
                          22);
    if (task != OS_NULL)
    {
        os_task_startup(task);
    }
    else
    {
        NRF_LOG_ERROR("SoftDevice task not created.");
    }
    return;
}
