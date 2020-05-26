/****************************************************************************
 *                                                                          *
 * Copyright (c) 2008 Nuvoton Technology Corp. All rights reserved.         *
 *                                                                          *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     lcm.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file is a LCM sample program
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
 *     2011/05/01		 Ver 1.0 Created by PX40 MHKuo
 *
 * REMARK
 *     None
 **************************************************************************/

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

#define LCD_ENABLE_INT		_IO('v', 28)
#define LCD_DISABLE_INT		_IO('v', 29)

static struct fb_var_screeninfo var;

//typedef struct Cursor
//{
//	unsigned char x;
//	unsigned char y;
//}Cursor;

int main()
{
	int fd;
//	int i, t = 0;
//	Cursor cur;		
	FILE *fpVideoImg;
	unsigned char *pVideoBuffer;
	unsigned long uVideoSize;
//	cur.x = cur.y = 10;	
	fd = open("/dev/fb1", O_RDWR);//设备描述符=打开设备fb0  读写功能打开
	if (fd == -1)
	{
		printf("Cannot open fb1!\n");//不能打开fb0
		return -1;
	}
	if (ioctl(fd, FBIOGET_VSCREENINFO, &var) < 0) { 
		perror("ioctl FBIOGET_VSCREENINFO");         //读取fb的信息
		close(fd);
		return -1;
	}

	uVideoSize = var.xres * var.yres * var.bits_per_pixel / 8;   //480*272*(16/8)=255k
	printf("uVideoSize = 0x%x\n", uVideoSize);
	printf("var.xres = 0x%x\n", var.xres);
	printf("var.yres = 0x%x\n", var.yres);	
	
	pVideoBuffer = mmap(NULL, uVideoSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);//PVideobuffer内存映射成功
	
	printf("pVideoBuffer = 0x%x\n", pVideoBuffer);
	if(pVideoBuffer == MAP_FAILED)
	{
		printf("LCD Video Map Failed!\n");
		exit(0);
	}	
		fpVideoImg = fopen("11_320x240.dat", "r");
		if(fpVideoImg == NULL){printf("open Image FILE fail !! \n");exit(0); }  
		if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
	        ioctl(fd,LCD_ENABLE_INT);//打开lcd
 		printf("Image File1\n");
		usleep(1000000);

		fpVideoImg = fopen("22_320x240.dat", "r");
		if(fpVideoImg == NULL) {printf("open Image FILE fail !! \n");exit(0);}  
		if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);
		printf("Image File2\n");
		usleep(1000000);usleep(1000000);

		fpVideoImg = fopen("33_320x240.dat", "r"); 
		if(fpVideoImg == NULL){printf("open Image FILE fail !! \n");exit(0);}  
		if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);
                printf("Image File3\n");
		usleep(1000000);

		fpVideoImg = fopen("44_320x240.dat", "r");	
	 	if(fpVideoImg == NULL) {printf("open Image FILE fail !! \n");exit(0);}  
	    	if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);
                printf("Image File4\n");
		usleep(1000000);

		fpVideoImg = fopen("55_320x240.dat", "r");
		if(fpVideoImg == NULL){printf("open Image FILE fail !! \n");exit(0); }  
	    	if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);	
		printf("Image File5\n");
		usleep(1000000);

		fpVideoImg = fopen("66_320x240.dat", "r");
		if(fpVideoImg == NULL){printf("open Image FILE fail !! \n");exit(0);}  
	    	if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);	
		printf("Image File6\n");
		usleep(1000000);

		fpVideoImg = fopen("77_320x240.dat", "r");
		if(fpVideoImg == NULL) {printf("open Image FILE fail !! \n");exit(0);}  
	    	if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);	
		printf("Image File7\n");
		usleep(1000000);

		fpVideoImg = fopen("88_320x240.dat", "r");
		if(fpVideoImg == NULL){printf("open Image FILE fail !! \n");exit(0); }  
	    	if( fread(pVideoBuffer, uVideoSize, 1, fpVideoImg) <= 0){printf("Cannot Read the Image File!\n");exit(0);}
		ioctl(fd,LCD_ENABLE_INT);	
		printf("Image File8\n");
		usleep(1000000);

	//close(fd);
        while(1);	
	return 0;
}
