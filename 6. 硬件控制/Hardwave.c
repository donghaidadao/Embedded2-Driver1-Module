
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>  //字符设备头文件
#include <asm/io.h>   //io操作的头文件(for  ioremap readl)




static
dev_t dev_num = 0;
#define DEV_NAME "Device Init"

#define GPIO3  	0x6000D200 // 第3个Bank GPIO 的基地址
#define CNF     0x04  //配置寄存器 (0:GPIO  1:SFIO)  偏移量
#define OE   	0x14  //输出使能寄存器 (1:使能 0:关闭)
#define OUT  	0x24  //输出寄存器（1：高电平 0：低电平）
#define MSK_CNF 0x84  //配置屏蔽寄存器(高位1:屏蔽 高位0:不屏蔽   低位1:GPIO模式 低位0:SFIO模式)
#define MSK_OE  0x94  //输出使能屏蔽寄存器(高位1:禁止写   低位1:使能)
#define MSK_OUT 0xA4  //输出屏蔽寄存器(高位1:禁止写   低位1:高电平)
#define  PINMUX_AUX_DAP4_SCLK_0  0x70003150   //管脚复用设置


unsigned char *gpio_base = NULL;
unsigned char *gpio_pinmux = NULL;


void LedOn(void)
{
	writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //引脚输出高电平，点亮灯
	printk("cmd is Hard led_on\r\n");
}

void LedOff(void)
{
	writel(readl(gpio_base+OUT) & ~(1 << 7), gpio_base+OUT);  //引脚输出低电平，灭灯
	printk("cmd is Hard led_off\r\n");
}

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
			LedOn();
		break;

		case LED_OFF:
			LedOff();
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
	cdev_init(&dev, &oper);
	if(cdev_add(&dev, dev_num, 1) < 0){
		printk("cdev_add Is Failed \n");
		goto error1;
	}else{
		printk("cdev_add Is Success \n");
		
	}	

	gpio_pinmux = ioremap(PINMUX_AUX_DAP4_SCLK_0, 8);

	if(gpio_pinmux == NULL){
		printk("remap1 is error \r\n");
		goto error2;
	}

	writel((readl(gpio_pinmux) & ~(1 << 4))|1, gpio_pinmux);

	gpio_base = ioremap(GPIO3, 0xFF);
	if(NULL == gpio_base){
		printk("remap2 is error \r\n");
		goto error3;
	}
    writel(readl(gpio_base+CNF) | 1 << 7, gpio_base+CNF);   //配置引脚GPIO3_PJ.07 为 GPIO模式
    writel(readl(gpio_base+OE) | 1 << 7, gpio_base+OE);	  //使能引脚(7号)
    writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //输出高电平，点亮灯
    writel(readl(gpio_base+MSK_CNF) | 1 << 7, gpio_base+MSK_CNF); //取消对GPIO模下引脚的屏蔽
    writel(readl(gpio_base+MSK_OE) | 1 << 7, gpio_base+MSK_OE);  //取消引脚 使能屏蔽

	
//void cdev_init(struct cdev *cdev, const struct file_operations *fops)	
//int cdev_add(struct cdev *p, dev_t dev, unsigned count)


	return 0;

error3:
	iounmap(gpio_base);
error2:
	cdev_del(&dev);
error1:
	unregister_chrdev_region(dev_num, 1);
	return -1;
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











