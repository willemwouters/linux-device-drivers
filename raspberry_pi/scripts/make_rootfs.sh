cd /home/topic/rasp

debootstrap=false
read -r -p "Do you want to download new rootfs file?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
  debootstrap=true
  sudo debootstrap --include=gedit --foreign --no-check-gpg --include=ca-certificates --arch=armhf testing rootfs http://mirror.nl.leaseweb.net/raspbian/raspbian/
fi


echo "Adding config file qemu-binfmt.conf"
#su -l
#sudo echo 'EXTRA_OPTS="-L/usr/lib/arm-linux-gnueabihf"' > /etc/qemu-binfmt.conf
#exit


echo "Copy qemu-arm-static to rootfs"
sudo cp $(which qemu-arm-static) rootfs/usr/bin

if [ debootstrap = true ]; then
echo "Finalize debootstrap"
sudo chroot rootfs/ /debootstrap/debootstrap --second-stage --verbose
fi

read -r -p "Do you want to download firmware files?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
	sudo rm -R firmware
	sudo git clone https://github.com/raspberrypi/firmware.git
	sudo cp -R firmware/hardfp/opt/* rootfs/opt/
fi


read -r -p "Do you want to set new password?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
	sudo chroot rootfs/ /usr/bin/passwd
fi

echo "Remove qemu-arm-static from rootfs"
sudo rm rootfs/usr/bin/qemu-arm-static

read -r -p "Do you want pack to rootfs?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
	cd /home/topic/rasp
	sudo qemu-img create -f raw rootfs.img 1G
	sudo mkfs.ext4 -F rootfs.img 
	sudo mkdir /mnt/rootfs
	sudo mount rootfs.img /mnt/rootfs -o loop
	sudo cp -R ./rootfs/* /mnt/rootfs
	sudo umount /mnt/rootfs
	
	sudo rm ./bootfs/rootfs.img
	cp ./rootfs.img ./bootfs/
	sudo rm ./rootfs.img
fi



read -r -p "Do you start emulation ?? [y/N]" response
response=${response,,} # tolower
if [[ $response =~ ^(yes|y) ]]; then
	cd qemu-rpi
	sudo ./arm-softmmu/qemu-system-arm -kernel ../bootfs/kernel-qemu -cpu arm1176 -m 256 -M versatilepb -no-reboot -serial stdio -append "root=/dev/sda panic=0 rootfstype=ext4 rw init=/bin/bash" -hda ~/rasp/bootfs/rootfs.img
fi


