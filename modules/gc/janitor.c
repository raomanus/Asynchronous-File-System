#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define TRASH_FOLDER "/.recycle_bin/"

unsigned int oldest = 0;
char *old_file = NULL;

static int
janitor_filldir(struct dir_context *ctx, const char *lower_name,
                 int lower_namelen, loff_t offset, u64 ino, unsigned int d_type);


static int
janitor_filldir(struct dir_context *ctx, const char *lower_name,
                 int lower_namelen, loff_t offset, u64 ino, unsigned int d_type)
{
	int error = 0;
	unsigned int temp_time;
	char *name = (char *)kmalloc((strlen(lower_name)+1)*sizeof(char), GFP_KERNEL);
	char *toFree = name;
	char *time_stamp = NULL;
	char *file_uid = NULL;
	
        if (strcmp(lower_name, "..") && strcmp(lower_name, ".")) {
        	strcpy(name, lower_name);
		file_uid = strsep(&name, ":");
		if (file_uid)
			time_stamp = strsep(&name, ":");
		if (time_stamp) {
			error = kstrtouint(time_stamp, 10, &temp_time);
			if (error < 0)	
				goto out;
			if ( !oldest || (oldest > temp_time) ) {
				oldest = temp_time;
				if (old_file)
					kfree(old_file);
				old_file = (char *) kmalloc((strlen(lower_name)+1)*sizeof(char), GFP_KERNEL);
				strcpy(old_file, lower_name);
				strcat(old_file, "\0");
			}
		}
        }

out:
	kfree(toFree);
        return 0;       
}

void mop(void)
{
        int error;
	char *del_file_path;      
	struct file *filp = NULL;
	struct file *del_file = NULL;
	struct path *trash_path;
	struct inode *pinode;
        struct dir_context context = { janitor_filldir, 0 };
        mm_segment_t oldfs;

        oldfs = get_fs();
        set_fs(KERNEL_DS);
        filp = filp_open(TRASH_FOLDER, O_RDONLY, 0);
        if (!filp || IS_ERR(filp)) {
        	printk("Trash folder open error\n"); 
	        return;
	}

        error = iterate_dir(filp, &context);

	printk("Iterate returned %d\n", error);	

	filp_close(filp, NULL);
	
        if (oldest && old_file) {
		printk("Deleting %s\n", old_file);
		del_file_path = (char *) kmalloc( (strlen(TRASH_FOLDER)+strlen(old_file)+1)*sizeof(char), GFP_KERNEL);
		if (!del_file_path)
			goto clean_exit;

		strcpy(del_file_path, TRASH_FOLDER);
		strcat(del_file_path, old_file);
		
		del_file = filp_open(del_file_path, O_RDONLY, 0);
		if (!del_file || IS_ERR(del_file))
			goto cleanup;

		trash_path = &del_file->f_path;
		pinode = trash_path->dentry->d_parent->d_inode;
		
		inode_lock(pinode);
		error = vfs_unlink(pinode, trash_path->dentry, NULL);
		inode_unlock(pinode);

		if (error >= 0)
			printk("Successfully deleted %s\n", del_file_path);
	cleanup:
		kfree(del_file_path);
		filp_close(del_file, NULL);
	}

clean_exit:
	oldest = 0;
        set_fs(oldfs);

        return;
}

int repeat_mop(void *data)
{

trash_cleanup:
	mop();
	
	msleep(20000);
	goto trash_cleanup;
	
	return 0;
}

int janitor_init(void)
{
	static struct task_struct *janitor;

	janitor = kthread_create(repeat_mop, NULL, "janitor_man");
	if (janitor) {
		wake_up_process(janitor);
	}

        return 0;
}

void janitor_exit(void)
{
        printk("exiting\n");
}

module_init(janitor_init);
module_exit(janitor_exit);

MODULE_LICENSE("GPL");

