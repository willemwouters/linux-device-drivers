cd qemu-rpi
	sudo ./arm-softmmu/qemu-system-arm -kernel ../bootfs/kernel-qemu -cpu arm1176 -m 256 -M versatilepb -no-reboot -serial stdio -append "root=/dev/sda panic=0 rootfstype=ext4 rw init=/bin/bash" -hda ~/rasp/bootfs/rootfs.img
