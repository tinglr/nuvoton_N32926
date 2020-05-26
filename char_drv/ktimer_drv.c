
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

#include <linux/poll.h>
#include <linux/interrupt.h>  //request_irq
#include <mach/w55fa92_reg.h>


//#include <linux/types.h>
//#include <linux/wait.h>
//#include <linux/sched.h>
//#include <linux/dma-mapping.h>
//#include <linux/compiler.h>
//#include <linux/mm.h>
//#include <linux/ioport.h>
//#include <linux/list.h>
//#include <linux/bootmem.h>
//#include <linux/slab.h>
//#include <linux/vmalloc.h>



static struct class *ktimer_drv_class;
//static struct class_device *key_drv_class_dev;

static struct timer_list ktimer_timer;

static DECLARE_WAIT_QUEUE_HEAD(ktimer_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

static struct fasync_struct *ktimer_async;

//static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量并初始化为1

//static unsigned char key_val;

static DECLARE_MUTEX(ktimer_lock);     //定义互斥锁


static irqreturn_t ktimer_irq(int irq, void *dev_id)
{
    printk("ktimer_irq\n");
    writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  
   // ev_press = 1;                            /* 表示中断发生了 */
   mod_timer(&ktimer_timer, jiffies+HZ/100);
    return IRQ_RETVAL(IRQ_HANDLED);
}


static int ktimer_drv_open(struct inode *inode,struct file *file)
{

	if (file->f_flags & O_NONBLOCK)
	{
		if (down_trylock(&ktimer_lock))
			return -EBUSY;
	}
	else
	{
		/* 获取信号量 */
		down(&ktimer_lock);
                printk("ktimer_drv_open\n");
	}


       //配置为中断方式，注册中断

     writel(readl(REG_GPEFUN0) & ~(0x0F00), REG_GPEFUN0);  
     writel(readl(REG_GPIOE_PUEN) & ~(1 << 2), REG_GPIOE_PUEN); // pull-up
     writel(readl(REG_GPIOE_OMD) & ~(1 << 2), REG_GPIOE_OMD);   // input

       //gpio_setlatchtrigger (1); 
       //outpw(REG_IRQLHSEL , 1);

        writel(readl(REG_IRQSRCGPE) & ~(0x30), REG_IRQSRCGPE); // GPE2 as nIRQ0 source
//	writel(readl(REG_IRQENGPE) | 0X040004, REG_IRQENGPE);  // UP&falling edge trigger
        writel(readl(REG_IRQENGPE) | 0X04, REG_IRQENGPE);      // UP&falling edge trigger
	writel((readl(REG_AIC_SCR1)& ~(0x00C70000)) | 0x00470000, REG_AIC_SCR1);
        writel(readl(REG_DBNCECON) |0x71, REG_DBNCECON);
        writel((1 << 2),  REG_AIC_SCCR);                       // force clear previous interrupt, if any.
        writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  // clear source

        request_irq(IRQ_GPIO0, ktimer_irq, IRQF_DISABLED, "ktimer",NULL);

	//enable falling edge triggers
	//writel((readl(REG_IRQENGPA)& ~(0x001C0000)) | 0x000001C, REG_IRQENGPA); 

    return 0;
}

static ssize_t ktimer_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	
	unsigned char ktimer_vals[1]={0};
	if (size != sizeof(ktimer_vals))
	return -EINVAL;
  
	if (file->f_flags & O_NONBLOCK)
	{
		if (!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(ktimer_waitq, ev_press);
	}

	copy_to_user(buf, &ktimer_vals, 1);
	ev_press = 0;
	
	return 1;
}

int ktimer_drv_close(struct inode *inode, struct file *file)
{

	writel((readl(REG_IRQENGPE)& ~(0x00040004)), REG_IRQENGPE); 
	free_irq(IRQ_GPIO0, NULL);
	up(&ktimer_lock);
	return 0;
}

static unsigned ktimer_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &ktimer_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static int ktimer_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: ktimer_drv_fasync\n");
	return fasync_helper (fd, filp, on, &ktimer_async);
}


static struct file_operations ktimer_div_operations = {
        .owner		= THIS_MODULE,
	.open		= ktimer_drv_open,
	.read	        = ktimer_drv_read,
	.release        = ktimer_drv_close,
	.poll   	= ktimer_drv_poll,
	.fasync	 	= ktimer_drv_fasync,
};

int major;

static void ktimer_function(unsigned long data)
{


    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&ktimer_waitq);   /* 唤醒休眠的进程 */
	
    kill_fasync (&ktimer_async, SIGIO, POLL_IN);
}


int ktimer_drv_init(void)
{

    init_timer(&ktimer_timer);
    ktimer_timer.function = ktimer_function;
    //buttons_timer.expires  = 0;
    add_timer(&ktimer_timer); 
    major = register_chrdev(0, "ktimer_drv", &ktimer_div_operations);//注册
   
    printk("<1>\n   This is ktimer_driver program.\n\n");
    ktimer_drv_class = class_create(THIS_MODULE, "ktimer_drv");
    device_create(ktimer_drv_class, NULL, MKDEV(major, 0), NULL, "ktimer");/* /dev/led */

   return 0;
}


void ktimer_drv_exit(void)
{
      unregister_chrdev(major, "ktimer_drv");//注册
      //device_unregister(led_drv_class_dev);
      device_destroy(ktimer_drv_class, MKDEV(major, 0));
      class_destroy(ktimer_drv_class);
}                                 

module_init(ktimer_drv_init);
module_exit(ktimer_drv_exit);


MODULE_LICENSE("GPL");

