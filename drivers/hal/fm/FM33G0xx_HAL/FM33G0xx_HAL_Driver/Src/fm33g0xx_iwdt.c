/**
  ******************************************************************************
  * @file    fm33g0xx_iwdt.c
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of....:
  *
*/

/* Includes ------------------------------------------------------------------*/
#include "FM33g0XX_iwdt.h"  

/** @addtogroup fm33g0xx_StdPeriph_Driver
  * @{
  */

/** @defgroup IWDT 
  * @brief IWDT driver modules
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup IWDT_Private_Functions
  * @{
  */ 

/********************************
IWDT�������
����:���IWDT
���룺д��0x1234_5A5Aʱ�幷
 ********************************/
void IWDT_IWDTSERV_Write(uint32_t SetValue)
{
	IWDT->IWDTSERV = (SetValue);
}

/********************************
����IWDT�������4096S�ĺ���
����:����IWDT�������Ϊ4096S
���룺�Ƿ�Ҫ����4096S���߼�ֵ
 ********************************/
void IWDT_IWDTCFG_IWDTSLP4096S_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		IWDT->IWDTCFG |= (IWDT_IWDTCFG_IWDTSLP4096S_Msk);
	}
	else
	{
		IWDT->IWDTCFG &= ~(IWDT_IWDTCFG_IWDTSLP4096S_Msk);
	}
}
/********************************
��ȡIWDT������ں���
����:��ȡIWDT��������Ƿ���4096S
�����4096S���߼�ֵ������
 ********************************/
FunState IWDT_IWDTCFG_IWDTSLP4096S_Getable(void)
{
	if (IWDT->IWDTCFG & (IWDT_IWDTCFG_IWDTSLP4096S_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/********************************
��ȡIWDT����������ú���
����:��ȡIWDT������ڵ�����
�����IWDT������ڵ�����ֵ
 ********************************/
void IWDT_IWDTCFG_IWDTOVP_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = IWDT->IWDTCFG;
	tmpreg &= ~(IWDT_IWDTCFG_IWDTOVP_Msk);
	tmpreg |= (SetValue & IWDT_IWDTCFG_IWDTOVP_Msk);
	IWDT->IWDTCFG = tmpreg;
}
/********************************
��ȡIWDT����������ú���
����:��ȡIWDT������ڵ�����
�����IWDT������ڵ�����ֵ
 ********************************/
uint32_t IWDT_IWDTCFG_IWDTOVP_Get(void)
{
	return (IWDT->IWDTCFG & IWDT_IWDTCFG_IWDTOVP_Msk);
}

/********************************
��ȡIWDT��ǰ����ֵ����
����:��ȡIWDT��ǰ����ֵ
�����IWDT��ǰ����ֵ
 ********************************/
uint32_t IWDT_IWDTCNT_Read(void)
{
	return (IWDT->IWDTCNT & IWDT_IWDTCNT_IWDTCNT_Msk);
}

/********************************
��ȡIWDT��ǰ����ֵ����
����:��ȡIWDT��ǰ����ֵ
�����IWDT��ǰ����ֵ
 ********************************/
void IWDT_Deinit(void)
{
	//IWDT->IWDTSERV = ;
	IWDT->IWDTCFG = 0x00000001;
	//IWDT->IWDTCNT = ;
}
//Code_End

/********************************
�幷����
����:�幷
********************************/
void IWDT_Clr(void)
{
    IWDT->IWDTSERV = WDTSERV_key;
//	IWDT_IWDTSERV_Write(WDTSERV_key);
}



/******END OF FILE****/



