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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "sdram_port.h"
#include <drv_gpio.h>
#include <os_memory.h>

const led_t led_table[] = {
    {GET_PIN(B, 1), PIN_LOW},
    {GET_PIN(B, 0), PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);

const struct push_button key_table[] = {
    {GET_PIN(H, 3), PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
    {GET_PIN(H, 2), PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
    {GET_PIN(C, 13), PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
    {GET_PIN(A, 0), PIN_MODE_INPUT_PULLDOWN, PIN_IRQ_MODE_RISING},
};

const int key_table_size = ARRAY_SIZE(key_table);

#if USE_SDRAM_HEAP

static os_memheap_t sdram_heap;
static int sdram_heap_inited = 0;

int init_sdram_heap(void)
{
  if (sdram_heap_inited == 0) {
    os_memheap_init(&sdram_heap, "sdram_heap");
    os_memheap_add(&sdram_heap,
      (void *)SDRAM_BASE,
      SDRAM_SIZE,
      OS_MEM_ALG_DEFAULT);
    sdram_heap_inited = 1;
  }
  return OS_EOK;
}
OS_CMPOENT_INIT(init_sdram_heap, OS_INIT_SUBLEVEL_HIGH);

void * sdram_alloc(size_t size)
{
  return os_memheap_alloc(&sdram_heap, size);
}

void * sdram_aligned_alloc(size_t align, size_t size)
{
  if (sdram_heap_inited == 0)
    init_sdram_heap();
  return os_memheap_aligned_alloc(&sdram_heap, align, size);
}

void sdram_free(void * ptr)
{
  os_memheap_free(&sdram_heap, ptr);
}

#endif

void *memcpy(void *dst, const void *src, size_t n) {
    if ((!(((uintptr_t)dst) & 3) && !(((uintptr_t)src) & 3))) {
        // pointers aligned
        uint32_t *d = dst;
        const uint32_t *s = src;

        // copy words first
        for (size_t i = (n >> 2); i; i--) {
            *d++ = *s++;
        }

        if (n & 2) {
            // copy half-word
            *(uint16_t*)d = *(const uint16_t*)s;
            d = (uint32_t*)((uint16_t*)d + 1);
            s = (const uint32_t*)((const uint16_t*)s + 1);
        }

        if (n & 1) {
            // copy byte
            *((uint8_t*)d) = *((const uint8_t*)s);
        }
    } else {
        // unaligned access, copy bytes
        uint8_t *d = dst;
        const uint8_t *s = src;

        for (; n; n--) {
            *d++ = *s++;
        }
    }

    return dst;
}