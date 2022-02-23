#include "state_machine.h"

#include <string.h>


struct model_misc_fun g_misc_fun;

/**
 *********************************************************************************************************
 *                                      time interface
 *********************************************************************************************************
*/

sm_err_t sm_mdelay(int32_t ms)
{
	return g_misc_fun.mdelay((void *)ms);
}


/**
 *********************************************************************************************************
 *                                      newlib c interface
 *********************************************************************************************************
*/

void *sm_memset(void*s,int c,size_t n)
{
	return memset(s,c,n);
}

void *sm_calloc(sm_size_t count, sm_size_t size)
{
	return g_misc_fun.calloc((void *)count , (void *)size);
}

void sm_free(void *ptr)
{
	g_misc_fun.free(ptr);
}


/**
 *********************************************************************************************************
 *                                      compute string hash
 *
 * @description: call the function to compute string hash
 *
 * @param      : data : source
 *
 *               len  : the length of data
 *
 * @returns    : hash
 *
* @note	   	   : djb2 algorithm; see http://www.cse.yorku.ca/~oz/hash.html
 *********************************************************************************************************
*/
uint32_t sm_compute_hash(const char *data, size_t len) { 
    uint32_t hash = 5381;
    for (const char *top = data + len; data < top; data++) {
        hash = ((hash << 5) + hash) ^ (*data); // hash * 33 ^ data
    }
    // Make sure that valid hash is never zero, zero means "hash not computed"
    return hash?hash & SM_HASH_MASK:1;
}


struct model_misc_fun *model_get_misc_structure(void)
{
	return &g_misc_fun;
}

