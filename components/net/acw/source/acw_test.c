#include <mo_wifi.h>
#include <mo_api.h>
#include <sys/socket.h>
#include <dlog.h>

//typedef void (*mo_netconn_data_callback)(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size);
static void esp8266_data_recv_cb(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size)
{
    LOG_I("MAIN", "recv_len=%d, addr=0x%08x, port=%d", size, addr.addr, htons(port));
    LOG_I("MAIN", "recv_data:%s", data);

    return;
}

static void esp8266_start_func(int argc, char **argv)
{
    mo_object_t *module;
    os_err_t do_err;
    struct sockaddr_in addr;
	static int socket_fd = -1;

    if (socket_fd >= 0)
    {
        LOG_I("MAIN", "fd exist");
        return;
    }

    module =  mo_get_default();
    if (OS_NULL == module)
    {
        LOG_I("MAIN", "no default mo");
        return;
    }
    
    do_err = mo_wifi_connect_ap(module, "hw_hsj", "Aa123456");
    LOG_I("MAIN", "mo_wifi_connect_ap = %d", do_err);
    do_err = mo_wifi_start_ap(module, "es8266_hsj", "Aa123456", 6, 3);
    LOG_I("MAIN", "mo_wifi_start_ap = %d", do_err);

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        LOG_I("MAIN", "Socket error, errno=%d", socket_fd);
        return;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    addr.sin_port = htons(9999);

    do_err = bind_with_cb(socket_fd, (struct sockaddr *)&addr, sizeof(addr), esp8266_data_recv_cb);
    if (do_err < 0)
    {
        LOG_I("MAIN", "connect skfd[%d] error, errno=%d", socket_fd, do_err);
        closesocket(socket_fd);
        return;  
    }

  #if 0  
    do_err = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (do_err < 0)
    {
        LOG_I("MAIN", "connect skfd[%d] error, errno=%d", socket_fd, do_err);
        closesocket(socket_fd);
        return;  
    }

    struct sockaddr_in from;
    socklen_t fromlen;
    fromlen = sizeof(from);
    int recv_len;
    char buf[128];

    do
    {
        memset(buf, 0, sizeof(buf));
        recv_len = recvfrom(socket_fd, (void *)buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
        if ( recv_len > 0)
        {
            LOG_I("MAIN", "recv_len=%d, addr=0x%08x, port=%d", recv_len, from.sin_addr.s_addr, htons(from.sin_port));
            LOG_I("MAIN", "recv_data:%s", buf);
            if (buf[0] == 'q')
            {
                break;
            }
        }

        os_task_msleep(os_tick_from_ms(1000));
    } while(1);
#endif
    return;
}

#include <st7789vw.h>

void lcd_show_on_off_stat(char *dev_id, os_bool_t on_off)
{
    char show_item[32];

    os_snprintf(show_item, sizeof(show_item), "%s:%s", dev_id, (on_off == OS_TRUE) ? "on" : "off");
    lcd_show_string(0, 16, 32, show_item);

    return;
}

void lcd_show_temportature(os_uint8_t temp)
{
    char show_item[32];

    os_snprintf(show_item, sizeof(show_item), "temportature:%u", temp);
    lcd_show_string(0, 64, 16, show_item);

    return;
}

void lcd_show_work_mode(char *work_mode)
{
    char show_item[32];

    os_snprintf(show_item, sizeof(show_item), "mode:%s", work_mode);
    lcd_show_string(0, 96, 16, show_item);

    return;
}

void lcd_show_speed(char *speed)
{
    char show_item[32];

    os_snprintf(show_item, sizeof(show_item), "speed:%s", speed);
    lcd_show_string(0, 128, 16, show_item);

    return;
}

static void lcd_start_func(int argc, char **argv)
{
    lcd_clear(WHITE);

    /* set the background color and foreground color */
    lcd_set_color(WHITE, BLACK);
    lcd_show_string(0, 16, 32, "Hold on++++++++++");
    lcd_show_string(0, 96, 16, "Mode:Mix");
    lcd_show_string(0, 144, 16, "state:wait");
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(esp8266_start, esp8266_start_func, "esp8266_start");
SH_CMD_EXPORT(lcd_start, lcd_start_func, "lcd_start");
#endif  /* end of using OS_USING_SHELL */