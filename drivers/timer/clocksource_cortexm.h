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
 * @file        clocksource_cortexm.h
 *
 * @brief       This file provides functions for cputime_cortexm init.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef CORTEXM_CLOCKSOURCE_H_
#define CORTEXM_CLOCKSOURCE_H_

void cortexm_dwt_init(void);
void cortexm_systick_clocksource_init(void);
void cortexm_systick_clockevent_init(void);
void cortexm_systick_clockevent_isr(void);

#endif /* CORTEXM_CLOCKSOURCE_H_ */

