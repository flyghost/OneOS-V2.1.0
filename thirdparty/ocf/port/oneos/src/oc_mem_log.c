#include <board.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <shell.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_clock.h>

//extern unsigned char *log_buff;

#define OCF_LOG_BUFF_SIZE    (128 * 1024)
#ifdef USE_SDRAM_HEAP
static char *log_buff = NULL;
#else
static char log_buff[OCF_LOG_BUFF_SIZE];
#endif

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

SH_CMD_EXPORT(log, ocf_print_log, "Print ocf log to console.");

static int ocf_clear_log(int argc, char *argv[])
{
    memset(log_buff, 0, OCF_LOG_BUFF_SIZE);
    log_pos = 0;
    return 0;
}

SH_CMD_EXPORT(log_clear, ocf_clear_log, "Clear ocf log.");
