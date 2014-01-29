obj-m = myDev.o

all: test compile install 

compile:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
install:
	sudo rmmod myDev
	sudo insmod myDev.ko
	sudo chmod a+r+w /dev/myDev

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:test.c
	cc -o test test.c
