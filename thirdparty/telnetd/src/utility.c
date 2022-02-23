/*
  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
  2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
  2013, 2014, 2015 Free Software Foundation, Inc.

  This file is part of GNU Inetutils.

  GNU Inetutils is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  GNU Inetutils is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see `http://www.gnu.org/licenses/'. */
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
 * @file        utility.c
 *
 * @brief       telnet input/output process code
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#define TELOPTS
#define TELCMDS
//#define SLC_NAMES
#include "telnetd.h"
#include <stdarg.h>
#ifdef HAVE_TERMIO_H
# include <termio.h>
#endif
#include <time.h>

#ifdef HAVE_TERMCAP_TGETENT
# include <termcap.h>
#elif defined HAVE_CURSES_TGETENT
# include <curses.h>
# include <term.h>
#endif

#if defined HAVE_STREAMSPTY && defined HAVE_GETMSG	\
  && defined HAVE_STROPTS_H
# include <stropts.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <dlog.h>
#include "os_mutex.h"

#define LOG_TAG "TELNET_UTILI"

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif
static char netobuf[BUFSIZ + NETSLOP], *nfrontp, *nbackp;
static char *neturg;		/* one past last byte of urgent data */

static char ptyobuf[BUFSIZ + NETSLOP], *pfrontp, *pbackp;

static char netibuf[BUFSIZ], *netip;
static int ncc;

static char ptyibuf[BUFSIZ], *ptyip;
static int pcc;
static os_mutex_t *pty_in, *pty_out;
extern int not42;

static os_size_t pty_output_datalen(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
static os_size_t pty_read(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
static struct os_device_ops telnetd_ops = {
    .read = pty_output_datalen,
    .write = pty_read,
};
/* ************************************************************************* */
/* Net and PTY I/O functions */
os_device_t telnetd_dev;
void io_setup(void)
{
    /* register telnet device */
    telnetd_dev.type = OS_DEVICE_TYPE_CHAR;
    telnetd_dev.ops = &telnetd_ops;

    /* register telnet device */
    os_device_register(&telnetd_dev, TELNETD_NAME);

    pfrontp = pbackp = ptyobuf;
    nfrontp = nbackp = netobuf;

    netip = netibuf;
    ptyip = ptyibuf;

    pty_in = os_mutex_create("pty_in", OS_FALSE);
    pty_out = os_mutex_create("pty_out", OS_FALSE);
}

void set_neturg(void)
{
    neturg = nfrontp - 1;
}

/* net-buffers */

void net_output_byte(int c)
{
    *nfrontp++ = c;
}

int net_output_data(const char *format, ...)
{
    va_list args;
    size_t remaining, ret;

    va_start(args, format);
    remaining = BUFSIZ - (nfrontp - netobuf);
    /* try a netflush() if the room is too low */
    if (strlen(format) > remaining || BUFSIZ / 4 > remaining)
    {
        netflush();
        remaining = BUFSIZ - (nfrontp - netobuf);
    }
    ret = vsnprintf(nfrontp, remaining, format, args);
    nfrontp += ((ret < remaining - 1) ? ret : remaining - 1);
    va_end(args);
    return ret;
}

int net_output_datalen(const void *buf, size_t l)
{
    size_t remaining;

    remaining = BUFSIZ - (nfrontp - netobuf);
    if (remaining < l)
    {
        netflush();
        remaining = BUFSIZ - (nfrontp - netobuf);
    }
    if (remaining < l)
        return -1;
    memmove(nfrontp, buf, l);
    nfrontp += l;
    return (int) l;
}

int net_input_level(void)
{
    return ncc;
}

int net_output_level(void)
{
    return nfrontp - nbackp;
}

int net_buffer_is_full(void)
{
    return (&netobuf[BUFSIZ] - nfrontp) < 2;
}

int net_get_char(int peek)
{
    if (peek)
        return *netip;
    else if (ncc > 0)
    {
        ncc--;
        return *netip++ & 0377;
    }

    return 0;
}

int net_read(void)
{
    ncc = recv(net, netibuf, sizeof(netibuf), 0);
    if (ncc > 0)
    {
        netip = netibuf;
        DEBUG(debug_report, 1,
              LOG_W(LOG_TAG, "td: netread %d chars\r\n", ncc));
        DEBUG(debug_net_data, 1, printdata("nd", netip, ncc));
    }
    return ncc;
}

/* PTY buffer functions */
int pty_buffer_is_full(void)
{
    int tmp, ret;
    ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    tmp = &ptyobuf[BUFSIZ] - pfrontp;
    os_mutex_unlock(pty_out);
    return tmp < 2;
}

void pty_output_byte(int c)
{
    int ret;
    ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    *pfrontp++ = c;
    os_mutex_unlock(pty_out);
}

// for shell read
os_size_t pty_output_datalen(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    int tmp, ret;
    ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    tmp = pfrontp - pbackp;
    tmp = MIN(tmp, size);
    
    if (tmp > 0)
    {
        DEBUG(debug_report, 1,
              LOG_W(LOG_TAG, "td: ptyoutput %d chars\r\n", tmp));
        DEBUG(debug_pty_data, 1, printdata("pd", pbackp, tmp));
        memcpy(buffer, pbackp, tmp);
    }

    if (tmp < 0)
    {
        *(char *) buffer = '\0';
        tmp = 0;
    }

    pbackp += tmp;
    if (pbackp == pfrontp)
        pbackp = pfrontp = ptyobuf;
    os_mutex_unlock(pty_out);
    return tmp;
}

int pty_input_level(void)
{
    int tmp, ret;
    ret = os_mutex_lock(pty_in, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    tmp = pcc;
    os_mutex_unlock(pty_in);
    
    return tmp;
}

int pty_output_level(void)
{
    int tmp, ret;
    ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    tmp = pfrontp - pbackp;
    os_mutex_unlock(pty_out);
    return tmp;
}

void ptyflush()
{
    int ret;
    struct os_device_cb_info info;

    ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    
    info.size = pfrontp - pbackp;
    /* indicate there are reception data */
    if (info.size > 0)
    {
        os_device_recv_notify(&telnetd_dev);
    }

    os_mutex_unlock(pty_out);
    return;
}

int pty_get_char(int peek)
{
    int ret;
    char tmp = 0;
    ret = os_mutex_lock(pty_in, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }
    if (peek)
    {
        tmp = *ptyip;
    }
    else if (pcc > 0)
    {
        pcc--;
        tmp = *ptyip++ & 0377;
    }
    os_mutex_unlock(pty_in);

    return tmp;
}

// for shell write
os_size_t pty_read(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    int tmp;

    tmp = MIN(size, BUFSIZ - pcc);

    // os_kprintf will irq lock fisrt
    /*ret = os_mutex_lock(pty_in, OS_WAIT_FOREVER);
    if (ret != OS_EOK)
    {
        LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
    }*/
    
    memcpy(ptyibuf + pcc, buffer, tmp);
    pcc += tmp;
    ptyip = ptyibuf;
    DEBUG(debug_report, 1, LOG_W(LOG_TAG, "td: ptyread %d chars\r\n", tmp));
    DEBUG(debug_pty_data, 1, printdata("pd", ptyip, tmp));
    /*os_mutex_unlock(pty_in);*/
    if ((&netobuf[BUFSIZ] - nfrontp) >= pcc)
    {
        while (pcc > 0) /* pty in data move to net out buffer */
        {
            net_output_byte(*ptyip++);
            pcc--;
        }
    }
    else // buffer is full and cant store pty in data
    {
        if (tmp == 0)
        {
            return (os_size_t)-1;
        }
    }

    return tmp;
}

/* ************************************************************************* */


/* io_drain ()
 *
 *
 *	A small subroutine to flush the network output buffer, get some data
 * from the network, and pass it through the telnet state machine.  We
 * also flush the pty input buffer (by dropping its data) if it becomes
 * too full.
 */

void io_drain(void)
{
    fd_set rfds;

    DEBUG(debug_report, 1, LOG_W(LOG_TAG, "td: ttloop\r\n"));
    if (nfrontp - nbackp > 0)
        netflush();

    FD_ZERO(&rfds);
    FD_SET(net, &rfds);
    if (1 != select(net + 1, &rfds, NULL, NULL, NULL))
    {
        LOG_I(LOG_TAG,  "ttloop:  select: %m\n");
        // exit(EXIT_FAILURE);
        return;
    }

    ncc = recv(net, netibuf, sizeof(netibuf), 0);
    if (ncc < 0)
    {
        LOG_I(LOG_TAG,  "ttloop:  read: %m\n");
        // exit(EXIT_FAILURE);
        return;
    }
    else if (ncc == 0)
    {
        LOG_I(LOG_TAG,  "ttloop:  peer died: %m\n");
        // exit(EXIT_FAILURE);
        return;
    }
    DEBUG(debug_report, 1,
          LOG_W(LOG_TAG, "td: ttloop read %d chars\r\n", ncc));
    netip = netibuf;
    telrcv();   /* state machine */
    if (ncc > 0)
    {
        int ret;
        ret = os_mutex_lock(pty_out, OS_WAIT_FOREVER);
        if (ret != OS_EOK)
        {
            LOG_E(LOG_TAG, "os_mutex_lock= %d", ret);
        }
        pfrontp = pbackp = ptyobuf;
        os_mutex_unlock(pty_out);
        telrcv();
    }
}

/*
 * Check a descriptor to see if out of band data exists on it.
 */
/* int	s; socket number */
int stilloob(int s)
{
    static struct timeval timeout = { 0, 0 };
    fd_set excepts;
    int value;

    do
    {
        FD_ZERO(&excepts);
        FD_SET(s, &excepts);
        value = select(s + 1, (fd_set *) 0, (fd_set *) 0, &excepts, &timeout);
    }
    while (value == -1 && errno == EINTR);

    return FD_ISSET(s, &excepts);
}

/*
 * nextitem()
 *
 *	Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */
char *nextitem(char *current)
{
    if ((*current & 0xff) != IAC)
        return current + 1;

    char *look = current + 2;
    switch (*(current + 1) & 0xff)
    {
        case DO:
        case DONT:
        case WILL:
        case WONT:
            return current + 3;

        case SB:			/* loop forever looking for the SE */
        {
            // char *look = current + 2;

            for (;;)
                if ((*look++ & 0xff) == IAC && (*look++ & 0xff) == SE)
                    return look;

            default:
                return current + 2;
            }
    }
}				/* end of nextitem */


/*
 * netclear()
 *
 *	We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *	Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *	A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */
#define wewant(p)					\
  ((nfrontp > p) && ((*p&0xff) == IAC) &&		\
   ((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))


void netclear(void)
{
    char *thisitem, *next;
    char *good;

    thisitem = netobuf;

    while ((next = nextitem(thisitem)) <= nbackp)
        thisitem = next;

    /* Now, thisitem is first before/at boundary. */

    good = netobuf;		/* where the good bytes go */

    while (nfrontp > thisitem)
    {
        if (wewant(thisitem))
        {
            int length;

            for (next = thisitem; wewant(next) && nfrontp > next;
                    next = nextitem(next))
                ;

            length = next - thisitem;
            memmove(good, thisitem, length);
            good += length;
            thisitem = next;
        }
        else
        {
            thisitem = nextitem(thisitem);
        }
    }

    nbackp = netobuf;
    nfrontp = good;		/* next byte to be sent */
    neturg = 0;
}				/* end of netclear */

/*
 *  netflush
 *		Send as much data as possible to the network,
 *	handling requests for urgent data.
 */
void netflush(void)
{
    int n;

    if ((n = nfrontp - nbackp) > 0)
    {
        /*
         * if no urgent data, or if the other side appears to be an
         * old 4.2 client (and thus unable to survive TCP urgent data),
         * write the entire buffer in non-OOB mode.
         */
        if (!neturg || !not42)
            n = send(net, nbackp, n, 0);	/* normal write */
        else
        {
            n = neturg - nbackp;
            /*
             * In 4.2 (and 4.3) systems, there is some question about
             * what byte in a sendOOB operation is the "OOB" data.
             * To make ourselves compatible, we only send ONE byte
             * out of band, the one WE THINK should be OOB (though
             * we really have more the TCP philosophy of urgent data
             * rather than the Unix philosophy of OOB data).
             */
            if (n > 1)
                n = send(net, nbackp, n - 1, 0);	/* send URGENT all by itself */
            else
                n = send(net, nbackp, n, MSG_OOB);	/* URGENT data */
        }
    }
    if (n < 0)
    {
        if (errno == EWOULDBLOCK || errno == EINTR)
            return;
        // cleanup(0);
        closesocket(net);
        /* NOT REACHED */
    }

    nbackp += n;

    if (nbackp >= neturg)
        neturg = 0;

    if (nbackp == nfrontp)
    {
        nbackp = nfrontp = netobuf;
    }
    DEBUG(debug_report, 1, LOG_W(LOG_TAG, "td: netflush %d chars\r\n", n));

}/* end of netflush */

/*
 * Print telnet options and commands in plain text, if possible.
 */
void printoption(char *fmt, int option)
{
#ifdef TELOPTS
    if (TELOPT_OK(option))
        LOG_W(LOG_TAG, "%s %s\r\n", fmt, TELOPT(option));
    else if (TELCMD_OK(option))
        LOG_W(LOG_TAG, "%s %s\r\n", fmt, TELCMD(option));
    else
#endif    
        LOG_W(LOG_TAG, "%s %d\r\n", fmt, option);
}

#ifdef TELNETD_SUBOPTION
/* char direction; '<' or '>' */
/* unsigned char *pointer;  where suboption data sits */
/* int length; length of suboption data */
void printsub(int direction, unsigned char *pointer, int length)
{
    int i = 0;

    /* Silence unwanted debugging to '/tmp/telnet.debug'.
     *
     * XXX: Better location?
     */
    if ((pointer[0] == TELOPT_AUTHENTICATION && telnetd_debug_level[debug_auth] < 1)
            || (pointer[0] == TELOPT_ENCRYPT && telnetd_debug_level[debug_encr] < 1))
        return;

    if (direction)
    {
        LOG_W(LOG_TAG, "td: %s suboption ",
                          direction == '<' ? "recv" : "send");
        if (length >= 3)
        {
            int j;

            i = pointer[length - 2];
            j = pointer[length - 1];

            if (i != IAC || j != SE)
            {
                LOG_W(LOG_TAG, "(terminated by ");
                if (TELOPT_OK(i))
                    LOG_W(LOG_TAG, "%s ", TELOPT(i));
                else if (TELCMD_OK(i))
                    LOG_W(LOG_TAG, "%s ", TELCMD(i));
                else
                    LOG_W(LOG_TAG, "%d ", i);
                if (TELOPT_OK(j))
                    LOG_W(LOG_TAG, "%s", TELOPT(j));
                else if (TELCMD_OK(j))
                    LOG_W(LOG_TAG, "%s", TELCMD(j));
                else
                    LOG_W(LOG_TAG, "%d", j);
                LOG_W(LOG_TAG, ", not IAC SE!) ");
            }
        }
        length -= 2;
    }
    if (length < 1)
    {
        LOG_W(LOG_TAG, "(Empty suboption??\?)");
        return;
    }

    switch (pointer[0])
    {
        case TELOPT_TTYPE:
            LOG_W(LOG_TAG, "TERMINAL-TYPE ");
            switch (pointer[1])
            {
                case TELQUAL_IS:
                    LOG_W(LOG_TAG, "IS \"%.*s\"", length - 2, (char *) pointer + 2);
                    break;

                case TELQUAL_SEND:
                    LOG_W(LOG_TAG, "SEND");
                    break;

                default:
                    LOG_W(LOG_TAG, "- unknown qualifier %d (0x%x).",
                                      pointer[1], pointer[1]);
            }
            break;

        case TELOPT_TSPEED:
            LOG_W(LOG_TAG, "TERMINAL-SPEED");
            if (length < 2)
            {
                LOG_W(LOG_TAG, " (empty suboption??\?)");
                break;
            }

            switch (pointer[1])
            {
                case TELQUAL_IS:
                    LOG_W(LOG_TAG, " IS %.*s", length - 2, (char *) pointer + 2);
                    break;

                default:
                    if (pointer[1] == 1)
                        LOG_W(LOG_TAG, " SEND");
                    else
                        LOG_W(LOG_TAG, " %d (unknown)", pointer[1]);
                    for (i = 2; i < length; i++)
                    {
                        LOG_W(LOG_TAG, " ?%d?", pointer[i]);
                    }
                    break;
            }
            break;

        case TELOPT_LFLOW:
            LOG_W(LOG_TAG, "TOGGLE-FLOW-CONTROL");
            if (length < 2)
            {
                LOG_W(LOG_TAG, " (empty suboption??\?)");
                break;
            }

            switch (pointer[1])
            {
                case LFLOW_OFF:
                    LOG_W(LOG_TAG, " OFF");
                    break;

                case LFLOW_ON:
                    LOG_W(LOG_TAG, " ON");
                    break;

                case LFLOW_RESTART_ANY:
                    LOG_W(LOG_TAG, " RESTART-ANY");
                    break;

                case LFLOW_RESTART_XON:
                    LOG_W(LOG_TAG, " RESTART-XON");
                    break;

                default:
                    LOG_W(LOG_TAG, " %d (unknown)", pointer[1]);
            }

            for (i = 2; i < length; i++)
                LOG_W(LOG_TAG, " ?%d?", pointer[i]);
            break;

        case TELOPT_NAWS:
            LOG_W(LOG_TAG, "NAWS");
            if (length < 2)
            {
                LOG_W(LOG_TAG, " (empty suboption??\?)");
                break;
            }
            if (length == 2)
            {
                LOG_W(LOG_TAG, " ?%d?", pointer[1]);
                break;
            }

            LOG_W(LOG_TAG, " %d %d (%d)",
                              pointer[1], pointer[2],
                              (int)((((unsigned int) pointer[1]) << 8) |
                                    ((unsigned int) pointer[2])));
            if (length == 4)
            {
                LOG_W(LOG_TAG, " ?%d?", pointer[3]);
                break;
            }

            LOG_W(LOG_TAG, " %d %d (%d)",
                              pointer[3], pointer[4],
                              (int)((((unsigned int) pointer[3]) << 8) |
                                    ((unsigned int) pointer[4])));
            for (i = 5; i < length; i++)
                LOG_W(LOG_TAG, " ?%d?", pointer[i]);
            break;

        case TELOPT_LINEMODE:
            LOG_W(LOG_TAG, "LINEMODE ");
            if (length < 2)
            {
                LOG_W(LOG_TAG, " (empty suboption??\?)");
                break;
            }

            switch (pointer[1])
            {
                case WILL:
                    LOG_W(LOG_TAG, "WILL ");
                    goto common;

                case WONT:
                    LOG_W(LOG_TAG, "WONT ");
                    goto common;

                case DO:
                    LOG_W(LOG_TAG, "DO ");
                    goto common;

                case DONT:
                    LOG_W(LOG_TAG, "DONT ");

common:
                    if (length < 3)
                    {
                        LOG_W(LOG_TAG, "(no option??\?)");
                        break;
                    }
                    switch (pointer[2])
                    {
                        case LM_FORWARDMASK:
                            LOG_W(LOG_TAG, "Forward Mask");
                            for (i = 3; i < length; i++)
                                LOG_W(LOG_TAG, " %x", pointer[i]);
                            break;

                        default:
                            LOG_W(LOG_TAG, "%d (unknown)", pointer[2]);
                            for (i = 3; i < length; i++)
                                LOG_W(LOG_TAG, " %d", pointer[i]);
                            break;
                    }
                    break;

                case LM_SLC:
                    LOG_W(LOG_TAG, "SLC");
                    for (i = 2; i < length - 2; i += 3)
                    {
                        if (SLC_NAME_OK(pointer[i + SLC_FUNC]))
                            LOG_W(LOG_TAG, " %s", SLC_NAME(pointer[i + SLC_FUNC]));
                        else
                            LOG_W(LOG_TAG, " %d", pointer[i + SLC_FUNC]);
                        switch (pointer[i + SLC_FLAGS] & SLC_LEVELBITS)
                        {
                            case SLC_NOSUPPORT:
                                LOG_W(LOG_TAG, " NOSUPPORT");
                                break;

                            case SLC_CANTCHANGE:
                                LOG_W(LOG_TAG, " CANTCHANGE");
                                break;

                            case SLC_VARIABLE:
                                LOG_W(LOG_TAG, " VARIABLE");
                                break;

                            case SLC_DEFAULT:
                                LOG_W(LOG_TAG, " DEFAULT");
                                break;
                        }

                        LOG_W(LOG_TAG, "%s%s%s",
                                          pointer[i +
                                                    SLC_FLAGS] & SLC_ACK ? "|ACK" : "",
                                          pointer[i +
                                                    SLC_FLAGS] & SLC_FLUSHIN ? "|FLUSHIN"
                                          : "",
                                          pointer[i +
                                                    SLC_FLAGS] & SLC_FLUSHOUT ?
                                          "|FLUSHOUT" : "");
                        if (pointer[i + SLC_FLAGS] &
                                ~(SLC_ACK | SLC_FLUSHIN | SLC_FLUSHOUT | SLC_LEVELBITS))
                            LOG_W(LOG_TAG, "(0x%x)", pointer[i + SLC_FLAGS]);
                        LOG_W(LOG_TAG, " %d;", pointer[i + SLC_VALUE]);

                        if ((pointer[i + SLC_VALUE] == IAC) &&
                                (pointer[i + SLC_VALUE + 1] == IAC))
                            i++;
                    }

                    for (; i < length; i++)
                        LOG_W(LOG_TAG, " ?%d?", pointer[i]);
                    break;

                case LM_MODE:
                    LOG_W(LOG_TAG, "MODE ");
                    if (length < 3)
                    {
                        LOG_W(LOG_TAG, "(no mode??\?)");
                        break;
                    }
                    {
                        char tbuf[32];
                        snprintf(tbuf, sizeof(tbuf), "%s%s%s%s%s",
                                 pointer[2] & MODE_EDIT ? "|EDIT" : "",
                                 pointer[2] & MODE_TRAPSIG ? "|TRAPSIG" : "",
                                 pointer[2] & MODE_SOFT_TAB ? "|SOFT_TAB" : "",
                                 pointer[2] & MODE_LIT_ECHO ? "|LIT_ECHO" : "",
                                 pointer[2] & MODE_ACK ? "|ACK" : "");
                        LOG_W(LOG_TAG, "%s", tbuf[1] ? &tbuf[1] : "0");
                    }

                    if (pointer[2] & ~(MODE_EDIT | MODE_TRAPSIG | MODE_ACK))
                        LOG_W(LOG_TAG, " (0x%x)", pointer[2]);

                    for (i = 3; i < length; i++)
                        LOG_W(LOG_TAG, " ?0x%x?", pointer[i]);
                    break;

                default:
                    LOG_W(LOG_TAG, "%d (unknown)", pointer[1]);
                    for (i = 2; i < length; i++)
                        LOG_W(LOG_TAG, " %d", pointer[i]);
            }
            break;

        case TELOPT_STATUS:
        {
            char *cp;
            int j, k;

            LOG_W(LOG_TAG, "STATUS");

            switch (pointer[1])
            {
                default:
                    if (pointer[1] == TELQUAL_SEND)
                        LOG_W(LOG_TAG, " SEND");
                    else
                        LOG_W(LOG_TAG, " %d (unknown)", pointer[1]);
                    for (i = 2; i < length; i++)
                        LOG_W(LOG_TAG, " ?%d?", pointer[i]);
                    break;

                case TELQUAL_IS:
                    LOG_W(LOG_TAG, " IS\r\n");

                    for (i = 2; i < length; i++)
                    {
                        switch (pointer[i])
                        {
                            case DO:
                                cp = "DO";
                                goto common2;

                            case DONT:
                                cp = "DONT";
                                goto common2;

                            case WILL:
                                cp = "WILL";
                                goto common2;

                            case WONT:
                                cp = "WONT";
                                goto common2;

common2:
                                i++;
                                if (TELOPT_OK(pointer[i]))
                                    LOG_W(LOG_TAG, " %s %s\r\n", cp,
                                                      TELOPT(pointer[i]));
                                else
                                    LOG_W(LOG_TAG, " %s %d\r\n", cp, pointer[i]);
                                break;

                            case SB:
                                LOG_W(LOG_TAG, " SB ");
                                i++;
                                j = k = i;
                                while (j < length)
                                {
                                    if (pointer[j] == SE)
                                    {
                                        if (j + 1 == length)
                                            break;
                                        if (pointer[j + 1] == SE)
                                            j++;
                                        else
                                            break;
                                    }
                                    pointer[k++] = pointer[j++];
                                }
                                printsub(0, &pointer[i], k - i);
                                if (i < length)
                                {
                                    LOG_W(LOG_TAG, " SE");
                                    i = j;
                                }
                                else
                                    i = j - 1;

                                LOG_W(LOG_TAG, "\r\n");
                                break;

                            default:
                                LOG_W(LOG_TAG, " %d", pointer[i]);
                                break;
                        }
                    }
                    break;
            }
            break;
        }

        case TELOPT_XDISPLOC:
            LOG_W(LOG_TAG, "X-DISPLAY-LOCATION ");
            switch (pointer[1])
            {
                case TELQUAL_IS:
                    LOG_W(LOG_TAG, "IS \"%.*s\"", length - 2, (char *) pointer + 2);
                    break;
                case TELQUAL_SEND:
                    LOG_W(LOG_TAG, "SEND");
                    break;
                default:
                    LOG_W(LOG_TAG, "- unknown qualifier %d (0x%x).",
                                      pointer[1], pointer[1]);
            }
            break;

        case TELOPT_NEW_ENVIRON:
            LOG_W(LOG_TAG, "NEW-ENVIRON ");
            goto env_common1;

        case TELOPT_OLD_ENVIRON:
            LOG_W(LOG_TAG, "OLD-ENVIRON");
env_common1:
            switch (pointer[1])
            {
                case TELQUAL_IS:
                    LOG_W(LOG_TAG, "IS ");
                    goto env_common;

                case TELQUAL_SEND:
                    LOG_W(LOG_TAG, "SEND ");
                    goto env_common;

                case TELQUAL_INFO:
                    LOG_W(LOG_TAG, "INFO ");

env_common:
                    {
                        char *quote = "";
                        for (i = 2; i < length; i++)
                        {
                            switch (pointer[i])
                            {
                                case NEW_ENV_VAR:
                                    LOG_W(LOG_TAG, "%sVAR ", quote);
                                    quote = "";
                                    break;

                                case NEW_ENV_VALUE:
                                    LOG_W(LOG_TAG, "%sVALUE ", quote);
                                    quote = "";
                                    break;

                                case ENV_ESC:
                                    LOG_W(LOG_TAG, "%sESC ", quote);
                                    quote = "";
                                    break;

                                case ENV_USERVAR:
                                    LOG_W(LOG_TAG, "%sUSERVAR ", quote);
                                    quote = "";
                                    break;

                                default:
                                    if (isprint(pointer[i]) && pointer[i] != '"')
                                    {
                                        if (strcmp(quote, "") == 0)
                                        {
                                            LOG_W(LOG_TAG, "\"");
                                            quote = "\" ";
                                        }
                                        debug_output_datalen((char *) &pointer[i], 1);
                                    }
                                    else
                                    {
                                        LOG_W(LOG_TAG, "%s%03o ", quote, pointer[i]);
                                        quote = "";
                                    }
                                    break;
                            }
                        }
                        if (strcmp(quote, "\" ") == 0)
                            LOG_W(LOG_TAG, "\"");
                        break;
                    }
            }
            break;

        default:
            if (TELOPT_OK(pointer[0]))
                LOG_W(LOG_TAG, "%s (unknown)", TELOPT(pointer[0]));
            else
                LOG_W(LOG_TAG, "%d (unknown)", pointer[0]);
            for (i = 1; i < length; i++)
                LOG_W(LOG_TAG, " %d", pointer[i]);
            break;
    }
    LOG_W(LOG_TAG, "\r\n");
}
#endif

/*
 * Dump a data buffer in hex and ascii to the output data stream.
 */
 
void printdata(char *tag, char *ptr, int cnt)
{
    int i;
    char xbuf[30];

    while (cnt)
    {
        /* add a line of output */
        // LOG_W(LOG_TAG, "%s: ", tag);
        dlog_output(DLOG_WARNING, LOG_TAG, OS_TRUE, "%s: ", tag);
        for (i = 0; i < 20 && cnt; i++)
        {
            dlog_output(DLOG_WARNING, LOG_TAG, OS_TRUE, "%02x", (unsigned char) *ptr);
            xbuf[i] = isprint((int) * ptr) ? *ptr : '.';
            cnt--;
            ptr++;
        }

        xbuf[i] = '\0';
        dlog_output(DLOG_WARNING, LOG_TAG, OS_TRUE, " %s\r\n", xbuf);
    }
}

#ifdef TELNETD_EXPANSION
/* ************************************************************************* */
/* String expansion functions */

#define EXP_STATE_CONTINUE 0
#define EXP_STATE_SUCCESS  1
#define EXP_STATE_ERROR    2

struct line_expander
{
    int state;			/* Current state */
    int level;			/* The nesting level */
    char *source;			/* The source string */
    char *cp;			/* Current position in the source */
    struct obstack stk;		/* Obstack for expanded version */
};

static char *_var_short_name(struct line_expander *exp);
static char *_var_long_name(struct line_expander *exp,
                            char *start, int length);
static char *_expand_var(struct line_expander *exp);
static void _expand_cond(struct line_expander *exp);
static void _skip_block(struct line_expander *exp);
static void _expand_block(struct line_expander *exp);

/* Expand a variable referenced by its short one-symbol name.
   Input: exp->cp points to the variable name.
   FIXME: not implemented */
char *_var_short_name(struct line_expander *exp)
{
    char *q;
    char timebuf[64];
    time_t t;

    switch (*exp->cp++)
    {
        case 'a':
#ifdef AUTHENTICATION
            if (auth_level >= 0 && autologin == AUTH_VALID)
                return xstrdup("ok");
#endif
            return NULL;

        case 'd':
            time(&t);
            strftime(timebuf, sizeof(timebuf),
                     "%l:%M%p on %A, %d %B %Y", localtime(&t));
            return xstrdup(timebuf);

        case 'h':
            return xstrdup(remote_hostname);

        case 'l':
            return xstrdup(local_hostname);

        case 'L':
            return xstrdup(line);

        case 't':
            q = strchr(line + 1, '/');
            if (q)
                q++;
            else
                q = line;
            return xstrdup(q);

        case 'T':
            return terminaltype ? xstrdup(terminaltype) : NULL;

        case 'u':
            return user_name ? xstrdup(user_name) : NULL;

        case 'U':
            return getenv("USER") ? xstrdup(getenv("USER")) : xstrdup("");

        default:
            exp->state = EXP_STATE_ERROR;
            return NULL;
    }
}

/* Expand a variable referenced by its long name.
   Input: exp->cp points to initial '('
   FIXME: not implemented */
char *_var_long_name(struct line_expander *exp, char *start, int length)
{
    (void) start;		/* Silence warnings until implemented.  */
    (void) length;
    exp->state = EXP_STATE_ERROR;
    return NULL;
}

/* Expand a variable to its value.
   Input: exp->cp points one character _past_ % (or ?) */
char *_expand_var(struct line_expander *exp)
{
    char *p;
    switch (*exp->cp)
    {
        case '{':
            /* Collect variable name */
            for (p = ++exp->cp; *exp->cp && *exp->cp != '}'; exp->cp++)
                ;
            if (*exp->cp == 0)
            {
                exp->cp = p;
                exp->state = EXP_STATE_ERROR;
                break;
            }
            p = _var_long_name(exp, p, exp->cp - p);
            exp->cp++;
            break;

        default:
            p = _var_short_name(exp);
            break;
    }
    return p;
}

/* Expand a conditional block. A conditional block is:
       %?<var>{true-stmt}[{false-stmt}]
   <var> may be either a one-symbol variable name or (string). The latter
   is not handled yet.
   On input exp->cp points to % character */
void _expand_cond(struct line_expander *exp)
{
    char *p;

    if (*++exp->cp == '?')
    {
        /* condition */
        exp->cp++;
        p = _expand_var(exp);
        if (p)
        {
            _expand_block(exp);
            _skip_block(exp);
        }
        else
        {
            _skip_block(exp);
            _expand_block(exp);
        }
        free(p);
    }
    else
    {
        p = _expand_var(exp);
        if (p)
            obstack_grow(&exp->stk, p, strlen(p));
        free(p);
    }
}

/* Skip the block. If the exp->cp does not point to the beginning of a
   block ({ character), the function does nothing */
void _skip_block(struct line_expander *exp)
{
    int level = exp->level;
    if (*exp->cp != '{')
        return;
    for (; *exp->cp; exp->cp++)
    {
        switch (*exp->cp)
        {
            case '{':
                exp->level++;
                break;

            case '}':
                exp->level--;
                if (exp->level == level)
                {
                    exp->cp++;
                    return;
                }
        }
    }
}

/* Expand a block within the formatted line. Stops either when end of source
   line was reached or the nesting reaches the initial value */
void _expand_block(struct line_expander *exp)
{
    int level = exp->level;
    if (*exp->cp == '{')
    {
        exp->level++;
        exp->cp++;		/*FIXME? */
    }
    while (exp->state == EXP_STATE_CONTINUE)
    {
        for (; *exp->cp && *exp->cp != '%'; exp->cp++)
        {
            switch (*exp->cp)
            {
                case '{':
                    exp->level++;
                    break;

                case '}':
                    exp->level--;
                    if (exp->level == level)
                    {
                        exp->cp++;
                        return;
                    }
                    break;

                case '\\':
                    exp->cp++;
                    break;
            }
            obstack_1grow(&exp->stk, *exp->cp);
        }

        if (*exp->cp == 0)
        {
            obstack_1grow(&exp->stk, 0);
            exp->state = EXP_STATE_SUCCESS;
            break;
        }
        else if (*exp->cp == '%' && exp->cp[1] == '%')
        {
            obstack_1grow(&exp->stk, *exp->cp);
            exp->cp += 2;
            continue;
        }

        _expand_cond(exp);
    }
}

/* Expand a format line */
char *expand_line(const char *line)
{
    char *p = NULL;
    struct line_expander exp;

    exp.state = EXP_STATE_CONTINUE;
    exp.level = 0;
    exp.source = (char *) line;
    exp.cp = (char *) line;
    obstack_init(&exp.stk);
    _expand_block(&exp);
    if (exp.state == EXP_STATE_SUCCESS)
        p = xstrdup(obstack_finish(&exp.stk));
    else
    {
        syslog(LOG_ERR, "can't expand line: %s", line);
        syslog(LOG_ERR, "stopped near %s", exp.cp ? exp.cp : "(END)");
    }
    obstack_free(&exp.stk, NULL);
    return p;
}
#endif
