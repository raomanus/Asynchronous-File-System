#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <asm/atomic.h>


#define	MAX_QUEUE_SIZE	10

int queue_len, queue_temp;
char *queue_buff;
unsigned int queue_size = 10; 
atomic_t items_count = ATOMIC_INIT(0);

EXPORT_SYMBOL(queue_size);
EXPORT_SYMBOL(items_count);

static ssize_t queue_read_proc(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	if (count > queue_temp)
		count = queue_temp;
	
	queue_temp -= count;
	error = copy_to_user(buf, queue_buff, count);
	
	if (count == 0)
		queue_temp = queue_len;
	return count;
}

ssize_t queue_write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	error = copy_from_user(queue_buff, buf, count);
	queue_buff[count] = '\0';
	queue_len = count;
	queue_temp = queue_len;

	error = kstrtouint(queue_buff, 10, &queue_size);
	if (error < 0)
		return error;	

	return count;
}

static const struct file_operations proc_fops = {
	.read = queue_read_proc,
	.write = queue_write_proc,
};

static void create_new_proc_entry(void) 
{
	proc_create("queue_size", 0666, NULL, &proc_fops);
	queue_buff = (char *) kmalloc(MAX_QUEUE_SIZE*sizeof(char), GFP_KERNEL);
}

static int __init proc_trash_init(void)
{
	create_new_proc_entry();
	return 0;
}
fs_initcall(proc_trash_init);
