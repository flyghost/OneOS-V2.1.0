/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: gcthread.c,v 1.3 2005/01/22 16:01:12 lunn Exp $
 *
 */
#include <linux/kernel.h>
#include "nodelist.h"
//#include <cyg/kernel/kapi.h> prife

#if defined(CYGOPT_FS_JFFS2_GCTHREAD)

#define GC_THREAD_FLAG_TRIG 1
#define GC_THREAD_FLAG_STOP 2
#define GC_THREAD_FLAG_HAS_EXIT 4


os_uint32_t cyg_current_time(void)
{
	return 0;
}

static void jffs2_garbage_collect_thread(unsigned long data);

void jffs2_garbage_collect_trigger(struct jffs2_sb_info *c)
{
     struct super_block *sb=OFNI_BS_2SFFJ(c);

     /* Wake up the thread */
     D1(printk("jffs2_garbage_collect_trigger\n"));

     os_event_send(&sb->s_gc_thread_flags,GC_THREAD_FLAG_TRIG);
}

static struct os_task gc_thread;
void
jffs2_start_garbage_collect_thread(struct jffs2_sb_info *c)
{
     struct super_block *sb=OFNI_BS_2SFFJ(c);
     cyg_mtab_entry *mte;
     int result;
	
     OS_ASSERT(c);
	 
     mte=(cyg_dir *) sb->s_root;
     OS_ASSERT(mte);
	 
     os_event_init(&sb->s_gc_thread_flags, "gc_event", OS_IPC_FLAG_FIFO);	 
     os_mutex_init(&sb->s_lock, "gc_mutex", OS_IPC_FLAG_FIFO);
//     os_mutex_init(&mte->fs->syncmode, "fs_lock", OS_IPC_FLAG_FIFO);
	 
     D1(printk("jffs2_start_garbage_collect_thread\n"));
     /* Start the thread. Doesn't matter if it fails -- it's only an
      * optimisation anyway */
     result =  os_task_init(&sb->s_gc_thread, 
	                   "jffs2_gc_thread",
                       jffs2_garbage_collect_thread,
                       (void *)c,
                       (void*)sb->s_gc_thread_stack,
                       sizeof(sb->s_gc_thread_stack),
					   CYGNUM_JFFS2_GC_THREAD_PRIORITY,
					   CYGNUM_JFFS2_GC_THREAD_TICKS
					   );
	 if (result != OS_EOK) {
		 os_task_startup(&sb->s_gc_thread);
		 /* how to deal with the following filed? */
		 /* sb->s_gc_thread_handle; */
	 }
}

void
jffs2_stop_garbage_collect_thread(struct jffs2_sb_info *c)
{
     struct super_block *sb=OFNI_BS_2SFFJ(c);
     cyg_mtab_entry *mte;
	 os_uint32_t  e;
	 
     //OS_ASSERT(sb->s_gc_thread_handle);

     D1(printk("jffs2_stop_garbage_collect_thread\n"));
     /* Stop the thread and wait for it if necessary */

     os_event_send(&sb->s_gc_thread_flags,GC_THREAD_FLAG_STOP);

     D1(printk("jffs2_stop_garbage_collect_thread wait\n"));
	 
     os_event_recv(&sb->s_gc_thread_flags,
                   GC_THREAD_FLAG_HAS_EXIT,
                   OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
				   OS_IPC_WAITING_FOREVER,  &e);

     // Kill and free the resources ...  this is safe due to the flag
     // from the thread.
     os_task_deinit(&sb->s_gc_thread);
     os_sem_deinit(&sb->s_lock);
     os_event_deinit(&sb->s_gc_thread_flags);
}


static void
jffs2_garbage_collect_thread(unsigned long data)
{
     struct jffs2_sb_info *c=(struct jffs2_sb_info *)data;
     struct super_block *sb=OFNI_BS_2SFFJ(c);
     cyg_mtab_entry *mte;
     os_uint32_t flag = 0;
	 
     D1(printk("jffs2_garbage_collect_thread START\n"));

     while(1) {
          os_event_recv(&sb->s_gc_thread_flags,
                        GC_THREAD_FLAG_TRIG | GC_THREAD_FLAG_STOP,
                        OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
				        cyg_current_time() + CYGNUM_JFFS2_GS_THREAD_TICKS,  
						&flag);

          if (flag & GC_THREAD_FLAG_STOP)
               break;

          D1(printk("jffs2: GC THREAD GC BEGIN\n"));

          mte=(cyg_dir *) sb->s_root;
          OS_ASSERT(mte != NULL);

          if (jffs2_garbage_collect_pass(c) == -ENOSPC) {
               printf("No space for garbage collection. "
                      "Aborting JFFS2 GC thread\n");
               break;
          }
          D1(printk("jffs2: GC THREAD GC END\n"));
     }

     D1(printk("jffs2_garbage_collect_thread EXIT\n"));
     os_event_send(&sb->s_gc_thread_flags,GC_THREAD_FLAG_HAS_EXIT);	 
}
#endif
