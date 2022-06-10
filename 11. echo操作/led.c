#include <linux/init.h>                        
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>

#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include <linux/platform_device.h>
#include <asm/io.h>


#include <dt-bindings/gpio/gpio.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif


int  led_gpio;
static ssize_t led_store(struct device *dev, struct device_attribute *attr,
              const char *buf, size_t count) {
              
    //写文件,控制 gpio 输出 （ echo 1 >   led_gpio)                
    if (buf[0] == '0') {
        gpio_direction_output(led_gpio, 0);
    }else if (buf[0] == '1') {
        gpio_direction_output(led_gpio, 1);
    }    

    printk(KERN_ERR "led_gpio_store %c \n",buf[0]);   
    return count;
}



ssize_t led_show(struct device *dev, struct device_attribute *attr,char *buf){  
	printk("led_show go \n");	
 	return 0;
}



static struct device_attribute dev_attr_file = {    
    .attr = {        
       .name = "led_gpio",        
       .mode = (S_IRUGO | S_IWUSR)  
    },   
    .store = led_store,
    .show = led_show,  //如果无需读的功能，可设为NULL,  且删除前面的S_IRUGO
};



int leds_probe(struct platform_device *pdev)
{
   int  ret = 0;
	
   enum of_gpio_flags flags;

  //获取设备树节点的属性 "led_gpio"
   led_gpio = of_get_named_gpio_flags(pdev->dev.of_node, "led_gpio", 0, &flags);
   if (gpio_is_valid(led_gpio))
   {
      ret = devm_gpio_request(&pdev->dev,led_gpio, "led_gpio"); //申请使用gpio(如果被占用，将申请失败)
      if (ret) {
	  printk("Failed to get led_gpio gpio.\n");
	  return -1;
       } 
   }

   //根据设备树节点属性，创建相应的 属性文件 /sys/devices/sys_rw_led/led_gpio
   device_create_file(&pdev->dev, &dev_attr_file);  // /device_create_file 里面是调用了 sysfs_create_file
   printk("leds_probe 1 ok\n");
   return 0;
}



int leds_remove(struct platform_device *pdev)
{   
    device_remove_file(&pdev->dev, &dev_attr_file);
    printk("leds_remove  ok\n");    
    return 0;
}


static const struct of_device_id of_led_match[] = {
    { .compatible = "sys_rw_led", },
    {},
};

MODULE_DEVICE_TABLE(of, of_led_match);

struct platform_driver leds_drv = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "sys_rw_led driver",
		.of_match_table = of_led_match,
	},
	.probe = leds_probe,
	.remove = leds_remove,
};





module_platform_driver(leds_drv);
MODULE_LICENSE("GPL");






