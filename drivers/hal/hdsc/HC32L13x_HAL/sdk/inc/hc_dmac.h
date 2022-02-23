/*****************************************************************************
* Copyright (C) 2016, Huada Semiconductor Co.,Ltd All rights reserved.
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
/** \file dma.h
**
** A detailed description is available at
** @link DmacGroup Dmac description @endlink
**
**   - 2018-03-09  1.0  Hongjh First version for Device Driver Library of Dmac.
**
******************************************************************************/
#ifndef __HC_DMAC_H__
#define __HC_DMAC_H__

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
  ** \defgroup DmacGroup Direct Memory Access Control(DMAC)
  **
  ******************************************************************************/
  //@{
  
  /*******************************************************************************
  * Global type definitions ('typedef')
  ******************************************************************************/
  /**
  *******************************************************************************
  ** \brief DMA ??
  **
  ******************************************************************************/
  typedef enum en_dma_channel
  {
    DmaCh0                          = 0x00,    ///< DMA ??0
    DmaCh1                          = 0x04,    ///< DMA ??1 
  } en_dma_channel_t;
  /**
  *******************************************************************************
  ** \brief DMA ???
  **
  ******************************************************************************/
  typedef enum en_dma_priority
  {
    DmaMskPriorityFix                          = 0x00000000,    ///< DMA ???????? (CH0>CH1)
    DmaMskPriorityLoop                         = 0x10000000,    ///< DMA ????????????
  } en_dma_priority_t;
  
  /**
  *******************************************************************************
  ** \brief DMA ??????
  **
  ******************************************************************************/
  typedef enum en_dma_transfer_width
  {
    DmaMsk8Bit                         = 0x00000000,    ///< 8 bit  ????
    DmaMsk16Bit                        = 0x04000000,    ///< 16 bit ????
    DmaMsk32Bit                        = 0x08000000     ///< 32 bit ???
  } en_dma_transfer_width_t;
  
  /**
  *******************************************************************************
  ** \brief DMA ????:?(Block)??????(Burst)??
  **
  ******************************************************************************/
  typedef enum en_dma_transfer_mode
  {
    DmaMskBlock                        = 0x00000000,    ///< ?(Block)??
    DmaMskBurst                        = 0x10000000,    ///< ??(Burst)??
  } en_dma_transfer_mode_t;
  
  /**
  *******************************************************************************
  ** \brief DMA??????
  **
  ******************************************************************************/
  typedef enum en_dma_stat
  {
    DmaDefault                                = 0U,    ///< ???
    DmaAddOverflow                                    = 1U,    ///< ????????(????)
    DmaHALT                         = 2U,    ///< ??????????(?????????????EB/DE????????)
    DmaAccSCRErr                    = 3U,    ///< ????????(?????????)
    DmaAccDestErr                   = 4U,    ///< ????????(??????????)
    DmaTransferComplete             = 5U,    ///< ??????
    DmaTransferPause                = 7U,    ///< ????      
  } en_dma_stat_t;
  
  /**
  *******************************************************************************
  ** \brief DMA???????:?????
  **
  ******************************************************************************/
  typedef enum en_src_address_mode
  {    
    DmaMskSrcAddrInc                 = 0x00000000,    ///< ????
    DmaMskSrcAddrFix                 = 0x02000000,    ///< ????
  } en_src_address_mode_t;
  
  /**
  *******************************************************************************
  ** \brief DMA????????:?????
  **
  ******************************************************************************/
  typedef enum en_dst_address_mode
  {    
    DmaMskDstAddrInc                 = 0x00000000,    ///< ????
    DmaMskDstAddrFix                 = 0x01000000,    ///< ????
  } en_dst_address_mode_t;
  
  /**
  *******************************************************************************
  ** \brief DMA CONFA:BC[3:0]?CONFA:TC[15:0]?????
  **
  ******************************************************************************/
  typedef enum en_bc_tc_reload_mode
  {    
    DmaMskBcTcReloadDisable                = 0x00000000,    ///< ????
    DmaMskBcTcReloadEnable                 = 0x00800000,    ///< ????
  } en_bc_tc_reload_mode_t;
  
  /**
  *******************************************************************************
  ** \brief DMA???????:?????
  **
  ******************************************************************************/
  typedef enum en_src_address_reload_mode
  {    
    DmaMskSrcAddrReloadDisable                = 0x00000000,    ///< ??DMA?????
    DmaMskSrcAddrReloadEnable                 = 0x00400000,    ///< ??DMA?????
  } en_src_address_reload_mode_t;
  
  /**
  *******************************************************************************
  ** \brief DMA????????:?????
  **
  ******************************************************************************/
  typedef enum en_dst_address_reload_mode
  {    
    DmaMskDstAddrReloadDisable                = 0x00000000,    ///< ??DMA??????
    DmaMskDstAddrReloadEnable                 = 0x00200000,    ///< ??DMA??????
  } en_dst_address_reload_mode_t;

  
  
  /**
  *******************************************************************************
  ** \brief DMA ??????
  **
  ******************************************************************************/
  typedef enum en_dma_msk
  {
    DmaMskOneTransfer                      = 0x00000000,    ///< ????,DMAC???????CONFA:ENS?
    DmaMskContinuousTransfer               = 0x00000001,    ///< ????,DMAC????????CONFA:ENS?
  } en_dma_msk_t;
  /**
  *******************************************************************************
  ** \brief DMA ?????
  **
  ******************************************************************************/  
  typedef enum stc_dma_trig_sel
  {
    DmaSWTrig                         = 0U,     ///< Select DMA software trig 
    DmaSPI0RXTrig                     = 32U,    ///< Select DMA hardware trig 0
    DmaSPI0TXTrig                     = 33U,    ///< Select DMA hardware trig 1
    DmaSPI1RXTrig                     = 34U,    ///< Select DMA hardware trig 2
    DmaSPI1TXTrig                     = 35U,    ///< Select DMA hardware trig 3
    DmaADCJQRTrig                     = 36U,    ///< Select DMA hardware trig 4
    DmaADCSQRTrig                     = 37U,    ///< Select DMA hardware trig 5
    DmaLCDTxTrig                      = 38U,    ///< Select DMA hardware trig 6
    DmaUart0RxTrig                    = 40U,    ///< Select DMA hardware trig 8
    DmaUart0TxTrig                    = 41U,    ///< Select DMA hardware trig 9             
    DmaUart1RxTrig                    = 42U,    ///< Select DMA hardware trig 10
    DmaUart1TxTrig                    = 43U,    ///< Select DMA hardware trig 11
    DmaLpUart0RxTrig                  = 44U,    ///< Select DMA hardware trig 12
    DmaLpUart0TxTrig                  = 45U,    ///< Select DMA hardware trig 13
    DmaLpUart1RxTrig                  = 46U,    ///< Select DMA hardware trig 14
    DmaLpUart1TxTrig                  = 47U,    ///< Select DMA hardware trig 15
    DmaTIM0ATrig                      = 50U,    ///< Select DMA hardware trig 18
    DmaTIM0BTrig                      = 51U,    ///< Select DMA hardware trig 19
    DmaTIM1ATrig                      = 52U,    ///< Select DMA hardware trig 20
    DmaTIM1BTrig                      = 53U,    ///< Select DMA hardware trig 21
    DmaTIM2ATrig                      = 54U,    ///< Select DMA hardware trig 22   
    DmaTIM2BTrig                      = 55U,    ///< Select DMA hardware trig 23   
    DmaTIM3ATrig                      = 56U,    ///< Select DMA hardware trig 24    
    DmaTIM3BTrig                      = 57U,    ///< Select DMA hardware trig 25   
    DmaTIM4ATrig                      = 58U,    ///< Select DMA hardware trig 26    
    DmaTIM4BTrig                      = 59U,    ///< Select DMA hardware trig 27   
    DmaTIM5ATrig                      = 60U,    ///< Select DMA hardware trig 28    
    DmaTIM5BTrig                      = 61U,    ///< Select DMA hardware trig 29   
    DmaTIM6ATrig                      = 62U,    ///< Select DMA hardware trig 30    
    DmaTIM6BTrig                      = 63U,    ///< Select DMA hardware trig 31   
  }en_dma_trig_sel_t;  
  
  /**
  *******************************************************************************
  ** \brief DMA??????????
  **
  ******************************************************************************/
  typedef struct stc_dma_cfg
  {
    en_dma_transfer_mode_t enMode;
    
    uint16_t u16BlockSize;                           ///< ?????
    uint16_t u16TransferCnt;                         ///< ?????
    en_dma_transfer_width_t enTransferWidth;         ///< ???????? ????????:en_dma_transfer_width_t
    
    en_src_address_mode_t enSrcAddrMode;             ///< DMA???????:?????
    en_dst_address_mode_t enDstAddrMode;             ///< DMA????????:?????  
    
    en_src_address_reload_mode_t enSrcAddrReloadCtl; ///< ?????  ????????:en_src_address_reload_mode_t
    en_dst_address_reload_mode_t enDestAddrReloadCtl;///< ?????? ????????:en_dst_address_reload_mode_t
    en_bc_tc_reload_mode_t enSrcBcTcReloadCtl;       ///< Bc/Tc????? ????????:en_bc_tc_reload_mode_t
        
    uint32_t u32SrcAddress;                          ///< ???>
    uint32_t u32DstAddress;                          ///< ????>
        
    en_dma_msk_t enTransferMode;                     ///DMA ?????? ????????:en_dma_msk_t
    en_dma_priority_t enPriority;                    ///DMA ????? ????????:en_dma_priority_t
    en_dma_trig_sel_t enRequestNum;                  ///<DMA ????? ????????:en_dma_trig_sel_t
  } stc_dma_cfg_t;

  /*******************************************************************************
  * Global pre-processor symbols/macros ('#define')
  ******************************************************************************/
  
  /*******************************************************************************
  * Global variable definitions ('extern')
  ******************************************************************************/
  
  /*******************************************************************************
  * Global function prototypes (definition in C source)
  ******************************************************************************/
  ///< ???DMAC??
  en_result_t Dma_InitChannel(en_dma_channel_t enCh, stc_dma_cfg_t* pstcCfg);  
    
  ///< DMA??????,?????????,????????????
  void Dma_Enable(void);   
  ///< DMA????????,????????.
  void Dma_Disable(void);  
  
  ///< ????DMA????????.
  void Dma_SwStart(en_dma_channel_t enCh);  
  ///< ????DMA????????.
  void Dma_SwStop(en_dma_channel_t enCh);

  ///< ????dma???(????)??.
  void Dma_EnableChannelIrq(en_dma_channel_t enCh);
  ///< ????dma???(????)??.
  void Dma_DisableChannelIrq(en_dma_channel_t enCh);
  ///< ????dma???(????)??..
  void Dma_EnableChannelErrIrq(en_dma_channel_t enCh);
  ///< ????dma???(????)??..
  void Dma_DisableChannelErrIrq(en_dma_channel_t enCh);
  
  ///< ????dma??
  void Dma_EnableChannel(en_dma_channel_t enCh);
  ///< ????dma??
  void Dma_DisableChannel(en_dma_channel_t enCh);
  
  ///< ????????(Block)??
  void Dma_SetBlockSize(en_dma_channel_t enCh, uint16_t u16BlkSize);
  ///< ???????(Block)????
  void Dma_SetTransferCnt(en_dma_channel_t enCh, uint16_t u16TrnCnt);
  
  ///< ???????????,?DMAC?????????CONFA:ENS?.
  void Dma_EnableContinusTranfer(en_dma_channel_t enCh);
  ///< ??????????,?DMAC????????.
  void Dma_DisableContinusTranfer(en_dma_channel_t enCh);

  ///< ????dma??.
  void Dma_HaltTranfer(void);
  ///< ??(?????)??dma??.
  void Dma_RecoverTranfer(void);
  ///< ????dma??.
  void Dma_PauseChannelTranfer(en_dma_channel_t enCh);
  ///< ??(?????)??dma??.
  void Dma_RecoverChannelTranfer(en_dma_channel_t enCh);

  ///< ????????????.
  void Dma_SetTransferWidth(en_dma_channel_t enCh, en_dma_transfer_width_t enWidth);  
  ///< ??dma?????.
  void Dma_SetChPriority(en_dma_priority_t enPrio);
  
  ///< ????DMA?????.
  en_dma_stat_t Dma_GetStat(en_dma_channel_t enCh);  
  ///< ????DMA??????.
  void Dma_ClrStat(en_dma_channel_t enCh);
  
  ///<?????????
  void Dma_SetSourceAddress(en_dma_channel_t enCh, uint32_t u32Address);
  ///<??????????.
  void Dma_SetDestinationAddress(en_dma_channel_t enCh, uint32_t u32Address);
  //@} // DmacGroup
  
#ifdef __cplusplus
}
#endif

#endif /* __HC_DMAC_H__ */

/*******************************************************************************
* EOF (not truncated)
******************************************************************************/
