./build.sh
qemu-system-i386 -m 4g -cdrom paros.iso -drive file=disk.img,format=raw,if=ide,media=disk,index=0
