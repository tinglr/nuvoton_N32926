

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
	int val = 1;

        fd = open("/dev/modbusen", O_RDWR);
	if (fd <0)printf("Cannot open /dev/modbusen\n");
	

for(i=0;i<10;i++)
{
 val = 1;
 write(fd, &val, 4);
 usleep(1000000);
 val = 0;
 write(fd, &val, 4);	
 usleep(1000000);
}

		
	return 0;
}
