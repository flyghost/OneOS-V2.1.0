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
 * @file        ingenic_gpio.c
 *
 * @brief       This file provides gpio functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <ingenic_gpio.h>

void gpio_port_set_value(int port, int pin, int value)
{
    if (value)
        writel(1 << pin, GPIO_PXPAT0S(port));
    else
        writel(1 << pin, GPIO_PXPAT0C(port));
}


void gpio_port_direction_output(int port, int pin, int value)
{
    writel(1 << pin, GPIO_PXINTC(port));
    writel(1 << pin, GPIO_PXMSKS(port));
    writel(1 << pin, GPIO_PXPAT1C(port));

    gpio_port_set_value(port, pin, value);
}

void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins)
{
    unsigned int base = GPIO_BASE + JZGPIO_GROUP_OFFSET * n;

    writel(func & 0x8? pins : 0, base + PXINTS);
    writel(func & 0x4? pins : 0, base + PXMSKS);
    writel(func & 0x2? pins : 0, base + PXPAT1S);
    writel(func & 0x1? pins : 0, base + PXPAT0S);
    writel(func & 0x8? 0 : pins, base + PXINTC);
    writel(func & 0x4? 0 : pins, base + PXMSKC);
    writel(func & 0x2? 0 : pins, base + PXPAT1C);
    writel(func & 0x1? 0 : pins, base + PXPAT0C);
    writel(func & 0x10? 0 : pins, base + PXPUENC);
    writel(func & 0x10? pins : 0, base + PXPUENS);
    writel(func & 0x20? 0 : pins, base + PXPDENC);
    writel(func & 0x20? pins : 0, base + PXPDENS);

}

