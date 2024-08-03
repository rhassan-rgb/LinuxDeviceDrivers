#include <linux/module.h>

static int __init my_module_init_func(void)
{
    pr_info("Hello World! This is my first Module");
    pr_info("Hello World! This is my first Module 2");
    pr_info("Hello World! This is my first Module 3");

    return 0;
}

static void __exit my_module_exit_func(void)
{
    pr_info("Goodbye World!");
}


module_init(my_module_init_func);
module_exit(my_module_exit_func);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ragab Hassan");
MODULE_DESCRIPTION("This is a dummy poc module");
MODULE_INFO(board, "Beaglebone Black Rev C");
