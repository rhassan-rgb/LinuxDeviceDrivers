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
#include <linux/platform_device.h>
#include "platform.h"

void pcdev_release(struct device * dev);

/* 1.1 create platform data */
struct pcdev_platform_data pcdev_pdata[4] = {
    [0] = {.size = 512, .perm = RDWR, .serial_number = "RGBPCD001"},
    [1] = {.size = 1024, .perm = RDWR, .serial_number = "RGBPCD002"},
    [2] = {.size = 128, .perm = RDWR, .serial_number = "RGBPCD003"},
    [3] = {.size = 32, .perm = RDWR, .serial_number = "RGBPCD004"}
};

/* 1. Create 2 platform devices */

struct platform_device platform_pcdev_1 = {
    // .name = "pseudo-char-device",
    .name = "pcdev-A1x",
    .id = 0,
    .dev = {
        .platform_data = &pcdev_pdata[0],
        .release = pcdev_release
    }
};

struct platform_device platform_pcdev_2 = {
    // .name = "pseudo-char-device",
    .name = "pcdev-B1x",
    .id = 1,
    .dev = {
        .platform_data = &pcdev_pdata[1],\
        .release = pcdev_release
    }
};

struct platform_device platform_pcdev_3 = {
    // .name = "pseudo-char-device",
    .name = "pcdev-C1x",
    .id = 2,
    .dev = {
        .platform_data = &pcdev_pdata[2],\
        .release = pcdev_release
    }
};

struct platform_device platform_pcdev_4 = {
    // .name = "pseudo-char-device",
    .name = "pcdev-D1x",
    .id = 3,
    .dev = {
        .platform_data = &pcdev_pdata[3],\
        .release = pcdev_release
    }
};

struct platform_device *platform_pcdevs[] = {
    &platform_pcdev_1,
    &platform_pcdev_2,
    &platform_pcdev_3,
    &platform_pcdev_4
};

/* 2. Create init function */
static int __init pcdev_platform_init(void)
{
    /* 6. Register platform device */
    // platform_device_register(&platform_pcdev_1);
    // platform_device_register(&platform_pcdev_2);
    platform_add_devices(platform_pcdevs, ARRAY_SIZE(platform_pcdevs));

    pr_info("Setup module loaded successfully \n");
    return 0;
}


/* 3. Create exit function */
static void __exit pcdev_platform_exit(void)
{
    /* 6.2 Register platform device */
    platform_device_unregister(&platform_pcdev_1);
    platform_device_unregister(&platform_pcdev_2);
    platform_device_unregister(&platform_pcdev_3);
    platform_device_unregister(&platform_pcdev_4);
    pr_info("Setup module unloaded successfully \n");
    return;
}

/* 7. Release function */
void pcdev_release(struct device * dev)
{
    pr_info("Device is released\n");

};
/* 4. Register the init and exit */
module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

/* 5. Module info */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module which registers 2 platfrom devices");
MODULE_AUTHOR("Ragab Hassan");