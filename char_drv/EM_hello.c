

#include <linux/module.h>
#include <linux/kernel.h>

         

static int __init_EM_hello_init(void)
{

    printk("<1>\n     Hello,tinglr!\n");
    printk("<1>\nThis is first driver program.\n\n");

    return 0;
}

static void __exit_EM_hello_exit(void)
{
    printk("<1>\n     Exit!\n");
    printk("<1>\nGoodbye tinglr!\n\n");
}                                    


