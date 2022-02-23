#include <arch_interrupt.h>
#include <device.h>
#include <shell.h>
#include <os_errno.h>
#include <board.h>
#include <drv_log.h>
#include <drv_gpio.h>

#define POWER_4G_PIN    GET_PIN(1, 27)
#define PWR_KEY         GET_PIN(0, 7)

int module_4g_test(os_int32_t argc, char **argv)
{	
    int  str_size = 0;
    int  cmd = 0;
    int  count = 0;
    char str1[] = "AT\r\n";
    char str2[] = "AT+CSQ\r\n";
    char str3[] = "AT+PSRAT\r\n";
    char str4[] = "AT+CCID\r\n";
    char *cmd_str;
    char buff[64];
    int length = 0;
    int i=0;
  
    if (argc != 3) 
    {
        os_kprintf("parameter error!\r\n");
        os_kprintf("USE: module_4g_test + UARTX + COMMAND \r\n");
        os_kprintf("SAMPLE: module_4g_test uart4 1 \r\n");
        return OS_ERROR;
    }
    
    /***pin initialize&power on***/
    os_pin_mode(POWER_4G_PIN, PIN_MODE_OUTPUT);
    os_pin_mode(PWR_KEY, PIN_MODE_OUTPUT);
    os_task_msleep(500);
    os_pin_write(POWER_4G_PIN, PIN_HIGH);
    os_pin_write(PWR_KEY, PIN_LOW);  
    os_task_msleep(500);
        
    os_kprintf("4G Module power on!\r\n");
    os_task_msleep(500);
    /***end***/

    os_kprintf("Open the 4G Module!\r\n");
    os_pin_write(PWR_KEY, PIN_HIGH);
    os_task_msleep(3000);
    os_pin_write(PWR_KEY, PIN_LOW);
    
    cmd = atoi(argv[2]);
     
    os_task_msleep(1000);
        
    /* use uart4 to debug the 4g module */
    os_device_t *uart_dev = os_device_find(argv[1]);
    if(!uart_dev)
    {
        os_kprintf("uart_dev find failed!\r\n");
    }
    
    os_device_open(uart_dev);
    
    if(cmd == 1)
    {
        cmd_str = str1;
        str_size = sizeof(str1);
    }
    if(cmd == 2)
    {
        cmd_str = str2;
        str_size = sizeof(str2);
    }
    if(cmd == 3)
    {
        cmd_str = str3;
        str_size = sizeof(str3);
    }
    if(cmd == 4)
    {
        cmd_str = str4;
        str_size = sizeof(str4);
    }        
    while(count < 10)
    {		
        os_device_write_nonblock(uart_dev,0,cmd_str,str_size);
        os_task_msleep(1500);
        
        length = os_device_read_nonblock(uart_dev,0,buff,64);
        for(i=0;i<length;i++)
        {
        os_kprintf("4G STATUS: %c\r\n",buff[i]);
        }

        count++;
    }
    
    os_kprintf("Close the 4G Module!\r\n");
    os_pin_write(PWR_KEY, PIN_HIGH);
    os_task_msleep(5000);
    os_pin_write(PWR_KEY, PIN_LOW);
    
    return OS_EOK;
		
}
SH_CMD_EXPORT(module_4g_test, module_4g_test, "4g module test");
