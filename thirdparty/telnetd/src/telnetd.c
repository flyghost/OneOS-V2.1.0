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
 * @file        telnetd.c
 *
 * @brief       telnet server entry
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sys/socket.h>
#include "oneos_config.h"
#include "telnetd.h"
#include "option_parse.h"
#include "console.h"
#include "shell.h"
#include <dlog.h>

#define LOG_TAG "TELNETD"

static void telnetd_setup(int fd);
static int telnetd_run(void);

static int keepalive = 1;        /* Should the TCP keepalive bit be set */

int telnetd_debug_level[debug_max_mode];    /* Debugging levels */
int debug_tcp = 0;        /* Should the SO_DEBUG be set? */

int net;            /* Network connection socket */
char *user_name;
char line[256];

char options[256];
char do_dont_resp[256];
char will_wont_resp[256];
int linemode;            /* linemode on/off */
// int editmode;            /* edit modes in use */
int useeditmode;        /* edit modes to use */
int lmodetype;          /* Client support for linemode */
// int flowmode;            /* current flow control state */
// int restartany;            /* restart output on any character state */

#ifdef TELNET_SLC
slcfun slctab[NSLC + 1];    /* slc mapping table */
#endif
int SYNCHing;            /* we are in TELNET SYNCH mode */
struct telnetd_clocks clocks;

static void telnetd_main(void *parameter)
{
    int sock = -1;
    struct sockaddr_in server_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        os_kprintf("[telnetd]socket failed! \r\n");
        return;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TELNET_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) != OS_EOK)
    {
        os_kprintf("[telnetd] bind failed \r\n");
        return;
    }

    if (listen(sock, 5) != OS_EOK)
    {
        os_kprintf("[telnetd] listen failed \r\n");
        return;
    }

    os_kprintf("[telnetd] io_setup\r\n");
    // register device
    io_setup();

    memset(line, 0, sizeof(line));
    memset(options, 0, sizeof(options));
    memset(do_dont_resp, 0, sizeof(do_dont_resp));
    memset(will_wont_resp, 0, sizeof(will_wont_resp));
    linemode = 0;
    useeditmode = 0;
    lmodetype = 0;

    while (1)
    {
        struct sockaddr_in addr;
        socklen_t addr_size;
        int client = -1;
        os_kprintf("[telnetd] waiting connection \r\n");
        client = accept(sock, (struct sockaddr *) &addr, &addr_size);
        if (client == OS_ERROR)
        {
            os_kprintf("[telnetd] connect fail \r\n");
            os_task_destroy(os_task_self());
            return;
        }
        os_kprintf("[telnetd] connected to (%s:%d) \r\n", inet_ntoa(addr.sin_addr), addr.sin_port);
        telnetd_setup(client);

        sh_disconnect_console();
        os_console_set_device(TELNETD_NAME);
        /* set console device as shell device */
        sh_reconnect_console();

        telnetd_run();    /* Never returning.  */

        sh_disconnect_console();
        os_console_set_device(OS_CONSOLE_DEVICE_NAME);
        sh_reconnect_console();
        os_kprintf("[telnetd] disconnected \r\n");
        closesocket(sock);
        os_task_destroy(os_task_self());
        return;
    }
}

static struct
{
    char *name;
    int modnum;
} telnetd_debug_mode[debug_max_mode] =
{
    {"options", debug_options},
    {"report", debug_report},
    {"netdata", debug_net_data},
    {"ptydata", debug_pty_data},
    {"auth", debug_auth},
    {"encr", debug_encr},
};

void parse_debug_level(char *str)
{
    int i;
    char *tok;

    if (!str)
    {
        for (i = 0; i < debug_max_mode; i++)
            telnetd_debug_level[telnetd_debug_mode[i].modnum] = MAX_DEBUG_LEVEL;
        return;
    }

    for (tok = strtok(str, ","); tok; tok = strtok(NULL, ","))
    {
        int length, level;
        char *p;

        if (strcmp(tok, "tcp") == 0)
        {
            debug_tcp = 1;
            continue;
        }

        p = strchr(tok, '=');
        if (p)
        {
            length = p - tok;
            level = strtoul(p + 1, NULL, 0);
        }
        else
        {
            length = strlen(tok);
            level = MAX_DEBUG_LEVEL;
        }

        for (i = 0; i < debug_max_mode; i++)
            if (strncmp(telnetd_debug_mode[i].name, tok, length) == 0)
            {
                telnetd_debug_level[telnetd_debug_mode[i].modnum] = level;
                break;
            }

        if (i == debug_max_mode)
            os_kprintf("unknown debug mode: %s", tok);
    }
}

/* static os_err_t parse_opt(int key, char *arg)
{
    switch (key)
    {

        case 'D':
            parse_debug_level(arg);
            break;

        case 'n':
            keepalive = 0;
            break;

        default:
            return OS_EINVAL;
    }

    return OS_EOK;
} */

static void telnetd(int argc, char **argv)
{
    os_task_t *task = OS_NULL;
    // os_int32_t  opt_ret;
    // os_int32_t  ret;
    // opt_state_t state;

    os_kprintf("[telnetd] start\r\n");
    
/*
    opt_init(&state, 1);
    while (1)
    {
        opt_ret = opt_get(argc, argv, "D:n:", &state);
        if (opt_ret == OPT_EOF)
        {
            return;
        }

        if ((opt_ret == OPT_BADOPT) || (opt_ret == OPT_BADARG))
        {
            ret = OS_ERROR;
            return;
        }
        ret = parse_opt(opt_ret, state.opt_arg);        
        if (ret != OS_EOK)
        {
            return;
        }
    }
*/

    task = os_task_create("telnetd", telnetd_main, OS_NULL, 2048, OS_TASK_PRIORITY_MAX/3);
    if (task)
    {
        os_task_startup(task);
    }
}
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(telnetd, telnetd, "telnetd server start");
#endif /* OS_USING_SHELL */

void telnetd_setup(int fd)
{
    int optrue = 1;

    /* Set socket options */
    if (keepalive && setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,(char *) &optrue, sizeof(optrue)) < 0)
        LOG_W(LOG_TAG, "setsockopt (SO_KEEPALIVE): %m");

    if (debug_tcp && setsockopt(fd, SOL_SOCKET, SO_DEBUG, (char *) &optrue, sizeof(optrue)) < 0)
        LOG_W(LOG_TAG, "setsockopt (SO_DEBUG): %m");

    net = fd;

#if defined SO_OOBINLINE
    setsockopt(net, SOL_SOCKET, SO_OOBINLINE, (char *) &optrue, sizeof(optrue));
#endif
}

int telnetd_run(void)
{
    int nfd;

   if (my_state_is_wont(TELOPT_SGA))
        send_will(TELOPT_SGA, 1);

    /* Old BSD 4.2 clients are unable to deal with TCP out-of-band data.
       To find out, we send out a "DO ECHO". If the remote side is
       a BSD 4.2 it will answer "WILL ECHO". See the response processing
       below. */
    send_do(TELOPT_ECHO, 1);
    if (his_state_is_wont(TELOPT_LINEMODE))
    {
        /* Query the peer for linemode support by trying to negotiate
           the linemode option. */
        linemode = 1;
        // editmode = 0;
        send_do(TELOPT_LINEMODE, 1);    /* send do linemode */
    }

    //send_do(TELOPT_NAWS, 1);
    send_will(TELOPT_STATUS, 1);
    // flowmode = 0;            /* default flow control state */
    // restartany = -1;        /* uninitialized... */
   /* send_do(TELOPT_LFLOW, 1);*/

    /* Wait for a response from the DO ECHO. Reportedly, some broken
       clients might not respond to it. To work around this, we wait
       for a response to NAWS, which should have been processed after
       DO ECHO (most dumb telnets respond with WONT for a DO that
       they don't understand).
       On the other hand, the client might have sent WILL NAWS as
       part of its startup code, in this case it surely should have
       answered our DO ECHO, so the second loop is waiting for
       the ECHO to settle down.  */
/*
    ttloop(his_will_wont_is_changing(TELOPT_NAWS));

    if (his_want_state_is_will(TELOPT_ECHO) && his_state_is_will(TELOPT_NAWS))
        ttloop(his_will_wont_is_changing(TELOPT_ECHO));
*/

    /* If the remote client is badly broken and did not respond to our
       DO ECHO, we simulate the receipt of a will echo. This will also
       send a WONT ECHO to the client, since we assume that the client
       failed to respond because it believes that it is already in DO ECHO
       mode, which we do not want. */

    if (his_want_state_is_will(TELOPT_ECHO))
    {
        DEBUG(debug_options, 1, LOG_W(LOG_TAG, "td: simulating recv\r\n"));
        willoption(TELOPT_ECHO);
    }

    /* Turn on our echo */
    if (my_state_is_wont(TELOPT_ECHO))
        send_will(TELOPT_ECHO, 1);

    /* Pick up anything received during the negotiations */
    telrcv();

    DEBUG(debug_report, 1,
          LOG_W(LOG_TAG, "td: Entering processing loop\r\n"));

    nfd = net + 1;

    for (;;)
    {
        fd_set ibits, obits, xbits;
        int c;
        struct timeval timeout = {0, 5};
        int tmp_pi = 0;
        tmp_pi = pty_input_level();
        if (net_input_level() < 0 && tmp_pi < 0)
            break;

        FD_ZERO(&ibits);
        FD_ZERO(&obits);
        FD_ZERO(&xbits);

        /* Never look for input if there's still stuff in the corresponding
           output buffer */
        if (net_output_level () || tmp_pi > 0)
        {
            FD_SET(net, &obits);
        }

        if (net_input_level() == 0)
        {
            FD_SET(net, &ibits);
        }

        if (!SYNCHing)
            FD_SET(net, &xbits);

        c = select(nfd, &ibits, &obits, &xbits, &timeout);
        if (c < 0)
        {
            break;
        }
        else if (c == 0)
        {            
            continue;
        }

        if (FD_ISSET(net, &xbits))
            SYNCHing = 1;

        if (FD_ISSET(net, &ibits))
        {
            /* Something to read from the network... */
            /*FIXME: handle  !defined(SO_OOBINLINE) */
            if (net_read() <= 0)
            {
                break;
            }
        }

        while (pty_input_level() > 0) /* pty in data move to net out buffer */
        {
            if (net_buffer_is_full())
                break;
            c = pty_get_char(0);
            if (c == IAC)
                net_output_byte(c);
            net_output_byte(c);
        }

        if (FD_ISSET(net, &obits) && net_output_level() > 0) /* send net out buffer data */
        {
            netflush();
        }

        if (net_input_level() > 0) /* move net in date to pty out buffer */
        {
            telrcv();
        }

        if (pty_output_level() > 0) /* notify device to read */
        {
            ptyflush();
        }
    }

    closesocket(net);

    return 0;
}

