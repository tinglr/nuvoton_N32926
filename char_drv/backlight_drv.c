
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
#include <mach/w55fa92_reg.h>



static struct class *backlight_drv_class;

static int backlight_drv_open(struct inode *inode,struct file *file)
{
 	// printk("backlight_drv_open\n");
	 writel((readl(REG_CLKDIV5) & ~(PWM_N1|PWM_S|PWM_N0)) | ( 0x18 | (5 << 5) | 3), REG_CLKDIV5);// 240M/24=10M
	 writel(readl(REG_APBCLK) | PWM_CKE, REG_APBCLK);     // PWM Clock Enable

	 writel(PWMRST, REG_APBIPRST);                       // PWM reset 
	 writel(readl(REG_APBIPRST) & ~PWMRST, REG_APBIPRST);
	
	 writel(readl(REG_PCR) & ~(1 << ((1 << 3) + 2) ), REG_PCR);//反向器off
	 writel(readl(REG_PCR) | (1 <<((1 << 3) + 3)), REG_PCR);   //定时器1切换模式Toggle Mode         
	 writel((readl(REG_PPR) & ~CP0) | ((10 - 1) & 0xFF), REG_PPR); //10分频
	 writel((readl(REG_PWM_CSR) & ~(0x7 << 4 )) | (4 << 4), REG_PWM_CSR);//输入时钟不分频
	 writel((1000 - 1), REG_CNR1);         //周期为1000个时钟
	 writel((10 * 80 - 1), REG_CMR1);      //占空比默认80%
	
	 writel(readl(REG_POE) | (1 << 1), REG_POE);    //PWM timer 1 Output Enable Setup. 	
	 writel((readl(REG_GPDFUN1) & ~(0xF << 20)) | (0x1 << 20), REG_GPDFUN1);           //配置PD13为pwm1
	 writel(readl(REG_PCR) | (1 << (1 << 3) ), REG_PCR); //Timer 1 Enable   	

                       
 return 0;
}

static ssize_t backlight_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  char val;
  copy_from_user(&val,buf,count);

if(val>100){//printk("val>100\n")
		;}
else if(val==0){
		//printk("val=0 \n");
                writel(readl(REG_PCR) | (1 << 10), REG_PCR);//反向器on
		writel((1000 - 1), REG_CMR1);   
               }
else {
      writel(readl(REG_PCR) & ~(1 << 10), REG_PCR);//反向器off
      writel((10 * val - 1), REG_CMR1);   
    // printk("val= %d \n",val);
     }

   return 0;
}

static struct file_operations backlight_drv_operations = {
        .owner		= THIS_MODULE,
	.open		= backlight_drv_open,
	.write	        = backlight_drv_write,
};

int major_backlight;

int backlight_drv_init(void)
{
    major_backlight = register_chrdev(0, "backlight_drv", &backlight_drv_operations);//注册
  //  printk("<1>\n   This is backlight_driver program.\n\n");
    backlight_drv_class = class_create(THIS_MODULE, "backlight_drv");
    device_create(backlight_drv_class, NULL, MKDEV(major_backlight, 0), NULL, "backlight");/* /dev/ntc */
    return 0;
}


void backlight_drv_exit(void)
{
      unregister_chrdev(major_backlight, "backlight_drv");//注册
      device_destroy(backlight_drv_class, MKDEV(major_backlight, 0));
      class_destroy(backlight_drv_class);
}                                 

module_init(backlight_drv_init);
module_exit(backlight_drv_exit);


MODULE_LICENSE("GPL");

