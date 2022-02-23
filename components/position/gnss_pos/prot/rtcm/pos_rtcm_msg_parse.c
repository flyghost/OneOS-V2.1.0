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
 * @file        pos_rtcm_msg_parse.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_util.h>
#include <os_errno.h>
#include <stdio.h>                                             /* for sscanf() sprintf()    */
#include <string.h>                                            /* for strstr()              */
#include <time.h>                                              /* for difftime() time_t     */
#include <math.h>                                              /* for floor() fmod() pow() fabs() atan() atan2() sqrt()*/

#include "pos_rtcm_msg.h"
#include "pos_rtcm_msg_parse.h"

#define MAXLEAPS                          (64)                 /* max number of leap seconds table */
#define PI                (3.1415926535897932)                 /* pi                               */
#define D2R                         (PI/180.0)                 /* deg to rad                       */
#define R2D                         (180.0/PI)                 /* rad to deg                       */
#define CLIGHT                   (299792458.0)                 /* speed of light (m/s)             */
#define RANGE_MS                (CLIGHT*0.001)                 /* range in 1 ms                    */
#define P2_10                   (0.0009765625)                 /* 2^-10                            */
#define P2_24          (5.960464477539063E-08)                 /* 2^-24                            */
#define P2_29          (1.862645149230957E-09)                 /* 2^-29                            */
#define RE_WGS84                   (6378137.0)                 /* earth semimajor axis (WGS84) (m) */
#define FE_WGS84           (1.0/298.257223563)                 /* earth flattening (WGS84)         */



#define RTCM_MSG_PREAMBLE_LEN                                                                                   (8)
#define RTCM_MSG_RESERVED_LEN                                                                                   (6)
#define RTCM_MSG_MSG_LENGTH_LEN                                                                                (10)
#define RTCM_FRAME_HEAD_LEN               (RTCM_MSG_PREAMBLE_LEN + RTCM_MSG_RESERVED_LEN + RTCM_MSG_MSG_LENGTH_LEN)
#define RTCM_DF001_LEN                                                                                          (7)
#define RTCM_DF002_LEN                                                                                         (12)
#define RTCM_DF003_LEN                                                                                         (12)
#define GNSS_EPOCH_TIME_LEN                                                                                    (30)
#define RTCM_DF021_LEN                                                                                          (6)
#define RTCM_DF025_LEN                                                                                         (38)
#define RTCM_DF026_LEN                                                                                         (38)
#define RTCM_DF027_LEN                                                                                         (38)
#define RTCM_DF029_LEN                                                                                          (8) 
#define SINGLE_ANTENNA_DESCRIPTOR_LEN                                                                           (8)
#define RTCM_DF031_LEN                                                                                          (8)
#define RTCM_DF032_LEN                                                                                          (8)
#define SINGLE_SERIAL_NUMBLE_LEN                                                                                (8)
#define RTCM_DF227_LEN                                                                                          (8)
#define SINGLE_RECEIVER_TYPE_DESCRIPTOR_LEN                                                                     (8)
#define RTCM_DF229_LEN                                                                                          (8)
#define SINGLE_RECEIVER_FIRMWARE_VERSION_LEN                                                                    (8)
#define RTCM_DF231_LEN                                                                                          (8)
#define SINGLE_RECEIVER_SERIAL_NUMBER_LEN                                                                       (8)
#define RTCM_DF393_LEN                                                                                          (1)
#define RTCM_DF409_LEN                                                                                          (3)
#define RTCM_DF411_LEN                                                                                          (2)
#define RTCM_DF412_LEN                                                                                          (2)
#define RTCM_DF417_LEN                                                                                          (1)
#define RTCM_DF418_LEN                                                                                          (3)
#define RTCM_DF394_LEN                                                                                         (64)
#define RTCM_DF395_LEN                                                                                         (32)
#define RTCM_DF396_MAX_LEN                                                                                     (64) 
#define RTCM_DF397_LEN                                                                                          (8)
#define EXTENDED_SATELLITE_INFO_LEN                                                                             (4)
#define RTCM_DF398_LEN                                                                                         (10)
#define RTCM_DF399_LEN                                                                                         (14)
#define RTCM_DF400_LEN                                                                                         (15)
#define RTCM_DF401_LEN                                                                                         (22)
#define RTCM_DF402_LEN                                                                                          (4)
#define RTCM_DF420_LEN                                                                                          (1)
#define RTCM_DF403_LEN                                                                                          (6)
#define RTCM_DF404_LEN                                                                                         (15)
#define RTCM_MSG_CRC_LEN                                                                                       (24)
#define RTCM_MSG1005_LEN                                                                                      (152)            
#define RTCM_MSMX_HEADER_MIN_LEN                                                                              (169)                   
#define RTCM_MSMX_HEADER_MAX_LEN                                    (RTCM_MSMX_HEADER_MIN_LEN + RTCM_DF396_MAX_LEN)  
#define RTCM_MSM5_SATELLITE_DATA_LEN                                                                           (36)
#define RTCM_MSM5_SINGNAL_DATA_LEN                                                                             (63)


/* | time_convert | */
#if 0
static double       timeoffset_          = 0.0;                    /* time offset (s) */
static const double gpst0[]              = {1980, 1, 6, 0, 0, 0};  /* gps time reference */
static double       leaps[MAXLEAPS+1][7] =                         /* leap seconds (y,m,d,h,m,s,utc-gpst) */
{ 
    {2017,1,1,0,0,0,-18},
    {2015,7,1,0,0,0,-17},
    {2012,7,1,0,0,0,-16},
    {2009,1,1,0,0,0,-15},
    {2006,1,1,0,0,0,-14},
    {1999,1,1,0,0,0,-13},
    {1997,7,1,0,0,0,-12},
    {1996,1,1,0,0,0,-11},
    {1994,7,1,0,0,0,-10},
    {1993,7,1,0,0,0, -9},
    {1992,7,1,0,0,0, -8},
    {1991,1,1,0,0,0, -7},
    {1990,1,1,0,0,0, -6},
    {1988,1,1,0,0,0, -5},
    {1985,7,1,0,0,0, -4},
    {1983,7,1,0,0,0, -3},
    {1982,7,1,0,0,0, -2},
    {1981,7,1,0,0,0, -1},
    {0}
};
#endif

/* ---------------MSM signal ID table ----------------*/
const char *msm_sig_gps[32] = 
{
    /* GPS: ref [17] table 3.5-91 */
    ""  ,"1C","1P","1W",""  ,""  ,""  ,"2C","2P","2W",""  ,""  , /*  1-12 */
    ""  ,""  ,"2S","2L","2X",""  ,""  ,""  ,""  ,"5I","5Q","5X", /* 13-24 */
    ""  ,""  ,""  ,""  ,""  ,"1S","1L","1X"                      /* 25-32 */
};
const char *msm_sig_glo[32] = 
{
    /* GLONASS: ref [17] table 3.5-96 */
    ""  ,"1C","1P",""  ,""  ,""  ,""  ,"2C","2P",""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};
const char *msm_sig_gal[32] = 
{
    /* Galileo: ref [17] table 3.5-99 */
    ""  ,"1C","1A","1B","1X","1Z",""  ,"6C","6A","6B","6X","6Z",
    ""  ,"7I","7Q","7X",""  ,"8I","8Q","8X",""  ,"5I","5Q","5X",
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};
const char *msm_sig_qzs[32] = 
{
    /* QZSS: ref [17] table 3.5-105 */
    ""  ,"1C",""  ,""  ,""  ,""  ,""  ,""  ,"6S","6L","6X",""  ,
    ""  ,""  ,"2S","2L","2X",""  ,""  ,""  ,""  ,"5I","5Q","5X",
    ""  ,""  ,""  ,""  ,""  ,"1S","1L","1X"
};
const char *msm_sig_sbs[32] = 
{
    /* SBAS: ref [17] table 3.5-102 */
    ""  ,"1C",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,"5I","5Q","5X",
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};
const char *msm_sig_cmp[32] = 
{
    /* BeiDou: ref [17] table 3.5-108 */
    ""  ,"2I","2Q","2X",""  ,""  ,""  ,"6I","6Q","6X",""  ,""  ,
    ""  ,"7I","7Q","7X",""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};
const char *msm_sig_irn[32] = 
{
    /* NavIC/IRNSS: ref [17] table 3.5-108.3 */
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,""  ,"5A",""  ,""  ,
    ""  ,""  ,""  ,""  ,""  ,""  ,""  ,""
};


/**
 ***********************************************************************************************************************
 * @brief           extract unsigned bits
 * 
 * @details         extract unsigned bits from byte data
 * 
 * @attention       Optional
 * 
 * @param[in]       os_uint8_t      *buff       byte data
 *                  os_int32_t       pos        bit position from start of data (bits)
 *                  os_int32_t       len        bit length (bits) (len<=32) 
 * 
 * @return          extracted unsigned bits
 ***********************************************************************************************************************
 */ 
static os_uint32_t getbitu(const os_uint8_t *buff, os_int32_t pos, os_int32_t len)
{
    os_uint32_t bits = 0;
    os_int32_t i;
    
    for (i = pos; i < pos + len; i++) 
    {
        bits = (bits << 1) + ((buff[i / 8] >> (7 - i%8)) & 1u);
    } 
    
    return bits;
}


/**
 ***********************************************************************************************************************
 * @brief           extract signed bits
 * 
 * @details         extract signed bits from byte data
 * 
 * @attention       Optional
 * 
 * @param[in]       os_uint8_t      *buff       byte data
 *                  os_int32_t       pos        bit position from start of data (bits)
 *                  os_int32_t       len        bit length (bits) (len<=32) 
 * 
 * @return          extracted signed bits
 ***********************************************************************************************************************
 */ 
static os_int32_t getbits(const os_uint8_t *buff, os_int32_t pos, os_int32_t len)
{
    os_uint32_t bits = getbitu(buff, pos, len);
    
    if (len <= 0 || 32 <= len || !(bits & (1u << (len - 1)))) 
    {
        return (os_int32_t)bits;
    }  
    
    /* extend sign */
    return (os_int32_t)(bits | (~0u << len)); 
}


/* get signed 38bit field */
static double getbits_38(const os_uint8_t *buff, os_int32_t pos)
{
    return (double)(getbits(buff, pos, 32) * 64.0 + getbitu(buff, pos+32, 6));
}

/* | time_convert | */
#if 0
/**
 ***********************************************************************************************************************
 * @brief           time difference
 * 
 * @details         difference between gtime_t structs
 * 
 * @attention       Optional
 * 
 * @param[in]       gtime_t t1         gtime_t structs
 *                  gtime_t t2         gtime_t structs
 *
 * @return          time difference   (t1-t2) (s)
 ***********************************************************************************************************************
 */ 
static double timediff(gtime_t t1, gtime_t t2)
{
    return difftime(t1.time,t2.time)+t1.sec-t2.sec;
}


/**
 ***********************************************************************************************************************
 * @brief           convert calendar day/time to time 
 * 
 * @details         convert calendar day/time to gtime_t struct
 * 
 * @attention       proper in 1970-2037 or 1970-2099 (64bit time_t)
 * 
 * @param[in]       double *ep      day/time {year,month,day,hour,min,sec}
 * 
 * @return          gtime_t struct
 ***********************************************************************************************************************
 */ 
static gtime_t epoch2time(const double *ep)
{
    const os_int32_t  doy[] = {1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
    gtime_t           time  = {0};
    os_int32_t        days;
    os_int32_t        sec;
    os_int32_t        year = (os_int32_t)ep[0];
    os_int32_t        mon  = (os_int32_t)ep[1];
    os_int32_t        day  = (os_int32_t)ep[2];
    
    if (year<1970 || 2099<year || mon<1 || 12<mon) 
    {
        return time;    
    }
    
    /* leap year if year%4==0 in 1901-2099 */
    days      = (year-1970) * 365 + (year-1969)/4 + doy[mon-1] + day-2 + (year%4 == 0 && mon>=3?1:0);
    sec       = (os_int32_t)floor(ep[5]);
    time.time = (time_t)days * 86400  + (os_int32_t)ep[3] * 3600 + (os_int32_t)ep[4] * 60 + sec;
    time.sec  = ep[5] - sec;
    return time;
}


/**
 ***********************************************************************************************************************
 * @brief           time to calendar day/time 
 * 
 * @details         convert gtime_t struct to calendar day/time
 * 
 * @attention       proper in 1970-2037 or 1970-2099 (64bit time_t)
 * 
 * @param[in]       gtime_t t          gtime_t struct
 *
 * @param[out]      double *ep         day/time {year,month,day,hour,min,sec}
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void time2epoch(gtime_t t, double *ep)
{
    const os_int32_t mday[]  =
    { /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    os_int32_t days,sec,mon,day;
    
    /* leap year if year%4==0 in 1901-2099 */
    days = (os_int32_t)(t.time / 86400);
    sec  = (os_int32_t)(t.time - (time_t)days * 86400);
    for (day = days%1461, mon = 0; mon < 48; mon++) 
    {
        if (day>=mday[mon]) 
        {
            day-=mday[mon];
        } 
        else 
        {
            break;
        }
    }
    ep[0] = 1970 + days/1461 * 4 + mon/12; 
    ep[1] = mon%12 + 1; 
    ep[2] = day + 1;
    ep[3] = sec / 3600; 
    ep[4] = sec%3600 / 60; 
    ep[5] = sec%60 + t.sec;
}


/**
 ***********************************************************************************************************************
 * @brief          add time
 * 
 * @details        add time to gtime_t struct
 * 
 * @attention      Optional
 * 
 * @param[in]      gtime_t t           gtime_t struct
 *                 double sec          time to add (s)
 * 
 * @return         gtime_t struct      (t+sec)
 ***********************************************************************************************************************
 */ 
static gtime_t timeadd(gtime_t t, double sec)
{
    double tt;
    
    t.sec  += sec; 
    tt     =  floor(t.sec); 
    t.time += (os_int32_t)tt; 
    t.sec  -= tt;
    
    return t;
}


/**
 ***********************************************************************************************************************
 * @brief           gpstime to utc
 * 
 * @details         convert gpstime to utc considering leap seconds
 * 
 * @attention       ignore slight time offset under 100 ns
 * 
 * @param[in]       gtime_t t          time expressed in gpstime
 * 
 * @return          time expressed in utc
 ***********************************************************************************************************************
 */ 
static gtime_t gpst2utc(gtime_t t)
{
    gtime_t    tu;
    os_int32_t i;
    
    for (i=0; leaps[i][0]>0; i++) 
    {
        tu = timeadd(t, leaps[i][6]);
        if (timediff(tu, epoch2time(leaps[i]))>=0.0) 
        {
            return tu;
        }
    }
    
    return t;
}


/**
 ***********************************************************************************************************************
 * @brief           utc to gpstime
 * 
 * @details         convert utc to gpstime considering leap seconds
 * 
 * @attention       ignore slight time offset under 100 ns
 * 
 * @param[in]       gtime_t t          time expressed in utc
 * 
 * @return          time expressed in gpstime
 ***********************************************************************************************************************
 */ 
static gtime_t utc2gpst(gtime_t t)
{
    os_int32_t i;
    
    for (i=0; leaps[i][0]>0; i++) 
    {
        if (timediff(t, epoch2time(leaps[i]))>=0.0) 
        {
            return timeadd(t,-leaps[i][6]);
        }
    }
    
    return t;
}


/**
 ***********************************************************************************************************************
 * @brief           gps time to time
 * 
 * @details         convert week and tow in gps time to gtime_t struct
 * 
 * @attention       Optional
 * 
 * @param[in]       os_int32_t       week   week number in gps time
 *                  double           sec    time of week in gps time (s)
 * 
 * @return          gtime_t struct
 ***********************************************************************************************************************
 */ 
static gtime_t gpst2time(os_int32_t week, double sec)
{
    gtime_t t = epoch2time(gpst0);
    
    if (sec<-1E9 || 1E9<sec) 
    {
        sec=0.0;
    }
    t.time += (time_t)86400 * 7 * week + (os_int32_t)sec;
    t.sec   = sec - (os_int32_t)sec;
    return t;
}


/**
 ***********************************************************************************************************************
 * @brief           time to gps time 
 * 
 * @details         convert gtime_t struct to week and tow in gps time
 * 
 * @attention       Optional
 * 
 * @param[in]       gtime_t          t        gtime_t struct
 *
 * @param[in/out]   os_int32_t      *week     week number in gps time (NULL: no output)
 *
 * @return          time of week in gps time (s)
 ***********************************************************************************************************************
 */
static double time2gpst(gtime_t t, os_int32_t *week)
{
    gtime_t t0   = epoch2time(gpst0);
    time_t  sec  = t.time - t0.time;
    os_int32_t w = (os_int32_t)(sec / (86400*7));
    
    if (week) 
    {
        *week=w;
    }
    
    return (double)(sec - (double)w * 86400 * 7) + t.sec;
}

/* ----------------------------------------------------------------------------
 * get current time in utc
 * args   : none
 * return : current time in utc
 *-----------------------------------------------------------------------------
 */
extern gtime_t timeget(void)
{
    gtime_t time;
    double  ep[6]={0};
#ifdef WIN32
    SYSTEMTIME ts;
    
    GetSystemTime(&ts); /* utc */
    ep[0] = ts.wYear; 
    ep[1] = ts.wMonth;  
    ep[2] = ts.wDay;
    ep[3] = ts.wHour; 
    ep[4] = ts.wMinute; 
    ep[5] = ts.wSecond+ts.wMilliseconds*1E-3;
#else
    struct timeval tv;
    struct tm *tt;
    
    if (!gettimeofday(&tv,NULL) && (tt = gmtime(&tv.tv_sec)))
    {
        ep[0]= tt->tm_year + 1900; 
        ep[1]= tt->tm_mon  + 1; 
        ep[2]= tt->tm_mday;
        ep[3]= tt->tm_hour; 
        ep[4]= tt->tm_min; 
        ep[5]= tt->tm_sec + tv.tv_usec*1E-6;
    }
#endif
    time = epoch2time(ep);
    
#ifdef CPUTIME_IN_GPST /* cputime operated in gpst */
    time = gpst2utc(time);
#endif
    return timeadd(time, timeoffset_);
}

/* adjust weekly rollover of GPS time */
static void adjweek(rtcm_t *rtcm, double tow)
{
    double     tow_p;
    os_int32_t week;
    
    /* if no time, get cpu time */
    if (rtcm->time.time == 0) 
    {
        rtcm->time = utc2gpst(timeget());
    }
    
    tow_p = time2gpst(rtcm->time,&week);
    
    if (tow < tow_p-302400.0) 
    {
        tow += 604800.0;
    }
    else if (tow > tow_p+302400.0) 
    {
        tow -= 604800.0;
    }
    
    rtcm->time = gpst2time(week, tow);
}

/* adjust daily rollover of GLONASS time */
static void adjday_glot(rtcm_t *rtcm, double tod)
{
    gtime_t    time;
    double     tow,tod_p;
    os_int32_t week;
    
    if (rtcm->time.time == 0) 
    {
        rtcm->time = utc2gpst(timeget());
    }
    
    time  = timeadd(gpst2utc(rtcm->time), 10800.0);  /* glonass time */
    tow   = time2gpst(time, &week);
    tod_p = fmod(tow, 86400.0); 
    tow  -= tod_p;
    if (tod < tod_p - 43200.0) 
    {
        tod += 86400.0;
    }
    else if (tod > tod_p + 43200.0) 
    {
        tod -= 86400.0;
    }
    
    time       = gpst2time(week, tow+tod);
    rtcm->time = utc2gpst(timeadd(time, -10800.0));
}


/**
 ***********************************************************************************************************************
 * @brief           time to string 
 * 
 * @details         convert gtime_t struct to string
 * 
 * @attention       Optional
 * 
 * @param[in]       gtime_t       t        gtime_t struct
 *                  os_int32_t    n        number of decimals
 *
 * @param[out]      char         *s        string ("yyyy/mm/dd hh:mm:ss.ssss")
 *
 * @return          none
 ***********************************************************************************************************************
 */ 
static void time2str(gtime_t t, char *s, os_int32_t n)
{
    double ep[6];
    
    if (n<0) 
    {
        n=0;
    } 
    else if (n>12) 
    { 
        n=12;
    }
    if (1.0-t.sec < 0.5/pow(10.0,n)) {t.time++; t.sec=0.0;};
    time2epoch(t,ep);
    sprintf(s, "%04.0f/%02.0f/%02.0f %02.0f:%02.0f:%0*.*f", ep[0], ep[1], ep[2],
            ep[3], ep[4], n<=0?2:n+3, n<=0?0:n, ep[5]);
}
#endif


/**
 ***********************************************************************************************************************
 * @brief           get rtcm frame's  MSG_TYPE
 * 
 * @details         Optional
 * 
 * @attention       Optional
 * 
 * @param[in]       os_uint8_t  *buff     (from frame's preamble)
 *                  os_uint32_t  buf_len  (from preamble to CRC)
 * 
 * @return          rtcm msg type
 ***********************************************************************************************************************
 */ 
os_int32_t onepos_parse_rtcm_msg_type(char *buff, os_uint32_t buf_len)
{
    os_int32_t i = RTCM_FRAME_HEAD_LEN;
    return getbitu((const os_uint8_t*)buff, i, RTCM_DF002_LEN);
}


/**
 ***********************************************************************************************************************
 * @brief           decode type MSM1~MSM7 message header 
 * 
 * @details         Optional
 * 
 * @attention       ncell == the sum of "1"bit in DF396 && necll <= (h->nsat * h->nsig)
 * 
 * @param[in]       os_uint8_t             *buff
 *                  os_uint32_t             buf_len
 *                  os_int32_t              sys
 *
 * @param[out]      os_int32_t             *sync
 *                  os_int32_t             *iod          
 *                  rtcm_msm_msg_header_t  *h 
 *                  os_int32_t             *hsize      size of msm_head, value: 24 + 169 + (h->nsat * h->nsig)
 *
 * @return          ncell 
 ***********************************************************************************************************************
 */ 
static os_int32_t decode_msm_head(os_uint8_t            *buff, 
                                  os_uint32_t            buf_len, 
                                  os_int32_t             sys, 
                                  os_int32_t            *sync, 
                                  os_int32_t            *iod, 
                                  rtcm_msm_msg_header_t *h, 
                                  os_int32_t            *hsize)
{
    double                tow;
    os_int32_t            ncell = 0;
    os_int32_t            i     = RTCM_FRAME_HEAD_LEN;
    os_int32_t            j;
    os_int32_t            mask;
    os_int32_t            type;
    
    type = getbitu(buff, i, RTCM_DF002_LEN); 
    i += RTCM_DF002_LEN;                                             

    memset(h, 0, sizeof(rtcm_msm_msg_header_t));
    
    if (i + RTCM_MSMX_HEADER_MIN_LEN - RTCM_DF002_LEN <= buf_len * 8) 
    {
        h->staid = getbitu(buff, i, RTCM_DF003_LEN);                          
        i += RTCM_DF003_LEN;                                         
        
        if (sys == SYS_GLO) 
        {
            os_kprintf("SYS_GLO is not supported in decode_msm_head()\n");
            return OS_ERROR;
        }
        else if (sys == SYS_CMP) 
        {
            tow  = getbitu(buff, i, 30) * 0.001;              
            i   += GNSS_EPOCH_TIME_LEN;                                               
            tow += 14.0; /* BDT -> GPST */
            /* | time_convert | */
            //adjweek(rtcm, tow);                                    
        }
        else 
        {
            tow = getbitu(buff, i, 30) * 0.001; 
            i  += GNSS_EPOCH_TIME_LEN;
            /* | time_convert | */
            //adjweek(rtcm, tow);
        }

        *sync      = getbitu(buff, i, 1);                                     
        i += RTCM_DF393_LEN;
        
        *iod       = getbitu(buff, i, 3);                                        
        i += RTCM_DF409_LEN;
        
        h->time_s  = getbitu(buff, i, 7);                        
        i += RTCM_DF001_LEN;
        
        h->clk_str = getbitu(buff, i, 2);                        
        i += RTCM_DF411_LEN;
        
        h->clk_ext = getbitu(buff, i, 2);                        
        i += RTCM_DF412_LEN;
        
        h->smooth  = getbitu(buff, i, 1);                             
        i += RTCM_DF417_LEN;
        
        h->tint_s  = getbitu(buff, i, 3);                        
        i += RTCM_DF418_LEN;
        
        for (j = 1; j <= RTCM_DF394_LEN; j++)                            
        {
            mask = getbitu(buff, i, 1); 
            i += 1;
            if (mask) 
            {
                h->sats[h->nsat++] = j;                             
            }
        }                                                             

                                                                                 
        for (j = 1; j <= RTCM_DF395_LEN; j++)                            
        {
            mask = getbitu(buff, i, 1); 
            i += 1;
            if (mask) 
            {
                h->sigs[h->nsig++] = j;                                    
            }
        }                                                                /*  i == 193 */                                                                      
    }
    else 
    {
        os_kprintf("rtcm3 %d length error: len=%d\n", type, buf_len);
        return OS_ERROR;
    }

    
    if (h->nsat * h->nsig > 64)                                     
    {
        os_kprintf("rtcm3 %d number of sats and sigs error: nsat=%d nsig=%d\n", type, h->nsat, h->nsig);
        return OS_ERROR;
    }

    if (i + h->nsat * h->nsig > buf_len * 8)                      
    {                                                               
        os_kprintf("rtcm3 %d length error: len=%d nsat=%d nsig=%d\n", type, buf_len, h->nsat, h->nsig);
        return OS_ERROR;
    }
    
    for (j = 0; j < h->nsat*h->nsig; j++)                               /* DF396 */
    {
        h->cellmask[j] = getbitu(buff, i, 1); 
        i += 1;
        if (h->cellmask[j]) 
        {
            ncell++;                                                    /* ncell == sum of "1"bit in DF396 */
        }                                                          
    }                                                               
    
    *hsize=i;                                                           /* *hsize == 193 + h->nsat * h->nsig */                                     
        
    return ncell;
}


/**
 ***********************************************************************************************************************
 * @brief           decode type decode_msm5 message
 * 
 * @details         Optional
 * 
 * @attention       sync == 0              the final MSM packet
 *                  sync == 1              not the final MSM packet
 *                  sync == OS_ERROR
 *
 * @param[in/out]   rtcm_msg_msm5_t        result
 * 
 * @return          os_int32_t             sync
 ***********************************************************************************************************************
 */ 
os_int32_t decode_msm5(os_uint8_t *buff, os_uint32_t buf_len, os_int32_t sys, rtcm_msg_msm5_t *result)
{
    rtcm_msm_msg_header_t    *h    =   result->header;
    double                   *r    = &(result->r[0]);
    double                   *rr   = &(result->rr[0]);
    double                   *pr   = &(result->pr[0]);
    double                   *cp   = &(result->cp[0]);
    double                   *rrf  = &(result->rrf[0]);
    double                   *cnr  = &(result->cnr[0]);
    os_int32_t               *lock = &(result->lock[0]);
    os_int32_t               *ex   = &(result->ex[0]);
    os_int32_t               *half = &(result->half[0]);
    
    os_int32_t               i;
    os_int32_t               j;
    os_int32_t               type;
    os_int32_t               sync;
    os_int32_t               iod;
    os_int32_t               ncell;
    os_int32_t               rng;
    os_int32_t               rng_m;
    os_int32_t               rate;
    os_int32_t               prv;
    os_int32_t               cpv;
    os_int32_t               rrv;
 
    type = getbitu(buff, RTCM_FRAME_HEAD_LEN, RTCM_DF002_LEN);          
 
    /* 1.decode msm header */
    if ((ncell = decode_msm_head(buff, buf_len, sys, &sync, &iod, h, &i)) < 0) 
    {
        return OS_ERROR;
    }

    h->ncell = ncell;

    /* 24 + 169 <= i <= 24 + 169 + 64 */
    /* buf_len*8 == 24 + 169 + nsat*nsig + nsat*36 + ncell*63 */
    if (i + h->nsat * RTCM_MSM5_SATELLITE_DATA_LEN + ncell * RTCM_MSM5_SINGNAL_DATA_LEN > buf_len * 8) 
    {
        os_kprintf("rtcm3 %d length error: nsat=%d ncell=%d len=%d\n", type, h->nsat, ncell, buf_len);
        return OS_ERROR;
    }


    /* initialization */
    for (j = 0; j < h->nsat; j++) 
    {
        r[j]  = rr[j] = 0.0; 
        ex[j] = 15;
    }
    
    for (j = 0; j < ncell; j++) 
    {
        pr[j] = cp[j] = rrf[j] = -1E16;
    }


    /* 2.decode satellite data */                                           
    for (j = 0; j < h->nsat; j++)                                           
    {   
        /* range */
        rng = getbitu(buff, i, RTCM_DF397_LEN); 
        i  += RTCM_DF397_LEN;
        if (rng != 255) 
        {
            r[j] = rng * RANGE_MS;
        }
    }
    
    for (j = 0; j < h->nsat; j++)                                           
    { 
        /* extended info */
        ex[j] = getbitu(buff, i, EXTENDED_SATELLITE_INFO_LEN); 
        i    += EXTENDED_SATELLITE_INFO_LEN;
    }
    
    for (j = 0; j < h->nsat; j++)                                            
    {
        rng_m = getbitu(buff, i, RTCM_DF398_LEN); 
        i    += RTCM_DF398_LEN;
        if (r[j] != 0.0) 
        {
            r[j] += rng_m * P2_10 * RANGE_MS;
        }
    }
    
    for (j = 0; j < h->nsat; j++)                                           
    { 
        /* phaserangerate */
        rate = getbits(buff, i, RTCM_DF399_LEN); 
        i   += RTCM_DF399_LEN;
        if (rate != -8192) 
        {
            rr[j] = rate * 1.0;
        }
    }

    
    /* 3.decode signal data */
    for (j = 0; j < ncell; j++)                                              
    { 
        /* pseudorange */
        prv = getbits(buff, i, RTCM_DF400_LEN);                                  
        i  += RTCM_DF400_LEN;
        if (prv != -16384) 
        {
            pr[j] = prv * P2_24 * RANGE_MS;
        }
    }
    
    for (j = 0; j < ncell; j++)                                                 
    {
        /* phaserange */
        cpv = getbits(buff, i, RTCM_DF401_LEN); 
        i  += RTCM_DF401_LEN;
        if (cpv != -2097152) 
        {
            cp[j] = cpv * P2_29 * RANGE_MS;
        }
    }
    
    for (j = 0; j < ncell; j++)                                              
    { 
        /* lock time */
        lock[j] = getbitu(buff, i, RTCM_DF402_LEN); 
        i      += RTCM_DF402_LEN;
    }
    
    for (j = 0; j < ncell; j++)                                              
    {    
        /* half-cycle ambiguity */
        half[j] = getbitu(buff, i, RTCM_DF420_LEN); 
        i      += RTCM_DF420_LEN;
    }
    
    for (j = 0; j < ncell; j++)                                             
    { 
        /* cnr */
        cnr[j] = getbitu(buff, i, RTCM_DF403_LEN) * 1.0; 
        i     += RTCM_DF403_LEN;
    }
    
    for (j = 0; j < ncell; j++)                                             
    { 
        /* phaserangerate */
        rrv = getbits(buff, i, RTCM_DF404_LEN); 
        i  += RTCM_DF404_LEN;
        if (rrv != -16384) 
        {
            rrf[j] = rrv * 0.0001;
        }
    }

    return sync?0:1;
}                         


/**
 ***********************************************************************************************************************
 * @brief           decode type 1005: stationary RTK reference station ARP
 * 
 * @details         Optional
 * 
 * @attention       Optional
 * 
 * @param[in/out]   rtcm_msg_1005_t    *result 
 * 
 * @return          os_int32_t
 ***********************************************************************************************************************
 */
os_int32_t decode_type1005(os_uint8_t *buff, os_uint32_t buf_len, rtcm_msg_1005_t *result)
{
    
    os_int32_t            *staid    = &(result->stationid);  
    sta_para_msg1005_t    *sta_para = result->sta_para;
    
    os_int32_t     itrf;      
    double         rr [3];    /* rr[0] abstract from DF025; rr[1] abstract from DF026; rr[2] abstract from DF027 */     
    os_int32_t     i = RTCM_FRAME_HEAD_LEN + RTCM_DF002_LEN;
    os_int32_t     j;
    
    if (i + RTCM_MSG1005_LEN - RTCM_DF002_LEN == buf_len * 8) 
    {
        *staid = getbitu(buff, i, RTCM_DF003_LEN); 
        i     += RTCM_DF003_LEN;
        
        itrf   = getbitu(buff, i, RTCM_DF021_LEN); 
        i     += RTCM_DF021_LEN + 4;
        
        rr[0]  = getbits_38(buff, i); 
        i     += RTCM_DF025_LEN + 2;
        
        rr[1]  = getbits_38(buff, i); 
        i     += RTCM_DF026_LEN + 2;
        
        rr[2]  = getbits_38(buff, i);
    }
    else 
    {
        os_kprintf("rtcm3 1005 length error: len=%d\n", buf_len);
        return -1;
    }
    
    memset(sta_para, 0, sizeof(sta_para_msg1005_t));
       
    sta_para->deltype = 0;                                       /* xyz */
    for (j = 0; j < 3; j++)
    {
        sta_para->pos[j] = rr[j] * 0.0001;
        sta_para->del[j] = 0.0;
    }
    sta_para->hgt  = 0.0;
    sta_para->itrf = itrf;
    return OS_EOK;
}


/**
 ***********************************************************************************************************************
 * @brief           decode type 1033: receiver and antenna descriptor
 * 
 * @details         Optional
 * 
 * @attention       Optional
 * 
 * @param[in/out]   rtcm_msg_1033_t   *result
 * 
 * @return          os_int32_t
 ***********************************************************************************************************************
 */
os_int32_t decode_type1033(os_uint8_t *buff, os_uint32_t buf_len, rtcm_msg_1033_t *result)
{
    os_int32_t            *staid    = &(result->stationid);  
    sta_para_msg1033_t    *sta_para = result->sta_para;
    
    char        des[32]    = "";
    char        sno[32]    = "";
    char        rec[32]    = "";
    char        ver[32]    = "";
    char        rsn[32]    = "";
    os_int32_t  i          = RTCM_FRAME_HEAD_LEN + RTCM_DF002_LEN;
    os_int32_t  index;
    os_int32_t  j;
    os_int32_t  n;
    os_int32_t  m;
    os_int32_t  n1;
    os_int32_t  n2;
    os_int32_t  n3;
    os_int32_t  setup;

    index = i + RTCM_DF003_LEN;                                                 
    n     = getbitu(buff, index, 8);                                       
    
    index = index + RTCM_DF029_LEN + 8 * n + RTCM_DF031_LEN;                     
    m     = getbitu(buff, index, 8);
    
    index = index + RTCM_DF032_LEN + 8 * m;   
    n1    = getbitu(buff, index, 8);
    
    index = index + RTCM_DF227_LEN + 8 * n1;  
    n2    = getbitu(buff, index, 8);
    
    index = index + RTCM_DF229_LEN + 8 * n2;   
    n3    = getbitu(buff, index, 8);
    
    if (i + 60 + 8*(n + m + n1 + n2 + n3) <= buf_len * 8) 
    {
        *staid = getbitu(buff, i, RTCM_DF003_LEN); 
        i     += RTCM_DF003_LEN + RTCM_DF029_LEN;

        
        for (j = 0; j < n && j < 31; j++) 
        {
            des[j] = (char)getbitu(buff, i, 8); 
            i     += SINGLE_ANTENNA_DESCRIPTOR_LEN;
        }

        
        setup = getbitu(buff, i, 8);
        i    += RTCM_DF031_LEN + RTCM_DF032_LEN;


        for (j = 0; j < m && j < 31; j++) 
        {
            sno[j] = (char)getbitu(buff, i, 8); 
            i     += SINGLE_SERIAL_NUMBLE_LEN;
        }
        i += RTCM_DF227_LEN;

        
        for (j = 0; j < n1 && j < 31; j++) 
        {
            rec[j] = (char)getbitu(buff, i, 8); 
            i     += SINGLE_RECEIVER_TYPE_DESCRIPTOR_LEN;
        }
        i += RTCM_DF229_LEN;

        
        for (j = 0; j < n2 && j < 31; j++) 
        {
            ver[j] = (char)getbitu(buff, i, 8); 
            i     += SINGLE_RECEIVER_FIRMWARE_VERSION_LEN;
        }
        i += RTCM_DF231_LEN;

        
        for (j = 0; j < n3 && j < 31; j++) 
        {
            rsn[j] = (char)getbitu(buff, i, 8); 
            i     += SINGLE_RECEIVER_SERIAL_NUMBER_LEN;
        }
    }
    else 
    {
        os_kprintf("rtcm3 1033 length error: len=%d\n", buf_len);
        return OS_ERROR;
    }
    
 
    memset(sta_para, 0, sizeof(sta_para_msg1033_t));
    
    strncpy(sta_para->antdes, des, n); 
    sta_para->antdes[n] = '\0';
    
    sta_para->antsetup = setup;
    
    strncpy(sta_para->antsno, sno, m);
    sta_para->antsno[m] = '\0';
    
    strncpy(sta_para->rectype, rec, n1); 
    sta_para->rectype[n1] = '\0';
    
    strncpy(sta_para->recver, ver, n2); 
    sta_para->recver [n2] = '\0';
    
    strncpy(sta_para->recsno, rsn, n3); 
    sta_para->recsno [n3] = '\0';
    
    os_kprintf("rtcm3 1033: ant=%s:%s rec=%s:%s:%s\n", des, sno, rec, ver, rsn);
    return OS_EOK;
}

