obj-m := pcd_platform_driver_dt.o pcd_device_setup.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KDIR=/home/rgb/workspace/ldd/source/beagle/
HOST_KDIR=/lib/modules/$(shell uname -r)/build/



all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help
host:
	echo $(shell uname -r)
	make -C $(HOST_KDIR) M=$(PWD) modules
hostclean:
	make -C $(HOST_KDIR) M=$(PWD) clean
copy-dtb:
	scp /home/rgb/workspace/ldd/source/beagle/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.7.2:/home/debian/drivers/
copy-drivers:
	scp *.ko debian@192.168.7.2:/home/debian/drivers/


