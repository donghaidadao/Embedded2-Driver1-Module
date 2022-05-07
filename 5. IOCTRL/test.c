#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define LED_MAGIC 'L'  //幻数

#define LED_ON	_IOW(LED_MAGIC, 2, int)   //用幻数加密控制命令
#define LED_OFF	_IOW(LED_MAGIC, 3, int)


int main(int argc, char **argv)
{
	int fd;
	int operTimers = 10;
	fd = open("/dev/ioctrl", O_RDWR);
	if (fd < 0) {
		perror("Open Is Error");
		exit(1);
	}else{
		printf("open led ok\n");  //注意要加\n 否则打印信息可能没有
		while(operTimers--){
			ioctl(fd, LED_ON);  //发送控制命令 LED_ON
			usleep(100000);
			ioctl(fd, LED_OFF); //发送控制命令 LED_OFF
			usleep(100000);	
		}
	}
	return 0;	
}





