
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



static int major;
static struct class *class;
static struct i2c_client *gt5663_client;


static int mtp_ft5x0x_i2c_wxdata(struct i2c_client *client, char *rxdata1,int length) {
	int ret;
	struct i2c_msg msgw[] = {
		   {
			.addr	= client->addr,
			.flags	= 0,   //写
			.len	= 3,   //长度
			.buf	= rxdata1,
                     }
	};

	ret = i2c_transfer(client->adapter, msgw, 1);
	if (ret < 0)
		printk("%s: i2c read error: %d\n", __func__, ret);

	return ret;
}

static int mtp_ft5x0x_i2c_rxdata(struct i2c_client *client, char *rxdata1, char *rxdata2,int length) {
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr	= client->addr,
			.flags	= 0,   //写
			.len	= 2,   //长度
			.buf	= rxdata1,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD, //读
			.len	= length,
			.buf	= rxdata2,   //长度
		},

	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
		printk("%s: i2c read error: %d\n", __func__, ret);

	return ret;
}


static ssize_t gt5663_read(struct file * file, char __user *buf, size_t count, loff_t *off)
{
	//unsigned char addr, data;
        unsigned char buf1[3] = { 0X81,0x4E};
	unsigned char buf2[32] = { 0 };
        unsigned char buf3[4] = {0X81,0x4e,0x0};
	int ret;

	ret = mtp_ft5x0x_i2c_rxdata(gt5663_client,buf1,buf2,6);

	if (ret < 0) {
		printk("%s: read touch data failed, %d\n", __func__, ret);
		return ret;
	}

	//copy_from_user(&addr, buf, 1);
	//data = i2c_smbus_read_byte_data(gt5663_client, addr);
	copy_to_user(buf, &buf2, 6);

	ret = mtp_ft5x0x_i2c_wxdata(gt5663_client,buf3,3);

	if (ret < 0) {
		printk("%s: write touch data failed, %d\n", __func__, ret);
		return ret;
	}
	return 1;
}

/* buf[0] : addr
 * buf[1] : data
 */
static ssize_t gt5663_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	unsigned char ker_buf[2];
	unsigned char addr, data;

	copy_from_user(ker_buf, buf, 2);
	addr = ker_buf[0];
	data = ker_buf[1];

	printk("addr = 0x%02x, data = 0x%02x\n", addr, data);

	if (!i2c_smbus_write_byte_data(gt5663_client, addr, data))
		return 2;
	else
		return -EIO;	
}


static struct file_operations gt5663_fops = {
	.owner = THIS_MODULE,
	.read  = gt5663_read,
	.write = gt5663_write,
};

static int __devinit gt5663_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	gt5663_client = client;
		
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "gt5663", &gt5663_fops);
	class = class_create(THIS_MODULE, "gt5663");
	device_create(class, NULL, MKDEV(major, 0), NULL, "gt5663"); /* /dev/gt5663 */
	
	return 0;
}

static int __devexit gt5663_remove(struct i2c_client *client)
{
		//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "gt5663");
		
	return 0;
}

static const struct i2c_device_id gt5663_id_table[] = {
	{ "gt5663", 0 },
	{}
};


/* 1. 分配/设置i2c_driver */
static struct i2c_driver gt5663_driver = {
	.driver	= {
		.name	= "100ask",
		.owner	= THIS_MODULE,
	},
	.probe		= gt5663_probe,
	.remove		= __devexit_p(gt5663_remove),
	.id_table	= gt5663_id_table,
};

static int gt5663_drv_init(void)
{
	/* 2. 注册i2c_driver */
	i2c_add_driver(&gt5663_driver);
	
	return 0;
}

static void gt5663_drv_exit(void)
{
	i2c_del_driver(&gt5663_driver);
}


module_init(gt5663_drv_init);
module_exit(gt5663_drv_exit);
MODULE_LICENSE("GPL");



