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
#include "lcd_16x2.h"

/* This driver manages the LCD 16x2 character device using 4-bit interface
 * which is controlled via gpio pins.
 * interaction with the device is done through an character device under /dev
 * via ioctl interface with the following commands:
 *      LCD_COMMAND
 *      LCD_DATA
 *      LCD_SCROLL
 *      LCD_POS_READ
 *      LCD_POS_WRITE
 * LCD Wriring is defined in the device tree using the following gpios names:
 *      lcd-en-gpios
 *      lcd-rs-gpios
 *      lcd-rw-gpios
 *      lcd-d4-gpios
 *      lcd-d5-gpios
 *      lcd-d6-gpios
 *      lcd-d7-gpios
 */


/* Module Functions */
/* driver init function */
static int __init lcd_16x2_init(void);
/* driver cleanup function */
static void __exit lcd_16x2_cleanup(void);

/* file operations */
/* handle ioctl operations */
long lcd_16x2_unlocked_ioctl(struct file *filp, unsigned int arg, unsigned long user_data_ptr);
/* handle open syscall */
int lcd_16x2_open (struct inode *inode, struct file *filp);

/* driver functions */
/* probe function to initialize matched device */
int lcd_16x2_probe(struct platform_device *pdev);
/* remove function to deinitialize matched device */
int lcd_16x2_remove(struct platform_device *pdev);

/* file operations */
struct file_operations f_ops ={
    /* open syscall implementation pointer*/
    .open=lcd_16x2_open,
    /* ioctl syscall implementation pointer*/
    .unlocked_ioctl=lcd_16x2_unlocked_ioctl
};

/* Open firmware device_id table used to store the comtaible devices strings 
 * used by the kernel in the matching process when the driver is loaded 
 * if the driver is a module or during the initialization if it's a static 
 * module
 * 
 * NOTE: this array should be NULL terminated
 */
struct of_device_id lcd_of_match_table[] = {
    {.compatible="rgb,16x2-lcd"},
    {}
};

/* platform driver */
/* Data structure to hold the driver data used by the kernel  
 * it defines:
 *  - probe()   : to handle the device initialization 
 *  - remove()  : to handle the device deinitialization
 *  - of_match_table : to hold the dt compatible list 
 */
struct platform_driver lcd_driver = {
    .probe=lcd_16x2_probe,
    .remove=lcd_16x2_remove,
    .driver ={
        .of_match_table=lcd_of_match_table
    }
};

/* driver private data */
/* Data structure to hold the private data needed by driver during its operation */
struct lcd_16x2_driver{
    /* holds the sysfs class pointer */
    struct class* class_lcd;
    /* number of devices managed by the driver */
    int managed_devices;
    /* first device number that's managed by the driver */
    dev_t first_device;
};

/* device private data */
/* Data structure to hold the device private data needed by driver to manage the device */
struct lcd_16x2_device{
    /* character device associated with the device */
    struct cdev* lcd_16x2_cdev;
    /* device number major:minor */
    dev_t device_number;
};

/* 
 * ioctl implementation 
 * filp: 
 * arg:
 * user_data_ptr:
 * 
 * return value: 
 */
long lcd_16x2_unlocked_ioctl(struct file *filp, unsigned int arg, unsigned long user_data_ptr)
{
    return 0;
}

/* 
 * open() implementation 
 * inode: 
 * filp:
 * 
 * return value: 
 */
int lcd_16x2_open (struct inode *inode, struct file *filp)
{
    return 0;
}

/* 
 * driver probe() implementation 
 * pdev: 
 * 
 * return value: 
 */
int lcd_16x2_probe(struct platform_device *pdev)
{
    return 0;
}

/* 
 * driver remove() implementation 
 * pdev: 
 * 
 * return value: 
 */
int lcd_16x2_remove(struct platform_device *pdev)
{
    return 0;
}

/* 
 * module init() implementation  
 * 
 * return value: 
 */
static int __init lcd_16x2_init(void)
{
    return 0;
}

/* 
 * module remove() implementation  
 * 
 * return value: 
 */
static void __exit lcd_16x2_cleanup(void)
{

}

module_init(lcd_16x2_init);
module_exit(lcd_16x2_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("\
This a platform driver for the 16x2 char lcd in 4-bit mode using GPIO\
");
MODULE_AUTHOR("Ragab H. Elkattawy <r.elkattawy@gmail.com>");