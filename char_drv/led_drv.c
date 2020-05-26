
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


//#define GPIO_BASE 0xB8001000
//#define  GPEFUN0     0xB00000A0 
//#define  GPIOE_OMD   0xB8001040   //R/W  GPIO Port E Bit Output Mode Enable
//#define  GPIOE_PUEN  0xB8001044   //R/W  GPIO Port E Bit Pull-up Resistor 
//#define  GPIOE_DOUT  0xB8001048   //R/W  GPIO Port E Data Output Value  
//#define  GPIOE_PIN   0xB800104C   //R  GPIO Port E Pin Value 

static struct class *led_drv_class;
static struct class_device *led_drv_class_dev;


volatile unsigned long *GPEFUN0 =NULL;
volatile unsigned long *GPIOE_OMD =NULL;
volatile unsigned long *GPIOE_PUEN =NULL;
volatile unsigned long *GPIOE_DOUT =NULL;


static int led_drv_open(struct inode *inode,struct file *file)
{
     printk("led_drv_open\n");

      *GPEFUN0 &= ~ (0xFF << 16);//PE4,PE5配置为gpio
      *GPIOE_DOUT |= 0x30;       //输出高电平
      *GPIOE_PUEN |= 0x30;       //上拉使能
      *GPIOE_OMD |= 0x30;        //输出使能

    return 0;
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  copy_from_user(&val,buf,count);

// if(val==1)
//{
//     printk("led_on\n");
//     *GPIOE_DOUT &= ~ (0x01 << 4);// LED1   PE4   0
//}
//else
//{
//      printk("led_off\n");
//     *GPIOE_DOUT |= 0x10;// LED1   PE4   1 
//}


    switch (val)
   {
      case 1:{//led1亮
              printk("led1 on\n");
             *GPIOE_DOUT &= ~ (0x01 << 4);// LED1   PE4   0
              }
              break;
      case 2:{//led1灭
              printk("led1 off\n");
             *GPIOE_DOUT |= 0x10;// LED1   PE4   1 
              }
              break;
      case 3:{//led2亮
              printk("led2 on\n");
             *GPIOE_DOUT &= ~ (0x01 << 5);// LED2   PE5   0
             }
      break;
      case 4:{//led2灭
             printk("led2 off\n");
             *GPIOE_DOUT |= 0x20;// LED2   PE5   1 
             }
      break;

      default:*GPIOE_DOUT |= 0x30;//led1 led2 灭
    }


   return 0;
}

static struct file_operations led_div_operations = {
        .owner		= THIS_MODULE,
	.open		= led_drv_open,
	.write	        = led_drv_write,
};

int major;

int led_drv_init(void)
{
    major = register_chrdev(0, "led_drv", &led_div_operations);//注册
   
    printk("<1>\n   This is led_driver program.\n\n");
    led_drv_class = class_create(THIS_MODULE, "led_drv");
    led_drv_class_dev = device_create(led_drv_class, NULL, MKDEV(major, 0), NULL, "led");/* /dev/led */
   
  GPEFUN0 = (volatile unsigned long *)ioremap(0xB00000A0,4);
  GPIOE_OMD = (volatile unsigned long *)ioremap(0xB8001040,4);
  GPIOE_PUEN = (volatile unsigned long *)ioremap(0xB8001044,4);
  GPIOE_DOUT = (volatile unsigned long *)ioremap(0xB8001048,4);

    return 0;
}


void led_drv_exit(void)
{
    unregister_chrdev(major, "led_drv");//注册
	//device_unregister(led_drv_class_dev);
        device_destroy(led_drv_class, MKDEV(major, 0));
	class_destroy(led_drv_class);
      iounmap(GPEFUN0);
      iounmap(GPIOE_OMD);
      iounmap(GPIOE_PUEN);
      iounmap(GPIOE_DOUT);
}                                 

module_init(led_drv_init);
module_exit(led_drv_exit);


MODULE_LICENSE("GPL");

