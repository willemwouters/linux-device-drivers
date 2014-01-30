ARCH=arm
TARGET = iodriver
obj-m += $(TARGET).o
iodriver-objs += ./signal_misc.o ./gpio_misc.o ./proc_misc.o ./main.o


CCPREFIX=/home/topic/rasp/kernel_new/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
KDIR = /home/topic/rasp/kernel_new/linux
PWD := $(shell pwd)
BASE=/home/topic/linux_device_driver/raspberry_pi/iodriver_proc/kernel_interrupt



default:
	make ARCH=$(ARCH) CROSS_COMPILE=${CCPREFIX} -C $(KDIR) SUBDIRS=$(BASE) modules

clean: 
	rm *.o; rm *.ko; make ARCH=$(ARCH) CROSS_COMPILE=${CCPREFIX} -C $(KDIR) SUBDIRS=$(BASE) clean

$(TARGET).o : $(OBJS)

