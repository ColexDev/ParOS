./build.sh
qemu-system-i386 -m 4g -cdrom paros.iso -d int -machine smm=off -no-reboot -no-shutdown -drive file=disk.img,format=raw,if=ide,media=disk,index=0
