#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/gpio/consumer.h>

int led_gpiod_probe(struct platform_device *pdev)
{	
	struct gpio_desc *gpiod = NULL;
	gpiod = gpiod_get(&pdev->dev, NULL, GPIOD_OUT_HIGH);
	if(NULL == gpiod){
          printk("gpiod_get err\n");
          return -ENODEV;		
	}
	gpiod_direction_output(gpiod,1);
	platform_set_drvdata(pdev,gpiod);
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





