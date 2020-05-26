
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>   //copy_from_user  copy_to_user 
#include <asm/io.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/device.h>



//#include <linux/version.h>
//#include <linux/types.h>
//#include <linux/interrupt.h>
//#include <linux/list.h>
//#include <linux/timer.h>
//#include <linux/init.h>
//#include <linux/serial_core.h>


/* 分配/设置/注册一个platform_device */

static struct resource busled_resource[] = {
    [0] = {
        .start = 0xB00000A0,             /* GPEFUN0 */
        .end   = 0xB00000A0 + 4 - 1,
        .flags = IORESOURCE_IO,
    },
    [1] = {
        .start = 0xB8001040,             /*GPIOE_OMD */
        .end   = 0xB8001040 + 4 - 1,
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = 0xB8001044,             /* GPIOE_PUEN */
        .end   = 0xB8001044 + 4 - 1,
        .flags = IORESOURCE_IRQ,
    },
    [3] = {
        .start = 0xB8001048,             /* GPIOE_DOUT */
        .end   = 0xB8001048 + 4 - 1,
        .flags = IORESOURCE_DMA,
    },


    [4] = {
        .start = 4,                      /* LED1 e4  led2 e5 */
        .end   = 4,
        .flags = IORESOURCE_BUS,
    }

};

static void busled_release(struct device * dev)
{
}


static struct platform_device busled_dev = {
    .name         = "busled",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(busled_resource),
    .resource     = busled_resource,
    .dev = { 
    	.release = busled_release, 
	},
};

static int busled_dev_init(void)
{
	platform_device_register(&busled_dev);
	return 0;
}

static void busled_dev_exit(void)
{
	platform_device_unregister(&busled_dev);
}



module_init(busled_dev_init);
module_exit(busled_dev_exit);


MODULE_LICENSE("GPL");

