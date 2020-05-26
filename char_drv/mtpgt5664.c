
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
#include <linux/input.h>
#include <linux/poll.h>
#include <linux/interrupt.h>  //request_irq
#include <mach/w55fa92_reg.h>
#include <linux/i2c.h>
#include <linux/err.h>

#include <linux/slab.h>



#define mtpgt5663_ADDR   0x5D
#define mtpgt5663_MAX_X  640
#define mtpgt5663_MAX_Y  480

//#define mtpgt5663_IRQ  gpio_to_irq(EXYNOS4_GPX1(6))

#define mtpgt5663_NAME "mtpgt5663"
#define mtpgt5663_MAX_ID 10 /* 由硬件决定 */

struct input_dev *ts_dev;
static struct work_struct mtpgt5663_work;
static struct i2c_client *mtpgt5663_client;


struct mtpgt5663_event {
	int x;
	int y;
	int id;
};

static struct mtpgt5663_event mtpgt5663_events[10];
static int mtpgt5663_points;



static irqreturn_t mtpgt5663_interrupt(int irq, void *dev_id) {

        writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  // clear source
    //    printk("tinglr ok\n");
	schedule_work(&mtpgt5663_work);

	return IRQ_HANDLED;
}



static int mtpgt5663_i2c_wxdata(struct i2c_client *client, char *rxdata1,int length) {
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

static int mtpgt5663_i2c_rxdata(struct i2c_client *client, char *rxdata1, char *rxdata2,int length) {
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




static int mtpgt5663_read_data(void) {
    unsigned char buf1[3] = { 0X81,0x4E};
    unsigned char buf2[32] = { 0 };
    unsigned char buf3[4] = {0X81,0x4e,0x0};
	int ret;

	ret = mtpgt5663_i2c_rxdata(mtpgt5663_client,buf1,buf2,6);

	if (ret < 0) {
		printk("%s: read touch data failed, %d\n", __func__, ret);
		return ret;
	}

	mtpgt5663_points = buf2[0] & 0x0f;//0x814E

	//    x= buf[2]+ (buf[3]*256);y=buf[4]+ (buf[5]*256);
	if (mtpgt5663_points)
	{
			mtpgt5663_events[0].x = (s16)(buf2[0x03] & 0x0F)<<8 | (s16)buf2[0x02];
			mtpgt5663_events[0].y = (s16)(buf2[0x05] & 0x0F)<<8 | (s16)buf2[0x04];
			mtpgt5663_events[0].id = 1;
             //   printk("touch data envents id%d (%d,%d) \n",mtpgt5663_events[0].id , mtpgt5663_events[0].x,mtpgt5663_events[0].y);		
	}
	
	ret = mtpgt5663_i2c_wxdata(mtpgt5663_client,buf3,3);

	if (ret < 0) {
	//	printk("%s: write touch data failed, %d\n", __func__, ret);
		return ret;
	}
	
	return 0;
}





static void mtpgt5663_work_func(struct work_struct *work)
{
	//int i;
	int ret;
	
	/* 读取I2C设备, 获得触点数据, 并上报 */
	/* 读取 */
	ret = mtpgt5663_read_data();
	if (ret < 0)
		return;

	/* 上报 */
	if (!mtpgt5663_points) {
		input_report_abs(ts_dev, ABS_PRESSURE, 0);
		input_report_key(ts_dev, BTN_TOUCH, 0);
		input_sync(ts_dev);
		return;
	}
	else
	{

	    input_report_abs(ts_dev, ABS_X, mtpgt5663_events[0].x/2);
	    input_report_abs(ts_dev, ABS_Y, mtpgt5663_events[0].y/2);
	    input_report_abs(ts_dev, ABS_PRESSURE, 1);
            input_report_key(ts_dev, BTN_TOUCH, 1);
	    input_sync(ts_dev);
	}

}

static int __devinit mtpgt5663_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* 终于进入了输入子系统 */
	mtpgt5663_client = client;

	/* 分配input_dev */
	ts_dev = input_allocate_device();
 
	/* 设置 */
	/* 2.1 能产生哪类事件 */
	set_bit(EV_SYN, ts_dev->evbit);
	set_bit(EV_KEY, ts_dev->evbit);
	set_bit(EV_ABS, ts_dev->evbit);

	/* 2.2 能产生这类事件里的哪些事件 */
	set_bit(BTN_TOUCH, ts_dev->keybit);

	/* 2.3 这些事件的范围 */
	
	input_set_abs_params(ts_dev, ABS_X, 0, mtpgt5663_MAX_X, 0, 0);
	input_set_abs_params(ts_dev, ABS_Y, 0, mtpgt5663_MAX_Y, 0, 0);	
        input_set_abs_params(ts_dev, ABS_PRESSURE, 0, 1, 0, 0);


	ts_dev->name = mtpgt5663_NAME; /* android会根据它找到配置文件 */

	/* 注册 */
	input_register_device(ts_dev);

	/* 硬件相关操作 */
	INIT_WORK(&mtpgt5663_work, mtpgt5663_work_func);
	
//	request_irq(mtpgt5663_IRQ, mtpgt5663_interrupt,
//	IRQ_TYPE_EDGE_FALLING /*IRQF_TRIGGER_FALLING*/, "100ask_mtpgt5663", ts_dev);
	
   

       //  writel(readl(REG_DBNCECON) |0x72, REG_DBNCECON);        //IRQ 1滤波128个时钟
         writel(readl(REG_IRQSRCGPE) & ~ 0x3 , REG_IRQSRCGPE);  //PE0配置为外部中断0
         writel(readl(REG_IRQENGPE) | 0x1, REG_IRQENGPE);        //PE0配置为下降沿触发
         writel((readl(REG_AIC_SCR1)& ~(0x00C70000)) | 0x00470000, REG_AIC_SCR1);//优先级为7 ，下降沿

//	sysInstallISR(IRQ_LEVEL_2, IRQ_EXTINT1, (PVOID)KEY_ISR);  //中断优先级为2，外部中断1
//	sysSetLocalInterrupt(ENABLE_IRQ);
//	sysEnableInterrupt(IRQ_EXTINT1);  

       

      //writel(0x01, REG_IRQLHSEL); //所存
        writel((1 << 3),  REG_AIC_SCCR);                       // force clear previous interrupt, if any.928
        writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  // clear source
   //   request_irq(IRQ_GPIO0, kinput_irq, IRQF_DISABLED, "ktimer",NULL);



	request_irq(IRQ_GPIO0, mtpgt5663_interrupt, IRQF_DISABLED, "mtpgt5663",ts_dev);
	
	return 0;
}

static int __devexit mtpgt5663_remove(struct i2c_client *client)
{
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	free_irq(IRQ_GPIO0, ts_dev);
	cancel_work_sync(&mtpgt5663_work);

	input_unregister_device(ts_dev);
	input_free_device(ts_dev);
	
	return 0;
}

static const struct i2c_device_id mtpgt5663_id_table[] = {
	{ "mtpgt5663", 0 },  /* 支持我们自己的mtpgt5663_driver自行探测到的I2C设备 */
//	{ "ft5x0x_ts", 0},    /* 支持mach-tiny4412.c中注册的名为"ft5x0x_ts"的I2C设备 */
	{}
};

static int mtpgt5663_ft5x06_valid(struct i2c_client *client)
{

	return 0;
}

static int mtpgt5663_detect(struct i2c_client *client,
		       struct i2c_board_info *info)
{
	/* 能运行到这里, 表示该addr的设备是存在的
	 * 但是有些设备单凭地址无法分辨(A芯片的地址是0x50, B芯片的地址也是0x50)
	 * 还需要进一步读写I2C设备来分辨是哪款芯片
	 * detect就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	 */
	//printk("mtpgt5663_detect : addr = 0x%x\n", client->addr);
         
                                       // 2*10ms=20Ms	//延时15ms 
      //  printk("tp reset\n");

	if (mtpgt5663_ft5x06_valid(client) < 0)
		return -1;
	
	strlcpy(info->type, "mtpgt5663", I2C_NAME_SIZE);
	return 0;

	/* 返回0之后, 会创建一个新的I2C设备
	 * i2c_new_device(adapter, &info), 其中的info->type = "mtpgt5663"
	 */
}

static const unsigned short addr_list[] = { mtpgt5663_ADDR, I2C_CLIENT_END };

/* 1. 分配/设置i2c_driver */
static struct i2c_driver mtpgt5663_driver = {
	.class  = I2C_CLASS_HWMON, /* 表示去哪些适配器上找设备 */
	.driver	= {
		.name	= "mtpgt5663",
		.owner	= THIS_MODULE,
	},
	.probe		= mtpgt5663_probe,
	.remove		= __devexit_p(mtpgt5663_remove),
	.id_table	= mtpgt5663_id_table,
	.detect     = mtpgt5663_detect,  /* 用这个函数来检测设备确实存在 */
	.address_list	= addr_list,   /* 这些设备的地址 */
};

static int mtpgt5663_drv_init(void)
{
//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	/* 2. 注册i2c_driver */
	i2c_add_driver(&mtpgt5663_driver);
	
	return 0;
}

static void mtpgt5663_drv_exit(void)
{
	i2c_del_driver(&mtpgt5663_driver);
}


module_init(mtpgt5663_drv_init);
module_exit(mtpgt5663_drv_exit);
MODULE_LICENSE("GPL");



