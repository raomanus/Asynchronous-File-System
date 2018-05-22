#include "wq.h"

void do_async_work(struct work_struct *data)
{
	int error = 0;
	struct work_item *item = (struct work_item *) data;
	
	printk("Performing operations asynchronously\n");

	error = do_encrypt_zip(item->ipfile, item->flags);
	
	if (error < 0)
		printk("Failed to do operations on %s\n", item->ipfile);
	
	atomic_dec(&items_count);

	/* Wake up any blocked process to be added to queue */
	if (async_wait_count.counter > 0) {
		atomic_dec(&async_wait_count);
		wake_up_interruptible(&wait_async_queue);
	}
}

int enqueue_work(const char *input_file, int flags)
{
	int ret = 0;
	struct work_item *new_item = NULL;
	
	/* BLock process if queue is full */
	if (items_count.counter >= queue_size) {
		atomic_inc(&async_wait_count);
		init_async_wait_queue();
		wait_event_interruptible(wait_async_queue, items_count.counter<queue_size);
	}

	if (!wq) {
		printk(KERN_INFO "Creating the work queue\n");
		wq = create_workqueue("async_delete");
	}

	/* Initialize new work item */
	new_item = (struct work_item *) kmalloc(sizeof(struct work_item), GFP_KERNEL);
	if (!new_item)
		return -ENOMEM;
	
	INIT_WORK( (struct work_struct *) new_item, do_async_work);

	new_item->ipfile = (char *) kmalloc(strlen(input_file)*sizeof(char), GFP_KERNEL);
	if (!new_item->ipfile)
		return -ENOMEM;
	
	strcpy(new_item->ipfile, input_file);
	strcat(new_item->ipfile, "\0");
	
	new_item->flags = flags;
	
	/* Add work item to queue */
	ret = queue_work(wq, (struct work_struct *) new_item);
	
	atomic_inc(&items_count);

	return ret;
}
EXPORT_SYMBOL(enqueue_work);
