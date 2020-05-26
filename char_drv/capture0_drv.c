
#include <linux/module.h>    //所有模块都需要这个头文件
#include <linux/kernel.h>    //声明了printk()这个内核态用的函数
#include <linux/fs.h>        //文件系统有关的，结构体file_operations在头文件 linux/fs.h中定义
#include <linux/init.h>      //init和exit相关宏
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/uaccess.h>     //copy_from_user  copy_to_user 
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
#include <linux/miscdevice.h>//在这个头文件中主要是misc(混合)设备注册和注销 linux中用struct miscdevice来描述一个混杂设备

//#include <linux/poll.h>
#include <linux/interrupt.h>  //request_irq
#include <mach/w55fa92_reg.h>


static struct class *capture0_drv_class;
//static struct class_device *key_drv_class_dev;

static struct timer_list capture0_timer;

static DECLARE_WAIT_QUEUE_HEAD(capture0_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0, u32i, u32_temp=0,count_temp=0,rev_flag=0,capture_val;

//static struct fasync_struct *capture0_async;

//static atomic_t canopen = ATOMIC_INIT(1);   //定义原子变量并初始化为1
//static unsigned char key_val;

static DECLARE_MUTEX(capture0_lock);          //定义互斥锁

static volatile int u32Count[100];
static volatile int u32ingTimet[100];

static irqreturn_t capture0_irq(int irq, void *dev_id)
{
    //printk("capture_irq\n");
    writel((readl(REG_CCR0) & 0xF000F) | (1 << 4), REG_CCR0);  //清中断

    u32Count[u32i++] = readl(REG_CFLR0);              //获得下降源时间戳
    u32Count[u32i++] = readl(REG_CRLR0);              //获得上升元时间戳
    if(u32i>90){u32i=0;}
    count_temp++;
    mod_timer(&capture0_timer, jiffies+HZ/25);
    return IRQ_RETVAL(IRQ_HANDLED);
}


static int capture0_drv_open(struct inode *inode,struct file *file)
{

	if (file->f_flags & O_NONBLOCK)
	{
		if (down_trylock(&capture0_lock))
		return -EBUSY;
	}
	else
	{
		/* 获取信号量 */
		down(&capture0_lock);
	//	printk("capture0_drv_open\n");
	}

         //配置为中断方式，注册中断
 //	 writel((readl(REG_CLKDIV5) & ~(PWM_N1|PWM_S|PWM_N0)) | ( 0x18 | (5 << 5) | 3), REG_CLKDIV5);// 240M/24=10M
//	 writel(readl(REG_APBCLK) | PWM_CKE, REG_APBCLK);     // PWM Clock Enable
//	 writel(PWMRST, REG_APBIPRST);                        // PWM reset 
//	 writel(readl(REG_APBIPRST) & ~PWMRST, REG_APBIPRST);

	 writel(readl(REG_CCR0) & ~ 1, REG_CCR0);        //反向器off
	 writel(readl(REG_PCR) | (1 << 3), REG_PCR);     //设置为切换模式Toggle Mode     
//	 writel((readl(REG_PPR) & ~CP0) | ((10 - 1) & 0xFF), REG_PPR); //10分频
	 writel((readl(REG_PWM_CSR) & ~ 0x7) | (4 & 0x7), REG_PWM_CSR);//输入时钟不分频
	 writel((65536 - 1), REG_CNR0);                  //周期为65536个时钟
	 writel((65536 * 50 / 100 - 1), REG_CMR0);       //占空比默认50%
				
        writel((readl(REG_CCR0) & ~ 0x06) | (1 << 1), REG_CCR0);        //上升沿触发
        writel((readl(REG_GPDFUN1) & ~(0xF << 16)) | (0x1 << 16), REG_GPDFUN1);        //配置PD12为cap0    
	
	writel(readl(REG_CAPENR) | 1, REG_CAPENR); 	 //开启捕获0功能
        writel((readl(REG_CCR0) & 0xF000F) | (1 << 7), REG_CCR0);  //清除PWM下降标志
	writel((readl(REG_CCR0) & 0xF000F) | (1 << 6), REG_CCR0);  //清除上升flag    
				
	writel(readl(REG_PCR) | 1, REG_PCR);                     //Timer 0 Enable
	writel(readl(REG_CCR0) | (1 << 3), REG_CCR0); 	       //捕获通道0转换启用
 
        request_irq(IRQ_PWM, capture0_irq, IRQF_DISABLED, "capture0",NULL);
        return 0;
}

static ssize_t capture0_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	
	unsigned char capture0_vals[4]={0};
	if (size != sizeof(capture0_vals))
	return -EINVAL;
  
	if (file->f_flags & O_NONBLOCK)
	{
		if (!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(capture0_waitq, ev_press);
	}


	if(rev_flag>33)		 
	{
	    // printk("rev_flag =%d \n\n",rev_flag);
	

		 writel(readl(REG_CCR0) & ~(1 << 1), REG_CCR0);   //禁止上升沿中断
		 if(u32Count[0]<u32Count[1])
		 {
			u32Count[0]=u32Count[0]+65536;
		 }
 
		 for(u32i=0;u32i<68;u32i=u32i+2)
		 {			 
			u32ingTimet[u32i] = u32Count[u32i]-u32Count[u32i+1];
			u32ingTimet[u32i+1] = 65536-u32Count[u32i+2];			 
		 } 
 	 
	//	 u32ingTimet[67] = 20000;
	//	 for(u32i=0;u32i<68;u32i++)
	//	 {	
	//		printk("u32ingTimet %d = %d\n",u32i,u32ingTimet[u32i]);			
	//	 }

                   if(u32ingTimet[0]>=8000 && u32ingTimet[0]<=10000 && u32ingTimet[1]>=4000 && u32ingTimet[1]<=5000)
                    {
                          capture_val=0;    
                          for(u32i=2;u32i<66;u32i=u32i+2)
		          {	
			   if(u32ingTimet[u32i]>=250 && u32ingTimet[u32i]<=2000 && u32ingTimet[u32i+1]>=250 && u32ingTimet[u32i+1]<=2000)
				{   
                                     if((u32ingTimet[u32i]+u32ingTimet[u32i+1])>1500)
					{capture_val=(capture_val<<1)+1; 
		                          
					}
                                      else {capture_val=capture_val<<1; 
					  }	

                                }
				else {capture_val=0; }		
		          
                          }

                    }
                     else{ capture_val=0;}

		capture0_vals[0] = capture_val & 0xff;
		capture0_vals[1] = (capture_val >> 8) & 0xff;
		capture0_vals[2] = (capture_val >> 16) & 0xff;
		capture0_vals[3] = (capture_val >> 24) & 0xff;


	//   printk("capture_val =%d  0x%x \n\n",capture_val,capture_val);
        //   printk("capture_val = 0x%x  0x%x  0x%x   0x%x \n\n",capture0_vals[0],capture0_vals[1],capture0_vals[2],capture0_vals[3]);
          u32i=0;
        writel((readl(REG_CCR0) & ~ 0x06) | (1 << 1), REG_CCR0);        //上升沿触发
   }
        rev_flag=0;
                  

	copy_to_user(buf, &capture0_vals, 4);
	ev_press = 0;
	
	return 1;
}

int capture0_drv_close(struct inode *inode, struct file *file)
{
        writel(readl(REG_CCR0) & ~(1 << 1), REG_CCR0);   //禁止上升沿中断	
	free_irq(IRQ_PWM, NULL);  
	up(&capture0_lock);
	return 0;
}

//static unsigned capture0_drv_poll(struct file *file, poll_table *wait)
//{
//	unsigned int mask = 0;
//	poll_wait(file, &capture0_waitq, wait);     //不会立即休眠
//
//	if (ev_press)
//		mask |= POLLIN | POLLRDNORM;
///
//	return mask;
//}

//static int capture0_drv_fasync (int fd, struct file *filp, int on)
//{
//	printk("driver: capture0_drv_fasync\n");
//	return fasync_helper (fd, filp, on, &capture0_async);
//}


static struct file_operations capture0_drv_operations = {
        .owner		= THIS_MODULE,
	.open		= capture0_drv_open,
	.read	        = capture0_drv_read,
	.release        = capture0_drv_close,
//	.poll   	= capture0_drv_poll,
//	.fasync	 	= capture0_drv_fasync,
};

int major_capture0;

static void capture0_function(unsigned long data)
{
  ev_press = 1;       /* 表示中断发生了 */
  rev_flag=count_temp;
 // printk("u32_temp=40ms,count_temp= %d \n \n \n",count_temp);  
  count_temp =0;u32i=0; 
  wake_up_interruptible(&capture0_waitq);   /* 唤醒休眠的进程 */	
 // kill_fasync (&capture0_async, SIGIO, POLL_IN);
}


int capture0_drv_init(void)
{
    init_timer(&capture0_timer);
    capture0_timer.function = capture0_function;
    add_timer(&capture0_timer); 
    major_capture0 = register_chrdev(0, "capture0_drv", &capture0_drv_operations);//注册
   
   // printk("<1>\n   This is capture0_driver program.\n\n");
    capture0_drv_class = class_create(THIS_MODULE, "capture0_drv");
    device_create(capture0_drv_class, NULL, MKDEV(major_capture0, 0), NULL, "capture0");/* /dev/led */

   return 0;
}


void capture0_drv_exit(void)
{
      del_timer(&capture0_timer);
      unregister_chrdev(major_capture0, "capture0_drv");//注册
      device_destroy(capture0_drv_class, MKDEV(major_capture0, 0));
      class_destroy(capture0_drv_class);
}                                 

module_init(capture0_drv_init);
module_exit(capture0_drv_exit);


MODULE_LICENSE("GPL");

