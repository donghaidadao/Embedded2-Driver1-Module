#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>  
#include <asm/io.h>   //io������ͷ�ļ�(for  ioremap readl)
#include <asm/uaccess.h>  //for copy_to_user
#include <linux/platform_device.h>
#include <linux/of.h>

struct resource *res1;
struct resource *res2;
struct class *led_class;	


#define LED_MA   500  //�������豸��(���ֲ�ͬ�����)

#define LED_MI   0  //���豸��

struct cdev cdev; //�����ַ��豸


#define LED_MAGIC   'L'  //����
#define  LED_ON   _IOW(LED_MAGIC,2,int)  //������
#define  LED_OFF  _IOW(LED_MAGIC,3,int)   //���

//��Ӳ����ַ���뵽�豸���У���ӷ���
//#define  PINMUX_AUX_DAP4_SCLK_0  0x70003150   //�ܽŸ�������

//#define GPIO3  	0x6000D200 // ��3��Bank GPIO �Ļ���ַ
#define CNF     0x04  //���üĴ��� (0:GPIO  1:SFIO)  ƫ����
#define OE   	0x14  //���ʹ�ܼĴ��� (1:ʹ�� 0:�ر�)
#define OUT  	0x24  //����Ĵ�����1���ߵ�ƽ 0���͵�ƽ��
#define MSK_CNF 0x84  //�������μĴ���(��λ1:���� ��λ0:������   ��λ1:GPIOģʽ ��λ0:SFIOģʽ)
#define MSK_OE  0x94  //���ʹ�����μĴ���(��λ1:��ֹд   ��λ1:ʹ��)
#define MSK_OUT 0xA4  //������μĴ���(��λ1:��ֹд   ��λ1:�ߵ�ƽ)

 

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

//����
void led_on(void)
{
        writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //��������ߵ�ƽ��������
	printk("out put high ,led on ����ߵ�ƽ��������\n");
}

//�ص�
void led_off(void)
{
        writel(readl(gpio_base+OUT) & ~(1 << 7), gpio_base+OUT);  //��������͵�ƽ�����
	printk("out put low, led off ����͵�ƽ�����\n");
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



 struct file_operations led_fops={//ʵ����Ҫ���ļ�����
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
};	


static int led_probe(struct platform_device *pdev)
{
     int  devno = MKDEV(LED_MA,LED_MI); //�ϲ������豸�ţ������豸ID
     int  ret; 

    //1.ע���豸��
      ret = register_chrdev_region(devno,1,"yhai_led");
     if(ret<0){
	 	printk("register_chrdev_region fail \n");
		return ret;
     }

    //2.��ʼ���ַ��豸
    cdev_init(&cdev,&led_fops);  //�ַ��豸�ĳ�ʼ��

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

     //Ӳ������
     //a. �ܽŸ��õ����ã�����ΪGPIO����
     //�ں������ﲻ��ֱ�Ӳ���Ӳ����ַ������ӳ��ת��
     	// *PINMUX_AUX_DAP4_SCLK_0 = 0x45;
	// *gpio_out  =0x1
      gpio_pinmux = ioremap(res2->start,8); /*�������ַPINMUX_AUX_DAP4_SCLK_0��ʼ��ӳ�� 8�ֽڳ��ȵĿռ䵽�ں˿ռ�
    ��̬ӳ�� �����ַ ���ں������ַ
	phys_addr  ��ʼ�����ַ
	size       ӳ�䷶Χ��С����λ�ֽ�
		����ֵ     ӳ�����ں������ַ 
	*/	
     	if(gpio_pinmux ==NULL){
			printk("ioremap  gpio_pinmux faile \n");		
			goto err2;
     	}
       writel((readl(gpio_pinmux) & ~(1 << 4))|1, gpio_pinmux); /*�ܽŸ����������� GPIO
        1:0 I2S4B    PM: 0 = I2S4B 1 = RSVD1 2 = RSVD2  3 = RSVD3 
        ��Ϊ��0����ʾ������I2S���ܣ���Ĭ������GPIO����

        4 TRISTATE TRISTATE:   0 = PASSTHROUGH  1 = TRISTATE
        ��Ϊ0����Ϊֱͨ״̬��������������豸

        �� 9.5.1 Per Pad Options
     Tristate     ����̬ -> ������ǶϿ��ģ�Ĭ��������Ϊ����̫����������Ӱ��������豸             
     passthrough  ֱ̬ͨ -> �������������豸   
	*/


       //b.GPIO���ܵ��ڲ�����
	    gpio_base = ioremap(res1->start, 0xFF); 
	    if (gpio_base == NULL) {
			printk("ioremap gpio_base error\n");
			goto err3;
	    }

		writel(readl(gpio_base+CNF) | 1 << 7, gpio_base+CNF);   //��������GPIO3_PJ.07 Ϊ GPIOģʽ
		writel(readl(gpio_base+OE) | 1 << 7, gpio_base+OE);	  //ʹ������(7��)
		writel(readl(gpio_base+OUT) | 1 << 7, gpio_base+OUT); //����ߵ�ƽ��������
		writel(readl(gpio_base+MSK_CNF) | 1 << 7, gpio_base+MSK_CNF); //ȡ����GPIOģ�����ŵ�����
		writel(readl(gpio_base+MSK_OE) | 1 << 7, gpio_base+MSK_OE);  //ȡ������ ʹ������


	led_class = class_create(THIS_MODULE, "yhai_dev_class");
	if (IS_ERR(led_class)) {
		printk("Err: failed in creating class.\n");
		goto err6;
	}

	/* register your own device in sysfs, and this will cause udevd to create corresponding device node */
	device_create(led_class,NULL, devno, NULL,"platled");     
		
	printk("platform led probe 1\n");
	return 0;

	//goto ʵ�� ˳�����룬�����ͷ�
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
	//���
	//ע���豸
	int  devno = MKDEV(LED_MA,LED_MI); 
	//˳�����룬�����ͷ�

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
MODULE_AUTHOR("chendu yhai"); //ģ����������(��ѡ)
MODULE_LICENSE("GPL"); //ģ����ѿ�Դ����








