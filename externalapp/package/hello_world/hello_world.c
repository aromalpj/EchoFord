#include <linux/module.h>
#include <linux/init.h>

static int __init hello_world_init(void)
{
    pr_info("Hello World module loaded.\n");
    return 0;
}

static void __exit hello_world_exit(void)
{
    pr_info("Hello World module unloaded.\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Midhun");
MODULE_DESCRIPTION("A simple Hello World Linux kernel module");
MODULE_VERSION("1.0");