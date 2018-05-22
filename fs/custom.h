/* Custom header file for changes in namei.c */

#include <linux/string.h>
#include <asm-generic/errno-base.h>
#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <linux/ceph/types.h>
#include <crypto/skcipher.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/completion.h>
#include <linux/random.h>
#include <linux/lz4.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <asm/atomic.h>

#define DEFAULT_TRASH_SIZE	10
#define DEFAULT_QUEUE_LEN	10

/* Temp file paths */
#define TEMP_ENC_FILE		"/.recycle_bin/.temp_enc_file"
#define TEMP_ZIP_FILE		"/.recycle_bin/.temp_zip_file"
#define TEMP_DEC_FILE		"/.recycle_bin/.temp_dec_file"
#define TEMP_UNZIP_FILE		"/.recycle_bin/.temp_unzip_file"
#define FILE_PATH_SIZE		80

#define ENCRYPT			1
#define ZIP			2
#define DECRYPT			4
#define UNZIP			8			


extern atomic_t waiting_count;		/* Number of processes in wait queue */
extern wait_queue_head_t wait_queue_head;  /* Wait queue pointer */
extern wait_queue_head_t wait_async_queue; /* Wait queue for async wait */

extern unsigned int trash_size;		/* Max number of files allowed in trash */
extern unsigned int queue_size;		/* Max number of files allowed in async queue */
extern int encrypt_key_size;		/* Size of encryption key */
extern char *encrypt_key_buff;		/* Buffer containing encryption key */
extern atomic_t items_count;			/* Number of items in async queue */
extern int enqueue_work(const char *, int );	/* Function to initialize and add task to async queue */
extern void init_wait_queue(void);		/*Function to initialize wait queue */
extern void init_async_wait_queue(void);

static int custom_filldir(struct dir_context *, const char *, int , loff_t , u64 , unsigned int );  	/* Custom filldir function */
static int trash_folder_check(void);  		       	         /* Function that checks for existence of trash folder */
void check_trash_contents(void); 				 /* Function that checks for the contents of the trash folder */
int encrypt_file(const char *, const char *, int ); 		 /* Function to encrypt file */
static void tcrypt_complete(struct crypto_async_request *, int ); /* Function to initialize crypto transformation object */
static int kencrypt(const char *, int, char *,char *, int, int ); /* Function that performs actual encryption */
int zip_file(const char *, const char *, int ); 		  /* Function for compressing files */
int do_encrypt_zip(const char *, int ); 			  /* Function that handles all encryption/compression/decryption/expansion requests */
static int temp_compare(const char *);

/* Custom structure for the encryption function */
struct tcrypt_result {
	struct completion completion;
	int err;
};


extern atomic_t trash_file_count;		/* Number of files moved to trash folder */
struct workqueue_struct *wq = NULL;	/* Pointer to work queue */
EXPORT_SYMBOL(wq);

