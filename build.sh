#!/bin/bash

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create temp directory
mkdir -p temp

# Find all source files and create object files
OBJECT_FILES=""
INCLUDE_DIRS=""

# Function to add include directories for each source file
add_include_dirs() {
  local source_file="$1"
  local include_dir=$(dirname "$source_file")
  INCLUDE_DIRS+=" -I$include_dir"
}

# Build all C files
for c_file in $(find kernel/ -name '*.c')
do
  add_include_dirs "$c_file"
  filename_no_dir=$(basename -- "$c_file")
  filename_no_extension="${filename_no_dir%.*}"
  object_file="temp/${filename_no_extension}.c.o"
  OBJECT_FILES+=" $object_file"
  echo -e "Compiling ${GREEN}$c_file${NC} to ${BLUE}$object_file${NC}"
  gcc $INCLUDE_DIRS -c "$c_file" -o "$object_file" -std=gnu99 -ffreestanding -g -fno-stack-protector
done

# Build all Assembly files
for asm_file in $(find kernel/ -name '*.asm')
do
  add_include_dirs "$asm_file"
  filename_no_dir=$(basename -- "$asm_file")
  filename_no_extension="${filename_no_dir%.*}"
  object_file="temp/${filename_no_extension}.s.o"
  OBJECT_FILES+=" $object_file"
  echo -e "Assembling ${RED}$asm_file${NC} to ${BLUE}$object_file${NC}"
  nasm -f elf64 "$asm_file" -o "$object_file" -g 
done

# Link all object files
echo -e "Linking all object files to ${RED}paros.bin${NC}"
ld -m elf_x86_64 -nostdlib -static -pie --no-dynamic-linker -z text -z max-page-size=0x1000 -T linker.ld -g $OBJECT_FILES -o paros.bin

# Clean up
rm -rf temp
rm -rf isodir
