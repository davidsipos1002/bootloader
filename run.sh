#!/bin/zsh
echo 'Starting QEMU...'
qemu-system-x86_64 -monitor stdio -bios bios/UEFI64.bin -drive file=build/img/bootloader.img,format=raw -vga std -full-screen -m 4G 