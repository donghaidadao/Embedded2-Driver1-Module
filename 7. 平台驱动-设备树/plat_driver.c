#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

struct resource *res1;
struct resource *res2;
static int plat_probe(struct platform_device *pdev)
{
    printk("match ok,plat_probe go\n");   
    res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0); /*��ƽ̨�豸���ȡ��Դ
       pdev: ָ��ƽ̨�豸��ָ��
       IORESOURCE_MEM:��Դ����
       0: ��Դ���
    */
    if (res1 == NULL) {
	printk("res1 platform_get_resource fail \n");
	return -ENODEV;
    }
    printk("res1->start=%llx\n",res1->start);

    res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (res2 == NULL) {
	printk("res2 platform_get_resource fail\n");
	return -ENODEV;
    }
    printk("res2->start=%llx\n",res2->start);
    return 0;
}

static int plat_remove(struct platform_device *pdev)
{
    printk("plat_remove go\n");
    return 0;
}

static const struct of_device_id of_plat_match[] = {
		{.compatible = "yhai-plat",},
		{},
};


MODULE_DEVICE_TABLE(of, of_plat_match);

static struct platform_driver plat_driver = {
	.driver = {
		.name = "yhai_plat driver",
		.owner = THIS_MODULE,
		.of_match_table = of_plat_match,	
	},
	.probe = plat_probe,
	.remove = plat_remove,

};





module_platform_drive(plat_driver)
MODULE_LICENSE("GPL");