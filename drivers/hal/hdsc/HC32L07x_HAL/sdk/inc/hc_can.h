/*****************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd All rights reserved.
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
/** \file hc_can.h
 **
 ** A detailed description is available at
 ** @link CanGroup CAN description @endlink
 **
 **   - 2018-11-27  1.0  Lux First version for Device Driver Library of CAN
 **
 ******************************************************************************/
#ifndef __HC_CAN_H__
#define __HC_CAN_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *******************************************************************************
 ** \defgroup CanGroup Controller Area Network(CAN)
 **
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief  CAN 错误类型.
 ******************************************************************************/
typedef enum
{
    NO_ERROR        = 0U,       ///< 无错误
    BIT_ERROR       = 1U,       ///< 位错误
    FORM_ERROR      = 2U,       ///< 形式错误
    STUFF_ERROR     = 3U,       ///< 填充错误
    ACK_ERROR       = 4U,       ///< 应答错误
    CRC_ERROR       = 5U,       ///< CRC错误
    UNKOWN_ERROR    = 6U,       ///< 未知错误
}en_can_error_t;

/**
 *******************************************************************************
 ** \brief  CAN传输缓冲器选择. (TCMD)
 ******************************************************************************/
typedef enum
{
    CanPTBSel       = 0U,        ///< 主缓冲器选择
    CanSTBSel       = 1U,        ///< 副缓冲器选择
}en_can_buffer_sel_t;

/**
 *******************************************************************************
 ** \brief  CAN 警告限定.(AFWL)
 ******************************************************************************/
typedef struct stc_can_warning_limit
{
    uint8_t CanWarningLimitVal;         ///< 接收缓冲器将满限定
    uint8_t CanErrorWarningLimitVal;    ///< 错误警告限定
}stc_can_warning_limit_t;

/**
 *******************************************************************************
 ** \brief  筛选器类型.(ACF)
 ******************************************************************************/
typedef enum en_can_acf_format_en
{
    CanStdFrames        = 0x02u,   ///< 仅接受标准帧
    CanExtFrames        = 0x03u,   ///< 仅接受扩展帧
    CanAllFrames        = 0x00u,   ///< 接收标准帧和扩展帧
}en_can_acf_format_en_t;

/**
 *******************************************************************************
 ** \brief  筛选器组使能. (ACFEN)
 ******************************************************************************/
typedef enum en_can_filter_sel
{
    CanFilterSel1        = 0u,   ///< 筛选器组1使能
    CanFilterSel2        = 1u,   ///< 筛选器组2使能
    CanFilterSel3        = 2u,   ///< 筛选器组3使能
    CanFilterSel4        = 3u,   ///< 筛选器组4使能
    CanFilterSel5        = 4u,   ///< 筛选器组5使能
    CanFilterSel6        = 5u,   ///< 筛选器组6使能
    CanFilterSel7        = 6u,   ///< 筛选器组7使能
    CanFilterSel8        = 7u,   ///< 筛选器组8使能
}en_can_filter_sel_t;

/**
 *******************************************************************************
 ** \brief  CAN 收发中断使能.(IE)
 ******************************************************************************/
typedef enum
{
    //<<Can Rx or Tx Irq En
    CanRxIrqEn              = 0x00000080,   ///< 接收中断使能
    CanRxOverIrqEn          = 0x00000040,   ///< 接收上溢中断使能
    CanRxBufFullIrqEn       = 0x00000020,   ///< 接收缓冲器满中断使能
    CanRxBufAlmostFullIrqEn = 0x00000010,   ///< 接收缓冲器将满中断使能
    CanTxPrimaryIrqEn       = 0x00000008,   ///< PTB发送中断使能
    CanTxSecondaryIrqEn     = 0x00000004,   ///< STB发送中断使能
    CanErrorIrqEn           = 0x00000002,   ///< 错误中断使能

    //<<Can Error Irq En
    CanErrorPassiveIrqEn    = 0x00200000,   ///< 错误被动中断使能
    CanArbiLostIrqEn        = 0x00080000,   ///< 仲裁失败中断使能
    CanBusErrorIrqEn        = 0x00020000,   ///< 总线错误中断使能

}en_can_irq_type_t;

/**
 *******************************************************************************
 ** \brief  CAN中断标志.(IF)
 ******************************************************************************/
typedef enum
{
    CanTxBufFullIrqFlg          = 0x00000001,   ///< 发送缓冲器满标志
    CanRxIrqFlg                 = 0x00008000,   ///< 接收中断标志
    CanRxOverIrqFlg             = 0x00004000,   ///< 接收上溢中断标志
    CanRxBufFullIrqFlg          = 0x00002000,   ///< 接收缓冲器满中断标志
    CanRxBufAlmostFullIrqFlg    = 0x00001000,   ///< 接收缓冲器将满中断标志
    CanTxPrimaryIrqFlg          = 0x00000800,   ///< PTB发送中断标志
    CanTxSecondaryIrqFlg        = 0x00000400,   ///< STB发送中断标志
    CanErrorIrqFlg              = 0x00000200,   ///< 错误中断标志
    CanAbortIrqFlg              = 0x00000100,   ///< 取消发送中断标志

    CanErrorWarningIrqFlg       = 0x00800000,   ///< 达到错误限定警告标志
    CanErrorPassivenodeIrqFlg   = 0x00400000,   ///< 错误被动节点标志
    CanErrorPassiveIrqFlg       = 0x00100000,   ///< 错误被动中断标志
    CanArbiLostIrqFlg           = 0x00040000,   ///< 仲裁失败中断标志
    CanBusErrorIrqFlg           = 0x00010000,   ///< 总线错误中断标志
}en_can_irq_flag_type_t;

/**
 *******************************************************************************
 ** \brief  CAN 模式.(CFG_STAT)
 ******************************************************************************/
typedef enum
{
    CanExternalLoopBackMode  = 0x40u,        ///< 外部回环模式
    CanInternalLoopBackMode  = 0x20u,        ///< 内部回环模式
    CanTxSignalPrimaryMode   = 0x10u,        ///< PTB单次传输模式
    CanTxSignalSecondaryMode = 0x08u,        ///< STB单次传输模式
    CanListenOnlyMode        = 0xFFu,        ///< 静默模式
}en_can_mode_t;

/**
 *******************************************************************************
 ** \brief  CAN 状态.(STAT)
 ******************************************************************************/
typedef enum
{
    CanRxActive = 0x04,        ///< 接收中状态
    CanTxActive = 0x02,        ///< 发送中状态
    CanBusoff   = 0x01,        ///< 总线关闭状态
}en_can_status_t;

/**
 *******************************************************************************
 ** \brief  CAN 发送命令.(TCMD)
 ******************************************************************************/
typedef enum
{
    CanPTBTxCmd      = 0x10,        ///< PTB发送命令
    CanPTBTxAbortCmd = 0x08,        ///< PTB发送取消命令
    CanSTBTxOneCmd   = 0x04,        ///< STB单帧发送命令
    CanSTBTxAllCmd   = 0x02,        ///< STB所有帧命令
    CanSTBTxAbortCmd = 0x01,        ///< STB发送取消命令
}en_can_tx_cmd_t;

/**
 *******************************************************************************
 ** \brief  CAN 副缓冲器发送模式选择.(TCTRL)
 ******************************************************************************/
typedef enum
{
    CanSTBFifoMode    = 0,        ///< FIFO模式
    CanSTBPrimaryMode = 1,        ///< 优先级模式
}en_can_stb_mode_t;

/**
 *******************************************************************************
 ** \brief  CAN 自应答.(RCTRL)
 ******************************************************************************/
typedef enum
{
    CanSelfAckDisable = 0,        ///< 无自应答
    CanSelfAckEnable  = 1,        ///< 使能自应答(LBME=1)
}en_can_self_ack_en_t;

/**
 *******************************************************************************
 ** \brief  接收缓冲器上溢模式.(RCTRL)
 ******************************************************************************/
typedef enum
{
    CanRxBufOverwritten = 0,        ///< 最早接收到的数据被覆盖
    CanRxBufNotStored   = 1,        ///< 最新接收到的数据不被存储
}en_can_rx_buf_mode_en_t;

/**
 *******************************************************************************
 ** \brief  接收缓冲器数据存储模式.(RCTRL)
 ******************************************************************************/
typedef enum
{
    CanRxNormal = 0,        ///< 正常模式
    CanRxAll    = 1,        ///< 存储所有(包括错误)数据
}en_can_rx_buf_all_t;

/**
 *******************************************************************************
 ** \brief  CAN 接收缓冲器状态.(RSTAT)
 ******************************************************************************/
typedef enum
{
    CanRxBufEmpty          = 0,        ///< 空
    CanRxBufnotAlmostFull  = 1,        ///< 非空但小于警告限定值
    CanRxBufAlmostFull     = 2,        ///< 大于警告限定值但未满
    CanRxBufFull           = 3,        ///< 满(上溢)
}en_can_rx_buf_status_t;

/**
 *******************************************************************************
 ** \brief  CAN 发送缓冲器状态.(TSSTAT)
 ******************************************************************************/
typedef enum
{
    CanTxBufEmpty        = 0,       ///< 空
    CanTxBufnotHalfFull  = 1,       ///< 小于等于半满
    CanTxBufHalfFull     = 2,       ///< 大于半满
    CanTxBufFull         = 3,       ///< 满
}en_can_tx_buf_status_t;

/**
 *******************************************************************************
 ** \brief  CAN 筛选器.
 ******************************************************************************/
typedef struct stc_can_filter
{
    uint32_t                u32CODE;        ///< CODE
    uint32_t                u32MASK;        ///< MASK
    en_can_filter_sel_t     enFilterSel;    ///< 筛选器组选择
    en_can_acf_format_en_t  enAcfFormat;    ///< 筛选帧格式.
}stc_can_filter_t;

/**
 *******************************************************************************
 ** \brief  CAN 时序.
 ******************************************************************************/
typedef struct stc_can_bt
{
    uint8_t SEG_1;      ///< 位段1时间(Tseg_1 = (SEG_1 + 2)*TQ)
    uint8_t SEG_2;      ///< 位段2时间(Tseg_2 = (SEG_2 + 1)*TQ)
    uint8_t SJW;        ///< 再同步补偿宽度时间(Tsjw = (SJW + 1)*TQ)
    uint8_t PRESC;      ///< CAN时钟预分频(TQ)
}stc_can_bt_t;

/**
 *******************************************************************************
 ** \brief  CAN 发送数据帧控制.
 ******************************************************************************/
typedef struct
{
    uint32_t DLC                     : 4;        ///< Data length code
    uint32_t RESERVED0               : 2;        ///< Ignore
    uint32_t RTR                     : 1;        ///< Remote transmission request
    uint32_t IDE                     : 1;        ///< IDentifier extension
    uint32_t RESERVED1               : 24;       ///< Ignore
}stc_can_txcontrol_t;

/**
 *******************************************************************************
 ** \brief  CAN 发送数据帧.
 ******************************************************************************/
typedef struct stc_can_txframe
{
    union
    {
        uint32_t TBUF32_0;                  ///< Ignore
        uint32_t StdID;                     ///< Standard ID
        uint32_t ExtID;                     ///< Extended ID
    };
    union
    {
        uint32_t TBUF32_1;                  ///< Ignore
        stc_can_txcontrol_t Control_f;      ///< CAN Tx Control
    };
    union
    {
        uint32_t TBUF32_2[2];               ///< Ignore
        uint8_t  Data[8];                   ///< CAN data
    };
    en_can_buffer_sel_t     enBufferSel;    ///< CAN Tx buffer select

}stc_can_txframe_t;

/**
 *******************************************************************************
 ** \brief  CAN 接收数据帧控制.
 ******************************************************************************/
typedef struct
{
    uint8_t DLC          : 4;       ///< Data length code
    uint8_t RESERVED0    : 2;       ///< Ignore
    uint8_t RTR          : 1;       ///< Remote transmission request
    uint8_t IDE          : 1;       ///< IDentifier extension
}stc_can_rxcontrol_t;

/**
 *******************************************************************************
 ** \brief  CAN接收数据帧状态.
 ******************************************************************************/
typedef struct
{
    uint8_t RESERVED0    : 4;       ///< Ignore
    uint8_t TX           : 1;       ///< TX is set to 1 if the loop back mode is activated
    uint8_t KOER         : 3;       ///< Kind of error
}stc_can_status_t;

/**
 *******************************************************************************
 ** \brief  CAN 数据控制、状态、CYCTIM.
 ******************************************************************************/
typedef struct
{
    stc_can_rxcontrol_t Control_f;      ///< @ref stc_can_rxcontrol_t
    stc_can_status_t    Status_f;       ///< @ref stc_can_status_t
    uint16_t            CycleTime;      ///< TTCAN cycletime
}stc_can_cst_t;

/**
 *******************************************************************************
 ** \brief  CAN 接收数据帧.
 ******************************************************************************/
typedef struct stc_can_rxframe
{
    union
    {
        uint32_t RBUF32_0;              ///< Ignore
        uint32_t StdID;                 ///< Standard ID
        uint32_t ExtID;                 ///< Extended ID
    };
    union
    {
        uint32_t        RBUF32_1;       ///< Ignore
        stc_can_cst_t   Cst;            ///< @ref stc_can_cst_t
    };
    union
    {
        uint32_t RBUF32_2[2];           ///< Ignore
        uint8_t  Data[8];               ///< CAN data
    };

}stc_can_rxframe_t;

/**
 *******************************************************************************
 ** \brief  CAN 初始化配置.
 ******************************************************************************/
typedef struct stc_can_init_config
{
    en_can_rx_buf_all_t     enCanRxBufAll;      ///< @ref en_can_rx_buf_all_t
    en_can_rx_buf_mode_en_t enCanRxBufMode;     ///< @ref en_can_rx_buf_mode_en_t
    en_can_stb_mode_t       enCanSTBMode;       ///< @ref en_can_stb_mode_t
    stc_can_bt_t            stcCanBt;           ///< @ref stc_can_bt_t
    stc_can_warning_limit_t stcWarningLimit;    ///< @ref stc_can_warning_limit_t
}stc_can_init_config_t;


/**
 *******************************************************************************
 ** \brief                         CAN TTCAN
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  TTCAN 缓冲器选择
 ******************************************************************************/
typedef enum
{
    CanTTcanPTBSel      = 0x00u,        ///< PTB
    CanTTcanSTB1Sel     = 0x01u,        ///< STB1
    CanTTcanSTB2Sel     = 0x02u,        ///< STB2
    CanTTcanSTB3Sel     = 0x03u,        ///< STB3
    CanTTcanSTB4Sel     = 0x04u,        ///< STB4
}en_can_ttcan_tbslot_t;

/**
 *******************************************************************************
 ** \brief  TTCAN 计数器预分频
 ******************************************************************************/
typedef enum
{
    CanTTcanTprescDiv1  = 0x00u,        ///< Div1
    CanTTcanTprescDiv2  = 0x01u,        ///< Div2
    CanTTcanTprescDiv3  = 0x02u,        ///< Div3
    CanTTcanTprescDiv4  = 0x03u,        ///< Div4
}en_can_ttcan_Tpresc_t;

/**
 *******************************************************************************
 ** \brief  TTCAN 触发类型
 ******************************************************************************/
typedef enum
{
    CanTTcanImmediate   = 0x00,         ///< 立即触发
    CanTTcanTime        = 0x01,         ///< 时间触发
    CanTTcanSingle      = 0x02,         ///< 单次发送触发
    CanTTcanTransStart  = 0x03,         ///< 发送开始触发
    CanTTcanTransStop   = 0x04,         ///< 发送停止触发
}en_can_ttcan_trigger_type_t;

/**
 *******************************************************************************
 ** \brief  TTCAN 触发中断标志
 ******************************************************************************/
typedef enum
{
    CanTTcanWdtTriggerIrq   = 0x80,       ///< 触发看门中断标志
    CanTTcanErrorTriggerIrq = 0x10,       ///< 触发错误中断标志    
    CanTTcanTimTriggerIrq   = 0x10,       ///< 时间触发中断标志
}en_can_ttcan_irq_type_t;


typedef struct stc_can_ttcan_ref_msg
{
    uint8_t       u8IDE;                ///< Reference message IDE:1-Extended; 0-Standard;
    union                               ///< Reference message ID
    {
        uint32_t RefStdID;              ///< Reference standard ID
        uint32_t RefExtID;              ///< Reference Extended ID
    };
}stc_can_ttcan_ref_msg_t;

typedef struct stc_can_ttcan_trigger_config
{
    en_can_ttcan_tbslot_t       enTbSlot;           ///< Transmit trigger TB slot pointer
    en_can_ttcan_trigger_type_t enTrigType;         ///< Trigger type
    en_can_ttcan_Tpresc_t       enTpresc;           ///< Timer prescaler
    uint8_t                     u8Tew;              ///< Transmit enable window
    uint16_t                    u16TrigTime;        ///< TTCAN trigger time
    uint16_t                    u16WatchTrigTime;   ///< TTCAN watch trigger time register
}stc_can_ttcan_trigger_config_t;


/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/


/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/
///< CAN 初始化配置 
void CAN_Init(stc_can_init_config_t *pstcCanInitCfg);
///< CAN 去初始化
void CAN_DeInit(void);
///< CAN 中断控制
void CAN_IrqCmd(en_can_irq_type_t enCanIrqType, boolean_t enNewState);
///< CAN 中断标志获取
boolean_t CAN_IrqFlgGet(en_can_irq_flag_type_t enCanIrqFlgType);
///< CAN 中断标志清除
void CAN_IrqFlgClr(en_can_irq_flag_type_t enCanIrqFlgType);
///< CAN 模式配置
void CAN_ModeConfig(en_can_mode_t enMode, en_can_self_ack_en_t enCanSAck, boolean_t enNewState);
///< CAN 错误类型获取
en_can_error_t CAN_ErrorStatusGet(void);
///< CAN CAN状态获取
boolean_t CAN_StatusGet(en_can_status_t enCanStatus);
///< CAN 筛选器配置
void CAN_FilterConfig(stc_can_filter_t *pstcFilter, boolean_t enNewState);
///< CAN 发送数据帧配置
void CAN_SetFrame(stc_can_txframe_t *pstcTxFrame);
///< CAN 数据帧传输命令
void CAN_TransmitCmd(en_can_tx_cmd_t enTxCmd);
///< CAN 发送缓冲状态获取
en_can_tx_buf_status_t CAN_TxBufStatusGet(void);
///< CAN 数据帧接收
void CAN_Receive(stc_can_rxframe_t *pstcRxFrame);
///< CAN 接收缓冲状态获取
en_can_rx_buf_status_t CAN_RxBufStatusGet(stc_can_rxframe_t *pstcRxFrame);

///< CAN 仲裁捕获
uint8_t CAN_ArbitrationLostCap(void);
///< CAN 接收错误计数值获取
uint8_t CAN_RxErrorCntGet(void);
///< CAN 发送错误计数值获取
uint8_t CAN_TxErrorCntGet(void);


//<< void CAN_TTCAN_Enable(void);
//<< void CAN_TTCAN_Disable(void);
//<< void CAN_TTCAN_IrqCmd(void);
//<< void CAN_TTCAN_ReferenceMsgSet(stc_can_ttcan_ref_msg_t *pstcRefMsg);
//<< void CAN_TTCAN_TriggerConfig(stc_can_ttcan_trigger_config_t *pstcTriggerCfg);

//@} // CanGroup

#ifdef __cplusplus
}
#endif

#endif /* __HC_CAN_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/

