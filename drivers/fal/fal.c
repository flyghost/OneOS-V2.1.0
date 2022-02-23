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
 * @file        fal.c
 *
 * @brief       fal
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <fal.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_assert.h>
#include <string.h>

static os_list_node_t fal_device_list = OS_LIST_INIT(fal_device_list);

int fal_flash_register(fal_flash_t *flash)
{
    os_base_t level;
    
    level = os_irq_lock();
    os_list_add_tail(&fal_device_list, &flash->list);
    os_irq_unlock(level);
    
    return 0;
}

int fal_flash_unregister(fal_flash_t *flash)
{
    os_kprintf("fal_flash_unregister unsupport \r\n");
    return -1;
}

fal_flash_t *fal_flash_find(const char *name)
{
    fal_flash_t *flash;
    
    os_base_t level;
    
    level = os_irq_lock();
    
    os_list_for_each_entry(flash, &fal_device_list, fal_flash_t, list)
    {
        if (!strcmp(name, flash->name))
        {
            os_irq_unlock(level);
            return (fal_flash_t *)flash;
        }
    }
    
    os_irq_unlock(level);

    return OS_NULL;
}
