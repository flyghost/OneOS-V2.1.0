/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        syslog.h
 *
 * @brief       Header file for syslog.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __SYSLOG_H__
#define __SYSLOG_H__

#include <os_stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 *
 * priorities (these are ordered)
 */
#define LOG_EMERG               0   /* System is unusable */
#define LOG_ALERT               1   /* Action must be taken immediately */
#define LOG_CRIT                2   /* Critical conditions */
#define LOG_ERR                 3   /* Error conditions */
#define LOG_WARNING             4   /* Warning conditions */
#define LOG_NOTICE              5   /* Normal but significant condition */
#define LOG_INFO                6   /* Informational */
#define LOG_DEBUG               7   /* Debug-level messages */

/* Acility codes */
#define LOG_KERN                (0 << 3)    /* Kernel messages */
#define LOG_USER                (1 << 3)    /* Random user-level messages */
#define LOG_MAIL                (2 << 3)    /* Mail system */
#define LOG_DAEMON              (3 << 3)    /* System daemons */
#define LOG_AUTH                (4 << 3)    /* Security/authorization messages */
#define LOG_SYSLOG              (5 << 3)    /* Messages generated internally by syslogd */
#define LOG_LPR                 (6 << 3)    /* Line printer subsystem */
#define LOG_NEWS                (7 << 3)    /* Network news subsystem */
#define LOG_UUCP                (8 << 3)    /* UUCP subsystem */
#define LOG_CRON                (9 << 3)    /* Clock daemon */
#define LOG_AUTHPRIV            (10 << 3)   /* Security/authorization messages (private) */

/* Other codes through 15 reserved for system use */
#define LOG_LOCAL0              (16 << 3)   /* Reserved for local use */
#define LOG_LOCAL1              (17 << 3)   /* Reserved for local use */
#define LOG_LOCAL2              (18 << 3)   /* Reserved for local use */
#define LOG_LOCAL3              (19 << 3)   /* Reserved for local use */
#define LOG_LOCAL4              (20 << 3)   /* Reserved for local use */
#define LOG_LOCAL5              (21 << 3)   /* Reserved for local use */
#define LOG_LOCAL6              (22 << 3)   /* Reserved for local use */
#define LOG_LOCAL7              (23 <<3 )   /* Reserved for local use */

#define LOG_NFACILITIES         24          /* Current number of facilities */


/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define LOG_PID                 0x01    /* Log the pid with each message */
#define LOG_CONS                0x02    /* Log on the console if errors in sending */
#define LOG_ODELAY              0x04    /* Delay open until first syslog() (default) */
#define LOG_NDELAY              0x08    /* Don't delay open */
#define LOG_NOWAIT              0x10    /* Don't wait for console forks: DEPRECATED */
#define LOG_PERROR              0x20    /* Log to stderr as well */

extern void closelog(void);
extern void openlog(const char *ident, int option, int facility);
extern void syslog(int priority, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SYSLOG_H__ */

