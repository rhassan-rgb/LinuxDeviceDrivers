obj-m := pcd_n.o
ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabihf-
KDIR=/home/rgb/workspace/ldd/source/6.1.80-ti-rt-r34/
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



