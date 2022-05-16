#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>  
#include <asm/io.h>   //io操作的头文件(for  ioremap readl)
#include <asm/uaccess.h>  //for copy_to_user
#include <linux/platform_device.h>
#include <linux/of.h>

struct resource *res1;
struct resource *res2;
struct class *led_class;	


#define LED_MA   500  //定义主设备号(区分不同的类别)

#define LED_MI   0  //次设备号

struct cdev cdev; //定义字符设备


#define LED_MAGIC   'L'  //幻数
#define  LED_ON   _IOW(LED_MAGIC,2,int)  //点亮灯
#define  LED_OFF  _IOW(LED_MAGIC,3,int)   //灭灯

//把硬件地址剥离到设备树中，间接访问
//#define  PINMUX_AUX_DAP4_SCLK_0  0x70003150   //管脚复用设置

//#define GPIO3  	0x6000D200 // 第3个Bank GPIO 的基地址
#define CNF     0x04  //配置寄存器 (0:GPIO  1:SFIO)  偏移量
#define OE   	0x14  //输出使能寄存器 (1:使能 0:关闭)
#define OUT  	0x24  //输出寄存器（1：高电平 0：低电平）
#define MSK_CNF 0x84  //配置屏蔽寄存器(高位1:屏蔽 高位0:不屏蔽   低位1:GPIO模式 低位0:SFIO模式)
#define MSK_OE  0x94  //输出使能屏蔽寄存器(高位1:禁止写   低位1:使能)
#define MSK_OUT 0xA4  //输出屏蔽寄存器(高位1:禁止写   低位1:高电平)

 

unsigned char *gpio_pinmux;
unsigned char * gpio_base;

int led_open (struct inode * inode, struct file * file){

    printk(" led open go\n");
    return 0;
}

int led_release(struct inode *inode, struct file * file){
    printk(" led release go\n");
    return 0;
}

//开灯
void led_on(void)
{
        writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //引脚输出高电平，点亮灯
	printk("out put high ,led on 输出高电平，点亮灯\n");
}

//关灯
void led_off(void)
{
        writel(readl(gpio_base+OUT) & ~(1 << 7), gpio_base+OUT);  //引脚输出低电平，灭灯
	printk("out put low, led off 输出低电平，灭灯\n");
}


long led_ioctl(struct file *inode, unsigned int cmd , unsigned long args)
{
      printk("cmd=%d\n",cmd);
   	switch(cmd)
   	{
	case LED_ON:
		led_on();
		break;   	
	case LED_OFF:
		led_off();
	default:
		printk("no fond cmd =%d\n",cmd);
		return -1;
   	}
   	return 0;
}



 struct file_operations led_fops={//实现需要的文件操作
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
};	


static int led_probe(struct platform_device *pdev)
{
     int  devno = MKDEV(LED_MA,LED_MI); //合并主次设备号，生成设备ID
     int  ret; 

    //1.注册设备号
      ret = register_chrdev_region(devno,1,"yhai_led");
     if(ret<0){
	 	printk("register_chrdev_region fail \n");
		return ret;
     }

    //2.初始化字符设备
    cdev_init(&cdev,&led_fops);  //字符设备的初始化

    ret =cdev_add(&cdev,devno,1); 
     if(ret<0){
	 	printk("cdev_add fail \n");
		goto err1;
     }

	res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0); 
	if (res1 == NULL) {
		printk("res1 platform_get_resource fail \n");
		goto err4;
	}
	printk("res1->start=%llx\n",res1->start);
	res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res2 == NULL) {
		printk("res2 platform_get_resource fail\n");
		goto err5;
	}	 

     //硬件控制
     //a. 管脚复用的配置，配置为GPIO功能
     //内核驱动里不能直接操作硬件地址，必须映射转化
     	// *PINMUX_AUX_DAP4_SCLK_0 = 0x45;
	// *gpio_out  =0x1
      gpio_pinmux = ioremap(res2->start,8); /*从物理地址PINMUX_AUX_DAP4_SCLK_0开始，映射 8字节长度的空间到内核空间
    动态映射 物理地址 到内核虚拟地址
	phys_addr  起始物理地址
	size       映射范围大小，单位字节
		返回值     映射后的内核虚拟地址 
	*/	
     	if(gpio_pinmux ==NULL){
			printk("ioremap  gpio_pinmux faile \n");		
			goto err2;
     	}
       writel((readl(gpio_pinmux) & ~(1 << 4))|1, gpio_pinmux); /*管脚复用配置用于 GPIO
        1:0 I2S4B    PM: 0 = I2S4B 1 = RSVD1 2 = RSVD2  3 = RSVD3 
        设为非0，表示不用作I2S功能，则默认用做GPIO功能

        4 TRISTATE TRISTATE:   0 = PASSTHROUGH  1 = TRISTATE
        设为0，设为直通状态才能驱动外面的设备

        见 9.5.1 Per Pad Options
     Tristate     高阻态 -> 与外界是断开的，默认启动设为高阻太，避免驱动影响外面的设备             
     passthrough  直通态 -> 才能驱动外面设备   
	*/


       //b.GPIO功能的内部设置
	    gpio_base = ioremap(res1->start, 0xFF); 
	    if (gpio_base == NULL) {
			printk("ioremap gpio_base error\n");
			goto err3;
	    }

		writel(readl(gpio_base+CNF) | 1 << 7, gpio_base+CNF);   //配置引脚GPIO3_PJ.07 为 GPIO模式
		writel(readl(gpio_base+OE) | 1 << 7, gpio_base+OE);	  //使能引脚(7号)
		writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //输出高电平，点亮灯
		writel(readl(gpio_base+MSK_CNF) | 1 << 7, gpio_base+MSK_CNF); //取消对GPIO模下引脚的屏蔽
		writel(readl(gpio_base+MSK_OE) | 1 << 7, gpio_base+MSK_OE);  //取消引脚 使能屏蔽


	led_class = class_create(THIS_MODULE, "yhai_dev_class");
	if (IS_ERR(led_class)) {
		printk("Err: failed in creating class.\n");
		goto err6;
	}

	/* register your own device in sysfs, and this will cause udevd to create corresponding device node */
	device_create(led_class,NULL, devno, NULL,"platled");     
		
	printk("platform led probe 1\n");
	return 0;

	//goto 实现 顺序申请，逆序释放
err6:	
err5:	
err4:	
err3:
	iounmap(gpio_pinmux);
err2:
	cdev_del(&cdev);
err1:
	unregister_chrdev_region(devno,1);
	return ret;
	
}

static int led_remove(struct platform_device *pdev)

{
	//配对
	//注销设备
	int  devno = MKDEV(LED_MA,LED_MI); 
	//顺序申请，逆序释放

	device_destroy(led_class, devno);
	class_destroy(led_class);
	iounmap(gpio_pinmux); 
	iounmap(gpio_base);
	cdev_del(&cdev);
	unregister_chrdev_region(devno,1);
	printk("platform led exit 1\n");
	return 0;
}



static const struct of_device_id of_led_match[] = {
    { .compatible = "yhai-plat", },
    {},  
};
MODULE_DEVICE_TABLE(of, of_led_match);

static struct platform_driver led_driver = {
    .driver = {
        .name   = "yhai platform led" ,
        .owner = THIS_MODULE,
        .of_match_table = of_led_match,
    },   
    .probe = led_probe,
    .remove = led_remove,
};

module_platform_driver(led_driver);

MODULE_DESCRIPTION(" cdev to platform device ");
MODULE_AUTHOR("chendu yhai"); //模块作者声明(可选)
MODULE_LICENSE("GPL"); //模块免费开源声明








