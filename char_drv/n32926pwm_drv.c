
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

#include <linux/interrupt.h>  //request_irq
#include <mach/w55fa92_reg.h>


static struct class *n32926pwm_drv_class;
static struct timer_list n32926pwm_timer;
static DECLARE_WAIT_QUEUE_HEAD(n32926pwm_waitq);
static volatile int ev_press = 0, u32i, u32_temp=0,count_temp=0,rev_flag=0,capture_val;

static DECLARE_MUTEX(n32926pwm_lock);          //定义互斥锁

static volatile int u32Count[100];
static volatile int u32ingTimet[100];

static irqreturn_t n32926pwm_irq(int irq, void *dev_id)
{
    printk("capture_irq\n");
    writel((readl(REG_CCR0) & 0xF000F) | (1 << 4), REG_CCR0);  //清中断

    return IRQ_RETVAL(IRQ_HANDLED);
}


static int n32926pwm_drv_open(struct inode *inode,struct file *file)
{

	if (file->f_flags & O_NONBLOCK)
	{
		if (down_trylock(&n32926pwm_lock))
		return -EBUSY;
	}
	else
	{
		/* 获取信号量 */
		down(&n32926pwm_lock);
		printk("n32926pwm_drv_open\n");
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
 
        request_irq(IRQ_PWM, n32926pwm_irq, IRQF_DISABLED, "n32926pwm",NULL);
       printk("tinglr ok\n");
        return 0;
}

static ssize_t n32926pwm_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	
	unsigned char n32926pwm_vals[4]={0};
	if (size != sizeof(n32926pwm_vals))
	return -EINVAL;
  
	if (file->f_flags & O_NONBLOCK)
	{
		if (!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
	//wait_event_interruptible(capture0_waitq, ev_press);
	}

        printk("tinglr1\n");
	if(rev_flag>33)		 
	{
        printk("tinglr2\n");
	     printk("rev_flag =%d \n\n",rev_flag);
	

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

		n32926pwm_vals[0] = capture_val & 0xff;
		n32926pwm_vals[1] = (capture_val >> 8) & 0xff;
		n32926pwm_vals[2] = (capture_val >> 16) & 0xff;
		n32926pwm_vals[3] = (capture_val >> 24) & 0xff;


	//   printk("capture_val =%d  0x%x \n\n",capture_val,capture_val);
        //   printk("capture_val = 0x%x  0x%x  0x%x   0x%x \n\n",capture0_vals[0],capture0_vals[1],capture0_vals[2],capture0_vals[3]);
          u32i=0;
        writel((readl(REG_CCR0) & ~ 0x06) | (1 << 1), REG_CCR0);        //上升沿触发
   }
        rev_flag=0;
                  

	copy_to_user(buf, &n32926pwm_vals, 4);
	ev_press = 0;
	
	return 1;
}

int n32926pwm_drv_close(struct inode *inode, struct file *file)
{
        writel(readl(REG_CCR0) & ~(1 << 1), REG_CCR0);   //禁止上升沿中断	
	free_irq(IRQ_PWM, NULL);  
	up(&n32926pwm_lock);
	return 0;
}




static struct file_operations n32926pwm_drv_operations = {
        .owner		= THIS_MODULE,
	.open		= n32926pwm_drv_open,
	.read	        = n32926pwm_drv_read,
	.release        = n32926pwm_drv_close,

};

int major;

static void n32926pwm_function(unsigned long data)
{
  ev_press = 1;       /* 表示中断发生了 */
  rev_flag=count_temp;
 // printk("u32_temp=40ms,count_temp= %d \n \n \n",count_temp);  
  count_temp =0;u32i=0; 
  wake_up_interruptible(&n32926pwm_waitq);   /* 唤醒休眠的进程 */	
 // kill_fasync (&capture0_async, SIGIO, POLL_IN);
}


int n32926pwm_drv_init(void)
{
    init_timer(&n32926pwm_timer);
    n32926pwm_timer.function = n32926pwm_function;
    add_timer(&n32926pwm_timer); 
    major = register_chrdev(0, "n32926pwm_drv", &n32926pwm_drv_operations);//注册
   
    printk("<1>\n   This is n32926pwm_driver program.\n\n");
    n32926pwm_drv_class = class_create(THIS_MODULE, "n32926pwm_drv");
    device_create(n32926pwm_drv_class, NULL, MKDEV(major, 0), NULL, "n32926pwm");/* /dev/led */

   return 0;
}


void n32926pwm_drv_exit(void)
{
      del_timer(&n32926pwm_timer);
      unregister_chrdev(major, "n32926pwm_drv");//注册
      device_destroy(n32926pwm_drv_class, MKDEV(major, 0));
      class_destroy(n32926pwm_drv_class);
}                                 

module_init(n32926pwm_drv_init);
module_exit(n32926pwm_drv_exit);


MODULE_LICENSE("GPL");

