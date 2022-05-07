
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

*/
int led_init(void)
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
			dev_num = MKDEV(major, minor);
			printk("alloc_chrdev_region Is Success \n");
		}
	}
	return 0;
}


/*
void unregister_chrdev_region(dev_t from, unsigned count)
*/
void Led_exit(void)
{
	printk("haha mojing led exit\n");
	unregister_chrdev_region(dev_num, 1);
}

module_init(led_init);
module_exit(Led_exit);
MODULE_LICENSE("GPL");











