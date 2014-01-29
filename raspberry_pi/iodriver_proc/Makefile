
userspace_mod:
/home/topic/rasp/kernel_new/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-g++ ./userspace.c  -o ./userpace

kernel_mod:
make  ARCH=arm CROSS_COMPILE=/home/topic/rasp/kernel_new/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi- -C /home/topic/rasp/kernel_new/linux M=/home/topic/rasp/iodriver modules CONFIG_DEBUG_INFO=y 

clean:
make clean  ARCH=arm CROSS_COMPILE=/home/topic/rasp/kernel_new/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi- -C /home/topic/rasp/kernel_new/linux M=/home/topic/rasp/iodriver modules CONFIG_DEBUG_INFO=y 
