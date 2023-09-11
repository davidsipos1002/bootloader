#!/bin/zsh

echo 'Cleaning working directory...'
rm -rf build
mkdir build
mkdir build/efi
mkdir build/img
mkdir build/kernel
mkdir build/config
cp kernel build/kernel
cp boot.cfg build/config