
#include <linux/module.h>
#include <linux/kernel.h>


int led_init(void)
{
	printk("haha mojing led init\n");
	return 0;
}

void Led_exit(void)
{
	printk("haha mojing led exit\n");

}

module_init(led_init);
module_exit(Led_exit);
MODULE_LICENSE("GPL");











