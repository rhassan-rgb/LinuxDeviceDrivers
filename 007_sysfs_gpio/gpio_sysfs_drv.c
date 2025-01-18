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
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__
/* module functions */
static int __init gpio_driver_init(void);
static void __exit pcd_driver_cleanup(void);

/* driver function */
static int gpio_driver_probe(struct platform_device *pdev);
static int gpio_driver_remove(struct platform_device *pdev);

/* attribute functions */
#define SHOW(_name) static ssize_t _name##_show (struct device *dev, struct device_attribute *attr, char *buf)
#define STORE(_name) static ssize_t _name##_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
SHOW(direction);
SHOW(value);
SHOW(label);

STORE(direction);
STORE(value);


/* per device private data <<dynamic>> */
struct gpiodev_private_data {
    char label[20];
    struct gpio_desc *desc;
};

/* driver private data <<static>>*/
struct gpiodrv_private_data
{
    int total_devices;
    struct class *class_gpio;
};

struct gpiodrv_private_data gpoi_driver_data;

struct of_device_id gipo_device_match_table[] = {
    {.compatible="rgb,bone-gpio-sysfs"},
    {}
};

struct platform_driver gpio_driver = {
    .probe = gpio_driver_probe,
    .remove = gpio_driver_remove,
    .driver = {
        .name = "rgb-gpio-driver",
        .of_match_table = of_match_ptr(gipo_device_match_table)
    }
};

/* attributes */
DEVICE_ATTR_RW(direction);
DEVICE_ATTR_RW(value);
DEVICE_ATTR_RO(label);

/* attributes list */
struct attribute *gpio_node_attrs[] = {
    &dev_attr_direction.attr,
    &dev_attr_label.attr,
    &dev_attr_value.attr,
    NULL
};

/* attribute group */
struct attribute_group gpio_dev_attr_group =  {
    .attrs = gpio_node_attrs,
};
/*attribute groups */
const struct attribute_group *gpio_dev_attr_groups[] = {
    &gpio_dev_attr_group,
    NULL
};

/* attribure functions */
/* size_t show (struct device *dev, struct device_attribute *attr, char *buf) */
SHOW(direction)
{
    /*extract device private data */
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
    /* get the direction */
    int direction = gpiod_get_direction(dev_data->desc);
    /* check for errors */
    if (direction < 0)
    {
        dev_err(dev,"cannot get gpio direction\n");
        return direction;
    }
    /* set direction_string*/
    return (direction == 0)? sprintf(buf, "out") : sprintf(buf, "in");
}

SHOW(value)
{
    /*extract device private data */
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
     /* get the value */
    int value = gpiod_get_value(dev_data->desc);
    /* check for errors */
    if (value)
    {
        dev_err(dev,"cannot get gpio direction\n");
        return value;
    }
    /* set direction_string*/
    return sprintf(buf, "%d", value);
}

SHOW(label)
{
    /*extract device private data */
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
    /* dump label in the buffer */
    return sprintf(buf, dev_data->label);
}
/*size_t store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)*/
STORE(direction)
{
    /*extract device private data */
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
    int ret;
    /* compare direction from the buffer */
    if(sysfs_streq("in", buf))
    {
        /* set gpio direction */
        ret = gpiod_direction_input(dev_data->desc);
        if(ret)
        {
            dev_err(dev, "cannot set dir\n");
            return ret;
        }
    }
    else if (sysfs_streq("out", buf))
    {
        /* set gpio direction */
        ret = gpiod_direction_output(dev_data->desc, 0);
        if(ret)
        {
            dev_err(dev, "cannot set dir\n");
            return ret;
        }
    }
    else 
    {
        dev_err(dev, "unsuppoertd value: %s\n", buf);
        return -EINVAL;
    }
    return count;
}
STORE(value)
{
    /*extract device private data */
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
    /* get the value */
    int value;
    int ret = kstrtol(buf, 10, (long*)&value);
    if(ret)
    {
        dev_err(dev, "cannot set value: %s\n", buf);
        return ret;
    }
    gpiod_set_value(dev_data->desc, value);
    return count;
}

int gpio_driver_probe(struct platform_device *pdev)
{
    /* get the dev */
    struct device *dev = &pdev->dev;
    /* holds the parent node */
    struct device_node *parent = dev->of_node;
    /* holds the child*/    
    struct device_node *child = NULL;

    /* hold return value */
    int ret = 0;

    /* holds the current index of a node */
    int i = 0;
    /* temp variable to hold device private data */
    struct gpiodev_private_data* dev_data = NULL;
    /* temp variable to hold the label */
    const char* of_label = NULL;
    /* temp variable to hold the sysfs device node */
    struct device *sysfs_dev = NULL;

    for_each_available_child_of_node(parent, child)
    {
        /* allocate memory for the device info */
        dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
        /* reset of label*/
        of_label = "";
        /* null means failure to allocate data */
        if(!dev_data)
        {
            dev_err(dev, "Cannot allocate memory for device data\n");
            return -ENOMEM;
        }
        /* non-zero means failure*/
        if(of_property_read_string(child, "label", &of_label))
        {
            dev_warn(dev, "Device label is missing, settign label to: unknowngpio-%d\n", i);
            /* set default label */
            snprintf(dev_data->label, sizeof(dev_data->label),"unknowngpio-%d", i);
        }
        else 
        {
            dev_info(dev, "GPIO Label: %s\n", of_label);
            /* copy label to driver data*/
            strcpy(dev_data->label, of_label);
        }
        /* get the GPIO info */
        dev_data->desc = devm_fwnode_gpiod_get(dev, &child->fwnode, "bone", GPIOD_ASIS, dev_data->label);
        if (IS_ERR(dev_data->desc))
        {
            ret = PTR_ERR(dev_data->desc);
            dev_err(dev, "Error occured while getting gpio info for : %s\n", child->name);           
            if (ret == -ENOENT)
                dev_err(dev, "No gpio entry found for : %s\n", child->name);    
            return ret;
        }
        /* set pin direction to output */
        ret = gpiod_direction_output(dev_data->desc, 0);
        if (ret)
        {
            dev_err(dev, "error while setting gpio direction\n");
            return ret;
        }

        /* create device using device create with groups */
        sysfs_dev = device_create_with_groups(gpoi_driver_data.class_gpio, dev, 0, dev_data, gpio_dev_attr_groups, dev_data->label);
        if (IS_ERR(sysfs_dev))
        {
            dev_err(dev, "Error creating sysfs device\n");
            return PTR_ERR(sysfs_dev);
        }

        i++;
        gpoi_driver_data.total_devices++;
    }
    /* for each available child node */
    return 0;
}
int device_unregister_wrapper(struct device* dev, void* data);
int device_unregister_wrapper(struct device* dev, void* data)
{
    struct gpiodev_private_data *dev_data = (struct gpiodev_private_data*)dev_get_drvdata(dev);
    dev_info(dev, "removing device: %s\n", dev_data->label);
    device_unregister(dev);
    return 0;
}
int gpio_driver_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "removing driver\n");
    device_for_each_child(&pdev->dev,NULL,device_unregister_wrapper);
    return 0;
}

int __init gpio_driver_init(void)
{
    /* holds the return value */
    int ret;
    
    gpoi_driver_data.total_devices = 0;
    /* creat the class in the sysfs */
    #ifdef HOST
    gpoi_driver_data.class_gpio = class_create("bone-gpios");
    #else
    gpoi_driver_data.class_gpio = class_create(THIS_MODULE, "bone-gpios");
    #endif
    /* check if class_create() returned error */
    if (IS_ERR(gpoi_driver_data.class_gpio))
    {
        pr_err("error Creating class \n");
        return PTR_ERR(gpoi_driver_data.class_gpio);
    }
    /* register the driver */
    ret = platform_driver_register(&gpio_driver);
    /* check if an error is returned */
    if (ret)
    {
        pr_err("error registering the driver \n");
        return ret;
    }
    return 0;

}


void __exit pcd_driver_cleanup(void)
{
    /* first unregister the platform driver */
    platform_driver_unregister(&gpio_driver);
    /* destroy the class*/
    class_destroy(gpoi_driver_data.class_gpio);
}





module_init(gpio_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is sysfs gpio platform driver");
MODULE_AUTHOR("Ragab Hasssan");