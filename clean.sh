echo 'Cleaning working directory...'
rm -rf build
mkdir build
mkdir build/efi
mkdir build/img
mkdir build/kernel
cp kernel build/kernel