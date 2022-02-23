/**
  ******************************************************************************
  * @file    fm33g0xx_lpuart.c
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Universal synchronous asynchronous receiver
  *          transmitter (UART):           
  *           
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "fm33g0xx_lpuart.h" 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*********************************
LPUART �������ݺ���
���ܣ���LPUART���ռĴ���
����: ��    
���: ���յ�����
*********************************/
uint32_t LPUART_LPURXD_Read(void)
{
	return (LPUART->LPURXD);
}

/*********************************
LPUART �������ݺ���
���ܣ�дLPUART���ͼĴ���
����: ���͵�����
���: ��
*********************************/
void LPUART_LPUTXD_Write(uint32_t SetValue)
{
	LPUART->LPUTXD = SetValue;
}


/*********************************
LPUART ������ɱ�־��⺯��
���ܣ���ⷢ����ɱ�־�Ƿ���λ������������λ��������ʹ��TC��Ϊ������ɼ���־��
����: ��
���: 0��1
*********************************/
FlagStatus LPUART_LPUSTA_TC_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_TC_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART ����buff�ձ�־λ��⺯��
���ܣ���ⷢ��buff�Ƿ�Ϊ��
����: ��
���:  0: ������
       1:��
*********************************/
FlagStatus LPUART_LPUSTA_TXE_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_TXE_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART ��ʼλ��0����
���ܣ�����־λ�� 0
����: ��
���:  ��
*********************************/
void LPUART_LPUSTA_START_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_START_Msk;
}

/*********************************
LPUART ��ʼλ��⺯��
���ܣ�����Ƿ���յ���ʼλ
����: ��
���:  0: û��⵽
       1:  ��⵽��ʼλ
*********************************/
FlagStatus LPUART_LPUSTA_START_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_START_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART У��λ�����־��0����
���ܣ���У��λ������0
����: ��
���:  ��
*********************************/
void LPUART_LPUSTA_PERR_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_PERR_Msk;
}

/*********************************
LPUART  У��λ�����⺯��
���ܣ����У����־�Ƿ���λ
����: ��
���:  0: ûУ���
       1:  ��У���
*********************************/
FlagStatus LPUART_LPUSTA_PERR_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_PERR_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ֡��ʽ�����־��0����
���ܣ���֡��ʽ������0
����: ��
���:  ��
*********************************/
void LPUART_LPUSTA_FERR_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_FERR_Msk;
}

/*********************************
LPUART  ֡��ʽ�����־λ��⺯��
���ܣ����֡��ʽ�����־�Ƿ���λ
����: ��
���:  0: û��֡��ʽ��
       1:  ��֡��ʽ��
*********************************/
FlagStatus LPUART_LPUSTA_FERR_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_FERR_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ���ջ��������־λ��0����
���ܣ������ջ������������0
����: ��
���:  ��
*********************************/
void LPUART_LPUSTA_RXOV_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_RXOV_Msk;
}

/*********************************
LPUART  ���ջ��������־λ��⺯��
����: �����ջ��������־λ�Ƿ���λ
����: ��
���:  0: û�н��ջ������
       1:  �н��ջ������
*********************************/
FlagStatus LPUART_LPUSTA_RXOV_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_RXOV_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ���ջ��������־λ��0����
����:  ���ջ��������־��0
����: ��
���:  ��
*********************************/

void LPUART_LPUSTA_RXF_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_RXF_Msk;
}

/*********************************
LPUART  ���ջ�������־λ��⺯��
����: �����ջ�������־λ�Ƿ���λ
����: ��
���:  0: ���ջ���û��
       1:  ���ջ�����
*********************************/
FlagStatus LPUART_LPUSTA_RXF_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_RXF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART ��������ƥ���־��0����
����: ��
���:  ��
*********************************/
void LPUART_LPUSTA_MATCH_Clr(void)
{
	LPUART->LPUSTA = LPUART_LPUSTA_MATCH_Msk;
}

/*********************************
LPUART ��������ƥ���־λ��⺯��
����: ��
���:  1��ƥ��
       0����ƥ��
*********************************/
FlagStatus LPUART_LPUSTA_MATCH_Chk(void)
{
	if (LPUART->LPUSTA & LPUART_LPUSTA_MATCH_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART ���ݷ��ͼ���ȡ��ʹ�ܺ���
����: 0����ȡ����Ĭ�ϣ�
      1��ȡ��
���:  ��
*********************************/
void LPUART_LPUCON_TXPOL_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TXPOL_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TXPOL_Msk);
	}
}

/*********************************
LPUART ���ݷ��ͼ���ȡ��״̬��⺯��
����: ��
���:  0����ȡ��
       1��ȡ��
*********************************/
FunState LPUART_LPUCON_TXPOL_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TXPOL_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART ��������ж�ʹ�ܺ���
����:   0����ֹ
        1��ʹ��
���:   ��
*********************************/
void LPUART_LPUCON_TCIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TCIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TCIE_Msk);
	}
}

/*********************************
LPUART ��������ж�ʹ��״̬��ȡ����
����:   ��
���:   0:��ֹ
        1��ʹ��
*********************************/
FunState LPUART_LPUCON_TCIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TCIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  ����buffer���ж�ʹ�ܺ���
����:   0����ֹ
        1��ʹ��
���:   ��
*********************************/
void LPUART_LPUCON_TXIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_TXIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_TXIE_Msk);
	}
}

/*********************************
LPUART  ����buffer���ж�ʹ��״̬��ȡ����
����:   ��
���:   0����ֹ
        1��ʹ��
*********************************/
FunState LPUART_LPUCON_TXIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_TXIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  �½��ز���ʹ��32K����
����:   0����ֹ
        1��ʹ��
���:   ��
*********************************/
void LPUART_LPUCON_NEDET_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_NEDET_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_NEDET_Msk);
	}
}
/*********************************
LPUART  �½��ز���ʹ��32K״̬��ȡ����
����:  ��
���:   0����ֹ
        1��ʹ��
*********************************/
FunState LPUART_LPUCON_NEDET_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_NEDET_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  ��������֡������żУ��λ����
����:   0��û��
        1����
���:   ��
*********************************/
void LPUART_LPUCON_PAREN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_PAREN_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_PAREN_Msk);
	}
}

/*********************************
LPUART  ����֡������żУ��״̬��ȡ
����:   ��
���:   0������żУ��
        1������żУ��
*********************************/
FunState LPUART_LPUCON_PAREN_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_PAREN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  ��������֡��żУ�麯��
����:   LPUART_LPUCON_PTYP_EVEN��żУ��
        LPUART_LPUCON_PTYP_ODD����У��
���:   ��
*********************************/
void LPUART_LPUCON_PTYP_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_PTYP_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART  ����֡��żУ��״̬��ȡ
����:   ��
���:   LPUART_LPUCON_PTYP_EVEN��żУ��
        LPUART_LPUCON_PTYP_ODD����У��
*********************************/
uint32_t LPUART_LPUCON_PTYP_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_PTYP_Msk);
}

/*********************************
LPUART   ֹͣλ�������ú���
����:    LPUART_LPUCON_SL_1BIT��1bit
         LPUART_LPUCON_SL_2BIT: 2bit
���:   ��
*********************************/
void LPUART_LPUCON_SL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_SL_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART   ֹͣλ��������״̬��ȡ����
����:    ��
���:    LPUART_LPUCON_SL_1BIT��1bit
         LPUART_LPUCON_SL_2BIT: 2bit
*********************************/
uint32_t LPUART_LPUCON_SL_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_SL_Msk);
}

/*********************************
LPUART   ���ݳ������ú���
����:    LPUART_LPUCON_DL_8BIT��8bit
         LPUART_LPUCON_DL_7BIT: 7bit
���:    ��
*********************************/
void LPUART_LPUCON_DL_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_DL_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART   ���ݳ�������״̬��ȡ����
����:    ��
���:    LPUART_LPUCON_DL_8BIT��8bit
         LPUART_LPUCON_DL_7BIT: 7bit
*********************************/
uint32_t LPUART_LPUCON_DL_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_DL_Msk);
}

/*********************************
LPUART   ���ռ���ȡ�����ú���
����:    0����ȡ��
         1��ȡ��
���:    ��
*********************************/
void LPUART_LPUCON_RXPOL_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_RXPOL_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_RXPOL_Msk);
	}
}

/*********************************
LPUART   ���ռ���ȡ������״̬��ȡ����
����:    ��
���:    0����ȡ��
         1��ȡ��
*********************************/
FunState LPUART_LPUCON_RXPOL_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_RXPOL_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART   �����ж�ʹ�ܺ���
����:    0����ֹ
         1��ʹ��
���:    ��
*********************************/
void LPUART_LPUCON_ERRIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_ERRIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_ERRIE_Msk);
	}
}

/*********************************
LPUART   �����ж�ʹ��״̬��ȡ����
����:    ��
���:    0����ֹ
         1��ʹ��
*********************************/
FunState LPUART_LPUCON_ERRIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_ERRIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART   �����ж�ʹ�ܺ���
����:    0����ֹ
         1��ʹ��
���:    ��
*********************************/
void LPUART_LPUCON_RXIE_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUCON |= (LPUART_LPUCON_RXIE_Msk);
	}
	else
	{
		LPUART->LPUCON &= ~(LPUART_LPUCON_RXIE_Msk);
	}
}

/*********************************
LPUART   �����ж�ʹ��״̬��ȡ
����:    ��
���:    0����ֹ
         1��ʹ��
*********************************/
FunState LPUART_LPUCON_RXIE_Getable(void)
{
	if (LPUART->LPUCON & (LPUART_LPUCON_RXIE_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  �����ж��¼����ú��������ڿ��ƺ����¼�����CPU�ṩ�����ж� ��
����:    00��STARTλ���
         01��1byte���ݽ������
         10����������ƥ��
         11��RXD�½��ػ���
�������
*********************************/
void LPUART_LPUCON_RXEV_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUCON;
	tmpreg &= ~(LPUART_LPUCON_RXEV_Msk);
	tmpreg |= SetValue;
	LPUART->LPUCON = tmpreg;
}

/*********************************
LPUART  �����ж��¼����ú���״̬��ȡ
����:    ��
�����   00��STARTλ���
         01��1byte���ݽ������
         10����������ƥ��
         11��RXD�½��ػ���
*********************************/
uint32_t LPUART_LPUCON_RXEV_Get(void)
{
	return (LPUART->LPUCON & LPUART_LPUCON_RXEV_Msk);
}

/*********************************
LPUART  ��������жϱ�־��0����
����:    ��
�����   ��
*********************************/
void LPUART_LPUIF_TC_IF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_TC_IF_Msk;
}

/*********************************
LPUART  ��������жϱ�־λ״̬��⺯��
����:    ��
�����   0�����ж�
         1��������һ֡���ݺ�����ж�
*********************************/
FlagStatus LPUART_LPUIF_TC_IF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_TC_IF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ����buffer���жϱ�־λ��0����
����:    ��
�����   ��
*********************************/
void LPUART_LPUIF_TXIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_TXIF_Msk;
}

/*********************************
LPUART  ����buffer���жϱ�־λ��⺯��
����:    ��
�����   0�����ж�
         1������buffer�պ�����ж�
*********************************/
FlagStatus LPUART_LPUIF_TXIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_TXIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  RXD�½����жϱ�־λ��0����
����:    ��
�����   ��
*********************************/
void LPUART_LPUIF_RXNEGIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_RXNEGIF_Msk;
}

/*********************************
LPUART  RXD�½����жϱ�־λ״̬��⺯��
����:   ��
�����  0�����жϲ���
        1���жϲ���
*********************************/
FlagStatus LPUART_LPUIF_RXNEGIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_RXNEGIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ��������жϱ�־λ��0����
����:   ��
�����  ��
*********************************/
void LPUART_LPUIF_RXIF_Clr(void)
{
	LPUART->LPUIF = LPUART_LPUIF_RXIF_Msk;
}

/*********************************
LPUART  ��������жϱ�־λ��⺯��
����:   ��
�����  0�����жϲ���
        1��������һ֡���ݺ�����ж�
*********************************/
FlagStatus LPUART_LPUIF_RXIF_Chk(void)
{
	if (LPUART->LPUIF & LPUART_LPUIF_RXIF_Msk)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}

/*********************************
LPUART  ���������ú���
����:   000��9600
        001��4800
        010��2400
        011��1200
        100��600
        101/110/111��300
�����  ��
*********************************/
void LPUART_LPUBAUD_BAUD_Set(uint32_t SetValue)
{
	uint32_t tmpreg;
	tmpreg = LPUART->LPUBAUD;
	tmpreg &= ~(LPUART_LPUBAUD_BAUD_Msk);
	tmpreg |= SetValue;
	LPUART->LPUBAUD = tmpreg;
}

/*********************************
LPUART  ����������״̬��ȡ
����:   ��
�����  000��9600
        001��4800
        010��2400
        011��1200
        100��600
        101/110/111��300
*********************************/
uint32_t LPUART_LPUBAUD_BAUD_Get(void)
{
	return (LPUART->LPUBAUD & LPUART_LPUBAUD_BAUD_Msk);
}

/*********************************
LPUART  ����ʹ�����ú���
����:   0���رշ���
        1���򿪷���
�����  ��
*********************************/
void LPUART_LPUEN_TXEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUEN |= (LPUART_LPUEN_TXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_TXEN_Msk)==0); //�Ĵ�����ģ�鲻ͬ��������ȷ���Ѿ�д���Ĵ�������ֹд����bitʱ���ǵ�
	}
	else
	{
		LPUART->LPUEN &= ~(LPUART_LPUEN_TXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_TXEN_Msk)!=0);
	}
}

/*********************************
LPUART  ����ʹ������״̬��ȡ����
����:   ��
�����  0���رշ���
        1���򿪷���
*********************************/
FunState LPUART_LPUEN_TXEN_Getable(void)
{
	if (LPUART->LPUEN & (LPUART_LPUEN_TXEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  ����ʹ�����ú���
����:   0���رս���
        1���򿪽���
�����  ��
*********************************/
void LPUART_LPUEN_RXEN_Setable(FunState NewState)
{
	if (NewState == ENABLE)
	{
		LPUART->LPUEN |= (LPUART_LPUEN_RXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_RXEN_Msk)==0);//�Ĵ�����ģ�鲻ͬ��������ȷ���Ѿ�д���Ĵ�������ֹд����bitʱ���ǵ�
	}
	else
	{
		LPUART->LPUEN &= ~(LPUART_LPUEN_RXEN_Msk);
    while(((LPUART->LPUEN)&LPUART_LPUEN_RXEN_Msk)!=0);
	}
}

/*********************************
LPUART  ����ʹ������״̬��ȡ����
����:   ��
�����  0���رս���
        1���򿪽���
*********************************/
FunState LPUART_LPUEN_RXEN_Getable(void)
{
	if (LPUART->LPUEN & (LPUART_LPUEN_RXEN_Msk))
	{
		return ENABLE;
	}
	else
	{
		return DISABLE;
	}
}

/*********************************
LPUART  ����ƥ�����ú�������ҪRXEV����Ϊ10����Ч��
����:   ƥ�������
�����  ��
*********************************/
void LPUART_COMPARE_Write(uint32_t SetValue)
{
	LPUART->COMPARE = SetValue;
}

/*********************************
LPUART  ����ƥ���������ݺ���
����:   ��
�����  ƥ�������
*********************************/
uint32_t LPUART_COMPARE_Read(void)
{
	return (LPUART->COMPARE);
}

/*********************************
LPUART  ÿ��bit�ĵ��ƿ����źź���
����:   12bit����
        ������9600��LPUART_MCTL_FOR9600BPS
        ������4800��LPUART_MCTL_FOR4800BPS
        ������2400��LPUART_MCTL_FOR2400BPS
        ������1200��LPUART_MCTL_FOR1200BPS
        ������600��LPUART_MCTL_FOR600BPS
        ������300��LPUART_MCTL_FOR300BPS
�����  ��
*********************************/
void LPUART_MCTL_Write(uint32_t SetValue)
{
	LPUART->MCTL = SetValue;
}

/*********************************
LPUART  ÿ��bit�ĵ��ƿ����źŶ�ȡ����
����:   ��
�����  12bit����
*********************************/
uint32_t LPUART_MCTL_Read(void)
{
	return (LPUART->MCTL);
}

/*********************************
LPUART  �Ĵ�����ʼ��
����:   ��
�����  ��
*********************************/
void LPUART_Deinit(void)
{
	//LPUART->LPURXD = ;
	//LPUART->LPUTXD = ;
	//LPUART->LPUSTA = 0x000000C0;
	LPUART->LPUCON = 0x00000000;
	//LPUART->LPUIF = ;
	LPUART->LPUBAUD = 0x00000000;
	LPUART->LPUEN = 0x00000000;
	LPUART->COMPARE = 0x00000000;
	LPUART->MCTL = 0x00000000;
}
//Code_End

/*********************************
LPUART  ����������ʼ������ 
����:  
�����  
*********************************/
void LPUART_Init(LPUART_InitTypeDef* para)
{
	LPUART_LPUBAUD_BAUD_Set(para->LPUBAUD);		//�����ʿ���
	LPUART_LPUEN_TXEN_Setable(para->TXEN);		//����ʹ��
	LPUART_LPUEN_RXEN_Setable(para->RXEN);		//����ʹ��
	LPUART_COMPARE_Write(para->COMPARE);		//����ƥ��Ĵ���
	LPUART_MCTL_Write(para->MCTL);				//���ƿ��ƼĴ���
	
	LPUART_LPUCON_SL_Set(para->SL);				//ֹͣλ����
	LPUART_LPUCON_DL_Set(para->DL);				//���ݳ���
	LPUART_LPUCON_PAREN_Setable(para->PAREN);	//У��λʹ��
	LPUART_LPUCON_PTYP_Set(para->PTYP);			//У��λ����
	
	LPUART_LPUCON_RXEV_Set(para->RXEV);		//�����ж��¼�����
	LPUART_LPUCON_TCIE_Setable(para->TCIE);		//��������ж�ʹ��
	LPUART_LPUCON_TXIE_Setable(para->TXIE);		//����buffer���ж�ʹ��
	LPUART_LPUCON_RXIE_Setable(para->RXIE);		//�����ж�ʹ��
	LPUART_LPUCON_TXPOL_Setable(para->TXPOL);		//���ݷ��ͼ���ȡ��ʹ��
	LPUART_LPUCON_RXPOL_Setable(para->RXPOL);		//���ݽ��ռ���ȡ������
	
	LPUART_LPUCON_NEDET_Setable(para->NEDET);		//�½��ؼ��ʹ��
	LPUART_LPUCON_ERRIE_Setable(para->ERRIE);		//�����ж�ʹ��
}

/*********************************
LPUART �򵥲�����ʼ������ 
����:  
�����  
*********************************/
void LPUART_SInit(LPUART_SInitTypeDef* para)
{
	LPUART_InitTypeDef para2;	
		
	switch(para->BaudRate)
	{
		case 9600:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_9600BPS;
			para2.MCTL = LPUART_MCTL_FOR9600BPS;
			break;

		case 4800:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_4800BPS;
			para2.MCTL = LPUART_MCTL_FOR4800BPS;
			break;

		case 2400:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_2400BPS;
			para2.MCTL = LPUART_MCTL_FOR2400BPS;
			break;
		
		case 1200:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_1200BPS;
			para2.MCTL = LPUART_MCTL_FOR1200BPS;
			break;
		
		case 600:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_600BPS;
			para2.MCTL = LPUART_MCTL_FOR600BPS;
			break;
		
		case 300:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_300BPS;
			para2.MCTL = LPUART_MCTL_FOR300BPS;
			break;
		
		default:
			para2.LPUBAUD = LPUART_LPUBAUD_BAUD_9600BPS;
			para2.MCTL = LPUART_MCTL_FOR9600BPS;
			break;
	}

	para2.TXEN = DISABLE;
	para2.RXEN = DISABLE;
	para2.COMPARE = 0;
	
	//ֹͣλ����
	if(OneBit == para->StopBit)
	{
		para2.SL = LPUART_LPUCON_SL_1BIT;
	}
	else
	{
		para2.SL = LPUART_LPUCON_SL_2BIT;
	}
	//����λ����
	if(Eight8Bit == para->DataBit)
	{
		para2.DL = LPUART_LPUCON_DL_8BIT;
	}
	else
	{
		para2.DL = LPUART_LPUCON_DL_7BIT;
	}
	
	//У��λ
	if(NONE == para->ParityBit)
	{
		para2.PAREN = DISABLE;
		para2.PTYP = LPUART_LPUCON_PTYP_EVEN;
	}
	else
	{
		para2.PAREN = ENABLE;
		if(EVEN == para->ParityBit)
		{
			para2.PTYP = LPUART_LPUCON_PTYP_EVEN;
		}
		else
		{
			para2.PTYP = LPUART_LPUCON_PTYP_ODD;
		}
	}
	
	para2.RXEV =LPUART_LPUCON_RXEV_MATCH;		//�����ж��¼�����
	para2.TCIE = DISABLE;		//��������ж�ʹ��
	para2.TXIE = DISABLE;		//����buffer���ж�ʹ��
	para2.RXIE = DISABLE;		//�����ж�ʹ��
	para2.TXPOL = DISABLE;		//���ݷ��ͼ���ȡ��ʹ��
	para2.RXPOL = DISABLE;		//���ݽ��ռ���ȡ������
	para2.NEDET = DISABLE;		//�½��ؼ��ʹ��
	para2.ERRIE = DISABLE;		//�����ж�ʹ��	
	
	LPUART_Init(&para2);
}

/******END OF FILE****/

