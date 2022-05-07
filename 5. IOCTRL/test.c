#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define LED_MAGIC 'L'  //����

#define LED_ON	_IOW(LED_MAGIC, 2, int)   //�û������ܿ�������
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
		printf("open led ok\n");  //ע��Ҫ��\n �����ӡ��Ϣ����û��
		while(operTimers--){
			ioctl(fd, LED_ON);  //���Ϳ������� LED_ON
			usleep(100000);
			ioctl(fd, LED_OFF); //���Ϳ������� LED_OFF
			usleep(100000);	
		}
	}
	return 0;	
}





