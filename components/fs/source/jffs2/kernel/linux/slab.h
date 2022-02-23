#ifndef __LINUX_SLAB_H__
#define __LINUX_SLAB_H__

//#include <stdlib.h>  //prife

#include <asm/page.h> /* Don't ask. Linux headers are a mess. */

#define kmalloc(x, y) os_malloc(x)
#define kfree(x) os_free(x)
#define vmalloc(x) os_malloc(x)
#define vfree(x) os_free(x)

#endif /* __LINUX_SLAB_H__ */

