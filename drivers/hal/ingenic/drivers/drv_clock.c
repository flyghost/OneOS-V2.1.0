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
 * @file        drv_clock.c
 *
 * @brief       This file provides drivers for clock.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_clock.h>
#include <os_memory.h>
#include <string.h>
#include <os_util.h>

struct clk *clk_get(const char *id)
{
    int i;
    struct clk *retval = NULL;
    struct clk *clk_src = get_clk_from_id(0);
    struct clk *parent_clk = NULL;

    for (i = 0; i < get_clk_sources_size(); i++)
    {
        if (id && clk_src[i].name && !strcmp(id, clk_src[i].name))
        {
            if (clk_src[i].flags & CLK_FLG_NOALLOC)
                return &clk_src[i];
            retval = os_malloc(sizeof(struct clk));
            if (!retval)
                return (NULL);

            memcpy(retval, &clk_src[i], sizeof(struct clk));
            retval->flags = 0;
            retval->source = &clk_src[i];
            if (CLK_FLG_RELATIVE & clk_src[i].flags)
            {
                parent_clk = get_clk_from_id(CLK_RELATIVE(clk_src[i].flags));
                parent_clk->child = NULL;
            }
            retval->count = 0;
            return retval;
        }
    }
    return NULL;
}

int clk_enable(struct clk *clk)
{
    int count;
    if (!clk)
        return -1;

    if(clk->source)
    {
        count = ++clk->count;
        if (count != 1)
            return 0;

        clk->flags |= CLK_FLG_ENABLE;
        clk = clk->source;
        if (clk->init_state)
        {
            clk->count = 1;
            clk->init_state = 0;
            return 0;
        }
    }

    count = ++clk->count;
    if(count == 1)
    {
        if(clk->parent)
        {
            clk_enable(clk->parent);
        }

        if(clk->ops && clk->ops->enable)
        {
            clk->ops->enable(clk,1);
        }
        clk->flags |= CLK_FLG_ENABLE;
    }
    return 0;
}

int clk_is_enabled(struct clk *clk)
{
    return !!(clk->flags & CLK_FLG_ENABLE);
}

void clk_disable(struct clk *clk)
{
    int count;
    if (!clk)
        return;
    if (clk->source)
    {

        count = --clk->count;
        if (count != 0)
        {
            if (count < 0)
            {
                clk->count = 0;
                return;
            }
        }

        clk->flags &= ~CLK_FLG_ENABLE;
        clk = clk->source;
    }

    count = --clk->count;
    if (count < 0)
    {
        clk->count++;
        return;
    }

    if(count == 0)
    {
        if(clk->ops && clk->ops->enable)
            clk->ops->enable(clk,0);
        clk->flags &= ~CLK_FLG_ENABLE;
        if(clk->parent)
            clk_disable(clk->parent);
    }
}

uint32_t clk_get_rate(struct clk *clk)
{
    if (!clk)
        return 0;
    if (clk->source)
        clk = clk->source;
    return clk ? clk->rate : 0;
}

void clk_put(struct clk *clk)
{
    struct clk *parent_clk;
    if (clk && !(clk->flags & CLK_FLG_NOALLOC))
    {
        if (clk->source && clk->count && clk->source->count > 0)
        {
            if (--(clk->source->count) == 0)
                clk->source->init_state = 1;
        }
        if (CLK_FLG_RELATIVE & clk->source->flags)
        {
            parent_clk = get_clk_from_id(CLK_RELATIVE(clk->source->flags));
            parent_clk->child = clk->source;
        }
        os_free(clk);
    }
}

int clk_set_rate(struct clk *clk, uint32_t rate)
{
    int ret = 0;
    if (!clk)
        return -1;
    if (clk->source)
        clk = clk->source;
    if (!clk->ops || !clk->ops->set_rate)
        return -1;
    if (clk->rate != rate)
        ret = clk->ops->set_rate(clk, rate);
    return ret;
}

int mips_clock_init(void)
{

    init_all_clk();

    return 0;
}
