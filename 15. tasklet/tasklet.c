#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/interrupt.h> 
#include <linux/interrupt.h>
#include <linux/delay.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif


struct key_dev{ //定义设备的类
  struct device *dev;
  const char *name; 
  unsigned int gpio_num;
  unsigned int irq_num;	
};


void tasklet_handler(unsigned long data)
{
	printk("tasklet_handler start  delay work =%d\n",data);
	mdelay(5000); //注：不能用msleep,   tasklet 运行于中断上下文，不能执行会睡眠函数 
	printk("tasklet_handler end delay work \n");
}

struct tasklet_struct tasklet;


irqreturn_t key_interrupt(int irqno, void *devid) 
{
  pr_info("do interrupt irqno = %d\n", irqno);
  tasklet_schedule(&tasklet);
  return IRQ_HANDLED; 
}

int keys_probe(struct platform_device *pdev)
{
  int  ret = -1;
  struct key_dev *key;  //创建设备对象

  enum of_gpio_flags flags;
  tasklet_init(&tasklet, tasklet_handler, 100);
  key = devm_kzalloc(&pdev->dev, sizeof(*key), GFP_KERNEL); /*为设备对象分配空间
    函数 devm_kzalloc() 和kzalloc()一样都是内核内存分配函数，但是devm_kzalloc()是跟设备(device)有关的，当设备(device)被detached或者驱动(driver)卸载(unloaded)时，内存会被自动释放。
    当然，当内存不再使用时，可以使用函数devm_kfree()释放。
     而kzalloc() 必须手动释放(配对使用kfree())，但如果工程师检查不仔细，则有可能造成内存泄漏
     devm_kzalloc 有在统一设备模型的设备树记录空间，有自动释放的第二道防线，更安全。
     如 在keys_remove中 不加 devm_kfree，但再probe中途异常要加
     */
   key->dev = &pdev->dev;
   key->name ="yhai key";
	
   key->gpio_num = of_get_named_gpio_flags((*key->dev).of_node, "key_gpio", 0, &flags); //获取设备树节点的属性key_gpio
	
  ret = devm_gpio_request(key->dev,key->gpio_num, "gpio_num");  //申请gpio的使用
	    //devm是为统一设备模型里 提供的api，尽量用这一套，更统一安全
  key->irq_num=gpio_to_irq(key->gpio_num); //根据IO编号获取其中断号(注：这里是软件编号只是为了区分，不是硬件芯片手册上的编号)

  ret = devm_request_irq(key->dev, 
                    key->irq_num,  //中断号(用于区分不同中断) 
                    key_interrupt,  //中断处理函数
                    IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, //中断标志  上升沿或下降沿触发
                    key->name,  //中断名称  在cat /proc/interrupts  克看到
                    key); // 共享中断时才用(共享一中断线, 用于标识中断处理程序，中断标志需设为IRQF_SHARED)                           

  pr_notice("keys_probe ok \n");
  return 0;
	
  //异常处理
err_devm_request_irq:	
  devm_gpio_free(key->dev,key->gpio_num);
err_devm_gpio_request:
err_of_get_named_gpio_flags:
  devm_kfree(key->dev,key);  //释放设备空间
err_devm_kzalloc:
  return ret;	
}

int keys_remove(struct platform_device *pdev)
{   
  //因devm分配的资源，在rmmod时会自动释放，故不用再加下面的释放函数
  //devm_free_irq(&pdev->dev,irq_num,NULL);	 //释放中断
  //devm_gpio_free(&pdev->dev,gpio_num); //释放gpio管脚
  printk("keys_remove  ok 1\n");
	tasklet_kill(&tasklet);  
  return 0;
}

static const struct of_device_id of_key_match[] = {
    { .compatible = "yhai_key_irq", },  //匹配设备树的ID
    {},
};

MODULE_DEVICE_TABLE(of, of_key_match);

struct platform_driver keys_drv = {
   .driver = {
     .owner = THIS_MODULE,
     .name = "yhai_key_irq driver" ,
     .of_match_table = of_key_match,
   },
   .probe = keys_probe,
   .remove = keys_remove,    
};

module_platform_driver(keys_drv);
MODULE_LICENSE("GPL");





























