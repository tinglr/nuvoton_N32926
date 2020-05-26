

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
        unsigned char key_vals[1]={0};


        fd = open("/dev/key", O_RDWR);
	if (fd <0)printf("Cannot open /dev/key\n");
	
//while(1)
//{
 read(fd, key_vals, sizeof(key_vals));

printf("key pressed:  %d\n", key_vals[0]);
while(1)
{;
 }


   return 0;
}
