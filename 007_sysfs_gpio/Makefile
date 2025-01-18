obj-m := gpio_sysfs_drv.o


all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) EXTRA_CFLAGS+="$(EXTRA_CFLAGS)" -C $(KDIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help

copy-dtb:
	scp /home/rgb/workspace/ldd/source/beagle/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.7.2:/home/debian/drivers/
copy-drivers:
	scp *.ko debian@192.168.7.2:/home/debian/drivers/


