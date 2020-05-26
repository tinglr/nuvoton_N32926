
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
//#define  GPIOE_PIN   0xB800104C   //R    GPIO Port E Pin Value 

static struct class *button_drv_class;
//static struct class_device *button_drv_class_dev;

volatile unsigned long *GPEFUN0    =NULL;
volatile unsigned long *GPIOE_PUEN =NULL;
volatile unsigned long *GPIOE_PIN  =NULL;
volatile unsigned long *GPIOE_OMD  =NULL;

static int button_drv_open(struct inode *inode,struct file *file)
{
       printk("button_drv_open\n");

      *GPEFUN0 &= ~ (0xF << 8);            //PE2配置为gpio
      *GPIOE_PUEN &= ~ (0x01 << 2);        //上拉失能
      *GPIOE_OMD  &= ~ (0x01 << 2);        //输入使能

    return 0;
}

static ssize_t button_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/* 返回4个引脚的电平 */
	unsigned char key_vals[1];

	int regval;

	if (size != sizeof(key_vals))
	return -EINVAL;

        regval = *GPIOE_PIN;

	key_vals[0] = (regval & (1<<2)) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));
	
	return sizeof(key_vals);

}

static struct file_operations button_div_operations = {
        .owner		= THIS_MODULE,
	.open		= button_drv_open,
	.read	        = button_drv_read,
};

int major;

int button_drv_init(void)
{
    major = register_chrdev(0, "button_drv", &button_div_operations);//注册
   
    printk("<1>\n   This is button_driver program.\n\n");
    button_drv_class = class_create(THIS_MODULE, "button_drv");
 //   button_drv_class_dev = device_create(button_drv_class, NULL, MKDEV(major, 0), NULL, "button");/* /dev/led */
   device_create(button_drv_class, NULL, MKDEV(major, 0), NULL, "button");/* /dev/led */
   GPEFUN0 = (volatile unsigned long *)ioremap(0xB00000A0,4);
   GPIOE_OMD = (volatile unsigned long *)ioremap(0xB8001040,4);
   GPIOE_PUEN = (volatile unsigned long *)ioremap(0xB8001044,4);
   GPIOE_PIN = (volatile unsigned long *)ioremap(0xB800104C,4);

   return 0;
}


void button_drv_exit(void)
{
      unregister_chrdev(major, "button_drv");//注册
      //device_unregister(led_drv_class_dev);
      device_destroy(button_drv_class, MKDEV(major, 0));
      class_destroy(button_drv_class);
      iounmap(GPEFUN0);
      iounmap(GPIOE_OMD);
      iounmap(GPIOE_PUEN);
      iounmap(GPIOE_PIN);
}                                 

module_init(button_drv_init);
module_exit(button_drv_exit);


MODULE_LICENSE("GPL");

