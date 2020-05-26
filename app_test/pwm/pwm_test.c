#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int fd1,fd2,temp;

int main(int argc,char **argv)
{	
	unsigned char key_val[4];
	int ret;
	char val = 1;

        fd2 = open("/dev/backlight", O_RDWR);
	if (fd2 <0)
	{printf("Cannot open /dev/backlight\n");return 0;}

     //   usleep(1000000);
        fd1 = open("/dev/capture0", O_RDWR);
	if (fd1 <0)
	{printf("Cannot open /dev/capture\n");return 0;}


	while(1)
	{
	printf("ok\n");
        ret = read(fd1, &key_val, 4);
        temp=(key_val[0]<<8)+key_val[1];
	printf("tinglr key_val: 0x%x 0x%x 0x%x ret = %d\n", key_val[0],key_val[1],temp, ret);

	if(temp==0x9768){val=1;write(fd2, &val, 1);} //1
	if(temp==0x6798){val=5;write(fd2, &val, 1);}//2
	if(temp==0x4fb0){val=20;write(fd2, &val, 1);} //3
	if(temp==0xcf30){val=30;write(fd2, &val, 1);}//4
	if(temp==0xe718){val=50;write(fd2, &val, 1);} //5
	if(temp==0x857a){val=60;write(fd2, &val, 1);}//6
	if(temp==0xef10){val=70;write(fd2, &val, 1);} //7
	if(temp==0xc738){val=80;write(fd2, &val, 1);}//8
	if(temp==0xa55a){val=100;write(fd2, &val, 1);} //9
	if(temp==0xbd42){val=0;write(fd2, &val, 1);}//0

	}
 	 return 0;
}
