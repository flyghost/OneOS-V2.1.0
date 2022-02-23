/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        pos_rtcm_msg.h
 *
 * @brief       rtcm msg struct
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef __POS_RTCM_MSG_H__
#define __POS_RTCM_MSG_H__
 
#include <os_types.h>
#include <time.h> 

#ifdef __cplusplus
 extern "C" {
#endif

#define SYS_GPS     0x01                      /* navigation system: GPS */
#define SYS_SBS     0x02                      /* navigation system: SBAS */
#define SYS_GLO     0x04                      /* navigation system: GLONASS */
#define SYS_GAL     0x08                      /* navigation system: Galileo */
#define SYS_QZS     0x10                      /* navigation system: QZSS */
#define SYS_CMP     0x20                      /* navigation system: BeiDou */
#define SYS_IRN     0x40                      /* navigation system: IRNS */

#ifndef NFREQ
#define NFREQ       3                         /* number of carrier frequencies */
#endif

#ifndef NEXOBS
#define NEXOBS      0                         /* number of extended obs codes */
#endif

#define MINPRNGPS   1                         /* min satellite PRN number of GPS */
#define MAXPRNGPS   32                        /* max satellite PRN number of GPS */
#define NSATGPS     (MAXPRNGPS-MINPRNGPS+1)   /* number of GPS satellites */

#ifdef  ENAGLO                                /* if enable glonass messages */
#define MINPRNGLO   1                         /* min satellite slot number of GLONASS */
#define MAXPRNGLO   27                        /* max satellite slot number of GLONASS */
#define NSATGLO     (MAXPRNGLO-MINPRNGLO+1)   /* number of GLONASS satellites */
#define NSYSGLO     1
#else
#define MINPRNGLO   0
#define MAXPRNGLO   0
#define NSATGLO     0
#define NSYSGLO     0
#endif
#ifdef  ENAGAL
#define MINPRNGAL   1                         /* min satellite PRN number of Galileo */
#define MAXPRNGAL   36                        /* max satellite PRN number of Galileo */
#define NSATGAL     (MAXPRNGAL-MINPRNGAL+1)   /* number of Galileo satellites */
#define NSYSGAL     1
#else
#define MINPRNGAL   0
#define MAXPRNGAL   0
#define NSATGAL     0
#define NSYSGAL     0
#endif
#ifdef  ENAQZS
#define MINPRNQZS   193                       /* min satellite PRN number of QZSS */
#define MAXPRNQZS   202                       /* max satellite PRN number of QZSS */
#define MINPRNQZS_S 183                       /* min satellite PRN number of QZSS L1S */
#define MAXPRNQZS_S 191                       /* max satellite PRN number of QZSS L1S */
#define NSATQZS     (MAXPRNQZS-MINPRNQZS+1)   /* number of QZSS satellites */
#define NSYSQZS     1
#else
#define MINPRNQZS   0
#define MAXPRNQZS   0
#define MINPRNQZS_S 0
#define MAXPRNQZS_S 0
#define NSATQZS     0
#define NSYSQZS     0
#endif
#ifdef  ENACMP
#define MINPRNCMP   1                         /* min satellite sat number of BeiDou */
#define MAXPRNCMP   63                        /* max satellite sat number of BeiDou */
#define NSATCMP     (MAXPRNCMP-MINPRNCMP+1)   /* number of BeiDou satellites */
#define NSYSCMP     1
#else
#define MINPRNCMP   0
#define MAXPRNCMP   0
#define NSATCMP     0
#define NSYSCMP     0
#endif
#ifdef  ENAIRN
#define MINPRNIRN   1                         /* min satellite sat number of IRNSS */
#define MAXPRNIRN   14                        /* max satellite sat number of IRNSS */
#define NSATIRN     (MAXPRNIRN-MINPRNIRN+1)   /* number of IRNSS satellites */
#define NSYSIRN     1
#else
#define MINPRNIRN   0
#define MAXPRNIRN   0
#define NSATIRN     0
#define NSYSIRN     0
#endif
#ifdef ENALEO
#define MINPRNLEO   1                         /* min satellite sat number of LEO */
#define MAXPRNLEO   10                        /* max satellite sat number of LEO */
#define NSATLEO     (MAXPRNLEO-MINPRNLEO+1)   /* number of LEO satellites */
#define NSYSLEO     1
#else
#define MINPRNLEO   0
#define MAXPRNLEO   0
#define NSATLEO     0
#define NSYSLEO     0
#endif

#define MINPRNSBS   120                       /* min satellite PRN number of SBAS */
#define MAXPRNSBS   158                       /* max satellite PRN number of SBAS */
#define NSATSBS     (MAXPRNSBS-MINPRNSBS+1)   /* number of SBAS satellites */

                                              /* max satellite number (1 to MAXSAT) */ 
#define MAXSAT      (NSATGPS+NSATGLO+NSATGAL+NSATQZS+NSATCMP+NSATIRN+NSATSBS+NSATLEO)

#define MAXANT      64                        /* max length of station name/antenna type */
#define MAXCODE     68                        /* max number of obs code */
#define MAXRCV      64                        /* max receiver number (1 to MAXRCV) */
#define MAXBAND     10                        /* max SBAS band of IGP */
#define MAXNIGP     201                       /* max number of IGP in SBAS band */

typedef struct                                /* time struct */
{      
    time_t time;                              /* time (s) expressed by standard time_t */
    double sec;                               /* fraction of second under 1 s */
} gtime_t;

typedef struct                                /* observation data record */
{        
    gtime_t     time;                         /* receiver sampling time (GPST) */
    os_uint8_t  sat;                          /* satellite/ */
    os_uint8_t  rcv;                          /* receiver number */
    os_uint16_t SNR [NFREQ+NEXOBS];           /* signal strength (0.001 dBHz) */
    os_uint8_t  LLI [NFREQ+NEXOBS];           /* loss of lock indicator */
    os_uint8_t  code[NFREQ+NEXOBS];           /* code indicator (CODE_???) */
    double      L   [NFREQ+NEXOBS];           /* observation data carrier-phase (cycle) */
    double      P   [NFREQ+NEXOBS];           /* observation data pseudorange (m) */
    float       D   [NFREQ+NEXOBS];           /* observation data doppler frequency (Hz) */
} obsd_t;

typedef struct                                /* observation data */
{        
    os_int32_t n;
    os_int32_t nmax;                          /* number of obervation data/allocated */
    obsd_t    *data;                          /* observation data records */
} obs_t;

typedef struct                                /* GPS/QZS/GAL broadcast ephemeris type */
{        
    os_int32_t sat;                           /* satellite number */
    os_int32_t iode;                          /* IODE */
    os_int32_t iodc;                          /* IODC */
    os_int32_t sva;                           /* SV accuracy (URA index) */
    os_int32_t svh;                           /* SV health (0:ok) */
    os_int32_t week;                          /* GPS/QZS: gps week, GAL: galileo week */
    os_int32_t code;                          /* GPS/QZS: code on L2 */
                                              /* GAL: data source defined as rinex 3.03 */
                                              /* BDS: data source (0:unknown,1:B1I,2:B1Q,3:B2I,4:B2Q,5:B3I,6:B3Q) */
    os_int32_t flag;                          /* GPS/QZS: L2 P data flag */
                                              /* BDS: nav type (0:unknown,1:IGSO/MEO,2:GEO) */
    gtime_t    toe;                           /* Toe    */
    gtime_t    toc;                           /* Toc    */
    gtime_t    ttr;                           /* T_trans*/                                              
    double     A;                             /* SV orbit parameters */
    double     e;
    double     i0;
    double     OMG0;
    double     omg;
    double     M0;
    double     deln;
    double     OMGd;
    double     idot;  
    double     crc;
    double     crs;
    double     cuc;
    double     cus;
    double     cic;
    double     cis;  
    double     toes;                          /* Toe (s) in week */
    double     fit;                           /* fit interval (h) */
    double     f0;                            /* SV clock parameters (af0,af1,af2) */
    double     f1;
    double     f2;
    double     tgd[6];                        /* group delay parameters */
                                              /* GPS/QZS:tgd[0]=TGD */
                                              /* GAL:tgd[0]=BGD_E1E5a,tgd[1]=BGD_E1E5b */
                                              /* CMP:tgd[0]=TGD_B1I ,tgd[1]=TGD_B2I/B2b,tgd[2]=TGD_B1Cp */
                                              /*     tgd[3]=TGD_B2ap,tgd[4]=ISC_B1Cd   ,tgd[5]=ISC_B2ad */
    double     Adot;                          /* Adot,ndot for CNAV */
    double     ndot;                                          
} eph_t;

typedef struct                                /* GLONASS broadcast ephemeris type */
{        
    os_int32_t sat;                           /* satellite number */
    os_int32_t iode;                          /* IODE (0-6 bit of tb field) */
    os_int32_t frq;                           /* satellite frequency number */
    os_int32_t svh;                           /* satellite health, accuracy, age of operation */
    os_int32_t sva;
    os_int32_t age;  
    gtime_t    toe;                           /* epoch of epherides (gpst) */
    gtime_t    tof;                           /* message frame time (gpst) */
    double     pos[3];                        /* satellite position (ecef) (m) */
    double     vel[3];                        /* satellite velocity (ecef) (m/s) */
    double     acc[3];                        /* satellite acceleration (ecef) (m/s^2) */ 
    double     taun;                          /* SV clock bias (s)/relative freq bias */
    double     gamn;
    double     dtaun;                         /* delay between L1 and L2 (s) */
} geph_t;

typedef struct                                /* SBAS ephemeris type */
{        
    os_int32_t sat;                           /* satellite number */
    gtime_t    t0;                            /* reference epoch time (GPST) */
    gtime_t    tof;                           /* time of message frame (GPST) */
    os_int32_t sva;                           /* SV accuracy (URA index) */
    os_int32_t svh;                           /* SV health (0:ok) */
    double     pos[3];                        /* satellite position (m) (ecef) */
    double     vel[3];                        /* satellite velocity (m/s) (ecef) */
    double     acc[3];                        /* satellite acceleration (m/s^2) (ecef) */
    double     af0;                           /* satellite clock-offset/drift (s,s/s) */
    double     af1;
} seph_t;

typedef struct                                /* precise ephemeris type */
{       
    gtime_t    time;                          /* time (GPST) */
    os_int32_t index;                         /* ephemeris index for multiple files */
    double     pos[MAXSAT][4];                /* satellite position/clock (ecef) (m|s) */
    float      std[MAXSAT][4];                /* satellite position/clock std (m|s) */
    double     vel[MAXSAT][4];                /* satellite velocity/clk-rate (m/s|s/s) */
    float      vst[MAXSAT][4];                /* satellite velocity/clk-rate std (m/s|s/s) */
    float      cov[MAXSAT][3];                /* satellite position covariance (m^2) */
    float      vco[MAXSAT][3];                /* satellite velocity covariance (m^2) */
} peph_t;

typedef struct                                /* precise clock type */
{       
    gtime_t    time;                          /* time (GPST) */
    os_int32_t index;                         /* clock index for multiple files */
    double     clk[MAXSAT][1];                /* satellite clock (s) */
    float      std[MAXSAT][1];                /* satellite clock std (s) */
} pclk_t;

typedef struct                                /* almanac type */
{                       
    os_int32_t sat;                           /* satellite number */
    os_int32_t svh;                           /* sv health (0:ok) */
    os_int32_t svconf;                        /* as and sv config */
    os_int32_t week;                          /* GPS/QZS: gps week, GAL: galileo week */
    gtime_t    toa;                           /* Toa */        
    double     A;                             /* SV orbit parameters */
    double     e;
    double     i0;
    double     OMG0;
    double     omg;
    double     M0;
    double     OMGd;   
    double     toas;                          /* Toa (s) in week */
    double     f0;                            /* SV clock parameters (af0,af1) */
    double     f1;
} alm_t;

typedef struct                                /* TEC grid type */
{        
    gtime_t    time;                          /* epoch time (GPST) */
    os_int32_t ndata[3];                      /* TEC grid data size {nlat,nlon,nhgt} */
    double     rb;                            /* earth radius (km) */
    double     lats[3];                       /* latitude start/interval (deg) */
    double     lons[3];                       /* longitude start/interval (deg) */
    double     hgts[3];                       /* heights start/interval (km) */
    double    *data;                          /* TEC grid data (tecu) */
    float     *rms;                           /* RMS values (tecu) */
} tec_t;

typedef struct                                /* earth rotation parameter data type */
{        
    double     mjd;                           /* mjd (days) */   
    double     xp;                            /* pole offset (rad) */
    double     yp;  
    double     xpr;                           /* pole offset rate (rad/day) */
    double     ypr;  
    double     ut1_utc;                       /* ut1-utc (s) */
    double     lod;                           /* length of day (s/day) */
} erpd_t;

typedef struct                                /* earth rotation parameter type */
{        
    os_int32_t n;                             /* number and max number of data */
    os_int32_t nmax;  
    erpd_t    *data;                          /* earth rotation parameter data */
} erp_t;

typedef struct                                /* antenna parameter type */
{       
    os_int32_t sat;                           /* satellite number (0:receiver) */
    char       type[MAXANT];                  /* antenna type */
    char       code[MAXANT];                  /* serial number or satellite code */
    gtime_t    ts;                            /* valid time start and end */
    gtime_t    te;   
    double     off [NFREQ][ 3];               /* phase center offset e/n/u or x/y/z (m) */
    double     var [NFREQ][19];               /* phase center variation (m) */
                                              /* el=90,85,...,0 or nadir=0,1,2,3,... (deg) */
} pcv_t;


typedef struct                                /* SBAS fast correction type */
{        
    gtime_t    t0;                            /* time of applicability (TOF) */
    double     prc;                           /* pseudorange correction (PRC) (m) */
    double     rrc;                           /* range-rate correction (RRC) (m/s) */
    double     dt;                            /* range-rate correction delta-time (s) */
    os_int32_t iodf;                          /* IODF (issue of date fast corr) */
    os_int16_t udre;                          /* UDRE+1 */
    os_int16_t ai;                            /* degradation factor indicator */
} sbsfcorr_t;

typedef struct                                /* SBAS long term satellite error correction type */
{        
    gtime_t    t0;                            /* correction time */
    os_int32_t iode;                          /* IODE (issue of date ephemeris) */
    double     dpos[3];                       /* delta position (m) (ecef) */
    double     dvel[3];                       /* delta velocity (m/s) (ecef) */   
    double     daf0;                          /* delta clock-offset/drift (s,s/s) */
    double     daf1;
} sbslcorr_t;

typedef struct                                /* SBAS satellite correction type */
{        
    os_int32_t sat;                           /* satellite number */
    sbsfcorr_t fcorr;                         /* fast correction */
    sbslcorr_t lcorr;                         /* long term correction */
} sbssatp_t;

typedef struct                                /* SBAS satellite corrections type */
{        
    os_int32_t iodp;                          /* IODP (issue of date mask) */
    os_int32_t nsat;                          /* number of satellites */
    os_int32_t tlat;                          /* system latency (s) */
    sbssatp_t  sat[MAXSAT];                   /* satellite correction */
} sbssat_t;

typedef struct                                /* SBAS ionospheric correction type */
{        
    gtime_t    t0;                            /* correction time */   
    os_int16_t lat;                           /* latitude/longitude (deg) */
    os_int16_t lon;    
    os_int16_t give;                          /* GIVI+1 */
    float      delay;                         /* vertical delay estimate (m) */
} sbsigp_t;

typedef struct                                /* SBAS ionospheric corrections type */
{        
    os_int32_t iodi;                          /* IODI (issue of date ionos corr) */
    os_int32_t nigp;                          /* number of igps */
    sbsigp_t   igp[MAXNIGP];                  /* ionospheric correction */
} sbsion_t;

typedef struct                                /* DGPS/GNSS correction type */
{        
    gtime_t    t0;                            /* correction time */
    double     prc;                           /* pseudorange correction (PRC) (m) */
    double     rrc;                           /* range rate correction (RRC) (m/s) */
    os_int32_t iod;                           /* issue of data (IOD) */
    double     udre;                          /* UDRE */
} dgps_t;

typedef struct                                /* SSR correction type */
{        
    gtime_t    t0 [6];                        /* epoch time (GPST) {eph,clk,hrclk,ura,bias,pbias} */
    double     udi[6];                        /* SSR update interval (s) */
    os_int32_t iod[6];                        /* iod ssr {eph,clk,hrclk,ura,bias,pbias} */
    os_int32_t iode;                          /* issue of data */
    os_int32_t iodcrc;                        /* issue of data crc for beidou/sbas */
    os_int32_t ura;                           /* URA indicator */
    os_int32_t refd;                          /* sat ref datum (0:ITRF,1:regional) */
    double     deph [3];                      /* delta orbit {radial,along,cross} (m) */
    double     ddeph[3];                      /* dot delta orbit {radial,along,cross} (m/s) */
    double     dclk [3];                      /* delta clock {c0,c1,c2} (m,m/s,m/s^2) */
    double     hrclk;                         /* high-rate clock corection (m) */
    float      cbias[MAXCODE];                /* code biases (m) */
    double     pbias[MAXCODE];                /* phase biases (m) */
    float      stdpb[MAXCODE];                /* std-dev of phase biases (m) */ 
    double     yaw_ang;                       /* yaw angle and yaw rate (deg,deg/s) */
    double     yaw_rate;  
    os_uint8_t update;                        /* update flag (0:no update,1:update) */
} ssr_t;

typedef struct                                /* navigation data type */
{        
    os_int32_t n;                             /* number of broadcast ephemeris */
    os_int32_t nmax;
    os_int32_t ng;                            /* number of glonass ephemeris */
    os_int32_t ngmax;
    os_int32_t ns;                            /* number of sbas ephemeris */
    os_int32_t nsmax;
    os_int32_t ne;                            /* number of precise ephemeris */
    os_int32_t nemax;
    os_int32_t nc;                            /* number of precise clock */
    os_int32_t ncmax;
    os_int32_t na;                            /* number of almanac data */
    os_int32_t namax;
    os_int32_t nt;                            /* number of tec grid data */
    os_int32_t ntmax;
    eph_t     *eph;                           /* GPS/QZS/GAL/BDS/IRN ephemeris */
    geph_t    *geph;                          /* GLONASS ephemeris */
    seph_t    *seph;                          /* SBAS ephemeris */
    peph_t    *peph;                          /* precise ephemeris */
    pclk_t    *pclk;                          /* precise clock */
    alm_t     *alm;                           /* almanac data */
    tec_t     *tec;                           /* tec grid data */
    erp_t      erp;                           /* earth rotation parameters */
    double     utc_gps[8];                    /* GPS delta-UTC parameters {A0,A1,Tot,WNt,dt_LS,WN_LSF,DN,dt_LSF} */
    double     utc_glo[8];                    /* GLONASS UTC time parameters {tau_C,tau_GPS} */
    double     utc_gal[8];                    /* Galileo UTC parameters */
    double     utc_qzs[8];                    /* QZS UTC parameters */
    double     utc_cmp[8];                    /* BeiDou UTC parameters */
    double     utc_irn[9];                    /* IRNSS UTC parameters {A0,A1,Tot,...,dt_LSF,A2} */
    double     utc_sbs[4];                    /* SBAS UTC parameters */
    double     ion_gps[8];                    /* GPS iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
    double     ion_gal[4];                    /* Galileo iono model parameters {ai0,ai1,ai2,0} */
    double     ion_qzs[8];                    /* QZSS iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
    double     ion_cmp[8];                    /* BeiDou iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
    double     ion_irn[8];                    /* IRNSS iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
    os_int32_t glo_fcn[32];                   /* GLONASS FCN + 8 */
    double     cbias [MAXSAT][3];             /* satellite DCB (0:P1-P2,1:P1-C1,2:P2-C2) (m) */
    double     rbias [MAXRCV][2][3];          /* receiver DCB (0:P1-P2,1:P1-C1,2:P2-C2) (m) */
    pcv_t      pcvs  [MAXSAT];                /* satellite antenna pcv */
    sbssat_t   sbssat;                        /* SBAS satellite corrections */
    sbsion_t   sbsion[MAXBAND+1];             /* SBAS ionosphere corrections */
    dgps_t     dgps  [MAXSAT];                /* DGPS corrections */
    ssr_t      ssr   [MAXSAT];                /* SSR corrections */
} nav_t;

typedef struct                                
{        
    os_int32_t  itrf;                         /* ITRF realization year, abstract from DF021 */
    os_int32_t  deltype;                      /* antenna delta type (0:enu,1:xyz) */
    double      pos[3];                       /* station position (ecef) (m) */
    double      del[3];                       /* antenna position delta (e/n/u or x/y/z) (m) */
    double      hgt;                          /* antenna height (m) */
} sta_para_msg1005_t;

typedef struct                                
{        
    char antdes [MAXANT];                     /* antenna descriptor */
    char antsno [MAXANT];                     /* antenna serial number */
    char rectype[MAXANT];                     /* receiver type descriptor */
    char recver [MAXANT];                     /* receiver firmware version */
    char recsno [MAXANT];                     /* receiver serial number */
    os_int32_t  antsetup;                     /* antenna setup id */
} sta_para_msg1033_t;

typedef struct                                /* station parameter type */
{        
    char name   [MAXANT];                     /* marker name */
    char marker [MAXANT];                     /* marker number */
    char antdes [MAXANT];                     /* antenna descriptor */
    char antsno [MAXANT];                     /* antenna serial number */
    char rectype[MAXANT];                     /* receiver type descriptor */
    char recver [MAXANT];                     /* receiver firmware version */
    char recsno [MAXANT];                     /* receiver serial number */
    os_int32_t  antsetup;                     /* antenna setup id */
    os_int32_t  itrf;                         /* ITRF realization year */
    os_int32_t  deltype;                      /* antenna delta type (0:enu,1:xyz) */
    double      pos[3];                       /* station position (ecef) (m) */
    double      del[3];                       /* antenna position delta (e/n/u or x/y/z) (m) */
    double      hgt;                          /* antenna height (m) */
    os_int32_t  glo_cp_align;                 /* GLONASS code-phase alignment (0:no,1:yes) */
    double      glo_cp_bias[4];               /* GLONASS code-phase biases {1C,1P,2C,2P} (m) */
} sta_t;


/*
 *          RTCM 3 message format:
 *            +----------+--------+-----------+--------------------+----------+
 *            | preamble | 000000 |  length   |    data message    |  parity  |
 *            +----------+--------+-----------+--------------------+----------+
 *            |<-- 8 --->|<- 6 -->|<-- 10 --->|<--- length x 8 --->|<-- 24 -->|
 *            
 *
 */

typedef struct                                /* RTCM control struct type */
{        
    os_int32_t  staid;                        /* station id */
    os_int32_t  stah;                         /* station health */
    os_int32_t  seqno;                        /* sequence number for rtcm 2 or iods msm */
    os_int32_t  outtype;                      /* output message type */
    gtime_t     time;                         /* message time */
    gtime_t     time_s;                       /* message start time */
    obs_t       obs;                          /* observation data (uncorrected) */
    nav_t       nav;                          /* satellite ephemerides */
    sta_t       sta;                          /* station parameters */
    dgps_t     *dgps;                         /* output of dgps corrections */
    ssr_t       ssr    [MAXSAT];              /* output of ssr corrections */
    char        msg    [128];                 /* special message */
    char        msgtype[256];                 /* last message type */
    char        msmtype[7][128];              /* msm signal types */
    os_int32_t  obsflag;                      /* obs data complete flag (1:ok,0:not complete) */
    os_int32_t  ephsat;                       /* input ephemeris satellite number */
    os_int32_t  ephset;                       /* input ephemeris set (0-1) */
    double      cp    [MAXSAT][NFREQ+NEXOBS]; /* carrier-phase measurement */
    os_uint16_t lock  [MAXSAT][NFREQ+NEXOBS]; /* lock time */
    os_uint16_t loss  [MAXSAT][NFREQ+NEXOBS]; /* loss of lock count */
    gtime_t     lltime[MAXSAT][NFREQ+NEXOBS]; /* last lock time */
    os_int32_t  nbyte;                        /* number of bytes in message buffer */ 
    os_int32_t  nbit;                         /* number of bits in word buffer */ 
    os_int32_t  len;                          /* message length (bytes): from preamble to end of the data message(not include CRC Only) */
    os_uint8_t  buff [1200];                  /* message buffer */
    os_uint32_t word;                         /* word buffer for rtcm 2 */
    os_uint32_t nmsg2[100];                   /* message count of RTCM 2 (1-99:1-99,0:other) */
    os_uint32_t nmsg3[400];                   /* message count of RTCM 3 (1-299:1001-1299,300-329:4070-4099,0:ohter) */
    char        opt  [256];                   /* RTCM dependent options */
} rtcm_t;


typedef struct rtcm_msm_msg_header            /* multi-signal-message header type     */
{   
    os_int32_t staid;                         /* station id */
    os_uint8_t iod;                           /* issue of data station                */
    os_uint8_t time_s;                        /* cumulative session transmitting time */
    os_uint8_t clk_str;                       /* clock steering indicator             */
    os_uint8_t clk_ext;                       /* external clock indicator             */
    os_uint8_t smooth;                        /* divergence free smoothing indicator  */
    os_uint8_t tint_s;                        /* soothing interval                    */
    os_uint8_t nsat;                          /* number of satellites/signals         */
    os_uint8_t nsig;
    os_uint8_t ncell;
    os_uint8_t sats    [64];                  /* satellites                           */
    os_uint8_t sigs    [32];                  /* signals                              */
    os_uint8_t cellmask[64];                  /* cell mask                            */
}rtcm_msm_msg_header_t;


typedef struct rtcm_msg_1005
{
    os_int32_t          stationid;                    
    sta_para_msg1005_t *sta_para;
}rtcm_msg_1005_t;


typedef struct rtcm_msg_1033
{
    os_int32_t          stationid;                    
    sta_para_msg1033_t *sta_para;
}rtcm_msg_1033_t;


typedef struct rtcm_msg_msm5                 /* msg_1075(GPS) && msg_1125(BD)               */
{
    rtcm_msm_msg_header_t    *header;        /* MSMs msg header                             */
    double                   r   [64];       /* satellite data: abstract from DF397 && DF398*/
    os_int32_t               ex  [64];       /* satellite data                              */
    double                   rr  [64];       /* satellite data: abstract from DF399         */
    double                   pr  [64];       /* signal data   : abstract from DF400         */
    double                   cp  [64];       /* signal data   : abstract from DF401         */
    os_int32_t               lock[64];       /* signal data   : abstract from DF402         */
    os_int32_t               half[64];       /* signal data   : abstract from DF420         */
    double                   cnr [64];       /* signal data   : abstract from DF403         */
    double                   rrf [64];       /* signal data   : abstract from DF404         */   
}rtcm_msg_msm5_t;


#endif

