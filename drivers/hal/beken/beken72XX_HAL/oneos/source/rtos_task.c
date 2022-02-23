/**
 * RTOS测试用例
 */ 

#include "sys_rtos.h"
#include "error.h"
#include "rtos_pub.h"
#include "shell.h"
#include "mem_pub.h"


//#include "finsh.h"

#define THREAD_TIMESLICE 5

#define RTOS_DEBUG   1
#if RTOS_DEBUG
#define RTOS_DBG(...)     os_kprintf("[RTOS]"),os_kprintf(__VA_ARGS__)
#else
#define RTOS_DBG(...)
#endif



/**
 * 消息队列测试
 */
static beken_queue_t mq;
static beken_thread_t queue_thread_revive;
static beken_thread_t queue_thread_send;
#define MSG_SIZE 12 /* 消息长度 */
#define MSG_NUM  10 /* 消息数量 */

static beken_semaphore_t sem;
static beken_semaphore_t sem_send;
static beken_semaphore_t sem_recv;

static beken_mutex_t mutex;


//OSStatus rtos_init_mutex( beken_mutex_t* mutex )



static void thread_queue_recive_entry(void* parameter)
{
    os_uint8_t buf[MSG_SIZE];

    while(1)
    {
    	rtos_get_semaphore(&sem,200);
		//rtos_get_semaphore(&sem_send,1000);
		//rtos_lock_mutex(&mutex);
    	os_kprintf("recive 1111 thread test!!!\n");
		
        os_memset(&buf[0], 0 , sizeof(buf));
        if(rtos_pop_from_queue(&mq, &buf[0], 100) == kNoErr)
        {
            os_kprintf("recive thread:recive msg = %s\n", &buf);
        }  
        else
        {
            os_kprintf("recive thread:failed\n");
        }
		os_kprintf("recive 2222 thread test!!!\n");
		
        rtos_delay_milliseconds(100);


		//rtos_unlock_mutex(&mutex);
		//rtos_set_semaphore(&sem_recv);
		rtos_set_semaphore(&sem);
    }
}

static void thread_queue_send_entry(void* parameter)
{
    os_uint8_t buf[MSG_SIZE];

    while(1)
    {
    	rtos_get_semaphore(&sem,200);
    	//rtos_get_semaphore(&sem_recv,1000);
    	//rtos_lock_mutex(&mutex);
    	os_kprintf("send 1111 thread test!!!\n");
		
        os_memset(&buf[0], 0 , sizeof(buf));
        for(int i = 0; i < MSG_SIZE - 1; i++)
        {
            buf[i] = 'a' + i;
        }
        if(!rtos_is_queue_full(&mq))
        {
            os_kprintf("send thread:%s\n", &buf[0]);
            rtos_push_to_queue(&mq, &buf[0], 0);
        }
        else
        {
            os_kprintf("send thread: queue full\n");
            rtos_delay_milliseconds(100);
            return;
        }

		os_kprintf("send 2222 thread test!!!\n");
		
        os_memset(&buf[0], 0 , sizeof(buf));
		
		//rtos_unlock_mutex(&mutex);
		//rtos_set_semaphore(&sem_send);
		rtos_set_semaphore(&sem);
    }
}

int msg_queue_simple_init(void)
{

    os_uint8_t buf[MSG_SIZE];
    os_err_t result;

    /* 初始化消息队列 */
    result = rtos_init_queue(&mq, "msg_queue", MSG_SIZE, MSG_NUM);
    if(result != kNoErr)
    {
        os_kprintf("init rtos queue failed\n");
    }

	rtos_init_semaphore_ex(&sem,"sem",1,1);
	rtos_init_semaphore(&sem_send,0);
	rtos_init_semaphore(&sem_recv,0);
	
	rtos_init_mutex(&mutex);

    /* 创建recive线程 */
    rtos_create_thread(&queue_thread_revive,3, "recv_thread", thread_queue_recive_entry, 1024, OS_NULL);

    /* 创建send线程 */
    rtos_create_thread(&queue_thread_send, 4, "send_thread", thread_queue_send_entry, 1024, OS_NULL);
}

SH_CMD_EXPORT(msg_queue_simple_init,msg_queue_simple_init,"msq queue simple");


/**
 * 定时器测试
 */
// static beken_thread_t one_shot_tim;
static beken2_timer_t one_shot;
static void one_shot_timeout(void* Larg, void* Rarg)
{
    os_kprintf("enter one shot time out !!! \n");
}

int one_shot_time_simple_init(void)
{
    rtos_init_oneshot_timer(&one_shot, 10000, one_shot_timeout, OS_NULL, OS_NULL);
    rtos_start_oneshot_timer(&one_shot);
}

SH_CMD_EXPORT(one_shot_time_simple_init, one_shot_time_simple_init,"one shot time simple");


