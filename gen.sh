#!/bin/zsh
ROOT_DIR=`pwd`
BUILD_DIR="${ROOT_DIR}/build"

echo 'Generating hard disk image...'
cd $BUILD_DIR
dd if=/dev/zero of=img/bootloader.img bs=1k count=1440
mformat -i img/bootloader.img -f 1440 ::
mmd -i img/bootloader.img ::/EFI
mmd -i img/bootloader.img ::/EFI/BOOT
mmd -i img/bootloader.img ::/EFI/KERNEL
mmd -i img/bootloader.img ::/config
mcopy -i img/bootloader.img efi/BOOTX64.EFI ::/EFI/BOOT
mcopy -i img/bootloader.img kernel/kernel ::/EFI/KERNEL
mcopy -i img/bootloader.img config/boot.cfg ::/config