#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define DEV_MEM_SIZE (512u)
#define BASE_NUMBER (0u)
#define DEVICE_COUNT (1u)

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open (struct inode *inode, struct file *filp);
int pcd_release (struct inode *inode, struct file *filp);

/* Module data r/w buffer*/
char device_buffer[DEV_MEM_SIZE];

/*this holds the device number*/
dev_t device_number;

/* cdev vriable that will be used to register our dev with the VFS*/
struct cdev pcd_cdev;

/* file_operations struct that will be used to hold the file operations of our dev*/
struct file_operations pcd_fOps = {
    .llseek = pcd_lseek,
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write
    };

/* sysfs class */
struct class* class_pcd = NULL;

/* sysfs device */
struct device* device_pcd = NULL;

static int __init pcd_driver_init(void)
{
    int ret;
    /*1. Dynamically allocate a device numer*/
    ret = alloc_chrdev_region(&device_number, BASE_NUMBER, DEVICE_COUNT, "pcd_devices");
    if (ret < 0)
    {
        pr_err("error allocating memory\n");
        goto err_alloc;
    }
    pr_info("Device Numer <Major>:<Minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));
    /*2. Initialize cdev structure */
    cdev_init(&pcd_cdev, &pcd_fOps);
    pcd_cdev.owner = THIS_MODULE;

    /*3. Register the device with VFS*/
    ret = cdev_add(&pcd_cdev, device_number, DEVICE_COUNT);
    if (ret < 0)
    {
        pr_err("error registering device\n");
        goto err_add;
    }
    /*4. Create device class under /sys/class/ */
    class_pcd = class_create("pcd_class");
    if (IS_ERR(class_pcd))
    {
        pr_err("error Creating class \n");
        ret = PTR_ERR(class_pcd);
        goto err_class_create;
    }
    /*5. Create device under /sys/class/my_class */
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
    if (IS_ERR(device_pcd))
    {
        pr_err("error Creating device \n");
        ret = PTR_ERR(device_pcd);
        goto err_device_create;
    }
    pr_info("Module loaded successfully\n");
    
    return 0;
err_device_create:
    class_destroy(class_pcd);
err_class_create:
    cdev_del(&pcd_cdev);
err_add:
    unregister_chrdev_region(device_number, DEVICE_COUNT);
err_alloc:
    return ret;

}

static void __exit pcd_driver_cleanup(void)
{
    /*1. destroy device */
    device_destroy(class_pcd, device_number);

    /*2. destroy class*/
    class_destroy(class_pcd);

    /*3. delete device*/
    cdev_del(&pcd_cdev);

    /*4. Deallocate cdar dev region*/
    unregister_chrdev_region(device_number, DEVICE_COUNT);
    pr_info("Module unloaded successfully\n");

}

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    loff_t temp;

    pr_info("lseek requested with offset %lld\n", offset);
    pr_info("Initial value of the file pointer %lld\n", filp->f_pos);

    switch (whence)
    {
        case SEEK_SET:
            if ((offset > DEV_MEM_SIZE) || (offset < 0))
                return -EINVAL;
            pr_info("SET value of the file pointer %lld\n", offset);
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > DEV_MEM_SIZE) || (temp < 0))
                return -EINVAL;

            pr_info("CUR value of the file pointer %lld\n", offset);
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = DEV_MEM_SIZE + offset;
            if ((temp > DEV_MEM_SIZE) || (temp < 0))
                return -EINVAL;
            pr_info("END value of the file pointer %lld\n", offset);

            filp->f_pos = temp;
            break;

        default:
            return -EINVAL;

    }
    pr_info("Final value of the file pointer %lld\n", filp->f_pos);
    return filp->f_pos;
}
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos)
{
    pr_info("Read requested for %zu bytes \n", count);
    pr_info("Position before read %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > DEV_MEM_SIZE)
    {
        pr_info("Requested count is out of boundary \n");
        count = DEV_MEM_SIZE - *f_pos;
    }

    /* Copy to user */
    if (copy_to_user(buff, &(device_buffer[*f_pos]), count))
    {
        pr_err("Error copying to user \n");
        return -EFAULT;
    }

    /* Update f_pos */
    *f_pos += count;
    pr_info("Position after read %lld \n", *f_pos);

    /* return the number of character successfully read*/
    return count;
}
ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Wrire requested for %zu bytes \n", count);
    pr_info("Position before writing %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > DEV_MEM_SIZE)
    {
        pr_info("Requested count is out of boundary \n");
        count = DEV_MEM_SIZE - *f_pos;
    }

    /* Copy from user */
    if (copy_from_user(&(device_buffer[*f_pos]),buff, count))
    {
        pr_err("Error copying from user \n");
        return -EFAULT;
    }

    /* Update f_pos */
    *f_pos += count;
    pr_info("Position after writing %lld \n", *f_pos);

    /* return the number of character successfully read*/
    return count;
}
int pcd_open (struct inode *inode, struct file *filp)
{
    pr_info("PCD file oped successfully!\n");
    return 0;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("Close requested\n");
    return 0;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");