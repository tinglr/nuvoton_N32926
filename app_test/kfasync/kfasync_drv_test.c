
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

	signal(SIGIO, my_signal_fun);

        fd = open("/dev/kfasync", O_RDWR);
	if (fd <0)printf("Cannot open /dev/kfasync\n");

	fcntl(fd, F_SETOWN, getpid());//告诉内核发给谁
	
	Oflags = fcntl(fd, F_GETFL); 
	
	fcntl(fd, F_SETFL, Oflags | FASYNC);//改变fasync标记，最终会调用到驱动的.fasync初始化/释放结构体fasync_struct
	
	while(1)
	{
	 sleep(1000);
	}

 	 return 0;
}
