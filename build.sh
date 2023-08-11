#!/bin/zsh
ROOT_DIR=`pwd`
COMPILE_COMMAND="x86_64-w64-mingw32-gcc -ffreestanding -I$ROOT_DIR/include/ -I$ROOT_DIR/include/UEFI -I$ROOT_DIR/include/UEFI/X64 -c -o" 
LINK_COMMAND="x86_64-w64-mingw32-gcc -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $ROOT_DIR/build/efi/BOOTX64.EFI"

./clean.sh
cd src

echo 'Compiling bootloader..'
for file in *.c
do
    echo Compiling ${file}...
    COMMAND_ARGS="$ROOT_DIR/build/objects/$file.o $file"
    COMMAND="${COMPILE_COMMAND} ${COMMAND_ARGS}"
    if eval ${COMMAND}
    then
        echo ${file} compiled succesfully
    else
        echo ${file} was not compiled
        echo 'Bootloader compilation failed'
        exit
    fi
done
echo 'Bootloader compiled successfully'

echo 'Linking bootloader...'
cd $ROOT_DIR/build
FILES=""
for file in objects/*.o
do
    FILES="$FILES $file"
done 
COMMAND="${LINK_COMMAND} ${FILES}"
if eval ${COMMAND} 
then
    echo 'Bootloader linked successfully'
else
    echo 'Bootloader linking failed' 
fi
