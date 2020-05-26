
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>


//#include <linux/i2c-dev.h>


/* i2c_test r addr
 * i2c_test w addr val
 */



int main(int argc, char **argv)
{
	int fd,x,y;
	unsigned char buf[10];

	fd = open("/dev/gt5663", O_RDWR);
	if (fd < 0)
	{
		printf("can't open /dev/gt5663\n");
		return -1;
	}

        while(1){
		read(fd, buf, 6);
                x= buf[2]+ (buf[3]*256);y=buf[4]+ (buf[5]*256);
		printf("data: 0x%2x,0x%2x,(%d,%d)\n", buf[0], buf[1], x, y);
                //  printf("data: %c,%c,%c,%c\n", buf[0], buf[1], buf[2], buf[3]);
                usleep(100000);

           }

}


