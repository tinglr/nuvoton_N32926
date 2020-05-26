
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



static struct class *kpoll_drv_class;
//static struct class_device *key_drv_class_dev;

static DECLARE_WAIT_QUEUE_HEAD(kpoll_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

//static unsigned char key_val;


static irqreturn_t kpoll_irq(int irq, void *dev_id)
{
    printk("kpoll_irq\n");
    writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  
     ev_press = 1;                         /* 表示中断发生了 */
    wake_up_interruptible(&kpoll_waitq);   /* 唤醒休眠的进程 */	
    return IRQ_RETVAL(IRQ_HANDLED);
}



static int kpoll_drv_open(struct inode *inode,struct file *file)
{
       printk("kpoll_drv_open\n");
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

        request_irq(IRQ_GPIO0, kpoll_irq, IRQF_DISABLED, "poll",NULL);

	//enable falling edge triggers
	//writel((readl(REG_IRQENGPA)& ~(0x001C0000)) | 0x000001C, REG_IRQENGPA); 

    return 0;
}

static ssize_t kpoll_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	
	unsigned char kpoll_vals[1]={0};
	if (size != sizeof(kpoll_vals))
	return -EINVAL;
    //     printk("wait_event_interruptible begain\n");
	wait_event_interruptible(kpoll_waitq, ev_press);
   //      printk("wait_event_interruptible end\n");
	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &kpoll_vals, 1);
	ev_press = 0;
	
	return 1;
}

int kpoll_drv_close(struct inode *inode, struct file *file)
{

	//disable falling edge triggers
	writel((readl(REG_IRQENGPE)& ~(0x00040004)), REG_IRQENGPE); 
	free_irq(IRQ_GPIO0, NULL);

	return 0;
}

static unsigned kpoll_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &kpoll_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}




static struct file_operations kpoll_div_operations = {
        .owner		= THIS_MODULE,
	.open		= kpoll_drv_open,
	.read	        = kpoll_drv_read,
	.release        = kpoll_drv_close,
	.poll   	= kpoll_drv_poll,
};

int major;

int kpoll_drv_init(void)
{
    major = register_chrdev(0, "kpoll_drv", &kpoll_div_operations);//注册
   
    printk("<1>\n   This is kpoll_driver program.\n\n");
    kpoll_drv_class = class_create(THIS_MODULE, "kpoll_drv");
    device_create(kpoll_drv_class, NULL, MKDEV(major, 0), NULL, "kpoll");/* /dev/led */

   return 0;
}


void kpoll_drv_exit(void)
{
      unregister_chrdev(major, "kpoll_drv");//注册
      //device_unregister(led_drv_class_dev);
      device_destroy(kpoll_drv_class, MKDEV(major, 0));
      class_destroy(kpoll_drv_class);
}                                 

module_init(kpoll_drv_init);
module_exit(kpoll_drv_exit);


MODULE_LICENSE("GPL");

