./build.sh
./create_iso.sh
# qemu-system-x86_64 -m 4g -serial stdio -d int -M smm=off -cdrom paros.iso -no-reboot -s -S
# qemu-system-x86_64 -m 4g -serial stdio -d int -M smm=off -cdrom paros.iso
qemu-system-x86_64 -m 4g -serial stdio -d int -M smm=off -cdrom paros.iso -smp 2
# qemu-system-x86_64 -m 4g -serial stdio -cdrom paros.iso -no-reboot
