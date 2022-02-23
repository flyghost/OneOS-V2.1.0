/**
  ******************************************************************************
  * @file    fm33g0xx_pmu.c
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   
  *
*/

/* Includes ------------------------------------------------------------------*/
#include "fm33g0xx_pmu.h" 

/** @addtogroup fm33g0xx_StdPeriph_Driver
  * @{
  */

/** @defgroup PMU 
  * @brief PMU driver modules
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup PMU_Private_Functions
  * @{
  */

/********************************
�͹�����FLASH DeepStandbyʹ�ܺ���
����:�͹�����FLASH DeepStandbyʹ��
���룺0��ʹ��
      1����ֹ
�������
 ********************************/
//void PMU_LPMCFG_FLSDPSEN_Setable(FunState NewState)
//{
//  if(NewState == ENABLE)
//  {
//    PMU->LPMCFG |= (PMU_LPMCFG_FLSDPSEN_EN);
//  }
//  else
//  {
//     PMU->LPMCFG &= (PMU_LPMCFG_FLSDPSEN_DIS);
//  }       
//}

/********************************
��FLSDPSEN ״̬����
����:��FLSDPSEN״̬
���룺��
�����1:ʹ��
      0����ֹ
 ********************************/
//FunState PMU_LPMCFG_FLSDPSEN_Getable(void)
//{
//  if (PMU->LPMCFG & (PMU_LPMCFG_FLSDPSEN_Msk))
//	{
//		return ENABLE;
//	}
//	else
//	{
//		return DISABLE;
//	}
//}
/********************************
�ر�XTLF����
����:�ر�XTLF������SLEEP/DEEPSLEEP��������
���룺�ر�XTLF��״̬
 ********************************/
void PMU_LPMCFG_XTOFF_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = PMU->LPMCFG;
	tmpreg &= ~(PMU_LPMCFG_XTOFF_Msk);
	tmpreg |= (SetValue & PMU_LPMCFG_XTOFF_Msk);
	PMU->LPMCFG = tmpreg;
}
/********************************
��ȡ�ر�XTLF��״̬����
����:��ȡ�ر�XTLF��״̬
������ر�XTLF��״̬
 ********************************/
uint32_t PMU_LPMCFG_XTOFF_Get(void)
{
	return (PMU->LPMCFG & PMU_LPMCFG_XTOFF_Msk);
}

/********************************
��ȡLDO15ʹ��״̬����
����:��ȡLDO15ʹ��״̬
�����LDO15ʹ��״̬
 ********************************/
FlagStatus PMU_LPMCFG_LDO15EN_Chk(void)
{
	if (PMU->LPMCFG & PMU_LPMCFG_LDO15EN_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/********************************
����DeepSleep���ƺ���
����:�����Ƿ����DeepSleep������Sleep����Ч
���룺�Ƿ����DeepSleep������
 ********************************/
void PMU_LPMCFG_SLPDP_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = PMU->LPMCFG;
	tmpreg &= ~(PMU_LPMCFG_SLPDP_Msk);
	tmpreg |= (SetValue & PMU_LPMCFG_SLPDP_Msk);
	PMU->LPMCFG = tmpreg;
}
/********************************
��ȡ�����DeepSleep�����ú���
����:���Ƿ����DeepSleep������
������Ƿ����DeepSleep������
 ********************************/
uint32_t PMU_LPMCFG_SLPDP_Get(void)
{
	return (PMU->LPMCFG & PMU_LPMCFG_SLPDP_Msk);
}
/********************************
�͹���ģʽ�ں˵�ѹ���ƺ���
����:���õ͹���ģʽ�ں˵�ѹ�Ŀ����رգ���λ����Sleep/DeepSleep��������
���룺�͹���ģʽ�ں˵�ѹ���ص�����
 ********************************/
void PMU_LPMCFG_CVS_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		PMU->LPMCFG |= (PMU_LPMCFG_CVS_Msk);
	}
	else
	{
		PMU->LPMCFG &= ~(PMU_LPMCFG_CVS_Msk);
	}
}
/********************************
��ȡ�Ե͹���ģʽ�ں˵�ѹ���ƺ���
����:�͹���ģʽ�ں˵�ѹ�Ŀ����رյ�����
������͹���ģʽ�ں˵�ѹ�Ŀ����ر�״̬
 ********************************/
FunState PMU_LPMCFG_CVS_Getable(void)
{
	if (PMU->LPMCFG & (PMU_LPMCFG_CVS_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
�͹���ģʽ���ú���
����:�Ե͹���ģʽ��������
���룺�͹���ģʽ������
 ********************************/
void PMU_LPMCFG_PMOD_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = PMU->LPMCFG;
	tmpreg &= ~(PMU_LPMCFG_PMOD_Msk);
	tmpreg |= (SetValue & PMU_LPMCFG_PMOD_Msk);
	PMU->LPMCFG = tmpreg;
}
/********************************
��ȡ�͹���ģʽ���ú���
����:��ȡ�͹���ģʽ������
������͹���ģʽ������
 ********************************/
uint32_t PMU_LPMCFG_PMOD_Get(void)
{
	return (PMU->LPMCFG & PMU_LPMCFG_PMOD_Msk);
}

/********************************
����ʱ����ƺ���
����:���ô�RTCBKP���ѵĿɱ����ʱ
���룺ʱ��������ֵ
 ********************************/
void PMU_WKDLYCON_T1a_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = PMU->WKDLYCON;
	tmpreg &= ~(PMU_WKDLYCON_T1a_Msk);
	tmpreg |= (SetValue & PMU_WKDLYCON_T1a_Msk);
	PMU->WKDLYCON = tmpreg;
}
/********************************
��ȡ����ʱ�亯��
����:��ȡ��RTCBKP���ѵĿɱ����ʱ������
�����ʱ��������ֵ
 ********************************/
uint32_t PMU_WKDLYCON_T1a_Get(void)
{
	return (PMU->WKDLYCON & PMU_WKDLYCON_T1a_Msk);
}

/********************************
���CPU Debugger���ѱ�־����
����:���CPU Debugger���ѱ�־
 ********************************/
void PMU_WKPFLAG_DBGWKF_Clr(void)
{
	PMU->WKPFLAG = PMU_WKPFLAG_DBGWKF_Msk;
}
/********************************
��ȡCPU Debugger���ѱ�־����
����:��ȡCPU Debugger���ѱ�־
�����CPU Debugger���ѱ�־
 ********************************/
FlagStatus PMU_WKPFLAG_DBGWKF_Chk(void)
{
	if (PMU->WKPFLAG & PMU_WKPFLAG_DBGWKF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/********************************
RTCBK�����ж�ʹ�ܺ���
����:����RTCBK�����ж�ʹ��
���룺RTCBK�����ж�ʹ�ܵ�����
 ********************************/
void PMU_LPREIE_RTCBKPEIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		PMU->LPREIE |= (PMU_LPREIE_RTCBKPEIE_Msk);
	}
	else
	{
		PMU->LPREIE &= ~(PMU_LPREIE_RTCBKPEIE_Msk);
	}
}
/********************************
��ȡRTCBK�����ж�ʹ�ܺ���
����:��ȡRTCBK�����ж�ʹ�ܵ�����
�����RTCBK�����ж�ʹ�ܵ�����
 ********************************/
FunState PMU_LPREIE_RTCBKPEIE_Getable(void)
{
	if (PMU->LPREIE & (PMU_LPREIE_RTCBKPEIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}
/********************************
SLEEP�����ж�ʹ�ܺ���
����:����SLEEP�����ж�ʹ��
���룺SLEEP�����ж�ʹ�ܵ�����
 ********************************/
void PMU_LPREIE_SLPEIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		PMU->LPREIE |= (PMU_LPREIE_SLPEIE_Msk);
	}
	else
	{
		PMU->LPREIE &= ~(PMU_LPREIE_SLPEIE_Msk);
	}
}
/********************************
��ȡSLEEP�����ж�ʹ�ܺ���
����:��ȡSLEEP�����ж�ʹ�ܵ�����
�����SLEEP�����ж�ʹ�ܵ�����
 ********************************/
FunState PMU_LPREIE_SLPEIE_Getable(void)
{
	if (PMU->LPREIE & (PMU_LPREIE_SLPEIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}


/********************************
LPRUN�����ж�ʹ�ܺ���
����:����LPRUN�����ж�ʹ��
���룺LPRUN�����ж�ʹ�ܵ�����
 ********************************/
void PMU_LPREIE_LPREIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		PMU->LPREIE |= (PMU_LPREIE_LPREIE_Msk);
	}
	else
	{
		PMU->LPREIE &= ~(PMU_LPREIE_LPREIE_Msk);
	}
}
/********************************
��ȡ LPRUN�����ж�ʹ�ܺ���
����:��ȡ LPRUN�����ж�ʹ�ܵ�����
����� LPRUN�����ж�ʹ�ܵ�����
 ********************************/
FunState PMU_LPREIE_LPREIE_Getable(void)
{
	if (PMU->LPREIE & (PMU_LPREIE_LPREIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
 ���RTCBK�����жϱ�־����
����:���RTCBK�����жϱ�־
********************************/
void PMU_LPREIF_RTCBKPEIF_Clr(void)
{
	PMU->LPREIF = PMU_LPREIF_RTCBKPEIF_Msk;
}
/********************************
��ȡRTCBK�����жϱ�־����
����:��ȡRTCBK�����жϱ�־������
����� RTCBK�����жϱ�־
 ********************************/
FlagStatus PMU_LPREIF_RTCBKPEIF_Chk(void)
{
	if (PMU->LPREIF & PMU_LPREIF_RTCBKPEIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/* SLEEP�����жϱ�־��Ӳ����λ�����д1����,��PMOD=2��h2��CPUִ��WFI/WFEָ��ǰ��λ��SLEEPDEEP�Ĵ���ʱ��λ ��غ��� */
/********************************
 ���SLEEP�����жϱ�־����
����:���SLEEP�����жϱ�־
********************************/
void PMU_LPREIF_SLPEIF_Clr(void)
{
	PMU->LPREIF = PMU_LPREIF_SLPEIF_Msk;
}
/********************************
��ȡSLEEP�����жϱ�־����
����:��ȡSLEEP�����жϱ�־������
����� SLEEP�����жϱ�־
 ********************************/
FlagStatus PMU_LPREIF_SLPEIF_Chk(void)
{
	if (PMU->LPREIF & PMU_LPREIF_SLPEIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/* LPRUN�����жϱ�־��Ӳ����λ�����д1���㣻�������LPRUNģʽʱ���������LPREIF����оƬ�Խ�ͣ����ACTIVEģʽ,LPRUN Condition Error��������LPRUNʱ�������������
1�� HSCLKѡ����LSCLK��RCLP����
2�� RCHF��PLL��ADCʹ��δ�ر�
 ��غ��� */
void PMU_LPREIF_LPREIF_Clr(void)
{
	PMU->LPREIF = PMU_LPREIF_LPREIF_Msk;
}

FlagStatus PMU_LPREIF_LPREIF_Chk(void)
{
	if (PMU->LPREIF & PMU_LPREIF_LPREIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}


void PMU_Deinit(void)
{
	PMU->LPMCFG = 0x00060000;
	PMU->WKDLYCON = 0x00000001;
	//PMU->WKPFLAG = 0x00000000;
	PMU->LPREIE = 0x00000000;
	//PMU->LPREIF = 0x00000000;
}
//Code_End		

///* WKUP����ʹ��PIN�궨�� */
//��GPIO.h��
//#define		PINWKEN_PD6		BIT7
//#define		PINWKEN_PE9		BIT6
//#define		PINWKEN_PE2		BIT5
//#define		PINWKEN_PA13	BIT4
//#define		PINWKEN_PG7		BIT3
//#define		PINWKEN_PC13	BIT2
//#define		PINWKEN_PB0		BIT1
//#define		PINWKEN_PF5		BIT0
/********************************
���NWKUPx Pin���ѱ�־����
����:���NWKUPx Pin���ѱ�־����Sleep/DeepSleepģʽ����Ч��
���룺����NWKUPx Pin���ѱ�־������ֵ��д1.
 ********************************/
void PMU_WKPFLAG_WKPxF_ClrEx(uint32_t NWKPinDef)
{
	PMU->WKPFLAG = NWKPinDef;
}
/********************************
��ȡNWKUPx Pin���ѱ�־����
����:��ȡNWKUPx Pin���ѱ�־��
�����NWKUPx Pin���ѱ�־��ֵ
 ********************************/
FlagStatus PMU_WKPFLAG_WKPxF_ChkEx(uint32_t NWKPinDef)
{
	if (PMU->WKPFLAG & NWKPinDef)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/********************************
�͹���ģʽ�ĳ�ʼ������
����:�͹���ģʽ�¸������ã�SLEEPģʽ��RAM�������ã��ں˵�ѹ�������·�ȡ�
���룺�͹��Ĺ��ܿ���ǰ������.
 ********************************/
void PMU_SleepCfg_Init(PMU_SleepCfg_InitTypeDef* SleepCfg_InitStruct)
{
	PMU_LPMCFG_SLPDP_Set(SleepCfg_InitStruct->SLPDP);				/*!<DeepSleep���ƼĴ���  */	
	PMU_LPMCFG_CVS_Setable(SleepCfg_InitStruct->CVS);				/*!<�ں˵�ѹ����ʹ�ܿ���  */	
	PMU_LPMCFG_XTOFF_Set(SleepCfg_InitStruct->XTOFF);				/*!<�ر�XTLF������SLEEP/DEEPSLEEP��������  */
	SCB->SCR = SleepCfg_InitStruct->SCR;							/*!<M0ϵͳ���ƼĴ�����һ������Ϊ0����  */
	PMU_LPMCFG_PMOD_Set(SleepCfg_InitStruct->PMOD);					/*!<�͹���ģʽ����  */	
}
