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
 * @file        target_util.c
 *
 * @brief       Adpt oneos delay functions for beken driver.
 *
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-18   OneOS Team      First version
 ***********************************************************************************************************************
 */
#include "include.h"
#include "arm_arch.h"

#include "target_util_pub.h"
#include <os_task.h>


extern os_err_t os_task_delay(os_tick_t tick);
extern os_tick_t os_tick_from_ms(os_uint32_t ms);



/*******************************************************************************
* Function Implemantation
*******************************************************************************/
/*
	MCLK:26MHz, delay(1): about 25us
				delay(10):about 125us
				delay(100):about 850us
 */
void delay(INT32 num)
{
    volatile INT32 i,j;
	
    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 100; j ++)
			;
    }
}

/*
	when parameter is 1, the return result is approximately 1 ms;
 */
void delay_ms(UINT32 ms_count)
{
   os_task_delay(os_tick_from_ms(ms_count));
}

/*
	[delay offset]worst case: delay about 1 second;
 */
void delay_sec(UINT32 ms_count)
{
    os_task_delay(os_tick_from_ms(ms_count * 1000));
}

/*
	[delay offset]worst case: delay about 1 tick;
 */
void delay_tick(UINT32 tick_count)
{
    os_task_delay(tick_count);	
}

// EOF
