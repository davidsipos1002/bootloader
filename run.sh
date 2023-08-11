#!/bin/zsh
echo 'Starting QEMU...'
qemu-system-x86_64 -bios bios/UEFI64.bin -drive file=build/img/bootloader.img,format=raw &