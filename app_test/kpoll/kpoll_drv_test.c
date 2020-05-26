
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

int main(int argc,char **argv)
{	
	int fd,i;
	unsigned char key_val;
	int ret;

	struct pollfd fds[1];

        fd = open("/dev/kpoll", O_RDWR);
	if (fd <0)printf("Cannot open /dev/kpoll\n");

	fds[0].fd     = fd;
	fds[0].events = POLLIN;
	
	while(1)
	{
		ret = poll(fds, 1, 5000);
		if (ret == 0)
		{
			printf("time out\n");
		}
		else
		{
			read(fd, &key_val, 1); 
                          i++;
			printf("key_val %d = 0x%x\n", i,key_val);
		}

	 }

 	 return 0;
}
