#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/timer.h> 

struct timer_list s_timer; //定义秒表定时器
int counter; //秒计数

//定时器超时 回调函数
static void second_timer_handle(unsigned long arg)
{
	mod_timer(&s_timer,jiffies + HZ); //重设超时值
	counter++;

	printk("second count = %d \n", counter);
}



static int timer_init(void)
{
	//初始化定时器
	init_timer(&s_timer);
	s_timer.function = second_timer_handle; //设置超时回调函数
	s_timer.expires = jiffies + HZ;  //设置超时值
	             //超时值是一个jiffies值，当jiffies值大于超时值timer->expires时，timer->function函数就会被运行

	add_timer(&s_timer); //添加（注册）定时器
	counter = 0; //计数清0	
        printk("timer_init ok\n");
        return 0;
}

static void timer_exit(void)
{
	del_timer(&s_timer);//删除定时器
        printk("timer_exit \n");
}

module_init(timer_init); 
module_exit(timer_exit); 
MODULE_LICENSE("GPL"); 