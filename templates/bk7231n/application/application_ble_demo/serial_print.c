#include <stdarg.h>
#include <os_types.h>
#include <os_clock.h>

#if 1
#define BLE_HCI_UART_H4_CMD 0x01
#define BLE_HCI_UART_H4_ACL 0x02
#define BLE_HCI_UART_H4_EVT 0x04

char strbuf[100];
static void bin2ascii(char *bin, int len, char *str, int str_len)
{
    const char visi_val[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int str_left = str_len;
    char *p = str;
    char bin_h, bin_l;
    int i = 0;

    while ((i != len) && (str_left > 4))
    {
        bin_h = (bin[i] >> 4) & 0x0F;
        *p++ = visi_val[bin_h];
        bin_l = bin[i] & 0x0F;
        *p++ = visi_val[bin_l];

        if (0x0F == (i & 0x0F))
        {
            *p++ = '\n';
        }
        else
        {
            *p++ = ' ';
        }

        str_left -= 3;
        i++;
    }
    // *p++ = '\n';
    *p = '\0';
}

void printf_bin(char *bin, int len)
{
    char *p = bin;
    int len_left = len;
    while (0 < len_left)
    {
        bin2ascii(p, (len_left > 32) ? 32 : len_left, strbuf, sizeof(strbuf));
        os_kprintf(strbuf);
        p += 32;
        len_left -= 32;
    }
}

unsigned char print_order = 0;

#define PKT_MAX_LEN (9 + 4)

struct hci_pkt
{
    char type;
    char len;
    unsigned char order;
    int ori_len;
    unsigned int time;
    char pkt[PKT_MAX_LEN];
};

struct hci_pkt hci_pkt_buf[0x10];
int hci_pkt_num_w = 0;
int hci_pkt_num_r = 0;

void hci_pkt_write(char type, char *pkt, int len)
{
    struct hci_pkt *hci_pkt_w = &hci_pkt_buf[hci_pkt_num_w];

    os_uint32_t tick = os_tick_get();

    hci_pkt_w->type = type;
    hci_pkt_w->len = (len > PKT_MAX_LEN) ? PKT_MAX_LEN : len;
    hci_pkt_w->order = print_order++;
    hci_pkt_w->ori_len = len;
    hci_pkt_w->time = tick;
    memcpy(hci_pkt_w->pkt, pkt, hci_pkt_w->len);

    hci_pkt_num_w++;
    hci_pkt_num_w &= 0x0F;
}

void hci_pkt_read(struct hci_pkt *hci_pkt_r)
{
    struct hci_pkt *hci_pkt_w = &hci_pkt_buf[hci_pkt_num_r];

    memcpy(hci_pkt_r, hci_pkt_w, 12 + hci_pkt_w->len);
    hci_pkt_num_r++;
    hci_pkt_num_r &= 0x0F;
}

int hci_pkt_is_empty(void)
{
    return (hci_pkt_num_r == hci_pkt_num_w) ? 1 : 0;
}

void print_hci_pkt(void)
{
    struct hci_pkt hci_pkt;
    char *str;

    while (!hci_pkt_is_empty())
    {
        hci_pkt_read(&hci_pkt);

        switch (hci_pkt.type)
        {
        case BLE_HCI_UART_H4_CMD:
            if(hci_pkt.ori_len != 3 + hci_pkt.pkt[2]) {
                str = "cmd err ";
            }
            else {
            str = "cmd ";
            }
            break;
        case BLE_HCI_UART_H4_ACL:
            if(hci_pkt.ori_len != 4 + hci_pkt.pkt[2] + (hci_pkt.pkt[3] << 8)) {
                str = "acl tx err ";
            }
            else {
            str = "acl tx ";
            }
            break;
        case BLE_HCI_UART_H4_EVT:
            if(hci_pkt.ori_len != 2 + hci_pkt.pkt[1]) {
                str = "evt err ";
            }
            else {
            str = "evt ";
            }
            break;
        case (BLE_HCI_UART_H4_ACL | 0x10):
            if(hci_pkt.ori_len != 4 + hci_pkt.pkt[2] + (hci_pkt.pkt[3] << 8)) {
                str = "acl rx-err ";
            }
            else {
            str = "acl rx ";
            }
            break;
        }
        os_kprintf("%03d\t%05d\t%d\t%s", hci_pkt.order, hci_pkt.time, hci_pkt.ori_len, str);
        printf_bin(hci_pkt.pkt, hci_pkt.len);
        os_kprintf("\n");
        // os_kprintf("%02x %02x %02x\n", hci_pkt.pkt[0], hci_pkt.pkt[1], hci_pkt.pkt[2]);
    }
    return;
}

struct str_print
{
    char *str_buf;
    unsigned char order;
    unsigned char para_c;
    unsigned int para[4];
};

struct str_print str_print_buf[0x10];
int str_buf_num_w = 0;
int str_buf_num_r = 0;

void str_buf_write(char *str, unsigned char para_c, ...)
{
    va_list Paras;

    str_print_buf[str_buf_num_w].str_buf = str;
    str_print_buf[str_buf_num_w].order = print_order++;
    str_print_buf[str_buf_num_w].para_c = para_c;

    va_start(Paras, para_c);
    for (int i = 0; (i < para_c) && (i < 4); i++)
    {
        str_print_buf[str_buf_num_w].para[i] = va_arg(Paras, unsigned int);
    }
    va_end(Paras);

    str_buf_num_w++;
    str_buf_num_w &= 0x0F;
}

void str_buf_read(struct str_print *str_print)
{
    str_print->str_buf = str_print_buf[str_buf_num_r].str_buf;
    str_print->order = str_print_buf[str_buf_num_r].order;
    str_print->para_c = str_print_buf[str_buf_num_r].para_c;

    for (int i = 0; i < str_print->para_c; i++)
    {
        str_print->para[i] = str_print_buf[str_buf_num_r].para[i];
    }

    str_buf_num_r++;
    str_buf_num_r &= 0x0F;
    return;
}

int str_buf_is_empty(void)
{
    return (str_buf_num_r == str_buf_num_w) ? 1 : 0;
}

void print_str_buf(void)
{
    unsigned char order;
    struct str_print str_print;

    while (!str_buf_is_empty())
    {
        str_buf_read(&str_print);
        os_kprintf("\n%03d\t", str_print.order);
        os_kprintf(str_print.str_buf, str_print.para[0],
                   str_print.para[1], str_print.para[2], str_print.para[3]);
    }
    return;
}
#endif