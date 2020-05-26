
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/device.h>


//#define  GPIO_BASE   0xB8001000
//#define  GPDFUN0     0xB0000098 
//#define  GPIOD_OMD   0xB8001030   //R/W  GPIO Port D Bit Output Mode Enable
//#define  GPIOD_PUEN  0xB8001034   //R/W  GPIO Port D Bit Pull-up Resistor 
//#define  GPIOD_DOUT  0xB8001038   //R/W  GPIO Port D Data Output Value  
//#define  GPIOD_PIN   0xB800103C   //R  GPIO Port D Pin Value 


static struct class *buzzer_drv_class;
static struct class_device *buzzer_drv_class_dev;


volatile unsigned long *GPDFUN0 =NULL;
volatile unsigned long *GPIOD_OMD =NULL;
volatile unsigned long *GPIOD_PUEN =NULL;
volatile unsigned long *GPIOD_DOUT =NULL;


static int buzzer_drv_open(struct inode *inode,struct file *file)
{
  //   printk("buzzer_drv_open\n");


      *GPDFUN0 &= ~ (0xF << 16);          //PD4配置为gpio
      *GPIOD_DOUT &= ~ (0x01 << 4);       //输出低电平
      *GPIOD_PUEN |= 0x10;                //上拉使能
      *GPIOD_OMD |= 0x10;                 //输出使能


    return 0;
}

static ssize_t buzzer_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  copy_from_user(&val,buf,count);

 if(val==1)
{
  //   printk("buzzer_on\n");
    *GPIOD_DOUT |= 0x10;                 // modbus   PD3 1
}
else
{
  //    printk("buzzer_off\n");
     *GPIOD_DOUT &= ~ (0x01 << 4);       // 输出低电平  PD4
}



   return 0;
}

static struct file_operations buzzer_div_operations = {
        .owner		= THIS_MODULE,
	.open		= buzzer_drv_open,
	.write	        = buzzer_drv_write,
};

int major_buzzer;

int buzzer_drv_init(void)
{
    major_buzzer = register_chrdev(0, "buzzer_drv", &buzzer_div_operations);//注册
   
  //  printk("<1>\n   This is buzzer_driver program.\n\n");
    buzzer_drv_class = class_create(THIS_MODULE, "buzzer_drv");
    buzzer_drv_class_dev = device_create(buzzer_drv_class, NULL, MKDEV(major_buzzer, 0), NULL, "buzzer");/* /dev/led */
   
  GPDFUN0 = (volatile unsigned long *)ioremap(0xB0000098,4);
  GPIOD_OMD = (volatile unsigned long *)ioremap(0xB8001030,4);
  GPIOD_PUEN = (volatile unsigned long *)ioremap(0xB8001034,4);
  GPIOD_DOUT = (volatile unsigned long *)ioremap(0xB8001038,4);

    return 0;
}


void buzzer_drv_exit(void)
{
      unregister_chrdev(major_buzzer, "buzzer_drv");//注册
	//device_unregister(led_drv_class_dev);
      device_destroy(buzzer_drv_class, MKDEV(major_buzzer, 0));
      class_destroy(buzzer_drv_class);
      iounmap(GPDFUN0);
      iounmap(GPIOD_OMD);
      iounmap(GPIOD_PUEN);
      iounmap(GPIOD_DOUT);

}                                 

module_init(buzzer_drv_init);
module_exit(buzzer_drv_exit);


MODULE_LICENSE("GPL");

