/*******************************************************************************
* Copyright (C) 2019, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file adt.h
 **
 ** Headerfile for Advance Timer functions
 ** @link ADT Group Some description @endlink
 **
 **   - 2018-04-19 Husj    First Version
 **
 ******************************************************************************/

#ifndef __HC_ADT_H__
#define __HC_ADT_H__

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_ddl.h"


/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup AdtGroup Advance Timer (ADT)
  **
 ******************************************************************************/
//@{

/******************************************************************************
 * Global type definitions
 ******************************************************************************/


 /**
 ******************************************************************************
 ** \brief ADT CHxͨ������
 *****************************************************************************/
typedef enum en_adt_CHxX_port
{
    AdtCHxA    = 0u,            ///< CHx Aͨ��
    AdtCHxB    = 1u,            ///< CHx Bͨ��
}en_adt_CHxX_port_t;

 /**
 ******************************************************************************
 ** \brief ADT TRIG�˿ڶ���
 *****************************************************************************/
typedef enum en_adt_trig_port
{
    AdtTrigA    = 0u,            ///< TIMx ����A�˿�
    AdtTrigB    = 1u,            ///< TIMx ����B�˿�
    AdtTrigC    = 2u,            ///< TIMx ����C�˿�
    AdtTrigD    = 3u,            ///< TIMx ����D�˿�
}en_adt_trig_port_t;

/**
 ******************************************************************************
 ** \brief ADTͨ�ÿ��� - Z����������������
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_gconr_zmsk
{
    AdtZMaskDis   = 0u,      ///< Z���������ι�����Ч
    AdtZMask4Cyl  = 1u,      ///< λ�ü���������������4�����������ڵ�Z�����뱻����
    AdtZMask8Cyl  = 2u,      ///< λ�ü���������������8�����������ڵ�Z�����뱻����
    AdtZMask16Cyl = 3u,      ///< λ�ü���������������16�����������ڵ�Z�����뱻����
}en_adt_gconr_zmsk_t;

/**
 ******************************************************************************
 ** \brief ADTͨ�ÿ��� - ����ʱ��ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_cnt_ckdiv
{
    AdtClkPClk0        = 0u,         ///< PCLK0
    AdtClkPClk0Div2    = 1u,         ///< PCLK0/2
    AdtClkPClk0Div4    = 2u,         ///< PCLK0/4
    AdtClkPClk0Div8    = 3u,         ///< PCLK0/8
    AdtClkPClk0Div16   = 4u,         ///< PCLK0/16
    AdtClkPClk0Div64   = 5u,         ///< PCLK0/64
    AdtClkPClk0Div256  = 6u,         ///< PCLK0/256
    AdtClkPClk0Div1024 = 7u,         ///< PCLK0/1024
}en_adt_cnt_ckdiv_t;

/**
 ******************************************************************************
 ** \brief ADT����ģʽ
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_cnt_mode
{
    AdtSawtoothMode  = 0u,          ///< ��ݲ�ģʽ
    AdtTriangleModeA = 4u,          ///< ���ǲ�Aģʽ
    AdtTriangleModeB = 5u,          ///< ���ǲ�Bģʽ
}en_adt_cnt_mode_t;

/**
 ******************************************************************************
 ** \brief ADT��������
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_cnt_dir
{
    AdtCntDown = 0u,      ///< �ݼ�����
    AdtCntUp   = 1u,      ///< �ݼӼ���
}en_adt_cnt_dir_t;

/**
 ******************************************************************************
 ** \brief ADTͨ�ñȽϻ�׼
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_compare
{
    AdtCompareA = 0u,            ///< ͨ�ñȽϻ�׼A
    AdtCompareB = 1u,            ///< ͨ�ñȽϻ�׼B
    AdtCompareC = 2u,            ///< ͨ�ñȽϻ�׼C
    AdtCompareD = 3u,            ///< ͨ�ñȽϻ�׼D
}en_adt_compare_t;

/**
 ******************************************************************************
 ** \brief ADTר�ñȽϻ�׼
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_special_compare
{
    AdtSpclCompA = 0u,            ///< ר�ñȽϻ�׼A
    AdtSpclCompB = 1u,            ///< ר�ñȽϻ�׼B
}en_adt_special_compare_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - TIMx���״̬����
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_disval
{
    AdtTIMxDisValNorm = 0u,     ///< ǿ�������Ч����0~3�б�ѡ�����������ʱ��CHx�˿��������
    AdtTIMxDisValHiZ  = 1u,     ///< ǿ�������Ч����0~3�б�ѡ�����������ʱ��CHx�˿��������̬
    AdtTIMxDisValLow  = 2u,     ///< ǿ�������Ч����0~3�б�ѡ�����������ʱ��CHx�˿�����͵�ƽ
    AdtTIMxDisValHigh = 3u,     ///< ǿ�������Ч����0~3�б�ѡ�����������ʱ��CHx�˿�����ߵ�ƽ
}en_adt_pconr_disval_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHxǿ�������Ч����ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_dissel
{
    AdtCHxDisSel0 = 0u,        ///< ѡ��ǿ�������Ч����0
    AdtCHxDisSel1 = 1u,        ///< ѡ��ǿ�������Ч����1
    AdtCHxDisSel2 = 2u,        ///< ѡ��ǿ�������Ч����2
    AdtCHxDisSel3 = 3u,        ///< ѡ��ǿ�������Ч����3
}en_adt_pconr_dissel_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHx����ֵƥ��ʱ�˿�״̬�趨
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_perc
{
    AdtCHxPeriodLow  = 0u,      ///< ����������ֵ������ֵ���ʱ��CHx�˿��������Ϊ�͵�ƽ
    AdtCHxPeriodHigh = 1u,      ///< ����������ֵ������ֵ���ʱ��CHx�˿�����趨Ϊ�ߵ�ƽ
    AdtCHxPeriodKeep = 2u,      ///< ����������ֵ������ֵ���ʱ��CHx�˿�����趨Ϊ��ǰ״̬
    AdtCHxPeriodInv  = 3u,      ///< ����������ֵ������ֵ���ʱ��CHx�˿�����趨Ϊ��ת��ƽ
}en_adt_pconr_perc_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHx�Ƚ�ֵƥ��ʱ�˿�״̬�趨
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_cmpc
{
    AdtCHxCompareLow  = 0u,     ///< ����������ֵ��GCMxR���ʱ��CHx�˿��������Ϊ�͵�ƽ
    AdtCHxCompareHigh = 1u,     ///< ����������ֵ��GCMxR���ʱ��CHx�˿�����趨Ϊ�ߵ�ƽ
    AdtCHxCompareKeep = 2u,     ///< ����������ֵ��GCMxR���ʱ��CHx�˿�����趨Ϊ��ǰ״̬
    AdtCHxCompareInv  = 3u,     ///< ����������ֵ��GCMxR���ʱ��CHx�˿�����趨Ϊ��ת��ƽ
}en_adt_pconr_cmpc_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHx�˿����
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_port_out
{
    AdtCHxPortOutLow  = 0u,  ///< CHx�˿�����趨Ϊ�͵�ƽ
    AdtCHxPortOutHigh = 1u,  ///< CHx�˿�����趨Ϊ�ߵ�ƽ
}en_adt_pconr_port_out_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHx�˿ڹ���ģʽѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_capc
{
    AdtCHxCompareOutput = 0u,   ///< CHx�˿��趨Ϊ�Ƚ��������
    AdtCHxCompareInput  = 1u,   ///< CHx�˿��趨Ϊ�������빦��
}en_adt_pconr_capc_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڿ��� - CHx������ʼֹͣ�˿�״̬ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pconr_stastps
{
    AdtCHxStateSelSS   = 0u,    ///< ������ʼ��ֹͣʱ��CHx�˿������STACB��STPCB����
    AdtCHxStateSelKeep = 1u,    ///< ������ʼ��ֹͣʱ��CHx�˿�����趨Ϊ��ǰ״̬
}en_adt_pconr_stastps_t;

/**
 ******************************************************************************
 ** \brief ADT�������� - CHx���������趨
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_dconr_sepa
{
    AdtCHxDtSeperate = 0u,      ///< DTUAR��DTDAR�ֱ��趨
    AdtCHxDtEqual    = 1u,      ///< DTDAR��ֵ��DTUAR��ֵ�Զ����
}en_adt_dconr_sepa_t;

/**
 ******************************************************************************
 ** \brief ADT�˲����� - TRIx/TIMxIx�˿��˲�������׼ʱ��ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_fconr_nofick
{
    AdtFltClkPclk0      = 0u,    ///< PCLK0
    AdtFltClkPclk0Div4  = 1u,    ///< PCLK0/4
    AdtFltClkPclk0Div16 = 2u,    ///< PCLK0/16
    AdtFltClkPclk0Div64 = 3u,    ///< PCLK0/64
}en_adt_fconr_nofick_t;

/**
 ******************************************************************************
 ** \brief ADT��Ч���� - TIMx��Ч����ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_vperr_pcnts
{
    AdtPeriodCnts0 = 0u,         ///< ��Ч����ѡ������Ч
    AdtPeriodCnts1 = 1u,         ///< ÿ��1��������Чһ��
    AdtPeriodCnts2 = 2u,         ///< ÿ��2��������Чһ��
    AdtPeriodCnts3 = 3u,         ///< ÿ��3��������Чһ��
    AdtPeriodCnts4 = 4u,         ///< ÿ��4��������Чһ��
    AdtPeriodCnts5 = 5u,         ///< ÿ��5��������Чһ��
    AdtPeriodCnts6 = 6u,         ///< ÿ��6��������Чһ��
    AdtPeriodCnts7 = 7u,         ///< ÿ��7��������Чһ��
}en_adt_vperr_pcnts_t;

/**
 ******************************************************************************
 ** \brief ADT��Ч���� - ��������ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_vperr_pcnte
{
    AdtPeriodCnteDisable = 0u,     ///< ��Ч����ѡ������Ч
    AdtPeriodCnteMin     = 1u,     ///< ��ݲ������ϡ����������ǲ�������Ϊ��������
    AdtPeriodCnteMax     = 2u,     ///< ��ݲ������ϡ����������ǲ�������Ϊ��������
    AdtPeriodCnteBoth    = 3u,     ///< ��ݲ������ϡ����������ǲ����壬������Ϊ��������
}en_adt_vperr_pcnte_t;

/**
 ******************************************************************************
 ** \brief ADT�˿ڴ������� - ����Դѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_ttrig_trigxs
{
    AdtTrigxSelPA3  = 0u,    ///< PA3
    AdtTrigxSelPB3  = 1u,    ///< PB3
    AdtTrigxSelPC3  = 2u,    ///< PC3
    AdtTrigxSelPD3  = 3u,    ///< PD3
    AdtTrigxSelPA7  = 4u,    ///< PA7
    AdtTrigxSelPB7  = 5u,    ///< PB7
    AdtTrigxSelPC7  = 6u,    ///< PC7
    AdtTrigxSelPD7  = 7u,    ///< PD7
    AdtTrigxSelPA11 = 8u,    ///< PA11
    AdtTrigxSelPB11 = 9u,    ///< PB11
    AdtTrigxSelPC11 = 10u,   ///< PC11
    AdtTrigxSelPD1  = 11u,   ///< PD1
    AdtTrigxSelPA15 = 12u,   ///< PA15
    AdtTrigxSelPB15 = 13u,   ///< PB15
    AdtTrigxSelPC5  = 14u,   ///< PC5
    AdtTrigxSelPD5  = 15u,   ///< PD5
}en_adt_ttrig_trigxs_t;

/**
 ******************************************************************************
 ** \brief ADT AOS�������� - AOSx����Դѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_itrig_iaosxs
{
    AdtAosxTrigSelTim0Int   = 0u,    ///< TIM0_INT
    AdtAosxTrigSelTim1Int   = 1u,    ///< TIM1_INT
    AdtAosxTrigSelTim2Int   = 2u,    ///< TIM2_INT
    AdtAosxTrigSelLpTimInt  = 3u,    ///< LPTIMER_INT
    AdtAosxTrigSelTim4Int   = 4u,    ///< TIM4_INT
    AdtAosxTrigSelTim5Int   = 5u,    ///< TIM5_INT
    AdtAosxTrigSelTim6Int   = 6u,    ///< TIM6_INT
    AdtAosxTrigSelUart0Int  = 7u,    ///< UART0_INT
    AdtAosxTrigSelUart1Int  = 8u,    ///< UART1_INT
    AdtAosxTrigSelLpUartInt = 9u,    ///< LPUART_INT
    AdtAosxTrigSelVc0Int    = 10u,   ///< VC0_INT
    AdtAosxTrigSelVc1Int    = 11u,   ///< VC1_INT
    AdtAosxTrigSelRtcInt    = 12u,   ///< RTC_INT
    AdtAosxTrigSelPcaInt    = 13u,   ///< PCA_INT
    AdtAosxTrigSelSpiInt    = 14u,   ///< SPI_INT
    AdtAosxTrigSelAdcInt    = 15u,   ///< ADC_INT
}en_adt_itrig_iaosxs_t;

/**
 ******************************************************************************
 ** \brief ADTӲ��(����/ֹͣ/����/����)�¼�����ѡ��
 **
 ** \note
 ******************************************************************************/

typedef enum en_adt_hw_trig
{
    AdtHwTrigAos0        = 0u,       ///< ��AOS�����¼�����0��Ч
    AdtHwTrigAos1        = 1u,       ///< ��AOS�����¼�����1��Ч
    AdtHwTrigAos2        = 2u,       ///< ��AOS�����¼�����2��Ч
    AdtHwTrigAos3        = 3u,       ///< ��AOS�����¼�����3��Ч
    AdtHwTrigCHxARise    = 4u,       ///< CHxA�˿��ϲ�����������
    AdtHwTrigCHxAFall    = 5u,       ///< CHxA�˿��ϲ������½���
    AdtHwTrigCHxBRise    = 6u,       ///< CHxB�˿��ϲ�����������
    AdtHwTrigCHxBFall    = 7u,       ///< CHxB�˿��ϲ������½���
    AdtHwTrigTimTriARise = 8u,       ///< TIMTRIA�˿��ϲ�����������
    AdtHwTrigTimTriAFall = 9u,       ///< TIMTRIA�˿��ϲ������½���
    AdtHwTrigTimTriBRise = 10u,      ///< TIMTRIB�˿��ϲ�����������
    AdtHwTrigTimTriBFall = 11u,      ///< TIMTRIB�˿��ϲ������½���
    AdtHwTrigTimTriCRise = 12u,      ///< TIMTRIC�˿��ϲ�����������
    AdtHwTrigTimTriCFall = 13u,      ///< TIMTRIC�˿��ϲ������½���
    AdtHwTrigTimTriDRise = 14u,      ///< TIMTRID�˿��ϲ�����������
    AdtHwTrigTimTriDFall = 15u,      ///< TIMTRID�˿��ϲ������½���
    AdtHwTrigEnd         = 16u,
}en_adt_hw_trig_t;

/**
 ******************************************************************************
 ** \brief ADTӲ��(�ݼ�/�ݼ�)�¼�����ѡ��
 **
 ** \note
 ******************************************************************************/

typedef enum en_adt_hw_cnt
{
    AdtHwCntCHxALowCHxBRise   = 0u,      ///< CHxA�˿�Ϊ�͵�ƽʱ��CHxB�˿��ϲ�����������
    AdtHwCntCHxALowCHxBFall   = 1u,      ///< CHxA�˿�Ϊ�͵�ƽʱ��CHxB�˿��ϲ������½���
    AdtHwCntCHxAHighCHxBRise  = 2u,      ///< CHxA�˿�Ϊ�ߵ�ƽʱ��CHxB�˿��ϲ�����������
    AdtHwCntCHxAHighCHxBFall  = 3u,      ///< CHxA�˿�Ϊ�ߵ�ƽʱ��CHxB�˿��ϲ������½���
    AdtHwCntCHxBLowCHxARise   = 4u,      ///< CHxB�˿�Ϊ�͵�ƽʱ��CHxA�˿��ϲ�����������
    AdtHwCntCHxBLowCHxAFall   = 5u,      ///< CHxB�˿�Ϊ�͵�ƽʱ��CHxA�˿��ϲ������½���
    AdtHwCntCHxBHighChxARise  = 6u,      ///< CHxB�˿�Ϊ�ߵ�ƽʱ��CHxA�˿��ϲ�����������
    AdtHwCntCHxBHighCHxAFall  = 7u,      ///< CHxB�˿�Ϊ�ߵ�ƽʱ��CHxA�˿��ϲ������½���
    AdtHwCntTimTriARise       = 8u,      ///< TIMTRIA�˿��ϲ�����������
    AdtHwCntTimTriAFall       = 9u,      ///< TIMTRIA�˿��ϲ������½���
    AdtHwCntTimTriBRise       = 10u,     ///< TIMTRIB�˿��ϲ�����������
    AdtHwCntTimTriBFall       = 11u,     ///< TIMTRIB�˿��ϲ������½���
    AdtHwCntTimTriCRise       = 12u,     ///< TIMTRIC�˿��ϲ�����������
    AdtHwCntTimTriCFall       = 13u,     ///< TIMTRIC�˿��ϲ������½���
    AdtHwCntTimTriDRise       = 14u,     ///< TIMTRID�˿��ϲ�����������
    AdtHwCntTimTriDFall       = 15u,     ///< TIMTRID�˿��ϲ������½���
    AdtHwCntAos0              = 16u,     ///< ��AOS�����¼�����0��Ч
    AdtHwCntAos1              = 17u,     ///< ��AOS�����¼�����1��Ч
    AdtHwCntAos2              = 18u,     ///< ��AOS�����¼�����2��Ч
    AdtHwCntAos3              = 19u,     ///< ��AOS�����¼�����3��Ч
    AdtHwCntMax               = 20u,
}en_adt_hw_cnt_t;

/**
 ******************************************************************************
 ** \brief ADT�˿�ɲ�����Կ���
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_ptbrk_polarity
{
    AdtPtBrkHigh = 0u,     ///< �˿�ɲ�����Ըߵ�ƽ��Ч
    AdtPtBrkLow  = 1u,     ///< �˿�ɲ�����Ե͵�ƽ��Ч
}en_adt_ptbrk_polarity_t;

/**
 ******************************************************************************
 ** \brief ADT PWMչƵ����ѡ��
 **
 ** \note
 ******************************************************************************/
typedef enum en_adt_pwm_dither_type
{
    AdtPwmDitherUnderFlow = 0u,    ///< PWMչƵ���������
    AdtPwmDitherOverFlow  = 1u,    ///< PWMչƵ���������
}en_adt_pwm_dither_type_t;

/**
 ******************************************************************************
 ** \brief ADT�ж�����
 **
 ** \note
 ******************************************************************************/

typedef enum en_adt_irq_type
{
    AdtCMAIrq  = 0u,     ///< ����ƥ��A���򲶻����룩�ж�
    AdtCMBIrq  = 1u,     ///< ����ƥ��B���򲶻����룩�ж�
    AdtCMCIrq  = 2u,     ///< ����ƥ��C�ж�
    AdtCMDIrq  = 3u,     ///< ����ƥ��D�ж�
    AdtOVFIrq  = 6u,     ///< ����ƥ���ж�
    AdtUDFIrq  = 7u,     ///< ����ƥ���ж�
    AdtDTEIrq  = 8u,     ///< ����ʱ������ж�
    AdtSAMLIrq = 14u,    ///< ͬ���ж�
    AdtSAMHIrq = 15u,    ///< ͬ���ж�
}en_adt_irq_type_t;

typedef enum en_adt_state_type
{
    AdtCMAF = 0,           ///< ����ƥ��A��־
    AdtCMBF = 1,           ///< ����ƥ��B��־
    AdtCMCF = 2,           ///< ����ƥ��C��־
    AdtCMDF = 3,           ///< ����ƥ��D��־    
    AdtOVFF = 6,           ///< ����ƥ���־
    AdtUDFF = 7,           ///< ����ƥ���־    
    AdtDTEF = 8,           ///< ����ʱ������־
    AdtCMSAUF = 9,         ///< ���ϼ���ר�ñȽϻ�׼ֵƥ��A��־
    AdtCMSADF = 10,        ///< ���¼���ר�ñȽϻ�׼ֵƥ��B��־    
    AdtCMSBUF = 11,        ///< ���ϼ���ר�ñȽϻ�׼ֵƥ��A��־  
    AdtCMSBDF = 12,        ///< ���¼���ר�ñȽϻ�׼ֵƥ��B��־    
    AdtCntDir = 31,        ///< ��������
}en_adt_state_type_t;

/**
 ******************************************************************************
 ** \brief ADT���ͬ������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_sw_sync
{
    boolean_t               bAdTim4;      ///< Timer 4
    boolean_t               bAdTim5;      ///< Timer 5
    boolean_t               bAdTim6;      ///< Timer 6

}stc_adt_sw_sync_t;

/**
 ******************************************************************************
 ** \brief ADT AOS��������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_aos_trig_cfg
{
    en_adt_itrig_iaosxs_t   enAos0TrigSrc;      ///< AOS0����Դѡ��
    en_adt_itrig_iaosxs_t   enAos1TrigSrc;      ///< AOS1����Դѡ��
    en_adt_itrig_iaosxs_t   enAos2TrigSrc;      ///< AOS2����Դѡ��
    en_adt_itrig_iaosxs_t   enAos3TrigSrc;      ///< AOS3����Դѡ��
}stc_adt_aos_trig_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT �жϴ�������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_irq_trig_cfg
{
    boolean_t   bAdtSpecilMatchBTrigDmaEn;  ///< ר�ñȽϻ�׼ֵƥ��Bʹ�ܴ���DMA
    boolean_t   bAdtSpecilMatchATrigDmaEn;  ///< ר�ñȽϻ�׼ֵƥ��Aʹ�ܴ���DMA
    boolean_t   bAdtUnderFlowTrigDmaEn;     ///< ����ƥ��ʹ�ܴ���DMA
    boolean_t   bAdtOverFlowTrigDmaEn;      ///< ����ƥ��ʹ�ܴ���DMA
    boolean_t   bAdtCntMatchDTrigDmaEn;     ///< ����ƥ��Dʹ�ܴ���DMA
    boolean_t   bAdtCntMatchCTrigDmaEn;     ///< ����ƥ��Cʹ�ܴ���DMA
    boolean_t   bAdtCntMatchBTrigDmaEn;     ///< ����ƥ��Bʹ�ܴ���DMA
    boolean_t   bAdtCntMatchATrigDmaEn;     ///< ����ƥ��Aʹ�ܴ���DMA
    boolean_t   bAdtSpecilMatchBTrigEn;     ///< ר�ñȽϻ�׼ֵƥ��Bʹ�ܴ���ADC
    boolean_t   bAdtSpecilMatchATrigEn;     ///< ר�ñȽϻ�׼ֵƥ��Aʹ�ܴ���ADC
    boolean_t   bAdtUnderFlowTrigEn;        ///< ����ƥ��ʹ�ܴ���ADC
    boolean_t   bAdtOverFlowTrigEn;         ///< ����ƥ��ʹ�ܴ���ADC
    boolean_t   bAdtCntMatchDTrigEn;        ///< ����ƥ��Dʹ�ܴ���ADC
    boolean_t   bAdtCntMatchCTrigEn;        ///< ����ƥ��Cʹ�ܴ���ADC
    boolean_t   bAdtCntMatchBTrigEn;        ///< ����ƥ��Bʹ�ܴ���ADC
    boolean_t   bAdtCntMatchATrigEn;        ///< ����ƥ��Aʹ�ܴ���ADC
}stc_adt_irq_trig_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT Trig�˿�����
 ** \note
 ******************************************************************************/
typedef struct stc_adt_port_trig_cfg
{
    en_adt_ttrig_trigxs_t   enTrigSrc;      ///< ����Դѡ��
    boolean_t               bFltEn;         ///< ����Դ���������˲�ʹ��
    en_adt_fconr_nofick_t   enFltClk;       ///< �˲�������׼ʱ��
}stc_adt_port_trig_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT Z���������ι�������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_zmask_cfg
{
    en_adt_gconr_zmsk_t     enZMaskCycle;              ///< Z���������μ�������ѡ��
    boolean_t               bFltPosCntMaksEn;          ///< Z������ʱ�����������ڣ�λ�ü����������㹦�ܲ����Σ�FALSE��������(TRUE)
    boolean_t               bFltRevCntMaksEn;          ///< Z������ʱ�����������ڣ���ת�������ļ������ܲ����Σ�FALSE��������(TRUE)
}stc_adt_zmask_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT TIMxX�˿�����
 ** \note
 ******************************************************************************/
typedef struct stc_adt_TIMxX_port_cfg
{
    en_adt_pconr_capc_t     enCap;      ///< �˿ڹ���ģʽ
    boolean_t               bOutEn;     ///< ���ʹ��
    en_adt_pconr_perc_t     enPerc;     ///< ����ֵƥ��ʱ�˿�״̬
    en_adt_pconr_cmpc_t     enCmpc;     ///< �Ƚ�ֵƥ��ʱ�˿�״̬
    en_adt_pconr_stastps_t  enStaStp;   ///< ������ʼֹͣ�˿�״̬ѡ��
    en_adt_pconr_port_out_t enStaOut;   ///< ������ʼ�˿����״̬
    en_adt_pconr_port_out_t enStpOut;   ///< ����ֹͣ�˿����״̬
    en_adt_pconr_disval_t   enDisVal;   ///< ǿ�������Чʱ���״̬����
    en_adt_pconr_dissel_t   enDisSel;   ///< ǿ�������Ч����ѡ��
    boolean_t               bFltEn;     ///< �˿ڲ��������˲�ʹ��
    en_adt_fconr_nofick_t   enFltClk;   ///< �˿��˲�������׼ʱ��
}stc_adt_CHxX_port_cfg_t;

/**
 ******************************************************************************
 ** \brief ADTɲ���˿�����
 ** \note
 ******************************************************************************/
typedef struct stc_adt_break_port_cfg
{
    boolean_t               bPortEn;    ///< �˿�ʹ��
    en_adt_ptbrk_polarity_t enPol;      ///< ����ѡ��
}stc_adt_break_port_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT��Ч����3����
 ** \note
 ******************************************************************************/
typedef struct stc_adt_disable_3_cfg
{
    stc_adt_break_port_cfg_t    stcBrkPtCfg[16];    ///< ɲ���˿�����
    boolean_t                   bFltEn;             ///< ɲ���˿��˲�ʹ��
    en_adt_fconr_nofick_t       enFltClk;           ///< �˲�������׼ʱ��
}stc_adt_disable_3_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT��Ч����1����
 ** \note
 ******************************************************************************/
typedef struct stc_adt_disable_1_cfg
{
    boolean_t   bTim6OutSH;     ///< TIM6���ͬ��
    boolean_t   bTim5OutSH;     ///< TIM5���ͬ��
    boolean_t   bTim4OutSH;     ///< TIM4���ͬ��
    boolean_t   bTim6OutSL;     ///< TIM6���ͬ��
    boolean_t   bTim5OutSL;     ///< TIM5���ͬ��
    boolean_t   bTim4OutSL;     ///< TIM4���ͬ��
}stc_adt_disable_1_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT PWMչƵ��������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_pwm_dither_cfg
{
    en_adt_pwm_dither_type_t    enAdtPDType;    ///< PWMչƵ����ѡ��
    boolean_t   bTimxBPDEn;                     ///< PWMͨ��BչƵʹ��
    boolean_t   bTimxAPDEn;                     ///< PWMͨ��AչƵʹ��
}stc_adt_pwm_dither_cfg_t;


/**
 ******************************************************************************
 ** \brief ADT������������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_basecnt_cfg
{
    en_adt_cnt_mode_t       enCntMode;      ///< ����ģʽ
    en_adt_cnt_dir_t        enCntDir;       ///< ��������
    en_adt_cnt_ckdiv_t      enCntClkDiv;    ///< ����ʱ��ѡ��
}stc_adt_basecnt_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT����״̬
 ** \note
 ******************************************************************************/
typedef struct stc_adt_cntstate_cfg
{
    uint16_t         u16Counter;      ///< ��ǰ�������ļ���ֵ
    boolean_t        enCntDir;        ///< ��������
    uint8_t          u8ValidPeriod;   ///< ��Ч���ڼ���
    boolean_t        bCMSBDF;         ///< ���¼���ר�ñȽϻ�׼ֵƥ��B��־
    boolean_t        bCMSBUF;         ///< ���ϼ���ר�ñȽϻ�׼ֵƥ��A��־
    boolean_t        bCMSADF;         ///< ���¼���ר�ñȽϻ�׼ֵƥ��B��־
    boolean_t        bCMSAUF;         ///< ���ϼ���ר�ñȽϻ�׼ֵƥ��A��־
    boolean_t        bDTEF;           ///< ����ʱ������־
    boolean_t        bUDFF;           ///< ����ƥ���־
    boolean_t        bOVFF;           ///< ����ƥ���־
    boolean_t        bCMDF;           ///< ����ƥ��D��־
    boolean_t        bCMCF;           ///< ����ƥ��C��־
    boolean_t        bCMBF;           ///< ����ƥ��B��־
    boolean_t        bCMAF;           ///< ����ƥ��A��־
}stc_adt_cntstate_cfg_t;

/**
 ******************************************************************************
 ** \brief ADT��Ч��������
 ** \note
 ******************************************************************************/
typedef struct stc_adt_validper_cfg
{
    en_adt_vperr_pcnts_t    enValidCnt;     ///< ��Ч����ѡ��
    en_adt_vperr_pcnte_t    enValidCdt;     ///< ��Ч���ڼ�������
    boolean_t               bPeriodD;       ///< ͨ���ź���Ч����ѡ��D
    boolean_t               bPeriodC;       ///< ͨ���ź���Ч����ѡ��C
    boolean_t               bPeriodB;       ///< ͨ���ź���Ч����ѡ��B
    boolean_t               bPeriodA;       ///< ͨ���ź���Ч����ѡ��A
}stc_adt_validper_cfg_t;

/******************************************************************************
 * Global definitions
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
//����Ӳ���ݼ��¼�
en_result_t Adt_CfgHwCntUp(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_cnt_t enAdtHwCntUp);
//���Ӳ���ݼ��¼�
en_result_t Adt_ClearHwCntUp(M0P_ADTIM_TypeDef *ADTx);
//����Ӳ���ݼ��¼�
en_result_t Adt_CfgHwCntDwn(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_cnt_t enAdtHwCntDwn);
//���Ӳ���ݼ��¼�
en_result_t Adt_ClearHwCntDwn(M0P_ADTIM_TypeDef *ADTx);
//����Ӳ�������¼�
en_result_t Adt_CfgHwStart(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_trig_t enAdtHwStart);
//���Ӳ�������¼�
en_result_t Adt_ClearHwStart(M0P_ADTIM_TypeDef *ADTx);
//ʹ��Ӳ�������¼�
en_result_t Adt_EnableHwStart(M0P_ADTIM_TypeDef *ADTx);
//��ֹӲ�������¼�
en_result_t Adt_DisableHwStart(M0P_ADTIM_TypeDef *ADTx);
//����Ӳ��ֹͣ�¼�
en_result_t Adt_CfgHwStop(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_trig_t enAdtHwStop);
//���Ӳ��ֹͣ�¼�
en_result_t Adt_ClearHwStop(M0P_ADTIM_TypeDef *ADTx);
//ʹ��Ӳ��ֹͣ�¼�
en_result_t Adt_EnableHwStop(M0P_ADTIM_TypeDef *ADTx);
//��ֹӲ��ֹͣ�¼�
en_result_t Adt_DisableHwStop(M0P_ADTIM_TypeDef *ADTx);
//����Ӳ�������¼�
en_result_t Adt_CfgHwClear(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_trig_t enAdtHwClear);
//���Ӳ�������¼�
en_result_t Adt_ClearHwClear(M0P_ADTIM_TypeDef *ADTx);
//ʹ��Ӳ�������¼�
en_result_t Adt_EnableHwClear(M0P_ADTIM_TypeDef *ADTx);
//��ֹӲ�������¼�
en_result_t Adt_DisableHwClear(M0P_ADTIM_TypeDef *ADTx);
//����Aͨ��Ӳ�������¼�
en_result_t Adt_CfgHwCaptureA(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_trig_t enAdtHwCaptureA);
//���Aͨ��Ӳ�������¼�
en_result_t Adt_ClearHwCaptureA(M0P_ADTIM_TypeDef *ADTx);
//����Bͨ��Ӳ�������¼�
en_result_t Adt_CfgHwCaptureB(M0P_ADTIM_TypeDef *ADTx, en_adt_hw_trig_t enAdtHwCaptureB);
//���Bͨ��Ӳ�������¼�
en_result_t Adt_ClearHwCaptureB(M0P_ADTIM_TypeDef *ADTx);
//���ͬ������
en_result_t Adt_SwSyncStart(stc_adt_sw_sync_t* pstcAdtSwSyncStart);
//���ͬ��ֹͣ
en_result_t Adt_SwSyncStop(stc_adt_sw_sync_t* pstcAdtSwSyncStop);
//���ͬ������
en_result_t Adt_SwSyncClear(stc_adt_sw_sync_t* pstcAdtSwSyncClear);
//��ȡ���ͬ��״̬
en_result_t Adt_GetSwSyncState(stc_adt_sw_sync_t* pstcAdtSwSyncState);
//AOS��������
en_result_t Adt_AosTrigCfg(stc_adt_aos_trig_cfg_t* pstcAdtAosTrigCfg);
//�жϴ�������
en_result_t Adt_IrqTrigCfg(M0P_ADTIM_TypeDef *ADTx,
                              stc_adt_irq_trig_cfg_t* pstcAdtIrqTrigCfg);
//�˿ڴ�������
en_result_t Adt_PortTrigCfg(en_adt_trig_port_t enAdtTrigPort,
                               stc_adt_port_trig_cfg_t* pstcAdtPortTrigCfg);
//CHxX�˿�����
en_result_t Adt_CHxXPortCfg(M0P_ADTIM_TypeDef *ADTx,
                                en_adt_CHxX_port_t enAdtCHxXPort,
                                stc_adt_CHxX_port_cfg_t* pstcAdtCHxXCfg);
//ʹ�ܶ˿�ɲ��
en_result_t Adt_EnableBrakePort(uint8_t port, stc_adt_break_port_cfg_t* pstcAdtBrkPtCfg);
//����˿�ɲ��
void Adt_ClearBrakePort(void);
//��Ч����3����(�˿�ɲ��)
en_result_t Adt_Disable3Cfg(stc_adt_disable_3_cfg_t* pstcAdtDisable3);
//���ɲ�� Enable/Disable(����������Ч����3ʹ�ܵ������)
en_result_t Adt_SwBrake(boolean_t bSwBrk);
//��ȡ�˿�ɲ����־
boolean_t Adt_GetPortBrakeFlag(void);
//����˿�ɲ����־
void Adt_ClearPortBrakeFlag(void);
//��Ч����1����(ͬ��ͬ��ɲ��)
en_result_t Adt_Disable1Cfg(stc_adt_disable_1_cfg_t* pstcAdtDisable1);
//��ȡͬ��ͬ��ɲ����־
boolean_t Adt_GetSameBrakeFlag(void);
//���ͬ��ͬ��ɲ����־
void Adt_ClearSameBrakeFlag(void);
//PWMչƵ����
en_result_t Adt_PwmDitherCfg(M0P_ADTIM_TypeDef *ADTx, stc_adt_pwm_dither_cfg_t* pstcAdtPwmDitherCfg);
//AdvTimer��ʼ��
en_result_t Adt_Init(M0P_ADTIM_TypeDef *ADTx, stc_adt_basecnt_cfg_t* pstcAdtBaseCntCfg);
//AdvTimerȥ��ʼ��
en_result_t Adt_DeInit(M0P_ADTIM_TypeDef *ADTx);
//AdvTimert����
en_result_t Adt_StartCount(M0P_ADTIM_TypeDef *ADTx);
//AdvTimertֹͣ
en_result_t Adt_StopCount(M0P_ADTIM_TypeDef *ADTx);
//���ü���ֵ
en_result_t Adt_SetCount(M0P_ADTIM_TypeDef *ADTx, uint16_t u16Value);
//��ȡ����ֵ
uint16_t Adt_GetCount(M0P_ADTIM_TypeDef *ADTx);
//�������ֵ
en_result_t Adt_ClearCount(M0P_ADTIM_TypeDef *ADTx);
//��ȡ��Ч���ڼ���ֵ
uint8_t Adt_GetVperNum(M0P_ADTIM_TypeDef *ADTx);
//��ȡ״̬��־
boolean_t Adt_GetState(M0P_ADTIM_TypeDef *ADTx, en_adt_state_type_t enstate);
//���ü�������
en_result_t Adt_SetPeriod(M0P_ADTIM_TypeDef *ADTx, uint16_t u16Period);
//���ü������ڻ���
en_result_t Adt_SetPeriodBuf(M0P_ADTIM_TypeDef *ADTx, uint16_t u16PeriodBuf);
//����������ڻ���
en_result_t Adt_ClearPeriodBuf(M0P_ADTIM_TypeDef *ADTx);
//������Ч��������
en_result_t Adt_SetValidPeriod(M0P_ADTIM_TypeDef *ADTx,
                               stc_adt_validper_cfg_t* pstcAdtValidPerCfg);
//���ñȽ����������׼ֵ
en_result_t Adt_SetCompareValue(M0P_ADTIM_TypeDef *ADTx,
                                en_adt_compare_t enAdtCompare,
                                uint16_t u16Compare);
//����ͨ�ñȽ�ֵ/����ֵ�Ļ��洫��
en_result_t Adt_EnableValueBuf(M0P_ADTIM_TypeDef *ADTx,
                                 en_adt_CHxX_port_t enAdtCHxXPort,
                                 boolean_t bCompareBufEn);
//����Ƚ��������ֵ/����ֵ����
en_result_t Adt_ClearValueBuf(M0P_ADTIM_TypeDef *ADTx,
                                     en_adt_CHxX_port_t enAdtCHxXPort);
//��ȡ����ֵ
en_result_t Adt_GetCaptureValue(M0P_ADTIM_TypeDef *ADTx,
                                en_adt_CHxX_port_t enAdtCHxXPort,
                                uint16_t* pu16Capture);
//��ȡ���񻺴�ֵ
en_result_t Adt_GetCaptureBuf(M0P_ADTIM_TypeDef *ADTx,
                                en_adt_CHxX_port_t enAdtCHxXPort,
                                uint16_t* pu16CaptureBuf);
//��������ʱ���ϻ�׼ֵ
en_result_t Adt_SetDTUA(M0P_ADTIM_TypeDef *ADTx,
                        uint16_t u16Value);
//��������ʱ���»�׼ֵ
en_result_t Adt_SetDTDA(M0P_ADTIM_TypeDef *ADTx,
                        uint16_t u16Value);
//��������ʱ�书��
en_result_t Adt_CfgDT(M0P_ADTIM_TypeDef *ADTx,
                         boolean_t bDTEn,
                         boolean_t bEqual);
//�����ж�
en_result_t Adt_CfgIrq(M0P_ADTIM_TypeDef *ADTx,
                          en_adt_irq_type_t enAdtIrq,
                          boolean_t bEn);
//��ȡ�жϱ�־
boolean_t Adt_GetIrqFlag(M0P_ADTIM_TypeDef *ADTx,
                         en_adt_irq_type_t enAdtIrq);
//����жϱ�־
en_result_t Adt_ClearIrqFlag(M0P_ADTIM_TypeDef *ADTx,
                             en_adt_irq_type_t enAdtIrq);
//��������жϱ�־
en_result_t Adt_ClearAllIrqFlag(M0P_ADTIM_TypeDef *ADTx);
//Z��������������
en_result_t Adt_CfgZMask(M0P_ADTIM_TypeDef *ADTx, 
                            stc_adt_zmask_cfg_t* pstcAdtZMaskCfg);

//@} // ADT Group

#ifdef __cplusplus
}
#endif

#endif /* __HC_ADT_H__ */
/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
