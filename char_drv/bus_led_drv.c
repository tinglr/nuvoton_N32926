
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>   //copy_from_user  copy_to_user 
#include <asm/io.h>
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

static struct class *busled_drv_class;
//static struct class_device *busled_drv_class_dev;

static int major;


static volatile unsigned long *GPEFUN0 =NULL;
static volatile unsigned long *GPIOE_OMD =NULL;
static volatile unsigned long *GPIOE_PUEN =NULL;
static volatile unsigned long *GPIOE_DOUT =NULL;
static int pin;


static int busled_drv_open(struct inode *inode,struct file *file)
{
     printk("led_drv_open\n");

      *GPEFUN0 &= ~ (0xFF << 16);//PE4,PE5配置为gpio
      *GPIOE_DOUT |= 0x30;       //输出高电平
      *GPIOE_PUEN |= 0x30;       //上拉使能
      *GPIOE_OMD |= 0x30;        //输出使能

    return 0;
}

static ssize_t busled_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
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




static struct file_operations busled_div_operations = {
        .owner		= THIS_MODULE,
	.open		= busled_drv_open,
	.write	        = busled_drv_write,
};



static int busled_probe(struct platform_device *pdev)
{
	struct resource		*res;

	/* 根据platform_device的资源进行ioremap */
	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	GPEFUN0 = ioremap(res->start, res->end - res->start + 1);
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	GPIOE_OMD = ioremap(res->start, res->end - res->start + 1);
	
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	GPIOE_PUEN = ioremap(res->start, res->end - res->start + 1);
	
	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	GPIOE_DOUT = ioremap(res->start, res->end - res->start + 1);
	
	res = platform_get_resource(pdev, IORESOURCE_BUS, 0);
	pin = res->start;

	/* 注册字符设备驱动程序 */

    major = register_chrdev(0, "busled_drv", &busled_div_operations);//注册
   
    printk("<1>\n   This is busled_driver program.\n\n");
    busled_drv_class = class_create(THIS_MODULE, "busled_drv");
    device_create(busled_drv_class, NULL, MKDEV(major, 0), NULL, "busled");/* /dev/busled */
	
	return 0;
}

static int busled_remove(struct platform_device *pdev)
{
	/* 卸载字符设备驱动程序 */
	/* iounmap */
	printk("busled_remove, remove led\n");

    unregister_chrdev(major, "busled_drv");//注册
	//device_unregister(led_drv_class_dev);
        device_destroy(busled_drv_class, MKDEV(major, 0));
	class_destroy(busled_drv_class);
      iounmap(GPEFUN0);
      iounmap(GPIOE_OMD);
      iounmap(GPIOE_PUEN);
      iounmap(GPIOE_DOUT);
	
	return 0;
}

struct platform_driver busled_drv = {
	.probe		= busled_probe,
	.remove		= busled_remove,
	.driver		= {
		.name	= "busled",
	}
};



int busled_drv_init(void)
{
 platform_driver_register(&busled_drv);

    return 0;
}


void busled_drv_exit(void)
{
platform_driver_unregister(&busled_drv);
}                                 

module_init(busled_drv_init);
module_exit(busled_drv_exit);


MODULE_LICENSE("GPL");

