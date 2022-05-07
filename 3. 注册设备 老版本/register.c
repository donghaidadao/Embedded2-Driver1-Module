
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>


static
dev_t dev_num = 0;
#define DEV_NAME "Register Test"

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

static 
struct file_operations oper = {
	.owner = THIS_MODULE,
};

int led_init(void)
{
	int major = 0;
	int minor = 0;
//	dev_num = MKDEV(major, minor);

	printk("Register Is Run init\n");
	major = register_chrdev(major, DEV_NAME, &oper);
	dev_num = MKDEV(major, minor);
	/*
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
			dev_num = MKDEV(major, minor);
			printk("alloc_chrdev_region Is Success \n");
		}
	}
	*/
	
	return 0;
}


/*
void unregister_chrdev_region(dev_t from, unsigned count)

static inline void unregister_chrdev(unsigned int major, const char *name)
{
	__unregister_chrdev(major, 0, 256, name);
}


*/
void Led_exit(void)
{
	printk("haha mojing led exit\n");
//	unregister_chrdev_region(dev_num, 1);
	unregister_chrdev(MAJOR(dev_num), DEV_NAME);
}

module_init(led_init);
module_exit(Led_exit);
MODULE_LICENSE("GPL");











