#include <os_task.h>
#include <device.h>
#include <unistd.h>
#include <stdio.h>
#include <shell.h>

#include <spi.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <os_clock.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#include <dlog.h>
#define DBG_TAG "spi_speed_test"

typedef struct spi_test_state
{
    char spi_name[4];
    int  state;
} spi_test_t;
                                   
int  spi_time_test_flag = 0;
char *spi_bus_name;
char *spi_device_name;
int  spi_cs_pin = 0;
os_uint32_t max_clock = 0;

os_uint64_t start_transfer_time = 0, stop_transfer_time = 0;  

void spi_test_task(void *parameter)
{   
    os_err_t           result;
    int i;
    struct os_spi_message message;
    struct os_spi_device *os_spi_device;
    os_uint8_t buffer[1024] = {0};  /* 1k */
    os_uint32_t ideal_time = 0,deviation = 0;
    os_uint32_t time_cost = 0,average_time = 0;
    float temp = 0;
    
    os_spi_device = (struct os_spi_device *)os_device_find(spi_device_name);
    if (os_spi_device == NULL)
    {
        os_kprintf("can not find the spi device!\n\r");
        return;
    }
       
    /* config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 and 3 */
        cfg.max_hz     = max_clock;                   /* Atmel RapidS Serial Interface: 66MHz Maximum Clock Frequency */

        os_spi_configure(os_spi_device, &cfg);
        
        if (os_spi_device->bus->owner != os_spi_device)
        {
            /* Not the same owner as current, re-configure SPI bus */
            result = os_spi_device->bus->ops->configure(os_spi_device, &os_spi_device->config);
            if (result == OS_EOK)
            {
                /* Set SPI bus owner */
                os_spi_device->bus->owner = os_spi_device;
            }
        }
        
    }
    
    /* Send data */
    {
        /* Initial message */
        message.send_buf = buffer;
        message.recv_buf = OS_NULL;
        message.length   = 1024;
        message.cs_take = message.cs_release = 0;

        /* Transfer message ,length = 10k*/
        for(i = 0; i < 10; i++)
        {
            start_transfer_time = os_clocksource_gettime();
            
            os_spi_device->bus->ops->xfer(os_spi_device, &message);
            
            stop_transfer_time = os_clocksource_gettime();
            
            time_cost = (stop_transfer_time - start_transfer_time) / 1000;
            average_time += time_cost;
        }
    }
    
    average_time /= 10;    
    max_clock /=1000;
    temp = 1024*8/(float)max_clock; //ms
    ideal_time = temp*1000;  //us
    
    LOG_I(DBG_TAG,"transfer 1k data actual time : %d us \r\n",average_time);
    LOG_I(DBG_TAG,"transfer 1k data ideal time: %d us \r\n",ideal_time);
    
    deviation = (average_time - ideal_time);
    LOG_I(DBG_TAG,"deviation_time: %d us \r\n",deviation);
    
}

spi_test_t spi_test_table[] = 
{
    {"spi1",0},
    {"spi2",0},
    {"spi3",0},
    {"spi4",0},
    {"spi5",0},
    {"spi6",0},
};

void spi_transfer_speed_test(int argc, char **argv)
{
    os_task_t *task;
    os_err_t           result;
    struct os_device *spi_bus;
    int i;
    
    if (argc == 5)
    {
        spi_bus_name = argv[1];
        spi_device_name = argv[2];
        spi_cs_pin = strtol(argv[3], OS_NULL, 0);
        max_clock = strtol(argv[4], OS_NULL, 0)*1000;
                        
        spi_bus = os_device_find(spi_bus_name);
        if (spi_bus == NULL)
        {
            os_kprintf("can not find the spi bus!\n\r");
            return;
        }
        
        for(i = 0; i < 6; i++)
        {
            if(!strcmp(spi_bus_name,spi_test_table[i].spi_name))
            break;
        }
        
        if(spi_test_table[i].state == 0)
        {
            result = os_hw_spi_device_attach(spi_bus_name, spi_device_name, spi_cs_pin);
            if (result != OS_EOK)
            {
                os_kprintf("can not attach the spi device!\n\r");
                return;
            }
            spi_test_table[i].state = 1;
        }
            
    }
    else
    {
        os_kprintf("Please input: \n\r <spi_bus_name> <spi_device_name> <cs_pin> <clock(k)> \n\r");
        return ;
    }

    task = os_task_create("spi test", spi_test_task, NULL, 2048, 5);
    OS_ASSERT(task);
    os_task_startup(task);
    return;
}

SH_CMD_EXPORT(spi_transfer_speed_test, spi_transfer_speed_test, "test the spi transfer speed");
