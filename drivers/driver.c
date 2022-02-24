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
 * @file        drivers.c
 *
 * @brief       this file implements timer related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <driver.h>
#include <os_clock.h>

void *interrupt_stack_addr = OS_NULL;

os_uint32_t uart_calc_byte_timeout_us(os_uint32_t baud)
{
    /* start + data + stop = 10 bits */

    return 10 * 1000000 / baud;
}

/**
 * @brief 对于clock而言，计算这两个值的本质目的，是避免频繁通过cycles/frequency来计算单调时间（monoronic time），因为这样做回导致浮点运算，开销会很大，当然也能保证时间的精度
 * 
 * 例如：例如from=1M=10^6 hz(us), to = 1G = 10^9 hz(ns), 那么时钟频率是1e6(counter freg), to=1e9就是最终要被转化成的时间粒度，mult/(1<<shift)就是from与to之间的时间粒度比
 * (1<<shift)/mult = 10^6/10^9 = 1/1000
 * mult/(1<<shift) = 1000 = mult >> shift
 * 
 * 如果直接将粒度比算出来存起来，下次直接时钟源的cycles*粒度比得到to粒度的时间值，精度没有 cycles*mult/(1<<shift) 得到的时间准确度肯定要高
 * 
 * from--->to :  to_cycle = from_cycle * mult >> shift 
 * 
 * @param mult 
 * @param shift 
 * @param from frequency to convert from   时钟的真正时钟频率
 * @param to frequency to convert to   最终转化时间的粒度
 * @param max_from guaranteed runtime conversion range in seconds   保证以秒为单位的运行时转换范围
 * 如果max_from越大，说明mask能表示的描述越大，这就意味着mask left shift的位数变少，从而导致mult和shift都会表少，导致时间精度下降
 */
void calc_mult_shift(os_uint32_t *mult, os_uint32_t *shift, os_uint32_t from, os_uint32_t to, os_uint32_t max_from)
{
    os_uint64_t tmp;
    os_uint32_t sft, sftacc = 32;   // shift accumulator，左移累加器

    // 计算 clocksource.mask能被左移的最大位数，从而也决定了mult的大小，即cycle*mult <= mask，这样不会发生溢出
    // 这里可以看出最大的left shift most = 32
    tmp = ((os_uint64_t)max_from * from) >> 32;
    while (tmp)
    {
        tmp >>= 1;
        sftacc--;
    }

    // find the conversion shift/mult pair which has the best 查找最好的转化 shift/mult 对
    for (sft = 32; sft > 0; sft--)
    {
        tmp = (os_uint64_t)to << sft;       // 将目的时间粒度左移sft，相当于粒度提高了2^sft倍，这里假设提高后的时间粒度定义为sft_resolution
        tmp += from / 2;                    // 类似div_roundup()功能，向上取整：4.1取整为5
        tmp /= from;                        // (定义为sft_period),这里得到的period的单位是sft_resolution
        
        // 这里(tmp = sft_period) >> sftacc, 右移sftacc位(clocksources.cycles能左移的最大值)，
        // 找出tmp右移sftacc位后等于零的tmp，说明以后cycles*tmp(最多左移sftacc位)，这样一定不会溢出(cycles * mult <= mask)。
        if ((tmp >> sftacc) == 0)
            break;
    }
    
    /**
     * 因此(1<<shift)/mult是 from与to之间的粒度比
     * 例如from = 10^6 hz(us), to = 10^9 hz(ns), (1<<shift)/mult = 10^6/10^9 = 1/1000, from的粒度是to的千分之一，因此每个cycle占用1us=1000ns
     * 可能有人会问，直接存储这比例就好了，cycles*1000就得到时间ns了，为什么还要cycles*mult/(1<<shift)呢，精度啊是不是。
     */
    *mult = tmp;
    *shift = sft;
}

static const unsigned short crc_ta[16] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
};

unsigned short crc16(unsigned short init_crc, void *buff, int len)
{
    unsigned short crc = init_crc;
    unsigned char *ptr = buff;
    unsigned char da;

    while (len-- > 0)
    {
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da^(*ptr>>4)];
        
        da = crc >> 12;
        crc <<= 4;
        crc ^= crc_ta[da^(*ptr&0x0f)];

        ptr++;
    }

    return crc;
}

void hex_dump(unsigned char *buff, int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (i % 16 == 0)
            os_kprintf("%08x: ", i);
    
        os_kprintf("%02x ", buff[i]);

        if (i % 8 == 7)
            os_kprintf(" ");

        if (i % 16 == 15)
            os_kprintf("\r\n");
    }

    os_kprintf("\r\n");
}

OS_WEAK void os_hw_cpu_reset(void){}

#ifdef OS_USING_SHELL
#include <shell.h>
static void reboot(int argc, char *argv[])
{
    os_hw_cpu_reset();
}
SH_CMD_EXPORT(reboot, reboot, "reboot");
#endif

