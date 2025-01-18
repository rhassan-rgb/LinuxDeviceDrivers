/*
 * This file is part of Linux Device Drivers (LDD) project.
 *
 * Linux Device Drivers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Linux Device Drivers is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Linux Device Drivers. If not, see <https://www.gnu.org/licenses/>.
 */

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
