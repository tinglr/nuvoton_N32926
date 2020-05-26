
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
#include <linux/i2c.h>
#include <linux/err.h>

#include <linux/slab.h>


static struct i2c_board_info gt5663_info = {	
	I2C_BOARD_INFO("gt5663", 0x5d), //名称很重要，要一样  5d
};

static struct i2c_client *gt5663_client;

static int gt5663_dev_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(0);
	gt5663_client = i2c_new_device(i2c_adap, &gt5663_info);
	i2c_put_adapter(i2c_adap);
	
	return 0;
}

static void gt5663_dev_exit(void)
{
	i2c_unregister_device(gt5663_client);
}


module_init(gt5663_dev_init);
module_exit(gt5663_dev_exit);
MODULE_LICENSE("GPL");



