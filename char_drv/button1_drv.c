
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
#include <linux/device.h>     // linux/miscdevice.h  在这个头文件中主要是misc(混合)设备注册和注销 linux中用struct miscdevice来描述一个混杂设备
#include <linux/miscdevice.h>

#include <linux/poll.h>
#include <linux/interrupt.h>  //request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,      const char *name, void *dev);

#include <mach/w55fa92_reg.h>

//#define  GPIO_BASE   0xB8001000
//#define  GPEFUN0     0xB00000A0 
//#define  GPIOE_OMD   0xB8001040   //R/W  GPIO Port E Bit Output Mode Enable
//#define  GPIOE_PUEN  0xB8001044   //R/W  GPIO Port E Bit Pull-up Resistor 
//#define  GPIOE_DOUT  0xB8001048   //R/W  GPIO Port E Data Output Value  
//#define  GPIOE_PIN   0xB800104C   //R    GPIO Port E Pin Value 

static struct class *button1_drv_class;
//static struct class_device *button_drv_class_dev;

//volatile unsigned long *GPEFUN0    =NULL;
//volatile unsigned long *GPIOE_PUEN =NULL;
//volatile unsigned long *GPIOE_PIN  =NULL;
//volatile unsigned long *GPIOE_OMD  =NULL;

static int button1_drv_open(struct inode *inode,struct file *file)
{
       printk("button1_drv_open\n");

      //*GPEFUN0 &= ~ (0xF << 8);            //PE2配置为gpio
     // *GPIOE_PUEN &= ~ (0x01 << 2);        //上拉失能
   //   *GPIOE_OMD  &= ~ (0x01 << 2);        //输入使能

     writel(readl(REG_GPEFUN0) & ~(0x0F00), REG_GPEFUN0);  
     writel(readl(REG_GPIOE_PUEN) & ~(1 << 2), REG_GPIOE_PUEN); // pull-up
     writel(readl(REG_GPIOE_OMD) & ~(1 << 2), REG_GPIOE_OMD); // input

  

    return 0;
}

static ssize_t button1_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/* 返回4个引脚的电平 */
	unsigned char key_vals[1];

	int regval;

	if (size != sizeof(key_vals))
	return -EINVAL;

        regval = readl(REG_GPIOE_PIN);                

	key_vals[0] = (regval & (1<<2)) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));
	
	return sizeof(key_vals);

}

static struct file_operations button1_div_operations = {
        .owner		= THIS_MODULE,
	.open		= button1_drv_open,
	.read	        = button1_drv_read,
};

int major;

int button1_drv_init(void)
{
    major = register_chrdev(0, "button1_drv", &button1_div_operations);//注册
   
    printk("<1>\n   This is button1_driver program.\n\n");
    button1_drv_class = class_create(THIS_MODULE, "button1_drv");
 //   button_drv_class_dev = device_create(button_drv_class, NULL, MKDEV(major, 0), NULL, "button");/* /dev/led */
   device_create(button1_drv_class, NULL, MKDEV(major, 0), NULL, "button1");/* /dev/led */
  // GPEFUN0 = (volatile unsigned long *)ioremap(0xB00000A0,4);
   //GPIOE_OMD = (volatile unsigned long *)ioremap(0xB8001040,4);
   //GPIOE_PUEN = (volatile unsigned long *)ioremap(0xB8001044,4);
   //GPIOE_PIN = (volatile unsigned long *)ioremap(0xB800104C,4);

   return 0;
}


void button1_drv_exit(void)
{
      unregister_chrdev(major, "button1_drv");//注册
      //device_unregister(led_drv_class_dev);
      device_destroy(button1_drv_class, MKDEV(major, 0));
      class_destroy(button1_drv_class);
     // iounmap(GPEFUN0);
     // iounmap(GPIOE_OMD);
     // iounmap(GPIOE_PUEN);
     // iounmap(GPIOE_PIN);
}                                 

module_init(button1_drv_init);
module_exit(button1_drv_exit);


MODULE_LICENSE("GPL");

