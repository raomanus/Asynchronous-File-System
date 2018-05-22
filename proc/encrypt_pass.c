#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define	MAX_KEY_SIZE	16

int encrypt_len, encrypt_temp;
char *encrypt_key_buff;
int encrypt_key_size = 0;

EXPORT_SYMBOL(encrypt_key_size);
EXPORT_SYMBOL(encrypt_key_buff);

static ssize_t encrypt_read_proc(struct file *filp, char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	if (count > encrypt_temp)
		count = encrypt_temp;
	
	encrypt_temp -= count;
	error = copy_to_user(buf, encrypt_key_buff, count);
	
	if (count == 0)
		encrypt_temp = encrypt_len;
	return count;
}

ssize_t encrypt_write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	int error = 0;
	error = copy_from_user(encrypt_key_buff, buf, count);
	encrypt_len = count;
	encrypt_key_size = count<MAX_KEY_SIZE?count:MAX_KEY_SIZE;
	encrypt_temp = encrypt_len;	

	return count;
}

static const struct file_operations proc_fops = {
	.read = encrypt_read_proc,
	.write = encrypt_write_proc,
};

static void create_new_proc_entry(void) 
{
	proc_create("encrypt_key", 0666, NULL, &proc_fops);
	encrypt_key_buff = (char *) kmalloc(MAX_KEY_SIZE*sizeof(char), GFP_KERNEL);
}

static int __init proc_trash_init(void)
{
	create_new_proc_entry();
	return 0;
}
fs_initcall(proc_trash_init);
