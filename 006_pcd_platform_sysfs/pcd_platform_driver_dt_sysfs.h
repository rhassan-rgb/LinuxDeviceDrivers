#ifndef PCD_PLATFORM_DRIVER_DT_SYSFS_H
#define PCD_PLATFORM_DRIVER_DT_SYSFS_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include "platform.h"

#define BASE_NUMBER 0u
#define DEVICE_COUNT 10u

/* per device private data <<dynamic>> */
struct pcdev_private_data {
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;
    struct device *device_pcd;
};

/* driver private data <<static>>*/
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_num_base;
    struct class *class_pcd;
};

struct device_configuration {
    int config_item_1;
    int config_item_2;
};

enum {
    DEV_A1X,
    DEV_B1X,
    DEV_C1X,
    DEV_D1X
};

int pcd_platform_driver_probe(struct platform_device *);
int pcd_platform_driver_remove(struct platform_device *);

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open (struct inode *inode, struct file *filp);
int pcd_release (struct inode *inode, struct file *filp);

struct pcdev_platform_data * pcdev_get_platfrom_from_dt(struct device *dev);

#endif /*PCD_PLATFORM_DRIVER_DT_SYSFS_H*/