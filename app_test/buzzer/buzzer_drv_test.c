

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc,char **argv)
{
	
	int fd1,fd2,i;
	int val = 1;

        fd1 = open("/dev/buzzer", O_RDWR);
	if (fd1 <0)printf("Cannot open /dev/buzzer\n");
	
        fd2 = open("/dev/modbus", O_RDWR);
	if (fd2 <0)printf("Cannot open /dev/modbus\n");

for(i=0;i<10;i++)
{
 val = 1;
 write(fd1, &val, 4);
 write(fd2, &val, 4);
 usleep(2000000);
 val = 0;
 write(fd1, &val, 4);	
 write(fd2, &val, 4);
 usleep(2000000);
}

		
	return 0;
}
