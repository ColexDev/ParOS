mkdir temp

OBJECT_FILES=""

# Build all C files
for i in $(find kernel/ -name '*.c')
do
    filename_no_dir=$(basename -- "$i")
    filename_no_extension="${filename_no_dir%.*}"
    object_file=" temp/${filename_no_extension}.c.o"
    OBJECT_FILES+=$object_file
    gcc -c "$i" -c -o $object_file -std=gnu99 -ffreestanding -g -fno-stack-protector
done

# Build all Assembly files
for i in $(find kernel/ -name '*.asm')
do
    filename_no_dir=$(basename -- "$i")
    filename_no_extension="${filename_no_dir%.*}"
    object_file=" temp/${filename_no_extension}.s.o"
    OBJECT_FILES+=$object_file
    nasm -f elf64 "$i" -o $object_file -g 
    # nasm "$i" -o $object_file
done

# Link all together
echo $OBJECT_FILES
# gcc -T linker.ld -o paros.bin -ffreestanding -O2 -nostdlib -lgcc -g -fno-stack-protector -z noexecstack -Wl,--build-id=none -v $OBJECT_FILES 
# gcc -T linker.ld -o paros.bin -ffreestanding -O2 -nostdlib temp/interrupt.s.o $OBJECT_FILES -lgcc -g -fno-stack-protector -z noexecstack -Wl,--build-id=none
ld -m elf_x86_64 -nostdlib -static -pie --no-dynamic-linker -z text -z max-page-size=0x1000 -T linker.ld -g $OBJECT_FILES -o paros.bin

rm -rf temp

rm -rf isodir
