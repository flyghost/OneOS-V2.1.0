#include "usr_lpower.h"
#include <string.h>
void Enter_Lpower(int mode) {
	//extern void mp_hal_stdout_tx_strn(const char *str, size_t len);
	//extern void HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry);
	//mp_hal_stdout_tx_strn("Enter_Lpower", strlen("Enter_Lpower"));

	//睡眠模式，内核停止，外设继续保持运行
	//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

	//停止模式，所有外设时钟关闭
	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	//待机模式，只有备份域中的RTC继续工作，可通过RTC闹钟、复位、看门狗复位唤醒
	//HAL_PWR_EnterSTANDBYMode
/* 	switch(mode){
		case SLEEP_MODE:
			HAL_SuspendTick();
			//UartRx_EXTIConfig();
			HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			HAL_ResumeTick();
			//内核停止，次数不会打印，当任意中断触发唤醒后此处打印
			mp_hal_stdout_tx_strn("Exit_Lpower\n", strlen("Exit_Lpower\n"));
			break;
		case STOP_MODE:
			//extern void UartRx_EXTIConfig(void);
			HAL_SuspendTick();

			UartRx_EXTIConfig();
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
			HAL_ResumeTick();
			break;
		case STAND_MODE:
			HAL_PWR_EnterSTANDBYMode();
			break;
	}
 */
}

//退出低功耗模式,调用此函数进行外设初始化
void Exit_Lpower(void){
/* 	mp_hal_stdout_tx_strn("Exit_Lpower\n", strlen("Exit_Lpower\n"));
	//重新初始化外设
	extern void InitPeripheral(void);
	InitPeripheral();
 */
}

