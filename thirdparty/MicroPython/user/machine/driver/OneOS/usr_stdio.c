#include "stdio.h"
#include "py/runtime.h"

#include "arch_interrupt.h"
#include "lib/utils/interrupt_char.h"

#include "ring_buff.h"
#include "string.h"
#include "usr_misc.h"
#include "usr_stdio.h"
#include <device.h>
#include <os_assert.h>
#include <console.h>

#ifndef OS_USING_SYS_HEAP
#error "SYS HEAP needs open if use stdio of microPython"
#endif

#ifdef MICROPYTHON_USING_REPL 
#if (SHELL_TASK_STACK_SIZE < 4096)
#error "SHELL_TASK_STACK_SIZE need more than 4096 bytes if using the microPython repl"
#endif
#endif

static os_device_t *console = OS_NULL;

/**
 *********************************************************************************************************
 *                                      initialize shell channel
 *
 * @description: This function initialize shell channel for micropython.
 *
 * @param 	   : void
 *
 * @return     : the flag of console
 *
 * @note       : The function use micropython shell channel to receive data from commond line replace OneOS
 *				 system shell channel.
 *********************************************************************************************************
*/
void usr_getchar_init(void) 
{
    console = os_console_get_device();
	OS_ASSERT(console != OS_NULL);
}

/**
 *********************************************************************************************************
 *                                      close shell channel
 *
 * @description: This function close micropython shell channel, recover the shell channel of OneOS system.
 *
 * @param 	   : flag	The old flag of console, that will be restore
 *
 * @return     : void
 *********************************************************************************************************
*/
void usr_getchar_deinit(void) 
{
	OS_ASSERT(console != OS_NULL);
    console = NULL;    
}

int mp_hal_stdin_rx_chr(void) 
{

    unsigned char ch;

    OS_ASSERT(console != OS_NULL);

    if (os_device_read_block(console, 0, &ch, 1) == 1)
    {
        return ch;
    }
    mp_err("Read console data failed.");
    MICROPY_EVENT_POLL_HOOK;
    return 0xFF;
}

void mp_hal_stdout_tx_strn(const char *str, size_t len)
{
	os_device_t *console  = os_console_get_device();
    if (console) {
        os_device_write_nonblock(console, 0, str, len);
    }
}

void mp_hal_stdout_tx_strn_stream(const char *str, size_t len) {
	usr_kprintf("%.*s", len, str);
}



