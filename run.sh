./build.sh
qemu-system-i386 -m 4g -cdrom paros.iso -d int -no-shutdown -no-reboot -M smm=off -drive file=disk.img,format=raw,if=ide,media=disk,index=0
