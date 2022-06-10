#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/timer.h> 

struct timer_list s_timer; //�������ʱ��
int counter; //�����

//��ʱ����ʱ �ص�����
static void second_timer_handle(unsigned long arg)
{
	mod_timer(&s_timer,jiffies + HZ); //���賬ʱֵ
	counter++;

	printk("second count = %d \n", counter);
}



static int timer_init(void)
{
	//��ʼ����ʱ��
	init_timer(&s_timer);
	s_timer.function = second_timer_handle; //���ó�ʱ�ص�����
	s_timer.expires = jiffies + HZ;  //���ó�ʱֵ
	             //��ʱֵ��һ��jiffiesֵ����jiffiesֵ���ڳ�ʱֵtimer->expiresʱ��timer->function�����ͻᱻ����

	add_timer(&s_timer); //��ӣ�ע�ᣩ��ʱ��
	counter = 0; //������0	
        printk("timer_init ok\n");
        return 0;
}

static void timer_exit(void)
{
	del_timer(&s_timer);//ɾ����ʱ��
        printk("timer_exit \n");
}

module_init(timer_init); 
module_exit(timer_exit); 
MODULE_LICENSE("GPL"); 