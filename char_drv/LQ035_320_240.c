
#include <linux/module.h>    //所有模块都需要这个头文件
#include <linux/kernel.h>    //声明了printk()这个内核态用的函数
#include <linux/fs.h>        //文件系统有关的，结构体file_operations在头文件 linux/fs.h中定义
#include <linux/init.h>     //init和exit相关宏
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/uaccess.h>   //copy_from_user  copy_to_user 
#include <asm/io.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/bitops.h>

#include <linux/io.h>
#include <linux/errno.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>     
#include <linux/miscdevice.h>// 在这个头文件中主要是misc(混合)设备注册和注销 linux中用struct miscdevice来描述一个混杂设备
#include <linux/input.h>
#include <linux/poll.h>
#include <linux/interrupt.h>  //request_irq
#include <mach/w55fa92_reg.h>
#include <mach/w55fa92_keypad.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>


static int LQ035_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info);


static struct fb_ops LQ035_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= LQ035_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static struct fb_info *LQ035_lcd;
static u32 pseudo_palette[16];//虚假调色板
static unsigned int clock_div;

/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int LQ035_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}


static int lcd_init(void)
{
       printk("tinglr open\n");
	/* 1. 分配一个fb_info */
	LQ035_lcd = framebuffer_alloc(0, NULL);//不需要额外空间

	/* 2. 设置 */
	/* 2.1 设置固定的参数 */
	strcpy(LQ035_lcd->fix.id, "LQ035_lcd");
	LQ035_lcd->fix.smem_len = 320*240 *16/8;
	LQ035_lcd->fix.type     = FB_TYPE_PACKED_PIXELS;//默认支持大部分屏
	LQ035_lcd->fix.visual   = FB_VISUAL_TRUECOLOR; /* TFT 真彩色 */
	LQ035_lcd->fix.line_length = 320*2;//一行320×2个字节
	
	/* 2.2 设置可变的参数 */
	LQ035_lcd->var.xres           = 320;
	LQ035_lcd->var.yres           = 240;
	LQ035_lcd->var.xres_virtual   = 320;//虚拟分辨率
	LQ035_lcd->var.yres_virtual   = 240;
	LQ035_lcd->var.bits_per_pixel = 16;//每个像素用565

	/* RGB:565 */
	LQ035_lcd->var.red.offset     = 11;//红色从bit11开始
	LQ035_lcd->var.red.length     = 5; 
	
	LQ035_lcd->var.green.offset   = 5;//绿色比他5开始
	LQ035_lcd->var.green.length   = 6;

	LQ035_lcd->var.blue.offset    = 0;//蓝色bit0开始
	LQ035_lcd->var.blue.length    = 5;

	LQ035_lcd->var.activate       = FB_ACTIVATE_NOW;
	
	
	/* 2.3 设置操作函数 */
	LQ035_lcd->fbops              = &LQ035_lcdfb_ops;
	
	/* 2.4 其他的设置 */
	LQ035_lcd->pseudo_palette = pseudo_palette;//虚拟调色板
	//s3c_lcd->screen_base  = ;  /* 显存的虚拟地址 */ 
	LQ035_lcd->screen_size   = 240*320*16/8;

/* 3. 硬件相关的操作 */
	/* 3.1 配置GPIO用于LCD */
 	// enable VPOST function pins
writel((readl(REG_GPBFUN1) & ~MF_GPB15)|0x20000000, REG_GPBFUN1);  // enable  PB15为时钟clk LPCLK pin
	writel(0x22222222, REG_GPEFUN0); // enable PC DATA[7:0] pins
	writel(0x22222222, REG_GPCFUN1); // enable PC DATA[15:8] pins	
		
		
	writel((readl(REG_GPDFUN1) & 0xFFFFFF0F)|0x00000020, REG_GPDFUN1); 	// enable PD9 HS  

	writel((readl(REG_GPAFUN0) & 0xFFFFFF00)|0x00000044, REG_GPAFUN0); // enable PA1 VS  PA0 EN 
	//writel((readl(REG_GPAFUN0) & 0xFFFFFF0F)|0x00000040, REG_GPAFUN0); // enable PA1 VS  PA0 EN 


	
	
	/* 3.2 根据LCD手册设置LCD控制器, 比如VCLK的频率等 */
	
	 writel(VPOST_RST, REG_AHBIPRST);  
	 writel(0, REG_AHBIPRST);

	clock_div = 36;       //240M/（36+1）=6.4M
	clock_div &= 0xFF;           
//	nvt_lock();		
	writel(readl(REG_CLKDIV1) & ~VPOST_N0, REG_CLKDIV1); // NVTBIT(2, 0) 0分频
	writel((readl(REG_CLKDIV1) & ~VPOST_N1)|(clock_div << 8), REG_CLKDIV1); //NVTBIT(15, 8)  分频/（36+1）  
	writel((readl(REG_CLKDIV1) & ~VPOST_S)|(3<<3), REG_CLKDIV1); 	//NVTBIT(4, 3)  // VPOST clock from UPLL	
	
//	nvt_unlock();	

     writel(readl(REG_LCM_LCDCPrm) & ~LCDCPrm_LCDSynTv, REG_LCM_LCDCPrm); //  BIT8清0  disable async with TV
     writel(readl(REG_LCM_LCDCPrm) & ~LCDCPrm_LCDTYPE, REG_LCM_LCDCPrm);  //NVTBIT(1,0)清0 高分辨率模式 并行输出数据 ok
	 writel(readl(REG_LCM_LCDCCtl) & ~LCDCCtl_PRDB_SEL, REG_LCM_LCDCCtl); //NVTBIT(21,20) PAL类型选择00 16-bit mode

	 writel((readl(REG_LCM_TVCtl) & ~TVCtl_LCDSrc)|(1 << 10), REG_LCM_TVCtl); // NVTBIT(11,10)   01 = Frame Buffer  
	 writel(readl(REG_LCM_TVCtl) & ~TVCtl_TV_D1, REG_LCM_TVCtl);              // 0 = 640x480 (VGA)
	 writel((readl(REG_LCM_TVCtl) & 0xFFFFFFFE)|0x00000010, REG_LCM_TVCtl);   // disable

	writel(0x1E026014, REG_LCM_TCON1);
	// Horizontal sync pulse width  水平同步脉冲宽度 通过计数LCD的数量来确定HSYNC脉冲的高电平宽度 像素时钟 0x1E=30   
	// Horizontal back porch        水平后廊       HSYNC的下降沿和起始点之间的LCD像素时钟周期数 的活动数据。0X026=38
	// Horizontal front porch       水平前廊       活动数据结束与上升之间的LCD像素时钟周期数  HSYNC的边缘。 0X014=20
	
	writel(0x00030F04, REG_LCM_TCON2);	
	//Vertical sync pulse width  垂直同步脉冲宽度  通过计算不活动的数量来确定VSYNC脉冲的高电平宽度线。 3
	//Vertical back porch   垂直后廊 在垂直同步之后的帧起始处的非活动行数周期 15
	//Vertical front porch   垂直前廊 在垂直同步之前，帧结束处的不活动行数期。4
	
    writel(0x013F00EF, REG_LCM_TCON3);	// 像素319x239  PPL*LPP
	
    writel(0x013F00EF, REG_LCM_FB_SIZE);	// 帧缓冲区大小设置 （319+1）x（239+1）   	   	
  //outl(0x013F00EF, REG_LCM_SCO_SIZE);	    // 缩放输出不缩放（319+1）x（239+1）  ??????????????????????????????
  //outl(inl(REG_LCM_LCDCCtl) | 0x20, REG_LCM_LCDCCtl); //缩放启用             ????????????????////????
  
    writel(0x14000002, REG_LCM_TCON4);
	//总MPU主动像素数（LCDSynTv = 0） 0x140=320
	//MPU Vsync脉冲宽度 0
	//MPU类型LCD  MARK极性等待FMARK输入为高电平触发帧起始    0
	//MPU类型LCD Vsync极性 同步脉冲低电平有效         0
	//Sync 类型LCD VSYNC极性低电平有效   0
	//Sync 类型LCD HSYNC极性低电平有效   0
	//Sync 类型LCD  VDEN极性高电平有效   1
	//Sync 类型LCD  像素时钟极性   输出视频数据和信号由像素时钟的下降沿释放	 0

     /* 3.3 分配显存(framebuffer), 并把地址告诉LCD控制器 */
       LQ035_lcd->screen_base = dma_alloc_writecombine(NULL, LQ035_lcd->fix.smem_len, &LQ035_lcd->fix.smem_start, GFP_KERNEL);

	  writel(LQ035_lcd->fix.smem_start, REG_LCM_FSADDR); //将缓存地址给控制器



     /* 启动LCD */
	writel((readl(REG_LCM_LCDCCtl) & 0xFFFEFFF0)|0x10003, REG_LCM_LCDCCtl); 
	//LITTLE-ENDIAN（小字节序、低字节序）,即低位字节排放在内存的低地址端，高位字节排放在内存的高地址端。 
	//帧缓冲区数据选择   RGB565
	//LCD Controller Run 


	/* 4. 注册 */
      printk("tinglr open\n");
	register_framebuffer(LQ035_lcd);
	return 0;
}

static void lcd_exit(void)
{
	unregister_framebuffer(LQ035_lcd);
        writel(readl(REG_LCM_LCDCCtl) & 0xFFFEFFFE, REG_LCM_LCDCCtl); 
	dma_free_writecombine(NULL, LQ035_lcd->fix.smem_len, LQ035_lcd->screen_base, LQ035_lcd->fix.smem_start);
	framebuffer_release(LQ035_lcd);
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");

