

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
        unsigned char key_vals[1];


        fd = open("/dev/button1", O_RDWR);
	if (fd <0)printf("Cannot open /dev/button1\n");
	
while(1)//for(i=0;i<2000;i++)
{
 read(fd, key_vals, sizeof(key_vals));
if (!key_vals[0] )
{
printf("key pressed:  %d\n", key_vals[0]);
		}
//usleep(10000);//10ms

}

   return 0;
}
