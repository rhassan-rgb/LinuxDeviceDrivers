#####################################################################################
# This file is part of Linux Device Drivers (LDD) project.                          #
#                                                                                   #
# Linux Device Drivers is free software: you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by              #
# the Free Software Foundation, either version 3 of the License, or                 #
# (at your option) any later version.                                               #
#                                                                                   #
# Linux Device Drivers is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                    #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                      #
# GNU General Public License for more details.                                      #
#                                                                                   #
# You should have received a copy of the GNU General Public License                 #
# along with Linux Device Drivers. If not, see <https://www.gnu.org/licenses/>.     #
#####################################################################################

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


