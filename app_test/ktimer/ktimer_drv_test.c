
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>


int fd;

void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val: 0x%x\n", key_val);
}

int main(int argc,char **argv)
{	
	int Oflags;
	unsigned char key_val;
	int ret;

//	signal(SIGIO, my_signal_fun);

        fd = open("/dev/ktimer", O_RDWR);
	if (fd <0)printf("Cannot open /dev/ktimer\n");

	
	while(1)
	{
	
	ret = read(fd, &key_val, 1);
	printf("key_val: 0x%x, ret = %d\n", key_val, ret);

	}

 	 return 0;
}
