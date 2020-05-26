
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc,char **argv)
{
	
	int fd,i;
	char val = 1;

        fd = open("/dev/backlight", O_RDWR);
	if (fd <0)printf("Cannot open /dev/backlight\n");
	
usleep(1000000);
for(i=0;i<5;i++)
{
 val=0;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
 val=100;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
 val=99;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
 val=1;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
 val=0;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
 val=50;write(fd, &val, 1);sleep(2);
 val=101;write(fd, &val, 1);sleep(2);
}

		
	return 0;
}
