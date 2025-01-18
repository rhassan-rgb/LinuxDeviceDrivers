#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define RDONLY 0x01u
#define WRONLY 0x10u
#define RDWR 0x11u

#define DEV0_MEM_SIZE (1024u)
#define DEV1_MEM_SIZE (512u)
#define DEV2_MEM_SIZE (1024u)
#define DEV3_MEM_SIZE (512u)

#define BASE_NUMBER (0u)
#define DEVICE_COUNT (4u)

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence);
ssize_t pcd_read (struct file * filp, char __user *buff, size_t count, loff_t * f_pos);
ssize_t pcd_write (struct file * filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open (struct inode *inode, struct file *filp);
int pcd_release (struct inode *inode, struct file *filp);

/* Module data r/w buffer*/
char device0_buffer[DEV0_MEM_SIZE];
char device1_buffer[DEV1_MEM_SIZE];
char device2_buffer[DEV2_MEM_SIZE];
char device3_buffer[DEV3_MEM_SIZE];

/* Device private data */
struct pcdev_private_data
{
    /* per device buffer */
    char* buffer;
    /* size of the device buffer */
    unsigned size;
    /* unique identifier of the device node */
    const char *serial_number;
    /* memory access permission of this device */
    int perm;
    /* cdev vriable that will be used to register our dev with the VFS*/
    struct cdev cdev;
    /* sysfs device */
    struct device* device_pcd;
};

/* Driver private data */
struct pcdrv_private_data
{
    /* number of devices managed by this driver */
    int total_devices;
    /* Devices' private data reference */
    struct pcdev_private_data pcdev_data[DEVICE_COUNT];
    /*this holds the device number => Module thing*/
    dev_t device_number;
    /* sysfs class */
    struct class* class_pcd;
};

/* Driver information data structure instance */
struct pcdrv_private_data pcdrv_data = 
{
    .total_devices = DEVICE_COUNT,
    .pcdev_data = {
        [0] = {
            .buffer = device0_buffer,
            .size = DEV0_MEM_SIZE,
            .serial_number = "RGBPCD0",
            .perm = RDONLY
        },
        [1] = {
            .buffer = device1_buffer,
            .size = DEV1_MEM_SIZE,
            .serial_number = "RGBPCD1",
            .perm = WRONLY
        },
        [2] = {
            .buffer = device2_buffer,
            .size = DEV2_MEM_SIZE,
            .serial_number = "RGBPCD2",
            .perm = RDWR
        },
        [3] = {
            .buffer = device3_buffer,
            .size = DEV3_MEM_SIZE,
            .serial_number = "RGBPCD3",
            .perm = RDWR
        }
    }
};

/* file_operations struct that will be used to hold the file operations of our dev*/
struct file_operations pcd_fOps = {
    .llseek = pcd_lseek,
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write
    };


static int __init pcd_driver_init(void)
{
    int ret;
    int i;
    /*1. Dynamically allocate a device numer*/
    ret = alloc_chrdev_region(&pcdrv_data.device_number, BASE_NUMBER, DEVICE_COUNT, "pcd_devices");
    if (ret < 0)
    {
        pr_err("error allocating memory\n");
        goto err_alloc;
    }
    for (i = 0; i < DEVICE_COUNT; i++)
        pr_info("Device Numer <Major>:<Minor> = %d:%d\n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));
    
    /*4. Create device class under /sys/class/ */
    pcdrv_data.class_pcd = class_create("pcdrv_class");
    if (IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("error Creating class \n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto err_class_create;
    }
    
    for (i = 0; i < DEVICE_COUNT; i++)
    {
        /*2. Initialize cdev structure */
        cdev_init(&(pcdrv_data.pcdev_data[i].cdev), &pcd_fOps);
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
        /*3. Register the device with VFS*/
        ret = cdev_add(&(pcdrv_data.pcdev_data[i].cdev), pcdrv_data.device_number + i, 1);
        if (ret < 0)
        {
            pr_err("error registering device\n");
            goto err_add;
        }
        /*5. Create device under /sys/class/my_class */
        pcdrv_data.pcdev_data[i].device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, "pcd-%d", i);
        if (IS_ERR(pcdrv_data.pcdev_data[i].device_pcd))
        {
            pr_err("error Creating device \n");
            ret = PTR_ERR(pcdrv_data.pcdev_data[i].device_pcd);
            goto err_device_create;
        }
    }

    pr_info("Module loaded successfully\n");
    
    return 0;
err_device_create:
err_add:
    for (;i >= 0; i--)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number+i);
        cdev_del(&(pcdrv_data.pcdev_data[i].cdev));
    }
    class_destroy(pcdrv_data.class_pcd);
err_class_create:
    unregister_chrdev_region(pcdrv_data.device_number, DEVICE_COUNT);
err_alloc:
    return ret;
}

static void __exit pcd_driver_cleanup(void)
{
    int i;
    for (i = 0;i < DEVICE_COUNT; i++)
    {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number+i);
        cdev_del(&(pcdrv_data.pcdev_data[i].cdev));
    }
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number, DEVICE_COUNT);
    pr_info("Module unloaded successfully\n");

}

/* File Methods */
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;

    loff_t temp;

    pr_info("%s: lseek requested with offset %lld\n", pcdev_data->serial_number, offset);
    pr_info("Initial value of the file pointer %lld\n", filp->f_pos);

    switch (whence)
    {
        case SEEK_SET:
            if ((offset > max_size) || (offset < 0))
                return -EINVAL;
            pr_info("SET value of the file pointer %lld\n", offset);
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;

            pr_info("CUR value of the file pointer %lld\n", offset);
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if ((temp > max_size) || (temp < 0))
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
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;

    pr_info("%s: Read requested for  %zu bytes \n", pcdev_data->serial_number, count);
    pr_info("Position before read %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > max_size)
    {
        pr_info("Requested count is out of boundary \n");
        count = max_size - *f_pos;
    }

    /* Copy to user */
    if (copy_to_user(buff, &(pcdev_data->buffer[*f_pos]), count))
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
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;

    int max_size = pcdev_data->size;
    pr_info("%s: Wrire requested for %zu bytes \n", pcdev_data->serial_number, count);
    pr_info("Position before writing %lld \n", *f_pos);
    
    /* Adjust the count */
    if ((count + *f_pos) > max_size)
    {
        pr_info("Requested count is out of boundary \n");
        if (*f_pos >= max_size)
        {
            /* discard writing */
            return count;
        }
        count = max_size - *f_pos;

    }
    }

    /* Copy from user */
    if (copy_from_user(&(pcdev_data->buffer[*f_pos]),buff, count))
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

int check_permission(int dev_perm, int access_mode)
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
    struct pcdev_private_data *pcdev_data = NULL;
    /* find out on which device file the open operation was attempted from the user space */
    minor_number = MINOR(inode->i_rdev);
    pr_info("Minor Access: %d\n", minor_number);

    /* get device private data structure */
    pcdev_data = container_of(inode->i_cdev,struct pcdev_private_data, cdev);
    /* save private data of this file in the file pointer so other methods can access it */
    filp->private_data = pcdev_data;
    /* check permission */
    ret = check_permission(pcdev_data->perm, filp->f_mode);
    (!ret)? pr_info("PCD %d file oped successfully!\n", minor_number) : pr_info("PCD %d file failed to open!\n", minor_number);

    return ret;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("Close requested\n");
    return 0;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a test module with multiple devices");
MODULE_AUTHOR("Ragab Hasssan");