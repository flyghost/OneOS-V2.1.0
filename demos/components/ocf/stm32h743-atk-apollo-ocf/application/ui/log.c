#include <board.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <shell.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_clock.h>

extern unsigned char *log_buff;

#define OCF_LOG_BUFF_SIZE    (128 * 1024)
// static char log_buff[OCF_LOG_BUFF_SIZE];
static int log_pos = 0u;
void print_to_mem(const char *fmt, ...)
{
    if(NULL == log_buff)
    {
        return;
    }
    va_list args;
    os_size_t length;
    uint32_t tmp_len = 0u;

    va_start(args, fmt);
    length = os_vsnprintf(log_buff + log_pos, OCF_LOG_BUFF_SIZE - log_pos, fmt, args);
    if (length > OCF_LOG_BUFF_SIZE - log_pos - 1)
    {
        length = OCF_LOG_BUFF_SIZE - log_pos - 1;
        log_buff[length + log_pos] = '\0';
    }

    va_end(args);

    if (length == OCF_LOG_BUFF_SIZE - log_pos - 1) {
        log_pos = 0;
    } else {
        tmp_len = log_pos;
        log_pos = ((tmp_len + length) % OCF_LOG_BUFF_SIZE);

    }

    return;
}

static int ocf_print_log(int argc, char *argv[])
{
    if(NULL == log_buff)
    {
        return -1;
    }
    int i = 0;
    for (i = log_pos; i < OCF_LOG_BUFF_SIZE; i++)
    {
        if (log_buff[i] != '\0')
        {
            printf("%c", log_buff[i]);
        }
    }
    for (i = 0; i < log_pos; i++)
    {
        if (log_buff[i] != '\0')
        {
            printf("%c", log_buff[i]);
        }
    }
    printf("\n");

    return 0;
}

SH_CMD_EXPORT(log, ocf_print_log, "test udp command.");

static int ocf_clear_log(int argc, char *argv[])
{
    memset(log_buff, 0, OCF_LOG_BUFF_SIZE);
    log_pos = 0;
    return 0;
}

SH_CMD_EXPORT(log_clear, ocf_clear_log, "test udp command.");

#include <fal_part.h>
static int ocf_check_flash(int argc, char *argv[])
{
    int j = 0;
    fal_part_t *part = fal_part_find("ocf_data");
    char name[128];
    //Count all ocf data from flash firstly
    for(int i = 0;i < (256 * 1024)/4096;i ++)
    {
        memset(name, 0, sizeof(name));
        fal_part_read(part, i * 4096, (uint8_t*)name, sizeof(name));
        if(NULL != strstr(name, "ocf_data"))
        {
            os_kprintf("OCF find item %s.\r\n", name);
            j ++;
        }
    }

    os_kprintf("OCF find %d ocf items in flash.\r\n", j);
    return 0;
}
SH_CMD_EXPORT(ocf_check_flash, ocf_check_flash, "ocf_check_flash");

#if 0
#include "ui.h"
static int ocf_test_temperature(int argc, char *argv[])
{
    if(argc != 2)
    {
        os_kprintf("usage: test_temperature <temperature> \r\n");
        os_kprintf("       test_temperature 32 \r\n");
        return -1;
    }
    ui_set_temperature(atoi(argv[1]));
}

SH_CMD_EXPORT(test_temperature, ocf_test_temperature, "test_temperature.")

static int ocf_test_binaryswitch(int argc, char *argv[])
{
    if(argc != 2)
    {
        os_kprintf("usage: test_binaryswitch <switch> \r\n");
        os_kprintf("       test_binaryswitch 0 \r\n");
        return -1;
    }
    ui_set_binary_switch(atoi(argv[1]));
}

SH_CMD_EXPORT(test_binaryswitch, ocf_test_binaryswitch, "test_binaryswitch.")
#endif
