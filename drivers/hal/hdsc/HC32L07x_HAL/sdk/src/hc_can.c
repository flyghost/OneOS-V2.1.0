/*******************************************************************************
* Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co., Ltd. ("HDSC").
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
* REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
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
/** \file hc_can.c
 **
 ** A detailed description is available at
 ** @link CanGroup CAN description @endlink
 **
 **   - 2018-12-13  1.0  Lux First version for Device Driver Library of CAN.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_can.h"

/**
 *******************************************************************************
 ** \addtogroup CanGroup
 ******************************************************************************/
//@{

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define CAN_RESET_ENABLE()                  M0P_CAN->CFG_STAT_f.RESET = 1;
#define CAN_RESET_DISABLE()                 M0P_CAN->CFG_STAT_f.RESET = 0;\
                                            while(M0P_CAN->CFG_STAT_f.RESET){;}

#define CAN_ACF_ID_REG_SEL                  ((uint8_t)0x00)
#define CAN_ACF_MASK_REG_SEL                ((uint8_t)0x01)


/*! Parameter validity check for CAN Mode \a CanMode. */
#define IS_CAN_MODE_VALID(CanMode)                                             \
(       (CanExternalLoopBackMode  == (CanMode)) ||                               \
        (CanInternalLoopBackMode  == (CanMode)) ||                               \
        (CanTxSignalPrimaryMode   == (CanMode)) ||                               \
        (CanTxSignalSecondaryMode == (CanMode)) ||                               \
        (CanListenOnlyMode        == (CanMode))                                  \
)

/*! Parameter validity check for CAN Tx Cmd \a TxCmd. */
#define IS_TX_CMD_VALID(TxCmd)                                                 \
(       (CanPTBTxCmd      == (TxCmd)) ||                                         \
        (CanPTBTxAbortCmd == (TxCmd)) ||                                         \
        (CanSTBTxOneCmd   == (TxCmd)) ||                                         \
        (CanSTBTxAllCmd   == (TxCmd)) ||                                         \
        (CanSTBTxAbortCmd == (TxCmd))                                            \
)

/*! Parameter validity check for CAN status \a enCanStatus. */
#define IS_CAN_STATUS_VALID(enCanStatus)                                       \
(       (CanRxActive == (enCanStatus)) ||                                        \
        (CanTxActive == (enCanStatus)) ||                                        \
        (CanBusoff   == (enCanStatus))                                           \
)

/*! Parameter validity check for CAN Irq type \a enCanIrqType. */
#define IS_CAN_IRQ_TYPE_VALID(enCanIrqType)                                    \
(       (CanRxIrqEn              == (enCanIrqType)) ||                           \
        (CanRxOverIrqEn          == (enCanIrqType)) ||                           \
        (CanRxBufFullIrqEn       == (enCanIrqType)) ||                           \
        (CanRxBufAlmostFullIrqEn == (enCanIrqType)) ||                           \
        (CanTxPrimaryIrqEn       == (enCanIrqType)) ||                           \
        (CanTxSecondaryIrqEn     == (enCanIrqType)) ||                           \
        (CanErrorIrqEn           == (enCanIrqType)) ||                           \
        (CanErrorPassiveIrqEn    == (enCanIrqType)) ||                           \
        (CanArbiLostIrqEn        == (enCanIrqType)) ||                           \
        (CanBusErrorIrqEn        == (enCanIrqType))                              \
)

/*! Parameter validity check for CAN Irq flag type \a enCanIrqFLg. */
#define IS_CAN_IRQ_FLAG_VALID(enCanIrqFLg)                                     \
(       (CanTxBufFullIrqFlg        == (enCanIrqFLg)) ||                          \
        (CanRxIrqFlg               == (enCanIrqFLg)) ||                          \
        (CanRxOverIrqFlg           == (enCanIrqFLg)) ||                          \
        (CanRxBufFullIrqFlg        == (enCanIrqFLg)) ||                          \
        (CanRxBufAlmostFullIrqFlg  == (enCanIrqFLg)) ||                          \
        (CanTxPrimaryIrqFlg        == (enCanIrqFLg)) ||                          \
        (CanTxSecondaryIrqFlg      == (enCanIrqFLg)) ||                          \
        (CanErrorIrqFlg            == (enCanIrqFLg)) ||                          \
        (CanAbortIrqFlg            == (enCanIrqFLg)) ||                          \
        (CanErrorWarningIrqFlg     == (enCanIrqFLg)) ||                          \
        (CanErrorPassivenodeIrqFlg == (enCanIrqFLg)) ||                          \
        (CanErrorPassiveIrqFlg     == (enCanIrqFLg)) ||                          \
        (CanArbiLostIrqFlg         == (enCanIrqFLg)) ||                          \
        (CanBusErrorIrqFlg         == (enCanIrqFLg))                             \
)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  CAN 初始化
 **
 ** \param  [in] pstcCanInitCfg      @ref stc_can_init_config_t.
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_Init(stc_can_init_config_t *pstcCanInitCfg)
{
    ASSERT(NULL != pstcCanInitCfg);

    CAN_RESET_ENABLE();

    M0P_CAN->BT_f.PRESC = pstcCanInitCfg->stcCanBt.PRESC;
    M0P_CAN->BT_f.SEG_1 = pstcCanInitCfg->stcCanBt.SEG_1;
    M0P_CAN->BT_f.SEG_2 = pstcCanInitCfg->stcCanBt.SEG_2;
    M0P_CAN->BT_f.SJW   = pstcCanInitCfg->stcCanBt.SJW;

    CAN_RESET_DISABLE();
    
    
    M0P_CAN->LIMIT_f.AFWL = pstcCanInitCfg->stcWarningLimit.CanWarningLimitVal;
    M0P_CAN->LIMIT_f.EWL  = pstcCanInitCfg->stcWarningLimit.CanErrorWarningLimitVal;
    
    M0P_CAN->TCTRL_f.TSMODE = pstcCanInitCfg->enCanSTBMode;
    
    M0P_CAN->RCTRL_f.RBALL  = pstcCanInitCfg->enCanRxBufAll;
    M0P_CAN->RCTRL_f.ROM    = pstcCanInitCfg->enCanRxBufMode;
    M0P_CAN->RTIE = 0x00;
}

/**
 *******************************************************************************
 ** \brief  CAN 去初始化 (RESET CAN register)
 **
 ** \param  None
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_DeInit(void)
{
    CAN_RESET_ENABLE();
}

/**
 *******************************************************************************
 ** \brief  CAN 模式配置
 **
 ** \param  [in] enMode                 模式. @ref en_can_mode_t     
 ** \param  [in] enCanSAck              自应答 @ref en_can_self_ack_en_t
 ** \param  [in] enNewState 
 ** \arg    Enable                      使能.
 ** \arg    Disable                     禁止.
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_ModeConfig(en_can_mode_t enMode, en_can_self_ack_en_t enCanSAck, boolean_t enNewState)
{
    ASSERT(IS_CAN_MODE_VALID(enMode));
    ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    if(CanListenOnlyMode == enMode)
    {
        M0P_CAN->TCMD_f.LOM = enNewState;
    }else
    {
        if(TRUE == enNewState)
        {
            M0P_CAN->CFG_STAT |= enMode;
        }else
        {
            M0P_CAN->CFG_STAT &= ~(uint32_t)enMode;
        }
    }
    
    M0P_CAN->RCTRL_f.SACK = enCanSAck;

}


/**
 *******************************************************************************
 ** \brief  CAN 筛选器配置
 **
 ** \param  [in] pstcFilter             @ref stc_can_filter_t.
 **
 ** \param  [in] enNewState             
 ** \arg    Enable                      使能.
 ** \arg    Disable                     禁止.
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_FilterConfig(stc_can_filter_t *pstcFilter, boolean_t enNewState)
{
    ASSERT(NULL != pstcFilter);
    ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    CAN_RESET_ENABLE();

    //<<Acceptance filter address
    M0P_CAN->ACFCTRL_f.ACFADR  = pstcFilter->enFilterSel;

    //<<ID config
    M0P_CAN->ACFCTRL_f.SELMASK = CAN_ACF_ID_REG_SEL;
    M0P_CAN->ACF               = pstcFilter->u32CODE;

    //<<MASK config
    M0P_CAN->ACFCTRL_f.SELMASK = CAN_ACF_MASK_REG_SEL;
    M0P_CAN->ACF               = pstcFilter->u32MASK;

    //<<Frame format config
    M0P_CAN->ACF_f.AIDEE = ((pstcFilter->enAcfFormat >> 1) & 0x01u);
    M0P_CAN->ACF_f.AIDE  = (pstcFilter->enAcfFormat & 0x01);

    if(TRUE == enNewState)
    {
        M0P_CAN->ACFEN |= 1u << pstcFilter->enFilterSel;
    }else
    {
        M0P_CAN->ACFEN &= ~(1u << pstcFilter->enFilterSel);
    }

    CAN_RESET_DISABLE();
}


/**
 *******************************************************************************
 ** \brief  CAN 发送数据帧设置
 **
 ** \param  [in] pstcTxFrame            @ref stc_can_txframe_t.
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_SetFrame(stc_can_txframe_t *pstcTxFrame)
{
    ASSERT(NULL != pstcTxFrame);

    M0P_CAN->TCMD_f.TBSEL = pstcTxFrame->enBufferSel;
    M0P_CAN->TBUF0 = pstcTxFrame->TBUF32_0;
    M0P_CAN->TBUF1 = pstcTxFrame->TBUF32_1;
    M0P_CAN->TBUF2 = pstcTxFrame->TBUF32_2[0];
    M0P_CAN->TBUF3 = pstcTxFrame->TBUF32_2[1];

    if(CanSTBSel == pstcTxFrame->enBufferSel)
    {
        M0P_CAN->TCTRL_f.TSNEXT = TRUE;
    }

}

/**
 *******************************************************************************
 ** \brief  CAN 数据帧发送命令
 **
 ** \param  [in] enTxCmd            @ref en_can_tx_cmd_t.
 **
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_TransmitCmd(en_can_tx_cmd_t enTxCmd)
{
    ASSERT(IS_TX_CMD_VALID(enTxCmd));

    M0P_CAN->TCMD |= enTxCmd;

}

/**
 *******************************************************************************
 ** \brief  CAN 发送数据缓冲器状态获取
 **
 ** \retval Can Tx buffer status    @ref en_can_tx_buf_status_t
 **
 ** \note   None
 **
 ******************************************************************************/
en_can_tx_buf_status_t CAN_TxBufStatusGet(void)
{
    return (en_can_tx_buf_status_t)M0P_CAN->TCTRL_f.TSSTAT;
}

/**
 *******************************************************************************
 ** \brief  CAN 数据帧接收
 **
 ** \param  [in] pstcRxFrame        @ref stc_can_rxframe_t.
 **                                 
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_Receive(stc_can_rxframe_t *pstcRxFrame)
{
    ASSERT(NULL != pstcRxFrame);

    pstcRxFrame->RBUF32_0    = M0P_CAN->RBUF0;
    pstcRxFrame->RBUF32_1    = M0P_CAN->RBUF1;
    pstcRxFrame->RBUF32_2[0] = M0P_CAN->RBUF2;
    pstcRxFrame->RBUF32_2[1] = M0P_CAN->RBUF3;

    M0P_CAN->RCTRL_f.RREL = 1;

}

/**
 *******************************************************************************
 ** \brief  CAN 接收数据缓冲器状态获取
 **
 **                                 
 ** \retval Can rx buffer status    @ref en_can_rx_buf_status_t
 **
 ** \note   None
 **
 ******************************************************************************/
en_can_rx_buf_status_t CAN_RxBufStatusGet(stc_can_rxframe_t *pstcRxFrame)
{
    return (en_can_rx_buf_status_t)M0P_CAN->RCTRL_f.RSSTAT;
}

/**
 *******************************************************************************
 ** \brief  CAN 错误状态获取
 **
 ** \param  None
 **
 ** \retval en_can_error_t          错误状态
 **
 ** \note   None
 **
 ******************************************************************************/
en_can_error_t CAN_ErrorStatusGet(void)
{
    if(6 > M0P_CAN->EALCAP_f.KOER)
    {
        return (en_can_error_t)M0P_CAN->EALCAP_f.KOER;
    }else
    {
        return UNKOWN_ERROR;
    }

}

/**
 *******************************************************************************
 ** \brief  CAN 通信状态获取
 **
 ** \param  enCanStatus             CAN 通信状态
 ** \arg    true
 ** \arg    false
 ** \retval boolean_t
 **
 ** \note   None
 **
 ******************************************************************************/
boolean_t CAN_StatusGet(en_can_status_t enCanStatus)
{
    ASSERT(IS_CAN_STATUS_VALID(enCanStatus));

    if(M0P_CAN->CFG_STAT & enCanStatus)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}

/**
 *******************************************************************************
 ** \brief  CAN 中断使能
 **
 ** \param  [in] enCanIrqType           @ref en_can_irq_type_t.
 ** \param  [in] enNewState             
 ** \arg    Enable                      使能.
 ** \arg    Disable                     禁止.
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_IrqCmd(en_can_irq_type_t enCanIrqType, boolean_t enNewState)
{
    volatile uint32_t *u32pIE;

    ASSERT(IS_CAN_IRQ_TYPE_VALID(enCanIrqType));
    ASSERT(IS_FUNCTIONAL_STATE(enNewState));

    u32pIE = (volatile uint32_t*)(&M0P_CAN->RTIE);

    if(TRUE == enNewState)
    {
        *u32pIE |= (uint32_t)enCanIrqType;
    }else
    {
        *u32pIE &= ~(uint32_t)enCanIrqType;
    }

}

/**
 *******************************************************************************
 ** \brief  CAN 中断标志获取
 **
 ** \param  [in] enCanIrqFlgType            @ref en_can_irq_flag_type_t.
 **
 ** \retval boolean_t TRUE or FALSE
 **
 ** \note   None
 **
 ******************************************************************************/
boolean_t CAN_IrqFlgGet(en_can_irq_flag_type_t enCanIrqFlgType)
{
    volatile uint32_t *u32pIE = NULL;

    ASSERT(IS_CAN_IRQ_FLAG_VALID(enCanIrqFlgType));

    u32pIE = (volatile uint32_t*)(&M0P_CAN->RTIE);

    if( *u32pIE & (uint32_t)enCanIrqFlgType)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }

}

/**
 *******************************************************************************
 ** \brief  CAN 中断标志清除
 **
 ** \param  [in] enCanIrqFlgType           @ref en_can_irq_flag_type_t.
 **
 ** \retval None
 **
 ** \note   None
 **
 ******************************************************************************/
void CAN_IrqFlgClr(en_can_irq_flag_type_t enCanIrqFlgType)
{
    volatile uint32_t *u32pIE = NULL;
    uint32_t u32IETempMsk = 0xFF2A00FF;
    
    
    ASSERT(IS_CAN_IRQ_FLAG_VALID(enCanIrqFlgType));

    u32pIE = (volatile uint32_t*)(&M0P_CAN->RTIE);

    *u32pIE = (((*u32pIE)&u32IETempMsk) | (uint32_t)enCanIrqFlgType);

}


/**
 *******************************************************************************
 ** \brief  CAN 接收错误计数值获取
 **
 ** \param  None
 **
 ** \retval Error Counter(0~255)
 **
 ** \note   None
 **
 ******************************************************************************/
uint8_t CAN_RxErrorCntGet(void)
{
    return M0P_CAN->RECNT;
}

/**
 *******************************************************************************
 ** \brief  CAN 发送错误计数值获取
 **
 ** \param  None
 **
 ** \retval Error Counter(0~255)
 **
 ** \note   None
 **
 ******************************************************************************/
uint8_t CAN_TxErrorCntGet(void)
{
    return M0P_CAN->TECNT;
}

/**
 *******************************************************************************
 ** \brief  CAN 仲裁捕获
 **
 ** \param  None
 **
 ** \retval address(0~31)
 **
 ** \note   None
 **
 ******************************************************************************/
uint8_t CAN_ArbitrationLostCap(void)
{
    return M0P_CAN->EALCAP_f.ALC;
}


//@} // CanGroup

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/

