#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/gpio/consumer.h>
#include <linux/device.h>
#include <linux/of.h>

int led_gpiod_probe(struct platform_device *pdev)
{	

	const char *name;
	unsigned int tmp;
	unsigned int array[3];
	struct device_node *np = pdev->dev.of_node;
	if(!of_property_read_string(np, "yh_addr", &name)){
		printk("yh_addr =%s\n",name);
	}
	if(!of_property_read_u32(np, "yh_age", &tmp)){
		printk("yh_age =%d\n",tmp);
	}
	
	if(!of_property_read_u32_array(np, "yh_info",array, 3)){
		printk("yh_infor[0] =%d\n",array[0]);
		printk("yh_infor[1] =%d\n",array[1]);		
		printk("yh_infor[2] =%x\n",array[2]);		
	}
    printk("led_gpiod_probe  ok\n");	
	return 0;
}

int led_gpiod_remove(struct platform_device *pdev)
{
	struct gpio_desc *gpiod = platform_get_drvdata(pdev);
	gpiod_set_value(gpiod, 0);
	gpiod_put(gpiod);
	printk("led_gpiod_remove  ok\n"); 

	return 0;
}



static
struct of_device_id ids[] = {
	{.compatible = "yhai,led_gpiod"},
	{},
};


static
struct platform_driver led_gpiod_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "led_gpiod_drv",
		.of_match_table = ids,
	},
	.probe = led_gpiod_probe,
	.remove = led_gpiod_remove,
};


module_platform_driver(led_gpiod_driver);
MODULE_LICENSE("GPL");





