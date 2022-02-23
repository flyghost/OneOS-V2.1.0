#include "include.h"

#if (CFG_SOC_NAME == SOC_BK7231U)
#include "arm_arch.h"
#include "target_util_pub.h"
#include "mem_pub.h"
#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "phy.h"
#include "bk7011_cal_pub.h"
#include "bk7011_cal.h"
#include <string.h>
#include "flash_pub.h"
#include "cmd_evm.h"
#include "temp_detect_pub.h"
#include "power_save_pub.h"

#if CFG_SUPPORT_CALIBRATION
#define TX_WANTED_POWER_CAL            0
#define TX_IQ_POWER_CAL                1
#define TX_IQ_LOOPBACK_POWER_CAL       2
#define TX_DC_CAL                      0
#define TX_DC_CAL_IQ                   1
#define TX_DC_LOOPBACK_CAL_IQ          2
#define TX_GAIN_IMB_CAL                0
#define TX_GAIN_LOOPBACK_IMB_CAL       1
#define TX_PHASE_IMB_CAL               0
#define TX_PHASE_LOOPBACK_IMB_CAL      1

#define CAL_DEBUG                      1
#include "uart_pub.h"
#if CAL_DEBUG
#define CAL_PRT              null_prf
#define CAL_WARN             null_prf
#define CAL_FATAL            fatal_prf
#define CAL_TIM_PRT          os_printf
#define CAL_FLASH_PRT        os_printf
#else
#define CAL_PRT              null_prf
#define CAL_WARN             null_prf
#define CAL_FATAL            null_prf
#define CAL_TIM_PRT          null_prf
#define CAL_FLASH_PRT        null_prf
#endif

extern void bk7011_cal_pll(void);
void rwnx_cal_set_txpwr_ble(UINT32 pwr_gain);
void rwnx_set_tpc_txpwr_by_tmpdetect(INT16 shift_b, INT16 shift_g);
void rwnx_cal_set_ble_txpwr_by_tmpdetect(INT16 shift_ble);
extern UINT32 ble_in_dut_mode(void);
extern uint8 is_rf_switch_to_ble(void);
extern uint32_t get_ate_mode_state(void);
extern void rwnx_cal_enable_lpfcap_iq_from_digit(UINT32 enable);

static INT32 gtx_dc_n = 0;
static UINT32 gst_rx_adc = CAL_DELAY100US;
static UINT32 gst_sar_adc = CAL_DELAY05US;

#define TRX_REG_0XA_VAL         0x036F2075//0x036A2075
#define TRX_REG_0XB_VAL         0xA7248F37//0x45248437
#define TRX_REG_0XC_VAL         0x0022A755
#define TRX_REG_0XD_VAL         0xDDF90339
#define TSSI_POUT_TH            0x54   ///0x50

#define TXIQ_IMB_TSSI_TH            70
#define TXIQ_IMB_TSSI_TH_LOOPBACK   60
#define TXIQ_TSSI_TH                0x40
#define TRX_REG_0XD_TX_IQ_VAL           0xD9FF0338
#define TRX_REG_0XD_TX_LOOPBACK_IQ_VAL  0xD9FE7FF1
#define TRX_REG_0XC_TXLO_LEAKAGE_VAL    0x5228100

#define TRX_REG_0XC_RXIQ_VAL        0x05228565
#define TRX_REG_0XE_RXIQ_VAL        0xD9F87FF1

#define gi_dc_tx_pa_dgainPA30               7
#define gi_dc_tx_pa_dgainbuf30              6
#define gi_gain_tx_pa_dgainPA30             2
#define gi_gain_tx_pa_dgainbuf30            4
#define gi_dc_tx_loopback_pa_dgainPA30      7
#define gi_dc_tx_loopback_pa_dgainbuf30     6
#define gi_gain_tx_loopback_pa_dgainPA30    6
#define gi_gain_tx_loopback_pa_dgainbuf30   4
#define gi_phase_tx_loopback_pa_dgainPA30   6
#define gi_phase_tx_loopback_pa_dgainbuf30  4
#define gi_cal_rx_iq_pa_dgainPA30           4
#define gi_cal_rx_iq_pa_dgainbuf30          5

#define TRX_BEKEN_REGS                                        \
{                                                             \
    0x00039042,                      /* 00-R0x00, //       */ \
    0xF0D9347B,                      /* 01-R0x01, //   26M spur    0xF259347B;*/ \
    0x4CAD2213,                      /* 02-R0x02, //       */ \
    0x7C945A75,                      /* 03-R0x03, //       */ \
    0x13D0110F,                      /* 04-R0x04, //       */ \
    0x1804AA47,                      /* 05-R0x05, //       */ \
    0x5FA44100,                      /* 06-R0x06, //       */ \
    0x000020F5,                      /* 07-R0x07, // 00F5:4th;20F5:2nd      */ \
    0x0728289E,                      /* 08-R0x08, //   0x072A289E    */ \
    0x000163AF,                      /* 09-R0x09, //       */ \
    TRX_REG_0XA_VAL,                 /* 10-R0x0A, //       */ \
    TRX_REG_0XB_VAL,                 /* 11-R0x0B, //       */ \
    TRX_REG_0XC_VAL,                 /* 12-R0x0C, //       */ \
    0xDDF90339,                      /* 13-R0x0D, //       */ \
    0xDA01BCF0,                      /* 14-R0x0E, //       */ \
    0x00018000,                      /* 15-R0x0F, //       */ \
    0xD8800000,                      /* 16-R0x10, //       */ \
    0x00000000,                      /* 17-R0x11, //       */ \
    0xD0090481,                      /* 18-R0x12, // 0xF0090481      */ \
    0x7B305ECC,                      /* 19-R0x13, //       */ \
    0x827C827C,                      /* 20-R0x14, //       */ \
    0x86788678,                      /* 21-R0x15, //       */ \
    0x8C748C74,                      /* 22-R0x16, //       */ \
    0xA45F9868,                      /* 23-R0x17, //       */ \
    0xA45FA45F,                      /* 24-R0x18, //       */ \
    0xA55EA45F,                      /* 25-R0x19, //       */ \
    0xA55DA55E,                      /* 26-R0x1A, //       */ \
    0xA55DA55D,                      /* 27-R0x1B, //       */ \
    0x20000000                       /* 28-R0x1C, //       */ \
};

#define RC_BEKEN_REGS                                         \
{                                                             \
    0x00000009,                      /* 00-R0x0,  0x0000;  */ \
    0xF0000000,                      /* 01-R0x1,  0x0004;  */ \
    0x00000030,                      /* 02-R0x5,  0x0014;  */ \
    0x00010001,                      /* 03-R0x8,  0x0020;  */ \
    0x000100e0,                      /* 04-R0xB,  0x002C;  */ \
    0x00010070,                      /* 05-R0xE,  0x0038;  */ \
    0x00010001,                      /* 06-R0x11, 0x0044;  */ \
    0x00010005,                      /* 07-R0x19, 0x0064;  */ \
    0x00000002,                      /* 08-R0x1C, 0x0070;  */ \
    0x0000012c,                      /* 09-R0x1E, 0x0078;  */ \
    0x1002DF4B,                      /* 10-R0x3C, 0x00F0;  */ \
    0x00000000,                      /* 11-R0x3E, 0x00F8;  */ \
    0x03E803CB,                      /* 12-R0x3F, 0x00FC;  */ \
    0x00000001,                      /* 13-R0x40, 0x0100;  */ \
    0x00000000,                      /* 14-R0x41, 0x0104;  */ \
    0x02000041,                      /* 15-R0x42, 0x0108;  */ \
    0x018B018B,                      /* 16-R0x4C, 0x0130;  */ \
    0x2CC02000,                      /* 17-R0x4D, 0x0134;  */ \
    0x02000200,                      /* 18-R0x4F, 0x013C;  */ \
    0x03ff03ff,                      /* 19-R0x50, 0x0140;  */ \
    0x02000200,                      /* 20-R0x51, 0x0144;  */ \
    0x44108800,                      /* 21-R0x52, 0x0148;  */ \
   (0x00025600|((TSSI_POUT_TH)<<1)), /* 22-R0x54, 0x0150;  */ \
    0x00000000,                      /* 23-R0x55, 0x0154;  */ \
    0x80000064,                      /* 24-R0x5C, 0x0170;  */ \
    0x00000001,                      /* 25-R0x4E, 0x0138;  */ \
    0x00000000,                      /* 26-R0x5A, 0x0168;  */ \
    0x00000000,                      /* 27-R0x5B, 0x016C;  */ \
    0x00000000,                      /* 28-R0x6A, 0x01A8;  */ \
    0x00000000,                      /* 29-R0x70, 0x01C0;  */ \
    0x00000000,                      /* 30-R0x71, 0x01C4;  */ \
    0x00000000,                      /* 31-R0x72, 0x01C8;  */ \
    0x00000000,                      /* 32-R0x73, 0x01CC;  */ \
    0x00000000,                      /* 33-R0x74, 0x01D0;  */ \
    0x00000000,                      /* 34-R0x75, 0x01D4;  */ \
    0x00000000,                      /* 35-R0x76, 0x01D8;  */ \
    0x00000000,                      /* 36-R0x77, 0x01DC;  */ \
};

const static INT32 bk7011_trx_val1[32] = TRX_BEKEN_REGS;
const static INT32 bk7011_rc_val1[37] = RC_BEKEN_REGS;

static INT32 bk7011_trx_val[32] = TRX_BEKEN_REGS;
static INT32 bk7011_rc_val[37] = RC_BEKEN_REGS;

#define gconst_iqcal_p      0xB0
INT32 gconst_pout = 0x210;
INT32 gav_tssi = 0;
INT32 gav_tssi_temp = 0;
INT32 grc_reg_map[32];
UINT8 gbias_after_cal = 0;

INT32 gtx_dcorMod = 0x4;
INT32 gtx_dcorPA = 0x8;
INT32 gtx_pre_gain = 0x10;

INT32 gtx_dcorMod_temp = 0x4;
INT32 gtx_dcorPA_temp = 0x8;
INT32 gtx_pre_gain_temp = 0x7;

INT32 gtx_dcorMod_temp_loopback = 0x4;
INT32 gtx_dcorPA_temp_loopback = 0x8;
INT32 gtx_pre_gain_temp_loopback = 0x7;

INT32 gtx_i_dc_comp_loopback = 0x202;
INT32 gtx_q_dc_comp_loopback = 0x1ff;

INT32 gtx_i_gain_comp_loopback = 0x3ff;
INT32 gtx_q_gain_comp_loopback = 0x3f3;

INT32 gtx_i_dc_comp = 0x200;
INT32 gtx_q_dc_comp = 0x200;

INT32 gtx_i_gain_comp = 0x3ff;
INT32 gtx_q_gain_comp = 0x3ff;

INT32 gtx_ifilter_corner = 0x10;
INT32 gtx_qfilter_corner = 0x10;
UINT32 g_capcal_sel = 14;

INT32 gtx_phase_comp_loopback = 0x200;
INT32 gtx_phase_ty2_loopback = 0x200;

INT32 gtx_phase_comp = 0x200;
INT32 gtx_phase_ty2 = 0x200;

INT32 gtx_power_cal_mode = TX_WANTED_POWER_CAL;
INT32 gtx_dc_cal_mode = TX_DC_CAL;
INT32 gtx_gain_imb_cal_mode = TX_GAIN_IMB_CAL;
INT32 gtx_phase_imb_cal_mode = TX_PHASE_IMB_CAL;

INT32 gtx_band = 0x4a;
static UINT32 is_tpc_used = 0;

typedef enum{
    CM_TX_DCOR_MOD_FLAG              = 0,
    CM_TX_DCOR_PA_FLAG,              // 1
    CM_TX_PREGAIN_FLAG,              // 2
    CM_TX_I_DC_COMP_FLAG,            // 3
    CM_TX_Q_DC_COMP_FLAG,            // 4
    CM_TX_I_GAIN_COMP_FLAG,          // 5
    CM_TX_Q_GAIN_COMP_FLAG,          // 6
    CM_TX_I_FILTER_CORNER_FLAG,      // 7
    CM_TX_Q_FILTER_CORNER_FLAG,      // 8
    CM_TX_PHASE_COMP_FLAG,           // 9
    CM_TX_PHASE_TY2_FLAG,            // 10
    CM_RX_DC_GAIN_TAB_FLAG,          // 11
    CM_RX_AMP_ERR_WR_FLAG,           // 12
    CM_RX_PHASE_ERR_WR_FLAG,         // 13
}CM_FLAG;

static UINT32 cali_main_flags = 0;
#define CM_SET_FLAG_BIT(bit)      cali_main_flags |= (1 << (bit))
#define CM_CLR_FLAG_BIT(bit)      cali_main_flags &= ~(1 << (bit))
#define CM_CLR_FLAG_ALLBIT()      cali_main_flags = 0
#define CM_FLAG_IS_SET()          (cali_main_flags != 0)?  1 : 0

struct cal_pwr_st 
{
    UINT8 idx;
    UINT8 mode;    
};

struct cal_pwr_st g_pwr_current;

INT32 g_rx_dc_gain_tab[8] =
{
    0x827C827C,
    0x86788678,
    0x8C748C74,
    0xA45F9868,
    0xA45FA45F,
    0xA55EA45F,
    0xA55DA55E,
    0xA55DA55D
};

INT32 grx_amp_err_wr = 0x200;
INT32 grx_phase_err_wr = 0x041;

#include "net_param_pub.h"
INT32 gtx_tssi_thred = TSSI_POUT_TH;
#define TSSI_TH_G       (115)
#define TSSI_TH_B       (125)
UINT32 g_cali_mode = CALI_MODE_MANUAL;
INT32 gtx_tssi_thred_b = TSSI_TH_B;
INT32 gtx_tssi_thred_g = TSSI_TH_G;

#define PWRI(gain, rb_28_31, rc_8_10, rc_4_6, rc_0_2, ra_8_13, ra_4_7, ra_0_1)      \
{                                     \
     .unuse      = 0,            \
     .pregain    = gain,         \
     .regb_28_31 = rb_28_31,     \
     .regc_8_10  = rc_8_10,      \
     .regc_4_6   = rc_4_6,       \
     .regc_0_2   = rc_0_2,       \
     .rega_8_13  = ra_8_13,      \
     .rega_4_7   = ra_4_7,       \
     .rega_0_1   = ra_0_1,       \
} 

PWR_REGS cfg_tab_b[32] = {                                                                       
      // pregain REGB<31:28> REGC<10:8> REGC<6:4> REGC<2:0> REGA<13:8> REGA<7:4> REGA<1:0>       
    PWRI( 0x16 ,      0xA,       7,        3,         1,      0x20,     0x7,     0x1   ),   // 0 
    PWRI( 0x13 ,      0xA,       7,        3,         1,      0x20,     0x7,     0x1   ),   // 1 
    PWRI( 0x10 ,      0xA,       7,        3,         1,      0x20,     0x7,     0x1   ),   // 2 
    PWRI( 0x19 ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 3 
    PWRI( 0x16 ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 4 
    PWRI( 0x13 ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 5 
    PWRI( 0x10 ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 6 
    PWRI( 0xD  ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 7 
    PWRI( 0xA  ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 8 
    PWRI( 0x7  ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 9 
    PWRI( 0x4  ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 10
    PWRI( 0x1  ,      0xA,       7,        3,         2,      0x20,     0x7,     0x1   ),   // 11
    PWRI( 0x12 ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 12
    PWRI( 0xF  ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 13
    PWRI( 0xC  ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 14
    PWRI( 0x9  ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 15
    PWRI( 0x6  ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 16
    PWRI( 0x3  ,      0xA,       7,        4,         3,      0x20,     0x7,     0x1   ),   // 17
    PWRI( 0x1E ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 18
    PWRI( 0x1C ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 19
    PWRI( 0x1A ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 20
    PWRI( 0x18 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 21
    PWRI( 0x16 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 22
    PWRI( 0x14 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 23
    PWRI( 0x12 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 24
    PWRI( 0x10 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 25
    PWRI( 0xE  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 26
    PWRI( 0xC  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 27
    PWRI( 0x9  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 28
    PWRI( 0x6  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 29
    PWRI( 0x3  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 30
    PWRI( 0x0  ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 31
};

PWR_REGS cfg_tab_g[32] = {                                                             
    // pregain REGB<31:28> REGC<10:8> REGC<6:4> REGC<2:0> REGA<13:8> REGA<7:4> REGA<1:0>
    PWRI( 0x18,      0xA,       7,        5,         2,      0x20,     0x7,     0x1   ),   // 0 
    PWRI( 0x15,      0xA,       7,        5,         2,      0x20,     0x7,     0x1   ),   // 1 
    PWRI( 0x1E,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 2 
    PWRI( 0x1B,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 3 
    PWRI( 0x18,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 4 
    PWRI( 0x15,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 5 
    PWRI( 0x12,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 6 
    PWRI( 0xF ,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 7 
    PWRI( 0xC ,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 8 
    PWRI( 0x9 ,      0xA,       7,        5,         3,      0x20,     0x7,     0x1   ),   // 9 
    PWRI( 0x1F,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 10
    PWRI( 0x1D,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 11
    PWRI( 0x1A,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 12
    PWRI( 0x17,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 13
    PWRI( 0x14,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 14
    PWRI( 0x11,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 15
    PWRI( 0xE ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 16
    PWRI( 0xB ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 17
    PWRI( 0x8 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 18
    PWRI( 0x5 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 19
    PWRI( 0x2 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 20
    PWRI( 0x0 ,      0xA,       7,        5,         5,      0x20,     0x7,     0x1   ),   // 21
    PWRI( 0xF ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 22
    PWRI( 0xC ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 23
    PWRI( 0x9 ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 24
    PWRI( 0x6 ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 25
    PWRI( 0x3 ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 26
    PWRI( 0x0 ,      0xA,       7,        6,         6,      0x20,     0x7,     0x1   ),   // 27
    PWRI( 0xC ,      0xA,       7,        7,         6,      0x20,     0x7,     0x2   ),   // 28
    PWRI( 0x9 ,      0xA,       7,        7,         6,      0x20,     0x7,     0x2   ),   // 29
    PWRI( 0x6 ,      0xA,       7,        7,         6,      0x20,     0x7,     0x2   ),   // 30
    PWRI( 0x3 ,      0xA,       7,        7,         6,      0x20,     0x7,     0x2   ),   // 31
};

PWR_REGS cfg_tab_ble[32] = {                                                                                                                                                               
    // pregain REGB<31:28> REGC<11:8> REGC<7:4> REGC<3:0> REGA<13:8> REGA<7:4> REGA<1:0>         
	PWRI( 0x1F,      0xF,       1,        0,         0,      0x20,      0x7,     0x1   ),   // 0   
	PWRI( 0x1F,      0xC,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 1   
	PWRI( 0x1F,      0xB,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 2   
	PWRI( 0x1F,      0xA,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 3   
	PWRI( 0x1D,      0xA,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 4   
	PWRI( 0x1A,      0xA,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 5   
	PWRI( 0x17,      0xA,       7,        0,         0,      0x20,      0x7,     0x1   ),   // 6   
	PWRI( 0x1F,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 7   
	PWRI( 0x1C,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 8   
	PWRI( 0x19,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 9   
	PWRI( 0x16,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 10  
	PWRI( 0x13,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 11  
	PWRI( 0x10,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 12  
	PWRI( 0xD ,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 13  
	PWRI( 0xA ,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 14  
	PWRI( 0x7 ,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 15  
	PWRI( 0x4 ,      0xA,       7,        1,         0,      0x20,      0x7,     0x1   ),   // 16  
	PWRI( 0x1F,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 17  
	PWRI( 0x1C,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 18  
	PWRI( 0x19,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 19  
	PWRI( 0x16,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 20  
	PWRI( 0x13,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 21  
	PWRI( 0x10,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 22  
	PWRI( 0xD ,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 23  
	PWRI( 0xA ,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 24  
	PWRI( 0x7 ,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 25  
	PWRI( 0x4 ,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 26  
	PWRI( 0x1 ,      0xA,       7,        3,         1,      0x20,      0x7,     0x1   ),   // 27  
	PWRI( 0x1B,      0xA,       7,        4,         3,      0x20,      0x7,     0x1   ),   // 28  
	PWRI( 0x18,      0xA,       7,        4,         3,      0x20,      0x7,     0x1   ),   // 29  
	PWRI( 0x15,      0xA,       7,        4,         3,      0x20,      0x7,     0x1   ),   // 30  
	PWRI( 0x12,      0xA,       7,        4,         3,      0x20,      0x7,     0x1   ),   // 31  
};      

#define TCP_PAMAP_TAB_LEN               (16)
#define TCP_PAMAP_DEF_PREGAIN           (0x0)
#define TCP_PAMAP_DEF_A13_8             (0x20)
#define TCP_PAMAP_DEF_A7_4              (0x1)
#define TCP_PAMAP_DEF_A1_0              (0x1)

#define TPCI(rb_28_31, rc_0_3, rc_4_7, rc_8_11)      \
    {                                     \
    .regb_28_31 = rb_28_31,     \
    .regc_0_3   = rc_0_3,       \
    .regc_4_7   = rc_4_7,       \
    .regc_8_11  = rc_8_11,      \
    .value      = (rb_28_31<<12)+(rc_0_3<<8)+(rc_4_7<<4)+(rc_8_11) \
    }

const PWR_REGS_TPC cfg_tab_tpc[TCP_PAMAP_TAB_LEN] = {    
    // REGB<31:28> REGC<3:0> REGC<7:4> REGC<11:8>  value  
    TPCI(  0xC,	      2,	      3,         7),   // 0
    TPCI(  0xA,	      2,	      3,         7),   // 1 
    TPCI(  0x8,	      2,	      3,         7),   // 2 
    TPCI(  0xB,	      3,	      3,         7),   // 3 
    TPCI(  0x9,	      3,	      3,         7),   // 4 
    TPCI(  0xC,	      3,	      5,         7),   // 5 
    TPCI(  0xA,	      3,	      5,         7),   // 6 
    TPCI(  0x8,	      3,	      5,         7),   // 7 
    TPCI(  0xB,	      4,	      5,         7),   // 8 
    TPCI(  0x9,	      4,	      5,         7),   // 9 
    TPCI(  0x7,	      4,	      5,         7),   // 10
    TPCI(  0xA,	      5,	      5,         7),   // 11
    TPCI(  0x8,	      5,	      5,         7),   // 12
    TPCI(  0x6,	      5,	      5,         7),   // 13
    TPCI(  0x9,	      6,	      5,         7),   // 14
    TPCI(  0x7,	      6,	      5,         7),   // 15
};

const UINT8 b_map[32] = {
    2,  3,  3,  4,   // 0 - 3
    4,  5,  5,  6,   // 4 - 7
    7,  7,  8,  8,   // 8 - 11
    9,  9,  10,  11,   // 12 - 15
    12,  12,  13,  13,   // 16 - 19
    13,  14,  14,  14,   // 20 - 23
    15,  15,  15, 15,   // 24 - 27
    15, 15, 15, 15,   // 28 - 31
};

const UINT8 gn_map[32] = {
    0,  0,  0,  0,   // 0 - 3
    1,  2,  2,  3,   // 4 - 7
    4,  4,  4,  5,   // 8 - 11
    5,  6,  7,  7,   // 12 - 15
    8,  9,  10,  10,   // 16 - 19
    11,  12,  12,  12,   // 20 - 23
    13,  13,  14, 14,   // 24 - 27
    14, 15, 15, 15,   // 28 - 31
};

struct BK7011RCBEKEN_TypeDef BK7011RCBEKEN =
{
    (volatile BK7011_RC_BEKEN_REG0x0_TypeDef *) (RC_BEKEN_BASE + 0 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1_TypeDef *) (RC_BEKEN_BASE + 1 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5_TypeDef *) (RC_BEKEN_BASE + 5 * 4),
    (volatile BK7011_RC_BEKEN_REG0x8_TypeDef *) (RC_BEKEN_BASE + 8 * 4),
    (volatile BK7011_RC_BEKEN_REG0xB_TypeDef *) (RC_BEKEN_BASE + 11 * 4),
    (volatile BK7011_RC_BEKEN_REG0xE_TypeDef *) (RC_BEKEN_BASE + 14 * 4),
    (volatile BK7011_RC_BEKEN_REG0x11_TypeDef *)(RC_BEKEN_BASE + 17 * 4),
    (volatile BK7011_RC_BEKEN_REG0x19_TypeDef *)(RC_BEKEN_BASE + 25 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1C_TypeDef *)(RC_BEKEN_BASE + 28 * 4),
    (volatile BK7011_RC_BEKEN_REG0x1E_TypeDef *)(RC_BEKEN_BASE + 30 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3C_TypeDef *)(RC_BEKEN_BASE + 60 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3E_TypeDef *)(RC_BEKEN_BASE + 62 * 4),
    (volatile BK7011_RC_BEKEN_REG0x3F_TypeDef *)(RC_BEKEN_BASE + 63 * 4),
    (volatile BK7011_RC_BEKEN_REG0x40_TypeDef *)(RC_BEKEN_BASE + 64 * 4),
    (volatile BK7011_RC_BEKEN_REG0x41_TypeDef *)(RC_BEKEN_BASE + 65 * 4),
    (volatile BK7011_RC_BEKEN_REG0x42_TypeDef *)(RC_BEKEN_BASE + 66 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4C_TypeDef *)(RC_BEKEN_BASE + 76 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4D_TypeDef *)(RC_BEKEN_BASE + 77 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4E_TypeDef *)(RC_BEKEN_BASE + 78 * 4),
    (volatile BK7011_RC_BEKEN_REG0x4F_TypeDef *)(RC_BEKEN_BASE + 79 * 4),
    (volatile BK7011_RC_BEKEN_REG0x50_TypeDef *)(RC_BEKEN_BASE + 80 * 4),
    (volatile BK7011_RC_BEKEN_REG0x51_TypeDef *)(RC_BEKEN_BASE + 81 * 4),
    (volatile BK7011_RC_BEKEN_REG0x52_TypeDef *)(RC_BEKEN_BASE + 82 * 4),
    (volatile BK7011_RC_BEKEN_REG0x54_TypeDef *)(RC_BEKEN_BASE + 84 * 4),
    (volatile BK7011_RC_BEKEN_REG0x55_TypeDef *)(RC_BEKEN_BASE + 85 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5A_TypeDef *)(RC_BEKEN_BASE + 90 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5B_TypeDef *)(RC_BEKEN_BASE + 91 * 4),
    (volatile BK7011_RC_BEKEN_REG0x5C_TypeDef *)(RC_BEKEN_BASE + 92 * 4),
    (volatile BK7011_RC_BEKEN_REG0x6A_TypeDef *)(RC_BEKEN_BASE + 106 * 4),
    (volatile BK7011_RC_BEKEN_REG0x70_TypeDef *)(RC_BEKEN_BASE + 112 * 4),
    (volatile BK7011_RC_BEKEN_REG0x71_TypeDef *)(RC_BEKEN_BASE + 113 * 4),
    (volatile BK7011_RC_BEKEN_REG0x72_TypeDef *)(RC_BEKEN_BASE + 114 * 4),
    (volatile BK7011_RC_BEKEN_REG0x73_TypeDef *)(RC_BEKEN_BASE + 115 * 4),
    (volatile BK7011_RC_BEKEN_REG0x74_TypeDef *)(RC_BEKEN_BASE + 116 * 4),
    (volatile BK7011_RC_BEKEN_REG0x75_TypeDef *)(RC_BEKEN_BASE + 117 * 4),
    (volatile BK7011_RC_BEKEN_REG0x76_TypeDef *)(RC_BEKEN_BASE + 118 * 4),
    (volatile BK7011_RC_BEKEN_REG0x77_TypeDef *)(RC_BEKEN_BASE + 119 * 4),
};

struct BK7011TRxV2A_TypeDef BK7011TRX =
{
    (BK7011_TRxV2A_REG0x0_TypeDef *)(&grc_reg_map[0]),
    (BK7011_TRxV2A_REG0x1_TypeDef *)(&grc_reg_map[1]),
    (BK7011_TRxV2A_REG0x2_TypeDef *)(&grc_reg_map[2]),
    (BK7011_TRxV2A_REG0x3_TypeDef *)(&grc_reg_map[3]),
    (BK7011_TRxV2A_REG0x4_TypeDef *)(&grc_reg_map[4]),
    (BK7011_TRxV2A_REG0x5_TypeDef *)(&grc_reg_map[5]),
    (BK7011_TRxV2A_REG0x6_TypeDef *)(&grc_reg_map[6]),
    (BK7011_TRxV2A_REG0x7_TypeDef *)(&grc_reg_map[7]),
    (BK7011_TRxV2A_REG0x8_TypeDef *)(&grc_reg_map[8]),
    (BK7011_TRxV2A_REG0x9_TypeDef *)(&grc_reg_map[9]),
    (BK7011_TRxV2A_REG0xA_TypeDef *)(&grc_reg_map[10]),
    (BK7011_TRxV2A_REG0xB_TypeDef *)(&grc_reg_map[11]),
    (BK7011_TRxV2A_REG0xC_TypeDef *)(&grc_reg_map[12]),
    (BK7011_TRxV2A_REG0xD_TypeDef *)(&grc_reg_map[13]),
    (BK7011_TRxV2A_REG0xE_TypeDef *)(&grc_reg_map[14]),
    (BK7011_TRxV2A_REG0xF_TypeDef *)(&grc_reg_map[15]),
    (BK7011_TRxV2A_REG0x10_TypeDef *)(&grc_reg_map[16]),
    (BK7011_TRxV2A_REG0x11_TypeDef *)(&grc_reg_map[17]),
    (BK7011_TRxV2A_REG0x12_TypeDef *)(&grc_reg_map[18]),
    (BK7011_TRxV2A_REG0x13_TypeDef *)(&grc_reg_map[19]),
    (BK7011_TRxV2A_REG0x14_TypeDef *)(&grc_reg_map[20]),
    (BK7011_TRxV2A_REG0x15_TypeDef *)(&grc_reg_map[21]),
    (BK7011_TRxV2A_REG0x16_TypeDef *)(&grc_reg_map[22]),
    (BK7011_TRxV2A_REG0x17_TypeDef *)(&grc_reg_map[23]),
    (BK7011_TRxV2A_REG0x18_TypeDef *)(&grc_reg_map[24]),
    (BK7011_TRxV2A_REG0x19_TypeDef *)(&grc_reg_map[25]),
    (BK7011_TRxV2A_REG0x1A_TypeDef *)(&grc_reg_map[26]),
    (BK7011_TRxV2A_REG0x1B_TypeDef *)(&grc_reg_map[27]),
    (BK7011_TRxV2A_REG0x1C_TypeDef *)(&grc_reg_map[28]),
};

struct BK7011TRxV2A_TypeDef BK7011TRXONLY =
{
    (volatile BK7011_TRxV2A_REG0x0_TypeDef *) (TRX_BEKEN_BASE + 0 * 4),
    (volatile BK7011_TRxV2A_REG0x1_TypeDef *) (TRX_BEKEN_BASE + 1 * 4),
    (volatile BK7011_TRxV2A_REG0x2_TypeDef *) (TRX_BEKEN_BASE + 2 * 4),
    (volatile BK7011_TRxV2A_REG0x3_TypeDef *) (TRX_BEKEN_BASE + 3 * 4),
    (volatile BK7011_TRxV2A_REG0x4_TypeDef *) (TRX_BEKEN_BASE + 4 * 4),
    (volatile BK7011_TRxV2A_REG0x5_TypeDef *) (TRX_BEKEN_BASE + 5 * 4),
    (volatile BK7011_TRxV2A_REG0x6_TypeDef *) (TRX_BEKEN_BASE + 6 * 4),
    (volatile BK7011_TRxV2A_REG0x7_TypeDef *) (TRX_BEKEN_BASE + 7 * 4),
    (volatile BK7011_TRxV2A_REG0x8_TypeDef *) (TRX_BEKEN_BASE + 8 * 4),
    (volatile BK7011_TRxV2A_REG0x9_TypeDef *) (TRX_BEKEN_BASE + 9 * 4),
    (volatile BK7011_TRxV2A_REG0xA_TypeDef *) (TRX_BEKEN_BASE + 10 * 4),
    (volatile BK7011_TRxV2A_REG0xB_TypeDef *) (TRX_BEKEN_BASE + 11 * 4),
    (volatile BK7011_TRxV2A_REG0xC_TypeDef *) (TRX_BEKEN_BASE + 12 * 4),
    (volatile BK7011_TRxV2A_REG0xD_TypeDef *) (TRX_BEKEN_BASE + 13 * 4),
    (volatile BK7011_TRxV2A_REG0xE_TypeDef *) (TRX_BEKEN_BASE + 14 * 4),
    (volatile BK7011_TRxV2A_REG0xF_TypeDef *) (TRX_BEKEN_BASE + 15 * 4),
    (volatile BK7011_TRxV2A_REG0x10_TypeDef *)(TRX_BEKEN_BASE + 16 * 4),
    (volatile BK7011_TRxV2A_REG0x11_TypeDef *)(TRX_BEKEN_BASE + 17 * 4),
    (volatile BK7011_TRxV2A_REG0x12_TypeDef *)(TRX_BEKEN_BASE + 18 * 4),
    (volatile BK7011_TRxV2A_REG0x13_TypeDef *)(TRX_BEKEN_BASE + 19 * 4),
    (volatile BK7011_TRxV2A_REG0x14_TypeDef *)(TRX_BEKEN_BASE + 20 * 4),
    (volatile BK7011_TRxV2A_REG0x15_TypeDef *)(TRX_BEKEN_BASE + 21 * 4),
    (volatile BK7011_TRxV2A_REG0x16_TypeDef *)(TRX_BEKEN_BASE + 22 * 4),
    (volatile BK7011_TRxV2A_REG0x17_TypeDef *)(TRX_BEKEN_BASE + 23 * 4),
    (volatile BK7011_TRxV2A_REG0x18_TypeDef *)(TRX_BEKEN_BASE + 24 * 4),
    (volatile BK7011_TRxV2A_REG0x19_TypeDef *)(TRX_BEKEN_BASE + 25 * 4),
    (volatile BK7011_TRxV2A_REG0x1A_TypeDef *)(TRX_BEKEN_BASE + 26 * 4),
    (volatile BK7011_TRxV2A_REG0x1B_TypeDef *)(TRX_BEKEN_BASE + 27 * 4),
    (volatile BK7011_TRxV2A_REG0x1C_TypeDef *)(TRX_BEKEN_BASE + 28 * 4),
};

struct temp_cal_pwr_st g_temp_pwr_current = {16, EVM_DEFUALT_RATE, 0, 0};

void delay05us(INT32 num)
{
    volatile INT32 i, j;

    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 5; j ++)
            ;
    }
}

void delay100us(INT32 num)
{
    volatile INT32 i, j;

    for(i = 0; i < num; i ++)
    {
        for(j = 0; j < 1050; j ++)
            ;
    }
}


#define CAL_WR_TRXREGS(reg)    do{\
                                    power_save_wake_rf_if_in_sleep();\
                                    while(BK7011RCBEKEN.REG0x1->value & (0x1 << reg));\
                                    BK7011TRXONLY.REG##reg->value = BK7011TRX.REG##reg->value;\
                                    cal_delay(6);\
                                    while(BK7011RCBEKEN.REG0x1->value & (0x1 << reg));\
                                    power_save_check_clr_rf_prevent_flag();\
                                }while(0)


void rwnx_cal_load_default_result(void)
{
    gtx_dcorMod = (bk7011_trx_val[11] >> 12) & 0xf;
    gtx_dcorPA = (bk7011_trx_val[12] >> 12) & 0xf;
    gtx_pre_gain = (bk7011_rc_val[21] >> 16) & 0x1f;

    gtx_i_dc_comp = (bk7011_rc_val[18] >> 16) & 0x3ff;
    gtx_q_dc_comp = bk7011_rc_val[18] & 0x3ff;

    gtx_i_gain_comp = (bk7011_rc_val[19] >> 16) & 0x3ff;
    gtx_q_gain_comp = bk7011_rc_val[19] & 0x3ff;

    gtx_ifilter_corner = (bk7011_trx_val[6] >> 10) & 0x3f;
    gtx_qfilter_corner = (bk7011_trx_val[6] >> 4) & 0x3f;
    gtx_phase_comp = (bk7011_rc_val[20] >> 16) & 0x3ff;
    gtx_phase_ty2 = bk7011_rc_val[20] & 0x3ff;

    gtx_band = (bk7011_trx_val[3] >> 17) & 0x7f;

    g_rx_dc_gain_tab[0] = bk7011_trx_val[20];
    g_rx_dc_gain_tab[1] = bk7011_trx_val[21];
    g_rx_dc_gain_tab[2] = bk7011_trx_val[22];
    g_rx_dc_gain_tab[3] = bk7011_trx_val[23];
    g_rx_dc_gain_tab[4] = bk7011_trx_val[24];
    g_rx_dc_gain_tab[5] = bk7011_trx_val[25];
    g_rx_dc_gain_tab[6] = bk7011_trx_val[26];
    g_rx_dc_gain_tab[7] = bk7011_trx_val[27];

    grx_amp_err_wr = (bk7011_rc_val[15] >> 16) & 0x3ff;
    grx_phase_err_wr = bk7011_rc_val[15] & 0x3ff;
}

void rwnx_cal_read_current_cal_result(void)
{
    CAL_FATAL("*********** finally result **********\r\n");
    CAL_FATAL("gtx_dcorMod            : 0x%x\r\n", gtx_dcorMod);
    CAL_FATAL("gtx_dcorPA             : 0x%x\r\n", gtx_dcorPA);
    CAL_FATAL("gtx_pre_gain           : 0x%x\r\n", gtx_pre_gain);
    CAL_FATAL("gtx_i_dc_comp          : 0x%x\r\n", gtx_i_dc_comp);
    CAL_FATAL("gtx_q_dc_comp          : 0x%x\r\n", gtx_q_dc_comp);
    CAL_FATAL("gtx_i_gain_comp        : 0x%x\r\n", gtx_i_gain_comp);
    CAL_FATAL("gtx_q_gain_comp        : 0x%x\r\n", gtx_q_gain_comp);
    CAL_FATAL("gtx_ifilter_corner over: 0x%x\r\n", gtx_ifilter_corner);
    CAL_FATAL("gtx_qfilter_corner over: 0x%x\r\n", gtx_qfilter_corner);
    CAL_FATAL("gtx_phase_comp         : 0x%x\r\n", gtx_phase_comp);
    CAL_FATAL("gtx_phase_ty2          : 0x%x\r\n", gtx_phase_ty2);
    
    CAL_FATAL("gbias_after_cal        : 0x%x\r\n", gbias_after_cal);
    CAL_FATAL("gav_tssi               : 0x%x\r\n", gav_tssi);

    CAL_FATAL("g_rx_dc_gain_tab 0 over: 0x%x\r\n", g_rx_dc_gain_tab[0]);
    CAL_FATAL("g_rx_dc_gain_tab 1 over: 0x%x\r\n", g_rx_dc_gain_tab[1]);
    CAL_FATAL("g_rx_dc_gain_tab 2 over: 0x%x\r\n", g_rx_dc_gain_tab[2]);
    CAL_FATAL("g_rx_dc_gain_tab 3 over: 0x%x\r\n", g_rx_dc_gain_tab[3]);
    CAL_FATAL("g_rx_dc_gain_tab 4 over: 0x%x\r\n", g_rx_dc_gain_tab[4]);
    CAL_FATAL("g_rx_dc_gain_tab 5 over: 0x%x\r\n", g_rx_dc_gain_tab[5]);
    CAL_FATAL("g_rx_dc_gain_tab 6 over: 0x%x\r\n", g_rx_dc_gain_tab[6]);
    CAL_FATAL("g_rx_dc_gain_tab 7 over: 0x%x\r\n", g_rx_dc_gain_tab[7]);
    CAL_FATAL("grx_amp_err_wr         : 0x%03x\r\n", grx_amp_err_wr);
    CAL_FATAL("grx_phase_err_wr       : 0x%03x\r\n", grx_phase_err_wr);
    CAL_FATAL("**************************************\r\n");
}

void rwnx_cal_set_lpfcap_iq(UINT32 lpfcap_i, UINT32 lpfcap_q)
{   
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = lpfcap_q;
    BK7011TRX.REG0x6->bits.lpfcapcali50 = lpfcap_i; 
    CAL_WR_TRXREGS(0x6);
    bk7011_trx_val[6] = BK7011TRX.REG0x6->value;
}

void rwnx_cal_enable_lpfcap_iq_from_digit(UINT32 enable)
{
    if(enable)
        BK7011TRX.REG0x6->bits.capcal_sel = 1; 
    else
        BK7011TRX.REG0x6->bits.capcal_sel = 0; 
    
    CAL_WR_TRXREGS(0x6);
    bk7011_trx_val[6] = BK7011TRX.REG0x6->value;
}

#if CFG_SUPPORT_MANUAL_CALI
static UINT32 rwnx_cal_translate_tx_rate(UINT32 rate)
{
    UINT32 param;

    switch(rate)
    {
    case 0 :
        param = 1;
        break;  // 1Mbps
    case 1 :
        param = 2;
        break;  // 2Mbps
    case 2 :
        param = 5;
        break;	// 5.5Mbps
    case 3:
        param = 11;
        break;	// 11Mbps
    case 4:
        param = 6;
        break;	// 6Mbps
    case 5 :
        param = 9;
        break;	// 9Mbps
    case 6:
        param = 12;
        break;	// 12Mbps
    case 7:
        param = 18;
        break;	// 18Mbps
    case 8:
        param = 24;
        break;	// 24Mbps
    case 9:
        param = 36;
        break;	// 36Mbps
    case 10:
        param = 48;
        break;	// 48Mbps
    case 11:
        param = 54;
        break;	// 54Mbps
    default:
        param = rate;
        break;	// 54Mbps
    }

    return param;
}
#endif

void rwnx_cal_set_txpwr_by_rate(INT32 rate, UINT32 test_mode)
{
#if CFG_SUPPORT_MANUAL_CALI     
    UINT32 ret;
    UINT32 pwr_gain;

    struct phy_channel_info info;
    UINT32 channel, bandwidth;   // PHY_CHNL_BW_20,  PHY_CHNL_BW_40:

    phy_get_channel(&info, 0);
    bandwidth = (info.info1 >> 8) & 0xff;

    channel = (((info.info2 & 0xffff) - 2400) - 7)/5;
    if(channel > 14)
        channel = 14;    
    if(!manual_cal_get_txpwr(rwnx_cal_translate_tx_rate(rate), 
        channel, bandwidth, &pwr_gain)) 
    {
        // unable get txpwr from manual cal
        return;
    }
    
    if(test_mode == 0)
    {
        ret = manual_cal_get_pwr_idx_shift(rate, bandwidth, &pwr_gain);
    }
    else
    {
        ret = 2;
        if((bandwidth == PHY_CHNL_BW_20) && (rate <= 3))
        {
            ret = 1;
        }
    }
    
    if(!ret ){
        // unable get txpwr from manual cal
        return;
    } else if(ret == 1) {
        rwnx_cal_set_txpwr(pwr_gain, EVM_DEFUALT_B_RATE);
    } else if(ret == 2) {
        rwnx_cal_set_txpwr(pwr_gain, EVM_DEFUALT_RATE);
    }

    if((rate <= 3) || ((test_mode) && ((rate == 4) || (rate == 128))))
    {
        // just for 11b
        BK7011RCBEKEN.REG0x5A->bits.TXCALCAPI = 0x3f;
        BK7011RCBEKEN.REG0x5B->bits.TXCALCAPQ = 0x3f;
    }
    else 
    {
        BK7011RCBEKEN.REG0x5A->bits.TXCALCAPI = gtx_ifilter_corner;
        BK7011RCBEKEN.REG0x5B->bits.TXCALCAPQ = gtx_qfilter_corner;   
    }
#endif
}

void rwnx_cal_set_txpwr_by_channel(UINT32 channel)
{
    UINT32 pwr_gain, rate;
    
    rate = EVM_DEFUALT_BLE_RATE;
    
    if(!manual_cal_get_txpwr(rate, channel, 0, &pwr_gain)) 
    {
        // unable get txpwr from manual cal
        return;
    }

    if(get_ate_mode_state())
    {
#ifdef ATE_PRINT_DEBUG
        os_printf("set pwr:%d - c:%d\r\n", pwr_gain, channel);
#else
        //os_printf("c:%d\r\n", channel);
#endif
    }

    power_save_wake_rf_if_in_sleep();
    rwnx_cal_set_txpwr_ble(pwr_gain);
    power_save_check_clr_rf_prevent_flag();
}

extern void tpc_init(void);
extern void tpc_deinit(void);
struct temp_cal_pwr_st g_temp_pwr_current_tpc = {0, EVM_DEFUALT_RATE, 0, 0};
void rwnx_set_tpc_txpwr_by_tmpdetect(INT16 shift_b, INT16 shift_g)
{
    g_temp_pwr_current_tpc.shift_g = shift_g;
    g_temp_pwr_current_tpc.shift = shift_b;

    if(ble_in_dut_mode() == 0)
    {
        os_printf("td set tpc pwr: shift_b:%d, shift_g:%d\r\n", 
            shift_b, shift_g);
    }
}  

UINT32 rwnx_tpc_pwr_idx_translate(UINT32 pwr_gain, UINT32 rate, UINT32 print_log)
{
    const UINT8 *idx_tab;
    INT16 idx = TCP_PAMAP_TAB_LEN - 1;
    INT16 shift;
    
    if(rate == EVM_DEFUALT_B_RATE) {
    // for b
        idx_tab = b_map;
        shift = g_temp_pwr_current_tpc.shift;
    } else if(rate == EVM_DEFUALT_RATE) {
    // for g
        idx_tab = gn_map;
        shift = g_temp_pwr_current_tpc.shift_g;
    } else {
        os_printf("no support :%d\r\n", rate);
        return idx;
    }

#if CFG_USE_TEMPERATURE_DETECT
    idx = pwr_gain + shift;

    if(idx > 32) {
	    idx = 32; 
	}else if (idx < 0)
	    idx = 0;
    
    g_temp_pwr_current_tpc.idx = idx;
#endif

    idx = idx_tab[idx];

    if(idx > 16) {
	   idx = 16; 
	}
    
    if (print_log)
    {
        os_printf("translate idx1:%d, td_shift:%d, b/g:%d --- idx2:%d\r\n", pwr_gain, 
            shift, rate, idx);
        
    }

    return idx;
    
}

UINT32 rwnx_tpc_get_pwridx_by_rate(UINT32 rate, UINT32 print_log)
{
    UINT32 ret, ret_bak;
    UINT32 pwr_gain;

    struct phy_channel_info info;
    UINT32 channel, bandwidth;   // PHY_CHNL_BW_20,  PHY_CHNL_BW_40:

    phy_get_channel(&info, 0);
    bandwidth = (info.info1 >> 8) & 0xff;
    if(rate <= 128)
        bandwidth = 0;

    channel = (((info.info2 & 0xffff) - 2400) - 7)/5;

    if(channel > 14)
        channel = 14;

    if(manual_cal_get_txpwr(rwnx_cal_translate_tx_rate(rate), 
        channel, bandwidth, &pwr_gain) == 0) 
    {
        os_printf("unable get txpwr %d, %d, %d\r\n", rate, channel, bandwidth);
        return 0;
    }

    ret_bak = ret = manual_cal_get_pwr_idx_shift(rate, bandwidth, &pwr_gain);
    
    if(!ret ){
        os_printf("unable get txpwr shift %d, %d, %d\r\n", rate, channel, bandwidth);
        return 0;
    } else if(ret == 1) {
        ret = rwnx_tpc_pwr_idx_translate(pwr_gain, EVM_DEFUALT_B_RATE, 0);
    } else if(ret == 2) {
        ret = rwnx_tpc_pwr_idx_translate(pwr_gain, EVM_DEFUALT_RATE, 0);
    }

    if (print_log)
    {
        INT16 shift = g_temp_pwr_current.shift;
        if(ret_bak == 2) {
            shift = g_temp_pwr_current.shift_g;
        }
        
        os_printf("tpc info- r:%d, c:%d, b:%d -- idx1:%d+(%d), idx2: %d\r\n", 
            rate, channel, bandwidth, pwr_gain, shift, ret);

        const PWR_REGS_TPC *ptr = &cfg_tab_tpc[ret];
        os_printf("b[31-28]:0x%02x, c[3-0]:0x%02x, c[7-4]:0x%02x, c[11-8]:0x%02x\r\n", 
            ptr->regb_28_31, ptr->regc_0_3, ptr->regc_4_7, ptr->regc_8_11);
    }
    
    return ret;
    
}

void rwnx_use_tpc_set_pwr(void)
{
    #if CFG_SUPPORT_TPC_PA_MAP
    tpc_init();

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = gtx_pre_gain = TCP_PAMAP_DEF_PREGAIN;
    bk7011_rc_val[21] = BK7011RCBEKEN.REG0x52->value;

    BK7011TRX.REG0xA->bits.dbpab30 = TCP_PAMAP_DEF_A7_4;
    BK7011TRX.REG0xA->bits.dbpaa30 = TCP_PAMAP_DEF_A13_8;
    BK7011TRX.REG0xA->bits.disrefPA10 = TCP_PAMAP_DEF_A1_0;
    BK7011TRX.REG0xA->bits.pamapen = 1;
    CAL_WR_TRXREGS(0xA);
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value;
    
    is_tpc_used = 1;
    #endif
}

void rwnx_no_use_tpc_set_pwr(void)
{
    tpc_deinit();
    
    BK7011TRX.REG0xA->bits.pamapen = 0;
    CAL_WR_TRXREGS(0xA);
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value;
}

UINT32 rwnx_is_tpc_bit_on(void)
{
    return (BK7011TRX.REG0xA->bits.pamapen == 1)? 1: 0;
}

UINT32 rwnx_sys_is_enable_hw_tpc(void)
{
    return (is_tpc_used == 1)? 1: 0;
}

void rwnx_tpc_pa_map_init(void)
{
    BK7011RCBEKEN.REG0x70->bits.palevel0map = cfg_tab_tpc[0].value;
    BK7011RCBEKEN.REG0x70->bits.palevel1map = cfg_tab_tpc[1].value;
    bk7011_rc_val[29] = BK7011RCBEKEN.REG0x70->value;
                                                               
    BK7011RCBEKEN.REG0x71->bits.palevel2map = cfg_tab_tpc[2].value;
    BK7011RCBEKEN.REG0x71->bits.palevel3map = cfg_tab_tpc[3].value;
    bk7011_rc_val[30] = BK7011RCBEKEN.REG0x71->value;
           
    BK7011RCBEKEN.REG0x72->bits.palevel4map = cfg_tab_tpc[4].value;
    BK7011RCBEKEN.REG0x72->bits.palevel5map = cfg_tab_tpc[5].value;
    bk7011_rc_val[31] = BK7011RCBEKEN.REG0x72->value;
              
    BK7011RCBEKEN.REG0x73->bits.palevel6map = cfg_tab_tpc[6].value;
    BK7011RCBEKEN.REG0x73->bits.palevel7map = cfg_tab_tpc[7].value;
    bk7011_rc_val[32] = BK7011RCBEKEN.REG0x73->value;

    BK7011RCBEKEN.REG0x74->bits.palevel8map = cfg_tab_tpc[8].value;
    BK7011RCBEKEN.REG0x74->bits.palevel9map = cfg_tab_tpc[9].value;
    bk7011_rc_val[33] = BK7011RCBEKEN.REG0x74->value;

    BK7011RCBEKEN.REG0x75->bits.palevel10map = cfg_tab_tpc[10].value;
    BK7011RCBEKEN.REG0x75->bits.palevel11map = cfg_tab_tpc[11].value;
    bk7011_rc_val[34] = BK7011RCBEKEN.REG0x75->value;

    BK7011RCBEKEN.REG0x76->bits.palevel12map = cfg_tab_tpc[12].value;
    BK7011RCBEKEN.REG0x76->bits.palevel13map = cfg_tab_tpc[13].value;
    bk7011_rc_val[35] = BK7011RCBEKEN.REG0x76->value;

    BK7011RCBEKEN.REG0x77->bits.palevel4map = cfg_tab_tpc[14].value;
    BK7011RCBEKEN.REG0x77->bits.palevel5map = cfg_tab_tpc[15].value;
    bk7011_rc_val[36] = BK7011RCBEKEN.REG0x77->value;
    
    rwnx_use_tpc_set_pwr();
    
    os_printf("rwnx_tpc_pa_map_init\r\n");
}

void rwnx_cal_initial_calibration(void)
{
    rwnx_cal_set_txpwr(12, EVM_DEFUALT_RATE);

    rwnx_tpc_pa_map_init();
}

void rwnx_cal_set_reg_adda_ldo(UINT32 val)
{
    val = val & 0x3;
	
    BK7011TRX.REG0x12->bits.ldoadda = val;
    CAL_WR_TRXREGS(0x12);
}

void rwnx_cal_en_extra_txpa(void)
{
    BK7011TRX.REG0x10->value = BK7011TRXONLY.REG0xD->value;
    BK7011TRX.REG0x10->bits.entxfebias = 0;
    CAL_WR_TRXREGS(0x10);
}

void rwnx_cal_dis_extra_txpa(void)
{
    BK7011TRX.REG0x10->value = bk7011_trx_val[16];
    CAL_WR_TRXREGS(0x10);
}

void rwnx_cal_set_reg_rx_ldo(void)
{
    BK7011TRX.REG0x9->bits.vsrxlnaldo10 = 0x3;
    CAL_WR_TRXREGS(0x9);

    //BK7011TRX.REG0x5->bits.cp_ldo = 0x0;
    //BK7011TRX.REG0x5->bits.pll_ldo = 0x0;
    //BK7011TRX.REG0x5->bits.vco_ldo = 0x0;
    //CAL_WR_TRXREGS(0x5);
}

void rwnx_cal_set_40M_extra_setting(UINT8 val)
{
    if (1==val)
    {
        BK7011TRX.REG0xF->bits.clkadc_sel = 1;
        CAL_WR_TRXREGS(0xF);
    }
    else
    {
        BK7011TRX.REG0xF->bits.clkadc_sel = 0;
        CAL_WR_TRXREGS(0xF);
    }
}

void rwnx_cal_set_40M_setting(void)
{
    BK7011TRX.REG0x12->bits.adcrefbwsel = 1;
    BK7011TRX.REG0x12->bits.adciselc20 = 0x4;
    BK7011TRX.REG0x12->bits.adciselr20 = 0x4;
    BK7011TRX.REG0x12->bits.fictrl30 = 7;
    CAL_WR_TRXREGS(0x12);

    BK7011TRX.REG0xD->bits.lpfrxbw = 1;
    BK7011TRX.REG0xD->bits.lpftxbw = 1;
    CAL_WR_TRXREGS(0xD);
   
    BK7011TRX.REG0xE->bits.lpfrxbw = 1;
    BK7011TRX.REG0xE->bits.lpftxbw = 1;
    CAL_WR_TRXREGS(0xE);

    BK7011TRX.REG0x10->bits.lpfrxbw = 1;
    BK7011TRX.REG0x10->bits.lpftxbw = 1;
    CAL_WR_TRXREGS(0x10);

    BK7011TRX.REG0xF->bits.clkdac_sel = 1;
    BK7011TRX.REG0xF->bits.clkadc_sel = 1;
    CAL_WR_TRXREGS(0xF);
}

#if CFG_SUPPORT_MANUAL_CALI
void rwnx_cal_set_txpwr(UINT32 pwr_gain, UINT32 grate)
{
    const PWR_REGS *pcfg;

    if(pwr_gain > 32) {
        os_printf("set_txpwr unknow pwr idx:%d \r\n", pwr_gain); 
        return;
    }

	g_temp_pwr_current.idx = pwr_gain;
	g_temp_pwr_current.mode = grate;
    
#if CFG_USE_TEMPERATURE_DETECT
    INT16 shift = g_temp_pwr_current.shift;

    if(g_temp_pwr_current.mode == EVM_DEFUALT_RATE)
        shift = g_temp_pwr_current.shift_g;

    if(ble_in_dut_mode() ==0 )
    {
        if(bk7011_is_rfcali_mode_auto() == 0)
        {
            bk_printf("-----pwr_gain:%d, g_idx:%d, shift_b:%d, shift_g:%d\n",
            pwr_gain,
            g_temp_pwr_current.idx,
            g_temp_pwr_current.shift,
            g_temp_pwr_current.shift_g);
        }
    }
    pwr_gain = g_temp_pwr_current.idx + shift;

    if(pwr_gain > 32) 
    {
        pwr_gain = 32; 
    }
    if(ble_in_dut_mode() ==0 )
    {
        if(bk7011_is_rfcali_mode_auto() == 0)
            bk_printf("-----[pwr_gain]%d\n",pwr_gain);
    }
#endif // CFG_USE_TEMPERATURE_DETECT

    if(grate == EVM_DEFUALT_B_RATE) {
    // for b
        pcfg = cfg_tab_b + pwr_gain;
    } else if(grate == EVM_DEFUALT_RATE) {
    // for g
        pcfg = cfg_tab_g + pwr_gain;
    } else if(grate == EVM_DEFUALT_BLE_RATE) {
    // for BLE
        pcfg = cfg_tab_ble + pwr_gain;
    } else {
        os_printf("set_txpwr unknow rate:%d \r\n", grate);  
        return;
    }

    if(get_ate_mode_state())
    {
        if(bk7011_is_rfcali_mode_auto() == 0) 
        {
#ifdef ATE_PRINT_DEBUG
            os_printf("idx:%02d,r:%03d- pg:0x%02x, %01x, %01x, %01x, %01x, %02x, %02x, %01x,\r\n", pwr_gain, grate,
                pcfg->pregain, pcfg->regb_28_31, pcfg->regc_8_10,pcfg->regc_4_6, pcfg->regc_0_2, 
                pcfg->rega_8_13, pcfg->rega_4_7, pcfg->rega_0_1);
            os_printf("Xtal C: %d\r\n", manual_cal_get_xtal());
#else
            os_printf("idx:%02d\r\n", pwr_gain);
#endif
        }
    }

    power_save_wake_rf_if_in_sleep();
    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = gtx_pre_gain = pcfg->pregain;
    bk7011_rc_val[21] = BK7011RCBEKEN.REG0x52->value;

    BK7011TRX.REG0xA->bits.dbpab30 = pcfg->rega_4_7;
    BK7011TRX.REG0xA->bits.dbpaa30 = pcfg->rega_8_13;
    BK7011TRX.REG0xA->bits.disrefPA10 = pcfg->rega_0_1;
    CAL_WR_TRXREGS(0xA);
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value ;

    BK7011TRX.REG0xB->bits.gctrlmod30 = pcfg->regb_28_31;
    CAL_WR_TRXREGS(0xB);
    bk7011_trx_val[11] = BK7011TRXONLY.REG0xB->value ;

    BK7011TRX.REG0xC->bits.dgainpga = pcfg->regc_0_2;
    BK7011TRX.REG0xC->bits.dgainbuf30 = pcfg->regc_4_6;
    BK7011TRX.REG0xC->bits.dgainPA30 = pcfg->regc_8_10; 
    CAL_WR_TRXREGS(0xC);
    bk7011_trx_val[12] = BK7011TRXONLY.REG0xC->value ;  
	power_save_check_clr_rf_prevent_flag();    
    g_pwr_current.idx = pwr_gain;
    g_pwr_current.mode = grate;
} 

#if CFG_USE_TEMPERATURE_DETECT
void rwnx_cal_set_txpwr_by_tmpdetect(INT16 shift_b, INT16 shift_g)
{
    UINT32 should_do = 0;
    
    if(g_temp_pwr_current.shift != shift_b)
    {
        g_temp_pwr_current.shift = shift_b;
        should_do = 1;
    }
    
    if(g_temp_pwr_current.shift_g!= shift_g)
    {
        g_temp_pwr_current.shift_g = shift_g;
        should_do = 1;
    }

    if( should_do)
    {
        if(ble_in_dut_mode() ==0 )
        {
            os_printf("td set pwr: shift_b:%d, shift_g:%d, rate:%d\r\n", 
                g_temp_pwr_current.shift,
                g_temp_pwr_current.shift_g, 
                g_temp_pwr_current.mode);
        }

        if((rwnx_is_tpc_bit_on() == 0) && (is_rf_switch_to_ble() == 0))
        {
            rwnx_cal_set_txpwr(g_temp_pwr_current.idx, g_temp_pwr_current.mode);
        }
        else
        {

        }
    }
}  
#endif  // CFG_USE_TEMPERATURE_DETECT

void rwnx_cal_set_reg_mod_pa(UINT16 reg_mod, UINT16 reg_pa)
{
    power_save_wake_rf_if_in_sleep();

    gtx_dcorMod = (INT32)reg_mod,
    gtx_dcorPA = (INT32)reg_pa;
    BK7011TRXONLY.REG0xB->bits.dcorMod30 = gtx_dcorMod;
    BK7011TRXONLY.REG0xC->bits.dcorPA30 = gtx_dcorPA;    
    bk7011_trx_val[11] = BK7011TRXONLY.REG0xB->value;
    bk7011_trx_val[12] = BK7011TRXONLY.REG0xC->value; 
    power_save_check_clr_rf_prevent_flag();
}

#if CFG_USE_TEMPERATURE_DETECT
INT16 g_ble_pwr_indx = 0, g_ble_pwr_shift = 0;
#endif

void rwnx_cal_set_txpwr_ble(UINT32 pwr_gain)
{
    const PWR_REGS *pcfg;

    if(pwr_gain > 32) {
        os_printf("set_txpwr unknow pwr idx:%d \r\n", pwr_gain); 
        return;
    }

    if(is_tpc_used)
        rwnx_no_use_tpc_set_pwr();

#if CFG_USE_TEMPERATURE_DETECT
    INT16 pwr_idx;

    pwr_idx = (INT16)pwr_gain;
    pwr_idx += g_ble_pwr_shift;

    if(ble_in_dut_mode())
    {
        //os_printf("ble setpwr idx:%d, g_td_shift:%d, idx_new:%d\r\n", 
        //    pwr_gain, g_ble_pwr_shift, pwr_idx);
    }
    
    if(pwr_idx > 32)
    {
        pwr_idx = 32;
    } 
    else if(pwr_idx < 0)
    {
        pwr_idx = 0;
    }
    
    g_ble_pwr_indx = pwr_gain = pwr_idx;
#endif
    
    pcfg = cfg_tab_ble + pwr_gain;

    if(get_ate_mode_state()) {
#ifdef ATE_PRINT_DEBUG
        os_printf("idx:%02d,ble- pg:0x%02x, %01x, %01x, %01x, %01x, %02x, %02x, %01x,\r\n", pwr_gain,
            pcfg->pregain, pcfg->regb_28_31, pcfg->regc_8_10,pcfg->regc_4_6, pcfg->regc_0_2, 
            pcfg->rega_0_1, pcfg->rega_4_7);
        os_printf("Xtal C: %d\r\n", manual_cal_get_xtal());
#else	
		//os_printf("idx:%02d\r\n", pwr_gain);
#endif
    }

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = gtx_pre_gain = pcfg->pregain;
    bk7011_rc_val[21] = BK7011RCBEKEN.REG0x52->value;


    BK7011TRX.REG0xA->bits.dbpab30 = pcfg->rega_4_7;
    BK7011TRX.REG0xA->bits.dbpaa30 = pcfg->rega_8_13;
    BK7011TRX.REG0xA->bits.disrefPA10 = pcfg->rega_0_1;
    CAL_WR_TRXREGS(0xA);
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value ;

    BK7011TRX.REG0xB->bits.gctrlmod30 = pcfg->regb_28_31;
    CAL_WR_TRXREGS(0xB);
    bk7011_trx_val[11] = BK7011TRXONLY.REG0xB->value ;

    BK7011TRX.REG0xC->bits.dgainpga = pcfg->regc_0_2;
    BK7011TRX.REG0xC->bits.dgainbuf30 = pcfg->regc_4_6;
    BK7011TRX.REG0xC->bits.dgainPA30 = pcfg->regc_8_10; 
    CAL_WR_TRXREGS(0xC);
    bk7011_trx_val[12] = BK7011TRXONLY.REG0xC->value ;  
}  

#if CFG_USE_TEMPERATURE_DETECT
void rwnx_cal_set_ble_txpwr_by_tmpdetect(INT16 shift_ble)
{
    UINT32 should_do = 0;
    
    if(g_ble_pwr_shift != shift_ble)
    {
        g_ble_pwr_shift = shift_ble;
        if(shift_ble)
            should_do = 1;
    }
    
    if( should_do)
    {
        //if(ble_in_dut_mode() ==0 )
        {
            os_printf("td set ble pwr: shift:%d, cur_idx:%d\r\n", 
                g_ble_pwr_shift, 
                g_ble_pwr_indx);
        }
        
        if(is_rf_switch_to_ble())
            rwnx_cal_set_txpwr_ble(g_ble_pwr_indx);
    }
}  
#endif  // CFG_USE_TEMPERATURE_DETECT

void rwnx_cal_set_txpwr_for_ble_boardcast(void)
{
    //rwnx_cal_set_txpwr_ble(12);
    rwnx_cal_set_txpwr_by_channel(19); // channel 2440
}

void rwnx_cal_recover_txpwr_for_wifi(void)
{
    if(is_tpc_used)
    {
        rwnx_use_tpc_set_pwr();
    } 
    else
    {
        rwnx_cal_set_txpwr(g_pwr_current.idx, g_pwr_current.mode);
    }
}

void rwnx_cal_ble_set_rfconfig(void)
{
    BK7011TRX.REG0x8->bits.rssith50 = 0xf; 
    CAL_WR_TRXREGS(0x8);
}

void rwnx_cal_ble_recover_rfconfig(void)
{
    BK7011TRX.REG0x8->value = bk7011_trx_val[8]; 
    CAL_WR_TRXREGS(0x8);
}

#endif

#if CFG_USE_TEMPERATURE_DETECT
void rwnx_cal_do_temp_detect(UINT16 cur_val, UINT16 thre, UINT16 *last)
{
    TMP_PWR_PTR tmp_pwr_ptr;
    tmp_pwr_ptr = manual_cal_set_tmp_pwr(cur_val, thre, last);
    if(tmp_pwr_ptr) 
    {
		manual_cal_do_xtal_temp_delta_set(tmp_pwr_ptr->xtal_c_dlta);
        manual_cal_do_xtal_cali(cur_val, 0, 0, 0);
        
		rwnx_cal_set_txpwr_by_tmpdetect((INT16)tmp_pwr_ptr->p_index_delta, (INT16)tmp_pwr_ptr->p_index_delta_g);
        rwnx_set_tpc_txpwr_by_tmpdetect((INT16)tmp_pwr_ptr->p_index_delta, (INT16)tmp_pwr_ptr->p_index_delta_g);
        rwnx_cal_set_ble_txpwr_by_tmpdetect((INT16)tmp_pwr_ptr->p_index_delta_ble);
    }
}
#endif // CFG_USE_TEMPERATURE_DETECT

void rwnx_tx_cal_save_cal_result(void)
{
    int val;

    bk7011_trx_val[11] = (bk7011_trx_val[11] & (~(0xf << 12))) | (((0xf)&gtx_dcorMod) << 12);
    bk7011_trx_val[12] = (bk7011_trx_val[12] & (~(0xf << 12))) | (((0xf)&gtx_dcorPA) << 12);
    bk7011_rc_val[21] = (bk7011_rc_val[21] & (~(0x1f << 16))) | (((0x1f)&gtx_pre_gain) << 16);

    bk7011_rc_val[18] = (bk7011_rc_val[18] & (~(0x3ff << 16))) | (((0x3ff)&gtx_i_dc_comp) << 16);
    bk7011_rc_val[18] = (bk7011_rc_val[18] & (~0x3ff)) | ((0x3ff)&gtx_q_dc_comp);

    bk7011_rc_val[19] = (bk7011_rc_val[19] & (~(0x3ff << 16))) | (((0x3ff)&gtx_i_gain_comp) << 16);
    bk7011_rc_val[19] = (bk7011_rc_val[19] & (~0x3ff)) | ((0x3ff)&gtx_q_gain_comp);

    bk7011_trx_val[6] = (bk7011_trx_val[6] & (~(0x3f << 10))) | (((0x3f)&gtx_ifilter_corner) << 10);
    bk7011_trx_val[6] = (bk7011_trx_val[6] & (~(0x3f << 4))) | (((0x3f)&gtx_qfilter_corner) << 4);

    bk7011_rc_val[20] = (bk7011_rc_val[20] & (~(0x3ff << 16))) | (((0x3ff)&gtx_phase_comp) << 16);
    bk7011_rc_val[20] = (bk7011_rc_val[20] & (~0x3ff)) | ((0x3ff)&gtx_phase_ty2);

    bk7011_rc_val[26] = (bk7011_rc_val[26] & (~(0x3f << 16))) | (((0x3f)&gtx_ifilter_corner) << 16);
    val = gtx_ifilter_corner + g_capcal_sel;
    if(val > 0x3f)
        val = 0x3f;
    bk7011_rc_val[26] = (bk7011_rc_val[26] & (~(0x3f << 8))) | (((0x3f)&val ) << 8);
    bk7011_rc_val[26] = (bk7011_rc_val[26] & (~(0x3f << 0))) | (((0x3f)&gtx_ifilter_corner) << 0);


    bk7011_rc_val[27] = (bk7011_rc_val[27] & (~(0x3f << 16))) | (((0x3f)&gtx_qfilter_corner) << 16);
    val = gtx_qfilter_corner + g_capcal_sel;
    if(val > 0x3f)
        val = 0x3f;
    bk7011_rc_val[27] = (bk7011_rc_val[27] & (~(0x3f << 8))) | (((0x3f)&val) << 8);
    bk7011_rc_val[27] = (bk7011_rc_val[27] & (~(0x3f << 0))) | (((0x3f)&gtx_qfilter_corner) << 0);

    //if(gstat_cal)
    //bk7011_rc_val[16] = bk7011_rc_val[16] | (1 << 29);
    //else
    //bk7011_rc_val[16] = bk7011_rc_val[16] & (~(1 << 29));
}

void rwnx_rx_cal_save_cal_result(void)
{
    bk7011_trx_val[20] = g_rx_dc_gain_tab[0];
    bk7011_trx_val[21] = g_rx_dc_gain_tab[1];
    bk7011_trx_val[22] = g_rx_dc_gain_tab[2];
    bk7011_trx_val[23] = g_rx_dc_gain_tab[3];
    bk7011_trx_val[24] = g_rx_dc_gain_tab[4];
    bk7011_trx_val[25] = g_rx_dc_gain_tab[5];
    bk7011_trx_val[26] = g_rx_dc_gain_tab[6];
    bk7011_trx_val[27] = g_rx_dc_gain_tab[7];

    bk7011_rc_val[15] = (bk7011_rc_val[15] & (~(0x3ff << 16))) | (((0x3ff)&grx_amp_err_wr) << 16);
    bk7011_rc_val[15] = (bk7011_rc_val[15] & (~0x3ff)) | ((0x3ff)&grx_phase_err_wr);
}

/*******************************************************************************
* Function Implemantation
*******************************************************************************/
void bk7011_read_cal_param(void)
{
    //gtx_dc_n = (BK7011RCBEKEN.REG0x54->bits.TXDCN & 0x03) + 2;
    gst_sar_adc = ((BK7011RCBEKEN.REG0x54->bits.STSARADC & 0x03) + 1) * CAL_DELAY05US;
    gst_rx_adc = ((BK7011RCBEKEN.REG0x54->bits.STRXADC & 0x03) + 1) *  CAL_DELAY100US;
    //gconst_iqcal_p = BK7011RCBEKEN.REG0x52->bits.IQCONSTANTIQCALP - 512;
    //gconst_iqcal_p =  abs(gconst_iqcal_p);
    gconst_pout = BK7011RCBEKEN.REG0x52->bits.IQCONSTANTPOUT;

    return;
}

INT32 rwnx_cal_load_trx_rcbekn_reg_val(void)
{
    BK7011RCBEKEN.REG0x0->value  = bk7011_rc_val[0];
    BK7011RCBEKEN.REG0x1->value  = bk7011_rc_val[1];
    BK7011RCBEKEN.REG0x5->value  = bk7011_rc_val[2];
    BK7011RCBEKEN.REG0x8->value  = bk7011_rc_val[3];
    BK7011RCBEKEN.REG0xB->value  = bk7011_rc_val[4];
    BK7011RCBEKEN.REG0xE->value  = bk7011_rc_val[5];
    BK7011RCBEKEN.REG0x11->value = bk7011_rc_val[6];
    BK7011RCBEKEN.REG0x19->value = bk7011_rc_val[7];
    BK7011RCBEKEN.REG0x1C->value = bk7011_rc_val[8];
    BK7011RCBEKEN.REG0x1E->value = bk7011_rc_val[9];

    /**********NEW ADDED************/
    BK7011RCBEKEN.REG0x3C->value = bk7011_rc_val[10];
    BK7011RCBEKEN.REG0x3E->value = bk7011_rc_val[11];
    BK7011RCBEKEN.REG0x3F->value = bk7011_rc_val[12];
    BK7011RCBEKEN.REG0x40->value = bk7011_rc_val[13];
    BK7011RCBEKEN.REG0x41->value = bk7011_rc_val[14];
    BK7011RCBEKEN.REG0x42->value = bk7011_rc_val[15];
    BK7011RCBEKEN.REG0x4C->value = bk7011_rc_val[16];
    BK7011RCBEKEN.REG0x4D->value = bk7011_rc_val[17];
    BK7011RCBEKEN.REG0x4F->value = bk7011_rc_val[18];
    BK7011RCBEKEN.REG0x50->value = bk7011_rc_val[19];
    BK7011RCBEKEN.REG0x51->value = bk7011_rc_val[20];
    BK7011RCBEKEN.REG0x52->value = bk7011_rc_val[21];
    BK7011RCBEKEN.REG0x54->value = bk7011_rc_val[22];
    BK7011RCBEKEN.REG0x55->value = bk7011_rc_val[23];
    BK7011RCBEKEN.REG0x5C->value = bk7011_rc_val[24];

    BK7011RCBEKEN.REG0x4E->value = bk7011_rc_val[25];
    BK7011RCBEKEN.REG0x5A->value = bk7011_rc_val[26];
    BK7011RCBEKEN.REG0x5B->value = bk7011_rc_val[27];
    BK7011RCBEKEN.REG0x6A->value = bk7011_rc_val[28];
    BK7011RCBEKEN.REG0x70->value = bk7011_rc_val[29];
    BK7011RCBEKEN.REG0x71->value = bk7011_rc_val[30];
    BK7011RCBEKEN.REG0x72->value = bk7011_rc_val[31];
    BK7011RCBEKEN.REG0x73->value = bk7011_rc_val[32];
    BK7011RCBEKEN.REG0x74->value = bk7011_rc_val[33];
    BK7011RCBEKEN.REG0x75->value = bk7011_rc_val[34];
    BK7011RCBEKEN.REG0x76->value = bk7011_rc_val[35];
    BK7011RCBEKEN.REG0x77->value = bk7011_rc_val[36];

    //BK7011RCBEKEN.REG0x3C->bits.RXIQSWAP = 1; /* I/Q SWAP*/

    os_memcpy(grc_reg_map, bk7011_trx_val, sizeof(INT32) * 29);
    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    BK7011TRXONLY.REG0x0->value = bk7011_trx_val[0];
    BK7011TRXONLY.REG0x1->value = bk7011_trx_val[1];
    BK7011TRXONLY.REG0x2->value = bk7011_trx_val[2];
    BK7011TRXONLY.REG0x3->value = bk7011_trx_val[3];
    BK7011TRXONLY.REG0x4->value = bk7011_trx_val[4];
    BK7011TRXONLY.REG0x5->value = bk7011_trx_val[5];
    BK7011TRXONLY.REG0x6->value = bk7011_trx_val[6];
    BK7011TRXONLY.REG0x7->value = bk7011_trx_val[7];
    BK7011TRXONLY.REG0x8->value = bk7011_trx_val[8];
    BK7011TRXONLY.REG0x9->value = bk7011_trx_val[9];
    BK7011TRXONLY.REG0xA->value = bk7011_trx_val[10];
    BK7011TRXONLY.REG0xB->value = bk7011_trx_val[11];
    BK7011TRXONLY.REG0xC->value = bk7011_trx_val[12];
    BK7011TRXONLY.REG0xD->value = bk7011_trx_val[13];
    BK7011TRXONLY.REG0xE->value = bk7011_trx_val[14];
    BK7011TRXONLY.REG0xF->value = bk7011_trx_val[15];
    BK7011TRXONLY.REG0x10->value = bk7011_trx_val[16];
    BK7011TRXONLY.REG0x11->value = bk7011_trx_val[17];
    BK7011TRXONLY.REG0x12->value = bk7011_trx_val[18];
    BK7011TRXONLY.REG0x13->value = bk7011_trx_val[19];
    BK7011TRXONLY.REG0x14->value = bk7011_trx_val[20];
    BK7011TRXONLY.REG0x15->value = bk7011_trx_val[21];
    BK7011TRXONLY.REG0x16->value = bk7011_trx_val[22];
    BK7011TRXONLY.REG0x17->value = bk7011_trx_val[23];
    BK7011TRXONLY.REG0x18->value = bk7011_trx_val[24];
    BK7011TRXONLY.REG0x19->value = bk7011_trx_val[25];
    BK7011TRXONLY.REG0x1A->value = bk7011_trx_val[26];
    BK7011TRXONLY.REG0x1B->value = bk7011_trx_val[27];

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    // cal rf pll when reload trx and rc beken value
    bk7011_cal_pll();

    return 0;
}

INT32 rwnx_cal_save_trx_rcbekn_reg_val(void)
{
    bk7011_rc_val[0] = BK7011RCBEKEN.REG0x0->value ;
    bk7011_rc_val[1] = BK7011RCBEKEN.REG0x1->value ;
    bk7011_rc_val[2] = BK7011RCBEKEN.REG0x5->value ;
    bk7011_rc_val[3] = BK7011RCBEKEN.REG0x8->value ;
    bk7011_rc_val[4] = BK7011RCBEKEN.REG0xB->value ;
    bk7011_rc_val[5] = BK7011RCBEKEN.REG0xE->value ;
    bk7011_rc_val[6] = BK7011RCBEKEN.REG0x11->value;
    bk7011_rc_val[7] = BK7011RCBEKEN.REG0x19->value;
    bk7011_rc_val[8] = BK7011RCBEKEN.REG0x1C->value;
    bk7011_rc_val[9] = BK7011RCBEKEN.REG0x1E->value;

    /**********NEW ADDED************/
    bk7011_rc_val[10] = BK7011RCBEKEN.REG0x3C->value;
    bk7011_rc_val[11] = BK7011RCBEKEN.REG0x3E->value;
    bk7011_rc_val[12] = BK7011RCBEKEN.REG0x3F->value;
    bk7011_rc_val[13] = BK7011RCBEKEN.REG0x40->value;
    bk7011_rc_val[14] = BK7011RCBEKEN.REG0x41->value;
    bk7011_rc_val[15] = BK7011RCBEKEN.REG0x42->value;
    bk7011_rc_val[16] = BK7011RCBEKEN.REG0x4C->value;
    bk7011_rc_val[17] = BK7011RCBEKEN.REG0x4D->value;
    bk7011_rc_val[18] = BK7011RCBEKEN.REG0x4F->value;
    bk7011_rc_val[19] = BK7011RCBEKEN.REG0x50->value;
    bk7011_rc_val[20] = BK7011RCBEKEN.REG0x51->value;
    bk7011_rc_val[21] = BK7011RCBEKEN.REG0x52->value;
    bk7011_rc_val[22] = BK7011RCBEKEN.REG0x54->value;
    bk7011_rc_val[23] = BK7011RCBEKEN.REG0x55->value;
    bk7011_rc_val[24] = BK7011RCBEKEN.REG0x5C->value;

    bk7011_rc_val[25] = BK7011RCBEKEN.REG0x4E->value;
    bk7011_rc_val[26] = BK7011RCBEKEN.REG0x5A->value;
    bk7011_rc_val[27] = BK7011RCBEKEN.REG0x5B->value;
    bk7011_rc_val[28] = BK7011RCBEKEN.REG0x6A->value;
    bk7011_rc_val[29] = BK7011RCBEKEN.REG0x70->value;
    bk7011_rc_val[30] = BK7011RCBEKEN.REG0x71->value;
    bk7011_rc_val[31] = BK7011RCBEKEN.REG0x72->value;
    bk7011_rc_val[32] = BK7011RCBEKEN.REG0x73->value;
    bk7011_rc_val[33] = BK7011RCBEKEN.REG0x74->value;
    bk7011_rc_val[34] = BK7011RCBEKEN.REG0x75->value;
    bk7011_rc_val[35] = BK7011RCBEKEN.REG0x76->value;
    bk7011_rc_val[36] = BK7011RCBEKEN.REG0x77->value;

    //BK7011RCBEKEN.REG0x3C->bits.RXIQSWAP = 1; /* I/Q SWAP*/

    bk7011_trx_val[0]  = BK7011TRXONLY.REG0x0->value ;
    bk7011_trx_val[1]  = BK7011TRXONLY.REG0x1->value ;
    bk7011_trx_val[2]  = BK7011TRXONLY.REG0x2->value ;
    bk7011_trx_val[3]  = BK7011TRXONLY.REG0x3->value ;
    bk7011_trx_val[4]  = BK7011TRXONLY.REG0x4->value ;
    bk7011_trx_val[5]  = BK7011TRXONLY.REG0x5->value ;
    bk7011_trx_val[6]  = BK7011TRXONLY.REG0x6->value ;
    bk7011_trx_val[7]  = BK7011TRXONLY.REG0x7->value ;
    bk7011_trx_val[8]  = BK7011TRXONLY.REG0x8->value ;
    bk7011_trx_val[9]  = BK7011TRXONLY.REG0x9->value ;
    bk7011_trx_val[10] = BK7011TRXONLY.REG0xA->value ;
    bk7011_trx_val[11] = BK7011TRXONLY.REG0xB->value ;
    bk7011_trx_val[12] = BK7011TRXONLY.REG0xC->value ;
    bk7011_trx_val[13] = BK7011TRXONLY.REG0xD->value ;
    bk7011_trx_val[14] = BK7011TRXONLY.REG0xE->value ;
    bk7011_trx_val[15] = BK7011TRXONLY.REG0xF->value ;
    bk7011_trx_val[16] = BK7011TRXONLY.REG0x10->value;
    bk7011_trx_val[17] = BK7011TRXONLY.REG0x11->value;
    bk7011_trx_val[18] = BK7011TRXONLY.REG0x12->value;
    bk7011_trx_val[19] = BK7011TRXONLY.REG0x13->value;
    bk7011_trx_val[20] = BK7011TRXONLY.REG0x14->value;
    bk7011_trx_val[21] = BK7011TRXONLY.REG0x15->value;
    bk7011_trx_val[22] = BK7011TRXONLY.REG0x16->value;
    bk7011_trx_val[23] = BK7011TRXONLY.REG0x17->value;
    bk7011_trx_val[24] = BK7011TRXONLY.REG0x18->value;
    bk7011_trx_val[25] = BK7011TRXONLY.REG0x19->value;
    bk7011_trx_val[26] = BK7011TRXONLY.REG0x1A->value;
    bk7011_trx_val[27] = BK7011TRXONLY.REG0x1B->value;

    os_memcpy(grc_reg_map, bk7011_trx_val, sizeof(INT32) * 29);
    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    return 0;
}

void bk7011_cal_ready(void)
{
    rwnx_cal_load_trx_rcbekn_reg_val();

    bk7011_read_cal_param();
    rwnx_cal_load_default_result();

    cpu_delay(1000);

    BK7011RCBEKEN.REG0x4C->bits.TXCOMPDIS = 0;

    return;
}

void bk7011_cal_dpll(void)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_CALI_DPLL, NULL);
}

#if (DO_BAND_CAL)
#define BAND_CAL_GPIO_TIMES            10
#define BAND_CAL_ADD_STEP              8
#define BAND_CAL_VAL_MAX               0x7FU
#define BAND_CAL_VAL_MIN               0x00U

#include "rtos_pub.h"
#define BAND_CAL_TIMER_INTVAL          500  // ms
beken_timer_t band_timer;

void bk7011_band_cal(void)
{
    UINT32 band, band_min, band_max;
    UINT32 unlocked;

    // backup register
    gtx_band = BK7011TRX.REG0x3->bits.bandm60;

    band = BAND_CAL_VAL_MIN - BAND_CAL_ADD_STEP;
    do {
        band += BAND_CAL_ADD_STEP;
        if(band > BAND_CAL_VAL_MAX) {
            os_printf("band cal failed- band up to 127\r\n");
            goto band_exit;
        }
        
        BK7011TRX.REG0x3->bits.bandm60 = band; 
        CAL_WR_TRXREGS(0x3); 
        delay100us(10);//delay 1ms
        
        unlocked = 0;
        for(int i=0; i<BAND_CAL_GPIO_TIMES; i++) {
            if(BK7011RCBEKEN.REG0x0->bits.ch0ld)
                unlocked = 1;
        }
    }while(unlocked);

    //os_printf("found band:%02x\r\n", band);    
    band_min = band_max = band;

    do {
        if(band_min <= BAND_CAL_VAL_MIN) {
            break;
        }
        band_min--;
        
        BK7011TRX.REG0x3->bits.bandm60 = band_min; 
        CAL_WR_TRXREGS(0x3); 
        delay100us(10);//delay 1ms
        
        unlocked = 0;
        for(int i=0; i<BAND_CAL_GPIO_TIMES; i++) {
            if(BK7011RCBEKEN.REG0x0->bits.ch0ld)
                unlocked = 1;
        }
    }while(!unlocked);  

    //os_printf("found band_min:%02x\r\n", band_min);    
    
    do {
        if(band_max >= BAND_CAL_VAL_MAX) {
            os_printf("band cal failed- chspi_max up to 0x7F\r\n");
            break;
        }
        
        band_max++;
        BK7011TRX.REG0x3->bits.bandm60 = band_max; 
        CAL_WR_TRXREGS(0x3); 
        delay100us(10);//delay 1ms
        
        unlocked = 0;
        for(int i=0; i<BAND_CAL_GPIO_TIMES; i++) {
            if(BK7011RCBEKEN.REG0x0->bits.ch0ld)
                unlocked = 1;
        }
    }while(!unlocked);  

    band = (band_max + band_min) / 2;
    //os_printf("found band_max:%02x- last:%02x\r\n", band_max, band);

    gtx_band = band;
band_exit:
    // end, recovery register
    BK7011TRX.REG0x3->bits.bandm60 = gtx_band; 
    CAL_WR_TRXREGS(0x3);
    bk7011_trx_val[3] = (bk7011_trx_val[3] & (~(0x7f << 17))) | (((0x7f)&gtx_band) << 17);
}

void bk7011_band_timer_handler(void* data)
{
    UINT32 unlocked;
    
    unlocked = 0;
    for(int i=0; i<BAND_CAL_GPIO_TIMES; i++) {
        if(BK7011RCBEKEN.REG0x0->bits.ch0ld)
            unlocked = 1;
    }
    
    if(unlocked){
        bk7011_band_cal();
    }
}

void bk7011_band_detect(void)
{

    rtos_init_timer(&band_timer, 
                            BAND_CAL_TIMER_INTVAL, 
                            bk7011_band_timer_handler, 
                            (void *)0);
	rtos_start_timer(&band_timer);

}
#endif

#define BIAS_DIFF_VAL1       (4u)
#define BIAS_DIFF_VAL2       (2u)
void bk7011_cal_bias(void)
{
    UINT32 param, param2;
    //    BK7011TRX.REG0xF->bits.biascalmanual = 0;
    param = PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_CLEAN, &param);

    //    BK7011TRX.REG0xF->bits.biascaltrig = 0;
    param = PARAM_BIAS_CAL_TRIGGER_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_CLEAN, &param);
    //trx_reg_is_write(st_TRXREG0F);
    //BK7011TRXONLY.REG0xF->value = BK7011TRX.REG0xF->value;
    cpu_delay(100);
    //    BK7011TRX.REG0xF->bits.biascaltrig = 1;
    param = PARAM_BIAS_CAL_TRIGGER_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_SET, &param);

    //trx_reg_is_write(st_TRXREG0F);
    //BK7011TRXONLY.REG0xF->value = BK7011TRX.REG0xF->value;
    cpu_delay(DELAY1US * 40);//40us = 30 + 10;

    //Read SYS_CTRL.REG0x4C->bias_cal_out
    param = sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_READ, &param);
    param = (param >> PARAM_BIAS_CAL_OUT_POSI) & PARAM_BIAS_CAL_OUT_MASK;

    //First, Write SYS_CTRL.REG0x4C->ldo_val_man = bias_cal_out + BIAS_DIFF_VAL1
    param += BIAS_DIFF_VAL1;
    param2 = param;
    if (param > 0x1f) param = 0x1f;
    param = ((param & PARAM_BIAS_CAL_OUT_MASK) << PARAM_LDO_VAL_MANUAL_POSI)
            | PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);

    //Second, Write SYS_CTRL.REG0x4C->ldo_val_man = ldo_val_man - BIAS_DIFF_VAL2
    param = param2 - BIAS_DIFF_VAL2;
    if (param > 0x1f) param = 0x1f;
    gbias_after_cal = param;
    param = ((param & PARAM_BIAS_CAL_OUT_MASK) << PARAM_LDO_VAL_MANUAL_POSI)
            | PARAM_BIAS_CAL_MANUAL_BIT;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);

    return;
}

void bk7011_cal_pll(void)
{
    BK7011TRX.REG0x10->bits.enrfpll = 1;
    BK7011TRX.REG0x10->bits.endpll = 1;
    CAL_WR_TRXREGS(0x10);

    BK7011TRX.REG0x3->bits.spi_trigger = 1;
    CAL_WR_TRXREGS(0x3);
    BK7011TRX.REG0x3->bits.spi_trigger = 0;
    CAL_WR_TRXREGS(0x3);
    BK7011TRX.REG0x3->bits.errdet_spien = 1;
    CAL_WR_TRXREGS(0x3);

    cpu_delay(DELAY1US * 10);
}

void bk7011_set_rfcali_mode(int mode)
{
    #if 0
    if((mode != CALI_MODE_AUTO) && (mode != CALI_MODE_MANUAL))
    {
        os_printf("rfcali_mode 0/1\r\n");
        return;
    }
    #endif
        
    if(g_cali_mode != mode)
    {
        if(g_cali_mode == CALI_MODE_AUTO)
        {
            // change from auto to mamual
            //rwnx_cal_set_reg_mod_pa(8, 0xa);
        } 
        else
        {
            // change from manual to auto
            
        }
    }

    g_cali_mode = mode;

    save_info_item(RF_CFG_MODE_ITEM, (UINT8 *)&g_cali_mode, NULL, NULL);

    os_printf("set rfcali_mode:%d\r\n", g_cali_mode);
}

void bk7011_get_rfcali_mode(void)
{
    int cali_mode, in_valid = 1;

    if(get_info_item(RF_CFG_MODE_ITEM, (UINT8 *)&cali_mode, NULL, NULL))
    {
        if((cali_mode == CALI_MODE_AUTO) || (cali_mode == CALI_MODE_MANUAL))
        {
            g_cali_mode = cali_mode;
            os_printf("load flash rfcali mode:%d \r\n", g_cali_mode);
            in_valid = 0;
        }
        else
        {
            os_printf("rfcali_mode other:%d\r\n", cali_mode);
            in_valid = 1;
        }
    }

    if(in_valid == 1)
    {
        if(rwnx_cal_load_user_rfcali_mode)
        {
            UINT32 is_used = rwnx_cal_load_user_rfcali_mode(&cali_mode);
            if(is_used)
            {
                in_valid = 0;
                g_cali_mode = cali_mode;
                os_printf("user define rfcali mode:%d \r\n", g_cali_mode);
            }
        }
    }

    if(in_valid == 1)
    {
        if(manual_cal_txpwr_tab_ready_in_flash() != TXPWR_NONE_RD)
        {
            // found rfcali flag in flash, turn it to manual mode
            g_cali_mode = CALI_MODE_MANUAL;
            in_valid = 0;
        }
    }
        
    os_printf("\r\nrfcali_mode:%d\r\n", g_cali_mode);
}

int bk7011_is_rfcali_mode_auto(void)
{
    return (g_cali_mode == CALI_MODE_AUTO) ? 1 : 0;
}

void bk7011_set_rf_config_tssithred_b(int tssi_thred_b)
{
    if((tssi_thred_b < 0) || (tssi_thred_b > 0xff))
    {
        os_printf("b tssi range:0-255, %d\r\n", tssi_thred_b);
        return;
    }

    gtx_tssi_thred_b = tssi_thred_b;

    save_info_item(RF_CFG_TSSI_B_ITEM, (UINT8 *)&gtx_tssi_thred_b, NULL, NULL);

    os_printf("set b_tssi_thred:%d\r\n", gtx_tssi_thred_b);
}

void bk7011_set_rf_config_tssithred_g(int tssi_thred_g)
{
    if((tssi_thred_g < 0) || (tssi_thred_g > 0xff))
    {
        os_printf("g tssi range:0-255, %d\r\n", tssi_thred_g);
        return;
    }

    gtx_tssi_thred_g = tssi_thred_g;

    save_info_item(RF_CFG_TSSI_ITEM, (UINT8 *)&gtx_tssi_thred_g, NULL, NULL);

    os_printf("set g_tssi_thred:%d\r\n", gtx_tssi_thred_g);
}

void bk7011_get_txpwr_config_reg(void)
{
    int tssi_thred;

    if(bk7011_is_rfcali_mode_auto() == 0)
        return;

    // load from flash first
    if(get_info_item(RF_CFG_TSSI_ITEM, (UINT8 *)&tssi_thred, NULL, NULL))
    {
        gtx_tssi_thred_g = tssi_thred;
        os_printf("load flash tssi_th:g-%d \r\n", gtx_tssi_thred_g);
    }
    // otherwise check if user set default value
    else if(rwnx_cal_load_user_g_tssi_threshold)
    {
        UINT32 is_used = rwnx_cal_load_user_g_tssi_threshold(&tssi_thred);
        if(is_used)
        {
            gtx_tssi_thred_g = tssi_thred;
            os_printf("user define tssi_th:g-%d \r\n", gtx_tssi_thred_g);
        }
    }

    if(get_info_item(RF_CFG_TSSI_B_ITEM, (UINT8 *)&tssi_thred, NULL, NULL))
    {
        gtx_tssi_thred_b = tssi_thred;
        os_printf("load flash tssi_th:b-%d \r\n", gtx_tssi_thred_b);
    }
    else if(rwnx_cal_load_user_b_tssi_threshold)
    {
        UINT32 is_used = rwnx_cal_load_user_b_tssi_threshold(&tssi_thred);
        if(is_used)
        {
            gtx_tssi_thred_b = tssi_thred;
            os_printf("user define tssi_th:b-%d \r\n", gtx_tssi_thred_b);
        }
    }
    
    os_printf("\r\ntssi:b-%d, g-%d\r\n", gtx_tssi_thred_b, gtx_tssi_thred_g);
}

void bk7011_tx_cal_en(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 1;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;
    return;
}

#define ADV_TSSI_CNT_MAX            8
#define ADV_TSSI_MIN                0
#define ADV_DROP_THRED              50
static UINT32 bk7011_get_cnt_tssi(int cnt, UINT32 *adv_tssi, int cmp_min)
{
    UINT32 tssi_tab[ADV_TSSI_CNT_MAX], i, totoal, tssi, cnt_bak;
    //os_memset(tssi_tab, 0, sizeof(UINT32) *ADV_TSSI_CNT_MAX);

    if(cnt > ADV_TSSI_CNT_MAX)
    {
        cnt = ADV_TSSI_CNT_MAX;
    }

    totoal = 0;
    for (i = 0; i < cnt; )
    {
        cal_delay(1 * gst_sar_adc);
        tssi = BK7011RCBEKEN.REG0x54->bits.TSSIRD;
        CAL_PRT("tssi:%d", tssi);
        if((tssi <= ADV_TSSI_MIN) && (cmp_min == 1))
        {
            // this print for warning
            os_printf(" <=%d, drop\r\n", ADV_TSSI_MIN);
            continue;
        }
        CAL_PRT("\r\n", tssi);
        tssi_tab[i] = tssi;
        totoal += tssi;
        i++;
    }

    // get adv for first
    tssi = totoal / cnt;
    cnt_bak = cnt;
    totoal = 0;
    for (i = 0; i < cnt; i++)
    {
        UINT32 dist;
        
        if(tssi_tab[i] >= tssi)
            dist = tssi_tab[i] - tssi;
        else
            dist = tssi - tssi_tab[i];

        // drop 
        if(dist >= ADV_DROP_THRED)
        {
            CAL_PRT("found :|%d-%d| >= %d\r\n", tssi_tab[i], tssi, ADV_DROP_THRED);
            tssi_tab[i] = 0;
            cnt_bak--;
        }

        totoal += tssi_tab[i];
    }

    // get adv second time, cnt_bak may not equ to cnt
    if(cnt_bak == 0) // check 0
        cnt_bak = 1;
    
    tssi = totoal / cnt_bak; 
    totoal = tssi * cnt;

    if(adv_tssi)
        *adv_tssi = tssi;

    CAL_PRT("ret %d times with total %d\r\n", cnt, totoal);
    
    return  totoal;
}

static INT32 bk7011_get_tx_output_power(void)
{
    INT32 tssioutpower = 0;

    tssioutpower = bk7011_get_cnt_tssi(4, NULL, 0);

    if(gtx_power_cal_mode == TX_WANTED_POWER_CAL) //
    {

	#if DIFFERENCE_PIECES_CFG
		tssioutpower = tssioutpower / 4 - gtx_tssi_thred - gav_tssi_temp;
		//bk_printf("tssioutpower:%d\n",tssioutpower);
	#else
        tssioutpower = tssioutpower / 4 - TSSI_POUT_TH - gav_tssi_temp;
	#endif
    }
    else if(gtx_power_cal_mode == TX_IQ_POWER_CAL) //
    {

        tssioutpower = tssioutpower / 4 - TXIQ_IMB_TSSI_TH - gav_tssi_temp;
    }
    else
    {
        tssioutpower = tssioutpower / 4 - TXIQ_IMB_TSSI_TH_LOOPBACK - gav_tssi_temp;
    }


    tssioutpower = abs(tssioutpower);
    //tssioutpower = abs((INT32)(((INT32)BK7011RCBEKEN.REG0x54->bits.TSSIRD) - gconst_tssi_pout_th));

    return tssioutpower;
}


static INT32 bk7011_set_tx_pa(INT32 val1, INT32 val2, INT32 val3, INT32 val4)
{
    BK7011TRX.REG0xC->bits.dgainPA30 = val1;
    BK7011TRX.REG0xC->bits.dgainbuf30 = val2;
    BK7011TRX.REG0xC->bits.dgainpga = val3;
    BK7011TRX.REG0xB->bits.gctrlmod30 = val4;
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);

    return 0;
}

#define GET_AV_TSSI_CNT         4

INT32 bk7011_cal_tx_output_power(INT32 *val)
{
    INT32 gold_index = 0;
    INT32 tssilow = 0;
    INT32 tssihigh = 0;
    INT32 index;
    INT16 high = 0, low = 0;
    INT32 cnt = 0;

    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = 7;

    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x200;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x200;
    BK7011TRX.REG0xC->value = TRX_REG_0XC_TXLO_LEAKAGE_VAL;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
		BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x280;
        BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x280;
        BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
        BK7011RCBEKEN.REG0x0->bits.forceenable = 1;

        BK7011TRX.REG0xD->value = 0xDDF90339;
        BK7011TRX.REG0xA->value = 0x036F2075;
        BK7011TRX.REG0xB->value = 0xA7248F37;
        BK7011TRX.REG0xC->value = 0x00228765;
        BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = 0x07;
		CAL_WR_TRXREGS(0xB);
        BK7011TRX.REG0xC->bits.dgainPA30 = 0;
        BK7011TRX.REG0xC->bits.dgainbuf30 = 0;
        
        BK7011TRX.REG0xD->bits.entxsw = 1;
        BK7011TRX.REG0xD->bits.enrxsw = 0;
        
	    CAL_WR_TRXREGS(0xC);
        BK7011TRX.REG0x0->bits.TSSIsel = 1;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 0;
        BK7011TRX.REG0x0->bits.enPcal = 1;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0; 
		BK7011TRX.REG0x0->bits.PcalATT = 2;
    }
    else if(gtx_power_cal_mode == TX_IQ_POWER_CAL)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
        BK7011TRX.REG0xA->value = TRX_REG_0XA_VAL;
        BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
        BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
	    CAL_WR_TRXREGS(0xC);
        
        bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA30, gi_gain_tx_pa_dgainbuf30, 0, 8);
        BK7011TRX.REG0x0->bits.TSSIsel = 0;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 1;
        BK7011TRX.REG0x0->bits.enPcal = 0;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0;           
    }
    else
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
        BK7011TRX.REG0xA->value = TRX_REG_0XA_VAL;
        BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
        BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
	    CAL_WR_TRXREGS(0xC);
        bk7011_set_tx_pa(gi_gain_tx_loopback_pa_dgainPA30, gi_gain_tx_loopback_pa_dgainbuf30, 0, 8);
        BK7011TRX.REG0x0->bits.TSSIsel = 0;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 1;
        BK7011TRX.REG0x0->bits.enPcal = 0;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0;        
    }
    
	if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
		BK7011TRX.REG0x0->bits.tssi_statectrl = 0;
    }
	else{
    	BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    }

    BK7011TRX.REG0xD->bits.entssi = 1;
    BK7011TRX.REG0xD->bits.entssiadc = 1;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;
    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xA);
    CAL_WR_TRXREGS(0xD);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);
    BK7011TRX.REG0xB->bits.dcorMod30 = 0;
    CAL_WR_TRXREGS(0xB);
    cal_delay(150);
    cal_delay(5 * gst_sar_adc);

    gav_tssi_temp = 0;
    gav_tssi_temp = bk7011_get_cnt_tssi(GET_AV_TSSI_CNT, NULL, 0);
    cnt = 0;
    gav_tssi_temp /= GET_AV_TSSI_CNT;
    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
        gav_tssi = gav_tssi_temp;
    }

    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
      	BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x280;
        BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x280;
        BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    	
        BK7011TRX.REG0xA->value = 0x036F2075;
        BK7011TRX.REG0xB->value = 0xA7248F37;
        BK7011TRX.REG0xC->value = 0x00228765;
		CAL_WR_TRXREGS(0xC);
		CAL_WR_TRXREGS(0xB);
        
        BK7011TRX.REG0xD->bits.entxsw = 1;
        BK7011TRX.REG0xD->bits.enrxsw = 0;
		
        BK7011TRX.REG0x0->bits.TSSIsel = 1;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 0;
        BK7011TRX.REG0x0->bits.enPcal = 1;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0; 
		BK7011TRX.REG0x0->bits.PcalATT = 2;
    }
    else if(gtx_power_cal_mode == TX_IQ_POWER_CAL)
    {
        BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x230;
        BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x230;
        BK7011TRX.REG0xC->value = 0x05228245;
        CAL_WR_TRXREGS(0xC);
        bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA30, gi_gain_tx_pa_dgainbuf30, 0, 8);
        BK7011TRX.REG0x0->bits.TSSIsel = 0;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 1;
        BK7011TRX.REG0x0->bits.enPcal = 0;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0;          
    }
    else 
    {
        BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x260;
        BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x260;
        BK7011TRX.REG0xC->value = 0x05228245;
        CAL_WR_TRXREGS(0xC);
        bk7011_set_tx_pa(gi_gain_tx_loopback_pa_dgainPA30, gi_gain_tx_loopback_pa_dgainbuf30, 0, 8);
        BK7011TRX.REG0x0->bits.TSSIsel = 0;
        BK7011TRX.REG0x0->bits.enDCcal = 0;
        BK7011TRX.REG0x0->bits.enIQcal = 1;
        BK7011TRX.REG0x0->bits.enPcal = 0;
        BK7011TRX.REG0x0->bits.tssi_selrange = 0;
    }

	if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
		BK7011TRX.REG0x0->bits.tssi_statectrl = 0;
    }
	else{
    	BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    }

    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;
    BK7011TRX.REG0xC->bits.dcorPA30 = 8;

    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xA);
    CAL_WR_TRXREGS(0xD);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;

 	if(gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
        index = 0;
        tssilow = 0;
        tssihigh = 0;
        cnt = 0;

        BK7011TRX.REG0xB->bits.dcorMod30 = cnt;
        CAL_WR_TRXREGS(0xB);
        tssilow = bk7011_get_tx_output_power();
        index = cnt;
        
        CAL_PRT("cnt:%d, index:%d, tssilow:%d, tssihigh:%d\r\n",
            cnt, index, tssilow, tssihigh);
        
        cnt ++;
        
        while(cnt <= 15)
        {
            BK7011TRX.REG0xB->bits.dcorMod30 = cnt;
            CAL_WR_TRXREGS(0xB);
            tssihigh = bk7011_get_tx_output_power();

            if(tssihigh < tssilow)
            {
                index = cnt;
                tssilow = tssihigh;
            }
            
            CAL_PRT("cnt:%d, index:%d, tssilow:%d, tssihigh:%d\r\n",
                cnt, index, tssilow, tssihigh);

            cnt ++;
        }
    }
	else  
    {
        low = 0;
        high = 15;

        BK7011TRX.REG0xB->bits.dcorMod30 = low;
        CAL_WR_TRXREGS(0xB);
        cal_delay(CAL_TX_NUM);
        tssilow = bk7011_get_tx_output_power();

        BK7011TRX.REG0xB->bits.dcorMod30 = high;
        CAL_WR_TRXREGS(0xB);
        tssihigh = bk7011_get_tx_output_power();
    
        do
        {
            if(tssilow < tssihigh)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011TRX.REG0xB->bits.dcorMod30 = high;
                CAL_WR_TRXREGS(0xB);
                tssihigh = bk7011_get_tx_output_power();
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011TRX.REG0xB->bits.dcorMod30 = low;
                CAL_WR_TRXREGS(0xB);
                tssilow = bk7011_get_tx_output_power();
            }
        }
        while((high - low) > 1);
        index = ((tssilow < tssihigh) ? low : high);
    }

    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
        gtx_dcorMod = index;
    }

    CAL_PRT("gtx_dcorMod over: 0x%x\r\n", gtx_dcorMod);
    CAL_PRT("cnt:%d, index:%d, tssilow:0x%x-%d, tssihigh:0x%x-%d\r\n",
            cnt++, index, tssilow, low, tssihigh, high);

    BK7011TRX.REG0xB->bits.dcorMod30 = index;
    CAL_WR_TRXREGS(0xB);
    gold_index = index << 8;
    cal_delay(6);

    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
        low = 8;
        high = 8;
    }
    else
    {
        low = 0;
        high = 15;
    }
    
    BK7011TRX.REG0xC->bits.dcorPA30 = low;
    CAL_WR_TRXREGS(0xC);
    tssilow = bk7011_get_tx_output_power();

    BK7011TRX.REG0xC->bits.dcorPA30 = high;
    CAL_WR_TRXREGS(0xC);
    tssihigh = bk7011_get_tx_output_power();

    do
    {
        CAL_PRT("cnt:%d, index:%d, tssilow:0x%x-%d, tssihigh:0x%x-%d\r\n",
                cnt++, index, tssilow, low, tssihigh, high);
        if(tssilow < tssihigh)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xC->bits.dcorPA30 = high;
            CAL_WR_TRXREGS(0xC);
            tssihigh = bk7011_get_tx_output_power();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0xC->bits.dcorPA30 = low;
            CAL_WR_TRXREGS(0xC);
            tssilow = bk7011_get_tx_output_power();
        }
    }
    while((high - low) > 1);

    index = ((tssilow < tssihigh) ? low : high);
    if (gtx_power_cal_mode == TX_WANTED_POWER_CAL)
    {
        gtx_dcorPA = index;
    }

    BK7011TRX.REG0xC->bits.dcorPA30 = index;
    CAL_WR_TRXREGS(0xC);
    gold_index += index;

    *val = gold_index;

    CAL_PRT("gtx_dcorMod:0x%x, gtx_dcorPA:0x%x\r\n", gtx_dcorMod, gtx_dcorPA);

    return (gold_index);
}

static INT32 bk7011_get_tx_tssi(INT32 tssi_thred, INT32 tssi_offset)
{
    int i;
    INT32 tssioutpower = 0;

    for (i = 0; i < 4; i++)
    {
        cal_delay(1 * gst_sar_adc);
        tssioutpower += BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    }

    tssioutpower = tssioutpower / 4 - tssi_thred - tssi_offset;
 
    return tssioutpower;
}

#define AUTO_CAL_PTR        null_prf
#define AUTO_CAL_NPTR       null_prf
static void bk7011_do_atuo_tx_cal_print(const char *fmt, ...)
{
    if(rwnx_cal_is_auto_rfcali_printf_on)
    {
        UINT32 is_on = rwnx_cal_is_auto_rfcali_printf_on();
        if(is_on)
        {
            os_printf(fmt);
        }
    }
}

static UINT32 bk7011_do_atuo_tx_cal(UINT32 channel, UINT32 rate, INT32 tssi_thred, INT32 tssi_offset)
{
    // calc tssi
    INT32 tssi_cur = 0, tssi_prev = 0;
    UINT32 pwr_idx = 0, pwr_idx_prev = 0;
    UINT32 pwr_idx_min, tssi_abs_min;

    if((channel < 1) || (channel > 14))
    {
        AUTO_CAL_NPTR("wrong channel:%d\r\n", channel);
        return 16;
    }
    else
    {
        bk7011_do_atuo_tx_cal_print("\r\n ******** do chan:%d ********\r\n", channel);
        if(channel != 14)
        {
            channel = 12 + 5 * (channel - 1);
        }
        else
            channel = 84;
    }

    // do for channel
    BK7011TRX.REG0x5->bits.chspi = channel; 
    CAL_WR_TRXREGS(0x5);
    delay100us(1);

    pwr_idx_min = 31;
    tssi_abs_min = 255;
    pwr_idx = 16;
    
    rwnx_cal_set_txpwr(pwr_idx, rate);
    tssi_prev = tssi_cur = bk7011_get_tx_tssi(tssi_thred, tssi_offset);
    tssi_abs_min = abs(tssi_cur);

    AUTO_CAL_NPTR("init tssi:%d, idx:%d\r\n", tssi_prev, pwr_idx);

    while(tssi_cur != 0) 
    {
        pwr_idx_prev = pwr_idx;
        tssi_prev = tssi_cur;
        
        if(tssi_cur > 0)
        {
            if(pwr_idx == 0)
            {
                pwr_idx = pwr_idx_min;
                AUTO_CAL_NPTR("res 1: idx:%d\r\n", pwr_idx);
                break;
            }
            else
            {
                pwr_idx--;
            }
        }
        else
        {
            if(tssi_cur < 0)
            {
                if(pwr_idx == 31)
                {
                    pwr_idx = pwr_idx_min;
                    AUTO_CAL_NPTR("res 2: idx:%d\r\n", pwr_idx);
                    break;
                }
                else
                {
                    pwr_idx++;
                }
            }
        }
        
        rwnx_cal_set_txpwr(pwr_idx, rate);
        tssi_cur = bk7011_get_tx_tssi(tssi_thred, tssi_offset);
        
        bk7011_do_atuo_tx_cal_print("tssi:%d:%d, idx:%d:%d\r\n", 
            tssi_prev, tssi_cur, pwr_idx_prev, pwr_idx);

        if(((tssi_cur > 0) && (tssi_prev < 0)) 
            || ((tssi_cur < 0) && (tssi_prev > 0)))
        {
            if(abs(tssi_cur) == abs(tssi_prev))
            {
                pwr_idx = (pwr_idx_prev < pwr_idx)? pwr_idx_prev : pwr_idx;
            }
            else
            {
                pwr_idx = (abs(tssi_cur) <= abs(tssi_prev)) ? pwr_idx : pwr_idx_prev;
            }
            AUTO_CAL_NPTR("res 3: idx:%d\r\n", pwr_idx);
            break;
        }

        if(tssi_abs_min > abs(tssi_cur))
        {
            tssi_abs_min = abs(tssi_cur);
            pwr_idx_min = pwr_idx;
            AUTO_CAL_NPTR("bakup min: %d:%d\r\n", tssi_abs_min, pwr_idx_min);
        }

        if(tssi_cur == 0)
        {
            AUTO_CAL_NPTR("res 4: idx:%d\r\n", pwr_idx);
        }
    }

    bk7011_do_atuo_tx_cal_print("******** end idx:%d ********\r\n", pwr_idx);

    return pwr_idx;
}

extern void manual_cal_11b_2_ble(void);
INT32 bk7011_cal_auto_tx_power(void)
{
    extern UINT32 g_dif_g_n40;

    // tx sinewave setting
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x200;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x200;
    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;

    // tx chanin gain setting
    BK7011RCBEKEN.REG0x52->bits.TXPREGAIN = 0x07;
    BK7011TRX.REG0xA->value = 0x036F2075;
    BK7011TRX.REG0xB->value = 0xA7248F37;
    BK7011TRX.REG0xC->value = 0x0022A765;
    CAL_WR_TRXREGS(0xA);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xC);


    // peak detector settings
    BK7011TRX.REG0xD->value = 0xD9FF0338;
    CAL_WR_TRXREGS(0xD);

    BK7011TRX.REG0x0->value = 0x0003a24e;
    CAL_WR_TRXREGS(0x0);

    BK7011TRX.REG0xD->bits.entssi = 1;
    BK7011TRX.REG0xD->bits.entssiadc = 1;
    BK7011TRX.REG0xD->bits.entxsw = 1;
    BK7011TRX.REG0xD->bits.enrxsw = 0;
    CAL_WR_TRXREGS(0xD);

    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;
    CAL_WR_TRXREGS(0xF);

    // Set tssi offset
    INT32 tssi_offset;
    BK7011TRX.REG0xC->bits.dgainPA30 = 0;
    BK7011TRX.REG0xC->bits.dgainbuf30 = 0;
    CAL_WR_TRXREGS(0xC);

    tssi_offset = 0;
    for(int cnt = 0; cnt < GET_AV_TSSI_CNT; cnt++)
    {
        cal_delay(1 * gst_sar_adc);
        tssi_offset += BK7011RCBEKEN.REG0x54->bits.TSSIRD;
    }
    tssi_offset /= GET_AV_TSSI_CNT;
    bk7011_do_atuo_tx_cal_print("get tssi offset:%d\r\n", tssi_offset);

    BK7011TRX.REG0xC->bits.dgainPA30 = 7;
    BK7011TRX.REG0xC->bits.dgainbuf30 = 6;
    CAL_WR_TRXREGS(0xC);

    UINT32 channel, rate, pwr_idx, tssi_thred;
    
    rate = EVM_DEFUALT_RATE;
    tssi_thred = gtx_tssi_thred_g;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x260;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x260;

    bk7011_do_atuo_tx_cal_print("\r\n do 11g: tssi_thred:%d\r\n", tssi_thred);
    channel = 1;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);
    pwr_idx = (pwr_idx > g_dif_g_n40) ? (pwr_idx - g_dif_g_n40) : 0;
    manual_cal_save_txpwr(135, 3, pwr_idx);

    channel = 7;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);
    pwr_idx = (pwr_idx > g_dif_g_n40) ? (pwr_idx - g_dif_g_n40) : 0;
    manual_cal_save_txpwr(135, 7, pwr_idx);
    
    channel = 13;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);
    pwr_idx = (pwr_idx > g_dif_g_n40) ? (pwr_idx - g_dif_g_n40) : 0;
    manual_cal_save_txpwr(135, 11, pwr_idx);
    bk7011_do_atuo_tx_cal_print("end 11g: tssi_thred:%d\r\n", tssi_thred);


    rate = EVM_DEFUALT_B_RATE;
    tssi_thred = gtx_tssi_thred_b;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = 0x2c0;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = 0x2c0;
    bk7011_do_atuo_tx_cal_print("\r\n do 11b: tssi_thred:%d\r\n", tssi_thred);
    channel = 1;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);

    channel = 7;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);
    
    channel = 13;
    pwr_idx = bk7011_do_atuo_tx_cal(channel, rate, tssi_thred, tssi_offset);
    manual_cal_save_txpwr(rate, channel, pwr_idx);
    bk7011_do_atuo_tx_cal_print("end 11b: tssi_thred:%d\r\n", tssi_thred);

    manual_cal_11b_2_ble();
    manual_cal_fitting_txpwr_tab();

    // exit, recover setting
    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 0;
    
    return 0;
}

void bk7011_micopwr_config_tssi_read_prepare(void)
{
    BK7011TRX.REG0x0->bits.TSSIsel = 1;
    BK7011TRX.REG0x0->bits.enDCcal = 0;
    BK7011TRX.REG0x0->bits.enIQcal = 0;
    BK7011TRX.REG0x0->bits.enPcal = 1;
    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0xD->bits.entssi = 1;
    BK7011TRX.REG0xD->bits.entssiadc = 1;
    BK7011TRX.REG0x0->bits.tssi_selrange = 1;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;

    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xD);
    CAL_WR_TRXREGS(0xF);
}

static INT32 bk7011_update_tx_power(void)
{
    bk7011_set_tx_pa(gi_dc_tx_pa_dgainPA30, gi_dc_tx_pa_dgainbuf30, 0, 8);
    return 0;
}

static INT32 bk7011_update_tx_loopback_power(void)
{
    bk7011_set_tx_pa(gi_dc_tx_loopback_pa_dgainPA30, gi_dc_tx_loopback_pa_dgainbuf30, 0, 8);
    return 0;
}

static INT32 bk7011_get_tx_dc(void)
{
    INT32 detect_dc = 0;
    
    cpu_delay(200);

    detect_dc = bk7011_get_cnt_tssi(SUMNUMBERS, NULL, 0);

    return detect_dc;
}

INT32 bk7011_cal_tx_dc(INT32 *val)
{
    INT32 detect_dc_low = 0;
    INT32 detect_dc_high = 0;
    INT16 high = 0, low = 0;
    INT32 index, gold_index = 0;
    INT32 i_index, q_index;
    INT32 srchcnt = 0;
    INT16 search_thrd = 64;

    if(gtx_dc_cal_mode == TX_DC_CAL)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
        CAL_WR_TRXREGS(0xD);
        bk7011_update_tx_power();
        BK7011TRX.REG0xB->bits.gctrlmod30 = (TRX_REG_0XB_VAL >> 28) & 0x0F;
        CAL_WR_TRXREGS(0xB);
    }
    else if (gtx_dc_cal_mode == TX_DC_CAL_IQ)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
        CAL_WR_TRXREGS(0xD);
        bk7011_update_tx_power();
    }
    else if (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
        CAL_WR_TRXREGS(0xD);
        bk7011_update_tx_loopback_power();
    }    

    // I DC calibration;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = UNSIGNEDOFFSET10 + 0;

    if(gtx_dc_cal_mode == TX_DC_CAL)
    {
        low = UNSIGNEDOFFSET10 - MINOFFSET ;
        high = UNSIGNEDOFFSET10 + MINOFFSET ;
    }
    else if (gtx_dc_cal_mode == TX_DC_CAL_IQ)
    {
        low = UNSIGNEDOFFSET10 - MINOFFSET ;
        high = UNSIGNEDOFFSET10 + MINOFFSET ;
    }
    else if (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;
        high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;
    }

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10  - 1;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - 1 ;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;
    BK7011TRX.REG0x0->bits.TSSIsel = 0;
    BK7011TRX.REG0x0->bits.enDCcal = 1;
    BK7011TRX.REG0x0->bits.tssi_selrange = 1;
    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0x0->bits.enIQcal = 0;
    BK7011TRX.REG0x0->bits.enPcal = 0;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;

    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xF);
    cal_delay(CAL_TX_NUM);

    detect_dc_low = bk7011_get_tx_dc();


    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    //Step 1 3~6 search;
    srchcnt = 0;

    if(detect_dc_low < detect_dc_high)
    {
        high = 511;
        low = high - search_thrd;
    }
    else
    {
        low = 512;
        high = low + search_thrd;
    }

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011TRX.REG0x0->value =  0x00019041;  
    CAL_WR_TRXREGS(0x0);

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();

    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
    }
    while((high - low) > 1);
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);

    // Q DC calibration;
    //Step 1 3~6 search;
    if ((gtx_dc_cal_mode == TX_DC_CAL) || (gtx_dc_cal_mode == TX_DC_CAL_IQ))
    {
        low = UNSIGNEDOFFSET10 - MINOFFSET ;
        high = UNSIGNEDOFFSET10 + MINOFFSET ;
    }
    else if (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;
        high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;
    }

    BK7011TRX.REG0x0->value =  0x00079042;      
    CAL_WR_TRXREGS(0x0);

    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();

    if(detect_dc_low < detect_dc_high)
    {
        high = 511;
        low = high - search_thrd;
    }
    else
    {
        low = 512;
        high = low + search_thrd;
    }

    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n)
            break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011TRX.REG0x0->value =  0x00019041;  
    CAL_WR_TRXREGS(0x0);

    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
    }
    while((high - low) > 1);

    q_index = ((detect_dc_low < detect_dc_high) ? low : high);



    if(gtx_dc_cal_mode == TX_DC_CAL)
    {
        gtx_q_dc_comp = q_index + 1;
        CAL_WARN("gtx_q_dc_comp:0x%x\r\n", gtx_q_dc_comp);
    }
    else if(gtx_dc_cal_mode == TX_DC_CAL_IQ)
    {
        gtx_q_dc_comp = q_index;
        CAL_WARN("gtx_q_dc_comp_iq:0x%x\r\n", gtx_q_dc_comp);
    }
    else if (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        gtx_q_dc_comp_loopback = q_index;
        CAL_WARN("gtx_q_dc_comp_loopback_iq:0x%x\r\n", gtx_q_dc_comp_loopback);
    }
    gold_index += q_index;

    // 2nd  I DC calibration;
    //Step 1 3~6 search;
    if ((gtx_dc_cal_mode == TX_DC_CAL) || (gtx_dc_cal_mode == TX_DC_CAL_IQ))
    {
        low = UNSIGNEDOFFSET10 - MINOFFSET ;
        high = UNSIGNEDOFFSET10 + MINOFFSET ;
    }
    else// (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        low = UNSIGNEDOFFSET10 - 3 * MINOFFSET ;
        high = UNSIGNEDOFFSET10 + 3 * MINOFFSET ;
    }
    BK7011TRX.REG0x0->value =  0x00079042;   
    CAL_WR_TRXREGS(0x0);

    srchcnt = 0;
    BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = q_index;
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();

    if(detect_dc_low < detect_dc_high)
    {
        high = 511;
        low = high - search_thrd;
    }
    else
    {
        low = 512;
        high = low + search_thrd;
    }

    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();

    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
        srchcnt++;
        if(srchcnt > gtx_dc_n) break;
    }
    while((high - low) > 1);

    //Step 2  search;
    BK7011TRX.REG0x0->value =  0x00019041;  
    CAL_WR_TRXREGS(0x0);

	
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
    detect_dc_low = bk7011_get_tx_dc();
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
    detect_dc_high = bk7011_get_tx_dc();
    do
    {
        if(detect_dc_low < detect_dc_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = high;
            detect_dc_high = bk7011_get_tx_dc();

        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = low;
            detect_dc_low = bk7011_get_tx_dc();
        }
    }
    while((high - low) > 1);

    
    i_index = ((detect_dc_low < detect_dc_high) ? low : high);
    BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = i_index;
    if(gtx_dc_cal_mode == TX_DC_CAL)
    {
        gtx_i_dc_comp = i_index + 1;
        CAL_WARN("gtx_i_dc_comp:0x%x\r\n", gtx_i_dc_comp); 	 
    }
    else if(gtx_dc_cal_mode == TX_DC_CAL_IQ)
    {
        gtx_i_dc_comp = i_index;
        CAL_WARN("gtx_i_dc_comp_iq:0x%x\r\n", gtx_i_dc_comp); 	 
    }
    else if (gtx_dc_cal_mode == TX_DC_LOOPBACK_CAL_IQ)
    {
        gtx_i_dc_comp_loopback= i_index;      		
        #ifdef CALIBRATE_TIMES
        if (p_gtx_i_dc_comp_temp_array != NULL)
        {
            p_gtx_i_dc_comp_temp_array[calibrate_time] = i_index;
        }
        #endif
        CAL_WARN("gtx_i_dc_comp_loopback_iq:0x%x\r\n", gtx_i_dc_comp_loopback); 	
    }	

    gold_index += (i_index << 16);
    *val = gold_index;
    (void)index;
    
    BK7011TRX.REG0x0->value =  0x00079042;    
    CAL_WR_TRXREGS(0x0);
    
    return gold_index;
}


#define TSSI_RD_TIMES		8
static INT32 bk7011_get_tx_i_gain(void)
{
    INT32 detector_i_gain_p, detector_i_gain_n, detector_i_gain;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + 0;
    detector_i_gain_p = 0;
    detector_i_gain_p = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + 0;
    detector_i_gain_n = 0;
    detector_i_gain_n = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    detector_i_gain = detector_i_gain_p + detector_i_gain_n;
    return detector_i_gain;
}
static INT32 bk7011_get_tx_q_gain(void)
{
    INT32 detector_q_gain_p, detector_q_gain_n, detector_q_gain;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + 0;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + gconst_iqcal_p;
    detector_q_gain_p = 0;
    detector_q_gain_p = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + 0;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - gconst_iqcal_p;
    detector_q_gain_n = 0;
    detector_q_gain_n = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    detector_q_gain = detector_q_gain_p + detector_q_gain_n;
    return detector_q_gain;
}
static INT32 bk7011_get_tx_i_phase(void)
{
    INT32 detector_i_phase_n, detector_i_phase_p, detector_i_phase;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + (gconst_iqcal_p*10)/14;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - (gconst_iqcal_p*10)/14;
    detector_i_phase_p = 0;
    detector_i_phase_p = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - (gconst_iqcal_p*10)/14;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + (gconst_iqcal_p*10)/14;
    detector_i_phase_n = 0;
    detector_i_phase_n = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);
	
    detector_i_phase = detector_i_phase_p + detector_i_phase_n;
    return detector_i_phase;
}
static INT32 bk7011_get_tx_q_phase(void)
{
    INT32 detector_q_phase_n, detector_q_phase_p, detector_q_phase;

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 + (gconst_iqcal_p*10)/14;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 + (gconst_iqcal_p*10)/14;
    detector_q_phase_p = 0;
    detector_q_phase_p = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);

    BK7011RCBEKEN.REG0x4C->bits.ICONSTANT = UNSIGNEDOFFSET10 - (gconst_iqcal_p*10)/14;
    BK7011RCBEKEN.REG0x4C->bits.QCONSTANT = UNSIGNEDOFFSET10 - (gconst_iqcal_p*10)/14;
    detector_q_phase_n = 0;
    detector_q_phase_n = bk7011_get_cnt_tssi(TSSI_RD_TIMES, NULL, 1);
	
    detector_q_phase = detector_q_phase_p + detector_q_phase_n;
    return detector_q_phase;
}
static INT32 bk7011_get_rx_i_avg_signed(void)
{
    INT32 val;

    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
    return val;
}

static INT32 bk7011_get_rx_q_avg_signed(void)
{
    INT32 val;
    
    val = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
    return val;
}

INT32 bk7011_cal_tx_gain_imbalance(INT32 *val)
{
    INT32 detect_gain_low = 0;
    INT32 detect_gain_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_gain;
    INT32 detector_q_gain;
    
    if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
    {
        BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp;
        BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp;
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
        CAL_WR_TRXREGS(0xD);	
    }
    else
    {
        BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp_loopback;
        BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp_loopback;
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
        CAL_WR_TRXREGS(0xD);
    }
	
    BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;

    if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
    {
        bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA30, gi_gain_tx_pa_dgainbuf30, 3, 8);
    }
    else
    {
        bk7011_set_tx_pa(gi_gain_tx_loopback_pa_dgainPA30, gi_gain_tx_loopback_pa_dgainbuf30, 3, 8);
    }

    BK7011TRX.REG0x0->bits.TSSIsel = 0;
    BK7011TRX.REG0x0->bits.enDCcal = 0;
    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0x0->bits.tssi_selrange = 0;
    BK7011TRX.REG0x0->bits.enIQcal = 1;
    BK7011TRX.REG0x0->bits.enPcal = 0;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;
    BK7011TRX.REG0x0->value =  0x0001A24E;
    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);

    if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
    {
        gtx_i_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
        gtx_q_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
    }
    else
    {
        gtx_i_gain_comp_loopback = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
        gtx_q_gain_comp_loopback = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
    }

    cpu_delay(500);
    detector_i_gain = bk7011_get_tx_i_gain();
    detector_q_gain = bk7011_get_tx_q_gain();

    if(abs(detector_q_gain - detector_i_gain) < 3)
    {
        *val = 0;
        if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
        {
            gtx_i_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
            gtx_q_gain_comp = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
        }
        else
        {
            gtx_i_gain_comp_loopback = BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP;
            gtx_q_gain_comp_loopback = BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP;
        }

        return (BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP + (BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP  << 16));
    }

    if(detector_i_gain > detector_q_gain) // TX_Q_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = index;

		if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
		{
			gtx_i_gain_comp = index;	
		}
		else
		{
			gtx_i_gain_comp_loopback= index;	
		}		
        gold_index = (index << 16) + 1023;
    }
    else  //// TX_I_GAIN_COMP NOT CHANGED
    {
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = 1023;
        low = 0;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_low = abs(detector_i_gain - detector_q_gain);

        high = 1023;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
        detector_i_gain = bk7011_get_tx_i_gain();
        detector_q_gain = bk7011_get_tx_q_gain();

        detect_gain_high = abs(detector_i_gain - detector_q_gain);
        do
        {
            if(detect_gain_low < detect_gain_high)
            {
                index = low;
                high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = high;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_high = abs(detector_i_gain - detector_q_gain);
            }
            else
            {
                index = high;
                low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
                BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = low;
                detector_i_gain = bk7011_get_tx_i_gain();
                detector_q_gain = bk7011_get_tx_q_gain();
                detect_gain_low = abs(detector_i_gain - detector_q_gain);
            }

        }
        while((high - low) > 1);
        index = ((detect_gain_low < detect_gain_high) ? low : high);
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = index;
		if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
		{
			gtx_q_gain_comp = index;	
		}
		else
		{
			gtx_q_gain_comp_loopback= index;	
		}
        gold_index += (1023 << 16) + index;
    }

    *val = gold_index;
    if(gtx_gain_imb_cal_mode == TX_GAIN_IMB_CAL)
    {
        CAL_WARN("gtx_i_gain_comp:%d\r\n", gtx_i_gain_comp);
        CAL_WARN("gtx_q_gain_comp:%d\r\n", gtx_q_gain_comp);
    }
    
    BK7011TRX.REG0x0->value =  0x00039042;// 
    CAL_WR_TRXREGS(0x0);
    return gold_index;
}


static INT32 bk7011_cal_tx_ty2(INT32 tx_phase_comp)
{
    float ty1, ty1_sqr, ty2;
    INT32 tx_ty2;

    ty1 = -1.0 * ((tx_phase_comp - 512) * (tx_phase_comp - 512)) / (1024.0 * 1024.0);
    ty1_sqr = ty1 * ty1;
    ty2 = 1 - ty1 / 2 + 3 * ty1_sqr / 8;
    tx_ty2 = (INT32)((ty2 - 0.5) * 1024 + 0.5);

    return tx_ty2;
}

INT32 bk7011_cal_tx_phase_imbalance(INT32 *val)
{
    INT32 detect_phase_low = 0;
    INT32 detect_phase_high = 0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;
    INT32 detector_i_phase;
    INT32 detector_q_phase;

    if(gtx_phase_imb_cal_mode == TX_PHASE_IMB_CAL)
    {
        BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp;
        BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = gtx_i_gain_comp;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = gtx_q_gain_comp;
        BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_IQ_VAL;
        CAL_WR_TRXREGS(0xD);	
    }
    else
    {
        BK7011RCBEKEN.REG0x4F->bits.TXIDCCOMP = gtx_i_dc_comp_loopback;
        BK7011RCBEKEN.REG0x4F->bits.TXQDCCOMP = gtx_q_dc_comp_loopback;
        BK7011RCBEKEN.REG0x50->bits.TXIGAINCOMP = gtx_i_gain_comp_loopback;
        BK7011RCBEKEN.REG0x50->bits.TXQGAINCOMP = gtx_q_gain_comp_loopback;
       BK7011TRX.REG0xD->value = TRX_REG_0XD_TX_LOOPBACK_IQ_VAL;
        CAL_WR_TRXREGS(0xD);
    }	

    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = 512;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = 512;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 1;

	if(gtx_phase_imb_cal_mode == TX_PHASE_IMB_CAL)
	{
	    bk7011_set_tx_pa(gi_gain_tx_pa_dgainPA30, gi_gain_tx_pa_dgainbuf30, 3, 8);
	}
	else
	{
	    bk7011_set_tx_pa(gi_phase_tx_loopback_pa_dgainPA30, gi_phase_tx_loopback_pa_dgainbuf30,3, 8);
			
	}

    BK7011TRX.REG0x0->bits.TSSIsel = 0;
    BK7011TRX.REG0x0->bits.enDCcal = 0;
    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0x0->bits.tssi_selrange = 1;
    BK7011TRX.REG0x0->bits.enIQcal = 1;
    BK7011TRX.REG0x0->bits.enPcal = 0;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 1;
    BK7011TRX.REG0x0->value =  0x0001A24E;// 

    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xC);
    CAL_WR_TRXREGS(0xB);
    CAL_WR_TRXREGS(0xF);

    low = bk7011_cal_tx_ty2(512);
    low = 1 + 256;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_low = abs(detector_i_phase - detector_q_phase);

    high = 1023 - 256;
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
    BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
    detector_i_phase = bk7011_get_tx_i_phase();
    detector_q_phase = bk7011_get_tx_q_phase();
    detect_phase_high = abs(detector_i_phase - detector_q_phase);

    do
    {
        if(detect_phase_low < detect_phase_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  high;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( high);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_high = abs(detector_i_phase - detector_q_phase);
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP =  low;
            BK7011RCBEKEN.REG0x51->bits.TXTY2 = bk7011_cal_tx_ty2( low);
            detector_i_phase = bk7011_get_tx_i_phase();
            detector_q_phase = bk7011_get_tx_q_phase();
            detect_phase_low = abs(detector_i_phase - detector_q_phase);
        }
    }
    while((high - low) > 1);
    index = ((detect_phase_low < detect_phase_high) ? low : high);
    BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP = index;

	if(gtx_phase_imb_cal_mode == TX_PHASE_IMB_CAL)
	{
	    gtx_phase_comp =  BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP;
	    gtx_phase_ty2 = BK7011RCBEKEN.REG0x51->bits.TXTY2;

	    CAL_WARN("gtx_phase_comp:%d\r\n", gtx_phase_comp);
	    CAL_WARN("gtx_phase_ty2:%d\r\n", gtx_phase_ty2);
	}
	else
	{
	    gtx_phase_comp_loopback =  BK7011RCBEKEN.REG0x51->bits.TXPHASECOMP;
	    gtx_phase_ty2_loopback = BK7011RCBEKEN.REG0x51->bits.TXTY2;

	    CAL_WARN("tx_phase_comp:%d\r\n", gtx_phase_comp_loopback);
	    CAL_WARN("tx_phase_ty2:%d\r\n", gtx_phase_ty2_loopback);
	}	
    gold_index = index;
    BK7011TRX.REG0x0->value =  0x00039042;// 
    CAL_WR_TRXREGS(0x0);
    return gold_index;
}

static float bk7011_get_tx_filter_i_ratio(void)
{
    INT32 rx_avg_i_14M, rx_avg_i_500K;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 250; 
    cal_delay_100us(3*gst_rx_adc);
    rx_avg_i_14M = bk7011_get_rx_i_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 125; 
    cal_delay_100us(3*gst_rx_adc);
    rx_avg_i_500K = bk7011_get_rx_i_avg_signed();

    rx_avg_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_14M - 1.36);
    return rx_avg_ratio;
}

static float bk7011_get_tx_filter_i_ratio1(void)
{
    int i;
    INT32 rx_avg_i_14M_array[100], rx_avg_i_500K_array[100];
    INT32 rx_avg_i_14M = 0, rx_avg_i_500K = 0;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 250;
    
    cal_delay_100us(2*gst_rx_adc);
    for (i=0; i<40; i++)
    {
        cal_delay_100us(gst_rx_adc);
        rx_avg_i_14M_array[i] = bk7011_get_rx_i_avg_signed();
        rx_avg_i_14M += rx_avg_i_14M_array[i];
    }
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 125;

    cal_delay_100us(2*gst_rx_adc);
    for (i=0; i<40; i++)
    {
        cal_delay_100us(gst_rx_adc);
        rx_avg_i_500K_array[i] = bk7011_get_rx_i_avg_signed();
        rx_avg_i_500K += rx_avg_i_500K_array[i];
    }

    rx_avg_ratio = abs(1.0 * rx_avg_i_500K / rx_avg_i_14M - 1.36);
    return rx_avg_ratio;
}


static float bk7011_get_tx_filter_q_ratio(void)
{
    INT32 rx_avg_q_14M = 0, rx_avg_q_500K = 0;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 250;
    cal_delay_100us(3*gst_rx_adc);
    rx_avg_q_14M = bk7011_get_rx_q_avg_signed();
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 125;
    cal_delay_100us(3*gst_rx_adc);
    rx_avg_q_500K = bk7011_get_rx_q_avg_signed();

    rx_avg_ratio = abs(1.0 * rx_avg_q_500K / rx_avg_q_14M - 1.36);
    return rx_avg_ratio;
}

static float bk7011_get_tx_filter_q_ratio1(void)
{
    int i;
    INT32 rx_avg_q_14M_array[100], rx_avg_q_500K_array[100];
    INT32 rx_avg_q_14M = 0, rx_avg_q_500K = 0;
    float rx_avg_ratio;
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 250;
    
    cal_delay_100us(2*gst_rx_adc);
    for (i=0; i<40; i++)
    {
        cal_delay_100us(gst_rx_adc);
        rx_avg_q_14M_array[i] = bk7011_get_rx_q_avg_signed();
        rx_avg_q_14M += rx_avg_q_14M_array[i];
    }
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 125;
    
    cal_delay_100us(2*gst_rx_adc);
    for (i=0; i<40; i++)
    {
        cal_delay_100us(gst_rx_adc);
        rx_avg_q_500K_array[i] = bk7011_get_rx_q_avg_signed();
        rx_avg_q_500K += rx_avg_q_500K_array[i];
    }

    rx_avg_ratio = abs(1.0 * rx_avg_q_500K / rx_avg_q_14M - 1.36);
    return rx_avg_ratio;
}

void bk7011_get_tx_filter_corner(INT32 *tx_ifilter_corner, INT32 *tx_qfilter_corner)
{
    *tx_ifilter_corner = gtx_ifilter_corner;
    *tx_qfilter_corner = gtx_qfilter_corner;
}

void bk7011_get_tx_dc_comp(INT32 *tx_i_dc_comp, INT32 *tx_q_dc_comp)
{
    *tx_i_dc_comp = gtx_i_dc_comp;
    *tx_q_dc_comp = gtx_q_dc_comp;
}

void bk7011_get_tx_gain_comp(INT32 *tx_i_gain_comp, INT32 *tx_q_gain_comp)
{
    *tx_i_gain_comp = gtx_i_gain_comp;
    *tx_q_gain_comp = gtx_q_gain_comp;
}

void bk7011_get_tx_phase(INT32 *tx_phase_comp, INT32 *tx_phase_ty2)
{
    *tx_phase_comp = gtx_phase_comp;
    *tx_phase_ty2 = gtx_phase_ty2;
}

void bk7011_get_rx_err_wr(INT32 *rx_amp_err_wr, INT32 *rx_phase_err_wr, INT32 *rx_dc_gain_tab)
{
    *rx_phase_err_wr = grx_phase_err_wr;
    *rx_amp_err_wr = grx_amp_err_wr;
    os_memcpy(rx_dc_gain_tab, g_rx_dc_gain_tab, sizeof(g_rx_dc_gain_tab));
}

void bk7011_get_tx_tssi_thred(INT32 *tx_tssi_thred_b, INT32 *tx_tssi_thred_g)
{
    *tx_tssi_thred_b = gtx_tssi_thred_b;
    *tx_tssi_thred_g = gtx_tssi_thred_g;
}

INT32 bk7011_cal_tx_filter_corner(INT32 *val)
{
    int i;
    float float_1 = 1100.00;
    float float_2 = 1100.00;
    float tx_avg_ratio_low = 0.0;
    float tx_avg_ratio_high = 0.0;
    INT16 high, low;
    INT32 index = 0, gold_index = 0;

    do 
    {
        int ret_i, tx_ifilter, ret_q, tx_qfilter;

        if(manual_cal_need_load_cmtag_from_flash() == 0)
        {
            break;
        }
        
        ret_i = manual_cal_load_calimain_tag_from_flash(CM_TX_I_FILTER_CORNER, &tx_ifilter, sizeof(int));
        ret_q = manual_cal_load_calimain_tag_from_flash(CM_TX_Q_FILTER_CORNER, &tx_qfilter, sizeof(int));

        if((ret_i == 1) && (ret_q == 1))
        {
            // found in flash
            CAL_WARN("gtx_ifilter_corner in flash: 0x%x\r\n", tx_ifilter);
            CAL_WARN("gtx_qfilter_corner in flash: 0x%x\r\n", tx_qfilter);
        }
        else if((ret_i == 0) || (ret_q == 0))
        {
            // tag not valid
            break;
        }
        // ret < 0, means not found in flash
        else if((ret_i == -1) || (ret_q == -1))
        {
            // not tlv in flash
            manual_set_cmtag(LOAD_FROM_CALI);
            break;
        }
        else if((ret_i <= -2) || (ret_q <= -2))
        {
            // has tlv, but no CALI_MAIN_TX tag
            #if 1
            CM_SET_FLAG_BIT(CM_TX_I_FILTER_CORNER_FLAG);
            CM_SET_FLAG_BIT(CM_TX_Q_FILTER_CORNER_FLAG);
            break;
            #else
            tx_ifilter = 0xd;
            tx_ifilter = 0xd;
            CAL_FATAL("gtx_ifilter_corner default: 0x%x\r\n", tx_ifilter);
            CAL_FATAL("gtx_qfilter_corner default: 0x%x\r\n", tx_qfilter);
            #endif
        }
  
        gtx_ifilter_corner = tx_ifilter;
        gtx_qfilter_corner = tx_qfilter;
        BK7011TRX.REG0x6->bits.lpfcapcali50 = gtx_ifilter_corner;
        BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_qfilter_corner;
        CAL_WR_TRXREGS(0x6);

        return 0;
    }while(0);

    BK7011TRX.REG0x6->bits.capcal_sel = 0;
    CAL_WR_TRXREGS(0x6);

    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    // I CAL
    BK7011TRX.REG0x6->bits.lpfcapcali50 = 0x20;
    BK7011TRX.REG0xD->value = 0xFC4E03B9;
    CAL_WR_TRXREGS(0x6);
    CAL_WR_TRXREGS(0xD);

    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 0;
    BK7011TRX.REG0xF->bits.sinad_rx_en = 0;
    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xF);

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 1;
    BK7011RCBEKEN.REG0x4D->bits.TXSINAMP = 6;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 1;

    low = 0;
    BK7011TRX.REG0x6->bits.lpfcapcali50 = low;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_low = bk7011_get_tx_filter_i_ratio();

    high = 63;
    BK7011TRX.REG0x6->bits.lpfcapcali50 = high;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_high = bk7011_get_tx_filter_i_ratio();

    do
    {
        if(tx_avg_ratio_low < tx_avg_ratio_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcali50 = high;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_high = bk7011_get_tx_filter_i_ratio();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcali50 = low;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_low = bk7011_get_tx_filter_i_ratio();
        }
    }
    while((high - low) > 1);
    index = ((tx_avg_ratio_low < tx_avg_ratio_high) ? low : high);
    gold_index = index << 8;

    for (i=0; i<5; i++)
    {
        if (((index - 2 + i) >= 0) && ((index - 2 + i) < 64))
        {
            BK7011TRX.REG0x6->bits.lpfcapcali50 = index - 2 + i;
            CAL_WR_TRXREGS(0x6);

            float_2 = bk7011_get_tx_filter_i_ratio1();

            if (float_1 > float_2)
            {
                float_1 = float_2;
                gtx_ifilter_corner = index - 2 + i;
            }
        }
    }
    
    // Q CAL
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = 0x20;
    BK7011TRX.REG0xD->value = 0xFC4E03B9;
    CAL_WR_TRXREGS(0x6);
    CAL_WR_TRXREGS(0xD);

    //12/10/2014 for D version
    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 0;
    BK7011TRX.REG0xF->bits.sinad_rx_en = 0;
    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xF);

    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINAMP = 6;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 1;

    low = 0;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = low;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_low = bk7011_get_tx_filter_q_ratio();

    high = 63;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = high;
    CAL_WR_TRXREGS(0x6);

    tx_avg_ratio_high = bk7011_get_tx_filter_q_ratio();

    do
    {
        if(tx_avg_ratio_low < tx_avg_ratio_high)
        {
            index = low;
            high = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcalq50 = high;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_high = bk7011_get_tx_filter_q_ratio();
        }
        else
        {
            index = high;
            low = ((low + high) >> 1) + (((low + high) & 0x01) ? 1 : 0);
            BK7011TRX.REG0x6->bits.lpfcapcalq50 = low;
            CAL_WR_TRXREGS(0x6);
            tx_avg_ratio_low = bk7011_get_tx_filter_q_ratio();
        }
    }
    while((high - low) > 1);
    index = ((tx_avg_ratio_low < tx_avg_ratio_high) ? low : high);
    gold_index += index;

    float_1 = 1100;
    for (i=0; i<5; i++)
    {
        if (((index - 2 + i) >= 0) && ((index - 2 + i) < 64))
        {
            BK7011TRX.REG0x6->bits.lpfcapcalq50 = index - 2 + i;
            CAL_WR_TRXREGS(0x6);
            float_2 = bk7011_get_tx_filter_q_ratio1();
            if (float_1 > float_2)
            {
                float_1 = float_2;
                gtx_qfilter_corner = index - 2 + i;
            }
        }
    }

    BK7011TRX.REG0x6->bits.lpfcapcali50 = gtx_ifilter_corner;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_qfilter_corner;

    CAL_WARN("gtx_ifilter_corner over: 0x%x\r\n", gtx_ifilter_corner);
    CAL_WARN("gtx_qfilter_corner over: 0x%x\r\n", gtx_qfilter_corner);

    CAL_WR_TRXREGS(0x6);

    *val = gold_index;
    return (gold_index);
}

void bk7011_rx_cal_en(void)
{
    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 0;

    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;

    BK7011RCBEKEN.REG0x0->bits.forceenable = 1;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;

    // ADC clock change to 80M
    BK7011TRX.REG0xF->bits.clkadc_sel = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0x12->bits.adcrefbwsel = 0;
    CAL_WR_TRXREGS(0x12);
}

INT32 bk7011_cal_rx_dc(void)
{
    INT32 index = 0;
    INT32 i, j, k, t, curr, value;
    UINT32 rx_dc_gain_tab_temp[8];
    UINT32 rx_dc_gain_tab_temp1[8];

    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXHP = 0; 
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;


    BK7011TRX.REG0x5->bits.chspi = 0x0;
    CAL_WR_TRXREGS(0x5);
    delay100us(1);
    
    BK7011TRX.REG0xE->value = 0xDA01BCF0;
    CAL_WR_TRXREGS(0xE);	

    for(i = 0; i < 16; i ++)
    {
        BK7011RCBEKEN.REG0x19->bits.FCH0B = (0x70 | i);
        for(j = 0; j < 2; j ++)
        {
            index = 128;
            k = 6;
            do
            {
                //set dc offset
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
                cal_delay_100us(1);

                //read dc avg, and calc mean
                value = 0;
                //for(t = 0; t < 10; t ++)
                {
                    if(j == 0)  curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
                    else        curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
                    if(curr >= 2048) curr -= 4096;
                    value += curr;
                    //cpu_delay(100);
                }
                curr = value;

                //calc new dc offset
                if(curr > 0) index += (0x1 << k);
                else         index -= (0x1 << k);
                k --;
            }
            while((k >= 0) && ((curr >= 16) || (curr <= -16)));
            if(k < 0)
            {
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
            }
        }
    }

    rx_dc_gain_tab_temp[0] = BK7011TRXONLY.REG0x14->value;
    rx_dc_gain_tab_temp[1] = BK7011TRXONLY.REG0x15->value;
    rx_dc_gain_tab_temp[2] = BK7011TRXONLY.REG0x16->value;
    rx_dc_gain_tab_temp[3] = BK7011TRXONLY.REG0x17->value;
    rx_dc_gain_tab_temp[4] = BK7011TRXONLY.REG0x18->value;
    rx_dc_gain_tab_temp[5] = BK7011TRXONLY.REG0x19->value;
    rx_dc_gain_tab_temp[6] = BK7011TRXONLY.REG0x1A->value;
    rx_dc_gain_tab_temp[7] = BK7011TRXONLY.REG0x1B->value;

    BK7011TRX.REG0x5->bits.chspi = 0x55;
    CAL_WR_TRXREGS(0x5);
    delay100us(1);
    
    BK7011TRX.REG0xE->value = 0xDA01BCF0;
    CAL_WR_TRXREGS(0xE);	

    for(i = 0; i < 16; i ++)
    {
        BK7011RCBEKEN.REG0x19->bits.FCH0B = (0x70 | i);
        for(j = 0; j < 2; j ++)
        {
            index = 128;
            k = 6;
            do
            {
                //set dc offset
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
                cal_delay_100us(1);

                //read dc avg, and calc mean
                value = 0;
                //for(t = 0; t < 10; t ++)
                {
                    if(j == 0)  curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
                    else        curr = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
                    if(curr >= 2048) curr -= 4096;
                    value += curr;
                    
                    //cpu_delay(100);
                }
                curr = value;

                //calc new dc offset
                if(curr > 0) index += (0x1 << k);
                else         index -= (0x1 << k);
                k --;
            }
            while((k >= 0) && ((curr >= 16) || (curr <= -16)));
            if(k < 0)
            {
                value = (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4)));
                curr = ~(0xff << (16 * (i % 2) + 8 * j));
                value &= curr;
                curr = (index << (16 * (i % 2) + 8 * j));
                value |= curr;
                (*((volatile unsigned long *)(TRX_BEKEN_BASE + (0x14 + i / 2) * 4))) = value;
                while(BK7011RCBEKEN.REG0x1->value & 0xfffffff);
            }
        }
    }

    rx_dc_gain_tab_temp1[0] = BK7011TRXONLY.REG0x14->value;
    rx_dc_gain_tab_temp1[1] = BK7011TRXONLY.REG0x15->value;
    rx_dc_gain_tab_temp1[2] = BK7011TRXONLY.REG0x16->value;
    rx_dc_gain_tab_temp1[3] = BK7011TRXONLY.REG0x17->value;
    rx_dc_gain_tab_temp1[4] = BK7011TRXONLY.REG0x18->value;
    rx_dc_gain_tab_temp1[5] = BK7011TRXONLY.REG0x19->value;
    rx_dc_gain_tab_temp1[6] = BK7011TRXONLY.REG0x1A->value;
    rx_dc_gain_tab_temp1[7] = BK7011TRXONLY.REG0x1B->value;

    for (i=0; i<8; i++)
    {
        g_rx_dc_gain_tab[i] = ((((rx_dc_gain_tab_temp[i] & 0x000000FF) + (rx_dc_gain_tab_temp1[i] & 0x000000FF)) / 2) & 0x000000FF)
                            | ((((rx_dc_gain_tab_temp[i] & 0x0000FF00) + (rx_dc_gain_tab_temp1[i] & 0x0000FF00)) / 2) & 0x0000FF00)
                            | ((((rx_dc_gain_tab_temp[i] & 0x00FF0000) + (rx_dc_gain_tab_temp1[i] & 0x00FF0000)) / 2) & 0x00FF0000)
                            | ((((((rx_dc_gain_tab_temp[i] >> 8) & 0x00FF0000) + ((rx_dc_gain_tab_temp1[i] >> 8) & 0x00FF0000)) / 2) & 0x00FF0000) << 8);
    }

    BK7011TRXONLY.REG0x14->value = g_rx_dc_gain_tab[0];
    CAL_WR_TRXREGS(0x14);
    BK7011TRXONLY.REG0x15->value = g_rx_dc_gain_tab[1];
    CAL_WR_TRXREGS(0x15);
    BK7011TRXONLY.REG0x16->value = g_rx_dc_gain_tab[2];
    CAL_WR_TRXREGS(0x16);
    BK7011TRXONLY.REG0x17->value = g_rx_dc_gain_tab[3];
    CAL_WR_TRXREGS(0x17);
    BK7011TRXONLY.REG0x18->value = g_rx_dc_gain_tab[4];
    CAL_WR_TRXREGS(0x18);
    BK7011TRXONLY.REG0x19->value = g_rx_dc_gain_tab[5];
    CAL_WR_TRXREGS(0x19);
    BK7011TRXONLY.REG0x1A->value = g_rx_dc_gain_tab[6];
    CAL_WR_TRXREGS(0x1A);
    BK7011TRXONLY.REG0x1B->value = g_rx_dc_gain_tab[7];
    CAL_WR_TRXREGS(0x1B);
	
    CAL_WARN("g_rx_dc_gain_tab 0 over: 0x%x\r\n", g_rx_dc_gain_tab[0]);
    CAL_WARN("g_rx_dc_gain_tab 1 over: 0x%x\r\n", g_rx_dc_gain_tab[1]);
    CAL_WARN("g_rx_dc_gain_tab 2 over: 0x%x\r\n", g_rx_dc_gain_tab[2]);
    CAL_WARN("g_rx_dc_gain_tab 3 over: 0x%x\r\n", g_rx_dc_gain_tab[3]);
    CAL_WARN("g_rx_dc_gain_tab 4 over: 0x%x\r\n", g_rx_dc_gain_tab[4]);
    CAL_WARN("g_rx_dc_gain_tab 5 over: 0x%x\r\n", g_rx_dc_gain_tab[5]);
    CAL_WARN("g_rx_dc_gain_tab 6 over: 0x%x\r\n", g_rx_dc_gain_tab[6]);
    CAL_WARN("g_rx_dc_gain_tab 7 over: 0x%x\r\n", g_rx_dc_gain_tab[7]);

    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 0;


    BK7011TRX.REG0x5->bits.chspi = 0xc;
    CAL_WR_TRXREGS(0x5);
    delay100us(1);
    
    return 0;
}

INT32 bk7011_cal_rx_iq(INT32 *val)
{
    INT32 rx_amp_err_rd, rx_phase_err_rd, rx_ty2_rd;
    INT32 rx_amp_err_wr;
    INT32 rx_phase_err_wr;
    float amp_err, phase_err, ty2_err;
    INT32 gold_index = 0;
    INT32 i, curr, value, value1, value2;

    do 
    {
        int ret_i, tx_ifilter, ret_q, tx_qfilter;

        if(manual_cal_need_load_cmtag_from_flash() == 0)
        {
            break;
        }
        
        ret_i = manual_cal_load_calimain_tag_from_flash(CM_RX_AMP_ERR_WR, &rx_amp_err_wr, sizeof(int));
        ret_q = manual_cal_load_calimain_tag_from_flash(CM_RX_PHASE_ERR_WR, &rx_phase_err_wr, sizeof(int));

        if((ret_i == 1) && (ret_q == 1))
        {
            // found in flash
            CAL_WARN("grx_amp_err_wr in flash: 0x%x\r\n", rx_amp_err_wr);
            CAL_WARN("grx_phase_err_wr in flash: 0x%x\r\n", rx_phase_err_wr);
        }
        else if((ret_i == 0) || (ret_q == 0))
        {
            // tag not valid
            break;
        }
        // ret < 0, means not found in flash
        else if((ret_i == -1) || (ret_q == -1))
        {
            // not tlv in flash
            manual_set_cmtag(LOAD_FROM_CALI);
            break;
        }
        else if((ret_i <= -2) || (ret_q <= -2))
        {
            // has tlv, but no CALI_MAIN_TX tag
            CM_SET_FLAG_BIT(CM_RX_AMP_ERR_WR_FLAG);
            CM_SET_FLAG_BIT(CM_RX_PHASE_ERR_WR_FLAG);
            break;
        }

        grx_amp_err_wr = rx_amp_err_wr;
        grx_phase_err_wr = rx_phase_err_wr;
        BK7011RCBEKEN.REG0x42->bits.RXAMPERRWR = rx_amp_err_wr;
        BK7011RCBEKEN.REG0x42->bits.RXPHASEERRWR = rx_phase_err_wr;

        return 0;
    }while(0);

    /*step 1*/
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;

    BK7011TRX.REG0x0->bits.tssi_statectrl = 1;
    BK7011TRX.REG0xF->bits.tssi_cal_en = 0;
    BK7011TRX.REG0xF->bits.sinad_rx_en = 0;
    CAL_WR_TRXREGS(0x0);
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0xE->value = TRX_REG_0XE_RXIQ_VAL;
    CAL_WR_TRXREGS(0xE);
    bk7011_set_tx_pa(gi_cal_rx_iq_pa_dgainPA30, gi_cal_rx_iq_pa_dgainbuf30, 4, 4);	

    BK7011RCBEKEN.REG0x19->bits.FCH0B = 0x1a;
    
    /*searching...*/
    BK7011RCBEKEN.REG0x3E->bits.RXCALEN = 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 2;
    BK7011RCBEKEN.REG0x4D->bits.TXSINMODE = 0;
    BK7011RCBEKEN.REG0x4D->bits.TXSINAMP = 0x04;  //  increase 6dB
    BK7011RCBEKEN.REG0x4D->bits.TXSINF = 179; 
    cal_delay_100us(6*gst_rx_adc);
    cpu_delay(500 * DELAY1US);

    BK7011RCBEKEN.REG0x41->bits.RXDCIWR = 0x0;
    BK7011RCBEKEN.REG0x41->bits.RXDCQWR = 0x0;

    value = 0;
    value1 = 0;
    value2 = 0;
    for(i = 0; i < 2; i ++)
    {
        curr = BK7011RCBEKEN.REG0x3F->bits.RXAMPERRRD;
        value += curr - ((curr < 512) ? 0: 1024);
        curr = BK7011RCBEKEN.REG0x3F->bits.RXPHASEERRRD;
        value1 += curr - ((curr < 512) ? 0: 1024);
        curr = BK7011RCBEKEN.REG0x40->bits.RXTY2RD;
        value2 += curr - ((curr < 512) ? 0: 1024);
        cpu_delay(gst_rx_adc);
    }
    rx_amp_err_rd = value / 2;
    rx_phase_err_rd = value1 / 2;
    rx_ty2_rd = value2 / 2;

    CAL_WARN("[rx_iq]rx_amp_err_rd: 0x%03x\r\n", rx_amp_err_rd );
    CAL_WARN("[rx_iq]rx_phase_err_rd: 0x%03x\r\n", rx_phase_err_rd );    
    CAL_WARN("[rx_iq]rx_ty2_rd: 0x%03x\r\n", rx_ty2_rd );    

    amp_err = 1.0 * rx_amp_err_rd / 1024;
    phase_err = 1.0 * rx_phase_err_rd / 1024;
    ty2_err = 1.0 * rx_ty2_rd / 1024;

    rx_amp_err_wr = (INT32) (512 * (ty2_err + 1) / (amp_err + 1));
    rx_phase_err_wr = (INT32) (512 * phase_err * (ty2_err + 1));

    BK7011RCBEKEN.REG0x42->bits.RXPHASEERRWR = rx_phase_err_wr;
    BK7011RCBEKEN.REG0x42->bits.RXAMPERRWR = rx_amp_err_wr;
    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 1;
    BK7011RCBEKEN.REG0x3E->bits.RXCALEN = 0;

    grx_amp_err_wr = rx_amp_err_wr;
    grx_phase_err_wr = rx_phase_err_wr;

    CAL_WARN("grx_amp_err_wr:0x%03x\r\n", grx_amp_err_wr);
    CAL_WARN("grx_phase_err_wr:0x%03x\r\n", grx_phase_err_wr);

    gold_index = (rx_amp_err_wr << 16 ) + rx_phase_err_wr;
    *val = gold_index;
	
    BK7011TRX.REG0x9->bits.agcrxfeEn = 1; //enable agc  
    BK7011TRX.REG0x7->bits.autorxifgen = 1;//ensable agc 
    CAL_WR_TRXREGS(0x7);
    CAL_WR_TRXREGS(0x9);

    return gold_index;
}

void bk7011_set_rx_avg_dc(void)
{
    INT32 rx_dc_i_rd, rx_dc_q_rd;

    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 0;
    BK7011RCBEKEN.REG0x3C->bits.RXAVGMODE = 0;
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 1;
    cal_delay_100us(gst_rx_adc);
    BK7011RCBEKEN.REG0x3C->bits.RXDCCALEN = 0;

    rx_dc_i_rd = BK7011RCBEKEN.REG0x3C->bits.RXAVGIRD;
    rx_dc_q_rd = BK7011RCBEKEN.REG0x3C->bits.RXAVGQRD;
    BK7011RCBEKEN.REG0x41->bits.RXDCIWR = rx_dc_i_rd;
    BK7011RCBEKEN.REG0x41->bits.RXDCQWR = rx_dc_q_rd;

    BK7011RCBEKEN.REG0x3E->bits.RXCOMPEN = 1;

    return;
}

INT32 bk7011_load_calibration_cfg(void)
{
    BK7011RCBEKEN.REG0x0->value  = BK7011RCBEKEN.REG0x0->value;
    BK7011RCBEKEN.REG0x1->value  = BK7011RCBEKEN.REG0x1->value;
    BK7011RCBEKEN.REG0x5->value  = BK7011RCBEKEN.REG0x5->value ;
    BK7011RCBEKEN.REG0x8->value  = BK7011RCBEKEN.REG0x8->value;
    BK7011RCBEKEN.REG0xB->value  = BK7011RCBEKEN.REG0xB->value;
    BK7011RCBEKEN.REG0x11->value = BK7011RCBEKEN.REG0x11->value;
    BK7011RCBEKEN.REG0x19->value = BK7011RCBEKEN.REG0x19->value;
    BK7011RCBEKEN.REG0x1E->value = BK7011RCBEKEN.REG0x1E->value;

    /**********NEW ADDED************/
    BK7011RCBEKEN.REG0x3C->value = BK7011RCBEKEN.REG0x3C->value;
    BK7011RCBEKEN.REG0x3E->value = BK7011RCBEKEN.REG0x3E->value;
    BK7011RCBEKEN.REG0x3F->value = BK7011RCBEKEN.REG0x3F->value;
    BK7011RCBEKEN.REG0x40->value = BK7011RCBEKEN.REG0x40->value;
    BK7011RCBEKEN.REG0x41->value = BK7011RCBEKEN.REG0x41->value;
    BK7011RCBEKEN.REG0x42->value = BK7011RCBEKEN.REG0x42->value ;
    BK7011RCBEKEN.REG0x4C->value = BK7011RCBEKEN.REG0x4C->value;
    BK7011RCBEKEN.REG0x4D->value = BK7011RCBEKEN.REG0x4D->value;
    BK7011RCBEKEN.REG0x4F->value = BK7011RCBEKEN.REG0x4F->value;
    BK7011RCBEKEN.REG0x50->value = BK7011RCBEKEN.REG0x50->value;
    BK7011RCBEKEN.REG0x51->value = BK7011RCBEKEN.REG0x51->value;
    BK7011RCBEKEN.REG0x52->value = BK7011RCBEKEN.REG0x52->value;
    BK7011RCBEKEN.REG0x54->value = BK7011RCBEKEN.REG0x54->value;
    BK7011RCBEKEN.REG0x5C->value = BK7011RCBEKEN.REG0x5C->value;

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }
    BK7011TRXONLY.REG0x0->value = grc_reg_map[0];
    BK7011TRXONLY.REG0x1->value = grc_reg_map[1];
    BK7011TRXONLY.REG0x2->value = grc_reg_map[2];
    BK7011TRXONLY.REG0x3->value = grc_reg_map[3];
    BK7011TRXONLY.REG0x4->value = grc_reg_map[4];
    BK7011TRXONLY.REG0x6->value = grc_reg_map[6];
    BK7011TRXONLY.REG0x7->value = grc_reg_map[7];
    BK7011TRXONLY.REG0x8->value = grc_reg_map[8];
    BK7011TRXONLY.REG0x9->value = grc_reg_map[9];
    BK7011TRXONLY.REG0xA->value = grc_reg_map[10];
    BK7011TRXONLY.REG0xB->value = grc_reg_map[11];
    BK7011TRXONLY.REG0xC->value = grc_reg_map[12];
    BK7011TRXONLY.REG0xD->value = grc_reg_map[13];
    BK7011TRXONLY.REG0xE->value = grc_reg_map[14];
    BK7011TRXONLY.REG0x11->value = grc_reg_map[17];
    BK7011TRXONLY.REG0x12->value = grc_reg_map[18];
    BK7011TRXONLY.REG0x13->value = grc_reg_map[19];
    BK7011TRXONLY.REG0x14->value = grc_reg_map[20];
    BK7011TRXONLY.REG0x15->value = grc_reg_map[21];
    BK7011TRXONLY.REG0x16->value = grc_reg_map[22];
    BK7011TRXONLY.REG0x17->value = grc_reg_map[23];
    BK7011TRXONLY.REG0x18->value = grc_reg_map[24];
    BK7011TRXONLY.REG0x19->value = grc_reg_map[25];
    BK7011TRXONLY.REG0x1A->value = grc_reg_map[26];
    BK7011TRXONLY.REG0x1B->value = grc_reg_map[27];

    while(BK7011RCBEKEN.REG0x1->value & 0x0FFFFFFF)
    {
        cpu_delay(1);
    }

    BK7011TRX.REG0x7->bits.autorxifgen = 0;
    BK7011TRX.REG0x7->bits.dig_dcoen = 0;
    BK7011TRX.REG0x7->bits.spilpfrxg30 = 6;
    CAL_WR_TRXREGS(0x7);

    BK7011TRX.REG0x6->bits.lpfcapcali50 = gtx_ifilter_corner;
    BK7011TRX.REG0x6->bits.lpfcapcalq50 = gtx_qfilter_corner;
    CAL_WR_TRXREGS(0x6);

    bk7011_set_rx_avg_dc(); // 11/11/2014

    return 0;
}

void bk7011_set_tx_after_cal(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 0;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 1;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 0;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 1;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 0;
	BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
}

void bk7011_set_rx_after_cal(void)
{
    BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
    cpu_delay(1);
    BK7011RCBEKEN.REG0x19->bits.FCH0EN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0SHDN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0RXEN = 1;
    BK7011RCBEKEN.REG0x19->bits.FCH0TXEN = 0;
    BK7011RCBEKEN.REG0x1C->bits.FRXON = 1;
    BK7011RCBEKEN.REG0x1C->bits.FTXON = 0;
    BK7011RCBEKEN.REG0x4C->bits.TESTPATTERN = 0;
    BK7011RCBEKEN.REG0x0->bits.forceenable = 0;
    BK7011TRX.REG0xE->value = 0xDA01BCF0;
    CAL_WR_TRXREGS(0xE);

    BK7011TRX.REG0xF->bits.clkadc_sel = 0;
    CAL_WR_TRXREGS(0xF);

    BK7011TRX.REG0x12->bits.adcrefbwsel = 0;
    CAL_WR_TRXREGS(0x12);
}

#define CALI_DPD_TEST       0
#if CALI_DPD_TEST
#define I_TABLE_ADDR        0x01050400
#define Q_TABLE_ADDR        0x01050600

static UINT16 i_table_val[256] =
{
    0, 6, 13, 19, 26, 35, 40, 47, 52, 57, 68, 73, 76, 82, 88, 91, 96, 102, 107, 107, 118, 118, 120, 127, 132, 134, 139, 141, 146, 149, 152, 158, 161, 161, 163, 164, 168, 172, 172, 176, 181, 177, 179, 181, 185, 187, 189, 185, 191, 195, 196, 195, 196, 197, 203, 198, 204, 201, 207, 199, 206, 207, 207, 207, 207, 210, 210, 212, 214, 215, 215, 215, 206, 216, 215, 221, 217, 219, 215, 219, 222, 222, 225, 229, 225, 223, 228, 226, 226, 229, 229, 226, 225, 227, 226, 226, 228, 232, 230, 229, 230, 231, 230, 231, 234, 235, 236, 238, 241, 244, 245, 247, 248, 251, 252, 255, 255, 258, 259, 262, 263, 265, 267, 268, 271, 272, 275, 275, 278, 280, 282, 284, 287, 288, 291, 293, 295, 297, 299, 301, 304, 306, 308, 310, 312, 314, 317, 319, 321, 323, 325, 327, 330, 332, 334, 336, 338, 341, 343, 345, 347, 349, 351, 354, 356, 358, 360, 362, 364, 367, 369, 371, 373, 375, 377, 380, 382, 384, 386, 388, 390, 393, 395, 397, 399, 401, 403, 406, 408, 410, 412, 414, 416, 419, 421, 423, 425, 427, 429, 432, 434, 436, 438, 440, 442, 445, 447, 449, 451, 453, 455, 458, 460, 462, 464, 466, 468, 471, 473, 475, 477, 479, 481, 484, 486, 488, 490, 492, 495, 497, 499, 501, 503, 505, 508, 510, 512, 514, 516, 518, 521, 523, 525, 527, 529, 531, 534, 536, 538, 540, 542, 544, 547, 549, 551, 562
};

static UINT16 q_table_val[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1, 5, 8, 5, 9, 6, 5, 7, 4, 7, 8, 17, 13, 12, 14, 15, 12, 12, 18, 12, 13, 16, 16, 17, 19, 20, 24, 22, 30, 23, 21, 24, 30, 27, 26, 24, 27, 26, 30, 28, 30, 32, 31, 31, 32, 32, 33, 35, 35, 33, 35, 34, 32, 32, 32, 34, 33, 32, 31, 32, 30, 33, 29, 30, 29, 30, 29, 29, 28, 27, 29, 27, 28, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51, 52, 52, 48
};

UINT32 bk7211_cal_tx_dpd_load(void)
{

    UINT16 *i_tbl_addr = (UINT16 *)I_TABLE_ADDR;
    UINT16 *q_tbl_addr = (UINT16 *)Q_TABLE_ADDR;
    UINT16 k;

    BK7011RCBEKEN.REG0x4C->bits.DPDEN = 0;

    os_memcpy(i_tbl_addr, (UINT16 *)&i_table_val[0], 256 * 2);
    os_memcpy(q_tbl_addr, (UINT16 *)&q_table_val[0], 256 * 2);

#if 1
    for(k = 0; k < 256; k++)
    {
        i_tbl_addr[k] = 1;
        q_tbl_addr[k] = 0;
    }
#endif
    return 0;

}
#endif

UINT32 bk7011_set_ldo_manual(void)
{
    UINT32 param, val, param_bak;
    param_bak = param = sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_READ, &param);
    val = (param >> PARAM_LDO_VAL_MANUAL_POSI) & PARAM_LDO_VAL_MANUAL_MASK;

    val = (val > 2)? (val - 2) : 0;
    param &= ~(PARAM_LDO_VAL_MANUAL_MASK << PARAM_LDO_VAL_MANUAL_POSI);
    param |= ((val & PARAM_LDO_VAL_MANUAL_MASK) << PARAM_LDO_VAL_MANUAL_POSI);
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);

    return param_bak;
}

void bk7011_recover_ldo_manual(UINT32 param)
{
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BIAS_REG_WRITE, &param);
}

void sctrl_dpll_int_open(void);
void calibration_main(void)
{
    INT32 goldval[32] = {0};
    UINT32 param_bak, ldoadda_bak;
    
    CM_CLR_FLAG_ALLBIT();

	bk7011_get_rfcali_mode();
	bk7011_get_txpwr_config_reg();
    
    memcpy(bk7011_trx_val, bk7011_trx_val1, sizeof(bk7011_trx_val));
    memcpy(bk7011_rc_val, bk7011_rc_val1, sizeof(bk7011_rc_val));

    BK7011RCBEKEN.REG0x4C->bits.TXCOMPDIS = 0;    
    bk7011_cal_ready();
    bk7011_cal_bias();

    // setting for tssi stable
    param_bak = bk7011_set_ldo_manual();
    ldoadda_bak = BK7011TRX.REG0x12->bits.ldoadda;
    rwnx_cal_set_reg_adda_ldo(0);

    //bk7011_doubler_cal();
    //bk7011_band_cal();

    //bk7011_cal_pll();
    //bk7011_rfpll_ld();

    bk7011_cal_dpll();
    sctrl_dpll_int_open();

    BK7011TRX.REG0xA->value = TRX_REG_0XA_VAL;
    CAL_WR_TRXREGS(0xA);
    BK7011TRX.REG0xB->value = TRX_REG_0XB_VAL;
    CAL_WR_TRXREGS(0xB);
    BK7011TRX.REG0xC->value = TRX_REG_0XC_VAL;
    CAL_WR_TRXREGS(0xC);
    BK7011RCBEKEN.REG0x3C->bits.RXHPFBYPASS = 1;

    BK7011TRX.REG0x5->bits.chspi = 0x00;
    CAL_WR_TRXREGS(0x5);
    delay100us(1);
	
    bk7011_tx_cal_en();
    bk7011_cal_bias();
    bk7011_cal_tx_filter_corner(goldval);    
    
	gtx_power_cal_mode = TX_IQ_POWER_CAL;	
    bk7011_cal_tx_output_power(goldval);  
    
    //bk7011_cal_bias();//second cal after PA on;

    //bk7011_cal_pll();//second cal after PA on;
    //delay100us(1);//delay 100us for RFPLL
    
  //  gtx_power_cal_mode = TX_IQ_POWER_CAL;	
   // bk7011_cal_tx_output_power(goldval);  // real PA cal.  

    BK7011TRX.REG0xB->bits.gctrlmod30 = 0x8;
    CAL_WR_TRXREGS(0xB);

    gtx_dc_cal_mode = TX_DC_CAL_IQ;
    bk7011_cal_tx_dc(goldval);
    
    gtx_gain_imb_cal_mode = TX_GAIN_IMB_CAL;    
    bk7011_cal_tx_gain_imbalance(goldval);
	
    gtx_phase_imb_cal_mode = TX_PHASE_IMB_CAL;  
    bk7011_cal_tx_phase_imbalance(goldval);

    gtx_dc_cal_mode = TX_DC_CAL;
    bk7011_cal_tx_dc(goldval);

    BK7011TRX.REG0x5->bits.chspi = 0x5d;
    CAL_WR_TRXREGS(0x5);
    delay100us(1);
    
    bk7011_cal_bias();

    if(bk7011_is_rfcali_mode_auto())
    {
        bk7011_cal_auto_tx_power();
    }
    // set pa & mod to 0xa 8 ?
    gtx_dcorPA = 0xa;
	gtx_dcorMod = 8;

    bk7011_set_tx_after_cal();
    rwnx_tx_cal_save_cal_result();

    BK7011TRX.REG0x5->bits.chspi = 0x5d;//2495MHz
    CAL_WR_TRXREGS(0x5);
    delay100us(1);

    bk7011_rx_cal_en();
    bk7011_cal_rx_dc();
    bk7011_cal_rx_iq(goldval);


    bk7011_load_calibration_cfg();
    bk7011_set_rx_after_cal();

    rwnx_rx_cal_save_cal_result();
    rwnx_cal_load_default_result();
    rwnx_cal_read_current_cal_result();
    BK7011RCBEKEN.REG0x4C->bits.TXCOMPDIS = 0;
	
    rwnx_tx_cal_save_cal_result();
    rwnx_cal_load_trx_rcbekn_reg_val();

    bk7011_cal_dpll();
    sctrl_dpll_int_open();
    
    // set i&q lpfcap select from digit in default
    rwnx_cal_enable_lpfcap_iq_from_digit(1);

    // recover for tssi stable
    bk7011_recover_ldo_manual(param_bak);
    rwnx_cal_set_reg_adda_ldo(ldoadda_bak);

    if(manual_cal_is_tlv_tag_in_flash() == 0) 
    {
        os_printf("NO RF TLV in flash, write def tab\r\n");
        // only auto cali mode need save default tab
        if(bk7011_is_rfcali_mode_auto())
        {
            manual_cal_save_chipinfo_tab_to_flash();
    		manual_cal_save_txpwr_tab_to_flash();
			
            manual_cal_save_cailmain_tx_tab_to_flash();
            manual_cal_save_cailmain_rx_tab_to_flash();
        }
    }
    else
    {
        manual_cal_updata_rfcali_status();
        
        if(CM_FLAG_IS_SET())
        {
            manual_cal_save_cailmain_tx_tab_to_flash();
            manual_cal_save_cailmain_rx_tab_to_flash();
        }
    }
    
    return ;
}

void do_all_calibration(void)
{
    if(manual_cal_need_load_cmtag_from_flash())
    {
        // last status is LOAD_FROM_FLASH, means it may load from flash or default
        manual_set_cmtag(LOAD_FROM_CALI);

        // only do one time in every pow on
        calibration_main();

        rwnx_tpc_pa_map_init();
    }
}

void bk7011_max_rxsens_setting(void)
{    
	BK7011TRX.REG0x8->bits.isrxref10 = 3;    
	BK7011TRX.REG0x8->bits.isrxlna30 = 15;
	CAL_WR_TRXREGS(0x8);    
	bk7011_trx_val[8] = BK7011TRXONLY.REG0x8->value ;   
}

void bk7011_normal_rxsens_setting(void)
{    
	BK7011TRX.REG0x8->bits.isrxref10 = 2;    
	BK7011TRX.REG0x8->bits.isrxlna30 = 7;
	CAL_WR_TRXREGS(0x8);    
	bk7011_trx_val[8] = BK7011TRXONLY.REG0x8->value ;   
}

void bk7011_default_rxsens_setting(void)
{    
	BK7011TRX.REG0x8->bits.isrxref10 = 2;    
	BK7011TRX.REG0x8->bits.isrxlna30 = 7;    
	CAL_WR_TRXREGS(0x8);    
	bk7011_trx_val[8] = BK7011TRXONLY.REG0x8->value ;   
}

void bk7011_set_tx_capcali_iq(int enable)
{
    if(enable == 1) 
    {
        // just for 11b
        BK7011RCBEKEN.REG0x5A->bits.TXCALCAPI = 0x3f;
        BK7011RCBEKEN.REG0x5B->bits.TXCALCAPQ = 0x3f; 
    }
    else 
    {
        BK7011RCBEKEN.REG0x5A->value = bk7011_rc_val[26];
        BK7011RCBEKEN.REG0x5A->value = bk7011_rc_val[26];
    }
}

#include "str_pub.h"
static int rfcali_cfg_tssi_b(int argc, char **argv)
{
    int tssi_thred_b;

    if(argc != 2)
    {
        os_printf("rfcali_cfg_tssi 0-255(for b)\r\n");
        return 0;
    }
    
    tssi_thred_b = os_strtoul(argv[1], NULL, 10);

    os_printf("cmd set tssi b_thred:%d\r\n", tssi_thred_b);

    bk7011_set_rf_config_tssithred_b(tssi_thred_b);
    return 0; 
}

static int rfcali_cfg_tssi_g(int argc, char **argv)
{
    int tssi_thred_g;

    if(argc != 2)
    {
        os_printf("rfcali_cfg_tssi 0-255(for b)\r\n");
        return 0;
    }
    
    tssi_thred_g = os_strtoul(argv[1], NULL, 10);

    os_printf("cmd set tssi g_thred:%d\r\n", tssi_thred_g);

    bk7011_set_rf_config_tssithred_g(tssi_thred_g);
    return 0; 
}

static int rfcali_cfg_rate_dist(int argc, char **argv)
{
    int dist_b, dist_g, dist_n40, dist_ble;

    if(argc != 5)
    {
        os_printf("rfcali_cfg_rate_dist b g n40 ble (0-31)\r\n");
        return 0;
    }
    
    dist_b = os_strtoul(argv[1], NULL, 10);
    dist_g = os_strtoul(argv[2], NULL, 10);
    dist_n40 = os_strtoul(argv[3], NULL, 10);
    dist_ble = os_strtoul(argv[4], NULL, 10);

    if((dist_b > 31) || (dist_g > 31) || (dist_n40 > 31) || (dist_ble > 31))
    {
        os_printf("rate_dist range:-31 - 31\r\n");
        return 0;
    }

    if((dist_b < -31) || (dist_g < -31) || (dist_n40 < -31) || (dist_ble < -31))
    {
        os_printf("rate_dist range:-31 - 31\r\n");
        return 0;
    }
    
   // manual_cal_set_rate_dist_for_txpwr(dist_b, dist_g, dist_n40, dist_ble);
    
    return 0; 
}

static int rfcali_cfg_mode(int argc, char **argv)
{
    int rfcali_mode = 0;

    if(argc != 2)
    {
        os_printf("rfcali_mode 0/1\r\n");
        return 0;
    }
    
    rfcali_mode = os_strtoul(argv[1], NULL, 10);
#if 0
    if((rfcali_mode != CALI_MODE_AUTO) && (rfcali_mode != CALI_MODE_MANUAL))
    {
        os_printf("rfcali_mode 0/1, %d\r\n", rfcali_mode);
        return 0;
    }
#endif
    bk7011_set_rfcali_mode(rfcali_mode);
    
    return 0; 
}

static int rfcali_show_data(int argc, char **argv)
{
    manual_cal_show_txpwr_tab();
    
    return 0; 
}

#if CFG_SUPPORT_RTT
FINSH_FUNCTION_EXPORT_ALIAS(rfcali_cfg_tssi_b, __cmd_rfcali_cfg_tssi_b, rfcali cfg tssi);
FINSH_FUNCTION_EXPORT_ALIAS(rfcali_cfg_mode, __cmd_rfcali_cfg_mode, rfcali cfg mode);
FINSH_FUNCTION_EXPORT_ALIAS(rfcali_cfg_rate_dist, __cmd_rfcali_cfg_rate_dist, rfcali cfg rate_dist);
FINSH_FUNCTION_EXPORT_ALIAS(rfcali_cfg_tssi_g, __cmd_rfcali_cfg_tssi_g, rfcali cfg tssi);
FINSH_FUNCTION_EXPORT_ALIAS(rfcali_show_data, __cmd_rfcali_show_data, rfcali show data);
#else
void cmd_rfcali_cfg_mode(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    rfcali_cfg_mode(argc, argv);
}

void cmd_rfcali_cfg_rate_dist(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    rfcali_cfg_rate_dist(argc, argv);
}

void cmd_rfcali_cfg_tssi_g(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    rfcali_cfg_tssi_g(argc, argv);
}

void cmd_rfcali_cfg_tssi_b(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    rfcali_cfg_tssi_b(argc, argv);
}

void cmd_rfcali_show_data(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    rfcali_show_data(argc, argv);
}
#endif


#if !(CFG_SUPPORT_BLE)//below two functions are for bk7231s compile, please do NOT remove them.
UINT32 ble_in_dut_mode(void)
{
    return 0;
}

uint8 is_rf_switch_to_ble(void)
{
    return 0;
}
#endif

#else  /* CFG_SUPPORT_CALIBRATION */
#endif  /* CFG_SUPPORT_CALIBRATION */

#endif //  (CFG_SOC_NAME != SOC_BK7231)
// eof

