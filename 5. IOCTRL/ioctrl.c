#include <linux/kernel.h>
#include <linux/module.h>  //模块的头文件 (for module_init MODULE_LICENSE)
#include <linux/fs.h>	  //for MKDEV register_chrdev_region
#include <linux/cdev.h>  //字符设备头文件

#include <asm/io.h>   //io操作的头文件(for  ioremap readl)





static
dev_t dev_num = 0;
#define DEV_NAME "Device Init"

/*
int register_chrdev_region(dev_t from, unsigned count, const char *name)
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
			const char *name)

static inline int register_chrdev(unsigned int major, const char *name,
				  const struct file_operations *fops)
{
	return __register_chrdev(major, 0, 256, name, fops);
}			

*/


/*
struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iterate) (struct file *, struct dir_context *);
	int (*iterate_shared) (struct file *, struct dir_context *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
			loff_t, size_t, unsigned int);
	int (*clone_file_range)(struct file *, loff_t, struct file *, loff_t,
			u64);
	ssize_t (*dedupe_file_range)(struct file *, u64, u64, struct file *,
			u64);
};

*/

int dev_open(struct inode *d_node, struct file *dfile)
{
	printk("Device Is Open \r\n");
	return 0;
}

int dev_close(struct inode *d_node, struct file *dfile)
{
	printk("Device Is Close \r\n");
	return 0;
}

#define LED_MAG		'L'
#define LED_ON		_IOW(LED_MAG, 2, int)
#define LED_OFF		_IOW(LED_MAG, 3, int)
long Ioctrl(struct file *dfile, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case LED_ON:
			printk("cmd is led_on\r\n");
		break;

		case LED_OFF:
			printk("cmd is led_off\r\n");
		break;

		default:
			break;
	}
	printk("Cmd Val is %d\r\n", cmd);
	return 0;
}


static 
struct file_operations oper = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.unlocked_ioctl = Ioctrl,
};

static 
struct cdev dev;

int Device_init(void)
{
	int major = 0;
	int minor = 0;
	dev_num = MKDEV(major, minor);
	printk("Register Is Run init\n");
	
	if(major){
		if(register_chrdev_region(dev_num, 1, DEV_NAME) < 0){
			printk("register_chrdev_region Is Error \n");
			return -1;
		}
	}else{
		if(alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME) < 0){
			printk("alloc_chrdev_region Is Error \n");
			return -1;
		}else{
			major = MAJOR(dev_num);
			minor = MINOR(dev_num);
			printk("alloc_chrdev_region Is Success major = %d, minor = %d \n", major, minor);
		}
	}
//void cdev_init(struct cdev *cdev, const struct file_operations *fops)	
//int cdev_add(struct cdev *p, dev_t dev, unsigned count)
	cdev_init(&dev, &oper);
	if(cdev_add(&dev, dev_num, 1) < 0){
		printk("cdev_add Is Failed \n");
		return -1;
	}else{
		printk("cdev_add Is Success \n");
		
	}
	
	return 0;
}


/*
void unregister_chrdev_region(dev_t from, unsigned count)

static inline void unregister_chrdev(unsigned int major, const char *name)
{
	__unregister_chrdev(major, 0, 256, name);
}


*/
void Device_exit(void)
{
	printk("Exit the Device Init\n");
	unregister_chrdev_region(dev_num, 1);
	cdev_del(&dev);
}

module_init(Device_init);
module_exit(Device_exit);
MODULE_LICENSE("GPL");











