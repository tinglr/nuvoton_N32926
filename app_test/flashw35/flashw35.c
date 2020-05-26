
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>


static int g_iFdTextFile;

int main()
{	
	unsigned char Buffer[100]={
	0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
	0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
	0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,
	0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,
	0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,
	0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,
	0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35,0x35};


	g_iFdTextFile=open("thermostat.dat",O_WRONLY);
	if (0 > g_iFdTextFile)
	{
		printf("can't open text file %s\n");
		return -1;
	}

      	if(write(g_iFdTextFile, Buffer, 100) <= 0)printf("cannot write!\n");



	if(fsync (g_iFdTextFile) !=0)printf("cannot fsync!\n");

 	printf("fsync ok !\n");

	close (g_iFdTextFile);
	printf(" app end !\n");

	return 0;
}

