/**
  ******************************************************************************
  * @file    fm33g0xx_scu.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the SCU & DCU firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_SCU_H
#define __FM33G0XX_SCU_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33g0xx_StdPeriph_Driver
  * @{
  */

/** @addtogroup SCU
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
	 


#define	SCU_MCUDBGCR_DBG_ET4_STOP_Pos	13	/* Debug״̬��ET4ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_ET4_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_ET4_STOP_Pos)
	/* 0��Debugʱ����ET4ԭ��״̬ */
	/* 1��Debugʱ�ر�ET4 */

#define	SCU_MCUDBGCR_DBG_ET3_STOP_Pos	12	/* Debug״̬��ET3ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_ET3_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_ET3_STOP_Pos)
	/* 0��Debugʱ����ET3ԭ��״̬ */
	/* 1��Debugʱ�ر�ET3 */

#define	SCU_MCUDBGCR_DBG_ET2_STOP_Pos	11	/* Debug״̬��ET2ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_ET2_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_ET2_STOP_Pos)
	/* 0��Debugʱ����ET2ԭ��״̬ */
	/* 1��Debugʱ�ر�ET2 */

#define	SCU_MCUDBGCR_DBG_ET1_STOP_Pos	10	/* Debug״̬��ET1ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_ET1_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_ET1_STOP_Pos)
	/* 0��Debugʱ����ET1ԭ��״̬ */
	/* 1��Debugʱ�ر�ET1 */

#define	SCU_MCUDBGCR_DBG_BT2_STOP_Pos	9	/* Debug״̬��BT2ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_BT2_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_BT2_STOP_Pos)
	/* 0��Debugʱ����BT2ԭ��״̬ */
	/* 1��Debugʱ�ر�BT2 */

#define	SCU_MCUDBGCR_DBG_BT1_STOP_Pos	8	/* Debug״̬��BT1ʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_BT1_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_BT1_STOP_Pos)
	/* 0��Debugʱ����BT1ԭ��״̬ */
	/* 1��Debugʱ�ر�BT1 */

#define	SCU_MCUDBGCR_DBG_WWDT_STOP_Pos	1	/* Debug״̬��WWDTʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_WWDT_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_WWDT_STOP_Pos)
	/* 0��Debugʱ����WWDTԭ��״̬ */
	/* 1��Debugʱ�ر�WWDT */

#define	SCU_MCUDBGCR_DBG_IWDT_STOP_Pos	0	/* Debug״̬��IWDTʹ�ܿ��� */
#define	SCU_MCUDBGCR_DBG_IWDT_STOP_Msk	(0x1U << SCU_MCUDBGCR_DBG_IWDT_STOP_Pos)
	/* 0��Debugʱ����IWDT���� */
	/* 1��Debugʱ�ر�IWDT */

#define	SCU_HDFFLAG_DABORT_ADDR_FLAG_Pos	6	/* ��ַ�Ƕ�����ʴ����־ */
#define	SCU_HDFFLAG_DABORT_ADDR_FLAG_Msk	(0x1U << SCU_HDFFLAG_DABORT_ADDR_FLAG_Pos)

#define	SCU_HDFFLAG_DABORT_RESP_FLAG_Pos	5	/* �Ƿ���ַ���ʴ����־ */
#define	SCU_HDFFLAG_DABORT_RESP_FLAG_Msk	(0x1U << SCU_HDFFLAG_DABORT_RESP_FLAG_Pos)

#define	SCU_HDFFLAG_SVCUNDEF_FLAG_Pos	4	/* SVC instructionsδ�����־ */
#define	SCU_HDFFLAG_SVCUNDEF_FLAG_Msk	(0x1U << SCU_HDFFLAG_SVCUNDEF_FLAG_Pos)

#define	SCU_HDFFLAG_BKPT_FLAG_Pos	3	/* ִ��BKPTָ���־ */
#define	SCU_HDFFLAG_BKPT_FLAG_Msk	(0x1U << SCU_HDFFLAG_BKPT_FLAG_Pos)

#define	SCU_HDFFLAG_TBIT_FLAG_Pos	2	/* Thumb-State��־ */
#define	SCU_HDFFLAG_TBIT_FLAG_Msk	(0x1U << SCU_HDFFLAG_TBIT_FLAG_Pos)

#define	SCU_HDFFLAG_SPECIAL_OP_FLAG_Pos	1	/* ����ָ���־ */
#define	SCU_HDFFLAG_SPECIAL_OP_FLAG_Msk	(0x1U << SCU_HDFFLAG_SPECIAL_OP_FLAG_Pos)

#define	SCU_HDFFLAG_HDF_REQUEST_FLAG_Pos	0	/* hardfault��־λ */
#define	SCU_HDFFLAG_HDF_REQUEST_FLAG_Msk	(0x1U << SCU_HDFFLAG_HDF_REQUEST_FLAG_Pos)
//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void SCU_Deinit(void);

/* Debug״̬��ET4ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_ET4_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_ET4_STOP_Getable(void);

/* Debug״̬��ET3ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_ET3_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_ET3_STOP_Getable(void);

/* Debug״̬��ET2ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_ET2_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_ET2_STOP_Getable(void);

/* Debug״̬��ET1ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_ET1_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_ET1_STOP_Getable(void);

/* Debug״̬��BT2ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_BT2_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_BT2_STOP_Getable(void);

/* Debug״̬��BT1ʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_BT1_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_BT1_STOP_Getable(void);

/* Debug״̬��WWDTʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_WWDT_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_WWDT_STOP_Getable(void);

/* Debug״̬��IWDTʹ�ܿ��� ��غ��� */
extern void SCU_MCUDBGCR_DBG_IWDT_STOP_Setable(FunState NewState);
extern FunState SCU_MCUDBGCR_DBG_IWDT_STOP_Getable(void);

/* ��ַ�Ƕ�����ʴ����־ ��غ��� */
extern void SCU_HDFFLAG_DABORT_ADDR_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_DABORT_ADDR_FLAG_Chk(void);

/* �Ƿ���ַ���ʴ����־ ��غ��� */
extern void SCU_HDFFLAG_DABORT_RESP_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_DABORT_RESP_FLAG_Chk(void);

/* SVC instructionsδ�����־ ��غ��� */
extern void SCU_HDFFLAG_SVCUNDEF_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_SVCUNDEF_FLAG_Chk(void);

/* ִ��BKPTָ���־ ��غ��� */
extern void SCU_HDFFLAG_BKPT_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_BKPT_FLAG_Chk(void);

/* Thumb-State��־ ��غ��� */
extern void SCU_HDFFLAG_TBIT_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_TBIT_FLAG_Chk(void);

/* ����ָ���־ ��غ��� */
extern void SCU_HDFFLAG_SPECIAL_OP_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_SPECIAL_OP_FLAG_Chk(void);

/* hardfault��־λ ��غ��� */
extern void SCU_HDFFLAG_HDF_REQUEST_FLAG_Clr(void);
extern FlagStatus SCU_HDFFLAG_HDF_REQUEST_FLAG_Chk(void);
//Announce_End



#ifdef __cplusplus
}
#endif

#endif 



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



