
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
//#define  GPEFUN0     0xB00000A0 
//#define  GPIOE_OMD   0xB8001040   //R/W  GPIO Port E Bit Output Mode Enable
//#define  GPIOE_PUEN  0xB8001044   //R/W  GPIO Port E Bit Pull-up Resistor 
//#define  GPIOE_DOUT  0xB8001048   //R/W  GPIO Port E Data Output Value  
//#define  GPIOE_PIN   0xB800104C   //R  GPIO Port E Pin Value 

static struct class *modbus_drv_class;
static struct class_device *modbus_drv_class_dev;


volatile unsigned long *GPEFUN0 =NULL;
volatile unsigned long *GPIOE_OMD =NULL;
volatile unsigned long *GPIOE_PUEN =NULL;
volatile unsigned long *GPIOE_DOUT =NULL;


static int modbus_drv_open(struct inode *inode,struct file *file)
{
   //  printk("modbus_drv_open\n");

      *GPEFUN0 &= ~ (0xF << 12);//PE3配置为gpio
      *GPIOE_DOUT &= ~ (0x01 << 3);       //输出低电平
      *GPIOE_PUEN |= 0x08;       //上拉使能
      *GPIOE_OMD |= 0x08;        //输出使能

    return 0;
}

static ssize_t modbus_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  copy_from_user(&val,buf,count);

 if(val==1)
{
   //  printk("modbus_on\n");
    *GPIOE_DOUT |= 0x08;// // buzzer   PE3   1
}
else
{
   //   printk("modbus_off\n");
     *GPIOE_DOUT &= ~ (0x01 << 3);// buzzer   PE3   0 
}

   return 0;
}

static struct file_operations modbus_div_operations = {
        .owner		= THIS_MODULE,
	.open		= modbus_drv_open,
	.write	        = modbus_drv_write,
};

int major_modbus;

int modbus_drv_init(void)
{
    major_modbus = register_chrdev(0, "modbus_drv", &modbus_div_operations);//注册
   
//    printk("<1>\n   This is modbus_driver program.\n\n");
    modbus_drv_class = class_create(THIS_MODULE, "modbus_drv");
    modbus_drv_class_dev = device_create(modbus_drv_class, NULL, MKDEV(major_modbus, 0), NULL, "modbus");/* /dev/led */
   
  GPEFUN0 = (volatile unsigned long *)ioremap(0xB00000A0,4);
  GPIOE_OMD = (volatile unsigned long *)ioremap(0xB8001040,4);
  GPIOE_PUEN = (volatile unsigned long *)ioremap(0xB8001044,4);
  GPIOE_DOUT = (volatile unsigned long *)ioremap(0xB8001048,4);

    return 0;
}


void modbus_drv_exit(void)
{
      unregister_chrdev(major_modbus, "modbus_drv");//注册
	//device_unregister(led_drv_class_dev);
      device_destroy(modbus_drv_class, MKDEV(major_modbus, 0));
      class_destroy(modbus_drv_class);
      iounmap(GPEFUN0);
      iounmap(GPIOE_OMD);
      iounmap(GPIOE_PUEN);
      iounmap(GPIOE_DOUT);
}                                 

module_init(modbus_drv_init);
module_exit(modbus_drv_exit);


MODULE_LICENSE("GPL");

