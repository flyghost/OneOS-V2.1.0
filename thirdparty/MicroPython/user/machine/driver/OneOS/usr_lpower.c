#include "usr_lpower.h"
#include <string.h>
void Enter_Lpower(int mode) {
	//extern void mp_hal_stdout_tx_strn(const char *str, size_t len);
	//extern void HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry);
	//mp_hal_stdout_tx_strn("Enter_Lpower", strlen("Enter_Lpower"));

	//˯��ģʽ���ں�ֹͣ�����������������
	//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

	//ֹͣģʽ����������ʱ�ӹر�
	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	//����ģʽ��ֻ�б������е�RTC������������ͨ��RTC���ӡ���λ�����Ź���λ����
	//HAL_PWR_EnterSTANDBYMode
/* 	switch(mode){
		case SLEEP_MODE:
			HAL_SuspendTick();
			//UartRx_EXTIConfig();
			HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			HAL_ResumeTick();
			//�ں�ֹͣ�����������ӡ���������жϴ������Ѻ�˴���ӡ
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

//�˳��͹���ģʽ,���ô˺������������ʼ��
void Exit_Lpower(void){
/* 	mp_hal_stdout_tx_strn("Exit_Lpower\n", strlen("Exit_Lpower\n"));
	//���³�ʼ������
	extern void InitPeripheral(void);
	InitPeripheral();
 */
}

