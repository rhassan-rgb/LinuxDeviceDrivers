#ifndef RGB_PLATFORM_H
#define RGB_PLATFORM_H

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

#define RDONLY 0x01u
#define WRONLY 0x10u
#define RDWR 0x11u

struct pcdev_platform_data
{
    int size;
    int perm;
    const char* serial_number;
};

#endif