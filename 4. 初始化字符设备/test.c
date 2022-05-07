#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>


int main(int argc, char **argv)
{
	int fd;
	fd = open("/dev/device_test", O_RDWR);
	if (fd < 0) {
		perror("Open Is Error");
		exit(1);
	}
	printf("open led ok\n");  //注意要加\n 否则打印信息可能没有

	return 0;	
	
}





