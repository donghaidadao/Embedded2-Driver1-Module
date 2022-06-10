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


struct key_dev{ //�����豸����
  struct device *dev;
  const char *name; 
  unsigned int gpio_num;
  unsigned int irq_num;	
};


void tasklet_handler(unsigned long data)
{
	printk("tasklet_handler start  delay work =%d\n",data);
	mdelay(5000); //ע��������msleep,   tasklet �������ж������ģ�����ִ�л�˯�ߺ��� 
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
  struct key_dev *key;  //�����豸����

  enum of_gpio_flags flags;
  tasklet_init(&tasklet, tasklet_handler, 100);
  key = devm_kzalloc(&pdev->dev, sizeof(*key), GFP_KERNEL); /*Ϊ�豸�������ռ�
    ���� devm_kzalloc() ��kzalloc()һ�������ں��ڴ���亯��������devm_kzalloc()�Ǹ��豸(device)�йصģ����豸(device)��detached��������(driver)ж��(unloaded)ʱ���ڴ�ᱻ�Զ��ͷš�
    ��Ȼ�����ڴ治��ʹ��ʱ������ʹ�ú���devm_kfree()�ͷš�
     ��kzalloc() �����ֶ��ͷ�(���ʹ��kfree())�����������ʦ��鲻��ϸ�����п�������ڴ�й©
     devm_kzalloc ����ͳһ�豸ģ�͵��豸����¼�ռ䣬���Զ��ͷŵĵڶ������ߣ�����ȫ��
     �� ��keys_remove�� ���� devm_kfree������probe��;�쳣Ҫ��
     */
   key->dev = &pdev->dev;
   key->name ="yhai key";
	
   key->gpio_num = of_get_named_gpio_flags((*key->dev).of_node, "key_gpio", 0, &flags); //��ȡ�豸���ڵ������key_gpio
	
  ret = devm_gpio_request(key->dev,key->gpio_num, "gpio_num");  //����gpio��ʹ��
	    //devm��Ϊͳһ�豸ģ���� �ṩ��api����������һ�ף���ͳһ��ȫ
  key->irq_num=gpio_to_irq(key->gpio_num); //����IO��Ż�ȡ���жϺ�(ע��������������ֻ��Ϊ�����֣�����Ӳ��оƬ�ֲ��ϵı��)

  ret = devm_request_irq(key->dev, 
                    key->irq_num,  //�жϺ�(�������ֲ�ͬ�ж�) 
                    key_interrupt,  //�жϴ�����
                    IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, //�жϱ�־  �����ػ��½��ش���
                    key->name,  //�ж�����  ��cat /proc/interrupts  �˿���
                    key); // �����ж�ʱ����(����һ�ж���, ���ڱ�ʶ�жϴ�������жϱ�־����ΪIRQF_SHARED)                           

  pr_notice("keys_probe ok \n");
  return 0;
	
  //�쳣����
err_devm_request_irq:	
  devm_gpio_free(key->dev,key->gpio_num);
err_devm_gpio_request:
err_of_get_named_gpio_flags:
  devm_kfree(key->dev,key);  //�ͷ��豸�ռ�
err_devm_kzalloc:
  return ret;	
}

int keys_remove(struct platform_device *pdev)
{   
  //��devm�������Դ����rmmodʱ���Զ��ͷţ��ʲ����ټ�������ͷź���
  //devm_free_irq(&pdev->dev,irq_num,NULL);	 //�ͷ��ж�
  //devm_gpio_free(&pdev->dev,gpio_num); //�ͷ�gpio�ܽ�
  printk("keys_remove  ok 1\n");
	tasklet_kill(&tasklet);  
  return 0;
}

static const struct of_device_id of_key_match[] = {
    { .compatible = "yhai_key_irq", },  //ƥ���豸����ID
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





























