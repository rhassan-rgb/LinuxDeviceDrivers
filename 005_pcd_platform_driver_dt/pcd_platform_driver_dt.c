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

int pcd_platform_driver_probe(struct platform_device *);
int pcd_platform_driver_remove(struct platform_device *);

/* helper functions */
struct pcdev_platform_data * pcdev_get_platfrom_from_dt(struct device *dev);
static int check_permission(int dev_perm, int access_mode);

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
/* device-driver data*/
struct device_configuration device_configs[] = 
{
    [DEV_A1X] = {.config_item_1 = 10, .config_item_2 = 100},
    [DEV_B1X] = {.config_item_1 = 20, .config_item_2 = 200},
    [DEV_C1X] = {.config_item_1 = 30, .config_item_2 = 300},
    [DEV_D1X] = {.config_item_1 = 40, .config_item_2 = 400}
};

/* when devices have IDs*/
struct platform_device_id pcd_id_table[] = 
{
    {.name = "pcdev-A1x", .driver_data = DEV_A1X}, 
    {.name = "pcdev-B1x", .driver_data = DEV_B1X}, 
    {.name = "pcdev-C1x", .driver_data = DEV_C1X},
    {.name = "pcdev-D1x", .driver_data = DEV_D1X},
    {}
};
/* Match table */
struct of_device_id pcd_of_match_table [] = {
    {.compatible = "pcdev-A1x", .data = (void*) DEV_A1X},
    {.compatible = "pcdev-B1x", .data = (void*) DEV_B1X},
    {.compatible = "pcdev-C1x", .data = (void*) DEV_C1X},
    {.compatible = "pcdev-D1x", .data = (void*) DEV_D1X},
    {}
};

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .id_table = pcd_id_table,
    .driver = {
        .name = "pseudo-char-device",
        .of_match_table = of_match_ptr(pcd_of_match_table)
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
    #ifdef HOST
    pcdrv_data.class_pcd = class_create("rgb_chrdev_class");
    #else
    pcdrv_data.class_pcd = class_create(THIS_MODULE, "rgb_chrdev_class");
    #endif
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
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    struct device *dev = pcdev_data->device_pcd;
    int max_size = pcdev_data->pdata.size;

    loff_t temp;

    dev_info(dev, "%s: lseek requested with offset %lld\n", pcdev_data->pdata.serial_number, offset);
    dev_info(dev, "Initial value of the file pointer %lld\n", filp->f_pos);

    switch (whence)
    {
        case SEEK_SET:
            if ((offset > max_size) || (offset < 0))
                return -EINVAL;
            dev_info(dev, "SET value of the file pointer %lld\n", offset);
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;

            dev_info(dev, "CUR value of the file pointer %lld\n", offset);
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            dev_info(dev, "END value of the file pointer %lld\n", offset);

            filp->f_pos = temp;
            break;

        default:
            return -EINVAL;

    }
    dev_info(dev, "Final value of the file pointer %lld\n", filp->f_pos);
    return filp->f_pos;
}
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    struct device *dev = pcdev_data->device_pcd;
    int max_size = pcdev_data->pdata.size;

    dev_info(dev, "%s: Read requested for  %zu bytes \n", pcdev_data->pdata.serial_number, count);
    dev_info(dev, "Position before read %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > max_size)
    {
        dev_info(dev, "Requested count is out of boundary \n");
        count = max_size - *f_pos;
    }

    /* Copy to user */
    if (copy_to_user(buff, &(pcdev_data->buffer[*f_pos]), count))
    {
        dev_err(dev, "Error copying to user \n");
        return -EFAULT;
    }

    /* Update f_pos */
    *f_pos += count;
    dev_info(dev, "Position after read %lld \n", *f_pos);

    /* return the number of character successfully read*/
    return count;
}

ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    /* added during open */
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    /* allocated during probe */
    struct device *dev = pcdev_data->device_pcd;
    
    int max_size = pcdev_data->pdata.size;
    dev_info(dev, "%s: Wrire requested for %zu bytes \n", pcdev_data->pdata.serial_number, count);
    dev_info(dev, "Position before writing %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > max_size)
    {
        dev_info(dev, "Requested count is out of boundary \n");
        count = max_size - *f_pos;
    }

    /* Copy from user */
    if (copy_from_user(&(pcdev_data->buffer[*f_pos]),buff, count))
    {
        dev_err(dev, "Error copying from user \n");
        return -EFAULT;
    }

    /* Update f_pos */
    *f_pos += count;
    dev_info(dev, "Position after writing %lld \n", *f_pos);

    /* return the number of character successfully read*/
    return count;
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
    int ret;
    int minor_number;
    struct pcdev_private_data *pcdev_data = NULL; /* contained in driver data of struct platform_device->struct device dev -> void* driver_data */
    struct device *dev = NULL;

    /* get device private data structure */
    pcdev_data = container_of(inode->i_cdev,struct pcdev_private_data, cdev);
    
    /* get pointer to struct device data structure */
    /* allocated during probe */
    dev = pcdev_data->device_pcd;

    /* find out on which device file the open operation was attempted from the user space */
    minor_number = MINOR(inode->i_rdev);
    dev_info(dev, "Minor Access: %d\n", minor_number);

    /* save private data of this file in the file pointer so other methods can access it */
    filp->private_data = pcdev_data;
    /* check permission */
    ret = check_permission(pcdev_data->pdata.perm, filp->f_mode);
    (!ret)? dev_info(dev, "PCD %d file oped successfully!\n", minor_number) : dev_info(dev, "PCD %d file failed to open!\n", minor_number);

    return ret;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("Close requested\n");
    return 0;
}
struct pcdev_platform_data * pcdev_get_platfrom_from_dt(struct device *dev)
{
    struct device_node* dev_node = dev->of_node;
    struct pcdev_platform_data *pdata;
    /* check if it's a DT node or a notmal module device*/
    if (!dev_node)
    {
        /* Device is not from device tree */
        dev_info(dev,"Device is not from DT: %s \n", dev->init_name);
        return NULL;
    }
    /* allocate memory for the platform device data <<our custom platform data>>*/
    pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
    {
        dev_info(dev, "Cannot allocate memory\n");
        return ERR_PTR(-ENOMEM);
    }
    /* read serial number*/
    if(of_property_read_string(dev_node, "rgb,device-serial-number", &pdata->serial_number))
    {
        dev_info(dev, "missing Serial Number Property\n");
        return ERR_PTR(-EINVAL);
    }
    /* read size*/
    if(of_property_read_u32(dev_node, "rgb,size", &pdata->size))
    {
        dev_info(dev, "missing size Property\n");
        return ERR_PTR(-EINVAL);
    }
    /* read permission*/
    if(of_property_read_u32(dev_node, "rgb,perm", &pdata->perm))
    {
        dev_info(dev, "missing permission Property\n");
        return ERR_PTR(-EINVAL);
    }
    return pdata;
}
int pcd_platform_driver_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data * pdata = NULL;
    /* holds driver data index -> identifier for the device so the driver can handle it properly */
    int driver_data;
    /*
        holds the matched device from a DT and matching table
        not needed if we will use of_device_get_match_data
    */
    /*
    struct of_device_id *match;
    */
    /* holds a pointer to device */
    struct device *dev = &pdev->dev;

    dev_info(dev,"A device is detected: %s-%d\n", pdev->name, pcdrv_data.total_devices);
    /* 
        1. get the platform data 
        Will be different in case of DT device
    */
    pdata = pcdev_get_platfrom_from_dt(dev);
    /* if error occured */
    if(IS_ERR(pdata))
    {
        dev_info(dev, "Error occured: %s\n", pdev->name);
        return -EINVAL;
    }
    /* device instantiation didn't happen from a DT*/
    if (!pdata)
    {
        pdata = (struct pcdev_platform_data *) dev_get_platdata(dev);
        if (!pdata)
        {
            dev_info(dev,"No Platfrom Data available\n");
            ret = -EINVAL;
            goto err_no_pdata;
        }
        driver_data = pdev->id_entry->driver_data;
    }
    else {
        /* a DT instantiation */
        /* get the device match of the match table */
        /* pcd_of_match_table -> can be fetched from: pdev->dev.driver->of_match_table */ 
        /* 
            in non DT nodes the driver data is automatically embedded in the id_entry during the matching \
            using the id_table in the struct platform_driver 
        */
        /* 
        match = of_match_device(pdev->dev.driver->of_match_table, &pdev->dev);
        driver_data = (int)match->data;
        */
        driver_data = (int)of_device_get_match_data(dev);

    }
    /* 2. Dynamically allocate memory for the private data */
    dev_data = (struct pcdev_private_data *) devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data)
    {
        dev_info(dev, "Cannot allocate memory \n");
        ret = -ENOMEM;
        goto err_no_memory;
    }
    /* save the allocated pointer in the driver_data inside the dev member of pdev in order to use it later at removal */
    /* pdev->dev.driver_data = dev_data; */
    dev_set_drvdata(dev, dev_data);

    dev_data->pdata.size  = pdata->size;
    dev_data->pdata.perm  = pdata->perm;
    dev_data->pdata.serial_number  = pdata->serial_number;

    dev_info(dev, "Device serial_number = %s\n", dev_data->pdata.serial_number);
    dev_info(dev, "Device perm = 0x%x\n", dev_data->pdata.perm);
    dev_info(dev, "Device size = %d\n", dev_data->pdata.size);

    dev_info(dev, "DRIVER DATA: config_item_1 = %d\n", device_configs[driver_data].config_item_1);
    dev_info(dev, "DRIVER DATA: config_item_2 = %d\n", device_configs[driver_data].config_item_2);
    
    /* 3. Dynamically allocate memory for the device buffer using size information from the platform data */
    dev_data->buffer = (char *) devm_kzalloc(dev, dev_data->pdata.size, GFP_KERNEL);
    if (!dev_data->buffer)
    {
        dev_info(dev, "Cannot allocate memory \n");
        ret = -ENOMEM;
        goto err_no_dev_memory;
    }
    /* 4. get the device number */
    dev_data->dev_num = pcdrv_data.device_num_base + pcdrv_data.total_devices;

    /* 5. do cdev_init and cdev_add */
    cdev_init(&dev_data->cdev, &pcd_fOps);
    dev_data->cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if (ret < 0)
    {
        dev_err(dev, "cdev add failed\n");
        goto err_cdev_add;
    }
    /* 6. Create device file for the detected platform */
    dev_data->device_pcd = device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL, "rgbpcdev-%d", pcdrv_data.total_devices);
    if (IS_ERR(dev_data->device_pcd))
    {
        dev_err(dev, "error Creating device \n");
        ret = PTR_ERR(dev_data->device_pcd);
        goto err_device_create;
    }
    /* 7. Error handling */
    dev_info(dev, "A device is probed: %s-%d\n", pdev->name, pdev->id);
    pcdrv_data.total_devices++;
    dev_info(dev, "Devices manged: %d\n", pcdrv_data.total_devices);

    return 0;
err_device_create:
    cdev_del(&dev_data->cdev);
err_cdev_add:
    devm_kfree(dev, dev_data->buffer);
err_no_dev_memory:
    devm_kfree(dev, dev_data);
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
    

    dev_info(&pdev->dev, "A device is removed: %s\n", pdev->name);
    pcdrv_data.total_devices--;
    dev_info(&pdev->dev, "Devices manged: %d\n", pcdrv_data.total_devices);
    return 0;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a platform driver");
MODULE_AUTHOR("Ragab Hasssan");