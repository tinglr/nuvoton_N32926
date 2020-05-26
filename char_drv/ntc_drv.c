
#include <linux/module.h>    //所有模块都需要这个头文件
#include <linux/kernel.h>    //声明了printk()这个内核态用的函数
#include <linux/fs.h>        //文件系统有关的，结构体file_operations在头文件 linux/fs.h中定义
#include <linux/init.h>      //init和exit相关宏
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/uaccess.h>     //copy_from_user  copy_to_user 
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
#include <linux/device.h>    //linux/miscdevice.h  在这个头文件中主要是misc(混合)设备注册和注销 linux中用struct miscdevice来描述一个混杂设备
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/interrupt.h>  
#include <mach/w55fa92_reg.h>



static struct class *ntc_drv_class;

static int ntc_drv_open(struct inode *inode,struct file *file)
{
// printk("ntc_drv_open\n");
 writel(readl(REG_APBCLK) | TOUCH_CKE, REG_APBCLK);      //TOUCH_CKE使能时钟 
 writel(readl(REG_CLKDIV5) & ~(TOUCH_N1 | TOUCH_S| TOUCH_N0), REG_CLKDIV5);//Fed to ADC clock need 12MHz=External clock  
 writel(readl(REG_APBIPRST) | TOUCHRST, REG_APBIPRST);   //TP复位
 writel(readl(REG_APBIPRST) & ~ TOUCHRST, REG_APBIPRST);
 writel(readl(REG_TP_CTL1) & ~(PD_Power|PD_BUF| ADC_SLEEP), REG_TP_CTL1);//13,14,15 PD_BUF);If REF_SEL!=0 PD_BUF==>1
 writel(readl(REG_TP_CTL1) | REF_SEL, REG_TP_CTL1);      //Vref reference voltage
 writel(0xFF, REG_TP_CTL2);                              //Delay 48 ADC
 return 0;
}

static ssize_t ntc_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    
	unsigned char vals[4];
	int regval=0;
        int i=0;
	if (size != sizeof(vals))
	return -EINVAL;

   writel((readl(REG_TP_CTL1) & ~IN_SEL) | (1<<16), REG_TP_CTL1);//通道1
   writel(readl(REG_TP_CTL1) | SW_GET, REG_TP_CTL1); 
   while(readl(REG_TP_CTL1)&SW_GET){i++;if(i>200)return -EINVAL;}
   regval = readl(REG_NORM_DATA);	                         //获得adc值
   vals[0] = (unsigned char)(regval >> 8);
   vals[1] = (unsigned char)regval;
//   printk("ntc 1  (1) tingl = %d  ( %d)\n",regval,i);
   i=0;
   writel((readl(REG_TP_CTL1) & ~IN_SEL) | (3<<16), REG_TP_CTL1);//通道3
   writel(readl(REG_TP_CTL1) | SW_GET, REG_TP_CTL1); 
   while(readl(REG_TP_CTL1)&SW_GET){i++;if(i>200)return -EINVAL;}
   regval = readl(REG_NORM_DATA);	                         //获得adc值
   vals[2] = (unsigned char)(regval >> 8);
   vals[3] = (unsigned char)regval;
//   printk("ntc 2  (2) = %d  ( %d)\n",regval,i);
            
   copy_to_user(buf, vals, sizeof(vals));
   return sizeof(vals);
}

static struct file_operations ntc_drv_operations = {
        .owner		= THIS_MODULE,
	.open		= ntc_drv_open,
	.read	        = ntc_drv_read,
};

int major_ntc;

int ntc_drv_init(void)
{
    major_ntc = register_chrdev(0, "ntc_drv", &ntc_drv_operations);//注册
//    printk("<1>\n   This is ntc_driver program.\n\n");
    ntc_drv_class = class_create(THIS_MODULE, "ntc_drv");
    device_create(ntc_drv_class, NULL, MKDEV(major_ntc, 0), NULL, "ntc");/* /dev/ntc */
    return 0;
}


void ntc_drv_exit(void)
{
      unregister_chrdev(major_ntc, "ntc_drv");//注册
      device_destroy(ntc_drv_class, MKDEV(major_ntc, 0));
      class_destroy(ntc_drv_class);
}                                 

module_init(ntc_drv_init);
module_exit(ntc_drv_exit);


MODULE_LICENSE("GPL");

