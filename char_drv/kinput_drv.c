
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
#include <mach/w55fa92_keypad.h>


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


//#include <linux/slab.h>
//#include <asm/errno.h>
//#include <linux/mm.h>
//#include <mach/hardware.h>
//#include <linux/completion.h>
//#include <mach/irqs.h>
//#include <mach/w55fa92_keypad.h>
//#include <mach/regs-clock.h>




static struct timer_list kinput_timer;
static struct input_dev *kinput_dev;
static u32 val_key;

static irqreturn_t kinput_irq(int irq, void *dev_id)
{
    printk("kinput_irq\n");

    writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  
    val_key = readl(REG_IRQLHGPE) & 0xffff;
   //inpw(REG_IRQLHGPE) & 0xffff;
    mod_timer(&kinput_timer, jiffies+HZ/100);
    return IRQ_RETVAL(IRQ_HANDLED);
}



static void kinput_function(unsigned long data)
{

  int i;
  i=val_key & 0x04;
if(i)
     {//printk("val_key %d  \n", i );
input_event(kinput_dev, EV_KEY, KEY_K, 0);//弹起
input_sync(kinput_dev);//用于事件同步
     }
else{  //printk("val_key %d  \n", i ); 
input_event(kinput_dev, EV_KEY, KEY_K, 1);//按下
input_sync(kinput_dev);//用于事件同步
    }

}


int kinput_drv_init(void)
{

   if(!(kinput_dev = input_allocate_device()))

           {
                printk("kinput Drvier Allocate Memory Failed!\n");
 
           }
   /* 2.1 能产生哪类事件 */
   set_bit(EV_KEY, kinput_dev->evbit);
  // set_bit(EV_REP, kinput_dev->evbit);

   set_bit(KEY_K, kinput_dev->keybit);  

   input_register_device(kinput_dev);

   init_timer(&kinput_timer);
   kinput_timer.function = kinput_function;
   //buttons_timer.expires  = 0;
   add_timer(&kinput_timer); 

     //配置为中断方式，注册中断

     writel(readl(REG_GPEFUN0) & ~(0x0F00), REG_GPEFUN0);  
     writel(readl(REG_GPIOE_PUEN) & ~(1 << 2), REG_GPIOE_PUEN); // pull-up
     writel(readl(REG_GPIOE_OMD) & ~(1 << 2), REG_GPIOE_OMD);   // input

      writel(0x01, REG_IRQLHSEL);

        writel(readl(REG_IRQSRCGPE) & ~(0x30), REG_IRQSRCGPE); // GPE2 as nIRQ0 source
//	writel(readl(REG_IRQENGPE) | 0X040004, REG_IRQENGPE);  // UP&falling edge trigger
        writel(readl(REG_IRQENGPE) | 0X04, REG_IRQENGPE);      // UP&falling edge trigger
	writel((readl(REG_AIC_SCR1)& ~(0x00C70000)) | 0x00470000, REG_AIC_SCR1);
        writel(readl(REG_DBNCECON) |0x71, REG_DBNCECON);
        writel((1 << 2),  REG_AIC_SCCR);           // force clear previous interrupt, if any.
        writel(readl(REG_IRQTGSRC2) & 0xffff, REG_IRQTGSRC2);  // clear source

        request_irq(IRQ_GPIO0, kinput_irq, IRQF_DISABLED, "ktimer",NULL);

	
	//enable falling edge triggers
	//writel((readl(REG_IRQENGPA)& ~(0x001C0000)) | 0x000001C, REG_IRQENGPA); 


   return 0;
}


void kinput_drv_exit(void)
{
      writel((readl(REG_IRQENGPE)& ~(0x00040004)), REG_IRQENGPE); 
      free_irq(IRQ_GPIO0, NULL);

      del_timer(&kinput_timer);

      input_unregister_device(kinput_dev);

      input_free_device(kinput_dev);	

}                                 

module_init(kinput_drv_init);
module_exit(kinput_drv_exit);


MODULE_LICENSE("GPL");

