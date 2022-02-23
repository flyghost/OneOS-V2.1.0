#ifndef __LPM_H_
#define __LPM_H_

os_err_t lpm_timer_start_once(os_uint32_t timeout_ms);

void lpm_enter_sleep(void);

os_err_t lpm_start(os_uint32_t timeout_ms);

#endif /* __LPM_H_ */

