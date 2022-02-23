

#ifndef NRF_SDH_ONEOS_H__
#define NRF_SDH_ONEOS_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*nrf_sdh_oneos_task_hook_t)(void *p_context);

/**@brief   Function for creating a task to retrieve SoftDevice events.
 * @param[in]   hook        Function to run in the SoftDevice OneOS task,
 *                          before entering the task loop.
 * @param[in]   p_context   Parameter for the function @p hook.
 */
void nrf_sdh_oneos_init(nrf_sdh_oneos_task_hook_t hook, void *p_context);

#ifdef __cplusplus
}
#endif

#endif /* NRF_SDH_ONEOS_H__ */
