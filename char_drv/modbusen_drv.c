
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



static struct class *modbusen_drv_class;
//static struct class_device *modbusen_drv_class_dev;
static struct device *modbusen_drv_class_dev;


volatile unsigned long *GPBFUN1 =NULL;
volatile unsigned long *GPIOB_OMD =NULL;
volatile unsigned long *GPIOB_PUEN =NULL;
volatile unsigned long *GPIOB_DOUT =NULL;


static int modbusen_drv_open(struct inode *inode,struct file *file)
{
   //  printk("modbusen_drv_open\n");

	*GPBFUN1 &= ~ (0xF << 4);//Pb9配置为gpio
	*GPIOB_DOUT &= ~ (0x01 << 9); //输出低电平
	*GPIOB_PUEN &= ~ (0x01 << 9); //下拉失能
	//*GPIOB_PUEN |= 0x08;       //上拉使能
	*GPIOB_OMD |= 0x0200;        //输出使能




    return 0;
}

static ssize_t modbusen_drv_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int val;
  copy_from_user(&val,buf,count);

 if(val==1)
{
	//printk("modbus_on\n");
    	*GPIOB_DOUT |= 0x0200;//输出高电平
}
else
{
	//printk("modbusen_off\n");
	*GPIOB_DOUT &= ~ (0x01 << 9);//输出低电平
}

   return 0;
}

static struct file_operations modbusen_div_operations = {
        .owner		= THIS_MODULE,
	.open		= modbusen_drv_open,
	.write	        = modbusen_drv_write,
};

int major_modbusen;

int modbusen_drv_init(void)
{
    major_modbusen = register_chrdev(0, "modbusen_drv", &modbusen_div_operations);//注册
   
//  printk("<1>\n   This is modbus_driver program.\n\n");
    modbusen_drv_class = class_create(THIS_MODULE, "modbusen_drv");
    modbusen_drv_class_dev = device_create(modbusen_drv_class, NULL, MKDEV(major_modbusen, 0), NULL, "modbusen");
   
  GPBFUN1    = (volatile unsigned long *)ioremap(0xB000008C,4);
  GPIOB_OMD  = (volatile unsigned long *)ioremap(0xB8001010,4);
  GPIOB_PUEN = (volatile unsigned long *)ioremap(0xB8001014,4);
  GPIOB_DOUT = (volatile unsigned long *)ioremap(0xB8001018,4);

    return 0;
}


void modbusen_drv_exit(void)
{
      unregister_chrdev(major_modbusen, "modbusen_drv");//注册
	//device_unregister(led_drv_class_dev);
      device_destroy(modbusen_drv_class, MKDEV(major_modbusen, 0));
      class_destroy(modbusen_drv_class);
      iounmap(GPBFUN1);
      iounmap(GPIOB_OMD);
      iounmap(GPIOB_PUEN);
      iounmap(GPIOB_DOUT);
}                                 

module_init(modbusen_drv_init);
module_exit(modbusen_drv_exit);


MODULE_LICENSE("GPL");

