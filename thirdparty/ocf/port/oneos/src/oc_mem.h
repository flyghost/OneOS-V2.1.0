#ifndef __OC_MEM_H__
#define __OC_MEM_H__

#include <os_types.h>

#ifdef CUSTOM_RAM
void oc_mem_init(void);
#endif

void *oc_alloc(os_size_t size);

void *oc_realloc(void *ptr, os_size_t size);

void *oc_calloc(os_size_t count, os_size_t size);

void oc_free(void *ptr);

#endif /*__OC_MEM_H__*/
