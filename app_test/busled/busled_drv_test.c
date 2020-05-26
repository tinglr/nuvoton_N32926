

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

        fd = open("/dev/busled", O_RDWR);
	if (fd <0)printf("Cannot open /dev/led\n");
	

for(i=0;i<10;i++)
{
 val = 1;
 write(fd, &val, 4);
// usleep(1000000);
 sleep(1);
 val = 3;
 write(fd, &val, 4);	
// usleep(1000000);
 sleep(1);
 val = 2;
 write(fd, &val, 4);
 // usleep(1000000);
 sleep(1);
 val = 4;
 write(fd, &val, 4);	
 //usleep(1000000);
 sleep(1);

}

		
	return 0;
}
