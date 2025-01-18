obj-m := pcd_n.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) EXTRA_CFLAGS+="$(EXTRA_CFLAGS)" -C $(KDIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean
help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help



