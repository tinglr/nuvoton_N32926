
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>


int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic)//原始结构体和变换后结构体
{
    unsigned long dwDstWidth = ptZoomPic->iWidth;//变换后宽度
    unsigned long* pdwSrcXTable = malloc(sizeof(unsigned long) * dwDstWidth);//创建320*4个字节的内存
	unsigned long x;
	unsigned long y;
	unsigned long dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;
	unsigned long dwPixelBytes = ptOriginPic->iBpp/8;//每个像素2个字节

	if (ptOriginPic->iBpp != ptZoomPic->iBpp)
	{
		return -1;
	}
	
    for (x = 0; x < dwDstWidth; x++)//生成表 pdwSrcXTable  转变后的宽度
    {
        pdwSrcXTable[x]=(x*ptOriginPic->iWidth/ptZoomPic->iWidth);
    }

    for (y = 0; y < ptZoomPic->iHeight; y++)
    {			
        dwSrcY = (y * ptOriginPic->iHeight / ptZoomPic->iHeight);

		pucDest = ptZoomPic->aucPixelDatas + y*ptZoomPic->iLineBytes;
		pucSrc  = ptOriginPic->aucPixelDatas + dwSrcY*ptOriginPic->iLineBytes;
		
        for (x = 0; x <dwDstWidth; x++)
        {
            /* 原图座标: pdwSrcXTable[x]，srcy
             * 缩放座标: x, y
			 */
			 memcpy(pucDest+x*dwPixelBytes, pucSrc+pdwSrcXTable[x]*dwPixelBytes, dwPixelBytes);
        }
    }

    free(pdwSrcXTable);
	return 0;
}

