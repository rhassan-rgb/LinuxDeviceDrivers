#include "pcd_platform_driver_dt_sysfs.h"
static int check_permission(int dev_perm, int access_mode);

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

    dev_info(dev, "Max size  %d bytes \n", pcdev_data->pdata.size);
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
    dev_info(dev, "Max size  %d bytes \n", pcdev_data->pdata.size);
    dev_info(dev, "%s: Wrire requested for %zu bytes \n", pcdev_data->pdata.serial_number, count);
    dev_info(dev, "Position before writing %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > max_size)
    {
        dev_info(dev, "Requested count is out of boundary \n");
        if (*f_pos >= max_size)
        {
            /* discard writing */
            return count;
        }
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