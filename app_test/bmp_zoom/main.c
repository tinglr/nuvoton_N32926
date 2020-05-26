#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <pic_operation.h>
#include <render.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


/* digitpic <bmp_file> */
int main(int argc, char **argv)
{
	int iFdBmp;
	int iRet;
	unsigned char *pucBMPmem;
	struct stat tBMPstat;
		
	PT_DispOpr ptDispOpr;

	extern T_PicFileParser g_tBMPParser;

	T_PixelDatas tPixelDatas;             // PixelDatas
	T_PixelDatas tPixelDatasSmall;

	T_PixelDatas tPixelDatasFB;

	if (argc != 2)
	{
		printf("%s <bmp_file>\n", argv[0]);
		return -1;
	}

	DebugInit();
	InitDebugChanel();

	DisplayInit();

	ptDispOpr = GetDispOpr("fb");
	ptDispOpr->DeviceInit();
	ptDispOpr->CleanScreen(0);
	

        /* 打开BMP文件 */
	iFdBmp = open(argv[1], O_RDWR);//打开bmp文件
	if (iFdBmp == -1)
	{
		DBG_PRINTF("can't open %s\n", argv[1]);
	}

	fstat(iFdBmp, &tBMPstat);//由文件描述词获得文件状态
	pucBMPmem = (unsigned char *)mmap(NULL , tBMPstat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFdBmp, 0);//图片bmp内存映射
	if (pucBMPmem == (unsigned char *)-1)
	{
		DBG_PRINTF("mmap error!\n");
		return -1;
	}

	/* 提取BMP文件的RGB数据, 缩放, 在LCD上显示出来 */
	iRet = g_tBMPParser.isSupport(pucBMPmem);  //判断是否支持bmp
	if (iRet == 0)
	{
		DBG_PRINTF("%s is not bmp file\n", argv[1]);
		return -1;		
	}

	tPixelDatas.iBpp = ptDispOpr->iBpp;
	iRet = g_tBMPParser.GetPixelDatas(pucBMPmem, &tPixelDatas);//获得bmp文件的结构体
	if (iRet)
	{
		DBG_PRINTF("GetPixelDatas error!\n");
		return -1;		
	}

	tPixelDatasFB.iWidth        = ptDispOpr->iXres; //320
	tPixelDatasFB.iHeight       = ptDispOpr->iYres; //240
	tPixelDatasFB.iBpp          = ptDispOpr->iBpp;  //16
	tPixelDatasFB.iLineBytes    = ptDispOpr->iXres * ptDispOpr->iBpp / 8; //640
	tPixelDatasFB.aucPixelDatas = ptDispOpr->pucDispMem;//	显存指针

	//printf("tPixelDatasFB.iLineBytes = %d \n", tPixelDatasFB.iLineBytes);
	PicMerge(0, 0, &tPixelDatas, &tPixelDatasFB);//显示的数据地址，显示的显存地址

	tPixelDatasSmall.iWidth  = tPixelDatas.iWidth/3;  //bmp宽度/4
	tPixelDatasSmall.iHeight = tPixelDatas.iHeight/3; //bmp高度/4
	tPixelDatasSmall.iBpp    = tPixelDatas.iBpp;      //16
	tPixelDatasSmall.iLineBytes = tPixelDatasSmall.iWidth * tPixelDatasSmall.iBpp / 8; //tPixelDatasSmall.iWidth8*2
	tPixelDatasSmall.aucPixelDatas = malloc(tPixelDatasSmall.iLineBytes * tPixelDatasSmall.iHeight);//分配这么多的空间
	
	PicZoom(&tPixelDatas, &tPixelDatasSmall);//缩小函数
	PicMerge(20,20, &tPixelDatasSmall, &tPixelDatasFB);//显示的数据地址，显示的显存地址
		
	return 0;
}

