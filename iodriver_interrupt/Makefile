obj-m += m_test.o

all:
make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C /home/topic/rasp/kernel_new/linux M=$(PWD) modules

clean:
make -C /home/topic/rasp/kernel_new/linux M=$(PWD) clean

