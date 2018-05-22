#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <asm/atomic.h>

extern atomic_t items_count;
extern int queue_size;
extern atomic_t async_wait_count;
extern wait_queue_head_t wait_async_queue;

extern void init_async_wait_queue(void);
extern int do_encrypt_zip(const char *, int );
extern struct workqueue_struct *wq;

void do_async_work(struct work_struct *);
int enqueue_work(const char *, int );

struct work_item {
	struct work_struct work;
	char *ipfile;
	int flags;
};


