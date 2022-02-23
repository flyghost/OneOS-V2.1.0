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
 * @file        drv_lpmgr.c
 *
 * @brief       This file implements low power manager for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <os_clock.h>
#include <timer/clockevent.h>
#include <sys/time.h>
#include <string.h>
#include <os_memory.h>


#define DBG_EXT_TAG "drv.lpmgr"
#define DBG_EXT_LVL DBG_EXT_INFO


#define OS_NS_TO_TICK(ns) (OS_TICK_PER_SECOND * ns / (1000 * 1000 * 1000))
#define LPMGR_MIN_SLEEPTICK     (2)

os_clockevent_t *lp_ce = NULL;

typedef struct
{
    os_uint32_t sys_load;
    os_uint32_t sys_ctrl;
}systick_val_sava_s;

static systick_val_sava_s sys_save;
void cortexm_systick_tick_deinit(void)
{
    sys_save.sys_load = SysTick->LOAD;
    sys_save.sys_ctrl = SysTick->CTRL;    
    SysTick->CTRL &= ~(1 << 0);
}

void cortexm_systick_tick_init(void)
{
    SysTick->LOAD = sys_save.sys_load;
    SysTick->VAL   = 0;
    SysTick->CTRL = sys_save.sys_ctrl;
}

//����ʱSWD�������������գ����ܻ��Ӧ��ƽ����IO����Ĵ�����ת���𹦺ı��
//���SWD�����ⲿ�����Ļ�������ǰ�ɿ�������ʹ��
void SWD_IO_PullUp(FunState NewState)
{
	if( DISABLE == NewState )//�ر�SWDTLK.SWDTDO����ʹ��
	{		
		AltFunIO( GPIOG, GPIO_Pin_8, ALTFUN_NORMAL );	//PG8;//SWDTCK
		AltFunIO( GPIOG, GPIO_Pin_9, ALTFUN_NORMAL );	//PG9;//SWDTDO	
	}
	else//��SWDTCK,SWDTDO����ʹ��
	{
		AltFunIO( GPIOG, GPIO_Pin_8, ALTFUN_PULLUP );	//PG8;//SWDTCK
		AltFunIO( GPIOG, GPIO_Pin_9, ALTFUN_PULLUP );	//PG9;//SWDTDO	
	}
}

void fm_MF_PMU_Init(void)
{
    SWD_IO_PullUp(ENABLE);
    IWDT_IWDTCFG_IWDTSLP4096S_Setable(ENABLE);	//��������ʱ�Ƿ�����4096s������
    IWDT_Clr();             //��ϵͳ���Ź�	
}

void fm_Sleep(os_uint32_t mode)
{   
    PMU_SleepCfg_InitTypeDef SleepCfg_InitStruct;

    /*�µ縴λ����*/
    //pdr��bor�����µ縴λ����Ҫ��һ��
    //����Դ��ѹ�����µ縴λʱ��оƬ�ᱻ��λס	
    //pdr��ѹ��λ��׼���ǹ��ļ���(�����޲�����
    //bor��ѹ��λ׼ȷ������Ҫ����2uA����
    ANAC_PDRCON_PDREN_Setable(ENABLE);		//��PDR
    ANAC_PDRCON_PDRCFG_Set(ANAC_PDRCON_PDRCFG_1P5V);//pdr��ѹ������1.5V
    ANAC_BORCON_OFF_BOR_Setable(ENABLE);	//�ر�BOR 3uA
    RCC_SYSCLKSEL_LPM_RCLP_OFF_Setable(ENABLE);	//�ر�rclp 0.2uA

    SleepCfg_InitStruct.PMOD = PMU_LPMCFG_PMOD_SLEEP;			//����ģʽ����
    SleepCfg_InitStruct.SLPDP = mode;			//Sleep����
    SleepCfg_InitStruct.CVS = DISABLE;							//�ں˵�ѹ���Ϳ���
    SleepCfg_InitStruct.XTOFF = PMU_LPMCFG_XTOFF_DIS;			//����XTLF
    SleepCfg_InitStruct.SCR = 0;								//M0ϵͳ���ƼĴ�����һ������Ϊ0����	

    PMU_SleepCfg_Init(&SleepCfg_InitStruct);//��������

    IWDT_Clr();	
    __WFI();//��������
    IWDT_Clr();	
}

/**
 ***********************************************************************************************************************
 * @brief           Put device into sleep mode.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       mode            Low power mode.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static os_err_t lpm_sleep(lpmgr_sleep_mode_e mode)
{
    switch (mode)
    {
    case SYS_SLEEP_MODE_NONE:
        break;

    case SYS_SLEEP_MODE_IDLE:
        // __WFI();
        break;

    case SYS_SLEEP_MODE_LIGHT:
//        cortexm_systick_tick_deinit();
        
        fm_MF_PMU_Init();  /* sleep mode */
        fm_Sleep(PMU_LPMCFG_SLPDP_SLEEP);
        
//        cortexm_systick_tick_init();
        break;

    case SYS_SLEEP_MODE_DEEP:
//        cortexm_systick_tick_deinit();
        
        fm_MF_PMU_Init();    /* DeepSleep mode */
        fm_Sleep(PMU_LPMCFG_SLPDP_DEEPSLEEP);
                
//        cortexm_systick_tick_init();
        break;

    case SYS_SLEEP_MODE_STANDBY:
    case SYS_SLEEP_MODE_SHUTDOWN:
        break;

    default:
        OS_ASSERT(0);
    }
        
    return OS_EOK;
}


/**
 ***********************************************************************************************************************
 * @brief           Caculate the PM tick from OS tick.
 *
 * @param[in]       tick            OS tick.
 *
 * @return          PM tick.
 ***********************************************************************************************************************
 */
static os_tick_t stm32l4_lpm_tick_from_os_tick(os_tick_t tick, os_uint32_t lpm_freq)
{
    return (lpm_freq * tick / OS_TICK_PER_SECOND);
}

/**
 ***********************************************************************************************************************
 * @brief           Caculate the OS tick from PM tick.
 *
 * @param[in]       tick            PM tick.
 *
 * @return          OS tick.
 ***********************************************************************************************************************
 */
static os_tick_t stm32l4_os_tick_from_lpm_tick(os_tick_t tick, os_uint32_t lpm_freq)
{
    static os_uint32_t os_tick_remain = 0;
    os_uint32_t ret;

    ret  = (tick * OS_TICK_PER_SECOND + os_tick_remain) / lpm_freq;

    /* Tick compensation, lpm tick is converted to os tick, there will be a residual value, 
   * which will be added to the next tick calculation 
   */
    os_tick_remain += (tick * OS_TICK_PER_SECOND);
    os_tick_remain %= lpm_freq;

    return ret;
}


/**
 ***********************************************************************************************************************
 * @brief           Start the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       timeout         How many OS ticks that MCU can sleep.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_start(struct os_lpmgr_dev *lpm, os_tick_t time_tick)
{
    OS_ASSERT(lpm != OS_NULL);
    OS_ASSERT(time_tick > 0);
    os_uint64_t nsec;

    if (time_tick != OS_TICK_MAX)
    {
        /* Convert OS Tick to pmtimer timeout value */
        time_tick = stm32l4_lpm_tick_from_os_tick(time_tick, lp_ce->freq);
        if (time_tick > lp_ce->mask)
        {
            time_tick = lp_ce->mask;
        }

        /* Enter LPM_TIMER_MODE */
        nsec = NSEC_PER_SEC * time_tick / lp_ce->freq;
        os_clockevent_start_oneshot(lp_ce, nsec);
    }

}


static void lpm_timer_stop(void)
{
    /* Reset pmtimer status */
    os_clockevent_stop(lp_ce);
}



/**
 ***********************************************************************************************************************
 * @brief           Calculate how many OS ticks that MCU has suspended.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          OS ticks.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_timer_get_tick(void)
{
    os_uint32_t timer_tick;

    timer_tick = os_clockevent_read(lp_ce);

    return stm32l4_os_tick_from_lpm_tick(timer_tick, lp_ce->freq);
}

static const struct os_lpmgr_ops lpmgr_ops = {
    lpm_sleep,
    lpm_timer_start,
    lpm_timer_stop,
    lpm_timer_get_tick
};

/**
***********************************************************************************************************************
* @brief           Initialise low power manager.
*
* @param[in]		None.
*
* @return          0.
***********************************************************************************************************************
*/
int drv_lpmgr_hw_init(void)
{
    os_uint8_t sleep_mask = 0;
    struct os_lpmgr_dev  *lpmgr;
    os_tick_t lp_min, lp_max, sleep_min;

    lpmgr = (struct os_lpmgr_dev  *)os_calloc(1, sizeof(struct os_lpmgr_dev));
    OS_ASSERT(lpmgr != NULL);
    memset(lpmgr, 0, sizeof(struct os_lpmgr_dev));
    
    lp_ce = (os_clockevent_t *)os_device_find("lptim");
    OS_ASSERT(lp_ce != NULL);
    
    lpmgr->ops = &lpmgr_ops;
    
    lp_min = OS_NS_TO_TICK(lp_ce->min_nsec);
    lp_max = OS_NS_TO_TICK(lp_ce->max_nsec);

    sleep_min = os_tick_from_ms(BSP_USING_MINSLEEP_MS);
        
    if (lp_min < sleep_min)
    {
        lp_min = sleep_min;
    }
    
    lpmgr->min_tick = lp_min;
    lpmgr->max_tick = lp_max;

    /* Initialize timer mask */
    sleep_mask = (1UL << SYS_SLEEP_MODE_DEEP) | (1UL << SYS_SLEEP_MODE_LIGHT);

    /* Initialize system lpmgr module */
    os_lpmgr_register(lpmgr, sleep_mask, OS_NULL);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init, OS_INIT_SUBLEVEL_MIDDLE);


