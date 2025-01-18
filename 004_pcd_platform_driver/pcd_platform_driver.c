#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include "platform.h"

#define BASE_NUMBER 0u
#define DEVICE_COUNT 10u

static int check_permission(int dev_perm, int access_mode);
int pcd_platform_driver_probe(struct platform_device *);
int pcd_platform_driver_remove(struct platform_device *);

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open (struct inode *inode, struct file *filp);
int pcd_release (struct inode *inode, struct file *filp);

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

/* file_operations struct that will be used to hold the file operations of our dev*/
struct file_operations pcd_fOps = {
    .llseek = pcd_lseek,
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write
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

struct device_configuration device_configs[] = 
{
    [DEV_A1X] = {.config_item_1 = 10, .config_item_2 = 100},
    [DEV_B1X] = {.config_item_1 = 20, .config_item_2 = 200},
    [DEV_C1X] = {.config_item_1 = 30, .config_item_2 = 300},
    [DEV_D1X] = {.config_item_1 = 40, .config_item_2 = 400}
};


struct platform_device_id pcd_id_table[] = 
{
    {.name = "pcdev-A1x", .driver_data = DEV_A1X}, 
    {.name = "pcdev-B1x", .driver_data = DEV_B1X}, 
    {.name = "pcdev-C1x", .driver_data = DEV_C1X},
    {.name = "pcdev-D1x", .driver_data = DEV_D1X},
    {}
};
struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .id_table = pcd_id_table,
    .driver = {
        .name = "pseudo-char-device"
    }
};


/* Driver private data object */
struct pcdrv_private_data pcdrv_data = {.total_devices = 0};


static int __init pcd_driver_init(void)
{   
    int ret;
    /* 1. dynamically allocate a device number for MAX_DEVICES */
    ret = alloc_chrdev_region(&pcdrv_data.device_num_base, BASE_NUMBER, DEVICE_COUNT, "rgb_devices");
    if (ret < 0)
    {
        pr_err("error allocating char dev\n");
        return ret;
    }
    /* 2. Create device Class under /sys/class */
    pcdrv_data.class_pcd = class_create("rgb_chrdev_class");
    if (IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("error Creating class \n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_num_base, DEVICE_COUNT);
    }
    /* 3. register a platform driver */
    platform_driver_register(&pcd_platform_driver);
    pr_info("Driver module added successfully \n");
    return 0;
}

static void __exit pcd_driver_cleanup(void)
{
    /* 1. unregister a platform driver */
    platform_driver_unregister(&pcd_platform_driver);
    /* 2. destroy class */
    class_destroy(pcdrv_data.class_pcd);
    /* 3. unregister device numbers for DEVICE_COUNT*/
    unregister_chrdev_region(pcdrv_data.device_num_base, DEVICE_COUNT);
    pr_info("Driver module removed successfully \n");
}

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    return 0;
}
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos)
{
    return 0;
}

ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
}

static int check_permission(int dev_perm, int access_mode)
{
    pr_info("Perm: 0x%x",dev_perm);
    switch (dev_perm)
    {
        case RDONLY:
            if((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE))
                return 0;
            break;
        case WRONLY:
            if(!(access_mode & FMODE_READ) && (access_mode & FMODE_WRITE))
                return 0;
            break;
        case RDWR:
            return 0;
    }
    return -EPERM;
}
int pcd_open (struct inode *inode, struct file *filp)
{
    return 0;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("Close requested\n");
    return 0;
}

int pcd_platform_driver_probe(struct platform_device *pdev)
{
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data * pdata = NULL;
    int ret;

    pr_info("A device is detected: %s-%d\n", pdev->name, pdev->id);
    /* 1. get the platform data */
    pdata = (struct pcdev_platform_data *) dev_get_platdata(&pdev->dev);
    if (!pdata)
    {
        pr_info("No Platfrom Data available\n");
        ret = -EINVAL;
        goto err_no_pdata;
    }
    /* 2. Dynamically allocate memory for the private data */
    dev_data = (struct pcdev_private_data *) devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data)
    {
        pr_info("Cannot allocate memory \n");
        ret = -ENOMEM;
        goto err_no_memory;
    }
    /* save the allocated pointer in the driver_data inside the dev member of pdev in order to use it later at removal */
    /* pdev->dev.driver_data = dev_data; */
    dev_set_drvdata(&pdev->dev, dev_data);

    dev_data->pdata.size  = pdata->size;
    dev_data->pdata.perm  = pdata->perm;
    dev_data->pdata.serial_number  = pdata->serial_number;

    pr_info("Device serial_number = %s\n", dev_data->pdata.serial_number);
    pr_info("Device perm = 0x%x\n", dev_data->pdata.perm);
    pr_info("Device size = %d\n", dev_data->pdata.size);

    pr_info("DRIVER DATA: config_item_1 = %d\n", device_configs[pdev->id_entry->driver_data].config_item_1);
    pr_info("DRIVER DATA: config_item_2 = %d\n", device_configs[pdev->id_entry->driver_data].config_item_2);
    
    /* 3. Dynamically allocate memory for the device buffer using size information from the platform data */
    dev_data->buffer = (char *) devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
    if (!dev_data->buffer)
    {
        pr_info("Cannot allocate memory \n");
        ret = -ENOMEM;
        goto err_no_dev_memory;
    }
    /* 4. get the device number */
    dev_data->dev_num = pcdrv_data.device_num_base + pdev->id;

    /* 5. do cdev_init and cdev_add */
    cdev_init(&dev_data->cdev, &pcd_fOps);
    dev_data->cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if (ret < 0)
    {
        pr_err("cdev add failed\n");
        goto err_cdev_add;
    }
    /* 6. Create device file for the detected platform */
    dev_data->device_pcd = device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL, "rgbpcdev-%d", pdev->id);
    if (IS_ERR(dev_data->device_pcd))
    {
        pr_err("error Creating device \n");
        ret = PTR_ERR(dev_data->device_pcd);
        goto err_device_create;
    }
    /* 7. Error handling */
    pr_info("A device is probed: %s-%d\n", pdev->name, pdev->id);
    pcdrv_data.total_devices++;
    pr_info("Devices manged: %d\n", pcdrv_data.total_devices);

    return 0;
err_device_create:
    cdev_del(&dev_data->cdev);
err_cdev_add:
    devm_kfree(&pdev->dev, dev_data->buffer);
err_no_dev_memory:
    devm_kfree(&pdev->dev, dev_data);
err_no_memory:
err_no_pdata:
    pr_info("A device probe failed\n");
    return ret;
}

int pcd_platform_driver_remove(struct platform_device *pdev)
{
    /* extract the driver data from the pdev */
    struct pcdev_private_data * dev_data = (struct pcdev_private_data *) dev_get_drvdata(&pdev->dev);
    /* 1. remove a device that was created with device create */
    device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
    /* 2. remove cdev entry from the system */
    cdev_del(&dev_data->cdev);
    /* 
        3. free the memory held by the device 
        Not needed when using devm_APIs
        kfree(dev_data->buffer);
        kfree(dev_data);
    */
    

    pr_info("A device is removed: %s\n", pdev->name);
    pcdrv_data.total_devices--;
    pr_info("Devices manged: %d\n", pcdrv_data.total_devices);
    return 0;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a platform driver");
MODULE_AUTHOR("Ragab Hasssan");