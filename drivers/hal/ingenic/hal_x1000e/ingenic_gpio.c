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

static struct jzgpio_chip jz_gpio_chips[GPIO_NR_PORTS];

void jz_gpio_set(struct jzgpio_chip *chip,
        unsigned offset, int value)
{
    if (value)
        writel(BIT(offset), chip->reg + PXPAT0S);
    else
        writel(BIT(offset), chip->reg + PXPAT0C);
}

int jz_gpio_get(struct jzgpio_chip *chip, unsigned offset)
{

    return !!(readl(chip->reg + PXPIN) & BIT(offset));
}


static void __gpio_set_func(struct jzgpio_chip *chip,
        enum gpio_function func, unsigned int pins)
{
    unsigned long flags;
    unsigned long grp;

    if ((func == GPIO_PULL_UP)||(func == GPIO_PULL_DOWN)||(func == GPIO_PULL_HIZ))
    {
        if ((func & 0xf0) == GPIO_PULL_UP)
        { /*pull up*/
            writel(pins, chip->reg + PXPDENC);
            writel(pins, chip->reg + PXPUENS);
        }
        else if ((func & 0xf0) == GPIO_PULL_DOWN)
        { /*pull down*/
            writel(pins, chip->reg + PXPUENC);
            writel(pins, chip->reg + PXPDENS);
        }
        else if ((func & 0xf0) == GPIO_PULL_HIZ)
        { /*UP and DOWN Disable*/
            writel(pins, chip->reg + PXPDENC);
            writel(pins, chip->reg + PXPUENC);
        }
    }
    else
    {
        if((func == GPIO_INT_LO)||(func == GPIO_INT_HI)||(func == GPIO_INT_FE)||(func == GPIO_INT_RE))
        {
            writel(pins, chip->shadow_reg + PZINTS);
            writel(pins, chip->shadow_reg + PZMSKC);
            switch (func)
            {
                case GPIO_INT_LO : //Low Level trigger interrupt
                    writel(pins, chip->shadow_reg + PZPAT1C);
                    writel(pins, chip->shadow_reg + PZPAT0C);
                    break;
                case GPIO_INT_HI : //High Level trigger interrupt
                    writel(pins, chip->shadow_reg + PZPAT1C);
                    writel(pins, chip->shadow_reg + PZPAT0S);
                    break;
                case GPIO_INT_FE : //Fall Edge trigger interrupt
                    writel(pins, chip->shadow_reg + PZPAT1S);
                    writel(pins, chip->shadow_reg + PZPAT0C);
                    break;
                case GPIO_INT_RE : //Rise Edge trigger interrupt
                    writel(pins, chip->shadow_reg + PZPAT1S);
                    writel(pins, chip->shadow_reg + PZPAT0S);
                    break;
                default : break;
            }
        }
        else
        {
            writel(pins, chip->shadow_reg + PZINTC);
            if((func == GPIO_OUTPUT0)||(func == GPIO_OUTPUT1)||(func == GPIO_INPUT))
            {
                writel(pins, chip->shadow_reg + PZMSKS);
                switch (func)
                {
                    case GPIO_OUTPUT0 :
                        writel(pins, chip->shadow_reg + PZPAT1C);
                        writel(pins, chip->shadow_reg + PZPAT0C);
                        break;
                    case GPIO_OUTPUT1 :
                        writel(pins, chip->shadow_reg + PZPAT1C);
                        writel(pins, chip->shadow_reg + PZPAT0S);
                        break;
                    case GPIO_INPUT :
                        writel(pins, chip->shadow_reg + PZPAT1S);
                        break;
                    default : break;
                }
            }
            else if((func == GPIO_FUNC_0)||(func == GPIO_FUNC_1)||(func == GPIO_FUNC_2)||(func == GPIO_FUNC_3))
            {
                writel(pins, chip->shadow_reg + PZMSKC);
                switch (func)
                {
                    case GPIO_FUNC_0 :
                        writel(pins, chip->shadow_reg + PZPAT1C);
                        writel(pins, chip->shadow_reg + PZPAT0C);
                        break;
                    case GPIO_FUNC_1 :
                        writel(pins, chip->shadow_reg + PZPAT1C);
                        writel(pins, chip->shadow_reg + PZPAT0S);
                        break;
                    case GPIO_FUNC_2 :
                        writel(pins, chip->shadow_reg + PZPAT1S);
                        writel(pins, chip->shadow_reg + PZPAT0C);
                        break;
                    case GPIO_FUNC_3 :
                        writel(pins, chip->shadow_reg + PZPAT1S);
                        writel(pins, chip->shadow_reg + PZPAT0S);
                        break;
                    default : break;
                }
            }
        }
        /* configure PzGID2LD to specify which port group to load */
        grp = ((unsigned int)chip->reg - (unsigned int)jz_gpio_chips[0].reg) >> 8;
        writel(grp & 0x3, chip->shadow_reg + PZGID2LD);
    }
}


int jzgpio_set_func(enum gpio_port port,
        enum gpio_function func,unsigned long pins)
{
    struct jzgpio_chip *jz = &jz_gpio_chips[port];

    __gpio_set_func(jz,func,pins);

    return 0;
}

static int jz_gpio_input(enum gpio_port port, unsigned offset)
{
    struct jzgpio_chip *jz = &jz_gpio_chips[port];

    __gpio_set_func(jz, GPIO_INPUT, BIT(offset));
    return 0;
}

static int jz_gpio_output(enum gpio_port port,
        unsigned offset, int value)
{
    struct jzgpio_chip *jz = &jz_gpio_chips[port];

    __gpio_set_func(jz, value? GPIO_OUTPUT1: GPIO_OUTPUT0
            , (offset));
    return 0;
}

int setup_gpio_pins(void)
{
    int i;
    unsigned int base;

    base = GPIO_BASE;
    for (i = 0; i < GPIO_NR_PORTS; i++) {
        jz_gpio_chips[i].reg = base + i * GPIO_PORT_OFF;
        jz_gpio_chips[i].shadow_reg = base + GPIO_SHADOW_OFF;
    }

    return 0;
}

void gpio_port_set_value(int port, int pin, int value)
{
    struct jzgpio_chip *jz = &jz_gpio_chips[port];

    jz_gpio_set(jz,pin,value);
}

int gpio_port_get_value(int port, int pin)
{
    struct jzgpio_chip *jz = &jz_gpio_chips[port];

    return jz_gpio_get(jz,pin);
}

void gpio_port_direction_output(int port, int pin, int value)
{
    jz_gpio_output(port,(1<<pin),value);
}

void gpio_port_direction_input(int port, int pin)
{
    jz_gpio_input(port,(1<<pin));
}

void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins)
{
    jzgpio_set_func(n,func,pins);
}




