mkdir temp

export PATH="$HOME/opt/cross/bin:$PATH"

# Build bootloader
# i686-elf-as bootloader/boot.S -o temp/boot.o
nasm -f elf32 bootloader/boot.S -o temp/boot.o

OBJECT_FILES="temp/boot.o"

# Build all C files
for i in $(find kernel/ -name '*.c')
do
    filename_no_dir=$(basename -- "$i")
    filename_no_extension="${filename_no_dir%.*}"
    object_file=" temp/${filename_no_extension}.o"
    OBJECT_FILES+=$object_file
    i686-elf-gcc -c "$i" -c -o $object_file -std=gnu99 -ffreestanding -O2
done

# Build all Assembly files
for i in $(find kernel/ -name '*.S')
do
    filename_no_dir=$(basename -- "$i")
    filename_no_extension="${filename_no_dir%.*}"
    object_file=" temp/${filename_no_extension}s.o"
    OBJECT_FILES+=$object_file
    nasm -f elf32 "$i" -o $object_file
done

# Link all together
i686-elf-gcc -T linker.ld -o paros.bin -ffreestanding -O2 -nostdlib $OBJECT_FILES -lgcc

rm -rf temp

rm -rf isodir

mkdir -p isodir/boot/grub
cp paros.bin isodir/boot/paros.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o paros.iso isodir
