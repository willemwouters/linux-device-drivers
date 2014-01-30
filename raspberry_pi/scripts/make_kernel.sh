cd ~/rasp/linux

read -r -p "Do you want to generate a default config?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
  make ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi- bcmrpi_cutdown_defconfig
fi

read -r -p "Do you want to adjust your config?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
     make ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi- menuconfig
fi


read -r -p "Do you want to clean the build?? [y/N]" response
 response=${response,,} # tolower
 if [[ $response =~ ^(yes|y) ]]; then
     make clean
 fi

read -r -p "Do you want to start compiling???? [Y/n]" response
 response=${response,,} # tolower
 if [ -z $response ]; then
     response=y
 fi
 if [[ $response =~ (yes|y) ]]; then
     make ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi- -k -j8
 fi

read -r -p "Do you want to pack the kernel?? [Y/n]" response
 response=${response,,} # tolower
 if [ -z $response ]; then
     response=y
 fi
 if [[ $response =~ ^(yes|y|" ") ]]; then
    	mkdir ../modules
	make modules_install ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi- INSTALL_MOD_PATH=../modules/
	cd ../tools/mkimage/
	./imagetool-uncompressed.py ../../linux/arch/arm/boot/Image
	rm ../../bootfs/kernel.img
	cp kernel.img ../../bootfs/kernel.img
 fi

