#include <os_memory.h>
#include <board.h>
#include <oc_mem.h>

#ifdef CUSTOM_RAM
os_memheap_t g_oc_sram_heap;

void oc_mem_init(void)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern int Image$$RW_IRAM1$$ZI$$Limit;
    void *sram_base = &Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
    #pragma section = "CSTACK"
    void *sram_base = __segment_end("CSTACK");
#else
    extern int __bss_end;
    void *sram_base = &__bss_end;
#endif
    memset(&g_oc_sram_heap, 0, sizeof(os_memheap_t));
    os_memheap_init(&g_oc_sram_heap, "SRAM_HEAP");
    os_memheap_add(&g_oc_sram_heap, (void *)sram_base, (os_size_t)STM32_SRAM_END - (os_size_t)sram_base, OS_MEM_ALG_DEFAULT);    
}

void *oc_alloc(os_size_t size)
{
    return os_memheap_alloc(&g_oc_sram_heap, size);
}

void *oc_realloc(void *ptr, os_size_t size)
{
    return os_memheap_realloc(&g_oc_sram_heap, ptr, size);
}

void *oc_calloc(os_size_t count, os_size_t size)
{
    void *ptr;

    ptr = os_memheap_alloc(&g_oc_sram_heap, count * size);
    if (ptr)
    {
        (void)memset(ptr, 0, count * size);
    }
    return ptr;
}

void oc_free(void *ptr)
{
    if(NULL != ptr)
    {
        os_memheap_free(&g_oc_sram_heap, ptr);
    }
}
#else
void *oc_alloc(os_size_t size)
{
    return os_malloc(size);
}

void *oc_realloc(void *ptr, os_size_t size)
{
    return os_realloc(ptr, size);
}

void *oc_calloc(os_size_t count, os_size_t size)
{
    return os_calloc(count, size);
}

void oc_free(void *ptr)
{
    if(NULL != ptr)
    {
        os_free(ptr);
    }
}
#endif
