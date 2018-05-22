#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define	MAX_INT_LEN	10

int trash_len, trash_temp;
char *trash_size_buff;
unsigned int trash_size = 10;

EXPORT_SYMBOL(trash_size);

static ssize_t trash_read_proc(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	if (count > trash_temp)
		count = trash_temp;
	
	trash_temp -= count;
	error = copy_to_user(buf, trash_size_buff, count);
	
	if (count == 0)
		trash_temp = trash_len;
	return count;
}

ssize_t trash_write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	error = copy_from_user(trash_size_buff, buf, count);
	trash_size_buff[count] = '\0';
	trash_len = count;
	trash_temp = trash_len;
	
	error = kstrtouint(trash_size_buff, 10, &trash_size);
	printk("Set trash size to %s , %d\n", trash_size_buff, trash_size);
	if (error < 0)
		return error;
	
	return count;
}

static const struct file_operations proc_fops = {
	.read = trash_read_proc,
	.write = trash_write_proc,
};

static void create_new_proc_entry(void) 
{
	proc_create("trash_size", 0666, NULL, &proc_fops);
	trash_size_buff = (char *) kmalloc(MAX_INT_LEN*sizeof(char), GFP_KERNEL);
}

static int __init proc_trash_init(void)
{
	create_new_proc_entry();
	return 0;
}
fs_initcall(proc_trash_init);
