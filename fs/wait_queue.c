#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <asm/atomic.h>
#include <linux/types.h>

atomic_t waiting_count = ATOMIC_INIT(0);
atomic_t trash_file_count = ATOMIC_INIT(-2);
atomic_t async_wait_count = ATOMIC_INIT(0);

/* Variables for wait queue head */
wait_queue_head_t wait_queue_head;
static wait_queue_head_t *wq_ptr = NULL;

wait_queue_head_t wait_async_queue;
static wait_queue_head_t *wq_async = NULL;

EXPORT_SYMBOL(trash_file_count);
EXPORT_SYMBOL(waiting_count);
EXPORT_SYMBOL(async_wait_count);
EXPORT_SYMBOL(wait_queue_head);
EXPORT_SYMBOL(wait_async_queue);

void init_wait_queue(void);
void init_async_wait_queue(void);

/* Functions to initialize wait queue */
void init_wait_queue()
{
	if (!wq_ptr) {
		init_waitqueue_head(&wait_queue_head);
		wq_ptr = &wait_queue_head;
	}
}
EXPORT_SYMBOL(init_wait_queue);

void init_async_wait_queue()
{
	if (!wq_async) {
		init_waitqueue_head(&wait_async_queue);
		wq_async = &wait_async_queue;
	}
}
EXPORT_SYMBOL(init_async_wait_queue);
