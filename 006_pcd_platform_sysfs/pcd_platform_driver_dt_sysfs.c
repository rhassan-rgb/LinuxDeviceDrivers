#include "pcd_platform_driver_dt_sysfs.h"

/* file_operations struct that will be used to hold the file operations of our dev*/
struct file_operations pcd_fOps = {
    .llseek = pcd_lseek,
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write
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
/* attribute functions */
ssize_t show_max_size(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t store_max_size(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

ssize_t show_serial_number(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t store_serial_number(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
/* Helper functions */
int pcd_create_attribute_files(struct device* dev);

ssize_t show_max_size(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* access device data */
    struct pcdev_private_data *priv_data = dev_get_drvdata(dev->parent);
    return sprintf(buf, "%d\n", priv_data->pdata.size);
}
ssize_t show_serial_number(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* access device data */
    struct pcdev_private_data *priv_data = dev_get_drvdata(dev->parent);
    return sprintf(buf, "%s\n", priv_data->pdata.serial_number);
}

ssize_t store_max_size(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    /* access device data */
    struct pcdev_private_data *priv_data = dev_get_drvdata(dev->parent);
    long new_size;
    int ret = kstrtol(buf, 10, &new_size);
    if (ret < 0)
    {
        return ret;
    }
    priv_data->pdata.size = new_size;
    dev_info(dev->parent, "new buffer size %ld\n", new_size);
    priv_data->buffer = devm_krealloc(dev->parent, priv_data->buffer, new_size, GFP_KERNEL);
    dev_info(dev->parent, "new buffer location %p\n", priv_data->buffer);

    return count;
}

ssize_t store_serial_number(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/*create the attributes*/
static DEVICE_ATTR(max_size, (S_IRUGO | S_IWUSR), show_max_size, store_max_size);
static DEVICE_ATTR(serial_number, S_IRUGO , show_serial_number, store_serial_number);


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
int pcd_create_attribute_files(struct device* dev)
{
    int ret;
    ret = sysfs_create_file(&dev->kobj, &dev_attr_max_size.attr);
    if (ret < 0)
    {
        return ret;
    }
    return sysfs_create_file(&dev->kobj, &dev_attr_serial_number.attr);
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
        driver_data = (size_t)of_device_get_match_data(dev);

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
    dev_data->device_pcd = device_create(pcdrv_data.class_pcd, dev, dev_data->dev_num, NULL, "rgbpcdev-%d", pcdrv_data.total_devices);
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
    /* 8. Create the attributes */
    ret = pcd_create_attribute_files(dev_data->device_pcd);
    if (ret < 0)
    {
        dev_err(dev, "failed to create attributes");
        goto err_attr_create;
    }
    dev_info(dev, "pdev->dev: 0x%p, dev_data->device_pcd: 0x%p", &pdev->dev, dev_data->device_pcd);
    return 0;
err_attr_create:
    device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
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